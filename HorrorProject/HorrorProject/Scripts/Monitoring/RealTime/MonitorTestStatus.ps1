# MonitorTestStatus.ps1 - 测试状态监控
param(
    [string]$ProjectPath = "D:\gptzuo\HorrorProject",
    [int]$CheckInterval = 30
)

$ErrorActionPreference = "Stop"
$MonitoringRoot = Split-Path -Parent $PSScriptRoot
$DataDir = Join-Path $MonitoringRoot "Data\Test"
$LogDir = Join-Path $MonitoringRoot "Logs"

New-Item -ItemType Directory -Force -Path $DataDir, $LogDir | Out-Null

class TestResult {
    [datetime]$Timestamp
    [string]$Status
    [int]$Total
    [int]$Passed
    [int]$Failed
    [int]$Skipped
    [double]$Duration
    [double]$FailureRate
    [array]$FailedTests
    [hashtable]$Coverage
}

function Get-LatestTestLog {
    $testLogs = @(
        "$LogDir\test-*.log",
        "$ProjectPath\Saved\Logs\test-*.log"
    )

    foreach ($pattern in $testLogs) {
        $logs = Get-ChildItem $pattern -ErrorAction SilentlyContinue | Sort-Object LastWriteTime -Descending
        if ($logs) { return $logs[0] }
    }
    return $null
}

function Parse-TestLog {
    param([string]$LogPath)

    if (-not (Test-Path $LogPath)) { return $null }

    $content = Get-Content $LogPath -Raw
    $result = [TestResult]::new()
    $result.Timestamp = (Get-Item $LogPath).LastWriteTime

    # 解析测试统计
    if ($content -match "Total Tests:\s*(\d+)") { $result.Total = [int]$matches[1] }
    if ($content -match "Passed:\s*(\d+)") { $result.Passed = [int]$matches[1] }
    if ($content -match "Failed:\s*(\d+)") { $result.Failed = [int]$matches[1] }
    if ($content -match "Skipped:\s*(\d+)") { $result.Skipped = [int]$matches[1] }
    if ($content -match "Duration:\s*(\d+\.?\d*)\s*seconds") { $result.Duration = [double]$matches[1] }

    # 计算失败率
    if ($result.Total -gt 0) {
        $result.FailureRate = [math]::Round($result.Failed / $result.Total * 100, 2)
    }

    # 提取失败的测试
    $result.FailedTests = @()
    $lines = $content -split "`n"
    foreach ($line in $lines) {
        if ($line -match "FAILED:\s*(.+)") {
            $result.FailedTests += $matches[1].Trim()
        }
    }

    # 解析覆盖率
    $result.Coverage = @{
        Line = 0
        Branch = 0
        Function = 0
    }
    if ($content -match "Line Coverage:\s*(\d+\.?\d*)%") { $result.Coverage.Line = [double]$matches[1] }
    if ($content -match "Branch Coverage:\s*(\d+\.?\d*)%") { $result.Coverage.Branch = [double]$matches[1] }
    if ($content -match "Function Coverage:\s*(\d+\.?\d*)%") { $result.Coverage.Function = [double]$matches[1] }

    # 确定状态
    $result.Status = if ($result.Failed -eq 0) { "Passed" } else { "Failed" }

    return $result
}

function Get-TestMetrics {
    $testLog = Get-LatestTestLog
    if (-not $testLog) {
        return @{
            Status = "NoData"
            Message = "No test logs found"
        }
    }

    $result = Parse-TestLog -LogPath $testLog.FullName
    if (-not $result) {
        return @{
            Status = "ParseError"
            Message = "Failed to parse test log"
        }
    }

    return @{
        Status = $result.Status
        Total = $result.Total
        Passed = $result.Passed
        Failed = $result.Failed
        Skipped = $result.Skipped
        Duration = $result.Duration
        FailureRate = $result.FailureRate
        FailedTests = $result.FailedTests
        Coverage = $result.Coverage
        Timestamp = $result.Timestamp
        LogFile = $testLog.FullName
    }
}

function Get-TestTrend {
    param([int]$Hours = 24)

    $cutoff = (Get-Date).AddHours(-$Hours)
    $testFiles = Get-ChildItem $DataDir -Filter "test-status-*.json" |
        Where-Object { $_.LastWriteTime -gt $cutoff } |
        Sort-Object LastWriteTime

    $trend = @{
        TotalRuns = 0
        PassedRuns = 0
        FailedRuns = 0
        AverageDuration = 0
        AverageFailureRate = 0
        TotalTests = 0
        TotalFailures = 0
        MostFailedTests = @{}
    }

    if ($testFiles.Count -eq 0) { return $trend }

    $durations = @()
    $failureRates = @()

    foreach ($file in $testFiles) {
        $data = Get-Content $file.FullName | ConvertFrom-Json
        $trend.TotalRuns++

        if ($data.Status -eq "Passed") {
            $trend.PassedRuns++
        } else {
            $trend.FailedRuns++
        }

        $durations += $data.Duration
        $failureRates += $data.FailureRate
        $trend.TotalTests += $data.Total
        $trend.TotalFailures += $data.Failed

        # 统计最常失败的测试
        foreach ($test in $data.FailedTests) {
            if (-not $trend.MostFailedTests.ContainsKey($test)) {
                $trend.MostFailedTests[$test] = 0
            }
            $trend.MostFailedTests[$test]++
        }
    }

    if ($durations.Count -gt 0) {
        $trend.AverageDuration = [math]::Round(($durations | Measure-Object -Average).Average, 2)
    }
    if ($failureRates.Count -gt 0) {
        $trend.AverageFailureRate = [math]::Round(($failureRates | Measure-Object -Average).Average, 2)
    }

    return $trend
}

