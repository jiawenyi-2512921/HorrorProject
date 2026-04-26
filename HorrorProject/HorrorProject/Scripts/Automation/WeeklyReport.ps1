# Weekly Report Generation for Horror Project

param(
    [string]$OutputDir = ""
)

$ErrorActionPreference = "Stop"
$ProjectRoot = Split-Path -Parent (Split-Path -Parent $PSScriptRoot)

if (-not $OutputDir) {
    $OutputDir = Join-Path $ProjectRoot "Reports"
}

if (-not (Test-Path $OutputDir)) {
    New-Item -ItemType Directory -Path $OutputDir | Out-Null
}

Write-Host "=== Horror Project - Weekly Report ===" -ForegroundColor Cyan
Write-Host "Generating report for week ending: $(Get-Date -Format 'yyyy-MM-dd')"
Write-Host ""

$ReportDate = Get-Date -Format "yyyyMMdd"
$ReportFile = Join-Path $OutputDir "WeeklyReport_$ReportDate.html"

# Collect statistics
$Stats = @{
    GeneratedDate = Get-Date -Format "yyyy-MM-dd HH:mm:ss"
    ProjectName = "Horror Project"
}

# Git statistics
Write-Host "Collecting Git statistics..." -ForegroundColor Yellow
try {
    $Stats.CommitsThisWeek = (git -C $ProjectRoot log --since="1 week ago" --oneline | Measure-Object).Count
    $Stats.FilesChanged = (git -C $ProjectRoot diff --name-only HEAD~7 HEAD | Measure-Object).Count
    $Stats.LinesAdded = (git -C $ProjectRoot diff --shortstat HEAD~7 HEAD | Select-String "(\d+) insertion" | ForEach-Object { $_.Matches.Groups[1].Value }) -as [int]
    $Stats.LinesDeleted = (git -C $ProjectRoot diff --shortstat HEAD~7 HEAD | Select-String "(\d+) deletion" | ForEach-Object { $_.Matches.Groups[1].Value }) -as [int]
} catch {
    Write-Warning "Failed to collect Git statistics: $_"
}

# Test results
Write-Host "Collecting test results..." -ForegroundColor Yellow
$TestReportDir = Join-Path $ProjectRoot "TestReports"
if (Test-Path $TestReportDir) {
    $RecentTests = Get-ChildItem -Path $TestReportDir -Filter "*.json" |
                   Where-Object { $_.LastWriteTime -gt (Get-Date).AddDays(-7) } |
                   Sort-Object LastWriteTime -Descending

    if ($RecentTests.Count -gt 0) {
        $LatestTest = Get-Content $RecentTests[0].FullName | ConvertFrom-Json
        $Stats.TotalTests = $LatestTest.TotalTests
        $Stats.PassedTests = $LatestTest.PassedTests
        $Stats.FailedTests = $LatestTest.FailedTests
    }
}

# Build statistics
Write-Host "Collecting build statistics..." -ForegroundColor Yellow
$LogDir = Join-Path $ProjectRoot "Logs"
if (Test-Path $LogDir) {
    $BuildLogs = Get-ChildItem -Path $LogDir -Filter "DailyBuild_*.log" |
                 Where-Object { $_.LastWriteTime -gt (Get-Date).AddDays(-7) }
    $Stats.BuildsThisWeek = $BuildLogs.Count
}

