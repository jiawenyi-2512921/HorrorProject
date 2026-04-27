# Texture Optimization Tool for Horror Project
# Optimizes texture compression, mipmaps, and resolution

param(
    [string]$ProjectPath = "",
    [string]$AssetReport = "",
    [switch]$EnableVirtualTextures
)

. (Join-Path (Split-Path -Parent $PSScriptRoot) "Validation\Common.ps1")

if ([string]::IsNullOrWhiteSpace($ProjectPath)) {
    $ProjectPath = Get-HorrorProjectRoot -StartPath $PSScriptRoot
} else {
    $ProjectPath = (Resolve-Path -LiteralPath $ProjectPath).Path
}
if ([string]::IsNullOrWhiteSpace($AssetReport)) {
    $AssetReport = Join-Path $PSScriptRoot "AssetReport.json"
}

$UE5Root = Get-HorrorUERoot
$UEEditorCmd = Get-HorrorEditorCmd -UERoot $UE5Root
$ProjectFile = Get-HorrorProjectFile -ProjectRoot $ProjectPath

Write-Host "=== Texture Optimization Tool ===" -ForegroundColor Cyan

# Python script for texture optimization
$PythonScript = @"
import unreal
import json

def optimize_texture(texture_path, enable_virtual=False):
    """Optimize a single texture"""
    try:
        texture = unreal.EditorAssetLibrary.load_asset(texture_path)
        if not texture:
            return False, "Failed to load"

        modified = False

        # Get current settings
        width = texture.blueprint_get_size_x()
        height = texture.blueprint_get_size_y()
        current_compression = texture.get_editor_property('compression_settings')

        # Determine optimal compression based on texture type
        texture_name = texture.get_name().lower()

        if 'normal' in texture_name or '_n' in texture_name:
            # Normal maps: BC5
            optimal_compression = unreal.TextureCompressionSettings.TC_NORMALMAP
        elif 'mask' in texture_name or 'rough' in texture_name or 'metal' in texture_name or 'ao' in texture_name:
            # Masks and single channel: BC4
            optimal_compression = unreal.TextureCompressionSettings.TC_MASKS
        elif 'alpha' in texture_name or texture.has_alpha_channel():
            # Textures with alpha: BC3
            optimal_compression = unreal.TextureCompressionSettings.TC_BC7
        else:
            # Base color and other: BC1 or BC7
            optimal_compression = unreal.TextureCompressionSettings.TC_DEFAULT

        if current_compression != optimal_compression:
            texture.set_editor_property('compression_settings', optimal_compression)
            modified = True

        # Enable mipmaps if not already
        mip_gen = texture.get_editor_property('mip_gen_settings')
        if mip_gen != unreal.TextureMipGenSettings.TMGS_FROM_TEXTURE_GROUP:
            texture.set_editor_property('mip_gen_settings', unreal.TextureMipGenSettings.TMGS_FROM_TEXTURE_GROUP)
            modified = True

        # Set LOD group
        lod_group = texture.get_editor_property('lod_group')
        if 'ui' in texture_name or 'hud' in texture_name:
            target_group = unreal.TextureGroup.TEXTUREGROUP_UI
        elif 'character' in texture_name:
            target_group = unreal.TextureGroup.TEXTUREGROUP_CHARACTER
        elif 'world' in texture_name or 'environment' in texture_name:
            target_group = unreal.TextureGroup.TEXTUREGROUP_WORLD
        else:
            target_group = unreal.TextureGroup.TEXTUREGROUP_WORLD

        if lod_group != target_group:
            texture.set_editor_property('lod_group', target_group)
            modified = True

        # Virtual texture streaming (optional)
        if enable_virtual and (width >= 2048 or height >= 2048):
            is_virtual = texture.get_editor_property('virtual_texture_streaming')
            if not is_virtual:
                texture.set_editor_property('virtual_texture_streaming', True)
                modified = True

        # Power of two check
        is_power_of_two = (width & (width - 1)) == 0 and (height & (height - 1)) == 0
        if not is_power_of_two:
            unreal.log_warning(f"Non-power-of-two texture: {texture_path} ({width}x{height})")

        if modified:
            unreal.EditorAssetLibrary.save_loaded_asset(texture)
            return True, "Optimized"
        else:
            return True, "Already optimal"

    except Exception as e:
        return False, str(e)

def batch_optimize_textures(texture_paths, enable_virtual=False):
    """Optimize multiple textures"""
    results = {
        'optimized': [],
        'already_optimal': [],
        'failed': []
    }

    total = len(texture_paths)
    for i, texture_path in enumerate(texture_paths):
        unreal.log(f"Processing {i+1}/{total}: {texture_path}")

        success, message = optimize_texture(texture_path, enable_virtual)

        if success:
            if message == "Optimized":
                results['optimized'].append(texture_path)
            else:
                results['already_optimal'].append(texture_path)
        else:
            results['failed'].append({'path': texture_path, 'error': message})

    return results

# Load texture list from report
report_path = r'$AssetReport'
with open(report_path, 'r') as f:
    report = json.load(f)

texture_paths = [t['path'] for t in report['textures']]
enable_virtual = $($EnableVirtualTextures.IsPresent.ToString().ToLower())

unreal.log(f"Processing {len(texture_paths)} textures...")

# Optimize textures
results = batch_optimize_textures(texture_paths, enable_virtual)

# Save results
output_path = r'$ProjectPath\Scripts\Assets\TextureOptimizationReport.json'
with open(output_path, 'w') as f:
    json.dump(results, f, indent=2)

unreal.log(f"Texture optimization complete!")
unreal.log(f"Optimized: {len(results['optimized'])}")
unreal.log(f"Already optimal: {len(results['already_optimal'])}")
unreal.log(f"Failed: {len(results['failed'])}")
"@

# Save Python script
$TempPyScript = "$ProjectPath\Saved\optimize_textures_temp.py"
$PythonScript | Out-File -FilePath $TempPyScript -Encoding UTF8

Write-Host "`nOptimizing textures..." -ForegroundColor Yellow

# Run UE Python script
& $UEEditorCmd $ProjectFile -ExecutePythonScript=$TempPyScript -stdout -unattended -nopause -nosplash -log

# Check results
$ResultsPath = "$ProjectPath\Scripts\Assets\TextureOptimizationReport.json"
if (Test-Path $ResultsPath) {
    Write-Host "`n=== Texture Optimization Complete ===" -ForegroundColor Green
    $results = Get-Content $ResultsPath | ConvertFrom-Json

    Write-Host "`nResults:" -ForegroundColor Cyan
    Write-Host "  Optimized: $($results.optimized.Count)" -ForegroundColor Green
    Write-Host "  Already optimal: $($results.already_optimal.Count)"
    Write-Host "  Failed: $($results.failed.Count)" -ForegroundColor $(if($results.failed.Count -gt 0){'Red'}else{'Green'})

    if ($results.failed.Count -gt 0) {
        Write-Host "`nFailed textures:" -ForegroundColor Red
        $results.failed | ForEach-Object { Write-Host "  - $($_.path): $($_.error)" }
    }
} else {
    Write-Host "`nError: Texture optimization failed. Check UE logs." -ForegroundColor Red
}

# Cleanup
Remove-Item $TempPyScript -ErrorAction SilentlyContinue
