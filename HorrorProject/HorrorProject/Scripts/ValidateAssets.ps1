# HorrorProject Asset Validator
# Performs static asset checks that are safe to run in CI and daily automation.

param(
    [switch]$Fix = $false,
    [int]$LargeAssetWarningMB = 100
)

$ErrorActionPreference = "Stop"

. (Join-Path $PSScriptRoot "Validation\Common.ps1")

$ProjectRoot = Get-HorrorProjectRoot -StartPath $PSScriptRoot
$ContentDir = Join-Path $ProjectRoot "Content"
$ConfigDir = Join-Path $ProjectRoot "Config"

Write-Host "=== HorrorProject Asset Validator ===" -ForegroundColor Cyan
Write-Host "Project: $ProjectRoot" -ForegroundColor Yellow
Write-Host ""

$Errors = New-Object System.Collections.Generic.List[string]
$Warnings = New-Object System.Collections.Generic.List[string]

function Convert-GamePathToAssetFile {
    param(
        [Parameter(Mandatory = $true)]
        [string]$GamePath
    )

    if ($GamePath -notmatch '^/Game/') {
        return $null
    }

    $relativePath = $GamePath.Substring('/Game/'.Length).Replace('/', [System.IO.Path]::DirectorySeparatorChar)
    $assetPath = Join-Path $ContentDir $relativePath

    return @(
        "$assetPath.umap",
        "$assetPath.uasset"
    )
}

function Add-MapsToCookIssues {
    $gameIni = Join-Path $ConfigDir "DefaultGame.ini"
    if (-not (Test-Path -LiteralPath $gameIni)) {
        $Errors.Add("Missing config file: $gameIni")
        return
    }

    $mapsToCook = Select-String -LiteralPath $gameIni -Pattern '^\+MapsToCook=\(FilePath="([^"]+)"\)' |
        ForEach-Object { $_.Matches[0].Groups[1].Value } |
        Sort-Object -Unique

    if (-not $mapsToCook) {
        $Warnings.Add("No MapsToCook entries found in DefaultGame.ini")
        return
    }

    foreach ($mapPath in $mapsToCook) {
        $candidateFiles = Convert-GamePathToAssetFile -GamePath $mapPath
        if (-not $candidateFiles) {
            $Warnings.Add("MapsToCook entry is outside /Game and was not statically checked: $mapPath")
            continue
        }

        if (-not ($candidateFiles | Where-Object { Test-Path -LiteralPath $_ })) {
            $Errors.Add("MapsToCook entry does not exist on disk: $mapPath")
        }
    }
}

function Add-RequiredDirectoryIssues {
    $requiredDirs = @(
        (Join-Path $ContentDir "_SM13\Maps"),
        (Join-Path $ContentDir "_SM13\Audio"),
        (Join-Path $ContentDir "_SM13\UI"),
        (Join-Path $ContentDir "_SM13\Blueprints")
    )

    foreach ($dir in $requiredDirs) {
        if (-not (Test-Path -LiteralPath $dir)) {
            $Errors.Add("Missing required directory: $dir")
        }
    }
}

function Add-ZeroByteAssetIssues {
    $zeroByteAssets = Get-ChildItem -LiteralPath $ContentDir -Recurse -File -Include *.uasset, *.umap -ErrorAction SilentlyContinue |
        Where-Object { $_.Length -eq 0 }

    foreach ($asset in $zeroByteAssets) {
        $Errors.Add("Zero-byte asset: $($asset.FullName)")
    }
}

function Add-LargeAssetWarnings {
    $thresholdBytes = $LargeAssetWarningMB * 1MB
    $largeAssets = Get-ChildItem -LiteralPath $ContentDir -Recurse -File -Include *.uasset, *.umap -ErrorAction SilentlyContinue |
        Where-Object { $_.Length -gt $thresholdBytes } |
        Sort-Object Length -Descending |
        Select-Object -First 20

    foreach ($asset in $largeAssets) {
        $sizeMB = [math]::Round($asset.Length / 1MB, 2)
        $Warnings.Add("Large asset: $($asset.FullName) ($sizeMB MB)")
    }
}

function Invoke-AssetCleanup {
    if (-not $Fix) {
        return
    }

    Write-Host "Running safe cleanup..." -ForegroundColor Yellow
    $emptyDirs = Get-ChildItem -LiteralPath $ContentDir -Recurse -Directory -ErrorAction SilentlyContinue |
        Where-Object { -not (Get-ChildItem -LiteralPath $_.FullName -Force -ErrorAction SilentlyContinue) }

    foreach ($dir in $emptyDirs) {
        Write-Host "Removing empty directory: $($dir.FullName)" -ForegroundColor Gray
        Remove-Item -LiteralPath $dir.FullName -Force -ErrorAction SilentlyContinue
    }
}

try {
    Write-Host "[1/5] Checking content directory..." -ForegroundColor Yellow
    if (-not (Test-Path -LiteralPath $ContentDir)) {
        $Errors.Add("Missing content directory: $ContentDir")
    }

    Write-Host "[2/5] Checking MapsToCook entries..." -ForegroundColor Yellow
    Add-MapsToCookIssues

    Write-Host "[3/5] Checking required directories..." -ForegroundColor Yellow
    Add-RequiredDirectoryIssues

    Write-Host "[4/5] Checking zero-byte assets..." -ForegroundColor Yellow
    Add-ZeroByteAssetIssues

    Write-Host "[5/5] Checking large assets..." -ForegroundColor Yellow
    Add-LargeAssetWarnings

    Invoke-AssetCleanup

    Write-Host ""
    if ($Warnings.Count -gt 0) {
        Write-Host "Warnings: $($Warnings.Count)" -ForegroundColor Yellow
        foreach ($warning in $Warnings) {
            Write-Host "  - $warning" -ForegroundColor Yellow
        }
        Write-Host ""
    }

    if ($Errors.Count -gt 0) {
        Write-Host "Errors: $($Errors.Count)" -ForegroundColor Red
        foreach ($errorItem in $Errors) {
            Write-Host "  - $errorItem" -ForegroundColor Red
        }
        exit 1
    }

    Write-Host "Asset validation passed" -ForegroundColor Green
    exit 0
}
catch {
    Write-Host "Asset validation failed: $_" -ForegroundColor Red
    exit 1
}
