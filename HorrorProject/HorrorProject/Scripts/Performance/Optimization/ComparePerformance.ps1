# ComparePerformance.ps1
# Compares two framerate benchmark reports.

param(
    [string]$BeforeReport,
    [string]$AfterReport,
    [string]$OutputDir = ""
)

$ErrorActionPreference = "Stop"

$ProjectRoot = Split-Path -Parent (Split-Path -Parent (Split-Path -Parent $PSScriptRoot))
if ([string]::IsNullOrWhiteSpace($OutputDir)) {
    $OutputDir = Join-Path $ProjectRoot "Saved\Comparison"
}

Write-Host "=== Performance Comparison Tool ===" -ForegroundColor Cyan
Write-Host ""

New-Item -ItemType Directory -Force -Path $OutputDir | Out-Null

$Timestamp = Get-Date -Format "yyyyMMdd_HHmmss"
$ReportFile = Join-Path $OutputDir "performance_comparison_$Timestamp.json"

if (-not $BeforeReport) {
    $BenchmarkDir = Join-Path $ProjectRoot "Saved\Benchmarks"
    $Reports = @(Get-ChildItem -Path $BenchmarkDir -Filter "framerate_benchmark_*.json" -ErrorAction SilentlyContinue | Sort-Object LastWriteTime)
    if ($Reports.Count -ge 2) {
        $BeforeReport = $Reports[-2].FullName
        $AfterReport = $Reports[-1].FullName
    } else {
        Write-Error "Not enough benchmark reports found. Specify -BeforeReport and -AfterReport."
        exit 1
    }
}

if (-not $AfterReport) {
    Write-Error "AfterReport is required when BeforeReport is specified."
    exit 1
}

foreach ($Report in @($BeforeReport, $AfterReport)) {
    if (-not (Test-Path -LiteralPath $Report)) {
        Write-Error "Report not found: $Report"
        exit 1
    }
}

Write-Host "Comparing reports:" -ForegroundColor Yellow
Write-Host "  Before: $BeforeReport" -ForegroundColor Gray
Write-Host "  After: $AfterReport" -ForegroundColor Gray
Write-Host ""

$BeforeData = Get-Content -LiteralPath $BeforeReport -Raw | ConvertFrom-Json
$AfterData = Get-Content -LiteralPath $AfterReport -Raw | ConvertFrom-Json

$ComparisonData = [ordered]@{
    Timestamp = $Timestamp
    BeforeReport = $BeforeReport
    AfterReport = $AfterReport
    Comparisons = [ordered]@{}
    Summary = [ordered]@{}
}

function Get-PercentChange {
    param(
        [double]$Delta,
        [double]$Base
    )

    if ($Base -eq 0) {
        return 0
    }

    return ($Delta / $Base) * 100
}

foreach ($Quality in @("Low", "Medium", "High", "Epic")) {
    if (-not $BeforeData.Results.$Quality -or -not $AfterData.Results.$Quality) {
        continue
    }

    $Before = $BeforeData.Results.$Quality
    $After = $AfterData.Results.$Quality

    $FpsDelta = [double]$After.FPS.Average - [double]$Before.FPS.Average
    $FrameTimeDelta = [double]$Before.FrameTime.Average - [double]$After.FrameTime.Average

    $Comparison = [ordered]@{
        Quality = $Quality
        FPS = [ordered]@{
            Before = [double]$Before.FPS.Average
            After = [double]$After.FPS.Average
            Improvement = $FpsDelta
            ImprovementPercent = Get-PercentChange -Delta $FpsDelta -Base ([double]$Before.FPS.Average)
        }
        FrameTime = [ordered]@{
            Before = [double]$Before.FrameTime.Average
            After = [double]$After.FrameTime.Average
            Improvement = $FrameTimeDelta
            ImprovementPercent = Get-PercentChange -Delta $FrameTimeDelta -Base ([double]$Before.FrameTime.Average)
        }
    }

    $ComparisonData.Comparisons[$Quality] = $Comparison

    $FpsColor = if ($FpsDelta -gt 5) { "Green" } elseif ($FpsDelta -gt 0) { "Cyan" } elseif ($FpsDelta -gt -5) { "Yellow" } else { "Red" }
    $FrameColor = if ($FrameTimeDelta -gt 2) { "Green" } elseif ($FrameTimeDelta -gt 0) { "Cyan" } elseif ($FrameTimeDelta -gt -2) { "Yellow" } else { "Red" }

    Write-Host "=== $Quality Quality ===" -ForegroundColor Yellow
    Write-Host "FPS before/after: $([math]::Round($Comparison.FPS.Before, 2)) -> $([math]::Round($Comparison.FPS.After, 2))" -ForegroundColor $FpsColor
    Write-Host "FPS change: $(if ($FpsDelta -gt 0) { '+' })$([math]::Round($FpsDelta, 2)) ($([math]::Round($Comparison.FPS.ImprovementPercent, 1))%)" -ForegroundColor $FpsColor
    Write-Host "Frame time before/after: $([math]::Round($Comparison.FrameTime.Before, 2)) ms -> $([math]::Round($Comparison.FrameTime.After, 2)) ms" -ForegroundColor $FrameColor
    Write-Host ""
}

$EpicComparison = $ComparisonData.Comparisons["Epic"]
if ($EpicComparison) {
    $Status = if ($EpicComparison.FPS.After -ge 60) {
        "Target Achieved"
    } elseif ($EpicComparison.FPS.Improvement -gt 5) {
        "Significant Improvement"
    } elseif ($EpicComparison.FPS.Improvement -gt 0) {
        "Minor Improvement"
    } elseif ($EpicComparison.FPS.Improvement -eq 0) {
        "No Change"
    } else {
        "Performance Regression"
    }

    $ComparisonData.Summary = [ordered]@{
        OverallFPSImprovement = $EpicComparison.FPS.Improvement
        OverallFPSImprovementPercent = $EpicComparison.FPS.ImprovementPercent
        TargetAchieved = $EpicComparison.FPS.After -ge 60
        Status = $Status
    }
}

$ComparisonData | ConvertTo-Json -Depth 10 | Out-File $ReportFile -Encoding UTF8

Write-Host "=== Overall Summary ===" -ForegroundColor Cyan
if ($ComparisonData.Summary.Count -gt 0) {
    $StatusColor = switch ($ComparisonData.Summary.Status) {
        "Target Achieved" { "Green" }
        "Significant Improvement" { "Cyan" }
        "Minor Improvement" { "Yellow" }
        "No Change" { "Gray" }
        "Performance Regression" { "Red" }
        default { "White" }
    }

    Write-Host "Status: $($ComparisonData.Summary.Status)" -ForegroundColor $StatusColor
    Write-Host "FPS Improvement (Epic): $(if ($ComparisonData.Summary.OverallFPSImprovement -gt 0) { '+' })$([math]::Round($ComparisonData.Summary.OverallFPSImprovement, 2))" -ForegroundColor $StatusColor
    Write-Host "60 FPS Target: $(if ($ComparisonData.Summary.TargetAchieved) { 'Achieved' } else { 'Not Yet Achieved' })" -ForegroundColor $(if ($ComparisonData.Summary.TargetAchieved) { "Green" } else { "Yellow" })
} else {
    Write-Host "No matching Epic benchmark data found." -ForegroundColor Yellow
}

Write-Host ""
Write-Host "Report saved to: $ReportFile" -ForegroundColor Green
Write-Host "Performance comparison complete." -ForegroundColor Green
