# MonitorBuildStatus.ps1 - 构建状态监控
param(
    [string]$ProjectPath = "D:\gptzuo\HorrorProject",
    [int]$CheckInterval = 30
)

$ErrorActionPreference = "Stop"
$MonitoringRoot = Split-Path -Parent $PSScriptRoot
$DataDir = Join-Path $MonitoringRoot "Data\Build"
$LogDir = Join-Path $MonitoringRoot "Logs"

New-Item -ItemType Directory -Force -Path $DataDir, $LogDir | Out-Null

class BuildStatus {
    [datetime]$Timestamp
    [string]$Status
    [int]$Duration
    [int]$Errors
    [int]$Warnings
    [string]$Configuration
    [string]$Platform
    [hashtable]$Details
}

function Get-LatestBuildLog {
    $buildLogs = @(
        "D:\gptzuo\HorrorProject\Saved\Logs\*.log",
        "$LogDir\build-*.log"
    )

    $latestLog = $null
    foreach ($pattern in $buildLogs) {
        $logs = Get-ChildItem $pattern -ErrorAction SilentlyContinue | Sort-Object LastWriteTime -Descending
        if ($logs) {
            $latestLog = $logs[0]
            break
        }
    }

    return $latestLog
}

function Parse-BuildLog {
    param([string]$LogPath)

    if (-not (Test-Path $LogPath)) {
        return $null
    }

    $content = Get-Content $LogPath -Raw
    $status = [BuildStatus]::new()
    $status.Timestamp = (Get-Item $LogPath).LastWriteTime

    # 解析构建状态
    if ($content -match "BUILD SUCCESSFUL") {
        $status.Status = "Success"
    } elseif ($content -match "BUILD FAILED") {
        $status.Status = "Failed"
    } elseif ($content -match "BUILD CANCELLED") {
        $status.Status = "Cancelled"
    } else {
        $status.Status = "InProgress"
    }

    # 解析构建时间
    if ($content -match "Total build time: (\d+\.?\d*)\s*seconds") {
        $status.Duration = [int][math]::Ceiling([double]$matches[1])
    }

    # 统计错误和警告
    $status.Errors = ([regex]::Matches($content, "(?i)error\s+[A-Z]+\d+:")).Count
    $status.Warnings = ([regex]::Matches($content, "(?i)warning\s+[A-Z]+\d+:")).Count

    # 解析配置和平台
    if ($content -match "Configuration:\s*(\w+)") {
        $status.Configuration = $matches[1]
    }
    if ($content -match "Platform:\s*(\w+)") {
        $status.Platform = $matches[1]
    }

    # 详细信息
    $status.Details = @{
        LogFile = $LogPath
        FileSize = (Get-Item $LogPath).Length
        ErrorLines = @()
        WarningLines = @()
    }

    # 提取错误和警告行
    $lines = $content -split "`n"
    foreach ($line in $lines) {
        if ($line -match "(?i)error") {
            $status.Details.ErrorLines += $line.Trim()
        }
        if ($line -match "(?i)warning" -and $status.Details.WarningLines.Count -lt 10) {
            $status.Details.WarningLines += $line.Trim()
        }
    }

    return $status
}

function Get-BuildMetrics {
    $buildLog = Get-LatestBuildLog
    if (-not $buildLog) {
        return @{
            Status = "NoData"
            Message = "No build logs found"
        }
    }

    $status = Parse-BuildLog -LogPath $buildLog.FullName
    if (-not $status) {
        return @{
            Status = "ParseError"
            Message = "Failed to parse build log"
        }
    }

    return @{
        Status = $status.Status
        Duration = $status.Duration
        Errors = $status.Errors
        Warnings = $status.Warnings
        Configuration = $status.Configuration
        Platform = $status.Platform
        Timestamp = $status.Timestamp
        LogFile = $buildLog.FullName
        Details = $status.Details
    }
}

function Get-BuildTrend {
    param([int]$Hours = 24)

    $cutoff = (Get-Date).AddHours(-$Hours)
    $buildFiles = Get-ChildItem $DataDir -Filter "build-status-*.json" |
        Where-Object { $_.LastWriteTime -gt $cutoff } |
        Sort-Object LastWriteTime

    $trend = @{
        TotalBuilds = 0
        SuccessfulBuilds = 0
        FailedBuilds = 0
        AverageDuration = 0
        TotalErrors = 0
        TotalWarnings = 0
        SuccessRate = 0
    }

    if ($buildFiles.Count -eq 0) {
        return $trend
    }

    $durations = @()
    foreach ($file in $buildFiles) {
        $data = Get-Content $file.FullName | ConvertFrom-Json
        $trend.TotalBuilds++

        if ($data.Status -eq "Success") {
            $trend.SuccessfulBuilds++
        } elseif ($data.Status -eq "Failed") {
            $trend.FailedBuilds++
        }

        $durations += $data.Duration
        $trend.TotalErrors += $data.Errors
        $trend.TotalWarnings += $data.Warnings
    }

    if ($durations.Count -gt 0) {
        $trend.AverageDuration = [math]::Round(($durations | Measure-Object -Average).Average, 2)
    }

    if ($trend.TotalBuilds -gt 0) {
        $trend.SuccessRate = [math]::Round($trend.SuccessfulBuilds / $trend.TotalBuilds * 100, 2)
    }

    return $trend
}

