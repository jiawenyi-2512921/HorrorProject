# HorrorProject - Daily Build Report
# 21-Day Sprint Optimized

param(
    [switch]$SendEmail,
    [string]$EmailTo = ""
)

$ErrorActionPreference = "Stop"
$ProjectRoot = "D:\gptzuo\HorrorProject\HorrorProject"
$LogDir = "$ProjectRoot\Build\Logs"
$ReportDir = "$ProjectRoot\Build\Reports"
$ReportDate = Get-Date -Format "yyyy-MM-dd"

New-Item -ItemType Directory -Force -Path $ReportDir | Out-Null
$ReportFile = "$ReportDir\DailyReport_$ReportDate.html"

function Get-BuildStatus {
    $buildLogs = Get-ChildItem -Path "$LogDir" -Filter "Build_*.log" -Recurse -ErrorAction SilentlyContinue |
        Where-Object { $_.LastWriteTime -gt (Get-Date).AddDays(-1) } |
        Sort-Object LastWriteTime -Descending

    if ($buildLogs.Count -eq 0) {
        return @{
            Status = "No builds"
            Count = 0
            LastBuild = "N/A"
        }
    }

    $latestBuild = $buildLogs[0]
    $buildContent = Get-Content $latestBuild.FullName -Raw

    $status = if ($buildContent -match "BUILD COMPLETE") { "Success" } else { "Failed" }

    return @{
        Status = $status
        Count = $buildLogs.Count
        LastBuild = $latestBuild.LastWriteTime.ToString("yyyy-MM-dd HH:mm:ss")
    }
}

function Get-TestStatus {
    $testLogs = Get-ChildItem -Path "$LogDir\Tests" -Filter "Test_*.log" -ErrorAction SilentlyContinue |
        Where-Object { $_.LastWriteTime -gt (Get-Date).AddDays(-1) } |
        Sort-Object LastWriteTime -Descending

    if ($testLogs.Count -eq 0) {
        return @{
            Status = "No tests"
            Count = 0
            Passed = 0
            Failed = 0
        }
    }

    $latestTest = $testLogs[0]
    $testContent = Get-Content $latestTest.FullName -Raw

    $status = if ($testContent -match "All tests passed") { "Success" } else { "Failed" }

    return @{
        Status = $status
        Count = $testLogs.Count
        Passed = 0
        Failed = 0
    }
}

function Get-AssetStatus {
    $assetLogs = Get-ChildItem -Path "$LogDir\AssetValidation" -Filter "AssetValidation_*.log" -ErrorAction SilentlyContinue |
        Where-Object { $_.LastWriteTime -gt (Get-Date).AddDays(-1) } |
        Sort-Object LastWriteTime -Descending

    if ($assetLogs.Count -eq 0) {
        return @{
            Status = "No validation"
            Issues = 0
        }
    }

    $latestValidation = $assetLogs[0]
    $validationContent = Get-Content $latestValidation.FullName -Raw

    $issueCount = ([regex]::Matches($validationContent, "WARN")).Count

    return @{
        Status = if ($issueCount -eq 0) { "Clean" } else { "Issues found" }
        Issues = $issueCount
    }
}

function Get-ProjectStats {
    $contentDir = "$ProjectRoot\Content"

    $stats = @{
        Maps = (Get-ChildItem -Path $contentDir -Filter *.umap -Recurse -ErrorAction SilentlyContinue).Count
        Assets = (Get-ChildItem -Path $contentDir -Filter *.uasset -Recurse -ErrorAction SilentlyContinue).Count
        SourceFiles = (Get-ChildItem -Path "$ProjectRoot\Source" -Filter *.cpp -Recurse -ErrorAction SilentlyContinue).Count
        HeaderFiles = (Get-ChildItem -Path "$ProjectRoot\Source" -Filter *.h -Recurse -ErrorAction SilentlyContinue).Count
    }

    return $stats
}

