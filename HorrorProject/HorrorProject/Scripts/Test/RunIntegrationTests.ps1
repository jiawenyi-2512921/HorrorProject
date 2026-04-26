# Run Integration Tests Script for Horror Project

param(
    [string]$TestFilter = "",
    [switch]$Verbose = $false
)

$ErrorActionPreference = "Stop"
$ProjectRoot = Split-Path -Parent (Split-Path -Parent $PSScriptRoot)
$ProjectFile = Join-Path $ProjectRoot "HorrorProject.uproject"
$UE5Root = if ($env:UE5_ROOT) { $env:UE5_ROOT } elseif ($env:UE_5_6_ROOT) { $env:UE_5_6_ROOT } elseif (Test-Path 'D:\UnrealEngine\UE_5.6') { 'D:\UnrealEngine\UE_5.6' } else { 'C:\Program Files\Epic Games\UE_5.6' }
$UE5Path = Join-Path $UE5Root "Engine\Binaries\Win64"
$EditorCmd = Join-Path $UE5Path "UnrealEditor-Cmd.exe"

Write-Host "=== Horror Project - Integration Tests ===" -ForegroundColor Cyan
Write-Host ""

$TestCommand = "Automation RunTests Integration"
if ($TestFilter) {
    $TestCommand += " $TestFilter"
}

$TestArgs = @(
    "`"$ProjectFile`"",
    "-ExecCmds=$TestCommand",
    "-TestExit=Automation Test Queue Empty",
    "-unattended",
    "-nopause",
    "-nosplash"
)

if ($Verbose) {
    $TestArgs += "-log"
}

Write-Host "Running integration tests..." -ForegroundColor Yellow
$StartTime = Get-Date

try {
    & $EditorCmd @TestArgs

    $Duration = (Get-Date) - $StartTime
    Write-Host ""
    Write-Host "Integration tests completed!" -ForegroundColor Green
    Write-Host "Duration: $($Duration.ToString('mm\:ss'))" -ForegroundColor Green

} catch {
    Write-Error "Integration tests failed: $_"
    exit 1
}
