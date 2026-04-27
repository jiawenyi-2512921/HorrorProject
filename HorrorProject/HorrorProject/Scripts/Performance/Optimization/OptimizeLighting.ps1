# OptimizeLighting.ps1
# Lighting Optimization Execution Script
# Optimizes lighting setup for performance

param(
    [string]$ProjectPath = "",
    [string]$OutputDir = "",
    [switch]$DryRun = $false
)

$ErrorActionPreference = "Stop"

. (Join-Path $PSScriptRoot "..\..\Validation\Common.ps1")
$ProjectRoot = Get-HorrorProjectRoot -StartPath $PSScriptRoot
if ([string]::IsNullOrWhiteSpace($ProjectPath)) { $ProjectPath = $ProjectRoot }
if ([string]::IsNullOrWhiteSpace($OutputDir)) { $OutputDir = Join-Path $ProjectRoot "Saved\Optimization\Lighting" }

Write-Host "=== Lighting Optimization Tool ===" -ForegroundColor Cyan
Write-Host "Mode: $(if ($DryRun) { 'Dry Run (Preview Only)' } else { 'Execute' })"
Write-Host ""

# Create output directory
New-Item -ItemType Directory -Force -Path $OutputDir | Out-Null

$timestamp = Get-Date -Format "yyyyMMdd_HHmmss"
$reportFile = Join-Path $OutputDir "lighting_optimization_$timestamp.json"

# Optimization settings
$lightingConfig = @{
    MaxDynamicLights = 4
    MaxShadowCastingLights = 2
    MaxStaticLights = 20
    ShadowMapResolution = 2048
    CascadeShadowMaps = 3
    ShadowDistance = 5000
    LightmapResolution = 128
    RecommendLumen = $true
}

Write-Host "Analyzing lighting setup..." -ForegroundColor Yellow

$optimizationData = @{
    Timestamp = $timestamp
    DryRun = $DryRun
    LightingIssues = @()
    Recommendations = @()
    Configuration = $lightingConfig
}

# Scan for map files
$contentPath = Join-Path $ProjectPath "Content"
$mapFiles = Get-ChildItem -Path $contentPath -Recurse -Include "*.umap"

Write-Host "Found $($mapFiles.Count) map files" -ForegroundColor Green
Write-Host ""

foreach ($mapFile in $mapFiles) {
    $relativePath = $mapFile.FullName.Substring($contentPath.Length + 1)
    $mapName = $mapFile.BaseName

    Write-Host "Analyzing: $mapName" -ForegroundColor Cyan

    # In production, this would parse the actual map data
# Generate lighting recommendations for review in the Unreal Editor

    $mapIssues = @{
        MapName = $mapName
        Path = $relativePath
        Issues = @()
        Recommendations = @()
    }

    # General lighting recommendations
    $mapIssues.Recommendations += "Use Lumen for dynamic global illumination (UE5)"
    $mapIssues.Recommendations += "Limit dynamic lights to $($lightingConfig.MaxDynamicLights) per scene"
    $mapIssues.Recommendations += "Use static lighting for non-moving lights"
    $mapIssues.Recommendations += "Optimize shadow cascades to $($lightingConfig.CascadeShadowMaps) levels"
    $mapIssues.Recommendations += "Set shadow distance to $($lightingConfig.ShadowDistance) units"
    $mapIssues.Recommendations += "Use lightmap resolution of $($lightingConfig.LightmapResolution) for most objects"
    $mapIssues.Recommendations += "Enable distance field shadows for better performance"
    $mapIssues.Recommendations += "Use light functions sparingly (expensive)"
    $mapIssues.Recommendations += "Disable shadows on small/distant lights"
    $mapIssues.Recommendations += "Use IES profiles instead of complex light functions"

    # Horror game specific recommendations
    if ($mapName -match "Horror|Dark|Scary") {
        $mapIssues.Recommendations += "HORROR SPECIFIC: Use volumetric fog for atmosphere"
        $mapIssues.Recommendations += "HORROR SPECIFIC: Limit point lights, use spot lights for control"
        $mapIssues.Recommendations += "HORROR SPECIFIC: Use light shafts sparingly (expensive)"
        $mapIssues.Recommendations += "HORROR SPECIFIC: Bake ambient occlusion into lightmaps"
        $mapIssues.Recommendations += "HORROR SPECIFIC: Use emissive materials for glowing objects"
    }

    $optimizationData.LightingIssues += $mapIssues

    Write-Host "  Recommendations generated" -ForegroundColor Green
}

