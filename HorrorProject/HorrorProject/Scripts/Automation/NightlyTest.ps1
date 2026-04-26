# Nightly Test Automation for Horror Project

param(
    [string]$Configuration = "Development",
    [string]$Platform = "Win64"
)

$ErrorActionPreference = "Stop"
$ProjectRoot = Split-Path -Parent (Split-Path -Parent $PSScriptRoot)
$ScriptsDir = Join-Path $ProjectRoot "Scripts"

Write-Host "=== Horror Project - Nightly Test ===" -ForegroundColor Cyan
Write-Host "Started: $(Get-Date -Format 'yyyy-MM-dd HH:mm:ss')"
Write-Host ""

$StartTime = Get-Date
$TestLog = Join-Path $ProjectRoot "Logs\NightlyTest_$(Get-Date -Format 'yyyyMMdd').log"

# Create log directory
$LogDir = Split-Path $TestLog
if (-not (Test-Path $LogDir)) {
    New-Item -ItemType Directory -Path $LogDir | Out-Null
}

function Write-Log {
    param([string]$Message, [string]$Level = "INFO")
    $Timestamp = Get-Date -Format "yyyy-MM-dd HH:mm:ss"
    $LogMessage = "[$Timestamp] [$Level] $Message"
    Write-Host $LogMessage
    Add-Content -Path $TestLog -Value $LogMessage
}

$TestResults = @{
    UnitTests = $false
    IntegrationTests = $false
    FunctionalTests = $false
    PerformanceTests = $false
}

try {
    # Unit Tests
    Write-Log "Running Unit Tests..." "INFO"
    try {
        & "$ScriptsDir\Test\RunUnitTests.ps1"
        $TestResults.UnitTests = $true
        Write-Log "Unit tests passed" "SUCCESS"
    } catch {
        Write-Log "Unit tests failed: $_" "ERROR"
    }

    # Integration Tests
    Write-Log "Running Integration Tests..." "INFO"
    try {
        & "$ScriptsDir\Test\RunIntegrationTests.ps1"
        $TestResults.IntegrationTests = $true
        Write-Log "Integration tests passed" "SUCCESS"
    } catch {
        Write-Log "Integration tests failed: $_" "ERROR"
    }

    # Performance Tests
    Write-Log "Running Performance Tests..." "INFO"
    try {
        & "$ScriptsDir\Performance\RunPerformanceTests.ps1"
        $TestResults.PerformanceTests = $true
        Write-Log "Performance tests passed" "SUCCESS"
    } catch {
        Write-Log "Performance tests failed: $_" "ERROR"
    }

    # Generate summary
    $Duration = (Get-Date) - $StartTime
    $PassedCount = ($TestResults.Values | Where-Object { $_ -eq $true }).Count
    $TotalCount = $TestResults.Count

    Write-Log "Nightly test completed in $($Duration.ToString('hh\:mm\:ss'))" "INFO"
    Write-Log "Results: $PassedCount/$TotalCount test suites passed" "INFO"

    Write-Host ""
    Write-Host "=== Test Summary ===" -ForegroundColor Cyan
    foreach ($Test in $TestResults.GetEnumerator()) {
        $Status = if ($Test.Value) { "PASSED" } else { "FAILED" }
        $Color = if ($Test.Value) { "Green" } else { "Red" }
        Write-Host "$($Test.Key): $Status" -ForegroundColor $Color
    }

    if ($PassedCount -lt $TotalCount) {
        exit 1
    }

} catch {
    Write-Log "Nightly test failed: $_" "ERROR"
    Write-Error "Nightly test failed: $_"
    exit 1
}
