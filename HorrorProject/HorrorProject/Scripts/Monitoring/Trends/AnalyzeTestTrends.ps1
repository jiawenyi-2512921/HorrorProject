# AnalyzeTestTrends.ps1 - 测试趋势分析
param(
    [int]$Days = 30
)

$ErrorActionPreference = "Stop"
$MonitoringRoot = Split-Path -Parent $PSScriptRoot
$DataDir = Join-Path $MonitoringRoot "Data\Test"

function Get-TestHistory {
    param([int]$Days)

    $cutoff = (Get-Date).AddDays(-$Days)
    $testFiles = Get-ChildItem $DataDir -Filter "test-status-*.json" |
        Where-Object { $_.LastWriteTime -gt $cutoff } |
        Sort-Object LastWriteTime

    $history = @()
    foreach ($file in $testFiles) {
        $data = Get-Content $file.FullName | ConvertFrom-Json
        $history += $data
    }

    return $history
}

function Analyze-TestTrends {
    param([array]$History)

    $trends = @{
        Period = "$Days days"
        TotalRuns = $History.Count
        TotalTests = 0
        PassRate = 0
        AverageDuration = 0
        FailureRateTrend = ""
        CoverageTrend = ""
        MostFailedTests = @{}
        FlakyTests = @{}
        DailyStats = @{}
    }

    if ($History.Count -eq 0) {
        return $trends
    }

    # 总测试数和通过率
    $totalTests = ($History | ForEach-Object { $_.Total } | Measure-Object -Sum).Sum
    $totalPassed = ($History | ForEach-Object { $_.Passed } | Measure-Object -Sum).Sum
    $trends.TotalTests = $totalTests
    $trends.PassRate = if ($totalTests -gt 0) { [math]::Round($totalPassed / $totalTests * 100, 2) } else { 0 }

    # 平均测试时间
    $durations = $History | ForEach-Object { $_.Duration }
    $trends.AverageDuration = [math]::Round(($durations | Measure-Object -Average).Average, 2)

    # 失败率趋势
    $failureRates = $History | ForEach-Object { $_.FailureRate }
    if ($History.Count -gt 10) {
        $firstHalf = $failureRates[0..([math]::Floor($failureRates.Count / 2) - 1)]
        $secondHalf = $failureRates[([math]::Floor($failureRates.Count / 2))..($failureRates.Count - 1)]
        $avgFirst = ($firstHalf | Measure-Object -Average).Average
        $avgSecond = ($secondHalf | Measure-Object -Average).Average

        if ($avgSecond -gt $avgFirst * 1.2) {
            $trends.FailureRateTrend = "Increasing"
        } elseif ($avgSecond -lt $avgFirst * 0.8) {
            $trends.FailureRateTrend = "Decreasing"
        } else {
            $trends.FailureRateTrend = "Stable"
        }
    }

    # 覆盖率趋势
    $coverages = $History | ForEach-Object { $_.Coverage.Line }
    if ($History.Count -gt 10) {
        $firstHalf = $coverages[0..([math]::Floor($coverages.Count / 2) - 1)]
        $secondHalf = $coverages[([math]::Floor($coverages.Count / 2))..($coverages.Count - 1)]
        $avgFirst = ($firstHalf | Measure-Object -Average).Average
        $avgSecond = ($secondHalf | Measure-Object -Average).Average

        if ($avgSecond -gt $avgFirst * 1.05) {
            $trends.CoverageTrend = "Improving"
        } elseif ($avgSecond -lt $avgFirst * 0.95) {
            $trends.CoverageTrend = "Declining"
        } else {
            $trends.CoverageTrend = "Stable"
        }
    }

    # 统计最常失败的测试
    foreach ($run in $History) {
        foreach ($test in $run.FailedTests) {
            if (-not $trends.MostFailedTests.ContainsKey($test)) {
                $trends.MostFailedTests[$test] = 0
            }
            $trends.MostFailedTests[$test]++
        }
    }

    # 检测不稳定的测试（flaky tests）
    $testResults = @{}
    foreach ($run in $History) {
        foreach ($test in $run.FailedTests) {
            if (-not $testResults.ContainsKey($test)) {
                $testResults[$test] = @{ Passed = 0; Failed = 0 }
            }
            $testResults[$test].Failed++
        }
        foreach ($test in $run.PassedTests) {
            if (-not $testResults.ContainsKey($test)) {
                $testResults[$test] = @{ Passed = 0; Failed = 0 }
            }
            $testResults[$test].Passed++
        }
    }

    foreach ($test in $testResults.Keys) {
        $results = $testResults[$test]
        $total = $results.Passed + $results.Failed
        $failureRate = $results.Failed / $total

        # 如果失败率在10%-90%之间，认为是不稳定的测试
        if ($failureRate -gt 0.1 -and $failureRate -lt 0.9) {
            $trends.FlakyTests[$test] = @{
                Passed = $results.Passed
                Failed = $results.Failed
                FailureRate = [math]::Round($failureRate * 100, 2)
            }
        }
    }

    # 每日统计
    $History | Group-Object { ([datetime]$_.Timestamp).Date } | ForEach-Object {
        $date = $_.Name
        $runs = $_.Group
        $trends.DailyStats[$date] = @{
            Runs = $runs.Count
            TotalTests = ($runs | ForEach-Object { $_.Total } | Measure-Object -Sum).Sum
            Passed = ($runs | ForEach-Object { $_.Passed } | Measure-Object -Sum).Sum
            Failed = ($runs | ForEach-Object { $_.Failed } | Measure-Object -Sum).Sum
            AvgDuration = [math]::Round(($runs | ForEach-Object { $_.Duration } | Measure-Object -Average).Average, 2)
        }
    }

    return $trends
}

