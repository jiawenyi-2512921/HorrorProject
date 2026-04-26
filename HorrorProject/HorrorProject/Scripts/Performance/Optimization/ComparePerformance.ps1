# ComparePerformance.ps1
# Performance Comparison Tool
# Compares performance before and after optimizations

param(
    [string]$BeforeReport,
    [string]$AfterReport,
    [string]$OutputDir = "D:\gptzuo\HorrorProject\HorrorProject\Saved\Comparison"
)

$ErrorActionPreference = "Stop"

Write-Host "=== Performance Comparison Tool ===" -ForegroundColor Cyan
Write-Host ""

# Create output directory
New-Item -ItemType Directory -Force -Path $OutputDir | Out-Null

$timestamp = Get-Date -Format "yyyyMMdd_HHmmss"
$reportFile = Join-Path $OutputDir "performance_comparison_$timestamp.json"

# Find latest reports if not specified
if (-not $BeforeReport) {
    $benchmarkDir = "D:\gptzuo\HorrorProject\HorrorProject\Saved\Benchmarks"
    $reports = Get-ChildItem -Path $benchmarkDir -Filter "framerate_benchmark_*.json" -ErrorAction SilentlyContinue | Sort-Object LastWriteTime
    if ($reports.Count -ge 2) {
        $BeforeReport = $reports[-2].FullName
        $AfterReport = $reports[-1].FullName
    } else {
        Write-Error "Not enough benchmark reports found. Please specify -BeforeReport and -AfterReport"
        exit 1
    }
}

Write-Host "Comparing reports:" -ForegroundColor Yellow
Write-Host "  Before: $BeforeReport" -ForegroundColor Gray
Write-Host "  After: $AfterReport" -ForegroundColor Gray
Write-Host ""

# Load reports
if (-not (Test-Path $BeforeReport)) {
    Write-Error "Before report not found: $BeforeReport"
    exit 1
}

if (-not (Test-Path $AfterReport)) {
    Write-Error "After report not found: $AfterReport"
    exit 1
}

$beforeData = Get-Content $BeforeReport | ConvertFrom-Json
$afterData = Get-Content $AfterReport | ConvertFrom-Json

$comparisonData = @{
    Timestamp = $timestamp
    BeforeReport = $BeforeReport
    AfterReport = $AfterReport
    Comparisons = @{}
    Summary = @{}
}

Write-Host "Analyzing performance differences..." -ForegroundColor Cyan
Write-Host ""

# Compare FPS for each quality setting
$qualityLevels = @("Low", "Medium", "High", "Epic")

foreach ($quality in $qualityLevels) {
    if ($beforeData.Results.$quality -and $afterData.Results.$quality) {
        $before = $beforeData.Results.$quality
        $after = $afterData.Results.$quality

        $fpsImprovement = $after.FPS.Average - $before.FPS.Average
        $fpsImprovementPercent = if ($before.FPS.Average -gt 0) { ($fpsImprovement / $before.FPS.Average) * 100 } else { 0 }

        $frameTimeImprovement = $before.FrameTime.Average - $after.FrameTime.Average
        $frameTimeImprovementPercent = if ($before.FrameTime.Average -gt 0) { ($frameTimeImprovement / $before.FrameTime.Average) * 100 } else { 0 }

        $comparison = @{
            Quality = $quality
            FPS = @{
                Before = $before.FPS.Average
                After = $after.FPS.Average
                Improvement = $fpsImprovement
                ImprovementPercent = $fpsImprovementPercent
            }
            FrameTime = @{
                Before = $before.FrameTime.Average
                After = $after.FrameTime.Average
                Improvement = $frameTimeImprovement
                ImprovementPercent = $frameTimeImprovementPercent
            }
            GameThread = @{
                Before = $before.GameThread.Average
                After = $after.GameThread.Average
                Improvement = $before.GameThread.Average - $after.GameThread.Average
            }
            RenderThread = @{
                Before = $before.RenderThread.Average
                After = $after.RenderThread.Average
                Improvement = $before.RenderThread.Average - $after.RenderThread.Average
            }
            GPU = @{
                Before = $before.GPU.Average
                After = $after.GPU.Average
                Improvement = $before.GPU.Average - $after.GPU.Average
            }
        }

        $comparisonData.Comparisons[$quality] = $comparison

        Write-Host "=== $quality Quality ===" -ForegroundColor Yellow
        Write-Host ""

        # FPS comparison
        $fpsColor = if ($fpsImprovement -gt 5) { "Green" } elseif ($fpsImprovement -gt 0) { "Cyan" } elseif ($fpsImprovement -gt -5) { "Yellow" } else { "Red" }
        Write-Host "FPS:" -ForegroundColor White
        Write-Host "  Before: $([math]::Round($before.FPS.Average, 2))" -ForegroundColor Gray
        Write-Host "  After: $([math]::Round($after.FPS.Average, 2))" -ForegroundColor Gray
        Write-Host "  Change: $(if ($fpsImprovement -gt 0) { '+' })$([math]::Round($fpsImprovement, 2)) ($(if ($fpsImprovementPercent -gt 0) { '+' })$([math]::Round($fpsImprovementPercent, 1))%)" -ForegroundColor $fpsColor
        Write-Host ""

        # Frame time comparison
        $ftColor = if ($frameTimeImprovement -gt 2) { "Green" } elseif ($frameTimeImprovement -gt 0) { "Cyan" } elseif ($frameTimeImprovement -gt -2) { "Yellow" } else { "Red" }
        Write-Host "Frame Time:" -ForegroundColor White
        Write-Host "  Before: $([math]::Round($before.FrameTime.Average, 2)) ms" -ForegroundColor Gray
        Write-Host "  After: $([math]::Round($after.FrameTime.Average, 2)) ms" -ForegroundColor Gray
        Write-Host "  Change: $(if ($frameTimeImprovement -gt 0) { '-' } else { '+' })$([math]::Round([math]::Abs($frameTimeImprovement), 2)) ms ($(if ($frameTimeImprovementPercent -gt 0) { '+' })$([math]::Round($frameTimeImprovementPercent, 1))%)" -ForegroundColor $ftColor
        Write-Host ""

        # Thread times
        Write-Host "Thread Times:" -ForegroundColor White
        Write-Host "  Game Thread: $([math]::Round($before.GameThread.Average, 2)) -> $([math]::Round($after.GameThread.Average, 2)) ms ($(if ($comparison.GameThread.Improvement -gt 0) { '-' } else { '+' })$([math]::Round([math]::Abs($comparison.GameThread.Improvement), 2)) ms)" -ForegroundColor $(if ($comparison.GameThread.Improvement -gt 0) { "Green" } else { "Red" })
        Write-Host "  Render Thread: $([math]::Round($before.RenderThread.Average, 2)) -> $([math]::Round($after.RenderThread.Average, 2)) ms ($(if ($comparison.RenderThread.Improvement -gt 0) { '-' } else { '+' })$([math]::Round([math]::Abs($comparison.RenderThread.Improvement), 2)) ms)" -ForegroundColor $(if ($comparison.RenderThread.Improvement -gt 0) { "Green" } else { "Red" })
        Write-Host "  GPU: $([math]::Round($before.GPU.Average, 2)) -> $([math]::Round($after.GPU.Average, 2)) ms ($(if ($comparison.GPU.Improvement -gt 0) { '-' } else { '+' })$([math]::Round([math]::Abs($comparison.GPU.Improvement), 2)) ms)" -ForegroundColor $(if ($comparison.GPU.Improvement -gt 0) { "Green" } else { "Red" })
        Write-Host ""
    }
}

