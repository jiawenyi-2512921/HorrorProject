# MonitorPerformance.ps1 - 性能指标监控
param(
    [string]$ProcessName = "UnrealEditor",
    [int]$CheckInterval = 10
)

$ErrorActionPreference = "Stop"
$MonitoringRoot = Split-Path -Parent $PSScriptRoot
$DataDir = Join-Path $MonitoringRoot "Data\Performance"
$LogDir = Join-Path $MonitoringRoot "Logs"

New-Item -ItemType Directory -Force -Path $DataDir, $LogDir | Out-Null

class PerformanceMetrics {
    [datetime]$Timestamp
    [string]$ProcessName
    [int]$ProcessId
    [double]$CPUUsage
    [double]$MemoryMB
    [double]$MemoryPercent
    [int]$ThreadCount
    [int]$HandleCount
    [double]$DiskReadMB
    [double]$DiskWriteMB
    [hashtable]$SystemMetrics
}

function Get-ProcessMetrics {
    param([string]$Name)

    $process = Get-Process -Name $Name -ErrorAction SilentlyContinue | Select-Object -First 1

    if (-not $process) {
        return @{
            Status = "NotRunning"
            Message = "Process not found: $Name"
        }
    }

    # CPU使用率（需要两次采样）
    $cpuBefore = $process.CPU
    Start-Sleep -Milliseconds 500
    $process.Refresh()
    $cpuAfter = $process.CPU
    $cpuUsage = [math]::Round(($cpuAfter - $cpuBefore) * 2, 2)

    # 内存使用
    $memoryMB = [math]::Round($process.WorkingSet64 / 1MB, 2)
    $totalMemory = (Get-CimInstance Win32_ComputerSystem).TotalPhysicalMemory
    $memoryPercent = [math]::Round($process.WorkingSet64 / $totalMemory * 100, 2)

    # 磁盘I/O
    $diskRead = [math]::Round($process.PagedMemorySize64 / 1MB, 2)
    $diskWrite = [math]::Round($process.PagedSystemMemorySize64 / 1MB, 2)

    return @{
        Status = "Running"
        ProcessId = $process.Id
        CPUUsage = $cpuUsage
        MemoryMB = $memoryMB
        MemoryPercent = $memoryPercent
        ThreadCount = $process.Threads.Count
        HandleCount = $process.HandleCount
        DiskReadMB = $diskRead
        DiskWriteMB = $diskWrite
        StartTime = $process.StartTime
        Uptime = (Get-Date) - $process.StartTime
    }
}

function Get-SystemMetrics {
    # CPU
    $cpu = (Get-Counter '\Processor(_Total)\% Processor Time' -SampleInterval 1 -MaxSamples 1).CounterSamples.CookedValue
    $cpuUsage = [math]::Round($cpu, 2)

    # 内存
    $memory = Get-CimInstance Win32_OperatingSystem
    $totalMemoryGB = [math]::Round($memory.TotalVisibleMemorySize / 1MB, 2)
    $freeMemoryGB = [math]::Round($memory.FreePhysicalMemory / 1MB, 2)
    $usedMemoryGB = $totalMemoryGB - $freeMemoryGB
    $memoryUsage = [math]::Round($usedMemoryGB / $totalMemoryGB * 100, 2)

    # 磁盘
    $disk = Get-PSDrive C
    $totalDiskGB = [math]::Round(($disk.Used + $disk.Free) / 1GB, 2)
    $usedDiskGB = [math]::Round($disk.Used / 1GB, 2)
    $diskUsage = [math]::Round($usedDiskGB / $totalDiskGB * 100, 2)

    # 网络
    $networkCounters = Get-Counter '\Network Interface(*)\Bytes Total/sec' -ErrorAction SilentlyContinue
    $networkUsage = if ($networkCounters) {
        [math]::Round(($networkCounters.CounterSamples | Measure-Object -Property CookedValue -Sum).Sum / 1MB, 2)
    } else { 0 }

    return @{
        CPU = @{
            Usage = $cpuUsage
            Cores = [Environment]::ProcessorCount
        }
        Memory = @{
            TotalGB = $totalMemoryGB
            UsedGB = $usedMemoryGB
            FreeGB = $freeMemoryGB
            Usage = $memoryUsage
        }
        Disk = @{
            TotalGB = $totalDiskGB
            UsedGB = $usedDiskGB
            FreeGB = $totalDiskGB - $usedDiskGB
            Usage = $diskUsage
        }
        Network = @{
            ThroughputMBps = $networkUsage
        }
    }
}

