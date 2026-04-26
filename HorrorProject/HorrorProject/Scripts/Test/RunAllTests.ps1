# Run All Tests Script for Horror Project

param(
    [string]$Configuration = "Development",
    [string]$Platform = "Win64",
    [switch]$GenerateReport = $true
)

$ErrorActionPreference = "Stop"
$ProjectRoot = Split-Path -Parent (Split-Path -Parent $PSScriptRoot)
$ProjectFile = Join-Path $ProjectRoot "HorrorProject.uproject"
$UE5Root = if ($env:UE5_ROOT) { $env:UE5_ROOT } elseif ($env:UE_5_6_ROOT) { $env:UE_5_6_ROOT } elseif (Test-Path 'D:\UnrealEngine\UE_5.6') { 'D:\UnrealEngine\UE_5.6' } else { 'C:\Program Files\Epic Games\UE_5.6' }
$UE5Path = Join-Path $UE5Root "Engine\Binaries\Win64"
$EditorCmd = Join-Path $UE5Path "UnrealEditor-Cmd.exe"

Write-Host "=== Horror Project - Run All Tests ===" -ForegroundColor Cyan
Write-Host ""

# Test categories
$TestCategories = @(
    "Unit",
    "Integration",
    "Functional",
    "Performance"
)

$TotalTests = 0
$PassedTests = 0
$FailedTests = 0
$StartTime = Get-Date

foreach ($Category in $TestCategories) {
    Write-Host "Running $Category tests..." -ForegroundColor Yellow

    $TestArgs = @(
        "`"$ProjectFile`"",
        "-ExecCmds=Automation RunTests $Category",
        "-TestExit=Automation Test Queue Empty",
        "-unattended",
        "-nopause",
        "-nosplash",
        "-nullrhi",
        "-log"
    )

    try {
        $Output = & $EditorCmd @TestArgs 2>&1

        # Parse results
        $CategoryPassed = ($Output | Select-String "Tests Passed: (\d+)" | ForEach-Object { $_.Matches.Groups[1].Value }) -as [int]
        $CategoryFailed = ($Output | Select-String "Tests Failed: (\d+)" | ForEach-Object { $_.Matches.Groups[1].Value }) -as [int]

        $TotalTests += $CategoryPassed + $CategoryFailed
        $PassedTests += $CategoryPassed
        $FailedTests += $CategoryFailed

        Write-Host "  Passed: $CategoryPassed, Failed: $CategoryFailed" -ForegroundColor $(if ($CategoryFailed -eq 0) { "Green" } else { "Red" })

    } catch {
        Write-Warning "Failed to run $Category tests: $_"
    }
}

$Duration = (Get-Date) - $StartTime

Write-Host ""
Write-Host "=== Test Summary ===" -ForegroundColor Cyan
Write-Host "Total Tests: $TotalTests"
Write-Host "Passed: $PassedTests" -ForegroundColor Green
Write-Host "Failed: $FailedTests" -ForegroundColor $(if ($FailedTests -eq 0) { "Green" } else { "Red" })
Write-Host "Duration: $($Duration.ToString('mm\:ss'))"

# Generate report
if ($GenerateReport) {
    $ReportDir = Join-Path $ProjectRoot "TestReports"
    if (-not (Test-Path $ReportDir)) {
        New-Item -ItemType Directory -Path $ReportDir | Out-Null
    }

    $ReportFile = Join-Path $ReportDir "TestReport_$(Get-Date -Format 'yyyyMMdd_HHmmss').json"

    $Report = @{
        Timestamp = Get-Date -Format "o"
        TotalTests = $TotalTests
        PassedTests = $PassedTests
        FailedTests = $FailedTests
        Duration = $Duration.TotalSeconds
        Configuration = $Configuration
        Platform = $Platform
    } | ConvertTo-Json

    $Report | Out-File -FilePath $ReportFile -Encoding UTF8
    Write-Host "Test report saved: $ReportFile" -ForegroundColor Green
}

if ($FailedTests -gt 0) {
    exit 1
}
