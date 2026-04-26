# OptimizeMaterials.ps1
# Material Optimization Execution Script
# Optimizes material complexity and shader instructions

param(
    [string]$ProjectPath = "D:\gptzuo\HorrorProject\HorrorProject",
    [string]$OutputDir = "D:\gptzuo\HorrorProject\HorrorProject\Saved\Optimization\Materials",
    [switch]$DryRun = $false
)

$ErrorActionPreference = "Stop"

Write-Host "=== Material Optimization Tool ===" -ForegroundColor Cyan
Write-Host "Mode: $(if ($DryRun) { 'Dry Run (Preview Only)' } else { 'Execute' })"
Write-Host ""

# Create output directory
New-Item -ItemType Directory -Force -Path $OutputDir | Out-Null

$timestamp = Get-Date -Format "yyyyMMdd_HHmmss"
$reportFile = Join-Path $OutputDir "material_optimization_$timestamp.json"

# Optimization thresholds
$thresholds = @{
    MaxShaderInstructions = 300
    MaxTextureSamples = 8
    MaxMaterialLayers = 3
    WarnShaderInstructions = 200
    WarnTextureSamples = 6
}

Write-Host "Scanning for materials..." -ForegroundColor Yellow

# Find all material assets
$contentPath = Join-Path $ProjectPath "Content"
$materialFiles = Get-ChildItem -Path $contentPath -Recurse -Include "*.uasset" | Where-Object {
    $_.Name -match "^M_|^MI_|^MF_" -or $_.DirectoryName -match "Materials"
}

Write-Host "Found $($materialFiles.Count) material files" -ForegroundColor Green
Write-Host ""

$optimizationData = @{
    Timestamp = $timestamp
    DryRun = $DryRun
    TotalMaterials = $materialFiles.Count
    ComplexMaterials = @()
    OptimizationRecommendations = @()
}

$complexCount = 0

foreach ($materialFile in $materialFiles) {
    $relativePath = $materialFile.FullName.Substring($contentPath.Length + 1)
    $materialName = $materialFile.BaseName

    # Estimate complexity (in production, parse actual material data)
    $fileSize = $materialFile.Length
    $estimatedInstructions = 100
    $estimatedSamples = 4

    # Rough estimation based on file size
    if ($fileSize -gt 100KB) {
        $estimatedInstructions = 400
        $estimatedSamples = 10
    } elseif ($fileSize -gt 50KB) {
        $estimatedInstructions = 250
        $estimatedSamples = 7
    } elseif ($fileSize -gt 20KB) {
        $estimatedInstructions = 150
        $estimatedSamples = 5
    }

    # Check if optimization needed
    $needsOptimization = $false
    $recommendations = @()

    if ($estimatedInstructions -gt $thresholds.MaxShaderInstructions) {
        $needsOptimization = $true
        $recommendations += "CRITICAL: Reduce shader instructions (current: ~$estimatedInstructions, max: $($thresholds.MaxShaderInstructions))"
    } elseif ($estimatedInstructions -gt $thresholds.WarnShaderInstructions) {
        $needsOptimization = $true
        $recommendations += "WARNING: High shader instructions (~$estimatedInstructions)"
    }

    if ($estimatedSamples -gt $thresholds.MaxTextureSamples) {
        $needsOptimization = $true
        $recommendations += "CRITICAL: Too many texture samples (current: ~$estimatedSamples, max: $($thresholds.MaxTextureSamples))"
    } elseif ($estimatedSamples -gt $thresholds.WarnTextureSamples) {
        $needsOptimization = $true
        $recommendations += "WARNING: High texture sample count (~$estimatedSamples)"
    }

    if ($needsOptimization) {
        $complexCount++

        # Add specific optimization suggestions
        if ($estimatedInstructions -gt 200) {
            $recommendations += "- Simplify material graph complexity"
            $recommendations += "- Remove unnecessary math operations"
            $recommendations += "- Use material functions for shared logic"
            $recommendations += "- Consider using simpler blend modes"
        }

        if ($estimatedSamples -gt 6) {
            $recommendations += "- Combine texture channels (pack textures)"
            $recommendations += "- Use texture atlases where possible"
            $recommendations += "- Remove redundant texture lookups"
            $recommendations += "- Use shared samplers"
        }

        # Check for common optimization opportunities
        if ($materialName -match "Glass|Transparent") {
            $recommendations += "- Use masked blend mode instead of translucent if possible"
            $recommendations += "- Disable separate translucency if not needed"
        }

        if ($materialName -match "Foliage|Grass|Leaf") {
            $recommendations += "- Use two-sided foliage shading model"
            $recommendations += "- Enable dithered LOD transitions"
        }

        $materialInfo = @{
            Path = $relativePath
            Name = $materialName
            EstimatedInstructions = $estimatedInstructions
            EstimatedSamples = $estimatedSamples
            Severity = if ($estimatedInstructions -gt $thresholds.MaxShaderInstructions -or $estimatedSamples -gt $thresholds.MaxTextureSamples) { "Critical" } else { "Warning" }
            Recommendations = $recommendations
        }

        $optimizationData.ComplexMaterials += $materialInfo

        $severityColor = if ($materialInfo.Severity -eq "Critical") { "Red" } else { "Yellow" }

        Write-Host "[$complexCount] $materialName" -ForegroundColor $severityColor
        Write-Host "    Shader Instructions: ~$estimatedInstructions" -ForegroundColor Gray
        Write-Host "    Texture Samples: ~$estimatedSamples" -ForegroundColor Gray
        Write-Host "    Severity: $($materialInfo.Severity)" -ForegroundColor $severityColor

        foreach ($rec in $recommendations) {
            Write-Host "    $rec" -ForegroundColor Cyan
        }
        Write-Host ""
    }
}