function Get-PerformanceMetrics {
    $processMetrics = Get-ProcessMetrics -Name $ProcessName
    $systemMetrics = Get-SystemMetrics

    return @{
        Timestamp = Get-Date
        Process = $processMetrics
        System = $systemMetrics
    }
}

function Get-PerformanceTrend {
    param([int]$Minutes = 60)

    $cutoff = (Get-Date).AddMinutes(-$Minutes)
    $perfFiles = Get-ChildItem $DataDir -Filter "perf-*.json" |
        Where-Object { $_.LastWriteTime -gt $cutoff } |
        Sort-Object LastWriteTime

    $trend = @{
        Samples = 0
        AvgCPU = 0
        MaxCPU = 0
        AvgMemory = 0
        MaxMemory = 0
        MemoryGrowth = 0
        CPUSpikes = 0
        MemoryLeaks = @()
    }

    if ($perfFiles.Count -eq 0) { return $trend }

    $cpuValues = @()
    $memoryValues = @()
    $timestamps = @()

    foreach ($file in $perfFiles) {
        $data = Get-Content $file.FullName | ConvertFrom-Json
        $trend.Samples++

        if ($data.Process.Status -eq "Running") {
            $cpuValues += $data.Process.CPUUsage
            $memoryValues += $data.Process.MemoryMB
            $timestamps += [datetime]$data.Timestamp

            if ($data.Process.CPUUsage -gt 80) {
                $trend.CPUSpikes++
            }
        }
    }

    if ($cpuValues.Count -gt 0) {
        $trend.AvgCPU = [math]::Round(($cpuValues | Measure-Object -Average).Average, 2)
        $trend.MaxCPU = [math]::Round(($cpuValues | Measure-Object -Maximum).Maximum, 2)
    }

    if ($memoryValues.Count -gt 0) {
        $trend.AvgMemory = [math]::Round(($memoryValues | Measure-Object -Average).Average, 2)
        $trend.MaxMemory = [math]::Round(($memoryValues | Measure-Object -Maximum).Maximum, 2)

        # 检测内存增长
        if ($memoryValues.Count -gt 10) {
            $firstHalf = $memoryValues[0..([math]::Floor($memoryValues.Count / 2) - 1)]
            $secondHalf = $memoryValues[([math]::Floor($memoryValues.Count / 2))..($memoryValues.Count - 1)]
            $avgFirst = ($firstHalf | Measure-Object -Average).Average
            $avgSecond = ($secondHalf | Measure-Object -Average).Average
            $trend.MemoryGrowth = [math]::Round($avgSecond - $avgFirst, 2)
        }
    }

    return $trend
}

function Detect-MemoryLeak {
    param([array]$MemoryValues, [int]$ThresholdMB = 100)

    if ($MemoryValues.Count -lt 10) { return $false }

    # 简单的线性回归检测内存泄漏
    $n = $MemoryValues.Count
    $x = 0..($n - 1)
    $y = $MemoryValues

    $sumX = ($x | Measure-Object -Sum).Sum
    $sumY = ($y | Measure-Object -Sum).Sum
    $sumXY = 0
    $sumX2 = 0

    for ($i = 0; $i -lt $n; $i++) {
        $sumXY += $x[$i] * $y[$i]
        $sumX2 += $x[$i] * $x[$i]
    }

    $slope = ($n * $sumXY - $sumX * $sumY) / ($n * $sumX2 - $sumX * $sumX)

    # 如果斜率大于阈值，可能存在内存泄漏
    return $slope -gt ($ThresholdMB / $n)
}

function Save-PerformanceMetrics {
    param([hashtable]$Metrics)

    $timestamp = $Metrics.Timestamp.ToString("yyyyMMdd-HHmmss")
    $metricsFile = Join-Path $DataDir "perf-$timestamp.json"

    $Metrics | ConvertTo-Json -Depth 10 | Set-Content $metricsFile

    $latestFile = Join-Path $DataDir "perf-latest.json"
    $Metrics | ConvertTo-Json -Depth 10 | Set-Content $latestFile
}

