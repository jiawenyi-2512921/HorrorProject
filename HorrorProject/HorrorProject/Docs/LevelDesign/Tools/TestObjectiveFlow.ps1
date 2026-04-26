# Test Objective Flow Tool
# Tests objective progression and flow

param(
    [Parameter(Mandatory=$false)]
    [string]$LevelName = "SM13",

    [Parameter(Mandatory=$false)]
    [string]$DataPath = "D:/gptzuo/HorrorProject/HorrorProject/Content/Maps",

    [Parameter(Mandatory=$false)]
    [switch]$RunFullTest,

    [Parameter(Mandatory=$false)]
    [switch]$GenerateReport
)

$ErrorActionPreference = "Stop"

Write-Host "=== Test Objective Flow ===" -ForegroundColor Cyan
Write-Host "Level: $LevelName"
Write-Host ""

$objectivesFile = Join-Path $DataPath "${LevelName}_Objectives.json"
$objectivesData = Get-Content $objectivesFile -Raw | ConvertFrom-Json

$testResults = @{
    LevelName = $LevelName
    Passed = $true
    Tests = @()
    Timestamp = Get-Date -Format "yyyy-MM-dd HH:mm:ss"
}

# Test 1: Beat progression
Write-Host "Testing beat progression..." -ForegroundColor Yellow

$beatTest = @{
    Name = "Beat Progression"
    Passed = $true
    Issues = @()
}

$beats = $objectivesData.Objectives | Sort-Object -Property Beat | Group-Object -Property Beat

if ($beats.Count -ne 7) {
    $beatTest.Issues += "Expected 7 beats, found $($beats.Count)"
    $beatTest.Passed = $false
}

foreach ($beat in $beats) {
    Write-Host "  - Beat $($beat.Name): $($beat.Count) objectives" -ForegroundColor Gray
}

$testResults.Tests += $beatTest

if ($beatTest.Passed) {
    Write-Host "  ✓ Beat progression test passed" -ForegroundColor Green
} else {
    Write-Host "  ✗ Beat progression test failed" -ForegroundColor Red
    $testResults.Passed = $false
}

# Test 2: Timing progression
Write-Host ""
Write-Host "Testing timing progression..." -ForegroundColor Yellow

$timingTest = @{
    Name = "Timing Progression"
    Passed = $true
    Issues = @()
}

$sortedObjectives = $objectivesData.Objectives | Where-Object { -not $_.IsOptional } | Sort-Object -Property TargetTime

$previousTime = 0
foreach ($obj in $sortedObjectives) {
    if ($obj.TargetTime -lt $previousTime) {
        $timingTest.Issues += "Objective $($obj.ID) time ($($obj.TargetTime)m) is before previous ($previousTime m)"
        $timingTest.Passed = $false
    }
    $previousTime = $obj.TargetTime
}

$testResults.Tests += $timingTest

if ($timingTest.Passed) {
    Write-Host "  ✓ Timing progression test passed" -ForegroundColor Green
} else {
    Write-Host "  ✗ Timing progression test failed" -ForegroundColor Red
    $testResults.Passed = $false
}

# Test 3: Difficulty curve
Write-Host ""
Write-Host "Testing difficulty curve..." -ForegroundColor Yellow

$difficultyTest = @{
    Name = "Difficulty Curve"
    Passed = $true
    Issues = @()
}

$difficultyMap = @{ "Easy" = 1; "Medium" = 2; "Hard" = 3 }
$difficulties = $objectivesData.Objectives | Where-Object { -not $_.IsOptional } | ForEach-Object { $difficultyMap[$_.Difficulty] }

$avgDifficulty = ($difficulties | Measure-Object -Average).Average

if ($avgDifficulty -lt 1.5 -or $avgDifficulty -gt 2.5) {
    $difficultyTest.Issues += "Average difficulty out of range: $([Math]::Round($avgDifficulty, 2)) (expected 1.5-2.5)"
    $difficultyTest.Passed = $false
}

$testResults.Tests += $difficultyTest

if ($difficultyTest.Passed) {
    Write-Host "  ✓ Difficulty curve test passed (avg: $([Math]::Round($avgDifficulty, 2)))" -ForegroundColor Green
} else {
    Write-Host "  ✗ Difficulty curve test failed" -ForegroundColor Red
    $testResults.Passed = $false
}

Write-Host ""
Write-Host "=== Objective Flow Test Complete ===" -ForegroundColor Cyan

if ($testResults.Passed) {
    Write-Host "Status: PASSED ✓" -ForegroundColor Green
} else {
    Write-Host "Status: FAILED ✗" -ForegroundColor Red
}

if ($GenerateReport) {
    $reportPath = Join-Path $DataPath "${LevelName}_ObjectiveFlow_Report.md"

    $status = if ($testResults.Passed) { "✓ PASSED" } else { "✗ FAILED" }

    $report = @"
# $LevelName Objective Flow Test Report

**Status**: $status
**Generated**: $($testResults.Timestamp)

## Test Results

"@

    foreach ($test in $testResults.Tests) {
        $testStatus = if ($test.Passed) { "✓" } else { "✗" }
        $report += "`n### $testStatus $($test.Name)`n`n"

        if ($test.Issues.Count -gt 0) {
            foreach ($issue in $test.Issues) {
                $report += "- $issue`n"
            }
        } else {
            $report += "No issues found.`n"
        }
    }

    $report | Set-Content -Path $reportPath -Encoding UTF8
    Write-Host "Report saved: $reportPath" -ForegroundColor Green
}
