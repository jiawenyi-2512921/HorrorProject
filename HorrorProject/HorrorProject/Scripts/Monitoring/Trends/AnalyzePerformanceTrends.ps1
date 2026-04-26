# AnalyzePerformanceTrends.ps1 - 性能趋势分析
param(
    [int]$Days = 7
)

$ErrorActionPreference = "Stop"
$MonitoringRoot = Split-Path -Parent $PSScriptRoot
$DataDir = Join-Path $MonitoringRoot "Data\Performance"

function Get-PerformanceHistory {
    param([int]$Days)

    $cutoff = (Get-Date).AddDays(-$Days)
    $perfFiles = Get-ChildItem $DataDir -Filter "perf-*.json" |
        Where-Object { $_.LastWriteTime -gt $cutoff } |
        Sort-Object LastWriteTime

    $history = @()
    foreach ($file in $perfFiles) {
        $data = Get-Content $file.FullName | ConvertFrom-Json
        $history += $data
    }

    return $history
}

function Analyze-PerformanceTrends {
    param([array]$History)

    $trends = @{
        Period = "$Days days"
        TotalSamples = $History.Count
        CPU = @{
            Average = 0
            Max = 0
            Min = 0
            Trend = ""
            Spikes = 0
        }
        Memory = @{
            Average = 0
            Max = 0
            Min = 0
            Trend = ""
            Growth = 0
            LeakDetected = $false
        }
        System = @{
            AvgCPU = 0
            AvgMemory = 0
            AvgDisk = 0
        }
        DailyStats = @{}
    }

    if ($History.Count -eq 0) {
        return $trends
    }

    # CPU统计
    $cpuValues = $History | Where-Object { $_.Process.Status -eq "Running" } | ForEach-Object { $_.Process.CPUUsage }
    if ($cpuValues.Count -gt 0) {
        $trends.CPU.Average = [math]::Round(($cpuValues | Measure-Object -Average).Average, 2)
        $trends.CPU.Max = [math]::Round(($cpuValues | Measure-Object -Maximum).Maximum, 2)
        $trends.CPU.Min = [math]::Round(($cpuValues | Measure-Object -Minimum).Minimum, 2)
        $trends.CPU.Spikes = @($cpuValues | Where-Object { $_ -gt 80 }).Count

        # CPU趋势
        if ($cpuValues.Count -gt 10) {
            $firstHalf = $cpuValues[0..([math]::Floor($cpuValues.Count / 2) - 1)]
            $secondHalf = $cpuValues[([math]::Floor($cpuValues.Count / 2))..($cpuValues.Count - 1)]
            $avgFirst = ($firstHalf | Measure-Object -Average).Average
            $avgSecond = ($secondHalf | Measure-Object -Average).Average

            if ($avgSecond -gt $avgFirst * 1.2) {
                $trends.CPU.Trend = "Increasing"
            } elseif ($avgSecond -lt $avgFirst * 0.8) {
                $trends.CPU.Trend = "Decreasing"
            } else {
                $trends.CPU.Trend = "Stable"
            }
        }
    }

    # 内存统计
    $memoryValues = $History | Where-Object { $_.Process.Status -eq "Running" } | ForEach-Object { $_.Process.MemoryMB }
    if ($memoryValues.Count -gt 0) {
        $trends.Memory.Average = [math]::Round(($memoryValues | Measure-Object -Average).Average, 2)
        $trends.Memory.Max = [math]::Round(($memoryValues | Measure-Object -Maximum).Maximum, 2)
        $trends.Memory.Min = [math]::Round(($memoryValues | Measure-Object -Minimum).Minimum, 2)

        # 内存增长
        if ($memoryValues.Count -gt 10) {
            $firstHalf = $memoryValues[0..([math]::Floor($memoryValues.Count / 2) - 1)]
            $secondHalf = $memoryValues[([math]::Floor($memoryValues.Count / 2))..($memoryValues.Count - 1)]
            $avgFirst = ($firstHalf | Measure-Object -Average).Average
            $avgSecond = ($secondHalf | Measure-Object -Average).Average
            $trends.Memory.Growth = [math]::Round($avgSecond - $avgFirst, 2)

            # 内存泄漏检测
            if ($trends.Memory.Growth -gt 100) {
                $trends.Memory.LeakDetected = $true
            }

            # 内存趋势
            if ($avgSecond -gt $avgFirst * 1.1) {
                $trends.Memory.Trend = "Increasing"
            } elseif ($avgSecond -lt $avgFirst * 0.9) {
                $trends.Memory.Trend = "Decreasing"
            } else {
                $trends.Memory.Trend = "Stable"
            }
        }
    }

    # 系统资源统计
    $systemCPU = $History | ForEach-Object { $_.System.CPU.Usage }
    $systemMemory = $History | ForEach-Object { $_.System.Memory.Usage }
    $systemDisk = $History | ForEach-Object { $_.System.Disk.Usage }

    $trends.System.AvgCPU = [math]::Round(($systemCPU | Measure-Object -Average).Average, 2)
    $trends.System.AvgMemory = [math]::Round(($systemMemory | Measure-Object -Average).Average, 2)
    $trends.System.AvgDisk = [math]::Round(($systemDisk | Measure-Object -Average).Average, 2)

    # 每日统计
    $History | Group-Object { ([datetime]$_.Timestamp).Date } | ForEach-Object {
        $date = $_.Name
        $samples = $_.Group

        $dayCPU = $samples | Where-Object { $_.Process.Status -eq "Running" } | ForEach-Object { $_.Process.CPUUsage }
        $dayMemory = $samples | Where-Object { $_.Process.Status -eq "Running" } | ForEach-Object { $_.Process.MemoryMB }

        $trends.DailyStats[$date] = @{
            Samples = $samples.Count
            AvgCPU = if ($dayCPU.Count -gt 0) { [math]::Round(($dayCPU | Measure-Object -Average).Average, 2) } else { 0 }
            MaxCPU = if ($dayCPU.Count -gt 0) { [math]::Round(($dayCPU | Measure-Object -Maximum).Maximum, 2) } else { 0 }
            AvgMemory = if ($dayMemory.Count -gt 0) { [math]::Round(($dayMemory | Measure-Object -Average).Average, 2) } else { 0 }
            MaxMemory = if ($dayMemory.Count -gt 0) { [math]::Round(($dayMemory | Measure-Object -Maximum).Maximum, 2) } else { 0 }
        }
    }

    return $trends
}

