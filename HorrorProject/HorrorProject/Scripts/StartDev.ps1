<#
.SYNOPSIS
    Quick-start development environment for HorrorProject.
    Sets up the environment and starts the continuous development pipeline.

.EXAMPLE
    .\StartDev.ps1
    .\StartDev.ps1 -SkipBuild
    .\StartDev.ps1 -WatchOnly
#>

param(
    [switch]$SkipBuild,
    [switch]$WatchOnly
)

$ProjectRoot = Split-Path -Parent $PSScriptRoot
$UERoot = $env:UE_5_6_ROOT
if (-not $UERoot) { $UERoot = "D:\UnrealEngine\UE_5.6" }

Write-Host "=====================================" -ForegroundColor Cyan
Write-Host "  HorrorProject - Dev Environment" -ForegroundColor Cyan
Write-Host "=====================================" -ForegroundColor Cyan
Write-Host ""

# Validate environment
Write-Host "[1/4] Validating environment..." -ForegroundColor Yellow

if (-not (Test-Path $UERoot)) {
    Write-Error "UE 5.6 not found at $UERoot"
    Write-Host "Set UE_5_6_ROOT environment variable or install to D:\UnrealEngine\UE_5.6"
    exit 1
}
Write-Host "  UE 5.6: $UERoot" -ForegroundColor Green

$uproject = "$ProjectRoot\HorrorProject.uproject"
if (-not (Test-Path $uproject)) {
    Write-Error "Project file not found at $uproject"
    exit 1
}
Write-Host "  Project: $uproject" -ForegroundColor Green

# Check git
$gitStatus = git -C $ProjectRoot status --porcelain 2>&1
if ($LASTEXITCODE -eq 0) {
    $changed = ($gitStatus | Measure-Object).Count
    Write-Host "  Git: OK ($changed changed files)" -ForegroundColor Green
} else {
    Write-Host "  Git: Not initialized" -ForegroundColor Yellow
}

# Source metrics
Write-Host ""
Write-Host "[2/4] Source metrics..." -ForegroundColor Yellow
$headers = Get-ChildItem -Path "$ProjectRoot\Source" -Recurse -Include *.h
$cpps = Get-ChildItem -Path "$ProjectRoot\Source" -Recurse -Include *.cpp
$lines = 0
foreach ($f in ($headers + $cpps)) {
    $lines += (Get-Content $f.FullName | Measure-Object -Line).Lines
}
Write-Host "  Headers: $($headers.Count)" -ForegroundColor White
Write-Host "  CPP files: $($cpps.Count)" -ForegroundColor White
Write-Host "  Total lines: $lines" -ForegroundColor White

# Build
if (-not $SkipBuild) {
    Write-Host ""
    Write-Host "[3/4] Building project..." -ForegroundColor Yellow

    $buildLog = "$ProjectRoot\Saved\Logs\DevStartup_$(Get-Date -Format 'yyyyMMdd_HHmmss').log"
    New-Item -ItemType Directory -Force -Path (Split-Path $buildLog) | Out-Null

    Write-Host "  Building Game module..."
    $result = & "$UERoot\Engine\Build\BatchFiles\Build.bat" `
        HorrorProject Win64 Development `
        -Project="$uproject" -WaitMutex -NoHotReload 2>&1
    $result | Out-File $buildLog

    if ($LASTEXITCODE -ne 0) {
        Write-Error "Build failed! Check log: $buildLog"
        exit 1
    }
    Write-Host "  Build: SUCCESS" -ForegroundColor Green
} else {
    Write-Host ""
    Write-Host "[3/4] Build skipped" -ForegroundColor Yellow
}

# Start pipeline
Write-Host ""
if ($WatchOnly) {
    Write-Host "[4/4] Starting watch mode..." -ForegroundColor Yellow
    & "$ProjectRoot\Scripts\DevPipeline.ps1" -Mode Watch
} else {
    Write-Host "[4/4] Running full pipeline..." -ForegroundColor Yellow
    & "$ProjectRoot\Scripts\DevPipeline.ps1" -Mode Full
}
