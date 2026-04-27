# Package Debug Build for Horror Project

param(
    [string]$Platform = "Win64",
    [string]$OutputDir = ""
)

$ErrorActionPreference = "Stop"

$ValidationCommon = Join-Path (Split-Path -Parent $PSScriptRoot) "Validation\Common.ps1"
. $ValidationCommon

$ProjectRoot = Get-HorrorProjectRoot -StartPath $PSScriptRoot
$ProjectFile = Get-HorrorProjectFile -ProjectRoot $ProjectRoot
$UE5Root = Get-HorrorUERoot
$PackageScript = Get-HorrorBuildScript -UERoot $UE5Root -ScriptName "RunUAT.bat"
if (-not $OutputDir) {
    $OutputDir = Join-Path $ProjectRoot "Packaged\Debug"
}

Write-Host "=== Horror Project - Package Debug ===" -ForegroundColor Cyan
Write-Host "Platform: $Platform"
Write-Host "Output: $OutputDir"
Write-Host ""

# Create output directory
if (-not (Test-Path $OutputDir)) {
    New-Item -ItemType Directory -Path $OutputDir | Out-Null
}

$PackageArgs = @(
    "BuildCookRun",
    "-project=`"$ProjectFile`"",
    "-platform=$Platform",
    "-clientconfig=Debug",
    "-cook",
    "-stage",
    "-pak",
    "-archive",
    "-archivedirectory=`"$OutputDir`"",
    "-build",
    "-utf8output"
)

Write-Host "Starting debug package..." -ForegroundColor Yellow
$StartTime = Get-Date

try {
    & $PackageScript @PackageArgs
    if ($LASTEXITCODE -ne 0) {
        throw "Package failed with exit code $LASTEXITCODE"
    }

    $Duration = (Get-Date) - $StartTime
    Write-Host ""
    Write-Host "Debug package completed successfully!" -ForegroundColor Green
    Write-Host "Duration: $($Duration.ToString('hh\:mm\:ss'))" -ForegroundColor Green
    Write-Host "Output: $OutputDir" -ForegroundColor Green

} catch {
    Write-Error "Package failed: $_"
    exit 1
}
