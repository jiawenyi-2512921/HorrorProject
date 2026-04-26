# Log Analyzer for HorrorProject
# Analyzes structured logs and generates reports

param(
    [Parameter(Mandatory=$false)]
    [string]$LogPath = "",

    [Parameter(Mandatory=$false)]
    [string]$OutputPath = "",

    [Parameter(Mandatory=$false)]
    [ValidateSet("All", "Errors", "Warnings", "Performance", "Memory")]
    [string]$AnalysisType = "All",

    [Parameter(Mandatory=$false)]
    [int]$TopCount = 10
)

# Default paths
if ([string]::IsNullOrEmpty($LogPath)) {
    $LogPath = Join-Path $PSScriptRoot "..\..\..\..\Saved\Logs\StructuredLog.txt"
}

if ([string]::IsNullOrEmpty($OutputPath)) {
    $timestamp = Get-Date -Format "yyyyMMdd_HHmmss"
    $OutputPath = Join-Path $PSScriptRoot "..\..\..\..\Saved\Diagnostics\LogAnalysis_$timestamp.txt"
}

Write-Host "=== HorrorProject Log Analyzer ===" -ForegroundColor Cyan
Write-Host "Log Path: $LogPath"
Write-Host "Output Path: $OutputPath"
Write-Host "Analysis Type: $AnalysisType"
Write-Host ""

# Check if log file exists
if (-not (Test-Path $LogPath)) {
    Write-Host "ERROR: Log file not found at $LogPath" -ForegroundColor Red
    exit 1
}

# Read log file
Write-Host "Reading log file..." -ForegroundColor Yellow
$logContent = Get-Content $LogPath

# Parse log entries
$logEntries = @()
foreach ($line in $logContent) {
    if ($line -match '^\[(.*?)\] \[(.*?)\] \[(.*?)\] (.*)$') {
        $entry = [PSCustomObject]@{
            Timestamp = [DateTime]::Parse($Matches[1])
            Level = $Matches[2]
            Category = $Matches[3]
            Message = $Matches[4]
        }
        $logEntries += $entry
    }
}

Write-Host "Parsed $($logEntries.Count) log entries" -ForegroundColor Green
Write-Host ""

# Initialize report
$report = @()
$report += "=== LOG ANALYSIS REPORT ==="
$report += "Generated: $(Get-Date -Format 'yyyy-MM-dd HH:mm:ss')"
$report += "Log File: $LogPath"
$report += "Total Entries: $($logEntries.Count)"
$report += ""

# Analysis functions
function Analyze-Errors {
    $errors = $logEntries | Where-Object { $_.Level -eq "ERROR" -or $_.Level -eq "FATAL" }

    $report += "=== ERROR ANALYSIS ==="
    $report += "Total Errors: $($errors.Count)"
    $report += ""

    if ($errors.Count -gt 0) {
        # Group by category
        $errorsByCategory = $errors | Group-Object Category | Sort-Object Count -Descending

        $report += "Errors by Category:"
        foreach ($group in $errorsByCategory) {
            $report += "  $($group.Name): $($group.Count)"
        }
        $report += ""

        # Recent errors
        $report += "Recent Errors (Last $TopCount):"
        $recentErrors = $errors | Sort-Object Timestamp -Descending | Select-Object -First $TopCount
        foreach ($error in $recentErrors) {
            $report += "  [$($error.Timestamp.ToString('HH:mm:ss'))] [$($error.Category)] $($error.Message)"
        }
        $report += ""
    }
}

function Analyze-Warnings {
    $warnings = $logEntries | Where-Object { $_.Level -eq "WARN" }

    $report += "=== WARNING ANALYSIS ==="
    $report += "Total Warnings: $($warnings.Count)"
    $report += ""

    if ($warnings.Count -gt 0) {
        # Group by category
        $warningsByCategory = $warnings | Group-Object Category | Sort-Object Count -Descending

        $report += "Warnings by Category:"
        foreach ($group in $warningsByCategory) {
            $report += "  $($group.Name): $($group.Count)"
        }
        $report += ""

        # Most common warnings
        $commonWarnings = $warnings | Group-Object Message | Sort-Object Count -Descending | Select-Object -First $TopCount
        $report += "Most Common Warnings:"
        foreach ($warning in $commonWarnings) {
            $report += "  [$($warning.Count)x] $($warning.Name)"
        }
        $report += ""
    }
}