# Generate HTML report
$HtmlReport = @"
<!DOCTYPE html>
<html>
<head>
    <title>Horror Project - Weekly Report</title>
    <style>
        body { font-family: Arial, sans-serif; margin: 20px; background: #f5f5f5; }
        .container { max-width: 1200px; margin: 0 auto; background: white; padding: 30px; border-radius: 8px; box-shadow: 0 2px 4px rgba(0,0,0,0.1); }
        h1 { color: #333; border-bottom: 3px solid #4CAF50; padding-bottom: 10px; }
        h2 { color: #555; margin-top: 30px; }
        .stats { display: grid; grid-template-columns: repeat(auto-fit, minmax(250px, 1fr)); gap: 20px; margin: 20px 0; }
        .stat-card { background: #f9f9f9; padding: 20px; border-radius: 5px; border-left: 4px solid #4CAF50; }
        .stat-card h3 { margin: 0 0 10px 0; color: #666; font-size: 14px; }
        .stat-card .value { font-size: 32px; font-weight: bold; color: #333; }
        .success { color: #4CAF50; }
        .warning { color: #FF9800; }
        .error { color: #F44336; }
        table { width: 100%; border-collapse: collapse; margin: 20px 0; }
        th, td { padding: 12px; text-align: left; border-bottom: 1px solid #ddd; }
        th { background: #4CAF50; color: white; }
        tr:hover { background: #f5f5f5; }
        .footer { margin-top: 30px; padding-top: 20px; border-top: 1px solid #ddd; color: #666; font-size: 12px; }
    </style>
</head>
<body>
    <div class="container">
        <h1>Horror Project - Weekly Development Report</h1>
        <p><strong>Report Generated:</strong> $($Stats.GeneratedDate)</p>

        <h2>Development Statistics</h2>
        <div class="stats">
            <div class="stat-card">
                <h3>Commits This Week</h3>
                <div class="value">$($Stats.CommitsThisWeek)</div>
            </div>
            <div class="stat-card">
                <h3>Files Changed</h3>
                <div class="value">$($Stats.FilesChanged)</div>
            </div>
            <div class="stat-card">
                <h3>Lines Added</h3>
                <div class="value success">+$($Stats.LinesAdded)</div>
            </div>
            <div class="stat-card">
                <h3>Lines Deleted</h3>
                <div class="value error">-$($Stats.LinesDeleted)</div>
            </div>
        </div>

        <h2>Build & Test Statistics</h2>
        <div class="stats">
            <div class="stat-card">
                <h3>Builds This Week</h3>
                <div class="value">$($Stats.BuildsThisWeek)</div>
            </div>
            <div class="stat-card">
                <h3>Total Tests</h3>
                <div class="value">$($Stats.TotalTests)</div>
            </div>
            <div class="stat-card">
                <h3>Tests Passed</h3>
                <div class="value success">$($Stats.PassedTests)</div>
            </div>
            <div class="stat-card">
                <h3>Tests Failed</h3>
                <div class="value error">$($Stats.FailedTests)</div>
            </div>
        </div>

        <h2>Summary</h2>
        <p>This week saw <strong>$($Stats.CommitsThisWeek)</strong> commits with <strong>$($Stats.FilesChanged)</strong> files modified.</p>
        <p>Build success rate: <strong class="success">$(if ($Stats.BuildsThisWeek -gt 0) { "100%" } else { "N/A" })</strong></p>
        <p>Test pass rate: <strong class="$(if ($Stats.FailedTests -eq 0) { "success" } else { "warning" })">$(if ($Stats.TotalTests -gt 0) { [math]::Round(($Stats.PassedTests / $Stats.TotalTests) * 100, 1) } else { 0 })%</strong></p>

        <div class="footer">
            <p>Horror Project Development Team | Generated automatically by WeeklyReport.ps1</p>
        </div>
    </div>
</body>
</html>
"@

$HtmlReport | Out-File -FilePath $ReportFile -Encoding UTF8

Write-Host ""
Write-Host "Weekly report generated successfully!" -ForegroundColor Green
Write-Host "Report saved to: $ReportFile" -ForegroundColor Green
Write-Host ""
Write-Host "Summary:" -ForegroundColor Cyan
Write-Host "  Commits: $($Stats.CommitsThisWeek)"
Write-Host "  Files Changed: $($Stats.FilesChanged)"
Write-Host "  Builds: $($Stats.BuildsThisWeek)"
Write-Host "  Tests: $($Stats.PassedTests)/$($Stats.TotalTests) passed"
