# AnalyzeBuildTrends.ps1 - 构建趋势分析
param(
    [int]$Days = 30
)

$ErrorActionPreference = "Stop"
$MonitoringRoot = Split-Path -Parent $PSScriptRoot
$DataDir = Join-Path $MonitoringRoot "Data\Build"

function Get-BuildHistory {
    param([int]$Days)

    $cutoff = (Get-Date).AddDays(-$Days)
    $buildFiles = Get-ChildItem $DataDir -Filter "build-status-*.json" |
        Where-Object { $_.LastWriteTime -gt $cutoff } |
        Sort-Object LastWriteTime

    $history = @()
    foreach ($file in $buildFiles) {
        $data = Get-Content $file.FullName | ConvertFrom-Json
        $history += $data
    }

    return $history
}

function Analyze-BuildTrends {
    param([array]$History)

    $trends = @{
        Period = "$Days days"
        TotalBuilds = $History.Count
        SuccessRate = 0
        AverageDuration = 0
        DurationTrend = ""
        ErrorTrend = ""
        WarningTrend = ""
        ByConfiguration = @{}
        ByPlatform = @{}
        DailyStats = @{}
    }

    if ($History.Count -eq 0) {
        return $trends
    }

    # 成功率
    $successful = @($History | Where-Object { $_.Status -eq "Success" }).Count
    $trends.SuccessRate = [math]::Round($successful / $History.Count * 100, 2)

    # 平均构建时间
    $durations = $History | ForEach-Object { $_.Duration }
    $trends.AverageDuration = [math]::Round(($durations | Measure-Object -Average).Average, 2)

    # 构建时间趋势
    if ($History.Count -gt 10) {
        $firstHalf = $durations[0..([math]::Floor($durations.Count / 2) - 1)]
        $secondHalf = $durations[([math]::Floor($durations.Count / 2))..($durations.Count - 1)]
        $avgFirst = ($firstHalf | Measure-Object -Average).Average
        $avgSecond = ($secondHalf | Measure-Object -Average).Average

        if ($avgSecond -gt $avgFirst * 1.1) {
            $trends.DurationTrend = "Increasing"
        } elseif ($avgSecond -lt $avgFirst * 0.9) {
            $trends.DurationTrend = "Decreasing"
        } else {
            $trends.DurationTrend = "Stable"
        }
    }

    # 错误趋势
    $errors = $History | ForEach-Object { $_.Errors }
    $avgErrors = ($errors | Measure-Object -Average).Average
    if ($avgErrors -gt 5) {
        $trends.ErrorTrend = "High"
    } elseif ($avgErrors -gt 0) {
        $trends.ErrorTrend = "Moderate"
    } else {
        $trends.ErrorTrend = "Low"
    }

    # 警告趋势
    $warnings = $History | ForEach-Object { $_.Warnings }
    $avgWarnings = ($warnings | Measure-Object -Average).Average
    if ($avgWarnings -gt 20) {
        $trends.WarningTrend = "High"
    } elseif ($avgWarnings -gt 5) {
        $trends.WarningTrend = "Moderate"
    } else {
        $trends.WarningTrend = "Low"
    }

    # 按配置统计
    $History | Group-Object Configuration | ForEach-Object {
        $config = $_.Name
        $builds = $_.Group
        $trends.ByConfiguration[$config] = @{
            Count = $builds.Count
            SuccessRate = [math]::Round((@($builds | Where-Object { $_.Status -eq "Success" }).Count / $builds.Count * 100), 2)
            AvgDuration = [math]::Round(($builds | ForEach-Object { $_.Duration } | Measure-Object -Average).Average, 2)
        }
    }

    # 按平台统计
    $History | Group-Object Platform | ForEach-Object {
        $platform = $_.Name
        $builds = $_.Group
        $trends.ByPlatform[$platform] = @{
            Count = $builds.Count
            SuccessRate = [math]::Round((@($builds | Where-Object { $_.Status -eq "Success" }).Count / $builds.Count * 100), 2)
            AvgDuration = [math]::Round(($builds | ForEach-Object { $_.Duration } | Measure-Object -Average).Average, 2)
        }
    }

    # 每日统计
    $History | Group-Object { ([datetime]$_.Timestamp).Date } | ForEach-Object {
        $date = $_.Name
        $builds = $_.Group
        $trends.DailyStats[$date] = @{
            Count = $builds.Count
            Successful = @($builds | Where-Object { $_.Status -eq "Success" }).Count
            Failed = @($builds | Where-Object { $_.Status -eq "Failed" }).Count
            AvgDuration = [math]::Round(($builds | ForEach-Object { $_.Duration } | Measure-Object -Average).Average, 2)
        }
    }

    return $trends
}

