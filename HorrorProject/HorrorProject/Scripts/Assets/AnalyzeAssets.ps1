# Asset Analysis Tool for Horror Project
# Analyzes polygon counts, texture resolutions, material complexity

param(
    [string]$ProjectPath = "D:\gptzuo\HorrorProject\HorrorProject",
    [string]$OutputPath = "D:\gptzuo\HorrorProject\HorrorProject\Scripts\Assets\AssetReport.json"
)

$UEEditorCmd = "C:\Program Files\Epic Games\UE_5.6\Engine\Binaries\Win64\UnrealEditor-Cmd.exe"
$ProjectFile = "$ProjectPath\HorrorProject.uproject"

Write-Host "=== Asset Analysis Tool ===" -ForegroundColor Cyan
Write-Host "Project: $ProjectPath"
Write-Host "Output: $OutputPath"

# Python script for asset analysis
$PythonScript = @"
import unreal
import json
import os

def analyze_static_mesh(asset):
    """Analyze static mesh asset"""
    try:
        mesh = asset.get_editor_property('static_mesh') if hasattr(asset, 'get_editor_property') else asset

        # Get LOD count
        lod_count = mesh.get_num_lods() if hasattr(mesh, 'get_num_lods') else 0

        # Get triangle count for LOD0
        render_data = mesh.get_editor_property('render_data') if hasattr(mesh, 'get_editor_property') else None
        triangles = 0
        vertices = 0

        if render_data and len(render_data.lod_resources) > 0:
            lod0 = render_data.lod_resources[0]
            triangles = lod0.get_num_triangles() if hasattr(lod0, 'get_num_triangles') else 0
            vertices = lod0.get_num_vertices() if hasattr(lod0, 'get_num_vertices') else 0

        # Get materials
        materials = mesh.get_editor_property('static_materials') if hasattr(mesh, 'get_editor_property') else []
        material_count = len(materials)

        # Get lightmap resolution
        lightmap_res = mesh.get_editor_property('light_map_resolution') if hasattr(mesh, 'get_editor_property') else 0

        # Check collision
        has_collision = mesh.get_editor_property('body_setup') is not None if hasattr(mesh, 'get_editor_property') else False

        return {
            'lod_count': lod_count,
            'triangles': triangles,
            'vertices': vertices,
            'material_count': material_count,
            'lightmap_resolution': lightmap_res,
            'has_collision': has_collision
        }
    except Exception as e:
        unreal.log_warning(f"Error analyzing mesh: {str(e)}")
        return None

def analyze_texture(asset):
    """Analyze texture asset"""
    try:
        # Get texture dimensions
        width = asset.blueprint_get_size_x() if hasattr(asset, 'blueprint_get_size_x') else 0
        height = asset.blueprint_get_size_y() if hasattr(asset, 'blueprint_get_size_y') else 0

        # Get compression settings
        compression = str(asset.get_editor_property('compression_settings')) if hasattr(asset, 'get_editor_property') else 'Unknown'

        # Get mip gen settings
        mip_gen = str(asset.get_editor_property('mip_gen_settings')) if hasattr(asset, 'get_editor_property') else 'Unknown'

        # Check if virtual texture
        is_virtual = asset.get_editor_property('virtual_texture_streaming') if hasattr(asset, 'get_editor_property') else False

        return {
            'width': width,
            'height': height,
            'compression': compression,
            'mip_gen': mip_gen,
            'is_virtual': is_virtual
        }
    except Exception as e:
        unreal.log_warning(f"Error analyzing texture: {str(e)}")
        return None

def analyze_material(asset):
    """Analyze material asset"""
    try:
        # Get shader complexity (approximate)
        expressions = asset.get_editor_property('expressions') if hasattr(asset, 'get_editor_property') else []
        expression_count = len(expressions) if expressions else 0

        # Get texture sample count
        texture_samples = [e for e in expressions if isinstance(e, unreal.MaterialExpressionTextureSample)] if expressions else []
        texture_sample_count = len(texture_samples)

        # Check if material instance
        is_instance = isinstance(asset, unreal.MaterialInstance)

        return {
            'expression_count': expression_count,
            'texture_sample_count': texture_sample_count,
            'is_instance': is_instance
        }
    except Exception as e:
        unreal.log_warning(f"Error analyzing material: {str(e)}")
        return None

