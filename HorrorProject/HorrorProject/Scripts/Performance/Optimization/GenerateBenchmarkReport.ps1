# GenerateBenchmarkReport.ps1
# Benchmark Report Generator
# Consolidates all benchmark results into comprehensive report

param(
    [string]$BenchmarkDir = "",
    [string]$OutputDir = ""
)

$ErrorActionPreference = "Stop"

. (Join-Path $PSScriptRoot "..\..\Validation\Common.ps1")
$ProjectRoot = Get-HorrorProjectRoot -StartPath $PSScriptRoot
if ([string]::IsNullOrWhiteSpace($BenchmarkDir)) { $BenchmarkDir = Join-Path $ProjectRoot "Saved\Benchmarks" }
if ([string]::IsNullOrWhiteSpace($OutputDir)) { $OutputDir = Join-Path $ProjectRoot "Saved\Reports" }

Write-Host "=== Benchmark Report Generator ===" -ForegroundColor Cyan
Write-Host ""

# Create output directory
New-Item -ItemType Directory -Force -Path $OutputDir | Out-Null

$timestamp = Get-Date -Format "yyyyMMdd_HHmmss"
$reportFile = Join-Path $OutputDir "benchmark_report_$timestamp.html"

Write-Host "Scanning for benchmark results..." -ForegroundColor Yellow

# Find latest benchmark files
$framerateFiles = Get-ChildItem -Path $BenchmarkDir -Filter "framerate_benchmark_*.json" -ErrorAction SilentlyContinue | Sort-Object LastWriteTime -Descending
$memoryFiles = Get-ChildItem -Path $BenchmarkDir -Filter "memory_benchmark_*.json" -ErrorAction SilentlyContinue | Sort-Object LastWriteTime -Descending
$loadingFiles = Get-ChildItem -Path $BenchmarkDir -Filter "loading_benchmark_*.json" -ErrorAction SilentlyContinue | Sort-Object LastWriteTime -Descending

$latestFramerate = if ($framerateFiles.Count -gt 0) { $framerateFiles[0] } else { $null }
$latestMemory = if ($memoryFiles.Count -gt 0) { $memoryFiles[0] } else { $null }
$latestLoading = if ($loadingFiles.Count -gt 0) { $loadingFiles[0] } else { $null }

Write-Host "Found benchmark files:" -ForegroundColor Green
if ($latestFramerate) { Write-Host "  Framerate: $($latestFramerate.Name)" }
if ($latestMemory) { Write-Host "  Memory: $($latestMemory.Name)" }
if ($latestLoading) { Write-Host "  Loading: $($latestLoading.Name)" }
Write-Host ""

# Load benchmark data
$framerateData = if ($latestFramerate) { Get-Content $latestFramerate.FullName | ConvertFrom-Json } else { $null }
$memoryData = if ($latestMemory) { Get-Content $latestMemory.FullName | ConvertFrom-Json } else { $null }
$loadingData = if ($latestLoading) { Get-Content $latestLoading.FullName | ConvertFrom-Json } else { $null }

Write-Host "Generating HTML report..." -ForegroundColor Yellow