function Analyze-Performance {
    $perfEntries = $logEntries | Where-Object { $_.Category -eq "Performance" }

    $report += "=== PERFORMANCE ANALYSIS ==="
    $report += "Total Performance Entries: $($perfEntries.Count)"
    $report += ""

    if ($perfEntries.Count -gt 0) {
        # FPS issues
        $fpsIssues = $perfEntries | Where-Object { $_.Message -like "*Low FPS*" -or $_.Message -like "*High frame time*" }
        $report += "FPS Issues: $($fpsIssues.Count)"

        # Thread issues
        $threadIssues = $perfEntries | Where-Object { $_.Message -like "*thread time*" }
        $report += "Thread Issues: $($threadIssues.Count)"

        # GPU issues
        $gpuIssues = $perfEntries | Where-Object { $_.Message -like "*GPU*" }
        $report += "GPU Issues: $($gpuIssues.Count)"
        $report += ""
    }
}

function Analyze-Memory {
    $memEntries = $logEntries | Where-Object { $_.Category -eq "Memory" }

    $report += "=== MEMORY ANALYSIS ==="
    $report += "Total Memory Entries: $($memEntries.Count)"
    $report += ""

    if ($memEntries.Count -gt 0) {
        # Memory warnings
        $memWarnings = $memEntries | Where-Object { $_.Level -eq "WARN" -or $_.Level -eq "ERROR" }
        $report += "Memory Warnings/Errors: $($memWarnings.Count)"

        # Leak detection
        $leakWarnings = $memEntries | Where-Object { $_.Message -like "*leak*" }
        $report += "Potential Memory Leaks: $($leakWarnings.Count)"

        if ($leakWarnings.Count -gt 0) {
            $report += ""
            $report += "Memory Leak Details:"
            foreach ($leak in $leakWarnings) {
                $report += "  [$($leak.Timestamp.ToString('HH:mm:ss'))] $($leak.Message)"
            }
        }
        $report += ""
    }
}

function Analyze-Timeline {
    $report += "=== TIMELINE ANALYSIS ==="

    if ($logEntries.Count -gt 0) {
        $firstEntry = $logEntries | Sort-Object Timestamp | Select-Object -First 1
        $lastEntry = $logEntries | Sort-Object Timestamp | Select-Object -Last 1

        $duration = $lastEntry.Timestamp - $firstEntry.Timestamp

        $report += "First Entry: $($firstEntry.Timestamp.ToString('yyyy-MM-dd HH:mm:ss'))"
        $report += "Last Entry: $($lastEntry.Timestamp.ToString('yyyy-MM-dd HH:mm:ss'))"
        $report += "Duration: $($duration.ToString())"
        $report += ""

        # Entries per minute
        if ($duration.TotalMinutes -gt 0) {
            $entriesPerMinute = $logEntries.Count / $duration.TotalMinutes
            $report += "Average Entries per Minute: $([math]::Round($entriesPerMinute, 2))"
        }
        $report += ""
    }
}

function Analyze-Categories {
    $report += "=== CATEGORY BREAKDOWN ==="

    $categories = $logEntries | Group-Object Category | Sort-Object Count -Descending

    foreach ($category in $categories) {
        $report += "$($category.Name): $($category.Count) entries"

        # Level breakdown for this category
        $levels = $category.Group | Group-Object Level
        $levelStr = ($levels | ForEach-Object { "$($_.Name):$($_.Count)" }) -join ", "
        $report += "  Levels: $levelStr"
    }
    $report += ""
}

# Run analyses based on type
switch ($AnalysisType) {
    "All" {
        Analyze-Timeline
        Analyze-Categories
        Analyze-Errors
        Analyze-Warnings
        Analyze-Performance
        Analyze-Memory
    }
    "Errors" {
        Analyze-Errors
    }
    "Warnings" {
        Analyze-Warnings
    }
    "Performance" {
        Analyze-Performance
    }
    "Memory" {
        Analyze-Memory
    }
}

# Summary statistics
$report += "=== SUMMARY STATISTICS ==="
$levelCounts = $logEntries | Group-Object Level
foreach ($level in $levelCounts) {
    $report += "$($level.Name): $($level.Count)"
}
$report += ""

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
Write-Host "=== QUICK SUMMARY ===" -ForegroundColor Yellow
$errors = ($logEntries | Where-Object { $_.Level -eq "ERROR" -or $_.Level -eq "FATAL" }).Count
$warnings = ($logEntries | Where-Object { $_.Level -eq "WARN" }).Count
Write-Host "Errors: $errors" -ForegroundColor $(if ($errors -gt 0) { "Red" } else { "Green" })
Write-Host "Warnings: $warnings" -ForegroundColor $(if ($warnings -gt 0) { "Yellow" } else { "Green" })
Write-Host "Info: $(($logEntries | Where-Object { $_.Level -eq "INFO" }).Count)" -ForegroundColor Cyan
