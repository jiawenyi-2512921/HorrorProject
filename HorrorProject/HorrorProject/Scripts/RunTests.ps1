# HorrorProject Test Runner
# Runs all automated tests and generates report

param(
    [string]$Filter = "",
    [switch]$Verbose = $false
)

$ErrorActionPreference = "Stop"
$ProjectRoot = Split-Path -Parent $PSScriptRoot
$ProjectFile = Join-Path $ProjectRoot "HorrorProject.uproject"
$UE5Root = if ($env:UE5_ROOT) { $env:UE5_ROOT } elseif ($env:UE_5_6_ROOT) { $env:UE_5_6_ROOT } elseif (Test-Path 'D:\UnrealEngine\UE_5.6') { 'D:\UnrealEngine\UE_5.6' } else { 'C:\Program Files\Epic Games\UE_5.6' }
$UE5Path = $UE5Root
$Editor = Join-Path $UE5Root "Engine\Binaries\Win64\UnrealEditor-Cmd.exe"
$ReportDir = Join-Path $ProjectRoot "Saved\Automation"

Write-Host "=== HorrorProject Test Runner ===" -ForegroundColor Cyan
Write-Host "Filter: $(if ($Filter) { $Filter } else { 'All tests' })"
Write-Host ""

# Create report directory
New-Item -ItemType Directory -Force -Path $ReportDir | Out-Null

# Build test command
$TestCmd = "Automation RunTests $(if ($Filter) { $Filter } else { 'HorrorProject' })"
if ($Verbose) {
    $TestCmd += " -Verbose"
}
$TestCmd += " -ReportOutputPath=`"$ReportDir`""

Write-Host "Running tests..." -ForegroundColor Yellow
& $Editor "$ProjectFile" -ExecCmds="$TestCmd" -Unattended -NullRHI -NoSplash -Log

# Parse results
$ResultFile = Get-ChildItem -Path $ReportDir -Filter "*.json" | Sort-Object LastWriteTime -Descending | Select-Object -First 1

if ($ResultFile) {
    $Results = Get-Content $ResultFile.FullName | ConvertFrom-Json
    $Passed = ($Results.tests | Where-Object { $_.state -eq "Success" }).Count
    $Failed = ($Results.tests | Where-Object { $_.state -eq "Fail" }).Count
    $Total = $Results.tests.Count

    Write-Host ""
    Write-Host "=== Test Results ===" -ForegroundColor Cyan
    Write-Host "Total: $Total" -ForegroundColor White
    Write-Host "Passed: $Passed" -ForegroundColor Green
    Write-Host "Failed: $Failed" -ForegroundColor $(if ($Failed -gt 0) { "Red" } else { "Green" })
    Write-Host ""
    Write-Host "Report: $($ResultFile.FullName)" -ForegroundColor Gray

    if ($Failed -gt 0) {
        Write-Host ""
        Write-Host "Failed tests:" -ForegroundColor Red
        $Results.tests | Where-Object { $_.state -eq "Fail" } | ForEach-Object {
            Write-Host "  - $($_.testDisplayName)" -ForegroundColor Red
        }
        exit 1
    }
} else {
    Write-Host "No test results found!" -ForegroundColor Red
    exit 1
}

Write-Host ""
Write-Host "All tests passed!" -ForegroundColor Green
