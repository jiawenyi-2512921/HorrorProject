# HorrorProject Test Runner
# Runs all automated tests and generates report

param(
    [string]$Filter = "",
    [switch]$Verbose = $false
)

$ErrorActionPreference = "Stop"

$ValidationCommon = Join-Path $PSScriptRoot "Validation\Common.ps1"
. $ValidationCommon

$ProjectRoot = Get-HorrorProjectRoot -StartPath $PSScriptRoot
$ProjectFile = Get-HorrorProjectFile -ProjectRoot $ProjectRoot
$UE5Root = Get-HorrorUERoot
$Editor = Get-HorrorEditorCmd -UERoot $UE5Root
$ReportRoot = Join-Path $ProjectRoot "Saved\Automation\Reports"
$ReportDir = Join-Path $ReportRoot ("RunTests_{0}" -f (Get-Date -Format "yyyyMMdd_HHmmss"))

Write-Host "=== HorrorProject Test Runner ===" -ForegroundColor Cyan
Write-Host "Filter: $(if ($Filter) { $Filter } else { 'All tests' })"
Write-Host ""

New-Item -ItemType Directory -Force -Path $ReportDir | Out-Null

# Build test command
$TestCmd = "Automation RunTests $(if ($Filter) { $Filter } else { 'HorrorProject' })"
if ($Verbose) {
    $TestCmd += " -Verbose"
}

Write-Host "Running tests..." -ForegroundColor Yellow
$TestArgs = @(
    $ProjectFile,
    "-ExecCmds=$TestCmd",
    "-TestExit=Automation Test Queue Empty",
    "-Unattended",
    "-NullRHI",
    "-NoSplash",
    "-NoSound",
    "-Log",
    "-ReportOutputPath=$ReportDir"
)

& $Editor @TestArgs
$EditorExitCode = $LASTEXITCODE
if ($EditorExitCode -ne 0) {
    Write-Host "UnrealEditor-Cmd exited with code $EditorExitCode" -ForegroundColor Red
    exit $EditorExitCode
}

# Parse results
$ResultFile = Get-ChildItem -Path $ReportDir -Filter "index.json" -Recurse -ErrorAction SilentlyContinue |
    Sort-Object LastWriteTime -Descending |
    Select-Object -First 1

if ($ResultFile) {
    $Results = Get-Content $ResultFile.FullName | ConvertFrom-Json
    $Passed = [int]$Results.succeeded + [int]$Results.succeededWithWarnings
    $Failed = [int]$Results.failed
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
    Write-Host "Expected report under: $ReportDir" -ForegroundColor Gray
    exit 1
}

Write-Host ""
Write-Host "All tests passed!" -ForegroundColor Green