function Show-PerformanceDashboard {
    param([hashtable]$Metrics, [hashtable]$Trend)

    Clear-Host
    Write-Host "╔════════════════════════════════════════════════════════════╗" -ForegroundColor Cyan
    Write-Host "║         PERFORMANCE MONITORING DASHBOARD                  ║" -ForegroundColor Cyan
    Write-Host "╚════════════════════════════════════════════════════════════╝" -ForegroundColor Cyan
    Write-Host ""

    # 进程状态
    Write-Host "Process Status: $ProcessName" -ForegroundColor Yellow
    if ($Metrics.Process.Status -eq "Running") {
        Write-Host "  Status: Running" -ForegroundColor Green
        Write-Host "  PID: $($Metrics.Process.ProcessId)"
        Write-Host "  CPU: $($Metrics.Process.CPUUsage)%" -ForegroundColor $(if ($Metrics.Process.CPUUsage -gt 80) { "Red" } elseif ($Metrics.Process.CPUUsage -gt 60) { "Yellow" } else { "Green" })
        Write-Host "  Memory: $($Metrics.Process.MemoryMB) MB ($($Metrics.Process.MemoryPercent)%)" -ForegroundColor $(if ($Metrics.Process.MemoryPercent -gt 80) { "Red" } elseif ($Metrics.Process.MemoryPercent -gt 60) { "Yellow" } else { "Green" })
        Write-Host "  Threads: $($Metrics.Process.ThreadCount)"
        Write-Host "  Handles: $($Metrics.Process.HandleCount)"
        Write-Host "  Uptime: $($Metrics.Process.Uptime.ToString('hh\:mm\:ss'))"
    } else {
        Write-Host "  Status: Not Running" -ForegroundColor Red
    }
    Write-Host ""

    # 系统资源
    Write-Host "System Resources:" -ForegroundColor Yellow
    Write-Host "  CPU: $($Metrics.System.CPU.Usage)% ($($Metrics.System.CPU.Cores) cores)" -ForegroundColor $(if ($Metrics.System.CPU.Usage -gt 90) { "Red" } elseif ($Metrics.System.CPU.Usage -gt 70) { "Yellow" } else { "Green" })
    Write-Host "  Memory: $($Metrics.System.Memory.UsedGB)/$($Metrics.System.Memory.TotalGB) GB ($($Metrics.System.Memory.Usage)%)" -ForegroundColor $(if ($Metrics.System.Memory.Usage -gt 90) { "Red" } elseif ($Metrics.System.Memory.Usage -gt 70) { "Yellow" } else { "Green" })
    Write-Host "  Disk: $($Metrics.System.Disk.UsedGB)/$($Metrics.System.Disk.TotalGB) GB ($($Metrics.System.Disk.Usage)%)" -ForegroundColor $(if ($Metrics.System.Disk.Usage -gt 90) { "Red" } elseif ($Metrics.System.Disk.Usage -gt 80) { "Yellow" } else { "Green" })
    Write-Host "  Network: $($Metrics.System.Network.ThroughputMBps) MB/s"
    Write-Host ""

    # 性能趋势
    Write-Host "Performance Trend (Last Hour):" -ForegroundColor Yellow
    Write-Host "  Samples: $($Trend.Samples)"
    Write-Host "  Avg CPU: $($Trend.AvgCPU)% | Max: $($Trend.MaxCPU)%"
    Write-Host "  Avg Memory: $($Trend.AvgMemory) MB | Max: $($Trend.MaxMemory) MB"
    Write-Host "  Memory Growth: $($Trend.MemoryGrowth) MB" -ForegroundColor $(if ($Trend.MemoryGrowth -gt 100) { "Red" } elseif ($Trend.MemoryGrowth -gt 50) { "Yellow" } else { "Green" })
    Write-Host "  CPU Spikes (>80%): $($Trend.CPUSpikes)" -ForegroundColor $(if ($Trend.CPUSpikes -gt 10) { "Red" } elseif ($Trend.CPUSpikes -gt 5) { "Yellow" } else { "Green" })
    Write-Host ""

    Write-Host "Last updated: $(Get-Date -Format 'yyyy-MM-dd HH:mm:ss')" -ForegroundColor Gray
}

function Start-PerformanceMonitoring {
    Write-Host "Starting Performance Monitoring..." -ForegroundColor Green
    Write-Host "Process: $ProcessName"
    Write-Host "Check Interval: $CheckInterval seconds"
    Write-Host "Press Ctrl+C to stop"
    Write-Host ""

    while ($true) {
        try {
            $metrics = Get-PerformanceMetrics
            $trend = Get-PerformanceTrend -Minutes 60

            Save-PerformanceMetrics -Metrics $metrics
            Show-PerformanceDashboard -Metrics $metrics -Trend $trend

            # 清理旧数据
            $cutoff = (Get-Date).AddDays(-1)
            Get-ChildItem $DataDir -Filter "perf-*.json" |
                Where-Object { $_.LastWriteTime -lt $cutoff } |
                Remove-Item -Force

        } catch {
            Write-Host "Error: $_" -ForegroundColor Red
        }

        Start-Sleep -Seconds $CheckInterval
    }
}

Start-PerformanceMonitoring
