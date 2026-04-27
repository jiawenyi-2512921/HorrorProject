# OptimizeMeshes.ps1
# Mesh Optimization Execution Script
# Optimizes mesh complexity and LOD settings

param(
    [string]$ProjectPath = "",
    [string]$OutputDir = "",
    [switch]$DryRun = $false
)

$ErrorActionPreference = "Stop"

. (Join-Path $PSScriptRoot "..\..\Validation\Common.ps1")
$ProjectRoot = Get-HorrorProjectRoot -StartPath $PSScriptRoot
if ([string]::IsNullOrWhiteSpace($ProjectPath)) { $ProjectPath = $ProjectRoot }
if ([string]::IsNullOrWhiteSpace($OutputDir)) { $OutputDir = Join-Path $ProjectRoot "Saved\Optimization\Meshes" }

Write-Host "=== Mesh Optimization Tool ===" -ForegroundColor Cyan
Write-Host "Mode: $(if ($DryRun) { 'Dry Run (Preview Only)' } else { 'Execute' })"
Write-Host ""

# Create output directory
New-Item -ItemType Directory -Force -Path $OutputDir | Out-Null

$timestamp = Get-Date -Format "yyyyMMdd_HHmmss"
$reportFile = Join-Path $OutputDir "mesh_optimization_$timestamp.json"

# Optimization thresholds
$thresholds = @{
    MaxTriangles = 50000
    WarnTriangles = 20000
    MinLODLevels = 3
    LOD0Distance = 0
    LOD1Distance = 1000
    LOD2Distance = 2500
    LOD3Distance = 5000
    LOD0ReductionPercent = 0
    LOD1ReductionPercent = 50
    LOD2ReductionPercent = 75
    LOD3ReductionPercent = 90
}

Write-Host "Scanning for meshes..." -ForegroundColor Yellow

# Find all static mesh assets
$contentPath = Join-Path $ProjectPath "Content"
$meshFiles = Get-ChildItem -Path $contentPath -Recurse -Include "*.uasset" | Where-Object {
    $_.Name -match "^SM_|^S_" -or $_.DirectoryName -match "Meshes|StaticMeshes"
}

Write-Host "Found $($meshFiles.Count) mesh files" -ForegroundColor Green
Write-Host ""

$optimizationData = @{
    Timestamp = $timestamp
    DryRun = $DryRun
    TotalMeshes = $meshFiles.Count
    ComplexMeshes = @()
    OptimizationActions = @()
}

$complexCount = 0

foreach ($meshFile in $meshFiles) {
    $relativePath = $meshFile.FullName.Substring($contentPath.Length + 1)
    $meshName = $meshFile.BaseName

    # Estimate triangle count based on file size (rough approximation)
    $fileSize = $meshFile.Length
    $estimatedTriangles = 5000

    if ($fileSize -gt 5MB) { $estimatedTriangles = 100000 }
    elseif ($fileSize -gt 2MB) { $estimatedTriangles = 50000 }
    elseif ($fileSize -gt 1MB) { $estimatedTriangles = 25000 }
    elseif ($fileSize -gt 500KB) { $estimatedTriangles = 10000 }
    elseif ($fileSize -gt 100KB) { $estimatedTriangles = 5000 }
    else { $estimatedTriangles = 1000 }

    # Check if optimization needed
    $needsOptimization = $false
    $recommendations = @()

    if ($estimatedTriangles -gt $thresholds.MaxTriangles) {
        $needsOptimization = $true
        $recommendations += "CRITICAL: Triangle count too high (~$estimatedTriangles, max: $($thresholds.MaxTriangles))"
        $recommendations += "- Reduce mesh complexity in modeling software"
        $recommendations += "- Use decimation/simplification tools"
    } elseif ($estimatedTriangles -gt $thresholds.WarnTriangles) {
        $needsOptimization = $true
        $recommendations += "WARNING: High triangle count (~$estimatedTriangles)"
    }

    # Always recommend LODs for meshes above certain complexity
    if ($estimatedTriangles -gt 5000) {
        $needsOptimization = $true
        $recommendations += "Generate LOD chain with $($thresholds.MinLODLevels) levels"
        $recommendations += "  LOD0: 0m - Full detail"
        $recommendations += "  LOD1: $($thresholds.LOD1Distance / 100)m - $($thresholds.LOD1ReductionPercent)% reduction"
        $recommendations += "  LOD2: $($thresholds.LOD2Distance / 100)m - $($thresholds.LOD2ReductionPercent)% reduction"
        $recommendations += "  LOD3: $($thresholds.LOD3Distance / 100)m - $($thresholds.LOD3ReductionPercent)% reduction"
    }

    # Check for specific mesh types
    if ($meshName -match "Prop|Detail|Small") {
        $recommendations += "- Enable mesh merging for small props"
        $recommendations += "- Consider using instanced static meshes"
    }

    if ($meshName -match "Building|Structure|Large") {
        $recommendations += "- Split into modular pieces if too complex"
        $recommendations += "- Use aggressive LODs for distant viewing"
        $recommendations += "- Consider Nanite for UE5.1+"
    }

    if ($meshName -match "Foliage|Tree|Plant") {
        $recommendations += "- Use foliage system for instancing"
        $recommendations += "- Enable billboard LOD for distant instances"
        $recommendations += "- Use impostor meshes for far distances"
    }

    if ($needsOptimization) {
        $complexCount++

        # Additional optimization actions
        $actions = @(
            "Enable collision complexity: Simple",
            "Enable lightmap UV generation",
            "Set distance field resolution based on size",
            "Enable nanite if mesh is static and complex (>10k tris)"
        )

        $meshInfo = @{
            Path = $relativePath
            Name = $meshName
            EstimatedTriangles = $estimatedTriangles
            FileSize = $fileSize
            Severity = if ($estimatedTriangles -gt $thresholds.MaxTriangles) { "Critical" } else { "Warning" }
            Recommendations = $recommendations
            Actions = $actions
        }

        $optimizationData.ComplexMeshes += $meshInfo

        $severityColor = if ($meshInfo.Severity -eq "Critical") { "Red" } else { "Yellow" }

        Write-Host "[$complexCount] $meshName" -ForegroundColor $severityColor
        Write-Host "    Estimated Triangles: ~$estimatedTriangles" -ForegroundColor Gray
        Write-Host "    File Size: $([math]::Round($fileSize / 1KB, 2)) KB" -ForegroundColor Gray

        foreach ($rec in $recommendations) {
            Write-Host "    $rec" -ForegroundColor Cyan
        }
        Write-Host ""
    }
}

