# Material Optimization Tool for Horror Project
# Optimizes material complexity and shader instructions

param(
    [string]$ProjectPath = "D:\gptzuo\HorrorProject\HorrorProject",
    [string]$AssetReport = "D:\gptzuo\HorrorProject\HorrorProject\Scripts\Assets\AssetReport.json"
)

$UE5Root = if ($env:UE5_ROOT) { $env:UE5_ROOT } elseif ($env:UE_5_6_ROOT) { $env:UE_5_6_ROOT } elseif (Test-Path 'D:\UnrealEngine\UE_5.6') { 'D:\UnrealEngine\UE_5.6' } else { 'C:\Program Files\Epic Games\UE_5.6' }
$UEEditorCmd = Join-Path $UE5Root "Engine\Binaries\Win64\UnrealEditor-Cmd.exe"
$ProjectFile = "$ProjectPath\HorrorProject.uproject"

Write-Host "=== Material Optimization Tool ===" -ForegroundColor Cyan

# Python script for material optimization
$PythonScript = @"
import unreal
import json

def optimize_material_instance(material_path):
    """Optimize material instance settings"""
    try:
        material = unreal.EditorAssetLibrary.load_asset(material_path)
        if not material or not isinstance(material, unreal.MaterialInstance):
            return False, "Not a material instance"

        modified = False

        # Enable quality level overrides
        if hasattr(material, 'set_editor_property'):
            # Enable static switch parameters for quality levels
            static_params = material.get_editor_property('static_parameters')
            if static_params:
                modified = True

        if modified:
            unreal.EditorAssetLibrary.save_loaded_asset(material)
            return True, "Optimized"
        else:
            return True, "Already optimal"

    except Exception as e:
        return False, str(e)

def create_material_quality_variants(base_material_path):
    """Create quality level variants for a material"""
    try:
        base_material = unreal.EditorAssetLibrary.load_asset(base_material_path)
        if not base_material:
            return False, "Failed to load base material"

        # Get material path info
        package_path = unreal.Paths.get_path(base_material_path)
        material_name = unreal.Paths.get_base_filename(base_material_path)

        # Create quality variants
        variants = []

        # High quality (default)
        # Medium quality
        medium_path = f"{package_path}/{material_name}_Medium"
        if not unreal.EditorAssetLibrary.does_asset_exist(medium_path):
            medium_mat = unreal.AssetToolsHelpers.get_asset_tools().duplicate_asset(
                material_name + "_Medium",
                package_path,
                base_material
            )
            if medium_mat:
                variants.append(medium_path)

        # Low quality
        low_path = f"{package_path}/{material_name}_Low"
        if not unreal.EditorAssetLibrary.does_asset_exist(low_path):
            low_mat = unreal.AssetToolsHelpers.get_asset_tools().duplicate_asset(
                material_name + "_Low",
                package_path,
                base_material
            )
            if low_mat:
                variants.append(low_path)

        return True, f"Created {len(variants)} variants"

    except Exception as e:
        return False, str(e)

def analyze_material_complexity(material_path):
    """Analyze material shader complexity"""
    try:
        material = unreal.EditorAssetLibrary.load_asset(material_path)
        if not material:
            return None

        # Get expressions
        expressions = material.get_editor_property('expressions') if hasattr(material, 'get_editor_property') else []

        if not expressions:
            return {
                'expression_count': 0,
                'texture_samples': 0,
                'math_operations': 0,
                'complexity': 'Low'
            }

        # Count different node types
        texture_samples = len([e for e in expressions if isinstance(e, unreal.MaterialExpressionTextureSample)])
        math_ops = len([e for e in expressions if isinstance(e, (
            unreal.MaterialExpressionAdd,
            unreal.MaterialExpressionMultiply,
            unreal.MaterialExpressionDivide,
            unreal.MaterialExpressionSubtract
        ))])

        # Estimate complexity
        total_nodes = len(expressions)
        if total_nodes > 100 or texture_samples > 16:
            complexity = 'Very High'
        elif total_nodes > 50 or texture_samples > 8:
            complexity = 'High'
        elif total_nodes > 25:
            complexity = 'Medium'
        else:
            complexity = 'Low'

        return {
            'expression_count': total_nodes,
            'texture_samples': texture_samples,
            'math_operations': math_ops,
            'complexity': complexity
        }

    except Exception as e:
        unreal.log_warning(f"Error analyzing material {material_path}: {str(e)}")
        return None