# Calculate overall summary
$epicComparison = $comparisonData.Comparisons["Epic"]
if ($epicComparison) {
    $comparisonData.Summary = @{
        OverallFPSImprovement = $epicComparison.FPS.Improvement
        OverallFPSImprovementPercent = $epicComparison.FPS.ImprovementPercent
        TargetAchieved = $epicComparison.FPS.After -ge 60
        Status = if ($epicComparison.FPS.After -ge 60) { "Target Achieved" }
                elseif ($epicComparison.FPS.Improvement -gt 5) { "Significant Improvement" }
                elseif ($epicComparison.FPS.Improvement -gt 0) { "Minor Improvement" }
                elseif ($epicComparison.FPS.Improvement -eq 0) { "No Change" }
                else { "Performance Regression" }
    }
}

# Save report
$comparisonData | ConvertTo-Json -Depth 10 | Out-File $reportFile -Encoding UTF8

Write-Host "=== Overall Summary ===" -ForegroundColor Cyan
Write-Host ""

if ($comparisonData.Summary.Count -gt 0) {
    $statusColor = switch ($comparisonData.Summary.Status) {
        "Target Achieved" { "Green" }
        "Significant Improvement" { "Cyan" }
        "Minor Improvement" { "Yellow" }
        "No Change" { "Gray" }
        "Performance Regression" { "Red" }
        default { "White" }
    }

    Write-Host "Status: $($comparisonData.Summary.Status)" -ForegroundColor $statusColor
    Write-Host "FPS Improvement (Epic): $(if ($comparisonData.Summary.OverallFPSImprovement -gt 0) { '+' })$([math]::Round($comparisonData.Summary.OverallFPSImprovement, 2)) ($(if ($comparisonData.Summary.OverallFPSImprovementPercent -gt 0) { '+' })$([math]::Round($comparisonData.Summary.OverallFPSImprovementPercent, 1))%)" -ForegroundColor $statusColor
    Write-Host "60 FPS Target: $(if ($comparisonData.Summary.TargetAchieved) { 'Achieved ✓' } else { 'Not Yet Achieved' })" -ForegroundColor $(if ($comparisonData.Summary.TargetAchieved) { "Green" } else { "Yellow" })
}

Write-Host ""
Write-Host "Report saved to: $reportFile" -ForegroundColor Green
Write-Host ""

# Recommendations
Write-Host "=== Recommendations ===" -ForegroundColor Cyan

if ($comparisonData.Summary.Status -eq "Target Achieved") {
    Write-Host "Performance target achieved! Continue monitoring and maintain optimizations." -ForegroundColor Green
} elseif ($comparisonData.Summary.Status -eq "Significant Improvement") {
    Write-Host "Good progress! Continue with current optimization strategy." -ForegroundColor Cyan
    Write-Host "Focus on remaining bottlenecks to reach 60 FPS target." -ForegroundColor Cyan
} elseif ($comparisonData.Summary.Status -eq "Minor Improvement") {
    Write-Host "Some improvement detected, but more optimization needed." -ForegroundColor Yellow
    Write-Host "Review optimization priorities and focus on high-impact areas." -ForegroundColor Yellow
} elseif ($comparisonData.Summary.Status -eq "Performance Regression") {
    Write-Host "WARNING: Performance has regressed!" -ForegroundColor Red
    Write-Host "Review recent changes and revert problematic optimizations." -ForegroundColor Red
}

Write-Host ""
Write-Host "Performance comparison complete!" -ForegroundColor Green
