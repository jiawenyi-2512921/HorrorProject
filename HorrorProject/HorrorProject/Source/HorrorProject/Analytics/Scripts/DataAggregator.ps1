# Data Aggregator - Aggregates analytics data from multiple sources
# Copyright Epic Games, Inc. All Rights Reserved.

param(
    [string]$InputPath = "",
    [string]$OutputPath = "",
    [switch]$IncludeTelemetry,
    [switch]$Verbose
)

$ErrorActionPreference = "Stop"

. (Join-Path $PSScriptRoot "..\..\..\..\Scripts\Validation\Common.ps1")
$ProjectRoot = Get-HorrorProjectRoot -StartPath $PSScriptRoot
if ([string]::IsNullOrWhiteSpace($InputPath)) { $InputPath = Join-Path $ProjectRoot "Saved\Analytics" }
if ([string]::IsNullOrWhiteSpace($OutputPath)) { $OutputPath = Join-Path $ProjectRoot "Saved\Analytics\Aggregated" }

function Write-Log {
    param([string]$Message, [string]$Level = "INFO")
    $timestamp = Get-Date -Format "yyyy-MM-dd HH:mm:ss"
    $logMessage = "[$timestamp] [$Level] $Message"
    Write-Host $logMessage
}

function Aggregate-Events {
    param([string]$EventsFile)

    Write-Log "Aggregating events from: $EventsFile"

    if (-not (Test-Path $EventsFile)) {
        Write-Log "Events file not found: $EventsFile" "WARNING"
        return @{}
    }

    $events = @{}
    $eventsByCategory = @{}
    $eventsBySession = @{}

    Get-Content $EventsFile | ForEach-Object {
        try {
            $event = $_ | ConvertFrom-Json

            # Count by event name
            if (-not $events.ContainsKey($event.event_name)) {
                $events[$event.event_name] = 0
            }
            $events[$event.event_name]++

            # Group by session
            if (-not $eventsBySession.ContainsKey($event.session_id)) {
                $eventsBySession[$event.session_id] = @()
            }
            $eventsBySession[$event.session_id] += $event

        } catch {
            Write-Log "Failed to parse event: $_" "ERROR"
        }
    }

    return @{
        EventCounts = $events
        EventsBySession = $eventsBySession
        TotalEvents = ($events.Values | Measure-Object -Sum).Sum
        UniqueEvents = $events.Count
        UniqueSessions = $eventsBySession.Count
    }
}

function Aggregate-Sessions {
    param([string]$SessionsFile)

    Write-Log "Aggregating sessions from: $SessionsFile"

    if (-not (Test-Path $SessionsFile)) {
        Write-Log "Sessions file not found: $SessionsFile" "WARNING"
        return @{}
    }

    $sessionsData = Get-Content $SessionsFile -Raw | ConvertFrom-Json
    $sessions = $sessionsData.sessions

    $totalDuration = 0
    $sessionCount = 0

    foreach ($session in $sessions) {
        $totalDuration += $session.duration
        $sessionCount++
    }

    $avgDuration = if ($sessionCount -gt 0) { $totalDuration / $sessionCount } else { 0 }

    return @{
        TotalSessions = $sessionCount
        TotalDuration = $totalDuration
        AverageDuration = $avgDuration
        Sessions = $sessions
    }
}

function Aggregate-Telemetry {
    param([string]$TelemetryPath)

    Write-Log "Aggregating telemetry from: $TelemetryPath"

    if (-not (Test-Path $TelemetryPath)) {
        Write-Log "Telemetry path not found: $TelemetryPath" "WARNING"
        return @{}
    }

    $telemetryFiles = Get-ChildItem -Path $TelemetryPath -Filter "report_*.json"

    $performanceMetrics = @{
        AvgFPS = @()
        MinFPS = @()
        MaxFPS = @()
        MemoryUsage = @()
    }

    $gameplayMetrics = @{
        TotalPlaytime = 0
        LevelsCompleted = 0
        Deaths = 0
        Achievements = 0
    }

    foreach ($file in $telemetryFiles) {
        try {
            $report = Get-Content $file.FullName -Raw | ConvertFrom-Json

            # Performance metrics
            if ($report.performance_metrics) {
                $performanceMetrics.AvgFPS += [float]$report.performance_metrics.avg_fps
                $performanceMetrics.MinFPS += [float]$report.performance_metrics.min_fps
                $performanceMetrics.MaxFPS += [float]$report.performance_metrics.max_fps
                $performanceMetrics.MemoryUsage += [float]$report.performance_metrics.memory_usage_mb
            }

            # Gameplay metrics
            if ($report.gameplay_metrics) {
                $gameplayMetrics.TotalPlaytime += [float]$report.gameplay_metrics.total_playtime
                $gameplayMetrics.LevelsCompleted += [int]$report.gameplay_metrics.levels_completed
                $gameplayMetrics.Deaths += [int]$report.gameplay_metrics.deaths
                $gameplayMetrics.Achievements += [int]$report.gameplay_metrics.achievements_unlocked
            }

        } catch {
            Write-Log "Failed to process telemetry file $($file.Name): $_" "ERROR"
        }
    }

    return @{
        PerformanceMetrics = @{
            AvgFPS = ($performanceMetrics.AvgFPS | Measure-Object -Average).Average
            MinFPS = ($performanceMetrics.MinFPS | Measure-Object -Minimum).Minimum
            MaxFPS = ($performanceMetrics.MaxFPS | Measure-Object -Maximum).Maximum
            AvgMemoryUsage = ($performanceMetrics.MemoryUsage | Measure-Object -Average).Average
        }
        GameplayMetrics = $gameplayMetrics
        ReportCount = $telemetryFiles.Count
    }
}

# Main execution
Write-Log "Starting data aggregation..."

# Create output directory
if (-not (Test-Path $OutputPath)) {
    New-Item -ItemType Directory -Path $OutputPath -Force | Out-Null
}

# Aggregate events
$eventsFile = Join-Path $InputPath "events.jsonl"
$eventsAggregation = Aggregate-Events -EventsFile $eventsFile

# Aggregate sessions
$sessionsFile = Join-Path $InputPath "sessions.json"
$sessionsAggregation = Aggregate-Sessions -SessionsFile $sessionsFile

# Aggregate telemetry (if requested)
$telemetryAggregation = @{}
if ($IncludeTelemetry) {
    $telemetryPath = Join-Path $InputPath "Telemetry"
    $telemetryAggregation = Aggregate-Telemetry -TelemetryPath $telemetryPath
}

# Create aggregated report
$aggregatedData = @{
    GeneratedAt = (Get-Date).ToString("o")
    Events = $eventsAggregation
    Sessions = $sessionsAggregation
    Telemetry = $telemetryAggregation
}

# Save aggregated data
$outputFile = Join-Path $OutputPath "aggregated_$(Get-Date -Format 'yyyyMMdd_HHmmss').json"
$aggregatedData | ConvertTo-Json -Depth 10 | Set-Content -Path $outputFile -Encoding UTF8

Write-Log "Aggregation complete. Output saved to: $outputFile"
Write-Log "Total events: $($eventsAggregation.TotalEvents)"
Write-Log "Total sessions: $($sessionsAggregation.TotalSessions)"

if ($Verbose) {
    Write-Log "Aggregated data:" "INFO"
    $aggregatedData | ConvertTo-Json -Depth 5 | Write-Host
}
