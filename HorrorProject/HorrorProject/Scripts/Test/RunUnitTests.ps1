# Run Unit Tests Script for Horror Project

param(
    [string]$TestFilter = "",
    [switch]$Verbose = $false
)

$ErrorActionPreference = "Stop"

$ValidationCommon = Join-Path (Split-Path -Parent $PSScriptRoot) "Validation\Common.ps1"
. $ValidationCommon

$ProjectRoot = Get-HorrorProjectRoot -StartPath $PSScriptRoot
$ProjectFile = Get-HorrorProjectFile -ProjectRoot $ProjectRoot
$UE5Root = Get-HorrorUERoot
$EditorCmd = Get-HorrorEditorCmd -UERoot $UE5Root

Write-Host "=== Horror Project - Unit Tests ===" -ForegroundColor Cyan
Write-Host ""

$TestCommand = "Automation RunTests Unit"
if ($TestFilter) {
    $TestCommand += " $TestFilter"
}

$TestArgs = @(
    "`"$ProjectFile`"",
    "-ExecCmds=$TestCommand",
    "-TestExit=Automation Test Queue Empty",
    "-unattended",
    "-nopause",
    "-nosplash",
    "-nullrhi"
)

if ($Verbose) {
    $TestArgs += "-log"
}

Write-Host "Running unit tests..." -ForegroundColor Yellow
$StartTime = Get-Date

try {
    & $EditorCmd @TestArgs

    $Duration = (Get-Date) - $StartTime
    Write-Host ""
    Write-Host "Unit tests completed!" -ForegroundColor Green
    Write-Host "Duration: $($Duration.ToString('mm\:ss'))" -ForegroundColor Green

} catch {
    Write-Error "Unit tests failed: $_"
    exit 1
}
