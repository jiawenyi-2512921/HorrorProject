# GenerateTrendReport.ps1 - 趋势报告生成
param(
    [ValidateSet("Daily", "Weekly", "Monthly")]
    [string]$ReportType = "Weekly",
    [string]$OutputPath = "trend-report.html"
)

$ErrorActionPreference = "Stop"
$MonitoringRoot = Split-Path -Parent $PSScriptRoot

function Get-ReportPeriod {
    param([string]$Type)

    $end = Get-Date
    $start = switch ($Type) {
        "Daily" { $end.AddDays(-1) }
        "Weekly" { $end.AddDays(-7) }
        "Monthly" { $end.AddDays(-30) }
    }

    return @{
        Start = $start
        End = $end
        Type = $Type
        Days = ($end - $start).Days
    }
}

function Get-TrendData {
    param([hashtable]$Period)

    # 运行各个趋势分析脚本
    $buildScript = Join-Path $PSScriptRoot "AnalyzeBuildTrends.ps1"
    $testScript = Join-Path $PSScriptRoot "AnalyzeTestTrends.ps1"
    $perfScript = Join-Path $PSScriptRoot "AnalyzePerformanceTrends.ps1"

    $data = @{
        Build = $null
        Test = $null
        Performance = $null
    }

    if (Test-Path $buildScript) {
        $buildJson = & $buildScript -Days $Period.Days
        $data.Build = $buildJson | ConvertFrom-Json
    }

    if (Test-Path $testScript) {
        $testJson = & $testScript -Days $Period.Days
        $data.Test = $testJson | ConvertFrom-Json
    }

    if (Test-Path $perfScript) {
        $perfJson = & $perfScript -Days $Period.Days
        $data.Performance = $perfJson | ConvertFrom-Json
    }

    return $data
}