function Show-TrendReport {
    param([hashtable]$Trends)

    Write-Host "╔════════════════════════════════════════════════════════════╗" -ForegroundColor Cyan
    Write-Host "║              TEST TREND ANALYSIS REPORT                   ║" -ForegroundColor Cyan
    Write-Host "╚════════════════════════════════════════════════════════════╝" -ForegroundColor Cyan
    Write-Host ""

    Write-Host "Period: $($Trends.Period)" -ForegroundColor Yellow
    Write-Host "Total Test Runs: $($Trends.TotalRuns)"
    Write-Host "Total Tests: $($Trends.TotalTests)"
    Write-Host "Pass Rate: $($Trends.PassRate)%" -ForegroundColor $(if ($Trends.PassRate -gt 95) { "Green" } elseif ($Trends.PassRate -gt 85) { "Yellow" } else { "Red" })
    Write-Host "Average Duration: $($Trends.AverageDuration)s"
    Write-Host ""

    Write-Host "Trends:" -ForegroundColor Yellow
    Write-Host "  Failure Rate: $($Trends.FailureRateTrend)" -ForegroundColor $(if ($Trends.FailureRateTrend -eq "Decreasing") { "Green" } elseif ($Trends.FailureRateTrend -eq "Increasing") { "Red" } else { "Gray" })
    Write-Host "  Coverage: $($Trends.CoverageTrend)" -ForegroundColor $(if ($Trends.CoverageTrend -eq "Improving") { "Green" } elseif ($Trends.CoverageTrend -eq "Declining") { "Red" } else { "Gray" })
    Write-Host ""

    if ($Trends.MostFailedTests.Count -gt 0) {
        Write-Host "Most Failed Tests:" -ForegroundColor Red
        $Trends.MostFailedTests.GetEnumerator() | Sort-Object Value -Descending | Select-Object -First 10 | ForEach-Object {
            Write-Host "  - $($_.Key): $($_.Value) failures" -ForegroundColor Red
        }
        Write-Host ""
    }

    if ($Trends.FlakyTests.Count -gt 0) {
        Write-Host "Flaky Tests (Unstable):" -ForegroundColor Yellow
        $Trends.FlakyTests.GetEnumerator() | Sort-Object { $_.Value.FailureRate } -Descending | Select-Object -First 10 | ForEach-Object {
            Write-Host "  - $($_.Key): $($_.Value.FailureRate)% failure rate ($($_.Value.Failed)/$($_.Value.Passed + $_.Value.Failed))" -ForegroundColor Yellow
        }
        Write-Host ""
    }
}

# 主执行
Write-Host "Analyzing test trends for the last $Days days..." -ForegroundColor Cyan
Write-Host ""

$history = Get-TestHistory -Days $Days
$trends = Analyze-TestTrends -History $history

Show-TrendReport -Trends $trends

# 保存分析结果
$outputFile = Join-Path $PSScriptRoot "test-trends-$(Get-Date -Format 'yyyyMMdd').json"
$trends | ConvertTo-Json -Depth 10 | Set-Content $outputFile

Write-Host "Analysis saved: $outputFile" -ForegroundColor Green
