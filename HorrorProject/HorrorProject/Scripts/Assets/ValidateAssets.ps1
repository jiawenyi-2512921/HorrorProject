# Asset Validation Tool for Horror Project
# Validates optimizations and checks performance budgets

param(
    [string]$ProjectPath = "",
    [string]$OutputPath = ""
)

. (Join-Path (Split-Path -Parent $PSScriptRoot) "Validation\Common.ps1")

if ([string]::IsNullOrWhiteSpace($ProjectPath)) {
    $ProjectPath = Get-HorrorProjectRoot -StartPath $PSScriptRoot
} else {
    $ProjectPath = (Resolve-Path -LiteralPath $ProjectPath).Path
}
if ([string]::IsNullOrWhiteSpace($OutputPath)) {
    $OutputPath = Join-Path $PSScriptRoot "ValidationReport.json"
}

$UE5Root = Get-HorrorUERoot
$UEEditorCmd = Get-HorrorEditorCmd -UERoot $UE5Root
$ProjectFile = Get-HorrorProjectFile -ProjectRoot $ProjectPath

Write-Host "=== Asset Validation Tool ===" -ForegroundColor Cyan

# Performance budgets
$Budgets = @{
    MaxTrianglesPerMesh = 50000
    MaxTextureSize = 4096
    MaxLightmapResolution = 512
    MinLODCount = 3
    MaxMaterialComplexity = 100
}

Write-Host "`nPerformance Budgets:" -ForegroundColor Yellow
$Budgets.GetEnumerator() | ForEach-Object { Write-Host "  $($_.Key): $($_.Value)" }

# Python script for validation
$PythonScript = @"
import unreal
import json

# Performance budgets
BUDGETS = {
    'max_triangles': $($Budgets.MaxTrianglesPerMesh),
    'max_texture_size': $($Budgets.MaxTextureSize),
    'max_lightmap_res': $($Budgets.MaxLightmapResolution),
    'min_lod_count': $($Budgets.MinLODCount),
    'max_material_complexity': $($Budgets.MaxMaterialComplexity)
}

def validate_static_mesh(mesh_path):
    """Validate static mesh against budgets"""
    issues = []
    warnings = []

    try:
        mesh = unreal.EditorAssetLibrary.load_asset(mesh_path)
        if not mesh:
            return {'path': mesh_path, 'issues': ['Failed to load'], 'warnings': []}

        # Check triangle count
        render_data = mesh.get_editor_property('render_data')
        if render_data and len(render_data.lod_resources) > 0:
            triangles = render_data.lod_resources[0].get_num_triangles()
            if triangles > BUDGETS['max_triangles']:
                issues.append(f"Triangle count {triangles} exceeds budget {BUDGETS['max_triangles']}")

        # Check LOD count
        lod_count = mesh.get_num_lods()
        if lod_count < BUDGETS['min_lod_count']:
            warnings.append(f"LOD count {lod_count} below recommended {BUDGETS['min_lod_count']}")

        # Check lightmap resolution
        lightmap_res = mesh.get_editor_property('light_map_resolution')
        if lightmap_res > BUDGETS['max_lightmap_res']:
            warnings.append(f"Lightmap resolution {lightmap_res} exceeds budget {BUDGETS['max_lightmap_res']}")

        # Check collision
        has_collision = mesh.get_editor_property('body_setup') is not None
        if not has_collision:
            warnings.append("No collision geometry")

        return {
            'path': mesh_path,
            'issues': issues,
            'warnings': warnings,
            'stats': {
                'triangles': triangles if 'triangles' in locals() else 0,
                'lod_count': lod_count,
                'lightmap_res': lightmap_res
            }
        }

    except Exception as e:
        return {'path': mesh_path, 'issues': [f"Validation error: {str(e)}"], 'warnings': []}

def validate_texture(texture_path):
    """Validate texture against budgets"""
    issues = []
    warnings = []

    try:
        texture = unreal.EditorAssetLibrary.load_asset(texture_path)
        if not texture:
            return {'path': texture_path, 'issues': ['Failed to load'], 'warnings': []}

        width = texture.blueprint_get_size_x()
        height = texture.blueprint_get_size_y()

        # Check size
        if width > BUDGETS['max_texture_size'] or height > BUDGETS['max_texture_size']:
            issues.append(f"Texture size {width}x{height} exceeds budget {BUDGETS['max_texture_size']}")

        # Check power of two
        is_pot_width = (width & (width - 1)) == 0
        is_pot_height = (height & (height - 1)) == 0
        if not (is_pot_width and is_pot_height):
            warnings.append(f"Non-power-of-two texture: {width}x{height}")

        # Check compression
        compression = texture.get_editor_property('compression_settings')
        if compression == unreal.TextureCompressionSettings.TC_DEFAULT:
            warnings.append("Using default compression, may not be optimal")

        # Check mipmaps
        mip_gen = texture.get_editor_property('mip_gen_settings')
        if mip_gen == unreal.TextureMipGenSettings.TMGS_NO_MIPMAPS:
            warnings.append("Mipmaps disabled")

        return {
            'path': texture_path,
            'issues': issues,
            'warnings': warnings,
            'stats': {
                'width': width,
                'height': height,
                'compression': str(compression)
            }
        }

    except Exception as e:
        return {'path': texture_path, 'issues': [f"Validation error: {str(e)}"], 'warnings': []}