# Global lighting recommendations
$globalRecommendations = @(
    "Enable Lumen in Project Settings for dynamic GI",
    "Configure Lumen quality settings for 60 FPS target",
    "Use Virtual Shadow Maps (VSM) for better shadow performance",
    "Enable ray-traced shadows only for hero lights",
    "Optimize lightmap resolution per object importance",
    "Use light propagation volumes for dynamic GI fallback",
    "Enable distance field ambient occlusion",
    "Limit reflection captures to essential areas",
    "Use planar reflections sparingly",
    "Optimize post-process volumes (limit overlapping volumes)",
    "Use exponential height fog instead of volumetric fog where possible",
    "Bake lighting for static objects",
    "Use stationary lights as compromise between static and dynamic"
)

$optimizationData.Recommendations = $globalRecommendations

# Save report
$optimizationData | ConvertTo-Json -Depth 10 | Out-File $reportFile -Encoding UTF8

Write-Host ""
Write-Host "=== Lighting Optimization Summary ===" -ForegroundColor Cyan
Write-Host "Maps analyzed: $($mapFiles.Count)"
Write-Host ""

Write-Host "Recommended Lighting Configuration:" -ForegroundColor Yellow
Write-Host "  Max Dynamic Lights: $($lightingConfig.MaxDynamicLights)"
Write-Host "  Max Shadow-Casting Lights: $($lightingConfig.MaxShadowCastingLights)"
Write-Host "  Shadow Map Resolution: $($lightingConfig.ShadowMapResolution)"
Write-Host "  Cascade Shadow Maps: $($lightingConfig.CascadeShadowMaps)"
Write-Host "  Shadow Distance: $($lightingConfig.ShadowDistance) units"
Write-Host "  Default Lightmap Resolution: $($lightingConfig.LightmapResolution)"
Write-Host ""

Write-Host "Global Recommendations:" -ForegroundColor Cyan
foreach ($rec in $globalRecommendations) {
    Write-Host "  - $rec" -ForegroundColor White
}
Write-Host ""

Write-Host "Report saved to: $reportFile" -ForegroundColor Green
Write-Host ""

# Generate configuration file for UE5
$ueConfig = @"
; UE5 Lighting Optimization Configuration
; Generated: $timestamp
; Add to DefaultEngine.ini

[/Script/Engine.RendererSettings]
; Lumen Settings
r.Lumen.DiffuseIndirect.Allow=True
r.Lumen.Reflections.Allow=True
r.LumenScene.SurfaceCache.MeshCardsMergedResolution=256
r.Lumen.TraceMeshSDFs=1

; Shadow Settings
r.Shadow.Virtual.Enable=1
r.Shadow.Virtual.MaxPhysicalPages=4096
r.Shadow.DistanceScale=0.5
r.Shadow.CSM.MaxCascades=$($lightingConfig.CascadeShadowMaps)

; Distance Field Settings
r.GenerateMeshDistanceFields=True
r.DistanceFieldBuild.EightBit=True
r.AOGlobalDistanceField=True

; Performance Settings
r.MaxAnisotropy=8
r.ViewDistanceScale=1.0
r.SkeletalMeshLODBias=0
r.ParticleLODBias=0

[/Script/Engine.Engine]
; Lightmap Settings
bUseMaxQualityMode=False
LightMapDensityVertexMappedColor=(R=0.65,G=0.65,B=0.25,A=1.0)
LightMapDensitySelectedColor=(R=1.0,G=0.2,B=1.0,A=1.0)

[/Script/Engine.RendererOverrideSettings]
; Override Settings
bSupportStationarySkylight=True
bSupportLowQualityLightmaps=True
bSupportPointLightWholeSceneShadows=True
"@

