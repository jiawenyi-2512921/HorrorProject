# GenerateDashboard.ps1 - 生成HTML仪表板
param(
    [string]$OutputPath = "dashboard.html"
)

$ErrorActionPreference = "Stop"
$MonitoringRoot = Split-Path -Parent $PSScriptRoot
$DataDir = Join-Path $MonitoringRoot "Data"

function Get-LatestMetrics {
    $metrics = @{
        Build = @{}
        Test = @{}
        Performance = @{}
        Assets = @{}
    }

    # 构建状态
    $buildFile = Join-Path $DataDir "Build\build-status-latest.json"
    if (Test-Path $buildFile) {
        $metrics.Build = Get-Content $buildFile | ConvertFrom-Json -AsHashtable
    }

    # 测试状态
    $testFile = Join-Path $DataDir "Test\test-status-latest.json"
    if (Test-Path $testFile) {
        $metrics.Test = Get-Content $testFile | ConvertFrom-Json -AsHashtable
    }

    # 性能指标
    $perfFile = Join-Path $DataDir "Performance\perf-latest.json"
    if (Test-Path $perfFile) {
        $metrics.Performance = Get-Content $perfFile | ConvertFrom-Json -AsHashtable
    }

    # 资产状态
    $assetFile = Join-Path $DataDir "Assets\asset-status-latest.json"
    if (Test-Path $assetFile) {
        $metrics.Assets = Get-Content $assetFile | ConvertFrom-Json -AsHashtable
    }

    return $metrics
}

function Get-RecentAlerts {
    param([int]$Count = 10)

    $alertDir = Join-Path $MonitoringRoot "Data\Alerts"
    if (-not (Test-Path $alertDir)) {
        return @()
    }

    $alerts = Get-ChildItem $alertDir -Filter "alert-*.json" |
        Sort-Object LastWriteTime -Descending |
        Select-Object -First $Count |
        ForEach-Object { Get-Content $_.FullName | ConvertFrom-Json }

    return $alerts
}