def scan_assets():
    """Scan all environment assets"""
    asset_registry = unreal.AssetRegistryHelpers.get_asset_registry()

    # Target paths
    paths = [
        '/Game/DeepWaterStation',
        '/Game/SD_Art',
        '/Game/Fab/Modular_Fence_Industrial_Prop'
    ]

    results = {
        'static_meshes': [],
        'textures': [],
        'materials': [],
        'summary': {
            'total_meshes': 0,
            'total_textures': 0,
            'total_materials': 0,
            'meshes_without_lods': 0,
            'high_poly_meshes': 0,
            'large_textures': 0,
            'complex_materials': 0
        }
    }

    for path in paths:
        unreal.log(f"Scanning path: {path}")

        # Get all assets in path
        assets = asset_registry.get_assets_by_path(path, recursive=True)

        for asset_data in assets:
            asset_class = str(asset_data.asset_class_path.asset_name)
            asset_path = str(asset_data.object_path)

            # Load asset
            asset = unreal.EditorAssetLibrary.load_asset(asset_path)
            if not asset:
                continue

            # Analyze based on type
            if asset_class == 'StaticMesh':
                analysis = analyze_static_mesh(asset)
                if analysis:
                    analysis['path'] = asset_path
                    analysis['name'] = str(asset_data.asset_name)
                    results['static_meshes'].append(analysis)
                    results['summary']['total_meshes'] += 1

                    if analysis['lod_count'] <= 1:
                        results['summary']['meshes_without_lods'] += 1
                    if analysis['triangles'] > 10000:
                        results['summary']['high_poly_meshes'] += 1

            elif asset_class == 'Texture2D':
                analysis = analyze_texture(asset)
                if analysis:
                    analysis['path'] = asset_path
                    analysis['name'] = str(asset_data.asset_name)
                    results['textures'].append(analysis)
                    results['summary']['total_textures'] += 1

                    if analysis['width'] > 2048 or analysis['height'] > 2048:
                        results['summary']['large_textures'] += 1

            elif 'Material' in asset_class:
                analysis = analyze_material(asset)
                if analysis:
                    analysis['path'] = asset_path
                    analysis['name'] = str(asset_data.asset_name)
                    analysis['type'] = asset_class
                    results['materials'].append(analysis)
                    results['summary']['total_materials'] += 1

                    if analysis['expression_count'] > 50:
                        results['summary']['complex_materials'] += 1

    return results

# Run analysis
unreal.log("Starting asset analysis...")
results = scan_assets()

# Save results
output_path = r'$OutputPath'
with open(output_path, 'w') as f:
    json.dump(results, f, indent=2)

unreal.log(f"Analysis complete. Results saved to: {output_path}")
unreal.log(f"Total meshes: {results['summary']['total_meshes']}")
unreal.log(f"Meshes without LODs: {results['summary']['meshes_without_lods']}")
unreal.log(f"High poly meshes (>10k tris): {results['summary']['high_poly_meshes']}")
unreal.log(f"Total textures: {results['summary']['total_textures']}")
unreal.log(f"Large textures (>2048): {results['summary']['large_textures']}")
unreal.log(f"Total materials: {results['summary']['total_materials']}")
unreal.log(f"Complex materials (>50 nodes): {results['summary']['complex_materials']}")
"@

# Save Python script
$TempPyScript = "$ProjectPath\Saved\analyze_assets_temp.py"
$PythonScript | Out-File -FilePath $TempPyScript -Encoding UTF8

Write-Host "`nRunning asset analysis..." -ForegroundColor Yellow
Write-Host "This may take several minutes for large asset packs..."

# Run UE Python script
& $UEEditorCmd $ProjectFile -ExecutePythonScript=$TempPyScript -stdout -unattended -nopause -nosplash -log

# Check results
if (Test-Path $OutputPath) {
    Write-Host "`n=== Analysis Complete ===" -ForegroundColor Green
    $report = Get-Content $OutputPath | ConvertFrom-Json

    Write-Host "`nSummary:" -ForegroundColor Cyan
    Write-Host "  Total Static Meshes: $($report.summary.total_meshes)"
    Write-Host "  Meshes without LODs: $($report.summary.meshes_without_lods)" -ForegroundColor $(if($report.summary.meshes_without_lods -gt 0){'Yellow'}else{'Green'})
    Write-Host "  High Poly Meshes (>10k tris): $($report.summary.high_poly_meshes)" -ForegroundColor $(if($report.summary.high_poly_meshes -gt 0){'Yellow'}else{'Green'})
    Write-Host "  Total Textures: $($report.summary.total_textures)"
    Write-Host "  Large Textures (>2048): $($report.summary.large_textures)" -ForegroundColor $(if($report.summary.large_textures -gt 0){'Yellow'}else{'Green'})
    Write-Host "  Total Materials: $($report.summary.total_materials)"
    Write-Host "  Complex Materials (>50 nodes): $($report.summary.complex_materials)" -ForegroundColor $(if($report.summary.complex_materials -gt 0){'Yellow'}else{'Green'})

    Write-Host "`nReport saved to: $OutputPath" -ForegroundColor Green
} else {
    Write-Host "`nError: Analysis failed. Check UE logs." -ForegroundColor Red
}

# Cleanup
Remove-Item $TempPyScript -ErrorAction SilentlyContinue
