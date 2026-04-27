# Lighting Optimization Tool for Horror Project
# Optimizes Lumen, lightmaps, shadows, and reflections

param(
    [string]$ProjectPath = "",
    [string]$MapPath = "/Game/Horror/Maps/MainLevel"
)

. (Join-Path (Split-Path -Parent $PSScriptRoot) "Validation\Common.ps1")

if ([string]::IsNullOrWhiteSpace($ProjectPath)) {
    $ProjectPath = Get-HorrorProjectRoot -StartPath $PSScriptRoot
} else {
    $ProjectPath = (Resolve-Path -LiteralPath $ProjectPath).Path
}

$UE5Root = Get-HorrorUERoot
$UEEditorCmd = Get-HorrorEditorCmd -UERoot $UE5Root
$ProjectFile = Get-HorrorProjectFile -ProjectRoot $ProjectPath

Write-Host "=== Lighting Optimization Tool ===" -ForegroundColor Cyan

# Python script for lighting optimization
$PythonScript = @"
import unreal
import json

def optimize_lightmap_settings(actor):
    """Optimize lightmap settings for static mesh actor"""
    try:
        static_mesh_component = actor.get_component_by_class(unreal.StaticMeshComponent)
        if not static_mesh_component:
            return False, "No static mesh component"

        modified = False

        # Get static mesh
        static_mesh = static_mesh_component.get_editor_property('static_mesh')
        if not static_mesh:
            return False, "No static mesh"

        # Optimize lightmap resolution based on mesh size
        bounds = static_mesh_component.get_editor_property('bounds')
        if bounds:
            box_extent = bounds.box_extent
            max_extent = max(box_extent.x, box_extent.y, box_extent.z)

            # Calculate optimal lightmap resolution
            if max_extent < 100:
                optimal_res = 32
            elif max_extent < 500:
                optimal_res = 64
            elif max_extent < 1000:
                optimal_res = 128
            elif max_extent < 2000:
                optimal_res = 256
            else:
                optimal_res = 512

            current_res = static_mesh.get_editor_property('light_map_resolution')
            if current_res != optimal_res:
                static_mesh.set_editor_property('light_map_resolution', optimal_res)
                unreal.EditorAssetLibrary.save_loaded_asset(static_mesh)
                modified = True

        return modified, "Optimized" if modified else "Already optimal"

    except Exception as e:
        return False, str(e)

def optimize_lumen_settings(world):
    """Optimize Lumen global illumination settings"""
    try:
        # Get post process volume
        all_actors = unreal.EditorLevelLibrary.get_all_level_actors()
        post_process_volumes = [a for a in all_actors if isinstance(a, unreal.PostProcessVolume)]

        if not post_process_volumes:
            unreal.log_warning("No post process volume found, creating one...")
            # Create post process volume
            location = unreal.Vector(0, 0, 0)
            rotation = unreal.Rotator(0, 0, 0)
            ppv = unreal.EditorLevelLibrary.spawn_actor_from_class(
                unreal.PostProcessVolume,
                location,
                rotation
            )
            ppv.set_actor_label("GlobalPostProcess")
            ppv.set_editor_property('unbound', True)
            post_process_volumes = [ppv]

        # Optimize each post process volume
        for ppv in post_process_volumes:
            settings = ppv.get_editor_property('settings')

            # Enable Lumen
            settings.set_editor_property('method', unreal.DynamicGlobalIlluminationMethod.LUMEN)

            # Lumen quality settings for horror game (balanced)
            # Final gather quality
            settings.set_editor_property('lumen_scene_lighting_quality', 1.0)
            settings.set_editor_property('lumen_scene_detail', 1.0)

            # Reflection quality
            settings.set_editor_property('lumen_reflection_quality', 1.0)

            # Screen traces (important for close-up detail)
            settings.set_editor_property('lumen_screen_traces', True)

            # Ray lighting mode (surface cache for performance)
            settings.set_editor_property('lumen_ray_lighting_mode', unreal.LumenRayLightingMode.SURFACE_CACHE)

            ppv.set_editor_property('settings', settings)

        return True, f"Optimized {len(post_process_volumes)} post process volumes"

    except Exception as e:
        return False, str(e)