# Generate HTML report
$html = @"
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>HorrorProject Performance Benchmark Report</title>
    <style>
        body {
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            margin: 0;
            padding: 20px;
            background: #1a1a1a;
            color: #e0e0e0;
        }
        .container {
            max-width: 1200px;
            margin: 0 auto;
            background: #2a2a2a;
            padding: 30px;
            border-radius: 10px;
            box-shadow: 0 4px 6px rgba(0,0,0,0.3);
        }
        h1 {
            color: #ff6b6b;
            border-bottom: 3px solid #ff6b6b;
            padding-bottom: 10px;
        }
        h2 {
            color: #4ecdc4;
            margin-top: 30px;
            border-left: 4px solid #4ecdc4;
            padding-left: 15px;
        }
        h3 {
            color: #95e1d3;
        }
        table {
            width: 100%;
            border-collapse: collapse;
            margin: 20px 0;
            background: #333;
        }
        th {
            background: #444;
            color: #4ecdc4;
            padding: 12px;
            text-align: left;
            font-weight: bold;
        }
        td {
            padding: 10px 12px;
            border-bottom: 1px solid #444;
        }
        tr:hover {
            background: #3a3a3a;
        }
        .metric {
            display: inline-block;
            margin: 10px 20px 10px 0;
            padding: 15px 25px;
            background: #333;
            border-radius: 8px;
            border-left: 4px solid #4ecdc4;
        }
        .metric-label {
            font-size: 0.9em;
            color: #999;
            display: block;
        }
        .metric-value {
            font-size: 1.8em;
            font-weight: bold;
            color: #4ecdc4;
        }
        .status-good { color: #51cf66; }
        .status-warn { color: #ffd43b; }
        .status-bad { color: #ff6b6b; }
        .section {
            margin: 30px 0;
            padding: 20px;
            background: #333;
            border-radius: 8px;
        }
        .timestamp {
            color: #999;
            font-size: 0.9em;
        }
        .recommendation {
            background: #3a3a3a;
            padding: 15px;
            margin: 10px 0;
            border-left: 4px solid #ffd43b;
            border-radius: 4px;
        }
        .chart-empty-state {
            background: #3a3a3a;
            padding: 40px;
            text-align: center;
            border-radius: 8px;
            margin: 20px 0;
            color: #666;
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>🎮 HorrorProject Performance Benchmark Report</h1>
        <p class="timestamp">Generated: $(Get-Date -Format "yyyy-MM-dd HH:mm:ss")</p>
"@

# Framerate section
if ($framerateData) {
    $html += @"
        <div class="section">
            <h2>📊 Framerate Performance</h2>
            <p class="timestamp">Benchmark Date: $($framerateData.Timestamp)</p>

            <h3>Quality Settings Comparison</h3>
            <table>
                <tr>
                    <th>Quality</th>
                    <th>Avg FPS</th>
                    <th>Min FPS</th>
                    <th>1% Low</th>
                    <th>Frame Time (ms)</th>
                    <th>Status</th>
                </tr>
"@

    foreach ($quality in @("Low", "Medium", "High", "Epic")) {
        if ($framerateData.Results.$quality) {
            $result = $framerateData.Results.$quality
            $avgFPS = [math]::Round($result.FPS.Average, 1)
            $minFPS = [math]::Round($result.FPS.Min, 1)
            $p1FPS = [math]::Round($result.FPS.P1, 1)
            $frameTime = [math]::Round($result.FrameTime.Average, 1)

            $status = if ($avgFPS -ge 60) { "✓ Excellent" }
                     elseif ($avgFPS -ge 45) { "~ Good" }
                     elseif ($avgFPS -ge 30) { "⚠ Acceptable" }
                     else { "✗ Poor" }

            $statusClass = if ($avgFPS -ge 60) { "status-good" }
                          elseif ($avgFPS -ge 30) { "status-warn" }
                          else { "status-bad" }

            $html += @"
                <tr>
                    <td><strong>$quality</strong></td>
                    <td>$avgFPS</td>
                    <td>$minFPS</td>
                    <td>$p1FPS</td>
                    <td>$frameTime</td>
                    <td class="$statusClass">$status</td>
                </tr>
"@
        }
    }

    $html += @"
            </table>

            <h3>Key Metrics (Epic Quality)</h3>
"@

    if ($framerateData.Results.Epic) {
        $epic = $framerateData.Results.Epic
        $html += @"
            <div class="metric">
                <span class="metric-label">Average FPS</span>
                <span class="metric-value">$([math]::Round($epic.FPS.Average, 1))</span>
            </div>
            <div class="metric">
                <span class="metric-label">1% Low FPS</span>
                <span class="metric-value">$([math]::Round($epic.FPS.P1, 1))</span>
            </div>
            <div class="metric">
                <span class="metric-label">Frame Time</span>
                <span class="metric-value">$([math]::Round($epic.FrameTime.Average, 1)) ms</span>
            </div>
            <div class="metric">
                <span class="metric-label">GPU Time</span>
                <span class="metric-value">$([math]::Round($epic.GPU.Average, 1)) ms</span>
            </div>
"@
    }

    $html += @"
        </div>
"@
}

# Memory section
if ($memoryData) {
    $html += @"
        <div class="section">
            <h2>💾 Memory Performance</h2>
            <p class="timestamp">Benchmark Date: $($memoryData.Timestamp)</p>

            <h3>Memory Usage by Scenario</h3>
            <table>
                <tr>
                    <th>Scenario</th>
                    <th>Avg (MB)</th>
                    <th>Max (MB)</th>
                    <th>Final (MB)</th>
                    <th>Status</th>
                </tr>
"@

    foreach ($scenarioName in $memoryData.Scenarios.PSObject.Properties.Name) {
        $scenario = $memoryData.Scenarios.$scenarioName
        $avg = [math]::Round($scenario.WorkingSet.Average, 0)
        $max = [math]::Round($scenario.WorkingSet.Max, 0)
        $final = [math]::Round($scenario.WorkingSet.Final, 0)

        $status = if ($max -lt 4000) { "✓ Good" }
                 elseif ($max -lt 6000) { "~ Acceptable" }
                 else { "✗ High" }

        $statusClass = if ($max -lt 4000) { "status-good" }
                      elseif ($max -lt 6000) { "status-warn" }
                      else { "status-bad" }

        $html += @"
                <tr>
                    <td><strong>$scenarioName</strong></td>
                    <td>$avg</td>
                    <td>$max</td>
                    <td>$final</td>
                    <td class="$statusClass">$status</td>
                </tr>
"@
    }

    $html += @"
            </table>
        </div>
"@
}

# Loading section
if ($loadingData) {
    $html += @"
        <div class="section">
            <h2>⏱️ Loading Performance</h2>
            <p class="timestamp">Benchmark Date: $($loadingData.Timestamp)</p>

            <h3>Level Loading Times</h3>
            <table>
                <tr>
                    <th>Map</th>
                    <th>Average (s)</th>
                    <th>Min (s)</th>
                    <th>Max (s)</th>
                    <th>Status</th>
                </tr>
"@

    foreach ($mapName in $loadingData.Maps.PSObject.Properties.Name) {
        $map = $loadingData.Maps.$mapName
        $avg = [math]::Round($map.Statistics.Average, 1)
        $min = [math]::Round($map.Statistics.Min, 1)
        $max = [math]::Round($map.Statistics.Max, 1)

        $status = if ($avg -lt 15) { "✓ Fast" }
                 elseif ($avg -lt 30) { "~ Acceptable" }
                 else { "✗ Slow" }

        $statusClass = if ($avg -lt 15) { "status-good" }
                      elseif ($avg -lt 30) { "status-warn" }
                      else { "status-bad" }

        $html += @"
                <tr>
                    <td><strong>$mapName</strong></td>
                    <td>$avg</td>
                    <td>$min</td>
                    <td>$max</td>
                    <td class="$statusClass">$status</td>
                </tr>
"@
    }

    $html += @"
            </table>
        </div>
"@
}

# Recommendations section
$html += @"
        <div class="section">
            <h2>💡 Optimization Recommendations</h2>
"@

$recommendations = @()

# Framerate recommendations
if ($framerateData -and $framerateData.Results.Epic) {
    $epicFPS = $framerateData.Results.Epic.FPS.Average
    if ($epicFPS -lt 60) {
        $recommendations += "Framerate below 60 FPS target at Epic quality. Consider texture and material optimizations."
    }
    if ($framerateData.Results.Epic.GPU.Average -gt 16) {
        $recommendations += "GPU time is high. Focus on rendering optimizations (shadows, post-processing, draw calls)."
    }
}

# Memory recommendations
if ($memoryData) {
    $maxMemory = ($memoryData.Scenarios.PSObject.Properties.Value | ForEach-Object { $_.WorkingSet.Max } | Measure-Object -Maximum).Maximum
    if ($maxMemory -gt 6000) {
        $recommendations += "Memory usage exceeds 6GB. Implement aggressive asset streaming and texture optimization."
    }
}

# Loading recommendations
if ($loadingData) {
    $avgLoading = ($loadingData.Maps.PSObject.Properties.Value | ForEach-Object { $_.Statistics.Average } | Measure-Object -Average).Average
    if ($avgLoading -gt 30) {
        $recommendations += "Loading times exceed 30 seconds. Implement level streaming and async loading."
    }
}

if ($recommendations.Count -eq 0) {
    $recommendations += "Performance is within acceptable ranges. Continue monitoring and incremental optimization."
}

foreach ($rec in $recommendations) {
    $html += @"
            <div class="recommendation">$rec</div>
"@
}

$html += @"
        </div>

        <div class="section">
            <h2>🎯 Performance Targets</h2>
            <table>
                <tr>
                    <th>Metric</th>
                    <th>Target</th>
                    <th>Current</th>
                    <th>Status</th>
                </tr>
                <tr>
                    <td>Framerate (Epic)</td>
                    <td>60 FPS</td>
                    <td>$( if ($framerateData -and $framerateData.Results.Epic) { [math]::Round($framerateData.Results.Epic.FPS.Average, 1) } else { "N/A" } ) FPS</td>
                    <td class="$( if ($framerateData -and $framerateData.Results.Epic -and $framerateData.Results.Epic.FPS.Average -ge 60) { "status-good" } elseif ($framerateData -and $framerateData.Results.Epic -and $framerateData.Results.Epic.FPS.Average -ge 30) { "status-warn" } else { "status-bad" } )">
                        $( if ($framerateData -and $framerateData.Results.Epic) { if ($framerateData.Results.Epic.FPS.Average -ge 60) { "✓ Met" } else { "✗ Not Met" } } else { "N/A" } )
                    </td>
                </tr>
                <tr>
                    <td>Memory Usage</td>
                    <td>&lt; 6 GB</td>
                    <td>$( if ($memoryData) { [math]::Round(($memoryData.Scenarios.PSObject.Properties.Value | ForEach-Object { $_.WorkingSet.Max } | Measure-Object -Maximum).Maximum / 1024, 1) } else { "N/A" } ) GB</td>
                    <td class="$( if ($memoryData) { $maxMem = ($memoryData.Scenarios.PSObject.Properties.Value | ForEach-Object { $_.WorkingSet.Max } | Measure-Object -Maximum).Maximum; if ($maxMem -lt 6000) { "status-good" } else { "status-bad" } } else { "" } )">
                        $( if ($memoryData) { $maxMem = ($memoryData.Scenarios.PSObject.Properties.Value | ForEach-Object { $_.WorkingSet.Max } | Measure-Object -Maximum).Maximum; if ($maxMem -lt 6000) { "✓ Met" } else { "✗ Not Met" } } else { "N/A" } )
                    </td>
                </tr>
                <tr>
                    <td>Loading Time</td>
                    <td>&lt; 30 seconds</td>
                    <td>$( if ($loadingData) { [math]::Round(($loadingData.Maps.PSObject.Properties.Value | ForEach-Object { $_.Statistics.Average } | Measure-Object -Average).Average, 1) } else { "N/A" } ) s</td>
                    <td class="$( if ($loadingData) { $avgLoad = ($loadingData.Maps.PSObject.Properties.Value | ForEach-Object { $_.Statistics.Average } | Measure-Object -Average).Average; if ($avgLoad -lt 30) { "status-good" } else { "status-bad" } } else { "" } )">
                        $( if ($loadingData) { $avgLoad = ($loadingData.Maps.PSObject.Properties.Value | ForEach-Object { $_.Statistics.Average } | Measure-Object -Average).Average; if ($avgLoad -lt 30) { "✓ Met" } else { "✗ Not Met" } } else { "N/A" } )
                    </td>
                </tr>
            </table>
        </div>
    </div>
</body>
</html>
"@

# Save HTML report
$html | Out-File $reportFile -Encoding UTF8

Write-Host ""
Write-Host "Report generated successfully!" -ForegroundColor Green
Write-Host "Location: $reportFile" -ForegroundColor Cyan
Write-Host ""

# Open report in browser
Start-Process $reportFile

Write-Host "Report opened in default browser" -ForegroundColor Green