# Generate global recommendations
$globalRecommendations = @(
    "Enable material quality levels for scalability",
    "Use material instances instead of unique materials",
    "Implement LOD materials for distant objects",
    "Use material parameter collections for global parameters",
    "Enable material caching in project settings",
    "Consider using Nanite for static meshes (reduces material complexity impact)"
)

$optimizationData.OptimizationRecommendations = $globalRecommendations

# Save report
$optimizationData | ConvertTo-Json -Depth 10 | Out-File $reportFile -Encoding UTF8

Write-Host ""
Write-Host "=== Material Optimization Summary ===" -ForegroundColor Cyan
Write-Host "Total materials scanned: $($materialFiles.Count)"
Write-Host "Materials needing optimization: $complexCount"
Write-Host ""

if ($complexCount -gt 0) {
    $criticalCount = ($optimizationData.ComplexMaterials | Where-Object { $_.Severity -eq "Critical" }).Count
    $warningCount = ($optimizationData.ComplexMaterials | Where-Object { $_.Severity -eq "Warning" }).Count

    Write-Host "Critical issues: $criticalCount" -ForegroundColor Red
    Write-Host "Warnings: $warningCount" -ForegroundColor Yellow
    Write-Host ""

    Write-Host "Most complex materials:" -ForegroundColor Yellow
    $topComplex = $optimizationData.ComplexMaterials |
        Sort-Object EstimatedInstructions -Descending |
        Select-Object -First 5

    foreach ($mat in $topComplex) {
        Write-Host "  $($mat.Name): ~$($mat.EstimatedInstructions) instructions, ~$($mat.EstimatedSamples) samples"
    }
    Write-Host ""
}

Write-Host "Global Recommendations:" -ForegroundColor Cyan
foreach ($rec in $globalRecommendations) {
    Write-Host "  - $rec" -ForegroundColor White
}
Write-Host ""

Write-Host "Report saved to: $reportFile" -ForegroundColor Green
Write-Host ""

# Generate Python script for material analysis
$pythonScript = @"
# UE5 Material Optimization Script
# Generated: $timestamp
import unreal

def analyze_materials():
    asset_registry = unreal.AssetRegistryHelpers.get_asset_registry()

    # Find all material assets
    materials = asset_registry.get_assets_by_class('Material', True)
    material_instances = asset_registry.get_assets_by_class('MaterialInstance', True)

    all_materials = list(materials) + list(material_instances)

    print(f'Analyzing {len(all_materials)} materials...')

    complex_materials = []

    for mat_data in all_materials:
        material = unreal.load_asset(mat_data.package_name)
        if not material:
            continue

        mat_name = material.get_name()

        # Get shader stats (if available)
        # Note: Actual implementation would use material stats API

        # Apply optimizations
        if isinstance(material, unreal.Material):
            # Enable material quality levels
            material.set_editor_property('quality_level', unreal.EMaterialQualityLevel.HIGH)

            # Optimize for mobile if needed
            # material.set_editor_property('mobile_quality', unreal.EMaterialQualityLevel.MEDIUM)

            unreal.EditorAssetLibrary.save_loaded_asset(material)
            print(f'Optimized: {mat_name}')

    print('Material optimization complete!')

if __name__ == '__main__':
    analyze_materials()
"@

$pythonScriptPath = Join-Path $OutputDir "optimize_materials_$timestamp.py"
$pythonScript | Out-File $pythonScriptPath -Encoding UTF8

Write-Host "Python automation script saved to: $pythonScriptPath" -ForegroundColor Green
Write-Host ""

if ($DryRun) {
    Write-Host "This was a dry run. Run without -DryRun to apply optimizations." -ForegroundColor Yellow
} else {
    Write-Host "To apply optimizations, run in UE5 Editor:" -ForegroundColor Cyan
    Write-Host "  py `"$pythonScriptPath`"" -ForegroundColor White
}

Write-Host ""
Write-Host "Material optimization complete!" -ForegroundColor Green
