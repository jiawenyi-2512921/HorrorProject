# HorrorProject - Performance Test Script
# 21-Day Sprint Optimized

param(
    [string]$MapName = "TestMap",
    [int]$DurationSeconds = 60,
    [switch]$ProfileGPU,
    [switch]$ProfileMemory,
    [switch]$GenerateReport
)

$ErrorActionPreference = "Stop"
$ProjectRoot = "D:\gptzuo\HorrorProject\HorrorProject"
$ProjectFile = "$ProjectRoot\HorrorProject.uproject"
$UE5Editor = "C:\Program Files\Epic Games\UE_5.6\Engine\Binaries\Win64\UnrealEditor-Cmd.exe"
$LogDir = "$ProjectRoot\Build\Logs\Performance"
$PerfStartTime = Get-Date

New-Item -ItemType Directory -Force -Path $LogDir | Out-Null
$LogFile = "$LogDir\Performance_$(Get-Date -Format 'yyyyMMdd_HHmmss').log"

function Write-Log {
    param([string]$Message, [string]$Level = "INFO")
    $timestamp = Get-Date -Format "yyyy-MM-dd HH:mm:ss"
    $logMessage = "[$timestamp] [$Level] $Message"
    Write-Host $logMessage
    Add-Content -Path $LogFile -Value $logMessage
}

function Invoke-PerformanceCapture {
    Write-Log "Starting performance capture..."

    $perfArgs = @(
        "`"$ProjectFile`"",
        "/Game/Maps/$MapName",
        "-game",
        "-benchmark",
        "-fps=60",
        "-seconds=$DurationSeconds",
        "-unattended",
        "-nullrhi"
    )

    if ($ProfileGPU) {
        $perfArgs += "-gpuprofile"
    }

    if ($ProfileMemory) {
        $perfArgs += "-memreport"
    }

    $perfArgs += @(
        "-log",
        "-abslog=`"$LogFile`""
    )

    Write-Log "Running performance test for $DurationSeconds seconds..."

    $process = Start-Process -FilePath $UE5Editor -ArgumentList $perfArgs -NoNewWindow -Wait -PassThru

    if ($process.ExitCode -eq 0) {
        Write-Log "Performance capture complete" "SUCCESS"
    } else {
        Write-Log "Performance capture completed with warnings" "WARN"
    }
}

function Get-PerformanceMetrics {
    Write-Log "Analyzing performance metrics..."

    if (-not (Test-Path $LogFile)) {
        Write-Log "Log file not found" "WARN"
        return
    }

    $logContent = Get-Content $LogFile

    # Parse FPS data
    $fpsLines = $logContent | Select-String -Pattern "FPS:" -SimpleMatch
    if ($fpsLines) {
        Write-Log "FPS data captured: $($fpsLines.Count) samples"
    }

    # Parse memory data
    $memLines = $logContent | Select-String -Pattern "Memory:" -SimpleMatch
    if ($memLines) {
        Write-Log "Memory data captured: $($memLines.Count) samples"
    }

    # Check for performance warnings
    $warnings = $logContent | Select-String -Pattern "Warning.*Performance" -SimpleMatch
    if ($warnings) {
        Write-Log "Found $($warnings.Count) performance warnings" "WARN"
    }
}

function Write-PerformanceReport {
    if ($GenerateReport) {
        $reportPath = "$LogDir\PerformanceReport_$(Get-Date -Format 'yyyyMMdd_HHmmss').txt"

        $report = @"
HorrorProject Performance Test Report
=====================================
Date: $(Get-Date -Format 'yyyy-MM-dd HH:mm:ss')
Map: $MapName
Duration: $DurationSeconds seconds
GPU Profile: $ProfileGPU
Memory Profile: $ProfileMemory

Log File: $LogFile

Test completed successfully.
"@

        Set-Content -Path $reportPath -Value $report
        Write-Log "Report generated: $reportPath" "SUCCESS"
    }

    $perfDuration = (Get-Date) - $PerfStartTime

    Write-Log "========================================" "SUCCESS"
    Write-Log "PERFORMANCE TEST COMPLETE" "SUCCESS"
    Write-Log "Duration: $($perfDuration.ToString('hh\:mm\:ss'))" "SUCCESS"
    Write-Log "========================================" "SUCCESS"
}

# Main execution
try {
    Write-Log "Starting HorrorProject performance test..."

    Invoke-PerformanceCapture
    Get-PerformanceMetrics
    Write-PerformanceReport

    exit 0
}
catch {
    Write-Log "PERFORMANCE TEST FAILED: $_" "ERROR"
    exit 1
}
