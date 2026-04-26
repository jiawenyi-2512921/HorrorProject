# Crash Report Analyzer for HorrorProject
# Analyzes Unreal Engine crash reports and generates diagnostic information

param(
    [Parameter(Mandatory=$false)]
    [string]$CrashReportPath = "",

    [Parameter(Mandatory=$false)]
    [string]$OutputPath = "",

    [Parameter(Mandatory=$false)]
    [switch]$Detailed
)

# Default paths
if ([string]::IsNullOrEmpty($CrashReportPath)) {
    $savedDir = Join-Path $PSScriptRoot "..\..\..\..\Saved"
    $crashesDir = Join-Path $savedDir "Crashes"

    if (Test-Path $crashesDir) {
        # Find most recent crash
        $crashes = Get-ChildItem $crashesDir -Directory | Sort-Object LastWriteTime -Descending
        if ($crashes.Count -gt 0) {
            $CrashReportPath = $crashes[0].FullName
        }
    }
}

if ([string]::IsNullOrEmpty($OutputPath)) {
    $timestamp = Get-Date -Format "yyyyMMdd_HHmmss"
    $OutputPath = Join-Path $PSScriptRoot "..\..\..\..\Saved\Diagnostics\CrashAnalysis_$timestamp.txt"
}

Write-Host "=== HorrorProject Crash Report Analyzer ===" -ForegroundColor Cyan
Write-Host "Crash Report Path: $CrashReportPath"
Write-Host "Output Path: $OutputPath"
Write-Host ""

# Check if crash report exists
if ([string]::IsNullOrEmpty($CrashReportPath) -or -not (Test-Path $CrashReportPath)) {
    Write-Host "ERROR: No crash report found" -ForegroundColor Red
    Write-Host "Please specify a crash report path with -CrashReportPath" -ForegroundColor Yellow
    exit 1
}

# Initialize report
$report = @()
$report += "=== CRASH REPORT ANALYSIS ==="
$report += "Generated: $(Get-Date -Format 'yyyy-MM-dd HH:mm:ss')"
$report += "Crash Report: $CrashReportPath"
$report += ""

# Parse crash context
$crashContextPath = Join-Path $CrashReportPath "CrashContext.runtime-xml"
if (Test-Path $crashContextPath) {
    Write-Host "Parsing crash context..." -ForegroundColor Yellow

    [xml]$crashContext = Get-Content $crashContextPath

    $report += "=== CRASH INFORMATION ==="
    $report += "Crash GUID: $($crashContext.FGenericCrashContext.CrashGUID)"
    $report += "Crash Type: $($crashContext.FGenericCrashContext.CrashType)"
    $report += "Error Message: $($crashContext.FGenericCrashContext.ErrorMessage)"
    $report += "Crash Time: $($crashContext.FGenericCrashContext.TimeOfCrash)"
    $report += ""

    $report += "=== ENGINE INFORMATION ==="
    $report += "Engine Version: $($crashContext.FGenericCrashContext.EngineVersion)"
    $report += "Engine Mode: $($crashContext.FGenericCrashContext.EngineMode)"
    $report += "Build Configuration: $($crashContext.FGenericCrashContext.BuildConfiguration)"
    $report += "Platform: $($crashContext.FGenericCrashContext.PlatformName)"
    $report += ""

    $report += "=== SYSTEM INFORMATION ==="
    $report += "OS Version: $($crashContext.FGenericCrashContext.PlatformNameIni)"
    $report += "CPU: $($crashContext.FGenericCrashContext.CPUBrand)"
    $report += "GPU: $($crashContext.FGenericCrashContext.PrimaryGPUBrand)"
    $report += "Memory: $($crashContext.FGenericCrashContext.MemoryStats.TotalPhysical) bytes"
    $report += ""
}

# Parse call stack
$logPath = Join-Path $CrashReportPath "*.log"
$logFiles = Get-ChildItem $logPath -ErrorAction SilentlyContinue

if ($logFiles.Count -gt 0) {
    Write-Host "Analyzing crash log..." -ForegroundColor Yellow

    $logContent = Get-Content $logFiles[0].FullName

    # Extract call stack
    $inCallStack = $false
    $callStack = @()

    foreach ($line in $logContent) {
        if ($line -match "Call stack:") {
            $inCallStack = $true
            continue
        }

        if ($inCallStack) {
            if ($line -match "^\s*$" -or $line -match "^Log file:") {
                break
            }
            $callStack += $line
        }
    }

    if ($callStack.Count -gt 0) {
        $report += "=== CALL STACK ==="
        foreach ($frame in $callStack) {
            $report += $frame
        }
        $report += ""
    }

    # Extract assertions and errors
    $errors = $logContent | Where-Object { $_ -match "Error:|Assertion failed:|Fatal error:" }
    if ($errors.Count -gt 0) {
        $report += "=== ERRORS AND ASSERTIONS ==="
        foreach ($error in $errors) {
            $report += $error
        }
        $report += ""
    }

    # Extract warnings before crash
    $warnings = $logContent | Where-Object { $_ -match "Warning:" } | Select-Object -Last 10
    if ($warnings.Count -gt 0) {
        $report += "=== RECENT WARNINGS (Last 10) ==="
        foreach ($warning in $warnings) {
            $report += $warning
        }
        $report += ""
    }
}