def optimize_shadow_settings(light_actor):
    """Optimize shadow settings for lights"""
    try:
        light_component = light_actor.get_component_by_class(unreal.LightComponent)
        if not light_component:
            return False, "No light component"

        modified = False

        # Get light type
        if isinstance(light_component, unreal.DirectionalLightComponent):
            # Directional light (sun)
            # Use cascaded shadow maps
            light_component.set_editor_property('dynamic_shadow_cascades', 4)
            light_component.set_editor_property('cascade_distribution_exponent', 3.0)
            modified = True

        elif isinstance(light_component, unreal.PointLightComponent):
            # Point light
            # Optimize shadow resolution
            current_res = light_component.get_editor_property('shadow_resolution_scale')
            if current_res > 1.0:
                light_component.set_editor_property('shadow_resolution_scale', 1.0)
                modified = True

        elif isinstance(light_component, unreal.SpotLightComponent):
            # Spot light
            # Optimize shadow resolution
            current_res = light_component.get_editor_property('shadow_resolution_scale')
            if current_res > 1.0:
                light_component.set_editor_property('shadow_resolution_scale', 1.0)
                modified = True

        # Enable contact shadows for all lights (good for horror atmosphere)
        light_component.set_editor_property('cast_contact_shadow', True)
        modified = True

        return modified, "Optimized" if modified else "Already optimal"

    except Exception as e:
        return False, str(e)

def optimize_reflection_captures():
    """Optimize reflection capture placement and settings"""
    try:
        all_actors = unreal.EditorLevelLibrary.get_all_level_actors()
        reflection_captures = [a for a in all_actors if isinstance(a, unreal.SphereReflectionCapture)]

        optimized = 0
        for capture in reflection_captures:
            component = capture.get_component_by_class(unreal.ReflectionCaptureComponent)
            if component:
                # Set reasonable resolution (512 is good for most cases)
                component.set_editor_property('cubemap_resolution', 512)
                optimized += 1

        return True, f"Optimized {optimized} reflection captures"

    except Exception as e:
        return False, str(e)

def analyze_lighting_setup():
    """Analyze current lighting setup"""
    all_actors = unreal.EditorLevelLibrary.get_all_level_actors()

    stats = {
        'directional_lights': 0,
        'point_lights': 0,
        'spot_lights': 0,
        'rect_lights': 0,
        'static_meshes': 0,
        'reflection_captures': 0,
        'post_process_volumes': 0,
        'lightmass_importance_volumes': 0
    }

    for actor in all_actors:
        if isinstance(actor, unreal.DirectionalLight):
            stats['directional_lights'] += 1
        elif isinstance(actor, unreal.PointLight):
            stats['point_lights'] += 1
        elif isinstance(actor, unreal.SpotLight):
            stats['spot_lights'] += 1
        elif isinstance(actor, unreal.RectLight):
            stats['rect_lights'] += 1
        elif isinstance(actor, unreal.StaticMeshActor):
            stats['static_meshes'] += 1
        elif isinstance(actor, unreal.SphereReflectionCapture):
            stats['reflection_captures'] += 1
        elif isinstance(actor, unreal.PostProcessVolume):
            stats['post_process_volumes'] += 1
        elif isinstance(actor, unreal.LightmassImportanceVolume):
            stats['lightmass_importance_volumes'] += 1

    return stats

