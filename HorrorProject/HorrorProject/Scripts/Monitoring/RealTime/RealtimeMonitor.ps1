# RealtimeMonitor.ps1 - 实时监控主程序
param(
    [int]$UpdateInterval = 60,
    [string]$ConfigFile = "monitor-config.json",
    [switch]$EnableAlerts
)

$ErrorActionPreference = "Stop"
$MonitoringRoot = Split-Path -Parent $PSScriptRoot
$DataDir = Join-Path $MonitoringRoot "Data"
$LogDir = Join-Path $MonitoringRoot "Logs"

New-Item -ItemType Directory -Force -Path $DataDir, $LogDir | Out-Null

class MonitoringMetrics {
    [datetime]$Timestamp
    [hashtable]$BuildStatus
    [hashtable]$TestStatus
    [hashtable]$Performance
    [hashtable]$AssetStatus
    [hashtable]$SystemHealth
}

function Initialize-MonitoringConfig {
    $config = @{
        UpdateInterval = $UpdateInterval
        EnableAlerts = $EnableAlerts.IsPresent
        Monitors = @{
            Build = $true
            Test = $true
            Performance = $true
            Assets = $true
        }
        Thresholds = @{
            BuildTime = 600
            TestFailureRate = 0.05
            MemoryUsage = 0.85
            CPUUsage = 0.90
            AssetErrors = 10
        }
        AlertChannels = @{
            Email = $true
            Slack = $false
            Console = $true
        }
    }

    $configPath = Join-Path $MonitoringRoot $ConfigFile
    $config | ConvertTo-Json -Depth 10 | Set-Content $configPath
    return $config
}

function Get-BuildMetrics {
    $buildLog = Join-Path $LogDir "build-latest.log"
    if (-not (Test-Path $buildLog)) {
        return @{
            Status = "Unknown"
            Duration = 0
            Errors = 0
            Warnings = 0
        }
    }

    $content = Get-Content $buildLog -Raw
    $status = if ($content -match "BUILD SUCCESSFUL") { "Success" }
              elseif ($content -match "BUILD FAILED") { "Failed" }
              else { "InProgress" }

    $duration = if ($content -match "Total time: (\d+)") { [int]$matches[1] } else { 0 }
    $errors = ([regex]::Matches($content, "error")).Count
    $warnings = ([regex]::Matches($content, "warning")).Count

    return @{
        Status = $status
        Duration = $duration
        Errors = $errors
        Warnings = $warnings
        LastUpdate = (Get-Item $buildLog).LastWriteTime
    }
}

function Get-TestMetrics {
    $testLog = Join-Path $LogDir "test-latest.log"
    if (-not (Test-Path $testLog)) {
        return @{
            Status = "Unknown"
            Total = 0
            Passed = 0
            Failed = 0
            Skipped = 0
        }
    }

    $content = Get-Content $testLog -Raw
    $total = if ($content -match "Total: (\d+)") { [int]$matches[1] } else { 0 }
    $passed = if ($content -match "Passed: (\d+)") { [int]$matches[1] } else { 0 }
    $failed = if ($content -match "Failed: (\d+)") { [int]$matches[1] } else { 0 }
    $skipped = if ($content -match "Skipped: (\d+)") { [int]$matches[1] } else { 0 }

    return @{
        Status = if ($failed -eq 0) { "Passed" } else { "Failed" }
        Total = $total
        Passed = $passed
        Failed = $failed
        Skipped = $skipped
        FailureRate = if ($total -gt 0) { $failed / $total } else { 0 }
        LastUpdate = (Get-Item $testLog).LastWriteTime
    }
}

function Get-PerformanceMetrics {
    $process = Get-Process -Name "UnrealEditor*" -ErrorAction SilentlyContinue | Select-Object -First 1

    if (-not $process) {
        return @{
            Status = "NotRunning"
            CPU = 0
            Memory = 0
            Threads = 0
        }
    }

    $cpu = [math]::Round($process.CPU / [Environment]::ProcessorCount, 2)
    $memory = [math]::Round($process.WorkingSet64 / 1GB, 2)

    return @{
        Status = "Running"
        CPU = $cpu
        Memory = $memory
        Threads = $process.Threads.Count
        HandleCount = $process.HandleCount
        LastUpdate = Get-Date
    }
}

function Get-AssetMetrics {
    $assetLog = Join-Path $LogDir "asset-validation.log"
    if (-not (Test-Path $assetLog)) {
        return @{
            Status = "Unknown"
            Total = 0
            Valid = 0
            Errors = 0
            Warnings = 0
        }
    }

    $content = Get-Content $assetLog -Raw
    $total = if ($content -match "Total Assets: (\d+)") { [int]$matches[1] } else { 0 }
    $errors = ([regex]::Matches($content, "ERROR")).Count
    $warnings = ([regex]::Matches($content, "WARNING")).Count

    return @{
        Status = if ($errors -eq 0) { "Healthy" } else { "Issues" }
        Total = $total
        Valid = $total - $errors
        Errors = $errors
        Warnings = $warnings
        LastUpdate = (Get-Item $assetLog).LastWriteTime
    }
}

