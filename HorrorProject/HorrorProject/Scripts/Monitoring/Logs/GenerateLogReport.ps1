# GenerateLogReport.ps1 - 日志报告生成
param(
    [ValidateSet("Daily", "Weekly")]
    [string]$ReportType = "Daily",
    [string]$OutputPath = "log-report.html"
)

$ErrorActionPreference = "Stop"
$MonitoringRoot = Split-Path -Parent $PSScriptRoot
$AggregatedDir = Join-Path $MonitoringRoot "Data\Aggregated"

function Get-ReportPeriod {
    param([string]$Type)

    $end = Get-Date
    $start = switch ($Type) {
        "Daily" { $end.AddDays(-1) }
        "Weekly" { $end.AddDays(-7) }
    }

    return @{
        Start = $start
        End = $end
        Type = $Type
    }
}

function Get-AggregatedData {
    param([datetime]$StartDate, [datetime]$EndDate)

    $data = @{
        Build = @()
        Test = @()
    }

    # 获取构建日志
    $buildFiles = Get-ChildItem $AggregatedDir -Filter "Build-aggregated-*.json" |
        Where-Object { $_.LastWriteTime -ge $StartDate -and $_.LastWriteTime -le $EndDate }

    foreach ($file in $buildFiles) {
        $content = Get-Content $file.FullName | ConvertFrom-Json
        $data.Build += $content
    }

    # 获取测试日志
    $testFiles = Get-ChildItem $AggregatedDir -Filter "Test-aggregated-*.json" |
        Where-Object { $_.LastWriteTime -ge $StartDate -and $_.LastWriteTime -le $EndDate }

    foreach ($file in $testFiles) {
        $content = Get-Content $file.FullName | ConvertFrom-Json
        $data.Test += $content
    }

    return $data
}

function Calculate-Statistics {
    param([hashtable]$Data)

    $stats = @{
        Build = @{
            TotalBuilds = $Data.Build.Count
            TotalLogs = 0
            TotalSizeMB = 0
            SuccessfulBuilds = 0
            FailedBuilds = 0
            TotalErrors = 0
            TotalWarnings = 0
        }
        Test = @{
            TotalRuns = $Data.Test.Count
            TotalLogs = 0
            TotalSizeMB = 0
            TotalTests = 0
            PassedTests = 0
            FailedTests = 0
            SkippedTests = 0
        }
    }

    # 构建统计
    foreach ($build in $Data.Build) {
        $stats.Build.TotalLogs += $build.TotalLogs
        $stats.Build.TotalSizeMB += $build.TotalSizeMB

        foreach ($log in $build.Logs) {
            if ($log.Parsed) {
                if ($log.Parsed.Status -eq "Success") {
                    $stats.Build.SuccessfulBuilds++
                } elseif ($log.Parsed.Status -eq "Failed") {
                    $stats.Build.FailedBuilds++
                }
                $stats.Build.TotalErrors += $log.Parsed.Statistics.TotalErrors
                $stats.Build.TotalWarnings += $log.Parsed.Statistics.TotalWarnings
            }
        }
    }

    # 测试统计
    foreach ($test in $Data.Test) {
        $stats.Test.TotalLogs += $test.TotalLogs
        $stats.Test.TotalSizeMB += $test.TotalSizeMB

        foreach ($log in $test.Logs) {
            if ($log.Parsed) {
                $stats.Test.TotalTests += $log.Parsed.Statistics.Total
                $stats.Test.PassedTests += $log.Parsed.Statistics.Passed
                $stats.Test.FailedTests += $log.Parsed.Statistics.Failed
                $stats.Test.SkippedTests += $log.Parsed.Statistics.Skipped
            }
        }
    }

    return $stats
}

