# Package Release Build for Horror Project

param(
    [string]$Platform = "Win64",
    [string]$OutputDir = ""
)

$ErrorActionPreference = "Stop"
$ProjectRoot = Split-Path -Parent (Split-Path -Parent $PSScriptRoot)
$ProjectFile = Join-Path $ProjectRoot "HorrorProject.uproject"
$UE5Root = if ($env:UE5_ROOT) { $env:UE5_ROOT } elseif ($env:UE_5_6_ROOT) { $env:UE_5_6_ROOT } elseif (Test-Path 'D:\UnrealEngine\UE_5.6') { 'D:\UnrealEngine\UE_5.6' } else { 'C:\Program Files\Epic Games\UE_5.6' }
$UE5Path = Join-Path $UE5Root "Engine\Build\BatchFiles"
if (-not $OutputDir) {
    $OutputDir = Join-Path $ProjectRoot "Packaged\Release"
}

Write-Host "=== Horror Project - Package Release ===" -ForegroundColor Cyan
Write-Host "Platform: $Platform"
Write-Host "Output: $OutputDir"
Write-Host ""

# Create output directory
if (-not (Test-Path $OutputDir)) {
    New-Item -ItemType Directory -Path $OutputDir | Out-Null
}

# Package command
$PackageScript = Join-Path $UE5Path "RunUAT.bat"
$PackageArgs = @(
    "BuildCookRun",
    "-project=`"$ProjectFile`"",
    "-platform=$Platform",
    "-clientconfig=Development",
    "-cook",
    "-stage",
    "-pak",
    "-archive",
    "-archivedirectory=`"$OutputDir`"",
    "-build",
    "-compressed",
    "-utf8output"
)

Write-Host "Starting release package..." -ForegroundColor Yellow
$StartTime = Get-Date

try {
    & $PackageScript @PackageArgs
    if ($LASTEXITCODE -ne 0) {
        throw "Package failed with exit code $LASTEXITCODE"
    }

    $Duration = (Get-Date) - $StartTime
    Write-Host ""
    Write-Host "Release package completed successfully!" -ForegroundColor Green
    Write-Host "Duration: $($Duration.ToString('hh\:mm\:ss'))" -ForegroundColor Green
    Write-Host "Output: $OutputDir" -ForegroundColor Green

} catch {
    Write-Error "Package failed: $_"
    exit 1
}
