# Data Analyzer - Analyzes aggregated analytics data
# Copyright Epic Games, Inc. All Rights Reserved.

param(
    [string]$InputPath = "",
    [string]$OutputPath = "",
    [switch]$GenerateCharts,
    [switch]$Verbose
)

$ErrorActionPreference = "Stop"

. (Join-Path $PSScriptRoot "..\..\..\..\Scripts\Validation\Common.ps1")
$ProjectRoot = Get-HorrorProjectRoot -StartPath $PSScriptRoot
if ([string]::IsNullOrWhiteSpace($InputPath)) { $InputPath = Join-Path $ProjectRoot "Saved\Analytics\Aggregated" }
if ([string]::IsNullOrWhiteSpace($OutputPath)) { $OutputPath = Join-Path $ProjectRoot "Saved\Analytics\Analysis" }

function Write-Log {
    param([string]$Message, [string]$Level = "INFO")
    $timestamp = Get-Date -Format "yyyy-MM-dd HH:mm:ss"
    Write-Host "[$timestamp] [$Level] $Message"
}

function Analyze-EventPatterns {
    param($EventData)

    Write-Log "Analyzing event patterns..."

    $patterns = @{
        TopEvents = @()
        EventFrequency = @{}
        SessionEngagement = @{}
    }

    # Find top events
    $sortedEvents = $EventData.EventCounts.GetEnumerator() | Sort-Object -Property Value -Descending
    $patterns.TopEvents = $sortedEvents | Select-Object -First 10

    # Calculate event frequency
    if ($EventData.TotalEvents -gt 0 -and $EventData.UniqueSessions -gt 0) {
        $patterns.EventFrequency = @{
            EventsPerSession = [math]::Round($EventData.TotalEvents / $EventData.UniqueSessions, 2)
            UniqueEventTypes = $EventData.UniqueEvents
        }
    }

    return $patterns
}

function Analyze-SessionBehavior {
    param($SessionData)

    Write-Log "Analyzing session behavior..."

    $behavior = @{
        AverageSessionDuration = [math]::Round($SessionData.AverageDuration, 2)
        TotalSessions = $SessionData.TotalSessions
        TotalPlaytime = [math]::Round($SessionData.TotalDuration / 3600, 2)  # Convert to hours
        SessionDistribution = @{}
    }

    # Categorize sessions by duration
    $shortSessions = 0
    $mediumSessions = 0
    $longSessions = 0

    foreach ($session in $SessionData.Sessions) {
        if ($session.duration -lt 300) {  # < 5 minutes
            $shortSessions++
        } elseif ($session.duration -lt 1800) {  # < 30 minutes
            $mediumSessions++
        } else {
            $longSessions++
        }
    }

    $behavior.SessionDistribution = @{
        Short = $shortSessions
        Medium = $mediumSessions
        Long = $longSessions
    }

    return $behavior
}

function Analyze-Performance {
    param($TelemetryData)

    Write-Log "Analyzing performance metrics..."

    if (-not $TelemetryData.PerformanceMetrics) {
        return @{}
    }

    $perf = $TelemetryData.PerformanceMetrics

    $analysis = @{
        PerformanceScore = 0
        FPSAnalysis = @{
            Average = [math]::Round($perf.AvgFPS, 2)
            Minimum = [math]::Round($perf.MinFPS, 2)
            Maximum = [math]::Round($perf.MaxFPS, 2)
            Rating = ""
        }
        MemoryAnalysis = @{
            AverageUsageMB = [math]::Round($perf.AvgMemoryUsage, 2)
            Rating = ""
        }
    }

    # Rate FPS performance
    if ($perf.AvgFPS -ge 60) {
        $analysis.FPSAnalysis.Rating = "Excellent"
        $fpsScore = 100
    } elseif ($perf.AvgFPS -ge 45) {
        $analysis.FPSAnalysis.Rating = "Good"
        $fpsScore = 75
    } elseif ($perf.AvgFPS -ge 30) {
        $analysis.FPSAnalysis.Rating = "Fair"
        $fpsScore = 50
    } else {
        $analysis.FPSAnalysis.Rating = "Poor"
        $fpsScore = 25
    }

    # Rate memory usage
    if ($perf.AvgMemoryUsage -lt 2048) {
        $analysis.MemoryAnalysis.Rating = "Excellent"
        $memScore = 100
    } elseif ($perf.AvgMemoryUsage -lt 4096) {
        $analysis.MemoryAnalysis.Rating = "Good"
        $memScore = 75
    } elseif ($perf.AvgMemoryUsage -lt 6144) {
        $analysis.MemoryAnalysis.Rating = "Fair"
        $memScore = 50
    } else {
        $analysis.MemoryAnalysis.Rating = "Poor"
        $memScore = 25
    }

    $analysis.PerformanceScore = [math]::Round(($fpsScore + $memScore) / 2, 0)

    return $analysis
}