function Save-BuildStatus {
    param([hashtable]$Status)

    $timestamp = Get-Date -Format "yyyyMMdd-HHmmss"
    $statusFile = Join-Path $DataDir "build-status-$timestamp.json"

    $Status | ConvertTo-Json -Depth 10 | Set-Content $statusFile

    # 更新最新状态
    $latestFile = Join-Path $DataDir "build-status-latest.json"
    $Status | ConvertTo-Json -Depth 10 | Set-Content $latestFile
}

function Show-BuildDashboard {
    param([hashtable]$Status, [hashtable]$Trend)

    Clear-Host
    Write-Host "╔════════════════════════════════════════════════════════════╗" -ForegroundColor Cyan
    Write-Host "║           BUILD STATUS MONITORING DASHBOARD               ║" -ForegroundColor Cyan
    Write-Host "╚════════════════════════════════════════════════════════════╝" -ForegroundColor Cyan
    Write-Host ""

    # 当前状态
    Write-Host "Current Build Status:" -ForegroundColor Yellow
    $statusColor = switch ($Status.Status) {
        "Success" { "Green" }
        "Failed" { "Red" }
        "InProgress" { "Yellow" }
        default { "Gray" }
    }
    Write-Host "  Status: $($Status.Status)" -ForegroundColor $statusColor
    Write-Host "  Duration: $($Status.Duration)s"
    Write-Host "  Errors: $($Status.Errors)" -ForegroundColor $(if ($Status.Errors -gt 0) { "Red" } else { "Green" })
    Write-Host "  Warnings: $($Status.Warnings)" -ForegroundColor $(if ($Status.Warnings -gt 0) { "Yellow" } else { "Green" })
    Write-Host "  Configuration: $($Status.Configuration)"
    Write-Host "  Platform: $($Status.Platform)"
    Write-Host "  Last Update: $($Status.Timestamp)"
    Write-Host ""

    # 24小时趋势
    Write-Host "24-Hour Trend:" -ForegroundColor Yellow
    Write-Host "  Total Builds: $($Trend.TotalBuilds)"
    Write-Host "  Success Rate: $($Trend.SuccessRate)%" -ForegroundColor $(if ($Trend.SuccessRate -gt 90) { "Green" } else { "Yellow" })
    Write-Host "  Average Duration: $($Trend.AverageDuration)s"
    Write-Host "  Total Errors: $($Trend.TotalErrors)"
    Write-Host "  Total Warnings: $($Trend.TotalWarnings)"
    Write-Host ""

    # 错误详情
    if ($Status.Details.ErrorLines.Count -gt 0) {
        Write-Host "Recent Errors:" -ForegroundColor Red
        $Status.Details.ErrorLines | Select-Object -First 5 | ForEach-Object {
            Write-Host "  $_" -ForegroundColor Red
        }
        Write-Host ""
    }

    Write-Host "Last updated: $(Get-Date -Format 'yyyy-MM-dd HH:mm:ss')" -ForegroundColor Gray
}

function Start-BuildMonitoring {
    Write-Host "Starting Build Status Monitoring..." -ForegroundColor Green
    Write-Host "Check Interval: $CheckInterval seconds"
    Write-Host "Press Ctrl+C to stop"
    Write-Host ""

    while ($true) {
        try {
            $status = Get-BuildMetrics
            $trend = Get-BuildTrend -Hours 24

            Save-BuildStatus -Status $status
            Show-BuildDashboard -Status $status -Trend $trend

            # 清理旧数据
            $cutoff = (Get-Date).AddDays(-7)
            Get-ChildItem $DataDir -Filter "build-status-*.json" |
                Where-Object { $_.LastWriteTime -lt $cutoff } |
                Remove-Item -Force

        } catch {
            Write-Host "Error: $_" -ForegroundColor Red
        }

        Start-Sleep -Seconds $CheckInterval
    }
}

Start-BuildMonitoring