function Show-TrendReport {
    param([hashtable]$Trends)

    Write-Host "╔════════════════════════════════════════════════════════════╗" -ForegroundColor Cyan
    Write-Host "║              BUILD TREND ANALYSIS REPORT                  ║" -ForegroundColor Cyan
    Write-Host "╚════════════════════════════════════════════════════════════╝" -ForegroundColor Cyan
    Write-Host ""

    Write-Host "Period: $($Trends.Period)" -ForegroundColor Yellow
    Write-Host "Total Builds: $($Trends.TotalBuilds)"
    Write-Host "Success Rate: $($Trends.SuccessRate)%" -ForegroundColor $(if ($Trends.SuccessRate -gt 90) { "Green" } elseif ($Trends.SuccessRate -gt 70) { "Yellow" } else { "Red" })
    Write-Host "Average Duration: $($Trends.AverageDuration)s"
    Write-Host ""

    Write-Host "Trends:" -ForegroundColor Yellow
    Write-Host "  Duration: $($Trends.DurationTrend)" -ForegroundColor $(if ($Trends.DurationTrend -eq "Decreasing") { "Green" } elseif ($Trends.DurationTrend -eq "Increasing") { "Red" } else { "Gray" })
    Write-Host "  Errors: $($Trends.ErrorTrend)" -ForegroundColor $(if ($Trends.ErrorTrend -eq "Low") { "Green" } elseif ($Trends.ErrorTrend -eq "Moderate") { "Yellow" } else { "Red" })
    Write-Host "  Warnings: $($Trends.WarningTrend)" -ForegroundColor $(if ($Trends.WarningTrend -eq "Low") { "Green" } elseif ($Trends.WarningTrend -eq "Moderate") { "Yellow" } else { "Red" })
    Write-Host ""

    if ($Trends.ByConfiguration.Count -gt 0) {
        Write-Host "By Configuration:" -ForegroundColor Yellow
        foreach ($config in $Trends.ByConfiguration.Keys) {
            $stats = $Trends.ByConfiguration[$config]
            Write-Host "  $config`: $($stats.Count) builds, $($stats.SuccessRate)% success, $($stats.AvgDuration)s avg"
        }
        Write-Host ""
    }

    if ($Trends.ByPlatform.Count -gt 0) {
        Write-Host "By Platform:" -ForegroundColor Yellow
        foreach ($platform in $Trends.ByPlatform.Keys) {
            $stats = $Trends.ByPlatform[$platform]
            Write-Host "  $platform`: $($stats.Count) builds, $($stats.SuccessRate)% success, $($stats.AvgDuration)s avg"
        }
        Write-Host ""
    }
}

# 主执行
Write-Host "Analyzing build trends for the last $Days days..." -ForegroundColor Cyan
Write-Host ""

$history = Get-BuildHistory -Days $Days
$trends = Analyze-BuildTrends -History $history

Show-TrendReport -Trends $trends

# 保存分析结果
$outputFile = Join-Path $PSScriptRoot "build-trends-$(Get-Date -Format 'yyyyMMdd').json"
$trends | ConvertTo-Json -Depth 10 | Set-Content $outputFile

Write-Host "Analysis saved: $outputFile" -ForegroundColor Green
