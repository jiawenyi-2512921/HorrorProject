# HorrorProject Asset Validator
# Validates all asset references before packaging

param(
    [switch]$Fix = $false
)

$ErrorActionPreference = "Stop"
$ProjectRoot = Split-Path -Parent $PSScriptRoot
$ContentDir = Join-Path $ProjectRoot "Content"

Write-Host "=== HorrorProject Asset Validator ===" -ForegroundColor Cyan
Write-Host ""

$Issues = @()

# Check 1: Missing asset references
Write-Host "[1/5] Checking for missing asset references..." -ForegroundColor Yellow
$MissingRefs = @(
    "/Game/FirstPerson/Anims/ABP_FP_Copy",
    "/Game/FirstPerson/MI_FirstPersonColorway"
)

foreach ($Ref in $MissingRefs) {
    $Path = $Ref -replace "/Game/", "$ContentDir\" -replace "/", "\"
    $Path += ".uasset"
    if (-not (Test-Path $Path)) {
        $Issues += "Missing reference: $Ref"
    }
}

# Check 2: Large texture files (>100MB)
Write-Host "[2/5] Checking for oversized textures..." -ForegroundColor Yellow
$LargeTextures = Get-ChildItem -Path $ContentDir -Recurse -Include "*.uasset" |
    Where-Object { $_.Length -gt 100MB } |
    Select-Object -First 10

if ($LargeTextures) {
    foreach ($Tex in $LargeTextures) {
        $SizeMB = [math]::Round($Tex.Length / 1MB, 2)
        $Issues += "Large texture: $($Tex.Name) ($SizeMB MB)"
    }
}

# Check 3: Unused demo content
Write-Host "[3/5] Checking for unused demo content..." -ForegroundColor Yellow
$DemoFolders = @(
    "$ContentDir\Bodycam_VHS_Effect\DEMO",
    "$ContentDir\FirstPerson"
)

foreach ($Folder in $DemoFolders) {
    if (Test-Path $Folder) {
        $Issues += "Demo content present: $Folder"
    }
}

# Check 4: MapsToCook configuration
Write-Host "[4/5] Checking MapsToCook configuration..." -ForegroundColor Yellow
$GameIni = Join-Path $ProjectRoot "Config\DefaultGame.ini"
$GameIniContent = Get-Content $GameIni -Raw

if ($GameIniContent -notmatch "SM13_Main") {
    $Issues += "SM13_Main not in MapsToCook list"
}

# Check 5: Required directories
Write-Host "[5/5] Checking required directories..." -ForegroundColor Yellow
$RequiredDirs = @(
    "$ContentDir\_SM13\Maps",
    "$ContentDir\_SM13\Audio",
    "$ContentDir\_SM13\UI",
    "$ContentDir\_SM13\Blueprints"
)

foreach ($Dir in $RequiredDirs) {
    if (-not (Test-Path $Dir)) {
        $Issues += "Missing directory: $Dir"
    }
}

# Report
Write-Host ""
if ($Issues.Count -eq 0) {
    Write-Host "✓ All checks passed!" -ForegroundColor Green
    exit 0
} else {
    Write-Host "✗ Found $($Issues.Count) issues:" -ForegroundColor Red
    foreach ($Issue in $Issues) {
        Write-Host "  - $Issue" -ForegroundColor Yellow
    }

    if ($Fix) {
        Write-Host ""
        Write-Host "Attempting automatic fixes..." -ForegroundColor Yellow
        # Add fix logic here
        Write-Host "Some issues require manual fixing in UE Editor" -ForegroundColor Yellow
    }

    exit 1
}
