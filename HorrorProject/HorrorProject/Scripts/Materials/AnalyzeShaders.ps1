# AnalyzeShaders.ps1 - Shader Complexity Analysis Tool

param(
    [string]$ProjectPath = "",
    [string]$OutputPath = ""
)

$ErrorActionPreference = "Continue"

. (Join-Path (Split-Path -Parent $PSScriptRoot) "Validation\Common.ps1")

if ([string]::IsNullOrWhiteSpace($ProjectPath)) {
    $ProjectPath = Get-HorrorProjectRoot -StartPath $PSScriptRoot
} else {
    $ProjectPath = (Resolve-Path -LiteralPath $ProjectPath).Path
}
if ([string]::IsNullOrWhiteSpace($OutputPath)) {
    $OutputPath = Join-Path $PSScriptRoot "ShaderAnalysis.json"
}

Write-Host "=== Shader Complexity Analysis ===" -ForegroundColor Cyan
Write-Host "Project Path: $ProjectPath" -ForegroundColor Yellow

# Find all material files
$materialFiles = @(Get-ChildItem -Path "$ProjectPath\Content" -Filter "*.uasset" -Recurse -ErrorAction SilentlyContinue | Where-Object {
    $_.Name -match "^M_|^MI_|^MF_|Material"
})

Write-Host "Found $($materialFiles.Count) material files" -ForegroundColor Green

# Material classification
$masterMaterials = @()
$materialInstances = @()
$materialFunctions = @()
$postProcessMaterials = @()

foreach ($file in $materialFiles) {
    $relativePath = $file.FullName.Replace("$ProjectPath\Content\", "")

    $materialInfo = @{
        Name = $file.BaseName
        Path = $relativePath
        FullPath = $file.FullName
        Size = $file.Length
        Type = "Unknown"
        Category = "Unknown"
        EstimatedComplexity = "Unknown"
    }

    # Classify material type
    if ($file.Name -match "^MI_") {
        $materialInfo.Type = "MaterialInstance"
        $materialInstances += $materialInfo
    }
    elseif ($file.Name -match "^MF_") {
        $materialInfo.Type = "MaterialFunction"
        $materialFunctions += $materialInfo
    }
    elseif ($file.Name -match "^M_") {
        $materialInfo.Type = "MasterMaterial"
        $masterMaterials += $materialInfo
    }

    # Classify usage category
    if ($relativePath -match "PostProcess") {
        $materialInfo.Category = "PostProcess"
        $postProcessMaterials += $materialInfo
    }
    elseif ($relativePath -match "Building|Wall|Floor|Ceiling") {
        $materialInfo.Category = "Architecture"
    }
    elseif ($relativePath -match "Light|Emissive") {
        $materialInfo.Category = "Lighting"
    }
    elseif ($relativePath -match "Water|Glass|Translucent") {
        $materialInfo.Category = "Translucent"
    }
    elseif ($relativePath -match "Prop|Object") {
        $materialInfo.Category = "Props"
    }
    elseif ($relativePath -match "Character|Mannequin") {
        $materialInfo.Category = "Character"
    }

    # Estimate complexity based on file size
    if ($file.Length -gt 100KB) {
        $materialInfo.EstimatedComplexity = "VeryHigh"
    }
    elseif ($file.Length -gt 50KB) {
        $materialInfo.EstimatedComplexity = "High"
    }
    elseif ($file.Length -gt 20KB) {
        $materialInfo.EstimatedComplexity = "Medium"
    }
    else {
        $materialInfo.EstimatedComplexity = "Low"
    }
}

# Generate analysis report
$analysis = @{
    Timestamp = Get-Date -Format "yyyy-MM-dd HH:mm:ss"
    ProjectPath = $ProjectPath
    Summary = @{
        TotalMaterials = $materialFiles.Count
        MasterMaterials = $masterMaterials.Count
        MaterialInstances = $materialInstances.Count
        MaterialFunctions = $materialFunctions.Count
        PostProcessMaterials = $postProcessMaterials.Count
    }
    MasterMaterials = $masterMaterials
    MaterialInstances = $materialInstances
    MaterialFunctions = $materialFunctions
    PostProcessMaterials = $postProcessMaterials
    ComplexityDistribution = @{
        VeryHigh = ($materialFiles | Where-Object { $_.Length -gt 100KB }).Count
        High = ($materialFiles | Where-Object { $_.Length -gt 50KB -and $_.Length -le 100KB }).Count
        Medium = ($materialFiles | Where-Object { $_.Length -gt 20KB -and $_.Length -le 50KB }).Count
        Low = ($materialFiles | Where-Object { $_.Length -le 20KB }).Count
    }
    CategoryDistribution = @{
        PostProcess = ($materialFiles | Where-Object { $_.FullName -match "PostProcess" }).Count
        Architecture = ($materialFiles | Where-Object { $_.FullName -match "Building|Wall|Floor|Ceiling" }).Count
        Lighting = ($materialFiles | Where-Object { $_.FullName -match "Light|Emissive" }).Count
        Translucent = ($materialFiles | Where-Object { $_.FullName -match "Water|Glass|Translucent" }).Count
        Props = ($materialFiles | Where-Object { $_.FullName -match "Prop|Object" }).Count
        Character = ($materialFiles | Where-Object { $_.FullName -match "Character|Mannequin" }).Count
    }
}

# Save analysis results
$analysis | ConvertTo-Json -Depth 10 | Out-File -FilePath $OutputPath -Encoding UTF8

Write-Host ""
Write-Host "=== Analysis Summary ===" -ForegroundColor Cyan
Write-Host "Total Materials: $($analysis.Summary.TotalMaterials)" -ForegroundColor White
Write-Host "Master Materials: $($analysis.Summary.MasterMaterials)" -ForegroundColor White
Write-Host "Material Instances: $($analysis.Summary.MaterialInstances)" -ForegroundColor White
Write-Host "Material Functions: $($analysis.Summary.MaterialFunctions)" -ForegroundColor White
Write-Host "PostProcess Materials: $($analysis.Summary.PostProcessMaterials)" -ForegroundColor White

Write-Host ""
Write-Host "=== Complexity Distribution ===" -ForegroundColor Cyan
Write-Host "Very High (>100KB): $($analysis.ComplexityDistribution.VeryHigh)" -ForegroundColor Red
Write-Host "High (50-100KB): $($analysis.ComplexityDistribution.High)" -ForegroundColor Yellow
Write-Host "Medium (20-50KB): $($analysis.ComplexityDistribution.Medium)" -ForegroundColor Green
Write-Host "Low (<20KB): $($analysis.ComplexityDistribution.Low)" -ForegroundColor Green

Write-Host ""
Write-Host "=== Category Distribution ===" -ForegroundColor Cyan
Write-Host "PostProcess: $($analysis.CategoryDistribution.PostProcess)" -ForegroundColor White
Write-Host "Architecture: $($analysis.CategoryDistribution.Architecture)" -ForegroundColor White
Write-Host "Lighting: $($analysis.CategoryDistribution.Lighting)" -ForegroundColor White
Write-Host "Translucent: $($analysis.CategoryDistribution.Translucent)" -ForegroundColor White
Write-Host "Props: $($analysis.CategoryDistribution.Props)" -ForegroundColor White
Write-Host "Character: $($analysis.CategoryDistribution.Character)" -ForegroundColor White

Write-Host ""
Write-Host "Analysis results saved to: $OutputPath" -ForegroundColor Green