function New-HTMLReport {
    $buildStatus = Get-BuildStatus
    $testStatus = Get-TestStatus
    $assetStatus = Get-AssetStatus
    $projectStats = Get-ProjectStats

    $html = @"
<!DOCTYPE html>
<html>
<head>
    <title>HorrorProject Daily Build Report - $ReportDate</title>
    <style>
        body { font-family: Arial, sans-serif; margin: 20px; background: #1e1e1e; color: #d4d4d4; }
        h1 { color: #4ec9b0; }
        h2 { color: #569cd6; margin-top: 30px; }
        table { border-collapse: collapse; width: 100%; margin: 20px 0; }
        th, td { border: 1px solid #3e3e3e; padding: 12px; text-align: left; }
        th { background-color: #2d2d2d; color: #4ec9b0; }
        tr:nth-child(even) { background-color: #252526; }
        .success { color: #4ec9b0; font-weight: bold; }
        .failed { color: #f48771; font-weight: bold; }
        .warning { color: #dcdcaa; font-weight: bold; }
        .section { background: #252526; padding: 20px; margin: 20px 0; border-radius: 5px; }
    </style>
</head>
<body>
    <h1>HorrorProject Daily Build Report</h1>
    <p><strong>Date:</strong> $ReportDate</p>
    <p><strong>21-Day Sprint Progress</strong></p>

    <div class="section">
        <h2>Build Status</h2>
        <table>
            <tr><th>Metric</th><th>Value</th></tr>
            <tr><td>Status</td><td class="$(if($buildStatus.Status -eq 'Success'){'success'}else{'failed'})">$($buildStatus.Status)</td></tr>
            <tr><td>Builds Today</td><td>$($buildStatus.Count)</td></tr>
            <tr><td>Last Build</td><td>$($buildStatus.LastBuild)</td></tr>
        </table>
    </div>

    <div class="section">
        <h2>Test Status</h2>
        <table>
            <tr><th>Metric</th><th>Value</th></tr>
            <tr><td>Status</td><td class="$(if($testStatus.Status -eq 'Success'){'success'}else{'failed'})">$($testStatus.Status)</td></tr>
            <tr><td>Test Runs Today</td><td>$($testStatus.Count)</td></tr>
        </table>
    </div>

    <div class="section">
        <h2>Asset Validation</h2>
        <table>
            <tr><th>Metric</th><th>Value</th></tr>
            <tr><td>Status</td><td class="$(if($assetStatus.Issues -eq 0){'success'}else{'warning'})">$($assetStatus.Status)</td></tr>
            <tr><td>Issues Found</td><td>$($assetStatus.Issues)</td></tr>
        </table>
    </div>

    <div class="section">
        <h2>Project Statistics</h2>
        <table>
            <tr><th>Metric</th><th>Count</th></tr>
            <tr><td>Maps</td><td>$($projectStats.Maps)</td></tr>
            <tr><td>Assets</td><td>$($projectStats.Assets)</td></tr>
            <tr><td>C++ Source Files</td><td>$($projectStats.SourceFiles)</td></tr>
            <tr><td>Header Files</td><td>$($projectStats.HeaderFiles)</td></tr>
        </table>
    </div>

    <div class="section">
        <h2>Quick Actions</h2>
        <ul>
            <li>Run Build: <code>.\Scripts\Automation\Build.ps1</code></li>
            <li>Run Tests: <code>.\Scripts\Automation\Test.ps1 -QuickTest</code></li>
            <li>Validate Assets: <code>.\Scripts\Automation\ValidateAssets.ps1</code></li>
            <li>Package: <code>.\Scripts\Automation\Package.ps1</code></li>
        </ul>
    </div>

    <footer style="margin-top: 40px; padding-top: 20px; border-top: 1px solid #3e3e3e; color: #808080;">
        <p>Generated: $(Get-Date -Format 'yyyy-MM-dd HH:mm:ss')</p>
        <p>HorrorProject - 21-Day Sprint CI/CD System</p>
    </footer>
</body>
</html>
"@

    Set-Content -Path $ReportFile -Value $html
    Write-Host "Report generated: $ReportFile" -ForegroundColor Green
}

# Main execution
try {
    Write-Host "Generating daily build report..." -ForegroundColor Cyan

    New-HTMLReport

    # Open report in browser
    Start-Process $ReportFile

    Write-Host "Daily report complete!" -ForegroundColor Green
    exit 0
}
catch {
    Write-Host "Report generation failed: $_" -ForegroundColor Red
    exit 1
}