function Generate-TrendReportHTML {
    param(
        [hashtable]$Period,
        [hashtable]$Data
    )

    $html = @"
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>$($Period.Type) Trend Report - HorrorProject</title>
    <style>
        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }

        body {
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            padding: 20px;
        }

        .container {
            max-width: 1400px;
            margin: 0 auto;
            background: white;
            border-radius: 15px;
            box-shadow: 0 10px 40px rgba(0,0,0,0.3);
            overflow: hidden;
        }

        header {
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            color: white;
            padding: 40px;
            text-align: center;
        }

        h1 {
            font-size: 2.5em;
            margin-bottom: 10px;
        }

        .period {
            font-size: 1.2em;
            opacity: 0.9;
        }

        .content {
            padding: 40px;
        }

        .section {
            margin-bottom: 50px;
        }

        .section-title {
            font-size: 2em;
            color: #667eea;
            margin-bottom: 20px;
            padding-bottom: 10px;
            border-bottom: 3px solid #667eea;
        }

        .metrics-grid {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(250px, 1fr));
            gap: 20px;
            margin-bottom: 30px;
        }

        .metric-card {
            background: linear-gradient(135deg, #f5f7fa 0%, #c3cfe2 100%);
            padding: 25px;
            border-radius: 10px;
            box-shadow: 0 4px 6px rgba(0,0,0,0.1);
            transition: transform 0.3s ease;
        }

        .metric-card:hover {
            transform: translateY(-5px);
        }

        .metric-label {
            font-size: 0.9em;
            color: #666;
            margin-bottom: 10px;
        }

        .metric-value {
            font-size: 2.5em;
            font-weight: bold;
            color: #333;
        }

        .metric-value.good {
            color: #28a745;
        }

        .metric-value.warning {
            color: #ffc107;
        }

        .metric-value.bad {
            color: #dc3545;
        }

        .trend-indicator {
            display: inline-block;
            padding: 5px 15px;
            border-radius: 20px;
            font-size: 0.9em;
            font-weight: bold;
            margin-top: 10px;
        }

        .trend-up {
            background: #dc3545;
            color: white;
        }

        .trend-down {
            background: #28a745;
            color: white;
        }

        .trend-stable {
            background: #6c757d;
            color: white;
        }

        .insights {
            background: #fff3cd;
            border-left: 4px solid #ffc107;
            padding: 20px;
            margin: 20px 0;
            border-radius: 5px;
        }

        .insights-title {
            font-weight: bold;
            color: #856404;
            margin-bottom: 10px;
        }

        .insights ul {
            list-style-position: inside;
            color: #856404;
        }

        .insights li {
            margin: 5px 0;
        }

        .chart-placeholder {
            width: 100%;
            height: 300px;
            background: linear-gradient(135deg, #f5f7fa 0%, #c3cfe2 100%);
            border-radius: 10px;
            display: flex;
            align-items: center;
            justify-content: center;
            color: #666;
            font-size: 1.2em;
            margin: 20px 0;
        }

        footer {
            background: #f8f9fa;
            padding: 30px;
            text-align: center;
            color: #666;
        }

        .summary-box {
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            color: white;
            padding: 30px;
            border-radius: 10px;
            margin-bottom: 30px;
        }

        .summary-box h2 {
            margin-bottom: 15px;
        }
    </style>
</head>
<body>
    <div class="container">
        <header>
            <h1>📈 $($Period.Type) Trend Report</h1>
            <div class="period">
                $($Period.Start.ToString('yyyy-MM-dd')) to $($Period.End.ToString('yyyy-MM-dd'))
            </div>
        </header>

        <div class="content">
            <!-- Executive Summary -->
            <div class="summary-box">
                <h2>Executive Summary</h2>
                <p>This report provides a comprehensive analysis of build, test, and performance trends over the past $($Period.Days) days.</p>
            </div>

            <!-- Build Trends -->
            <div class="section">
                <div class="section-title">🔨 Build Trends</div>
                <div class="metrics-grid">
                    <div class="metric-card">
                        <div class="metric-label">Total Builds</div>
                        <div class="metric-value">$($Data.Build.TotalBuilds)</div>
                    </div>
                    <div class="metric-card">
                        <div class="metric-label">Success Rate</div>
                        <div class="metric-value $(if ($Data.Build.SuccessRate -gt 90) { 'good' } elseif ($Data.Build.SuccessRate -gt 70) { 'warning' } else { 'bad' })">
                            $($Data.Build.SuccessRate)%
                        </div>
                    </div>
                    <div class="metric-card">
                        <div class="metric-label">Avg Duration</div>
                        <div class="metric-value">$($Data.Build.AverageDuration)s</div>
                        <span class="trend-indicator trend-$(if ($Data.Build.DurationTrend -eq 'Decreasing') { 'down' } elseif ($Data.Build.DurationTrend -eq 'Increasing') { 'up' } else { 'stable' })">
                            $($Data.Build.DurationTrend)
                        </span>
                    </div>
                    <div class="metric-card">
                        <div class="metric-label">Error Level</div>
                        <div class="metric-value $(if ($Data.Build.ErrorTrend -eq 'Low') { 'good' } elseif ($Data.Build.ErrorTrend -eq 'Moderate') { 'warning' } else { 'bad' })">
                            $($Data.Build.ErrorTrend)
                        </div>
                    </div>
                </div>

                <div class="insights">
                    <div class="insights-title">Key Insights:</div>
                    <ul>
                        <li>Build success rate is $($Data.Build.SuccessRate)%</li>
                        <li>Build duration trend: $($Data.Build.DurationTrend)</li>
                        <li>Error level: $($Data.Build.ErrorTrend)</li>
                    </ul>
                </div>
            </div>

            <!-- Test Trends -->
            <div class="section">
                <div class="section-title">🧪 Test Trends</div>
                <div class="metrics-grid">
                    <div class="metric-card">
                        <div class="metric-label">Total Test Runs</div>
                        <div class="metric-value">$($Data.Test.TotalRuns)</div>
                    </div>
                    <div class="metric-card">
                        <div class="metric-label">Pass Rate</div>
                        <div class="metric-value $(if ($Data.Test.PassRate -gt 95) { 'good' } elseif ($Data.Test.PassRate -gt 85) { 'warning' } else { 'bad' })">
                            $($Data.Test.PassRate)%
                        </div>
                    </div>
                    <div class="metric-card">
                        <div class="metric-label">Avg Duration</div>
                        <div class="metric-value">$($Data.Test.AverageDuration)s</div>
                    </div>
                    <div class="metric-card">
                        <div class="metric-label">Failure Rate Trend</div>
                        <div class="metric-value">
                            <span class="trend-indicator trend-$(if ($Data.Test.FailureRateTrend -eq 'Decreasing') { 'down' } elseif ($Data.Test.FailureRateTrend -eq 'Increasing') { 'up' } else { 'stable' })">
                                $($Data.Test.FailureRateTrend)
                            </span>
                        </div>
                    </div>
                </div>

                <div class="insights">
                    <div class="insights-title">Key Insights:</div>
                    <ul>
                        <li>Test pass rate: $($Data.Test.PassRate)%</li>
                        <li>Failure rate trend: $($Data.Test.FailureRateTrend)</li>
                        <li>Coverage trend: $($Data.Test.CoverageTrend)</li>
                        <li>Flaky tests detected: $($Data.Test.FlakyTests.Count)</li>
                    </ul>
                </div>
            </div>

            <!-- Performance Trends -->
            <div class="section">
                <div class="section-title">⚡ Performance Trends</div>
                <div class="metrics-grid">
                    <div class="metric-card">
                        <div class="metric-label">Avg CPU Usage</div>
                        <div class="metric-value $(if ($Data.Performance.CPU.Average -lt 50) { 'good' } elseif ($Data.Performance.CPU.Average -lt 70) { 'warning' } else { 'bad' })">
                            $($Data.Performance.CPU.Average)%
                        </div>
                        <span class="trend-indicator trend-$(if ($Data.Performance.CPU.Trend -eq 'Decreasing') { 'down' } elseif ($Data.Performance.CPU.Trend -eq 'Increasing') { 'up' } else { 'stable' })">
                            $($Data.Performance.CPU.Trend)
                        </span>
                    </div>
                    <div class="metric-card">
                        <div class="metric-label">Avg Memory</div>
                        <div class="metric-value">$($Data.Performance.Memory.Average) MB</div>
                        <span class="trend-indicator trend-$(if ($Data.Performance.Memory.Trend -eq 'Decreasing') { 'down' } elseif ($Data.Performance.Memory.Trend -eq 'Increasing') { 'up' } else { 'stable' })">
                            $($Data.Performance.Memory.Trend)
                        </span>
                    </div>
                    <div class="metric-card">
                        <div class="metric-label">Memory Growth</div>
                        <div class="metric-value $(if ($Data.Performance.Memory.Growth -lt 50) { 'good' } elseif ($Data.Performance.Memory.Growth -lt 100) { 'warning' } else { 'bad' })">
                            $($Data.Performance.Memory.Growth) MB
                        </div>
                    </div>
                    <div class="metric-card">
                        <div class="metric-label">CPU Spikes</div>
                        <div class="metric-value $(if ($Data.Performance.CPU.Spikes -lt 5) { 'good' } elseif ($Data.Performance.CPU.Spikes -lt 10) { 'warning' } else { 'bad' })">
                            $($Data.Performance.CPU.Spikes)
                        </div>
                    </div>
                </div>

                <div class="insights">
                    <div class="insights-title">Key Insights:</div>
                    <ul>
                        <li>CPU trend: $($Data.Performance.CPU.Trend)</li>
                        <li>Memory trend: $($Data.Performance.Memory.Trend)</li>
                        <li>Memory growth: $($Data.Performance.Memory.Growth) MB</li>
"@

    if ($Data.Performance.Memory.LeakDetected) {
        $html += @"
                        <li style="color: #dc3545; font-weight: bold;">⚠️ Potential memory leak detected!</li>
"@
    }

    $html += @"
                    </ul>
                </div>
            </div>
        </div>

        <footer>
            <p>Generated on $(Get-Date -Format 'yyyy-MM-dd HH:mm:ss')</p>
            <p>HorrorProject Monitoring System</p>
        </footer>
    </div>
</body>
</html>
"@

    return $html
}

# 主执行
Write-Host "Generating $ReportType trend report..." -ForegroundColor Cyan
Write-Host ""

$period = Get-ReportPeriod -Type $ReportType
Write-Host "Collecting trend data..." -ForegroundColor Yellow
$data = Get-TrendData -Period $period

Write-Host "Generating HTML report..." -ForegroundColor Yellow
$html = Generate-TrendReportHTML -Period $period -Data $data

$outputFile = Join-Path $PSScriptRoot $OutputPath
$html | Set-Content $outputFile -Encoding UTF8

Write-Host ""
Write-Host "Trend report generated successfully!" -ForegroundColor Green
Write-Host "Output: $outputFile" -ForegroundColor Cyan
Write-Host "Period: $($period.Start.ToString('yyyy-MM-dd')) to $($period.End.ToString('yyyy-MM-dd'))" -ForegroundColor Gray
