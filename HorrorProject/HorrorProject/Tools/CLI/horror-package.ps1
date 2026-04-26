#!/usr/bin/env pwsh
# Horror Project Package CLI Tool

param(
    [Parameter(Position=0)]
    [ValidateSet('debug', 'release', 'shipping')]
    [string]$PackageType = 'release',

    [Parameter()]
    [ValidateSet('Win64', 'Linux', 'Mac')]
    [string]$Platform = 'Win64',

    [Parameter()]
    [string]$OutputDir = "",

    [switch]$CreateInstaller,
    [switch]$Help
)

$ErrorActionPreference = "Stop"

function Show-Help {
    Write-Host @"
Horror Project Package Tool

USAGE:
    horror-package [PACKAGE_TYPE] [OPTIONS]

PACKAGE TYPES:
    debug       Package with debug symbols
    release     Package for release (default)
    shipping    Package for final shipping

OPTIONS:
    -Platform <platform>    Target platform (Win64|Linux|Mac)
    -OutputDir <path>       Custom output directory
    -CreateInstaller        Create installer package
    -Help                   Show this help message

EXAMPLES:
    horror-package release
    horror-package shipping -Platform Win64
    horror-package debug -OutputDir "C:\Builds"
    horror-package shipping -CreateInstaller

"@ -ForegroundColor Cyan
}

if ($Help) {
    Show-Help
    exit 0
}

$ScriptRoot = Split-Path -Parent $PSScriptRoot
$ProjectRoot = Split-Path -Parent $ScriptRoot
$ScriptsDir = Join-Path $ProjectRoot "Scripts\Package"

Write-Host "Horror Package Tool" -ForegroundColor Cyan
Write-Host "Package Type: $PackageType | Platform: $Platform" -ForegroundColor Yellow
Write-Host ""

try {
    $PackageArgs = @{
        Platform = $Platform
    }

    if ($OutputDir) {
        $PackageArgs.OutputDir = $OutputDir
    }

    switch ($PackageType) {
        'debug' {
            & "$ScriptsDir\PackageDebug.ps1" @PackageArgs
        }
        'release' {
            & "$ScriptsDir\PackageRelease.ps1" @PackageArgs
        }
        'shipping' {
            $PackageArgs.CreateInstaller = $CreateInstaller
            & "$ScriptsDir\PackageShipping.ps1" @PackageArgs
        }
    }

    if ($LASTEXITCODE -ne 0) {
        throw "Package failed with exit code $LASTEXITCODE"
    }

    Write-Host ""
    Write-Host "Package completed successfully!" -ForegroundColor Green

} catch {
    Write-Error "Package failed: $_"
    exit 1
}
