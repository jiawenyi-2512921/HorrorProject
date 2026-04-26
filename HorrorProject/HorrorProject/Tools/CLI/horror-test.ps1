#!/usr/bin/env pwsh
# Horror Project Test CLI Tool

param(
    [Parameter(Position=0)]
    [ValidateSet('all', 'unit', 'integration', 'functional', 'performance')]
    [string]$TestType = 'all',

    [Parameter()]
    [string]$Filter = "",

    [switch]$Verbose,
    [switch]$GenerateReport,
    [switch]$Help
)

$ErrorActionPreference = "Stop"

function Show-Help {
    Write-Host @"
Horror Project Test Tool

USAGE:
    horror-test [TEST_TYPE] [OPTIONS]

TEST TYPES:
    all           Run all tests (default)
    unit          Run unit tests only
    integration   Run integration tests only
    functional    Run functional tests only
    performance   Run performance tests only

OPTIONS:
    -Filter <pattern>    Filter tests by name pattern
    -Verbose            Show detailed test output
    -GenerateReport     Generate HTML test report
    -Help               Show this help message

EXAMPLES:
    horror-test all
    horror-test unit -Verbose
    horror-test integration -Filter "AI*"
    horror-test performance -GenerateReport

"@ -ForegroundColor Cyan
}

if ($Help) {
    Show-Help
    exit 0
}

$ScriptRoot = Split-Path -Parent $PSScriptRoot
$ProjectRoot = Split-Path -Parent $ScriptRoot
$ScriptsDir = Join-Path $ProjectRoot "Scripts\Test"

Write-Host "Horror Test Tool" -ForegroundColor Cyan
Write-Host "Test Type: $TestType" -ForegroundColor Yellow
if ($Filter) {
    Write-Host "Filter: $Filter" -ForegroundColor Yellow
}
Write-Host ""

try {
    switch ($TestType) {
        'all' {
            & "$ScriptsDir\RunAllTests.ps1" -GenerateReport:$GenerateReport
        }
        'unit' {
            & "$ScriptsDir\RunUnitTests.ps1" -TestFilter $Filter -Verbose:$Verbose
        }
        'integration' {
            & "$ScriptsDir\RunIntegrationTests.ps1" -TestFilter $Filter -Verbose:$Verbose
        }
        'functional' {
            Write-Host "Running functional tests..." -ForegroundColor Yellow
            # Placeholder for functional tests
        }
        'performance' {
            $PerfScript = Join-Path $ProjectRoot "Scripts\Performance\RunPerformanceTests.ps1"
            if (Test-Path $PerfScript) {
                & $PerfScript
            } else {
                Write-Warning "Performance test script not found"
            }
        }
    }

    if ($LASTEXITCODE -ne 0) {
        throw "Tests failed with exit code $LASTEXITCODE"
    }

    Write-Host ""
    Write-Host "Tests completed successfully!" -ForegroundColor Green

} catch {
    Write-Error "Tests failed: $_"
    exit 1
}