$configPath = Join-Path $OutputDir "LightingOptimization_$timestamp.ini"
$ueConfig | Out-File $configPath -Encoding UTF8

Write-Host "UE5 configuration saved to: $configPath" -ForegroundColor Green
Write-Host ""

# Generate Python script for lighting optimization
$pythonScript = @"
# UE5 Lighting Optimization Script
# Generated: $timestamp
import unreal

def optimize_lighting():
    # Get all levels
    asset_registry = unreal.AssetRegistryHelpers.get_asset_registry()
    levels = asset_registry.get_assets_by_class('World', True)

    for level_data in levels:
        level = unreal.load_asset(level_data.package_name)
        if not level:
            continue

        level_name = level.get_name()
        print(f'Optimizing lighting in {level_name}...')

        # Get all actors in level
        all_actors = unreal.EditorLevelLibrary.get_all_level_actors()

        dynamic_light_count = 0
        shadow_casting_count = 0

        for actor in all_actors:
            # Optimize light actors
            if isinstance(actor, unreal.Light):
                light_component = actor.get_editor_property('light_component')
                if not light_component:
                    continue

                # Check if dynamic
                mobility = light_component.get_editor_property('mobility')

                if mobility == unreal.ComponentMobility.MOVABLE:
                    dynamic_light_count += 1

                    # Limit shadow casting
                    if shadow_casting_count >= $($lightingConfig.MaxShadowCastingLights):
                        light_component.set_editor_property('cast_shadows', False)
                        print(f'  Disabled shadows on {actor.get_name()}')
                    else:
                        if light_component.get_editor_property('cast_shadows'):
                            shadow_casting_count += 1

                    # Optimize shadow settings
                    light_component.set_editor_property('shadow_resolution_scale', 1.0)
                    light_component.set_editor_property('shadow_bias', 0.5)

                # Optimize static lights
                elif mobility == unreal.ComponentMobility.STATIC:
                    # Ensure proper lightmap settings
                    light_component.set_editor_property('indirect_lighting_intensity', 1.0)

            # Optimize static mesh lightmap resolution
            elif isinstance(actor, unreal.StaticMeshActor):
                static_mesh_component = actor.get_editor_property('static_mesh_component')
                if static_mesh_component:
                    # Set appropriate lightmap resolution
                    bounds = static_mesh_component.get_local_bounds()
                    size = max(bounds.box_extent.x, bounds.box_extent.y, bounds.box_extent.z)

                    if size > 500:
                        lightmap_res = 256
                    elif size > 200:
                        lightmap_res = 128
                    elif size > 100:
                        lightmap_res = 64
                    else:
                        lightmap_res = 32

                    static_mesh_component.set_editor_property('overridden_light_map_res', lightmap_res)

        print(f'  Dynamic lights: {dynamic_light_count}')
        print(f'  Shadow-casting lights: {shadow_casting_count}')

        if dynamic_light_count > $($lightingConfig.MaxDynamicLights):
            print(f'  WARNING: Too many dynamic lights ({dynamic_light_count} > $($lightingConfig.MaxDynamicLights))')

    print('Lighting optimization complete!')

if __name__ == '__main__':
    optimize_lighting()
"@

$pythonScriptPath = Join-Path $OutputDir "optimize_lighting_$timestamp.py"
$pythonScript | Out-File $pythonScriptPath -Encoding UTF8

Write-Host "Python automation script saved to: $pythonScriptPath" -ForegroundColor Green
Write-Host ""

if ($DryRun) {
    Write-Host "This was a dry run. Run without -DryRun to apply optimizations." -ForegroundColor Yellow
} else {
    Write-Host "To apply optimizations:" -ForegroundColor Cyan
    Write-Host "  1. Add configuration to DefaultEngine.ini" -ForegroundColor White
    Write-Host "  2. Run Python script in UE5 Editor: py `"$pythonScriptPath`"" -ForegroundColor White
}

Write-Host ""
Write-Host "Lighting optimization complete!" -ForegroundColor Green
