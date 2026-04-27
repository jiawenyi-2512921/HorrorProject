# Integration Test Runner
# Automates execution of HorrorProject integration tests

param(
    [string]$ProjectPath = "",
    [string]$EngineDir = "",
    [string]$TestFilter = "HorrorProject.Integration",
    [string]$OutputDir = "",
    [switch]$GenerateReport,
    [switch]$Verbose,
    [int]$Timeout = 3600
)

$ErrorActionPreference = "Stop"

. (Join-Path $PSScriptRoot "..\..\..\..\Scripts\Validation\Common.ps1")
$ProjectRoot = Get-HorrorProjectRoot -StartPath $PSScriptRoot
if ([string]::IsNullOrWhiteSpace($ProjectPath)) { $ProjectPath = Get-HorrorProjectFile -ProjectRoot $ProjectRoot }
if ([string]::IsNullOrWhiteSpace($OutputDir)) { $OutputDir = Join-Path $ProjectRoot "Saved\Automation\Reports" }

$UE5Root = if ($env:UE5_ROOT) { $env:UE5_ROOT } elseif ($env:UE_5_6_ROOT) { $env:UE_5_6_ROOT } elseif (Test-Path 'D:\UnrealEngine\UE_5.6') { 'D:\UnrealEngine\UE_5.6' } else { 'C:\Program Files\Epic Games\UE_5.6' }
if (-not $EngineDir) {
    $EngineDir = Join-Path $UE5Root "Engine"
}

# Configuration
$EditorExe = Join-Path $EngineDir "Binaries\Win64\UnrealEditor-Cmd.exe"
$ReportFile = Join-Path $OutputDir "IntegrationTestResults_$(Get-Date -Format 'yyyyMMdd_HHmmss').json"
$LogFile = Join-Path $OutputDir "IntegrationTestLog_$(Get-Date -Format 'yyyyMMdd_HHmmss').log"

# Ensure output directory exists
if (-not (Test-Path $OutputDir)) {
    New-Item -ItemType Directory -Path $OutputDir -Force | Out-Null
}

Write-Host "=== HorrorProject Integration Test Runner ===" -ForegroundColor Cyan
Write-Host "Project: $ProjectPath"
Write-Host "Test Filter: $TestFilter"
Write-Host "Output: $OutputDir"
Write-Host ""

# Validate paths
if (-not (Test-Path $ProjectPath)) {
    Write-Error "Project file not found: $ProjectPath"
    exit 1
}

if (-not (Test-Path $EditorExe)) {
    Write-Error "Unreal Editor not found: $EditorExe"
    exit 1
}

# Build test command
$TestCommand = "Automation RunTests $TestFilter"
if ($GenerateReport) {
    $TestCommand += "; Automation Report"
}

$Arguments = @(
    "`"$ProjectPath`"",
    "-ExecCmds=`"$TestCommand`"",
    "-unattended",
    "-nopause",
    "-NullRHI",
    "-testexit=`"Automation Test Queue Empty`"",
    "-log",
    "-log=`"$LogFile`""
)

if ($Verbose) {
    $Arguments += "-verbose"
}

Write-Host "Starting test execution..." -ForegroundColor Yellow
Write-Host "Command: $EditorExe $($Arguments -join ' ')" -ForegroundColor Gray
Write-Host ""

# Execute tests
$StartTime = Get-Date
$Process = Start-Process -FilePath $EditorExe -ArgumentList $Arguments -PassThru -NoNewWindow

# Wait for completion with timeout
$Completed = $Process.WaitForExit($Timeout * 1000)

if (-not $Completed) {
    Write-Host "Test execution timed out after $Timeout seconds" -ForegroundColor Red
    $Process.Kill()
    exit 2
}

$EndTime = Get-Date
$Duration = $EndTime - $StartTime
$ExitCode = $Process.ExitCode

Write-Host ""
Write-Host "=== Test Execution Complete ===" -ForegroundColor Cyan
Write-Host "Duration: $($Duration.ToString('hh\:mm\:ss'))"
Write-Host "Exit Code: $ExitCode"

# Parse test results from log
if (Test-Path $LogFile) {
    Write-Host ""
    Write-Host "=== Test Results Summary ===" -ForegroundColor Cyan

    $LogContent = Get-Content $LogFile -Raw

    # Extract test statistics
    if ($LogContent -match "Success: (\d+)") {
        $SuccessCount = $Matches[1]
        Write-Host "Passed: $SuccessCount" -ForegroundColor Green
    }

    if ($LogContent -match "Fail: (\d+)") {
        $FailCount = $Matches[1]
        Write-Host "Failed: $FailCount" -ForegroundColor Red
    }

    if ($LogContent -match "Warning: (\d+)") {
        $WarningCount = $Matches[1]
        Write-Host "Warnings: $WarningCount" -ForegroundColor Yellow
    }

    # Extract failed tests
    $FailedTests = [regex]::Matches($LogContent, "LogAutomationController: Error: (.+?) - (.+)")
    if ($FailedTests.Count -gt 0) {
        Write-Host ""
        Write-Host "Failed Tests:" -ForegroundColor Red
        foreach ($Match in $FailedTests) {
            Write-Host "  - $($Match.Groups[1].Value): $($Match.Groups[2].Value)" -ForegroundColor Red
        }
    }

    # Extract performance metrics
    $PerfMetrics = [regex]::Matches($LogContent, "LogAutomationController: Display: (.+?) - (.+?) ms")
    if ($PerfMetrics.Count -gt 0) {
        Write-Host ""
        Write-Host "Performance Metrics:" -ForegroundColor Cyan
        foreach ($Match in $PerfMetrics) {
            Write-Host "  - $($Match.Groups[1].Value): $($Match.Groups[2].Value) ms"
        }
    }
}

# Generate JSON report
if ($GenerateReport) {
    Write-Host ""
    Write-Host "Generating test report..." -ForegroundColor Yellow

    $Report = @{
        Timestamp = $StartTime.ToString("o")
        Duration = $Duration.TotalSeconds
        ExitCode = $ExitCode
        TestFilter = $TestFilter
        Results = @{
            Passed = if ($SuccessCount) { [int]$SuccessCount } else { 0 }
            Failed = if ($FailCount) { [int]$FailCount } else { 0 }
            Warnings = if ($WarningCount) { [int]$WarningCount } else { 0 }
        }
        LogFile = $LogFile
    }

    $Report | ConvertTo-Json -Depth 10 | Out-File $ReportFile -Encoding UTF8
    Write-Host "Report saved: $ReportFile" -ForegroundColor Green
}

Write-Host ""
Write-Host "Log file: $LogFile" -ForegroundColor Gray

# Exit with appropriate code
if ($ExitCode -eq 0) {
    Write-Host ""
    Write-Host "All tests passed!" -ForegroundColor Green
    exit 0
} else {
    Write-Host ""
    Write-Host "Tests failed or encountered errors" -ForegroundColor Red
    exit $ExitCode
}