# Global optimization recommendations
$globalRecommendations = @(
    "Enable automatic LOD generation in project settings",
    "Use Hierarchical LOD (HLOD) for large scenes",
    "Enable mesh instancing for repeated objects",
    "Use Nanite for static meshes (UE5.1+)",
    "Implement occlusion culling",
    "Use distance-based mesh streaming",
    "Enable mesh merging for small props",
    "Optimize collision meshes separately from visual meshes"
)

$optimizationData.OptimizationActions = $globalRecommendations

# Save report
$optimizationData | ConvertTo-Json -Depth 10 | Out-File $reportFile -Encoding UTF8

Write-Host ""
Write-Host "=== Mesh Optimization Summary ===" -ForegroundColor Cyan
Write-Host "Total meshes scanned: $($meshFiles.Count)"
Write-Host "Meshes needing optimization: $complexCount"
Write-Host ""

if ($complexCount -gt 0) {
    $criticalCount = ($optimizationData.ComplexMeshes | Where-Object { $_.Severity -eq "Critical" }).Count
    $warningCount = ($optimizationData.ComplexMeshes | Where-Object { $_.Severity -eq "Warning" }).Count

    Write-Host "Critical issues: $criticalCount" -ForegroundColor Red
    Write-Host "Warnings: $warningCount" -ForegroundColor Yellow
    Write-Host ""

    Write-Host "Most complex meshes:" -ForegroundColor Yellow
    $topComplex = $optimizationData.ComplexMeshes |
        Sort-Object EstimatedTriangles -Descending |
        Select-Object -First 5

    foreach ($mesh in $topComplex) {
        Write-Host "  $($mesh.Name): ~$($mesh.EstimatedTriangles) triangles"
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

# Generate Python script for mesh optimization
$pythonScript = @"
# UE5 Mesh Optimization Script
# Generated: $timestamp
import unreal

def optimize_meshes():
    asset_registry = unreal.AssetRegistryHelpers.get_asset_registry()
    editor_util = unreal.EditorAssetLibrary()

    # Find all static mesh assets
    meshes = asset_registry.get_assets_by_class('StaticMesh', True)

    print(f'Optimizing {len(meshes)} meshes...')

    for mesh_data in meshes:
        mesh = unreal.load_asset(mesh_data.package_name)
        if not mesh:
            continue

        mesh_name = mesh.get_name()

        # Get triangle count
        lod0 = mesh.get_static_mesh_description(0)
        if not lod0:
            continue

        # Generate LODs if needed
        lod_count = mesh.get_num_lods()
        if lod_count < $($thresholds.MinLODLevels):
            print(f'Generating LODs for {mesh_name}...')

            # LOD generation settings
            lod_settings = []

            # LOD1
            lod1 = unreal.StaticMeshReductionSettings()
            lod1.percent_triangles = $(100 - $thresholds.LOD1ReductionPercent) / 100.0
            lod1.screen_size = 0.5
            lod_settings.append(lod1)

            # LOD2
            lod2 = unreal.StaticMeshReductionSettings()
            lod2.percent_triangles = $(100 - $thresholds.LOD2ReductionPercent) / 100.0
            lod2.screen_size = 0.25
            lod_settings.append(lod2)

            # LOD3
            lod3 = unreal.StaticMeshReductionSettings()
            lod3.percent_triangles = $(100 - $thresholds.LOD3ReductionPercent) / 100.0
            lod3.screen_size = 0.1
            lod_settings.append(lod3)

            # Apply LOD settings
            mesh.set_num_lods(len(lod_settings) + 1)
            for i, lod_setting in enumerate(lod_settings):
                mesh.set_lod_reduction_settings(i + 1, lod_setting)

        # Optimize collision
        body_setup = mesh.get_editor_property('body_setup')
        if body_setup:
            body_setup.set_editor_property('collision_trace_flag', unreal.CollisionTraceFlag.CTF_USE_SIMPLE_AS_COMPLEX)

        # Enable distance field
        mesh.set_editor_property('generate_mesh_distance_field', True)

        # Save asset
        editor_util.save_loaded_asset(mesh)
        print(f'Optimized: {mesh_name}')

    print('Mesh optimization complete!')

if __name__ == '__main__':
    optimize_meshes()
"@

$pythonScriptPath = Join-Path $OutputDir "optimize_meshes_$timestamp.py"
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
Write-Host "Mesh optimization complete!" -ForegroundColor Green