def optimize_level_lighting():
    """Main optimization function"""
    results = {
        'lightmaps_optimized': 0,
        'lights_optimized': 0,
        'lumen_configured': False,
        'reflections_optimized': False,
        'stats': {},
        'errors': []
    }

    try:
        # Get world
        world = unreal.EditorLevelLibrary.get_editor_world()
        if not world:
            results['errors'].append("Failed to get editor world")
            return results

        # Analyze current setup
        results['stats'] = analyze_lighting_setup()
        unreal.log(f"Found {results['stats']['static_meshes']} static meshes")
        unreal.log(f"Found {results['stats']['point_lights']} point lights")
        unreal.log(f"Found {results['stats']['spot_lights']} spot lights")

        # Optimize Lumen settings
        success, message = optimize_lumen_settings(world)
        if success:
            results['lumen_configured'] = True
            unreal.log(f"Lumen: {message}")
        else:
            results['errors'].append(f"Lumen optimization failed: {message}")

        # Optimize reflection captures
        success, message = optimize_reflection_captures()
        if success:
            results['reflections_optimized'] = True
            unreal.log(f"Reflections: {message}")
        else:
            results['errors'].append(f"Reflection optimization failed: {message}")

        # Optimize all actors
        all_actors = unreal.EditorLevelLibrary.get_all_level_actors()

        for actor in all_actors:
            # Optimize lightmaps for static meshes
            if isinstance(actor, unreal.StaticMeshActor):
                success, message = optimize_lightmap_settings(actor)
                if success and message == "Optimized":
                    results['lightmaps_optimized'] += 1

            # Optimize lights
            elif isinstance(actor, (unreal.DirectionalLight, unreal.PointLight, unreal.SpotLight)):
                success, message = optimize_shadow_settings(actor)
                if success and message == "Optimized":
                    results['lights_optimized'] += 1

        unreal.log(f"Optimized {results['lightmaps_optimized']} lightmap settings")
        unreal.log(f"Optimized {results['lights_optimized']} light settings")

    except Exception as e:
        results['errors'].append(f"Optimization error: {str(e)}")

    return results

# Run optimization
unreal.log("Starting lighting optimization...")
results = optimize_level_lighting()

# Save results
output_path = r'$ProjectPath\Scripts\Assets\LightingOptimizationReport.json'
with open(output_path, 'w') as f:
    json.dump(results, f, indent=2)

unreal.log("Lighting optimization complete!")
unreal.log(f"Lightmaps optimized: {results['lightmaps_optimized']}")
unreal.log(f"Lights optimized: {results['lights_optimized']}")
unreal.log(f"Lumen configured: {results['lumen_configured']}")
unreal.log(f"Reflections optimized: {results['reflections_optimized']}")

if results['errors']:
    unreal.log_warning(f"Errors encountered: {len(results['errors'])}")
    for error in results['errors']:
        unreal.log_warning(f"  - {error}")
"@

# Save Python script
$TempPyScript = "$ProjectPath\Saved\optimize_lighting_temp.py"
$PythonScript | Out-File -FilePath $TempPyScript -Encoding UTF8

Write-Host "`nOptimizing lighting..." -ForegroundColor Yellow
Write-Host "Map: $MapPath"

# Run UE Python script
& $UEEditorCmd $ProjectFile -ExecutePythonScript=$TempPyScript -stdout -unattended -nopause -nosplash -log

# Check results
$ResultsPath = "$ProjectPath\Scripts\Assets\LightingOptimizationReport.json"
if (Test-Path $ResultsPath) {
    Write-Host "`n=== Lighting Optimization Complete ===" -ForegroundColor Green
    $results = Get-Content $ResultsPath | ConvertFrom-Json

    Write-Host "`nResults:" -ForegroundColor Cyan
    Write-Host "  Lightmaps optimized: $($results.lightmaps_optimized)" -ForegroundColor Green
    Write-Host "  Lights optimized: $($results.lights_optimized)" -ForegroundColor Green
    Write-Host "  Lumen configured: $($results.lumen_configured)" -ForegroundColor $(if($results.lumen_configured){'Green'}else{'Red'})
    Write-Host "  Reflections optimized: $($results.reflections_optimized)" -ForegroundColor $(if($results.reflections_optimized){'Green'}else{'Red'})

    Write-Host "`nScene Statistics:" -ForegroundColor Cyan
    Write-Host "  Static Meshes: $($results.stats.static_meshes)"
    Write-Host "  Directional Lights: $($results.stats.directional_lights)"
    Write-Host "  Point Lights: $($results.stats.point_lights)"
    Write-Host "  Spot Lights: $($results.stats.spot_lights)"
    Write-Host "  Reflection Captures: $($results.stats.reflection_captures)"
    Write-Host "  Post Process Volumes: $($results.stats.post_process_volumes)"

    if ($results.errors.Count -gt 0) {
        Write-Host "`nErrors:" -ForegroundColor Red
        $results.errors | ForEach-Object { Write-Host "  - $_" }
    }
} else {
    Write-Host "`nError: Lighting optimization failed. Check UE logs." -ForegroundColor Red
}

# Cleanup
Remove-Item $TempPyScript -ErrorAction SilentlyContinue
