# LOD Generation Tool for Horror Project
# Generates LOD chain for all static meshes

param(
    [string]$ProjectPath = "",
    [string]$AssetReport = "",
    [int]$MinTriangles = 100,
    [switch]$ForceRegenerate
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

Write-Host "=== LOD Generation Tool ===" -ForegroundColor Cyan

# Check if analysis report exists
if (-not (Test-Path $AssetReport)) {
    Write-Host "Asset report not found. Run AnalyzeAssets.ps1 first." -ForegroundColor Red
    exit 1
}

$report = Get-Content $AssetReport | ConvertFrom-Json
$meshesToProcess = $report.static_meshes | Where-Object { $_.lod_count -le 1 -and $_.triangles -ge $MinTriangles }

Write-Host "Found $($meshesToProcess.Count) meshes requiring LOD generation" -ForegroundColor Yellow

# Python script for LOD generation
$PythonScript = @"
import unreal
import json

def generate_lods_for_mesh(mesh_path, force=False):
    """Generate LOD chain for a static mesh"""
    try:
        # Load mesh
        mesh = unreal.EditorAssetLibrary.load_asset(mesh_path)
        if not mesh:
            unreal.log_warning(f"Failed to load mesh: {mesh_path}")
            return False

        # Check if already has LODs
        current_lods = mesh.get_num_lods()
        if current_lods > 1 and not force:
            unreal.log(f"Mesh already has {current_lods} LODs, skipping: {mesh_path}")
            return True

        # LOD generation settings
        lod_settings = []

        # LOD1: 75% reduction
        lod1 = unreal.EditorScriptingMeshReductionSettings()
        lod1.percent_triangles = 0.75
        lod1.percent_vertices = 0.75
        lod_settings.append(lod1)

        # LOD2: 50% reduction
        lod2 = unreal.EditorScriptingMeshReductionSettings()
        lod2.percent_triangles = 0.50
        lod2.percent_vertices = 0.50
        lod_settings.append(lod2)

        # LOD3: 25% reduction
        lod3 = unreal.EditorScriptingMeshReductionSettings()
        lod3.percent_triangles = 0.25
        lod3.percent_vertices = 0.25
        lod_settings.append(lod3)

        # LOD4: 10% reduction (impostor base)
        lod4 = unreal.EditorScriptingMeshReductionSettings()
        lod4.percent_triangles = 0.10
        lod4.percent_vertices = 0.10
        lod_settings.append(lod4)

        # Apply LOD settings
        options = unreal.EditorScriptingMeshReductionOptions()
        options.auto_compute_lod_screen_size = True

        # Set LOD settings on mesh
        for i, lod_setting in enumerate(lod_settings):
            lod_index = i + 1
            unreal.EditorStaticMeshLibrary.set_lod_reduction_settings(mesh, lod_index, lod_setting)

        # Regenerate LODs
        unreal.EditorStaticMeshLibrary.regenerate_lods(mesh)

        # Save asset
        unreal.EditorAssetLibrary.save_loaded_asset(mesh)

        unreal.log(f"Generated LODs for: {mesh_path}")
        return True

    except Exception as e:
        unreal.log_error(f"Error generating LODs for {mesh_path}: {str(e)}")
        return False

def batch_generate_lods(mesh_paths, force=False):
    """Generate LODs for multiple meshes"""
    results = {
        'success': [],
        'failed': [],
        'skipped': []
    }

    total = len(mesh_paths)
    for i, mesh_path in enumerate(mesh_paths):
        unreal.log(f"Processing {i+1}/{total}: {mesh_path}")

        if generate_lods_for_mesh(mesh_path, force):
            results['success'].append(mesh_path)
        else:
            results['failed'].append(mesh_path)

    return results

# Load mesh list from report
report_path = r'$AssetReport'
with open(report_path, 'r') as f:
    report = json.load(f)

# Filter meshes that need LODs
min_triangles = $MinTriangles
force = $($ForceRegenerate.IsPresent.ToString().ToLower())

meshes_to_process = [
    m['path'] for m in report['static_meshes']
    if (m['lod_count'] <= 1 or force) and m['triangles'] >= min_triangles
]

unreal.log(f"Processing {len(meshes_to_process)} meshes...")

# Generate LODs
results = batch_generate_lods(meshes_to_process, force)

# Save results
output_path = r'$ProjectPath\Scripts\Assets\LODGenerationReport.json'
with open(output_path, 'w') as f:
    json.dump(results, f, indent=2)

unreal.log(f"LOD generation complete!")
unreal.log(f"Success: {len(results['success'])}")
unreal.log(f"Failed: {len(results['failed'])}")
unreal.log(f"Skipped: {len(results['skipped'])}")
"@

# Save Python script
$TempPyScript = "$ProjectPath\Saved\generate_lods_temp.py"
$PythonScript | Out-File -FilePath $TempPyScript -Encoding UTF8

Write-Host "`nGenerating LODs..." -ForegroundColor Yellow
Write-Host "This will take a while for large meshes..."

# Run UE Python script
& $UEEditorCmd $ProjectFile -ExecutePythonScript=$TempPyScript -stdout -unattended -nopause -nosplash -log

# Check results
$ResultsPath = "$ProjectPath\Scripts\Assets\LODGenerationReport.json"
if (Test-Path $ResultsPath) {
    Write-Host "`n=== LOD Generation Complete ===" -ForegroundColor Green
    $results = Get-Content $ResultsPath | ConvertFrom-Json

    Write-Host "`nResults:" -ForegroundColor Cyan
    Write-Host "  Success: $($results.success.Count)" -ForegroundColor Green
    Write-Host "  Failed: $($results.failed.Count)" -ForegroundColor $(if($results.failed.Count -gt 0){'Red'}else{'Green'})

    if ($results.failed.Count -gt 0) {
        Write-Host "`nFailed meshes:" -ForegroundColor Red
        $results.failed | ForEach-Object { Write-Host "  - $_" }
    }
} else {
    Write-Host "`nError: LOD generation failed. Check UE logs." -ForegroundColor Red
}

# Cleanup
Remove-Item $TempPyScript -ErrorAction SilentlyContinue
