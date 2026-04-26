# Daily Build Automation for Horror Project

param(
    [string]$Configuration = "Development",
    [string]$Platform = "Win64"
)

$ErrorActionPreference = "Stop"
$ProjectRoot = Split-Path -Parent (Split-Path -Parent $PSScriptRoot)
$ScriptsDir = Join-Path $ProjectRoot "Scripts"

Write-Host "=== Horror Project - Daily Build ===" -ForegroundColor Cyan
Write-Host "Started: $(Get-Date -Format 'yyyy-MM-dd HH:mm:ss')"
Write-Host ""

$StartTime = Get-Date
$BuildLog = Join-Path $ProjectRoot "Logs\DailyBuild_$(Get-Date -Format 'yyyyMMdd').log"

# Create log directory
$LogDir = Split-Path $BuildLog
if (-not (Test-Path $LogDir)) {
    New-Item -ItemType Directory -Path $LogDir | Out-Null
}

function Write-Log {
    param([string]$Message, [string]$Level = "INFO")
    $Timestamp = Get-Date -Format "yyyy-MM-dd HH:mm:ss"
    $LogMessage = "[$Timestamp] [$Level] $Message"
    Write-Host $LogMessage
    Add-Content -Path $BuildLog -Value $LogMessage
}

try {
    # Step 1: Clean build
    Write-Log "Step 1: Full Build" "INFO"
    & "$ScriptsDir\Build\FullBuild.ps1" -Configuration $Configuration -Platform $Platform
    if ($LASTEXITCODE -ne 0) { throw "Build failed" }
    Write-Log "Build completed successfully" "SUCCESS"

    # Step 2: Run all tests
    Write-Log "Step 2: Run All Tests" "INFO"
    & "$ScriptsDir\Test\RunAllTests.ps1" -Configuration $Configuration -Platform $Platform
    if ($LASTEXITCODE -ne 0) { throw "Tests failed" }
    Write-Log "All tests passed" "SUCCESS"

    # Step 3: Validate assets
    Write-Log "Step 3: Asset Validation" "INFO"
    & "$ScriptsDir\ValidateAssets.ps1"
    Write-Log "Asset validation completed" "SUCCESS"

    # Step 4: Package
    Write-Log "Step 4: Package Build" "INFO"
    & "$ScriptsDir\Package\PackageRelease.ps1" -Platform $Platform
    if ($LASTEXITCODE -ne 0) { throw "Package failed" }
    Write-Log "Package completed successfully" "SUCCESS"

    $Duration = (Get-Date) - $StartTime
    Write-Log "Daily build completed successfully in $($Duration.ToString('hh\:mm\:ss'))" "SUCCESS"

    # Send success notification
    Write-Host ""
    Write-Host "Daily build completed successfully!" -ForegroundColor Green

} catch {
    Write-Log "Daily build failed: $_" "ERROR"
    Write-Error "Daily build failed: $_"
    exit 1
}
