# HorrorProject - Automated Test Script
# 21-Day Sprint Optimized

param(
    [string]$TestFilter = "",
    [switch]$RunAll,
    [switch]$QuickTest,
    [switch]$GenerateReport
)

$ErrorActionPreference = "Stop"
$ProjectRoot = "D:\gptzuo\HorrorProject\HorrorProject"
$ProjectFile = "$ProjectRoot\HorrorProject.uproject"
$UE5Editor = "C:\Program Files\Epic Games\UE_5.6\Engine\Binaries\Win64\UnrealEditor-Cmd.exe"
$LogDir = "$ProjectRoot\Build\Logs\Tests"
$TestStartTime = Get-Date

New-Item -ItemType Directory -Force -Path $LogDir | Out-Null
$LogFile = "$LogDir\Test_$(Get-Date -Format 'yyyyMMdd_HHmmss').log"

function Write-Log {
    param([string]$Message, [string]$Level = "INFO")
    $timestamp = Get-Date -Format "yyyy-MM-dd HH:mm:ss"
    $logMessage = "[$timestamp] [$Level] $Message"
    Write-Host $logMessage
    Add-Content -Path $LogFile -Value $logMessage
}

function Invoke-AutomationTests {
    Write-Log "Running automation tests..."

    $testArgs = @(
        "`"$ProjectFile`"",
        "-ExecCmds=Automation RunTests Now"
    )

    if ($TestFilter) {
        $testArgs += "-TestFilter=$TestFilter"
    } elseif ($QuickTest) {
        $testArgs += "-TestFilter=System.Core"
    } elseif ($RunAll) {
        $testArgs += "-TestFilter=Project"
    }

    $testArgs += @(
        "-unattended",
        "-nopause",
        "-nosplash",
        "-nullrhi",
        "-log",
        "-ReportOutputPath=`"$LogDir`""
    )

    Write-Log "Running: UnrealEditor-Cmd.exe $($testArgs -join ' ')"

    $process = Start-Process -FilePath $UE5Editor -ArgumentList $testArgs -NoNewWindow -Wait -PassThru

    $exitCode = $process.ExitCode

    if ($exitCode -eq 0) {
        Write-Log "All tests passed" "SUCCESS"
        return $true
    } else {
        Write-Log "Tests failed with exit code: $exitCode" "ERROR"
        return $false
    }
}

function Get-TestResults {
    Write-Log "Parsing test results..."

    $reportFiles = Get-ChildItem -Path $LogDir -Filter "*.json" -ErrorAction SilentlyContinue | Sort-Object LastWriteTime -Descending

    if ($reportFiles.Count -eq 0) {
        Write-Log "No test reports found" "WARN"
        return
    }

    $latestReport = $reportFiles[0]
    Write-Log "Latest report: $($latestReport.FullName)"

    try {
        $reportData = Get-Content $latestReport.FullName | ConvertFrom-Json

        $totalTests = $reportData.TotalTests
        $passedTests = $reportData.SucceededTests
        $failedTests = $reportData.FailedTests

        Write-Log "Total: $totalTests, Passed: $passedTests, Failed: $failedTests" "SUCCESS"
    }
    catch {
        Write-Log "Could not parse test report: $_" "WARN"
    }
}

function Write-TestReport {
    $testDuration = (Get-Date) - $TestStartTime

    Write-Log "========================================" "SUCCESS"
    Write-Log "TEST RUN COMPLETE" "SUCCESS"
    Write-Log "Duration: $($testDuration.ToString('hh\:mm\:ss'))" "SUCCESS"
    Write-Log "Log: $LogFile" "SUCCESS"
    Write-Log "========================================" "SUCCESS"
}

# Main execution
try {
    Write-Log "Starting HorrorProject tests..."

    $testsPassed = Invoke-AutomationTests
    Get-TestResults
    Write-TestReport

    if ($testsPassed) {
        exit 0
    } else {
        exit 1
    }
}
catch {
    Write-Log "TEST RUN FAILED: $_" "ERROR"
    exit 1
}