def validate_material(material_path):
    """Validate material complexity"""
    issues = []
    warnings = []

    try:
        material = unreal.EditorAssetLibrary.load_asset(material_path)
        if not material:
            return {'path': material_path, 'issues': ['Failed to load'], 'warnings': []}

        # Get expression count
        expressions = material.get_editor_property('expressions')
        expression_count = len(expressions) if expressions else 0

        if expression_count > BUDGETS['max_material_complexity']:
            warnings.append(f"Material complexity {expression_count} exceeds budget {BUDGETS['max_material_complexity']}")

        # Check texture samples
        texture_samples = [e for e in expressions if isinstance(e, unreal.MaterialExpressionTextureSample)] if expressions else []
        if len(texture_samples) > 16:
            warnings.append(f"High texture sample count: {len(texture_samples)}")

        return {
            'path': material_path,
            'issues': issues,
            'warnings': warnings,
            'stats': {
                'expression_count': expression_count,
                'texture_samples': len(texture_samples)
            }
        }

    except Exception as e:
        return {'path': material_path, 'issues': [f"Validation error: {str(e)}"], 'warnings': []}

def validate_all_assets():
    """Validate all environment assets"""
    asset_registry = unreal.AssetRegistryHelpers.get_asset_registry()

    paths = [
        '/Game/DeepWaterStation',
        '/Game/SD_Art',
        '/Game/Fab/Modular_Fence_Industrial_Prop'
    ]

    results = {
        'meshes': [],
        'textures': [],
        'materials': [],
        'summary': {
            'total_issues': 0,
            'total_warnings': 0,
            'meshes_with_issues': 0,
            'textures_with_issues': 0,
            'materials_with_issues': 0
        }
    }

    for path in paths:
        unreal.log(f"Validating path: {path}")
        assets = asset_registry.get_assets_by_path(path, recursive=True)

        for asset_data in assets:
            asset_class = str(asset_data.asset_class_path.asset_name)
            asset_path = str(asset_data.object_path)

            if asset_class == 'StaticMesh':
                validation = validate_static_mesh(asset_path)
                results['meshes'].append(validation)
                if validation['issues']:
                    results['summary']['meshes_with_issues'] += 1
                    results['summary']['total_issues'] += len(validation['issues'])
                results['summary']['total_warnings'] += len(validation['warnings'])

            elif asset_class == 'Texture2D':
                validation = validate_texture(asset_path)
                results['textures'].append(validation)
                if validation['issues']:
                    results['summary']['textures_with_issues'] += 1
                    results['summary']['total_issues'] += len(validation['issues'])
                results['summary']['total_warnings'] += len(validation['warnings'])

            elif 'Material' in asset_class:
                validation = validate_material(asset_path)
                results['materials'].append(validation)
                if validation['issues']:
                    results['summary']['materials_with_issues'] += 1
                    results['summary']['total_issues'] += len(validation['issues'])
                results['summary']['total_warnings'] += len(validation['warnings'])

    return results

# Run validation
unreal.log("Starting asset validation...")
results = validate_all_assets()

# Save results
output_path = r'$OutputPath'
with open(output_path, 'w') as f:
    json.dump(results, f, indent=2)

unreal.log(f"Validation complete!")
unreal.log(f"Total issues: {results['summary']['total_issues']}")
unreal.log(f"Total warnings: {results['summary']['total_warnings']}")
unreal.log(f"Meshes with issues: {results['summary']['meshes_with_issues']}")
unreal.log(f"Textures with issues: {results['summary']['textures_with_issues']}")
unreal.log(f"Materials with issues: {results['summary']['materials_with_issues']}")
"@

# Save Python script
$TempPyScript = "$ProjectPath\Saved\validate_assets_temp.py"
$PythonScript | Out-File -FilePath $TempPyScript -Encoding UTF8

Write-Host "`nValidating assets..." -ForegroundColor Yellow

# Run UE Python script
& $UEEditorCmd $ProjectFile -ExecutePythonScript=$TempPyScript -stdout -unattended -nopause -nosplash -log

# Check results
if (Test-Path $OutputPath) {
    Write-Host "`n=== Validation Complete ===" -ForegroundColor Green
    $results = Get-Content $OutputPath | ConvertFrom-Json

    Write-Host "`nSummary:" -ForegroundColor Cyan
    Write-Host "  Total Issues: $($results.summary.total_issues)" -ForegroundColor $(if($results.summary.total_issues -gt 0){'Red'}else{'Green'})
    Write-Host "  Total Warnings: $($results.summary.total_warnings)" -ForegroundColor $(if($results.summary.total_warnings -gt 0){'Yellow'}else{'Green'})
    Write-Host "  Meshes with issues: $($results.summary.meshes_with_issues)"
    Write-Host "  Textures with issues: $($results.summary.textures_with_issues)"
    Write-Host "  Materials with issues: $($results.summary.materials_with_issues)"

    # Show critical issues
    $criticalMeshes = $results.meshes | Where-Object { $_.issues.Count -gt 0 }
    if ($criticalMeshes.Count -gt 0) {
        Write-Host "`nCritical Mesh Issues:" -ForegroundColor Red
        $criticalMeshes | Select-Object -First 5 | ForEach-Object {
            Write-Host "  $($_.path)"
            $_.issues | ForEach-Object { Write-Host "    - $_" -ForegroundColor Red }
        }
        if ($criticalMeshes.Count -gt 5) {
            Write-Host "  ... and $($criticalMeshes.Count - 5) more"
        }
    }

    Write-Host "`nReport saved to: $OutputPath" -ForegroundColor Green
} else {
    Write-Host "`nError: Validation failed. Check UE logs." -ForegroundColor Red
}

# Cleanup
Remove-Item $TempPyScript -ErrorAction SilentlyContinue