function Generate-DashboardHTML {
    param([hashtable]$Metrics, [array]$Alerts)

    $html = @"
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>HorrorProject Monitoring Dashboard</title>
    <style>
        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }

        body {
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            background: linear-gradient(135deg, #1e1e2e 0%, #2d2d44 100%);
            color: #e0e0e0;
            padding: 20px;
        }

        .container {
            max-width: 1400px;
            margin: 0 auto;
        }

        header {
            text-align: center;
            padding: 30px 0;
            background: rgba(255, 255, 255, 0.05);
            border-radius: 10px;
            margin-bottom: 30px;
            box-shadow: 0 4px 6px rgba(0, 0, 0, 0.3);
        }

        h1 {
            font-size: 2.5em;
            color: #00d4ff;
            text-shadow: 0 0 10px rgba(0, 212, 255, 0.5);
        }

        .last-update {
            color: #888;
            margin-top: 10px;
        }

        .grid {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(300px, 1fr));
            gap: 20px;
            margin-bottom: 30px;
        }

        .card {
            background: rgba(255, 255, 255, 0.05);
            border-radius: 10px;
            padding: 20px;
            box-shadow: 0 4px 6px rgba(0, 0, 0, 0.3);
            border: 1px solid rgba(255, 255, 255, 0.1);
            transition: transform 0.3s ease;
        }

        .card:hover {
            transform: translateY(-5px);
        }

        .card-header {
            display: flex;
            justify-content: space-between;
            align-items: center;
            margin-bottom: 15px;
            padding-bottom: 10px;
            border-bottom: 2px solid rgba(255, 255, 255, 0.1);
        }

        .card-title {
            font-size: 1.3em;
            color: #00d4ff;
        }

        .status-badge {
            padding: 5px 15px;
            border-radius: 20px;
            font-size: 0.9em;
            font-weight: bold;
        }

        .status-success {
            background: #28a745;
            color: white;
        }

        .status-warning {
            background: #ffc107;
            color: #333;
        }

        .status-error {
            background: #dc3545;
            color: white;
        }

        .status-info {
            background: #17a2b8;
            color: white;
        }

        .metric {
            display: flex;
            justify-content: space-between;
            padding: 10px 0;
            border-bottom: 1px solid rgba(255, 255, 255, 0.05);
        }

        .metric:last-child {
            border-bottom: none;
        }

        .metric-label {
            color: #aaa;
        }

        .metric-value {
            font-weight: bold;
            color: #fff;
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

        .alerts-section {
            background: rgba(255, 255, 255, 0.05);
            border-radius: 10px;
            padding: 20px;
            box-shadow: 0 4px 6px rgba(0, 0, 0, 0.3);
        }

        .alerts-header {
            font-size: 1.5em;
            color: #00d4ff;
            margin-bottom: 20px;
        }

        .alert-item {
            background: rgba(255, 255, 255, 0.03);
            border-left: 4px solid;
            padding: 15px;
            margin-bottom: 10px;
            border-radius: 5px;
        }

        .alert-item.critical {
            border-left-color: #dc3545;
        }

        .alert-item.warning {
            border-left-color: #ffc107;
        }

        .alert-item.info {
            border-left-color: #17a2b8;
        }

        .alert-time {
            color: #888;
            font-size: 0.9em;
        }

        .alert-message {
            margin-top: 5px;
            color: #e0e0e0;
        }

        .progress-bar {
            width: 100%;
            height: 20px;
            background: rgba(255, 255, 255, 0.1);
            border-radius: 10px;
            overflow: hidden;
            margin-top: 5px;
        }

        .progress-fill {
            height: 100%;
            background: linear-gradient(90deg, #00d4ff, #0099cc);
            transition: width 0.3s ease;
        }

        .chart-placeholder {
            width: 100%;
            height: 200px;
            background: rgba(255, 255, 255, 0.03);
            border-radius: 5px;
            display: flex;
            align-items: center;
            justify-content: center;
            color: #666;
            margin-top: 15px;
        }
    </style>
</head>
<body>
    <div class="container">
        <header>
            <h1>🎮 HorrorProject Monitoring Dashboard</h1>
            <div class="last-update">Last Updated: $(Get-Date -Format 'yyyy-MM-dd HH:mm:ss')</div>
        </header>

        <div class="grid">
            <!-- Build Status Card -->
            <div class="card">
                <div class="card-header">
                    <div class="card-title">🔨 Build Status</div>
                    <span class="status-badge status-$(if ($Metrics.Build.Status -eq 'Success') { 'success' } elseif ($Metrics.Build.Status -eq 'Failed') { 'error' } else { 'info' })">
                        $($Metrics.Build.Status)
                    </span>
                </div>
                <div class="metric">
                    <span class="metric-label">Duration</span>
                    <span class="metric-value">$($Metrics.Build.Duration)s</span>
                </div>
                <div class="metric">
                    <span class="metric-label">Errors</span>
                    <span class="metric-value $(if ($Metrics.Build.Errors -eq 0) { 'good' } else { 'bad' })">$($Metrics.Build.Errors)</span>
                </div>
                <div class="metric">
                    <span class="metric-label">Warnings</span>
                    <span class="metric-value $(if ($Metrics.Build.Warnings -eq 0) { 'good' } elseif ($Metrics.Build.Warnings -lt 10) { 'warning' } else { 'bad' })">$($Metrics.Build.Warnings)</span>
                </div>
                <div class="metric">
                    <span class="metric-label">Configuration</span>
                    <span class="metric-value">$($Metrics.Build.Configuration)</span>
                </div>
            </div>

            <!-- Test Status Card -->
            <div class="card">
                <div class="card-header">
                    <div class="card-title">🧪 Test Status</div>
                    <span class="status-badge status-$(if ($Metrics.Test.Status -eq 'Passed') { 'success' } else { 'error' })">
                        $($Metrics.Test.Status)
                    </span>
                </div>
                <div class="metric">
                    <span class="metric-label">Total Tests</span>
                    <span class="metric-value">$($Metrics.Test.Total)</span>
                </div>
                <div class="metric">
                    <span class="metric-label">Passed</span>
                    <span class="metric-value good">$($Metrics.Test.Passed)</span>
                </div>
                <div class="metric">
                    <span class="metric-label">Failed</span>
                    <span class="metric-value $(if ($Metrics.Test.Failed -eq 0) { 'good' } else { 'bad' })">$($Metrics.Test.Failed)</span>
                </div>
                <div class="metric">
                    <span class="metric-label">Failure Rate</span>
                    <span class="metric-value $(if ($Metrics.Test.FailureRate -eq 0) { 'good' } elseif ($Metrics.Test.FailureRate -lt 5) { 'warning' } else { 'bad' })">$($Metrics.Test.FailureRate)%</span>
                </div>
            </div>

            <!-- Performance Card -->
            <div class="card">
                <div class="card-header">
                    <div class="card-title">⚡ Performance</div>
                    <span class="status-badge status-$(if ($Metrics.Performance.Process.Status -eq 'Running') { 'success' } else { 'info' })">
                        $($Metrics.Performance.Process.Status)
                    </span>
                </div>
                <div class="metric">
                    <span class="metric-label">CPU Usage</span>
                    <span class="metric-value $(if ($Metrics.Performance.System.CPU.Usage -lt 70) { 'good' } elseif ($Metrics.Performance.System.CPU.Usage -lt 90) { 'warning' } else { 'bad' })">$($Metrics.Performance.System.CPU.Usage)%</span>
                </div>
                <div class="progress-bar">
                    <div class="progress-fill" style="width: $($Metrics.Performance.System.CPU.Usage)%"></div>
                </div>
                <div class="metric">
                    <span class="metric-label">Memory Usage</span>
                    <span class="metric-value $(if ($Metrics.Performance.System.Memory.Usage -lt 70) { 'good' } elseif ($Metrics.Performance.System.Memory.Usage -lt 85) { 'warning' } else { 'bad' })">$($Metrics.Performance.System.Memory.UsedGB)/$($Metrics.Performance.System.Memory.TotalGB) GB</span>
                </div>
                <div class="progress-bar">
                    <div class="progress-fill" style="width: $($Metrics.Performance.System.Memory.Usage)%"></div>
                </div>
            </div>

            <!-- Assets Card -->
            <div class="card">
                <div class="card-header">
                    <div class="card-title">📦 Assets</div>
                    <span class="status-badge status-$(if ($Metrics.Assets.Status -eq 'Healthy') { 'success' } else { 'warning' })">
                        $($Metrics.Assets.Status)
                    </span>
                </div>
                <div class="metric">
                    <span class="metric-label">Total Assets</span>
                    <span class="metric-value">$($Metrics.Assets.TotalAssets)</span>
                </div>
                <div class="metric">
                    <span class="metric-label">Total Size</span>
                    <span class="metric-value">$($Metrics.Assets.TotalSizeGB) GB</span>
                </div>
                <div class="metric">
                    <span class="metric-label">Errors</span>
                    <span class="metric-value $(if ($Metrics.Assets.Errors -eq 0) { 'good' } else { 'bad' })">$($Metrics.Assets.Errors)</span>
                </div>
                <div class="metric">
                    <span class="metric-label">Warnings</span>
                    <span class="metric-value $(if ($Metrics.Assets.Warnings -eq 0) { 'good' } else { 'warning' })">$($Metrics.Assets.Warnings)</span>
                </div>
            </div>
        </div>

        <!-- Recent Alerts Section -->
        <div class="alerts-section">
            <div class="alerts-header">🚨 Recent Alerts</div>
"@

    if ($Alerts.Count -eq 0) {
        $html += @"
            <div class="alert-item info">
                <div class="alert-message">No recent alerts</div>
            </div>
"@
    } else {
        foreach ($alert in $Alerts) {
            $severityClass = $alert.Severity.ToLower()
            $html += @"
            <div class="alert-item $severityClass">
                <div class="alert-time">$($alert.Timestamp) - [$($alert.Severity)] $($alert.Type)</div>
                <div class="alert-message">$($alert.Message)</div>
            </div>
"@
        }
    }

    $html += @"
        </div>
    </div>

    <script>
        // Auto-refresh every 60 seconds
        setTimeout(function() {
            location.reload();
        }, 60000);
    </script>
</body>
</html>
"@

    return $html
}

# 主执行
Write-Host "Generating dashboard..." -ForegroundColor Cyan

$metrics = Get-LatestMetrics
$alerts = Get-RecentAlerts -Count 10

$html = Generate-DashboardHTML -Metrics $metrics -Alerts $alerts

$outputFile = Join-Path $PSScriptRoot $OutputPath
$html | Set-Content $outputFile -Encoding UTF8

Write-Host "Dashboard generated: $outputFile" -ForegroundColor Green
Write-Host "Open in browser to view" -ForegroundColor Yellow
