# HorrorProject Build Script
# Automates compilation and validation

param(
    [string]$Configuration = "Development",
    [switch]$RunTests = $false,
    [switch]$Package = $false
)

$ErrorActionPreference = "Stop"
$ProjectRoot = Split-Path -Parent $PSScriptRoot
$ProjectFile = Join-Path $ProjectRoot "HorrorProject.uproject"
$UE5Root = if ($env:UE5_ROOT) { $env:UE5_ROOT } elseif ($env:UE_5_6_ROOT) { $env:UE_5_6_ROOT } elseif (Test-Path 'D:\UnrealEngine\UE_5.6') { 'D:\UnrealEngine\UE_5.6' } else { 'C:\Program Files\Epic Games\UE_5.6' }
$UE5Path = $UE5Root
$UAT = Join-Path $UE5Path "Engine\Build\BatchFiles\RunUAT.bat"
$UBT = Join-Path $UE5Path "Engine\Build\BatchFiles\Build.bat"

Write-Host "=== HorrorProject Build Script ===" -ForegroundColor Cyan
Write-Host "Configuration: $Configuration"
Write-Host "Run Tests: $RunTests"
Write-Host "Package: $Package"
Write-Host ""

# Step 1: Compile C++ code
Write-Host "[1/4] Compiling C++ code..." -ForegroundColor Yellow
& $UBT HorrorProjectEditor Win64 $Configuration -Project="$ProjectFile" -WaitMutex -FromMsBuild
if ($LASTEXITCODE -ne 0) {
    Write-Host "Build failed!" -ForegroundColor Red
    exit 1
}
Write-Host "Build successful!" -ForegroundColor Green

# Step 2: Run tests (if requested)
if ($RunTests) {
    Write-Host "[2/4] Running automated tests..." -ForegroundColor Yellow
    & $UAT BuildCookRun -project="$ProjectFile" -platform=Win64 -clientconfig=$Configuration -RunAutomationTests -ReportOutputPath="$ProjectRoot\Saved\Automation"
    if ($LASTEXITCODE -ne 0) {
        Write-Host "Tests failed!" -ForegroundColor Red
        exit 1
    }
    Write-Host "Tests passed!" -ForegroundColor Green
} else {
    Write-Host "[2/4] Skipping tests" -ForegroundColor Gray
}

# Step 3: Package (if requested)
if ($Package) {
    Write-Host "[3/4] Packaging project..." -ForegroundColor Yellow
    $ArchiveDir = Join-Path $ProjectRoot "..\Builds\Windows\$Configuration"
    & $UAT BuildCookRun -project="$ProjectFile" -platform=Win64 -clientconfig=$Configuration -cook -stage -pak -archive -archivedirectory="$ArchiveDir"
    if ($LASTEXITCODE -ne 0) {
        Write-Host "Packaging failed!" -ForegroundColor Red
        exit 1
    }
    Write-Host "Package created at: $ArchiveDir" -ForegroundColor Green
} else {
    Write-Host "[3/4] Skipping packaging" -ForegroundColor Gray
}

# Step 4: Summary
Write-Host "[4/4] Build complete!" -ForegroundColor Green
Write-Host ""
Write-Host "Next steps:" -ForegroundColor Cyan
Write-Host "  - Open project in UE Editor"
Write-Host "  - Run 'stat fps' in-game to check performance"
Write-Host "  - Test playthrough from start to end"