function Save-TestStatus {
    param([hashtable]$Status)

    $timestamp = Get-Date -Format "yyyyMMdd-HHmmss"
    $statusFile = Join-Path $DataDir "test-status-$timestamp.json"

    $Status | ConvertTo-Json -Depth 10 | Set-Content $statusFile

    $latestFile = Join-Path $DataDir "test-status-latest.json"
    $Status | ConvertTo-Json -Depth 10 | Set-Content $latestFile
}

function Show-TestDashboard {
    param([hashtable]$Status, [hashtable]$Trend)

    Clear-Host
    Write-Host "╔════════════════════════════════════════════════════════════╗" -ForegroundColor Cyan
    Write-Host "║            TEST STATUS MONITORING DASHBOARD               ║" -ForegroundColor Cyan
    Write-Host "╚════════════════════════════════════════════════════════════╝" -ForegroundColor Cyan
    Write-Host ""

    # 当前测试状态
    Write-Host "Current Test Status:" -ForegroundColor Yellow
    $statusColor = if ($Status.Status -eq "Passed") { "Green" } else { "Red" }
    Write-Host "  Status: $($Status.Status)" -ForegroundColor $statusColor
    Write-Host "  Total Tests: $($Status.Total)"
    Write-Host "  Passed: $($Status.Passed)" -ForegroundColor Green
    Write-Host "  Failed: $($Status.Failed)" -ForegroundColor $(if ($Status.Failed -gt 0) { "Red" } else { "Green" })
    Write-Host "  Skipped: $($Status.Skipped)" -ForegroundColor Gray
    Write-Host "  Duration: $($Status.Duration)s"
    Write-Host "  Failure Rate: $($Status.FailureRate)%" -ForegroundColor $(if ($Status.FailureRate -gt 5) { "Red" } elseif ($Status.FailureRate -gt 0) { "Yellow" } else { "Green" })
    Write-Host ""

    # 代码覆盖率
    Write-Host "Code Coverage:" -ForegroundColor Yellow
    Write-Host "  Line: $($Status.Coverage.Line)%" -ForegroundColor $(if ($Status.Coverage.Line -gt 80) { "Green" } elseif ($Status.Coverage.Line -gt 60) { "Yellow" } else { "Red" })
    Write-Host "  Branch: $($Status.Coverage.Branch)%" -ForegroundColor $(if ($Status.Coverage.Branch -gt 70) { "Green" } elseif ($Status.Coverage.Branch -gt 50) { "Yellow" } else { "Red" })
    Write-Host "  Function: $($Status.Coverage.Function)%" -ForegroundColor $(if ($Status.Coverage.Function -gt 80) { "Green" } elseif ($Status.Coverage.Function -gt 60) { "Yellow" } else { "Red" })
    Write-Host ""

    # 24小时趋势
    Write-Host "24-Hour Trend:" -ForegroundColor Yellow
    Write-Host "  Total Runs: $($Trend.TotalRuns)"
    Write-Host "  Passed Runs: $($Trend.PassedRuns)" -ForegroundColor Green
    Write-Host "  Failed Runs: $($Trend.FailedRuns)" -ForegroundColor $(if ($Trend.FailedRuns -gt 0) { "Red" } else { "Green" })
    Write-Host "  Average Duration: $($Trend.AverageDuration)s"
    Write-Host "  Average Failure Rate: $($Trend.AverageFailureRate)%"
    Write-Host ""

    # 失败的测试
    if ($Status.FailedTests.Count -gt 0) {
        Write-Host "Failed Tests:" -ForegroundColor Red
        $Status.FailedTests | Select-Object -First 10 | ForEach-Object {
            Write-Host "  - $_" -ForegroundColor Red
        }
        Write-Host ""
    }

    # 最常失败的测试
    if ($Trend.MostFailedTests.Count -gt 0) {
        Write-Host "Most Failed Tests (24h):" -ForegroundColor Yellow
        $Trend.MostFailedTests.GetEnumerator() | Sort-Object Value -Descending | Select-Object -First 5 | ForEach-Object {
            Write-Host "  - $($_.Key): $($_.Value) failures" -ForegroundColor Yellow
        }
        Write-Host ""
    }

    Write-Host "Last updated: $(Get-Date -Format 'yyyy-MM-dd HH:mm:ss')" -ForegroundColor Gray
}

function Start-TestMonitoring {
    Write-Host "Starting Test Status Monitoring..." -ForegroundColor Green
    Write-Host "Check Interval: $CheckInterval seconds"
    Write-Host "Press Ctrl+C to stop"
    Write-Host ""

    while ($true) {
        try {
            $status = Get-TestMetrics
            $trend = Get-TestTrend -Hours 24

            Save-TestStatus -Status $status
            Show-TestDashboard -Status $status -Trend $trend

            # 清理旧数据
            $cutoff = (Get-Date).AddDays(-7)
            Get-ChildItem $DataDir -Filter "test-status-*.json" |
                Where-Object { $_.LastWriteTime -lt $cutoff } |
                Remove-Item -Force

        } catch {
            Write-Host "Error: $_" -ForegroundColor Red
        }

        Start-Sleep -Seconds $CheckInterval
    }
}

Start-TestMonitoring
