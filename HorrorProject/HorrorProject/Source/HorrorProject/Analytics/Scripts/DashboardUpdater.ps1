# Dashboard Updater - Updates analytics dashboard with latest data
# Copyright Epic Games, Inc. All Rights Reserved.

param(
    [string]$ReportsPath = "",
    [string]$DashboardPath = "",
    [string]$DashboardUrl = "http://localhost:8080/dashboard",
    [switch]$AutoRefresh
)

$ErrorActionPreference = "Stop"

. (Join-Path $PSScriptRoot "..\..\..\..\Scripts\Validation\Common.ps1")
$ProjectRoot = Get-HorrorProjectRoot -StartPath $PSScriptRoot
if ([string]::IsNullOrWhiteSpace($ReportsPath)) { $ReportsPath = Join-Path $ProjectRoot "Saved\Analytics\Reports" }
if ([string]::IsNullOrWhiteSpace($DashboardPath)) { $DashboardPath = Join-Path $ProjectRoot "Saved\Analytics\Dashboard" }

function Write-Log {
    param([string]$Message, [string]$Level = "INFO")
    $timestamp = Get-Date -Format "yyyy-MM-dd HH:mm:ss"
    Write-Host "[$timestamp] [$Level] $Message"
}

function Create-DashboardHTML {
    param($ReportData, $OutputFile)

    Write-Log "Creating dashboard HTML..."

    $html = @"
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Horror Project - Analytics Dashboard</title>
    <style>
        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }
        body {
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            background: linear-gradient(135deg, #1a1a1a 0%, #2d1b2e 100%);
            color: #e0e0e0;
            min-height: 100vh;
        }
        .header {
            background: rgba(0,0,0,0.5);
            padding: 20px;
            text-align: center;
            border-bottom: 2px solid #ff6b6b;
        }
        .header h1 {
            color: #ff6b6b;
            font-size: 2.5em;
            text-shadow: 0 0 10px rgba(255,107,107,0.5);
        }
        .last-updated {
            color: #999;
            margin-top: 10px;
        }
        .dashboard-container {
            max-width: 1400px;
            margin: 30px auto;
            padding: 0 20px;
        }
        .metrics-grid {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(280px, 1fr));
            gap: 20px;
            margin-bottom: 30px;
        }
        .metric-card {
            background: rgba(42, 42, 42, 0.9);
            padding: 25px;          border-radius: 12px;
            border-left: 5px solid #4ecdc4;
            box-shadow: 0 4px 15px rgba(0,0,0,0.3);
            transition: transform 0.3s ease;
        }
        .metric-card:hover {
            transform: translateY(-5px);
        }
        .metric-value {
            font-size: 3em;
            font-weight: bold;
            color: #4ecdc4;
            text-shadow: 0 0 10px rgba(78,205,196,0.3);
        }
        .metric-label {
            color: #999;
            margin-top: 10px;
            font-size: 1.1em;
        }
        .section {
            background: rgba(42, 42, 42, 0.9);
            padding: 30px;
            border-radius: 12px;
            margin-bottom: 30px;
            box-shadow: 0 4px 15px rgba(0,0,0,0.3);
        }
        .section h2 {
            color: #4ecdc4;
            margin-bottom: 20px;
            font-size: 1.8em;
            border-bottom: 2px solid #4ecdc4;
            padding-bottom: 10px;
        }
        .chart-empty-state {
            background: rgba(0,0,0,0.3);
            height: 300px;
            border-radius: 8px;
            display: flex;
            align-items: center;
            justify-content: center;
            color: #666;
            font-size: 1.2em;
        }
        .status-indicator {
            display: inline-block;
            width: 12px;
            height: 12px;
            border-radius: 50%;
            margin-right: 8px;
            animation: pulse 2s infinite;
        }
        .status-excellent { background-color: #48bb78; }
        .status-good { background-color: #4299e1; }
        .status-fair { background-color: #ed8936; }
        .status-poor { background-color: #f56565; }
        @keyframes pulse {
            0%, 100% { opacity: 1; }
            50% { opacity: 0.5; }
        }
        .insight-list {
            list-style: none;
        }
        .insight-item {
            background: rgba(0,0,0,0.3);
            padding: 15px;
            margin: 10px 0;
            border-left: 4px solid #ffd93d;
            border-radius: 4px;
        }
        .footer {
            text-align: center;
            padding: 30px;
            color: #666;
            border-top: 1px solid #444;
        }
        .refresh-btn {
            background: #4ecdc4;
            color: #1a1a1a;
            border: none;
            padding: 12px 30px;
            border-radius: 25px;
            font-size: 1em;
            font-weight: bold;
            cursor: pointer;
            transition: all 0.3s ease;
        }
        .refresh-btn:hover {
            background: #45b8b0;
            transform: scale(1.05);
        }
    </style>
    <script>
        function refreshDashboard() {
            location.reload();
        }

        // Auto-refresh every 5 minutes if enabled
        const autoRefresh = $($AutoRefresh.ToString().ToLower());
        if (autoRefresh) {
            setInterval(refreshDashboard, 300000);
        }
    </script>
</head>
<body>
    <div class="header">
        <h1>🎮 Horror Project Analytics Dashboard</h1>
        <div class="last-updated">Last Updated: $(Get-Date -Format "yyyy-MM-dd HH:mm:ss")</div>
        <button class="refresh-btn" onclick="refreshDashboard()">🔄 Refresh</button>
    </div>

    <div class="dashboard-container">
        <div class="metrics-grid">
            <div class="metric-card">
                <div class="metric-value">$($ReportData.Performance.PerformanceScore)</div>
                <div class="metric-label">
                    <span class="status-indicator status-$(($ReportData.Performance.FPSAnalysis.Rating).ToLower())"></span>
                    Performance Score
                </div>
            </div>
            <div class="metric-card">
                <div class="metric-value">$($ReportData.Performance.FPSAnalysis.Average)</div>
                <div class="metric-label">Average FPS</div>
            </div>
            <div class="metric-card">
                <div class="metric-value">$($ReportData.SessionBehavior.TotalSessions)</div>
                <div class="metric-label">Total Sessions</div>
            </div>
            <div class="metric-card">
                <div class="metric-value">$($ReportData.SessionBehavior.TotalPlaytime)h</div>
                <div class="metric-label">Total Playtime</div>
            </div>
        </div>

        <div class="section">
            <h2>📊 Performance Metrics</h2>
            <div class="metrics-grid">
                <div>
                    <h3 style="color: #999;">FPS Analysis</h3>
                    <p>Average: <strong style="color: #4ecdc4;">$($ReportData.Performance.FPSAnalysis.Average) FPS</strong></p>
                    <p>Min: <strong>$($ReportData.Performance.FPSAnalysis.Minimum) FPS</strong></p>
                    <p>Max: <strong>$($ReportData.Performance.FPSAnalysis.Maximum) FPS</strong></p>
                    <p>Rating: <strong style="color: #ffd93d;">$($ReportData.Performance.FPSAnalysis.Rating)</strong></p>
                </div>
                <div>
                    <h3 style="color: #999;">Memory Usage</h3>
                    <p>Average: <strong style="color: #4ecdc4;">$($ReportData.Performance.MemoryAnalysis.AverageUsageMB) MB</strong></p>
                    <p>Rating: <strong style="color: #ffd93d;">$($ReportData.Performance.MemoryAnalysis.Rating)</strong></p>
                </div>
            </div>
        </div>

        <div class="section">
            <h2>🎯 Gameplay Metrics</h2>
            <div class="metrics-grid">
                <div class="metric-card">
                    <div class="metric-value">$($ReportData.Gameplay.Engagement.LevelsCompleted)</div>
                    <div class="metric-label">Levels Completed</div>
                </div>
                <div class="metric-card">
                    <div class="metric-value">$($ReportData.Gameplay.Difficulty.Deaths)</div>
                    <div class="metric-label">Total Deaths</div>
                </div>
                <div class="metric-card">
                    <div class="metric-value">$($ReportData.Gameplay.Difficulty.DeathRate)</div>
                    <div class="metric-label">Deaths per Hour</div>
                </div>
                <div class="metric-card">
                    <div class="metric-value">$($ReportData.Gameplay.Difficulty.DifficultyRating)</div>
                    <div class="metric-label">Difficulty Rating</div>
                </div>
            </div>
        </div>

        <div class="section">
            <h2>💡 Key Insights</h2>
            <ul class="insight-list">
"@

    foreach ($insight in $ReportData.Insights) {
        $html += @"
                <li class="insight-item">$insight</li>
"@
    }

    $html += @"
            </ul>
        </div>

        <div class="section">
            <h2>📈 Session Distribution</h2>
            <div class="metrics-grid">
                <div class="metric-card">
                    <div class="metric-value">$($ReportData.SessionBehavior.SessionDistribution.Short)</div>
                    <div class="metric-label">Short Sessions (&lt; 5 min)</div>
                </div>
                <div class="metric-card">
                    <div class="metric-value">$($ReportData.SessionBehavior.SessionDistribution.Medium)</div>
                    <div class="metric-label">Medium Sessions (5-30 min)</div>
                </div>
                <div class="metric-card">
                    <div class="metric-value">$($ReportData.SessionBehavior.SessionDistribution.Long)</div>
                    <div class="metric-label">Long Sessions (&gt; 30 min)</div>
                </div>
            </div>
        </div>
    </div>

    <div class="footer">
        <p><strong>Horror Project Analytics System</strong></p>
        <p>GDPR/CCPA Compliant - All data is anonymized</p>
        <p style="margin-top: 10px; font-size: 0.9em;">© 2026 Epic Games, Inc. All Rights Reserved.</p>
    </div>
</body>
</html>
"@

    $html | Set-Content -Path $OutputFile -Encoding UTF8
}

# Main execution
Write-Log "Starting dashboard update..."

# Create dashboard directory
if (-not (Test-Path $DashboardPath)) {
    New-Item -ItemType Directory -Path $DashboardPath -Force | Out-Null
}

# Find latest report
$latestReport = Get-ChildItem -Path $ReportsPath -Filter "analysis_*.json" -ErrorAction SilentlyContinue |
    Sort-Object LastWriteTime -Descending |
    Select-Object -First 1

if (-not $latestReport) {
    Write-Log "No report data found. Creating sample dashboard..." "WARNING"

    # Create sample data
    $sampleData = @{
        Performance = @{
            PerformanceScore = 85
            FPSAnalysis = @{
                Average = 58.5
                Minimum = 45.2
                Maximum = 60.0
                Rating = "Good"
            }
            MemoryAnalysis = @{
                AverageUsageMB = 2048.5
                Rating = "Excellent"
            }
        }
        SessionBehavior = @{
            TotalSessions = 0
            TotalPlaytime = 0
            AverageSessionDuration = 0
            SessionDistribution = @{
                Short = 0
                Medium = 0
                Long = 0
            }
        }
        Gameplay = @{
            Engagement = @{
                LevelsCompleted = 0
            }
            Difficulty = @{
                Deaths = 0
                DeathRate = 0
                DifficultyRating = "N/A"
            }
        }
        Insights = @(
            "No analytics data available yet",
            "Start playing to generate analytics"
        )
    }

    $reportData = $sampleData
} else {
    Write-Log "Loading report from: $($latestReport.Name)"
    $reportData = Get-Content $latestReport.FullName -Raw | ConvertFrom-Json
}

# Create dashboard
$dashboardFile = Join-Path $DashboardPath "index.html"
Create-DashboardHTML -ReportData $reportData -OutputFile $dashboardFile

Write-Log "Dashboard updated successfully: $dashboardFile"

# Open dashboard in browser if requested
if ($PSBoundParameters.ContainsKey('DashboardUrl')) {
    Write-Log "Opening dashboard in browser..."
    Start-Process $dashboardFile
}

Write-Log "Dashboard is ready at: $dashboardFile"
