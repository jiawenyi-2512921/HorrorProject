#!/usr/bin/env pwsh
# Horror Project Build CLI Tool

param(
    [Parameter(Position=0)]
    [ValidateSet('quick', 'full', 'incremental', 'clean')]
    [string]$BuildType = 'quick',

    [Parameter()]
    [ValidateSet('Debug', 'Development', 'Shipping')]
    [string]$Config = 'Development',

    [Parameter()]
    [ValidateSet('Win64', 'Linux', 'Mac')]
    [string]$Platform = 'Win64',

    [switch]$Verbose,
    [switch]$Help
)

$ErrorActionPreference = "Stop"

function Show-Help {
    Write-Host @"
Horror Project Build Tool

USAGE:
    horror-build [BUILD_TYPE] [OPTIONS]

BUILD TYPES:
    quick         Fast incremental build (default)
    full          Complete clean build
    incremental   Build only changed files
    clean         Clean all build artifacts

OPTIONS:
    -Config <config>      Build configuration (Debug|Development|Shipping)
    -Platform <platform>  Target platform (Win64|Linux|Mac)
    -Verbose             Show detailed output
    -Help                Show this help message

EXAMPLES:
    horror-build quick
    horror-build full -Config Shipping
    horror-build incremental -Verbose
    horror-build clean

"@ -ForegroundColor Cyan
}

if ($Help) {
    Show-Help
    exit 0
}

$ScriptRoot = Split-Path -Parent $PSScriptRoot
$ProjectRoot = Split-Path -Parent $ScriptRoot
$ScriptsDir = Join-Path $ProjectRoot "Scripts\Build"

Write-Host "Horror Build Tool" -ForegroundColor Cyan
Write-Host "Build Type: $BuildType | Config: $Config | Platform: $Platform" -ForegroundColor Yellow
Write-Host ""

try {
    switch ($BuildType) {
        'quick' {
            & "$ScriptsDir\QuickBuild.ps1" -Configuration $Config -Platform $Platform
        }
        'full' {
            & "$ScriptsDir\FullBuild.ps1" -Configuration $Config -Platform $Platform
        }
        'incremental' {
            & "$ScriptsDir\IncrementalBuild.ps1" -Configuration $Config -Platform $Platform -Verbose:$Verbose
        }
        'clean' {
            Write-Host "Cleaning build artifacts..." -ForegroundColor Yellow
            $IntermediateDir = Join-Path $ProjectRoot "Intermediate"
            $BinariesDir = Join-Path $ProjectRoot "Binaries"

            if (Test-Path $IntermediateDir) {
                Remove-Item -Path $IntermediateDir -Recurse -Force
                Write-Host "Cleaned: Intermediate" -ForegroundColor Green
            }
            if (Test-Path $BinariesDir) {
                Remove-Item -Path $BinariesDir -Recurse -Force
                Write-Host "Cleaned: Binaries" -ForegroundColor Green
            }
            Write-Host "Clean completed!" -ForegroundColor Green
        }
    }

    if ($LASTEXITCODE -ne 0) {
        throw "Build failed with exit code $LASTEXITCODE"
    }

    Write-Host ""
    Write-Host "Build completed successfully!" -ForegroundColor Green

} catch {
    Write-Error "Build failed: $_"
    exit 1
}