# Analyze crash patterns
$report += "=== CRASH ANALYSIS ==="

$crashType = ""
if ($crashContext) {
    $errorMsg = $crashContext.FGenericCrashContext.ErrorMessage
    $crashType = $crashContext.FGenericCrashContext.CrashType

    # Common crash patterns
    if ($errorMsg -match "Access violation|nullptr|null pointer") {
        $report += "DIAGNOSIS: Null Pointer Dereference"
        $report += "CAUSE: Attempting to access memory through a null pointer"
        $report += "SOLUTION: Check for null pointer checks before dereferencing"
        $report += ""
    }
    elseif ($errorMsg -match "Stack overflow") {
        $report += "DIAGNOSIS: Stack Overflow"
        $report += "CAUSE: Infinite recursion or excessive stack allocation"
        $report += "SOLUTION: Check for recursive function calls or large stack allocations"
        $report += ""
    }
    elseif ($errorMsg -match "Out of memory|OOM") {
        $report += "DIAGNOSIS: Out of Memory"
        $report += "CAUSE: Memory exhaustion"
        $report += "SOLUTION: Check for memory leaks, reduce memory usage, or increase available memory"
        $report += ""
    }
    elseif ($errorMsg -match "Assertion failed") {
        $report += "DIAGNOSIS: Assertion Failure"
        $report += "CAUSE: Runtime assertion check failed"
        $report += "SOLUTION: Review the assertion condition and fix the underlying issue"
        $report += ""
    }
    elseif ($errorMsg -match "Pure virtual function") {
        $report += "DIAGNOSIS: Pure Virtual Function Call"
        $report += "CAUSE: Calling pure virtual function on abstract class"
        $report += "SOLUTION: Ensure proper object initialization and inheritance"
        $report += ""
    }
    else {
        $report += "DIAGNOSIS: General Crash"
        $report += "CAUSE: See error message and call stack for details"
        $report += ""
    }
}

# Recommendations
$report += "=== RECOMMENDATIONS ==="
$report += "1. Review the call stack to identify the crash location"
$report += "2. Check recent code changes related to the crash"
$report += "3. Enable additional logging in the crash area"
$report += "4. Run with debug build for more detailed information"
$report += "5. Check for similar crashes in crash reporting system"
$report += "6. Review memory usage and potential leaks"
$report += "7. Verify all pointers are valid before dereferencing"
$report += "8. Check for race conditions in multithreaded code"
$report += ""

# Detailed analysis
if ($Detailed) {
    $report += "=== DETAILED LOG ANALYSIS ==="

    if ($logFiles.Count -gt 0) {
        $logContent = Get-Content $logFiles[0].FullName

        # Extract initialization
        $initLines = $logContent | Select-Object -First 50
        $report += "Initialization (First 50 lines):"
        foreach ($line in $initLines) {
            $report += $line
        }
        $report += ""

        # Extract last lines before crash
        $lastLines = $logContent | Select-Object -Last 100
        $report += "Last 100 lines before crash:"
        foreach ($line in $lastLines) {
            $report += $line
        }
        $report += ""
    }
}

# Save report
$reportDir = Split-Path $OutputPath -Parent
if (-not (Test-Path $reportDir)) {
    New-Item -ItemType Directory -Path $reportDir -Force | Out-Null
}

$report | Out-File -FilePath $OutputPath -Encoding UTF8

Write-Host "Analysis complete!" -ForegroundColor Green
Write-Host "Report saved to: $OutputPath" -ForegroundColor Cyan
Write-Host ""

# Display summary
if ($crashContext) {
    Write-Host "=== CRASH SUMMARY ===" -ForegroundColor Yellow
    Write-Host "Type: $($crashContext.FGenericCrashContext.CrashType)" -ForegroundColor Red
    Write-Host "Error: $($crashContext.FGenericCrashContext.ErrorMessage)" -ForegroundColor Red
    Write-Host "Time: $($crashContext.FGenericCrashContext.TimeOfCrash)" -ForegroundColor Cyan
}
