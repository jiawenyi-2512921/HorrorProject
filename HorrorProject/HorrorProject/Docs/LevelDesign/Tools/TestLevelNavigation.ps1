# Test Level Navigation Tool
# Tests player navigation through the level

param(
    [Parameter(Mandatory=$false)]
    [string]$LevelName = "SM13",

    [Parameter(Mandatory=$false)]
    [string]$DataPath = "D:/gptzuo/HorrorProject/HorrorProject/Content/Maps",

    [Parameter(Mandatory=$false)]
    [switch]$CollectAllEvidence,

    [Parameter(Mandatory=$false)]
    [switch]$SpeedrunMode,

    [Parameter(Mandatory=$false)]
    [switch]$GenerateReport
)

$ErrorActionPreference = "Stop"

Write-Host "=== Test Level Navigation ===" -ForegroundColor Cyan
Write-Host "Level: $LevelName"
Write-Host "Mode: $(if ($SpeedrunMode) { 'Speedrun' } else { 'Normal' })"
Write-Host ""

# Load objectives
$objectivesFile = Join-Path $DataPath "${LevelName}_Objectives.json"
$objectivesData = Get-Content $objectivesFile -Raw | ConvertFrom-Json

# Simulate navigation
Write-Host "Simulating navigation..." -ForegroundColor Yellow

$testResults = @{
    LevelName = $LevelName
    Mode = if ($SpeedrunMode) { "Speedrun" } else { "Normal" }
    StartTime = Get-Date
    Objectives = @()
    TotalDistance = 0
    EstimatedTime = 0
    Timestamp = Get-Date -Format "yyyy-MM-dd HH:mm:ss"
}

$currentPosition = @{ X = 0; Y = 0; Z = 120 }  # Spawn point
$objectives = $objectivesData.Objectives

if ($SpeedrunMode) {
    $objectives = $objectives | Where-Object { -not $_.IsOptional }
}

foreach ($obj in $objectives) {
    $dx = $obj.Position.X - $currentPosition.X
    $dy = $obj.Position.Y - $currentPosition.Y
    $dz = $obj.Position.Z - $currentPosition.Z

    $distance = [Math]::Sqrt($dx * $dx + $dy * $dy + $dz * $dz) / 100
    $testResults.TotalDistance += $distance

    # Estimate time (4 m/s average speed + objective time)
    $travelTime = $distance / 4 / 60  # minutes
    $objectiveTime = if ($SpeedrunMode) { $obj.TargetTime * 0.5 } else { $obj.TargetTime }
    $totalTime = $travelTime + ($objectiveTime - ($testResults.EstimatedTime))

    $testResults.EstimatedTime = $objectiveTime

    Write-Host "  - Objective $($obj.ID): $($obj.Name)" -ForegroundColor Gray
    Write-Host "    Distance: $([Math]::Round($distance, 1))m | Time: $([Math]::Round($totalTime, 1))m" -ForegroundColor DarkGray

    $testResults.Objectives += @{
        ID = $obj.ID
        Name = $obj.Name
        Distance = [Math]::Round($distance, 1)
        Time = [Math]::Round($totalTime, 1)
    }

    $currentPosition = $obj.Position
}

$testResults.EndTime = Get-Date

Write-Host ""
Write-Host "=== Navigation Test Complete ===" -ForegroundColor Cyan
Write-Host "Total Distance: $([Math]::Round($testResults.TotalDistance, 1))m" -ForegroundColor Green
Write-Host "Estimated Time: $([Math]::Round($testResults.EstimatedTime, 1)) minutes" -ForegroundColor Green
Write-Host "Objectives Completed: $($testResults.Objectives.Count)" -ForegroundColor Green

if ($GenerateReport) {
    $reportPath = Join-Path $DataPath "${LevelName}_Navigation_Report.md"

    $report = @"
# $LevelName Navigation Test Report

**Mode**: $($testResults.Mode)
**Generated**: $($testResults.Timestamp)

## Summary

- **Total Distance**: $([Math]::Round($testResults.TotalDistance, 1))m
- **Estimated Time**: $([Math]::Round($testResults.EstimatedTime, 1)) minutes
- **Objectives**: $($testResults.Objectives.Count)

## Route

| Objective | Name | Distance (m) | Time (m) |
|-----------|------|--------------|----------|
"@

    foreach ($obj in $testResults.Objectives) {
        $report += "`n| $($obj.ID) | $($obj.Name) | $($obj.Distance) | $($obj.Time) |"
    }

    $report | Set-Content -Path $reportPath -Encoding UTF8
    Write-Host "Report saved: $reportPath" -ForegroundColor Green
}