function Generate-ReportHTML {
    param(
        [hashtable]$Period,
        [hashtable]$Statistics
    )

    $html = @"
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>$($Period.Type) Log Report - HorrorProject</title>
    <style>
        body {
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            background: #f5f5f5;
            padding: 20px;
            margin: 0;
        }
        .container {
            max-width: 1200px;
            margin: 0 auto;
            background: white;
            padding: 30px;
            border-radius: 10px;
            box-shadow: 0 2px 10px rgba(0,0,0,0.1);
        }
        h1 {
            color: #333;
            border-bottom: 3px solid #007bff;
            padding-bottom: 10px;
        }
        .period {
            color: #666;
            margin-bottom: 30px;
        }
        .section {
            margin-bottom: 40px;
        }
        .section-title {
            font-size: 1.5em;
            color: #007bff;
            margin-bottom: 15px;
        }
        .stats-grid {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(200px, 1fr));
            gap: 15px;
            margin-bottom: 20px;
        }
        .stat-card {
            background: #f8f9fa;
            padding: 15px;
            border-radius: 5px;
            border-left: 4px solid #007bff;
        }
        .stat-label {
            color: #666;
            font-size: 0.9em;
        }
        .stat-value {
            font-size: 2em;
            font-weight: bold;
            color: #333;
        }
        .stat-value.success {
            color: #28a745;
        }
        .stat-value.error {
            color: #dc3545;
        }
        .stat-value.warning {
            color: #ffc107;
        }
        table {
            width: 100%;
            border-collapse: collapse;
            margin-top: 15px;
        }
        th, td {
            padding: 12px;
            text-align: left;
            border-bottom: 1px solid #ddd;
        }
        th {
            background: #007bff;
            color: white;
        }
        tr:hover {
            background: #f5f5f5;
        }
        .footer {
            text-align: center;
            color: #666;
            margin-top: 40px;
            padding-top: 20px;
            border-top: 1px solid #ddd;
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>📊 $($Period.Type) Log Report</h1>
        <div class="period">
            Period: $($Period.Start.ToString('yyyy-MM-dd HH:mm')) - $($Period.End.ToString('yyyy-MM-dd HH:mm'))
        </div>

        <div class="section">
            <div class="section-title">🔨 Build Statistics</div>
            <div class="stats-grid">
                <div class="stat-card">
                    <div class="stat-label">Total Builds</div>
                    <div class="stat-value">$($Statistics.Build.TotalBuilds)</div>
                </div>
                <div class="stat-card">
                    <div class="stat-label">Successful</div>
                    <div class="stat-value success">$($Statistics.Build.SuccessfulBuilds)</div>
                </div>
                <div class="stat-card">
                    <div class="stat-label">Failed</div>
                    <div class="stat-value error">$($Statistics.Build.FailedBuilds)</div>
                </div>
                <div class="stat-card">
                    <div class="stat-label">Total Errors</div>
                    <div class="stat-value error">$($Statistics.Build.TotalErrors)</div>
                </div>
                <div class="stat-card">
                    <div class="stat-label">Total Warnings</div>
                    <div class="stat-value warning">$($Statistics.Build.TotalWarnings)</div>
                </div>
                <div class="stat-card">
                    <div class="stat-label">Log Size</div>
                    <div class="stat-value">$([math]::Round($Statistics.Build.TotalSizeMB, 2)) MB</div>
                </div>
            </div>
        </div>

        <div class="section">
            <div class="section-title">🧪 Test Statistics</div>
            <div class="stats-grid">
                <div class="stat-card">
                    <div class="stat-label">Total Test Runs</div>
                    <div class="stat-value">$($Statistics.Test.TotalRuns)</div>
                </div>
                <div class="stat-card">
                    <div class="stat-label">Total Tests</div>
                    <div class="stat-value">$($Statistics.Test.TotalTests)</div>
                </div>
                <div class="stat-card">
                    <div class="stat-label">Passed</div>
                    <div class="stat-value success">$($Statistics.Test.PassedTests)</div>
                </div>
                <div class="stat-card">
                    <div class="stat-label">Failed</div>
                    <div class="stat-value error">$($Statistics.Test.FailedTests)</div>
                </div>
                <div class="stat-card">
                    <div class="stat-label">Skipped</div>
                    <div class="stat-value">$($Statistics.Test.SkippedTests)</div>
                </div>
                <div class="stat-card">
                    <div class="stat-label">Log Size</div>
                    <div class="stat-value">$([math]::Round($Statistics.Test.TotalSizeMB, 2)) MB</div>
                </div>
            </div>
        </div>

        <div class="footer">
            Generated on $(Get-Date -Format 'yyyy-MM-dd HH:mm:ss')<br>
            HorrorProject Monitoring System
        </div>
    </div>
</body>
</html>
"@

    return $html
}

# 主执行
Write-Host "Generating $ReportType log report..." -ForegroundColor Cyan

$period = Get-ReportPeriod -Type $ReportType
$data = Get-AggregatedData -StartDate $period.Start -EndDate $period.End
$statistics = Calculate-Statistics -Data $data

$html = Generate-ReportHTML -Period $period -Statistics $statistics

$outputFile = Join-Path $PSScriptRoot $OutputPath
$html | Set-Content $outputFile -Encoding UTF8

Write-Host "Report generated: $outputFile" -ForegroundColor Green
Write-Host "Period: $($period.Start.ToString('yyyy-MM-dd')) to $($period.End.ToString('yyyy-MM-dd'))" -ForegroundColor Gray