function Get-SystemHealth {
    $disk = Get-PSDrive C | Select-Object Used, Free
    $diskUsage = [math]::Round($disk.Used / ($disk.Used + $disk.Free), 2)

    $memory = Get-CimInstance Win32_OperatingSystem
    $memoryUsage = [math]::Round(($memory.TotalVisibleMemorySize - $memory.FreePhysicalMemory) / $memory.TotalVisibleMemorySize, 2)

    $cpu = (Get-Counter '\Processor(_Total)\% Processor Time' -SampleInterval 1 -MaxSamples 1).CounterSamples.CookedValue
    $cpuUsage = [math]::Round($cpu / 100, 2)

    return @{
        DiskUsage = $diskUsage
        MemoryUsage = $memoryUsage
        CPUUsage = $cpuUsage
        Status = if ($diskUsage -gt 0.9 -or $memoryUsage -gt 0.9) { "Critical" }
                 elseif ($diskUsage -gt 0.8 -or $memoryUsage -gt 0.8) { "Warning" }
                 else { "Healthy" }
    }
}

function Collect-Metrics {
    $metrics = [MonitoringMetrics]::new()
    $metrics.Timestamp = Get-Date
    $metrics.BuildStatus = Get-BuildMetrics
    $metrics.TestStatus = Get-TestMetrics
    $metrics.Performance = Get-PerformanceMetrics
    $metrics.AssetStatus = Get-AssetMetrics
    $metrics.SystemHealth = Get-SystemHealth

    return $metrics
}

function Save-Metrics {
    param([MonitoringMetrics]$Metrics)

    $timestamp = $Metrics.Timestamp.ToString("yyyyMMdd-HHmmss")
    $metricsFile = Join-Path $DataDir "metrics-$timestamp.json"

    $Metrics | ConvertTo-Json -Depth 10 | Set-Content $metricsFile

    # 保存最新指标
    $latestFile = Join-Path $DataDir "metrics-latest.json"
    $Metrics | ConvertTo-Json -Depth 10 | Set-Content $latestFile

    Write-Host "Metrics saved: $metricsFile"
}

function Check-Thresholds {
    param(
        [MonitoringMetrics]$Metrics,
        [hashtable]$Config
    )

    $alerts = @()
    $thresholds = $Config.Thresholds

    # 检查构建时间
    if ($Metrics.BuildStatus.Duration -gt $thresholds.BuildTime) {
        $alerts += @{
            Type = "BuildTime"
            Severity = "Warning"
            Message = "Build time exceeded threshold: $($Metrics.BuildStatus.Duration)s > $($thresholds.BuildTime)s"
        }
    }

    # 检查测试失败率
    if ($Metrics.TestStatus.FailureRate -gt $thresholds.TestFailureRate) {
        $alerts += @{
            Type = "TestFailure"
            Severity = "Critical"
            Message = "Test failure rate: $([math]::Round($Metrics.TestStatus.FailureRate * 100, 2))%"
        }
    }

    # 检查内存使用
    if ($Metrics.SystemHealth.MemoryUsage -gt $thresholds.MemoryUsage) {
        $alerts += @{
            Type = "Memory"
            Severity = "Warning"
            Message = "Memory usage: $([math]::Round($Metrics.SystemHealth.MemoryUsage * 100, 2))%"
        }
    }

    # 检查资产错误
    if ($Metrics.AssetStatus.Errors -gt $thresholds.AssetErrors) {
        $alerts += @{
            Type = "Assets"
            Severity = "Warning"
            Message = "Asset errors: $($Metrics.AssetStatus.Errors)"
        }
    }

    return $alerts
}

function Send-Alerts {
    param([array]$Alerts)

    if ($Alerts.Count -eq 0) { return }

    foreach ($alert in $Alerts) {
        $message = "[$($alert.Severity)] $($alert.Type): $($alert.Message)"
        Write-Host $message -ForegroundColor $(if ($alert.Severity -eq "Critical") { "Red" } else { "Yellow" })

        # 调用告警系统
        $alertScript = Join-Path $MonitoringRoot "Alerts\AlertSystem.ps1"
        if (Test-Path $alertScript) {
            & $alertScript -Type $alert.Type -Severity $alert.Severity -Message $alert.Message
        }
    }
}

function Start-RealtimeMonitoring {
    Write-Host "=== Realtime Monitoring Started ===" -ForegroundColor Green
    Write-Host "Update Interval: $UpdateInterval seconds"
    Write-Host "Press Ctrl+C to stop"
    Write-Host ""

    $config = Initialize-MonitoringConfig
    $iteration = 0

    while ($true) {
        $iteration++
        Write-Host "`n[$(Get-Date -Format 'yyyy-MM-dd HH:mm:ss')] Iteration #$iteration" -ForegroundColor Cyan

        try {
            # 收集指标
            $metrics = Collect-Metrics

            # 显示摘要
            Write-Host "Build: $($metrics.BuildStatus.Status) | Tests: $($metrics.TestStatus.Status) | Assets: $($metrics.AssetStatus.Status) | System: $($metrics.SystemHealth.Status)"

            # 保存指标
            Save-Metrics -Metrics $metrics

            # 检查阈值并发送告警
            if ($config.EnableAlerts) {
                $alerts = Check-Thresholds -Metrics $metrics -Config $config
                if ($alerts.Count -gt 0) {
                    Send-Alerts -Alerts $alerts
                }
            }

            # 清理旧数据（保留7天）
            $cutoffDate = (Get-Date).AddDays(-7)
            Get-ChildItem $DataDir -Filter "metrics-*.json" |
                Where-Object { $_.LastWriteTime -lt $cutoffDate } |
                Remove-Item -Force

        } catch {
            Write-Host "Error in monitoring iteration: $_" -ForegroundColor Red
        }

        Start-Sleep -Seconds $UpdateInterval
    }
}

# 主执行
Start-RealtimeMonitoring