function Show-TrendReport {
    param([hashtable]$Trends)

    Write-Host "╔════════════════════════════════════════════════════════════╗" -ForegroundColor Cyan
    Write-Host "║           PERFORMANCE TREND ANALYSIS REPORT               ║" -ForegroundColor Cyan
    Write-Host "╚════════════════════════════════════════════════════════════╝" -ForegroundColor Cyan
    Write-Host ""

    Write-Host "Period: $($Trends.Period)" -ForegroundColor Yellow
    Write-Host "Total Samples: $($Trends.TotalSamples)"
    Write-Host ""

    Write-Host "CPU Performance:" -ForegroundColor Yellow
    Write-Host "  Average: $($Trends.CPU.Average)%"
    Write-Host "  Max: $($Trends.CPU.Max)%"
    Write-Host "  Min: $($Trends.CPU.Min)%"
    Write-Host "  Trend: $($Trends.CPU.Trend)" -ForegroundColor $(if ($Trends.CPU.Trend -eq "Decreasing") { "Green" } elseif ($Trends.CPU.Trend -eq "Increasing") { "Red" } else { "Gray" })
    Write-Host "  Spikes (>80%): $($Trends.CPU.Spikes)" -ForegroundColor $(if ($Trends.CPU.Spikes -gt 10) { "Red" } elseif ($Trends.CPU.Spikes -gt 5) { "Yellow" } else { "Green" })
    Write-Host ""

    Write-Host "Memory Performance:" -ForegroundColor Yellow
    Write-Host "  Average: $($Trends.Memory.Average) MB"
    Write-Host "  Max: $($Trends.Memory.Max) MB"
    Write-Host "  Min: $($Trends.Memory.Min) MB"
    Write-Host "  Trend: $($Trends.Memory.Trend)" -ForegroundColor $(if ($Trends.Memory.Trend -eq "Decreasing") { "Green" } elseif ($Trends.Memory.Trend -eq "Increasing") { "Red" } else { "Gray" })
    Write-Host "  Growth: $($Trends.Memory.Growth) MB" -ForegroundColor $(if ($Trends.Memory.Growth -gt 100) { "Red" } elseif ($Trends.Memory.Growth -gt 50) { "Yellow" } else { "Green" })

    if ($Trends.Memory.LeakDetected) {
        Write-Host "  WARNING: Potential memory leak detected!" -ForegroundColor Red
    }
    Write-Host ""

    Write-Host "System Resources:" -ForegroundColor Yellow
    Write-Host "  Average CPU: $($Trends.System.AvgCPU)%"
    Write-Host "  Average Memory: $($Trends.System.AvgMemory)%"
    Write-Host "  Average Disk: $($Trends.System.AvgDisk)%"
    Write-Host ""

    if ($Trends.DailyStats.Count -gt 0) {
        Write-Host "Daily Statistics:" -ForegroundColor Yellow
        $Trends.DailyStats.GetEnumerator() | Sort-Object Name | ForEach-Object {
            $date = ([datetime]$_.Name).ToString('yyyy-MM-dd')
            $stats = $_.Value
            Write-Host "  $date`: CPU $($stats.AvgCPU)% (max $($stats.MaxCPU)%), Memory $($stats.AvgMemory) MB (max $($stats.MaxMemory) MB)"
        }
        Write-Host ""
    }
}

# 主执行
Write-Host "Analyzing performance trends for the last $Days days..." -ForegroundColor Cyan
Write-Host ""

$history = Get-PerformanceHistory -Days $Days
$trends = Analyze-PerformanceTrends -History $history

Show-TrendReport -Trends $trends

# 保存分析结果
$outputFile = Join-Path $PSScriptRoot "performance-trends-$(Get-Date -Format 'yyyyMMdd').json"
$trends | ConvertTo-Json -Depth 10 | Set-Content $outputFile

Write-Host "Analysis saved: $outputFile" -ForegroundColor Green
