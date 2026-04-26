# Report Generator - Generates comprehensive analytics reports
# Copyright Epic Games, Inc. All Rights Reserved.

param(
    [string]$AnalysisPath = "D:\gptzuo\HorrorProject\HorrorProject\Saved\Analytics\Analysis",
    [string]$OutputPath = "D:\gptzuo\HorrorProject\HorrorProject\Saved\Analytics\Reports",
    [string]$Format = "HTML",  # HTML, Markdown, JSON
    [switch]$IncludeCharts
)

$ErrorActionPreference = "Stop"

function Write-Log {
    param([string]$Message)
    $timestamp = Get-Date -Format "yyyy-MM-dd HH:mm:ss"
    Write-Host "[$timestamp] $Message"
}

function Generate-HTMLReport {
    param($AnalysisData, $OutputFile)

    Write-Log "Generating HTML report..."

    $html = @"
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Analytics Report - Horror Project</title>
    <style>
        body {
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            margin: 0;
            padding: 20px;
            background-color: #1a1a1a;
            color: #e0e0e0;
        }
        .container {
            max-width: 1200px;
            margin: 0 auto;
            background-color: #2a2a2a;
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
        }
        .metric-grid {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(250px, 1fr));
            gap: 20px;
            margin: 20px 0;
        }
        .metric-card {
            background-color: #333;
            padding: 20px;
            border-radius: 8px;
            border-left: 4px solid #4ecdc4;
        }
        .metric-value {
            font-size: 2em;
            font-weight: bold;
            color: #4ecdc4;
        }
        .metric-label {
            color: #999;
            margin-top: 5px;
        }
        .insight {
            background-color: #2d3748;
            padding: 15px;
            margin: 10px 0;
            border-left: 4px solid #ffd93d;
            border-radius: 4px;
        }
        .rating {
            display: inline-block;
            padding: 5px 15px;
            border-radius: 20px;
            font-weight: bold;
        }
        .rating-excellent { background-color: #48bb78; color: white; }
        .rating-good { background-color: #4299e1; color: white; }
        .rating-fair { background-color: #ed8936; color: white; }
        .rating-poor { background-color: #f56565; color: white; }
        table {
            width: 100%;
            border-collapse: collapse;
            margin: 20px 0;
        }
        th, td {
            padding: 12px;
            text-align: left;
            border-bottom: 1px solid #444;
        }
        th {
            background-color: #333;
            color: #4ecdc4;
        }
        .footer {
            margin-top: 40px;
            padding-top: 20px;
            border-top: 1px solid #444;
            text-align: center;
            color: #666;
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>Horror Project - Analytics Report</h1>
        <p>Generated: $($AnalysisData.GeneratedAt)</p>

        <h2>Performance Overview</h2>
        <div class="metric-grid">
            <div class="metric-card">
                <div class="metric-value">$($AnalysisData.Performance.PerformanceScore)</div>
                <div class="metric-label">Performance Score</div>
            </div>
            <div class="metric-card">
                <div class="metric-value">$($AnalysisData.Performance.FPSAnalysis.Average)</div>
                <div class="metric-label">Average FPS</div>
            </div>
            <div class="metric-card">
                <div class="metric-value">$($AnalysisData.Performance.MemoryAnalysis.AverageUsageMB) MB</div>
                <div class="metric-label">Avg Memory Usage</div>
            </div>
        </div>

        <p>FPS Rating: <span class="rating rating-$(($AnalysisData.Performance.FPSAnalysis.Rating).ToLower())">$($AnalysisData.Performance.FPSAnalysis.Rating)</span></p>
        <p>Memory Rating: <span class="rating rating-$(($AnalysisData.Performance.MemoryAnalysis.Rating).ToLower())">$($AnalysisData.Performance.MemoryAnalysis.Rating)</span></p>

        <h2>Session Behavior</h2>
        <div class="metric-grid">
            <div class="metric-card">
                <div class="metric-value">$($AnalysisData.SessionBehavior.TotalSessions)</div>
                <div class="metric-label">Total Sessions</div>
            </div>
            <div class="metric-card">
                <div class="metric-value">$($AnalysisData.SessionBehavior.AverageSessionDuration)s</div>
                <div class="metric-label">Avg Session Duration</div>
            </div>
            <div class="metric-card">
                <div class="metric-value">$($AnalysisData.SessionBehavior.TotalPlaytime)h</div>
                <div class="metric-label">Total Playtime</div>
            </div>
        </div>

        <h2>Gameplay Metrics</h2>
        <div class="metric-grid">
            <div class="metric-card">
                <div class="metric-value">$($AnalysisData.Gameplay.Engagement.LevelsCompleted)</div>
                <div class="metric-label">Levels Completed</div>
            </div>
            <div class="metric-card">
                <div class="metric-value">$($AnalysisData.Gameplay.Difficulty.Deaths)</div>
                <div class="metric-label">Total Deaths</div>
            </div>
            <div class="metric-card">
                <div class="metric-value">$($AnalysisData.Gameplay.Difficulty.DeathRate)</div>
                <div class="metric-label">Deaths per Hour</div>
            </div>
        </div>

        <p>Difficulty Rating: <span class="rating rating-fair">$($AnalysisData.Gameplay.Difficulty.DifficultyRating)</span></p>

        <h2>Top Events</h2>
        <table>
            <thead>
                <tr>
                    <th>Event Name</th>
                    <th>Count</th>
                </tr>
            </thead>
            <tbody>
"@

    foreach ($event in $AnalysisData.EventPatterns.TopEvents) {
        $html += @"
                <tr>
                    <td>$($event.Name)</td>
                    <td>$($event.Value)</td>
                </tr>
"@
    }

    $html += @"
            </tbody>
        </table>

        <h2>Key Insights</h2>
"@

    foreach ($insight in $AnalysisData.Insights) {
        $html += @"
        <div class="insight">$insight</div>
"@
    }

    $html += @"
        <div class="footer">
            <p>Horror Project Analytics System - GDPR/CCPA Compliant</p>
            <p>All data is anonymized and processed in accordance with privacy regulations</p>
        </div>
    </div>
</body>
</html>
"@

    $html | Set-Content -Path $OutputFile -Encoding UTF8
}

function Generate-MarkdownReport {
    param($AnalysisData, $OutputFile)

    Write-Log "Generating Markdown report..."

    $markdown = @"
# Horror Project - Analytics Report

**Generated:** $($AnalysisData.GeneratedAt)

## Performance Overview

| Metric | Value | Rating |
|--------|-------|--------|
| Performance Score | $($AnalysisData.Performance.PerformanceScore) | - |
| Average FPS | $($AnalysisData.Performance.FPSAnalysis.Average) | $($AnalysisData.Performance.FPSAnalysis.Rating) |
| Min FPS | $($AnalysisData.Performance.FPSAnalysis.Minimum) | - |
| Max FPS | $($AnalysisData.Performance.FPSAnalysis.Maximum) | - |
| Avg Memory Usage | $($AnalysisData.Performance.MemoryAnalysis.AverageUsageMB) MB | $($AnalysisData.Performance.MemoryAnalysis.Rating) |

## Session Behavior

- **Total Sessions:** $($AnalysisData.SessionBehavior.TotalSessions)
- **Average Session Duration:** $($AnalysisData.SessionBehavior.AverageSessionDuration) seconds
- **Total Playtime:** $($AnalysisData.SessionBehavior.TotalPlaytime) hours

### Session Distribution

- Short Sessions (< 5 min): $($AnalysisData.SessionBehavior.SessionDistribution.Short)
- Medium Sessions (5-30 min): $($AnalysisData.SessionBehavior.SessionDistribution.Medium)
- Long Sessions (> 30 min): $($AnalysisData.SessionBehavior.SessionDistribution.Long)

## Gameplay Metrics

- **Levels Completed:** $($AnalysisData.Gameplay.Engagement.LevelsCompleted)
- **Total Deaths:** $($AnalysisData.Gameplay.Difficulty.Deaths)
- **Death Rate:** $($AnalysisData.Gameplay.Difficulty.DeathRate) per hour
- **Difficulty Rating:** $($AnalysisData.Gameplay.Difficulty.DifficultyRating)

## Top Events

| Event Name | Count |
|------------|-------|
"@

    foreach ($event in $AnalysisData.EventPatterns.TopEvents) {
        $markdown += "`n| $($event.Name) | $($event.Value) |"
    }

    $markdown += @"

## Key Insights

"@

    foreach ($insight in $AnalysisData.Insights) {
        $markdown += "- $insight`n"
    }

    $markdown += @"

---

*Horror Project Analytics System - GDPR/CCPA Compliant*
*All data is anonymized and processed in accordance with privacy regulations*
"@

    $markdown | Set-Content -Path $OutputFile -Encoding UTF8
}

# Main execution
Write-Log "Starting report generation..."

# Create output directory
if (-not (Test-Path $OutputPath)) {
    New-Item -ItemType Directory -Path $OutputPath -Force | Out-Null
}

# Find latest analysis
$latestAnalysis = Get-ChildItem -Path $AnalysisPath -Filter "analysis_*.json" |
    Sort-Object LastWriteTime -Descending |
    Select-Object -First 1

if (-not $latestAnalysis) {
    Write-Log "No analysis data found in: $AnalysisPath"
    exit 1
}

Write-Log "Generating report from: $($latestAnalysis.Name)"

$analysisData = Get-Content $latestAnalysis.FullName -Raw | ConvertFrom-Json

# Generate report based on format
$timestamp = Get-Date -Format 'yyyyMMdd_HHmmss'

switch ($Format.ToUpper()) {
    "HTML" {
        $outputFile = Join-Path $OutputPath "report_$timestamp.html"
        Generate-HTMLReport -AnalysisData $analysisData -OutputFile $outputFile
    }
    "MARKDOWN" {
        $outputFile = Join-Path $OutputPath "report_$timestamp.md"
        Generate-MarkdownReport -AnalysisData $analysisData -OutputFile $outputFile
    }
    "JSON" {
        $outputFile = Join-Path $OutputPath "report_$timestamp.json"
        $analysisData | ConvertTo-Json -Depth 10 | Set-Content -Path $outputFile -Encoding UTF8
    }
    default {
        Write-Log "Unknown format: $Format. Using HTML."
        $outputFile = Join-Path $OutputPath "report_$timestamp.html"
        Generate-HTMLReport -AnalysisData $analysisData -OutputFile $outputFile
    }
}

Write-Log "Report generated successfully: $outputFile"
