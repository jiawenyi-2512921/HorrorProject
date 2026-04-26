# Log Viewer for HorrorProject
# Interactive log viewer with filtering and search

param(
    [Parameter(Mandatory=$false)]
    [string]$LogPath = "",

    [Parameter(Mandatory=$false)]
    [ValidateSet("All", "TRACE", "DEBUG", "INFO", "WARN", "ERROR", "FATAL")]
    [string]$MinLevel = "All",

    [Parameter(Mandatory=$false)]
    [string]$Category = "",

    [Parameter(Mandatory=$false)]
    [string]$Search = "",

    [Parameter(Mandatory=$false)]
    [int]$Tail = 0,

    [Parameter(Mandatory=$false)]
    [switch]$Follow
)

# Default paths
if ([string]::IsNullOrEmpty($LogPath)) {
    $LogPath = Join-Path $PSScriptRoot "..\..\..\..\Saved\Logs\StructuredLog.txt"
}

Write-Host "=== HorrorProject Log Viewer ===" -ForegroundColor Cyan
Write-Host "Log Path: $LogPath"
Write-Host ""

# Check if log file exists
if (-not (Test-Path $LogPath)) {
    Write-Host "ERROR: Log file not found at $LogPath" -ForegroundColor Red
    exit 1
}

# Level hierarchy for filtering
$levelHierarchy = @{
    "TRACE" = 0
    "DEBUG" = 1
    "INFO" = 2
    "WARN" = 3
    "ERROR" = 4
    "FATAL" = 5
}

$minLevelValue = if ($MinLevel -eq "All") { -1 } else { $levelHierarchy[$MinLevel] }

# Color mapping
function Get-LevelColor {
    param([string]$Level)

    switch ($Level) {
        "TRACE" { return "DarkGray" }
        "DEBUG" { return "Gray" }
        "INFO" { return "White" }
        "WARN" { return "Yellow" }
        "ERROR" { return "Red" }
        "FATAL" { return "Magenta" }
        default { return "White" }
    }
}

# Parse and display log entry
function Show-LogEntry {
    param([string]$Line)

    if ($Line -match '^\[(.*?)\] \[(.*?)\] \[(.*?)\] (.*)$') {
        $timestamp = $Matches[1]
        $level = $Matches[2]
        $category = $Matches[3]
        $message = $Matches[4]

        # Apply filters
        if ($minLevelValue -ge 0) {
            $currentLevelValue = $levelHierarchy[$level]
            if ($currentLevelValue -lt $minLevelValue) {
                return
            }
        }

        if (-not [string]::IsNullOrEmpty($Category) -and $category -ne $Category) {
            return
        }

        if (-not [string]::IsNullOrEmpty($Search) -and $message -notlike "*$Search*") {
            return
        }

        # Display with colors
        $color = Get-LevelColor $level
        Write-Host "[$timestamp] " -NoNewline -ForegroundColor DarkGray
        Write-Host "[$level] " -NoNewline -ForegroundColor $color
        Write-Host "[$category] " -NoNewline -ForegroundColor Cyan
        Write-Host $message -ForegroundColor White
    }
}

# Follow mode (tail -f)
if ($Follow) {
    Write-Host "Following log file (Ctrl+C to stop)..." -ForegroundColor Yellow
    Write-Host ""

    # Show initial tail
    if ($Tail -gt 0) {
        $lines = Get-Content $LogPath -Tail $Tail
        foreach ($line in $lines) {
            Show-LogEntry $line
        }
    }

    # Follow new entries
    Get-Content $LogPath -Wait -Tail 0 | ForEach-Object {
        Show-LogEntry $_
    }
}
# Normal mode
else {
    $lines = Get-Content $LogPath

    # Apply tail if specified
    if ($Tail -gt 0) {
        $lines = $lines | Select-Object -Last $Tail
    }

    $displayedCount = 0
    foreach ($line in $lines) {
        Show-LogEntry $line
        $displayedCount++
    }

    Write-Host ""
    Write-Host "Displayed $displayedCount entries" -ForegroundColor Green
}
