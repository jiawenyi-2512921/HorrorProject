# Full Asset Optimization Pipeline for Horror Project
# Runs complete optimization workflow

param(
    [string]$ProjectPath = "D:\gptzuo\HorrorProject\HorrorProject",
    [switch]$SkipAnalysis,
    [switch]$SkipLODs,
    [switch]$SkipTextures,
    [switch]$SkipMaterials,
    [switch]$SkipLighting,
    [switch]$SkipValidation,
    [switch]$EnableVirtualTextures
)

$ScriptsPath = "$ProjectPath\Scripts\Assets"

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "  Horror Project - Full Asset Optimization" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

$startTime = Get-Date

# Step 1: Asset Analysis
if (-not $SkipAnalysis) {
    Write-Host "[1/6] Running Asset Analysis..." -ForegroundColor Yellow
    & "$ScriptsPath\AnalyzeAssets.ps1" -ProjectPath $ProjectPath
    if ($LASTEXITCODE -ne 0) {
        Write-Host "Asset analysis failed!" -ForegroundColor Red
        exit 1
    }
    Write-Host ""
} else {
    Write-Host "[1/6] Skipping Asset Analysis" -ForegroundColor Gray
}

# Step 2: LOD Generation
if (-not $SkipLODs) {
    Write-Host "[2/6] Generating LODs..." -ForegroundColor Yellow
    & "$ScriptsPath\GenerateLODs.ps1" -ProjectPath $ProjectPath
    if ($LASTEXITCODE -ne 0) {
        Write-Host "LOD generation failed!" -ForegroundColor Red
        exit 1
    }
    Write-Host ""
} else {
    Write-Host "[2/6] Skipping LOD Generation" -ForegroundColor Gray
}

# Step 3: Texture Optimization
if (-not $SkipTextures) {
    Write-Host "[3/6] Optimizing Textures..." -ForegroundColor Yellow
    $texParams = @{
        ProjectPath = $ProjectPath
    }
    if ($EnableVirtualTextures) {
        $texParams.EnableVirtualTextures = $true
    }
    & "$ScriptsPath\OptimizeTextures.ps1" @texParams
    if ($LASTEXITCODE -ne 0) {
        Write-Host "Texture optimization failed!" -ForegroundColor Red
        exit 1
    }
    Write-Host ""
} else {
    Write-Host "[3/6] Skipping Texture Optimization" -ForegroundColor Gray
}

# Step 4: Material Optimization
if (-not $SkipMaterials) {
    Write-Host "[4/6] Optimizing Materials..." -ForegroundColor Yellow
    & "$ScriptsPath\OptimizeMaterials.ps1" -ProjectPath $ProjectPath
    if ($LASTEXITCODE -ne 0) {
        Write-Host "Material optimization failed!" -ForegroundColor Red
        exit 1
    }
    Write-Host ""
} else {
    Write-Host "[4/6] Skipping Material Optimization" -ForegroundColor Gray
}

# Step 5: Lighting Optimization
if (-not $SkipLighting) {
    Write-Host "[5/6] Optimizing Lighting..." -ForegroundColor Yellow
    & "$ScriptsPath\OptimizeLighting.ps1" -ProjectPath $ProjectPath
    if ($LASTEXITCODE -ne 0) {
        Write-Host "Lighting optimization failed!" -ForegroundColor Red
        exit 1
    }
    Write-Host ""
} else {
    Write-Host "[5/6] Skipping Lighting Optimization" -ForegroundColor Gray
}

# Step 6: Validation
if (-not $SkipValidation) {
    Write-Host "[6/6] Validating Assets..." -ForegroundColor Yellow
    & "$ScriptsPath\ValidateAssets.ps1" -ProjectPath $ProjectPath
    if ($LASTEXITCODE -ne 0) {
        Write-Host "Asset validation failed!" -ForegroundColor Red
        exit 1
    }
    Write-Host ""
} else {
    Write-Host "[6/6] Skipping Validation" -ForegroundColor Gray
}

$endTime = Get-Date
$duration = $endTime - $startTime

Write-Host "========================================" -ForegroundColor Green
Write-Host "  Optimization Complete!" -ForegroundColor Green
Write-Host "========================================" -ForegroundColor Green
Write-Host "Total time: $($duration.ToString('hh\:mm\:ss'))" -ForegroundColor Cyan
Write-Host ""

# Generate summary report
Write-Host "Generating summary report..." -ForegroundColor Yellow

$summary = @{
    timestamp = Get-Date -Format "yyyy-MM-dd HH:mm:ss"
    duration_seconds = $duration.TotalSeconds
    reports = @{}
}

# Load all reports
$reportFiles = @(
    "AssetReport.json",
    "LODGenerationReport.json",
    "TextureOptimizationReport.json",
    "MaterialOptimizationReport.json",
    "LightingOptimizationReport.json",
    "ValidationReport.json"
)

foreach ($reportFile in $reportFiles) {
    $reportPath = "$ScriptsPath\$reportFile"
    if (Test-Path $reportPath) {
        $reportName = $reportFile -replace "Report\.json$", ""
        $summary.reports[$reportName] = Get-Content $reportPath | ConvertFrom-Json
    }
}

# Save summary
$summaryPath = "$ScriptsPath\OptimizationSummary.json"
$summary | ConvertTo-Json -Depth 10 | Out-File -FilePath $summaryPath -Encoding UTF8

Write-Host "Summary saved to: $summaryPath" -ForegroundColor Green
Write-Host ""

# Print key metrics
if ($summary.reports.Asset) {
    Write-Host "Asset Statistics:" -ForegroundColor Cyan
    Write-Host "  Total Meshes: $($summary.reports.Asset.summary.total_meshes)"
    Write-Host "  Total Textures: $($summary.reports.Asset.summary.total_textures)"
    Write-Host "  Total Materials: $($summary.reports.Asset.summary.total_materials)"
    Write-Host ""
}

if ($summary.reports.LODGeneration) {
    Write-Host "LOD Generation:" -ForegroundColor Cyan
    Write-Host "  Success: $($summary.reports.LODGeneration.success.Count)" -ForegroundColor Green
    Write-Host "  Failed: $($summary.reports.LODGeneration.failed.Count)" -ForegroundColor $(if($summary.reports.LODGeneration.failed.Count -gt 0){'Red'}else{'Green'})
    Write-Host ""
}

if ($summary.reports.TextureOptimization) {
    Write-Host "Texture Optimization:" -ForegroundColor Cyan
    Write-Host "  Optimized: $($summary.reports.TextureOptimization.optimized.Count)" -ForegroundColor Green
    Write-Host "  Already Optimal: $($summary.reports.TextureOptimization.already_optimal.Count)"
    Write-Host ""
}

if ($summary.reports.Validation) {
    Write-Host "Validation Results:" -ForegroundColor Cyan
    Write-Host "  Total Issues: $($summary.reports.Validation.summary.total_issues)" -ForegroundColor $(if($summary.reports.Validation.summary.total_issues -gt 0){'Red'}else{'Green'})
    Write-Host "  Total Warnings: $($summary.reports.Validation.summary.total_warnings)" -ForegroundColor $(if($summary.reports.Validation.summary.total_warnings -gt 0){'Yellow'}else{'Green'})
    Write-Host ""
}

Write-Host "All reports available in: $ScriptsPath" -ForegroundColor Cyan