function Analyze-Gameplay {
    param($TelemetryData)

    Write-Log "Analyzing gameplay metrics..."

    if (-not $TelemetryData.GameplayMetrics) {
        return @{}
    }

    $gameplay = $TelemetryData.GameplayMetrics

    $analysis = @{
        Engagement = @{
            TotalPlaytimeHours = [math]::Round($gameplay.TotalPlaytime / 3600, 2)
            LevelsCompleted = $gameplay.LevelsCompleted
            AchievementRate = 0
        }
        Difficulty = @{
            Deaths = $gameplay.Deaths
            DeathRate = 0
            DifficultyRating = ""
        }
    }

    # Calculate rates
    if ($gameplay.TotalPlaytime -gt 0) {
        $analysis.Difficulty.DeathRate = [math]::Round($gameplay.Deaths / ($gameplay.TotalPlaytime / 3600), 2)
    }

    # Rate difficulty
    if ($analysis.Difficulty.DeathRate -lt 1) {
        $analysis.Difficulty.DifficultyRating = "Easy"
    } elseif ($analysis.Difficulty.DeathRate -lt 3) {
        $analysis.Difficulty.DifficultyRating = "Moderate"
    } elseif ($analysis.Difficulty.DeathRate -lt 5) {
        $analysis.Difficulty.DifficultyRating = "Hard"
    } else {
        $analysis.Difficulty.DifficultyRating = "Very Hard"
    }

    return $analysis
}

function Generate-Insights {
    param($AnalysisData)

    Write-Log "Generating insights..."

    $insights = @()

    # Performance insights
    if ($AnalysisData.Performance.PerformanceScore) {
        $score = $AnalysisData.Performance.PerformanceScore
        if ($score -ge 80) {
            $insights += "Performance is excellent with consistent high FPS"
        } elseif ($score -ge 60) {
            $insights += "Performance is good but has room for optimization"
        } else {
            $insights += "Performance needs improvement - consider optimization"
        }
    }

    # Session insights
    if ($AnalysisData.SessionBehavior.AverageSessionDuration) {
        $avgDuration = $AnalysisData.SessionBehavior.AverageSessionDuration
        if ($avgDuration -lt 300) {
            $insights += "Short average session duration - consider improving retention"
        } elseif ($avgDuration -gt 1800) {
            $insights += "High engagement with long session durations"
        }
    }

    # Gameplay insights
    if ($AnalysisData.Gameplay.Difficulty.DifficultyRating) {
        $difficulty = $AnalysisData.Gameplay.Difficulty.DifficultyRating
        $insights += "Game difficulty is perceived as: $difficulty"
    }

    return $insights
}

# Main execution
Write-Log "Starting data analysis..."

# Create output directory
if (-not (Test-Path $OutputPath)) {
    New-Item -ItemType Directory -Path $OutputPath -Force | Out-Null
}

# Find latest aggregated data
$latestAggregation = Get-ChildItem -Path $InputPath -Filter "aggregated_*.json" |
    Sort-Object LastWriteTime -Descending |
    Select-Object -First 1

if (-not $latestAggregation) {
    Write-Log "No aggregated data found in: $InputPath" "ERROR"
    exit 1
}

Write-Log "Analyzing data from: $($latestAggregation.Name)"

$aggregatedData = Get-Content $latestAggregation.FullName -Raw | ConvertFrom-Json

# Perform analysis
$analysis = @{
    GeneratedAt = (Get-Date).ToString("o")
    SourceFile = $latestAggregation.Name
    EventPatterns = Analyze-EventPatterns -EventData $aggregatedData.Events
    SessionBehavior = Analyze-SessionBehavior -SessionData $aggregatedData.Sessions
    Performance = Analyze-Performance -TelemetryData $aggregatedData.Telemetry
    Gameplay = Analyze-Gameplay -TelemetryData $aggregatedData.Telemetry
}

$analysis.Insights = Generate-Insights -AnalysisData $analysis

# Save analysis
$outputFile = Join-Path $OutputPath "analysis_$(Get-Date -Format 'yyyyMMdd_HHmmss').json"
$analysis | ConvertTo-Json -Depth 10 | Set-Content -Path $outputFile -Encoding UTF8

Write-Log "Analysis complete. Output saved to: $outputFile"

if ($Verbose) {
    Write-Log "Analysis results:" "INFO"
    $analysis | ConvertTo-Json -Depth 5 | Write-Host
}

# Display key insights
Write-Log "`nKey Insights:" "INFO"
foreach ($insight in $analysis.Insights) {
    Write-Log "  - $insight" "INFO"
}
