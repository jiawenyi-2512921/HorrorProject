# AlertOnPerformanceDrop.ps1 - 性能下降告警
param(
    [int]$CheckInterval = 60,
    [double]$CPUThreshold = 90.0,
    [double]$MemoryThreshold = 85.0
)

$ErrorActionPreference = "Stop"
$MonitoringRoot = Split-Path -Parent $PSScriptRoot
$DataDir = Join-Path $MonitoringRoot "Data\Performance"
$AlertScript = Join-Path $PSScriptRoot "AlertSystem.ps1"

function Check-PerformanceIssues {
    $latestFile = Join-Path $DataDir "perf-latest.json"

    if (-not (Test-Path $latestFile)) {
        return $null
    }

    $metrics = Get-Content $latestFile | ConvertFrom-Json

    # 检查CPU使用率
    if ($metrics.System.CPU.Usage -gt $CPUThreshold) {
        return @{
            Type = "HighCPU"
            Severity = "Critical"
            Message = "CPU usage is critically high: $($metrics.System.CPU.Usage)%"
            Details = @{
                CPUUsage = $metrics.System.CPU.Usage
                Threshold = $CPUThreshold
                Cores = $metrics.System.CPU.Cores
            }
        }
    }

    # 检查内存使用率
    if ($metrics.System.Memory.Usage -gt $MemoryThreshold) {
        return @{
            Type = "HighMemory"
            Severity = "Critical"
            Message = "Memory usage is critically high: $($metrics.System.Memory.Usage)%"
            Details = @{
                MemoryUsage = $metrics.System.Memory.Usage
                UsedGB = $metrics.System.Memory.UsedGB
                TotalGB = $metrics.System.Memory.TotalGB
                Threshold = $MemoryThreshold
            }
        }
    }

    # 检查磁盘空间
    if ($metrics.System.Disk.Usage -gt 90) {
        return @{
            Type = "LowDiskSpace"
            Severity = "Warning"
            Message = "Disk space is running low: $($metrics.System.Disk.Usage)%"
            Details = @{
                DiskUsage = $metrics.System.Disk.Usage
                UsedGB = $metrics.System.Disk.UsedGB
                TotalGB = $metrics.System.Disk.TotalGB
            }
        }
    }

    # 检查进程性能
    if ($metrics.Process.Status -eq "Running") {
        if ($metrics.Process.CPUUsage -gt 80) {
            return @{
                Type = "ProcessHighCPU"
                Severity = "Warning"
                Message = "Process CPU usage is high: $($metrics.Process.CPUUsage)%"
                Details = @{
                    ProcessId = $metrics.Process.ProcessId
                    CPUUsage = $metrics.Process.CPUUsage
                    MemoryMB = $metrics.Process.MemoryMB
                    ThreadCount = $metrics.Process.ThreadCount
                }
            }
        }

        if ($metrics.Process.MemoryPercent -gt 70) {
            return @{
                Type = "ProcessHighMemory"
                Severity = "Warning"
                Message = "Process memory usage is high: $($metrics.Process.MemoryPercent)%"
                Details = @{
                    ProcessId = $metrics.Process.ProcessId
                    MemoryMB = $metrics.Process.MemoryMB
                    MemoryPercent = $metrics.Process.MemoryPercent
                }
            }
        }
    }

    return $null
}

Write-Host "Starting Performance Drop Alert Monitor..." -ForegroundColor Green
Write-Host "Check Interval: $CheckInterval seconds"
Write-Host "CPU Threshold: $CPUThreshold%"
Write-Host "Memory Threshold: $MemoryThreshold%"
Write-Host ""

while ($true) {
    try {
        $alert = Check-PerformanceIssues

        if ($alert) {
            & $AlertScript -Type $alert.Type -Severity $alert.Severity -Message $alert.Message -Details $alert.Details
        }

    } catch {
        Write-Host "Error: $_" -ForegroundColor Red
    }

    Start-Sleep -Seconds $CheckInterval
}