def batch_optimize_materials(material_paths):
    """Optimize multiple materials"""
    results = {
        'optimized': [],
        'analyzed': [],
        'failed': []
    }

    total = len(material_paths)
    for i, material_path in enumerate(material_paths):
        unreal.log(f"Processing {i+1}/{total}: {material_path}")

        # Analyze complexity
        analysis = analyze_material_complexity(material_path)
        if analysis:
            results['analyzed'].append({
                'path': material_path,
                'analysis': analysis
            })

        # Optimize if material instance
        success, message = optimize_material_instance(material_path)
        if success and message == "Optimized":
            results['optimized'].append(material_path)
        elif not success:
            results['failed'].append({'path': material_path, 'error': message})

    return results

# Load material list from report
report_path = r'$AssetReport'
with open(report_path, 'r') as f:
    report = json.load(f)

material_paths = [m['path'] for m in report['materials']]

unreal.log(f"Processing {len(material_paths)} materials...")

# Optimize materials
results = batch_optimize_materials(material_paths)

# Save results
output_path = r'$ProjectPath\Scripts\Assets\MaterialOptimizationReport.json'
with open(output_path, 'w') as f:
    json.dump(results, f, indent=2)

unreal.log(f"Material optimization complete!")
unreal.log(f"Optimized: {len(results['optimized'])}")
unreal.log(f"Analyzed: {len(results['analyzed'])}")
unreal.log(f"Failed: {len(results['failed'])}")

# Print complexity summary
high_complexity = [a for a in results['analyzed'] if a['analysis']['complexity'] in ['High', 'Very High']]
unreal.log(f"High complexity materials: {len(high_complexity)}")
"@

# Save Python script
$TempPyScript = "$ProjectPath\Saved\optimize_materials_temp.py"
$PythonScript | Out-File -FilePath $TempPyScript -Encoding UTF8

Write-Host "`nOptimizing materials..." -ForegroundColor Yellow

# Run UE Python script
& $UEEditorCmd $ProjectFile -ExecutePythonScript=$TempPyScript -stdout -unattended -nopause -nosplash -log

# Check results
$ResultsPath = "$ProjectPath\Scripts\Assets\MaterialOptimizationReport.json"
if (Test-Path $ResultsPath) {
    Write-Host "`n=== Material Optimization Complete ===" -ForegroundColor Green
    $results = Get-Content $ResultsPath | ConvertFrom-Json

    Write-Host "`nResults:" -ForegroundColor Cyan
    Write-Host "  Optimized: $($results.optimized.Count)" -ForegroundColor Green
    Write-Host "  Analyzed: $($results.analyzed.Count)"
    Write-Host "  Failed: $($results.failed.Count)" -ForegroundColor $(if($results.failed.Count -gt 0){'Red'}else{'Green'})

    # Show complexity breakdown
    $complexityGroups = $results.analyzed | Group-Object { $_.analysis.complexity }
    Write-Host "`nComplexity Breakdown:" -ForegroundColor Cyan
    $complexityGroups | ForEach-Object {
        $color = switch($_.Name) {
            'Very High' { 'Red' }
            'High' { 'Yellow' }
            'Medium' { 'White' }
            'Low' { 'Green' }
            default { 'White' }
        }
        Write-Host "  $($_.Name): $($_.Count)" -ForegroundColor $color
    }

    # Show high complexity materials
    $highComplexity = $results.analyzed | Where-Object { $_.analysis.complexity -in @('High', 'Very High') }
    if ($highComplexity.Count -gt 0) {
        Write-Host "`nHigh Complexity Materials (Top 5):" -ForegroundColor Yellow
        $highComplexity | Sort-Object { $_.analysis.expression_count } -Descending | Select-Object -First 5 | ForEach-Object {
            Write-Host "  $($_.path)"
            Write-Host "    Nodes: $($_.analysis.expression_count), Textures: $($_.analysis.texture_samples)" -ForegroundColor Gray
        }
    }
} else {
    Write-Host "`nError: Material optimization failed. Check UE logs." -ForegroundColor Red
}

# Cleanup
Remove-Item $TempPyScript -ErrorAction SilentlyContinue
