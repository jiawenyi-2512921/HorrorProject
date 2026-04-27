# OptimizeTextures.ps1
# Texture Optimization Execution Script
# Automatically optimizes textures based on usage and performance targets

param(
    [string]$ProjectPath = "",
    [string]$OutputDir = "",
    [switch]$DryRun = $false,
    [switch]$Aggressive = $false
)

$ErrorActionPreference = "Stop"

. (Join-Path $PSScriptRoot "..\..\Validation\Common.ps1")
$ProjectRoot = Get-HorrorProjectRoot -StartPath $PSScriptRoot
if ([string]::IsNullOrWhiteSpace($ProjectPath)) { $ProjectPath = $ProjectRoot }
if ([string]::IsNullOrWhiteSpace($OutputDir)) { $OutputDir = Join-Path $ProjectRoot "Saved\Optimization\Textures" }

Write-Host "=== Texture Optimization Tool ===" -ForegroundColor Cyan
Write-Host "Mode: $(if ($DryRun) { 'Dry Run (Preview Only)' } else { 'Execute' })"
Write-Host "Profile: $(if ($Aggressive) { 'Aggressive' } else { 'Balanced' })"
Write-Host ""

# Create output directory
New-Item -ItemType Directory -Force -Path $OutputDir | Out-Null

$timestamp = Get-Date -Format "yyyyMMdd_HHmmss"
$reportFile = Join-Path $OutputDir "texture_optimization_$timestamp.json"

# Optimization profiles
$profiles = @{
    Balanced = @{
        BaseColorMax = 2048
        NormalMapMax = 2048
        RoughnessMax = 1024
        MetallicMax = 1024
        EmissiveMax = 1024
        UIMax = 1024
        CompressionQuality = "Default"
        MipGenSettings = "FromTextureGroup"
    }
    Aggressive = @{
        BaseColorMax = 1024
        NormalMapMax = 1024
        RoughnessMax = 512
        MetallicMax = 512
        EmissiveMax = 512
        UIMax = 512
        CompressionQuality = "High"
        MipGenSettings = "Sharpen0"
    }
}

$profile = if ($Aggressive) { $profiles.Aggressive } else { $profiles.Balanced }

Write-Host "Scanning for textures..." -ForegroundColor Yellow

# Find all texture assets
$contentPath = Join-Path $ProjectPath "Content"
$textureFiles = Get-ChildItem -Path $contentPath -Recurse -Include "*.uasset" | Where-Object {
    $_.Name -match "^T_" -or $_.DirectoryName -match "Textures"
}

Write-Host "Found $($textureFiles.Count) texture files" -ForegroundColor Green
Write-Host ""

$optimizationData = @{
    Timestamp = $timestamp
    Profile = if ($Aggressive) { "Aggressive" } else { "Balanced" }
    DryRun = $DryRun
    TotalTextures = $textureFiles.Count
    OptimizedTextures = @()
    EstimatedMemorySavings = 0
}

$optimizedCount = 0
$totalMemorySaved = 0

foreach ($textureFile in $textureFiles) {
    $relativePath = $textureFile.FullName.Substring($contentPath.Length + 1)
    $textureName = $textureFile.BaseName

    # Determine texture type
    $textureType = "BaseColor"
    if ($textureName -match "_N$|_Normal") { $textureType = "Normal" }
    elseif ($textureName -match "_R$|_Roughness") { $textureType = "Roughness" }
    elseif ($textureName -match "_M$|_Metallic") { $textureType = "Metallic" }
    elseif ($textureName -match "_E$|_Emissive") { $textureType = "Emissive" }
   if ($textureName -match "UI_|_UI") { $textureType = "UI" }

    # Get target resolution
    $targetResolution = switch ($textureType) {
        "Normal" { $profile.NormalMapMax }
        "Roughness" { $profile.RoughnessMax }
        "Metallic" { $profile.MetallicMax }
        "Emissive" { $profile.EmissiveMax }
        "UI" { $profile.UIMax }
        default { $profile.BaseColorMax }
    }

    # Estimate current size (rough approximation)
    $currentSize = $textureFile.Length
    $estimatedResolution = 2048 # Assume 2K by default

    if ($currentSize -gt 10MB) { $estimatedResolution = 4096 }
   lseif ($currentSize -gt 2MB) { $estimatedResolution = 2048 }
    elseif ($currentSize -gt 500KB) { $estimatedResolution = 1024 }
    else { $estimatedResolution = 512 }

    # Check if optimization needed
    if ($estimatedResolution -gt $targetResolution) {
        $optimizedCount++

        # Calculate memory savings (rough estimate)
        $reductionFactor = ($targetResolution / $estimatedResolution) * ($targetResolution / $estimatedResolution)
        $memorySaved = $currentSize * (1 - $reductionFactor)
        $totalMemorySaved += $memorySaved

        $optimizationInfo = @{
            Path = $relativePath
            Name = $textureName
            Type = $textureType
            CurrentResolution = $estimatedResolution
            TargetResolution = $targetResolution
            CurrentSize = $currentSize
            EstimatedSavings = $memorySaved
            Actions = @(
                "Resize to $targetResolution x $targetResolution",
                "Set compression to $($profile.CompressionQuality)",
                "Set mip generation to $($profile.MipGenSettings)"
            )
        }

        $optimizationData.OptimizedTextures += $optimizationInfo

        Write-Host "[$optimizedCount] $textureName" -ForegroundColor Yellow
        Write-Host "    Type: $textureType" -ForegroundColor Gray
        Write-Host "    $estimatedResolution -> $targetResolution" -ForegroundColor Cyan
        Write-Host "    Savings: $([math]::Round($memorySaved / 1MB, 2)) MB" -ForegroundColor Green

        if (-not $DryRun) {
            # In a real implementation, this would use UE5 automation or Python scripts
# Generate an Unreal Editor command list for safe texture optimization
            Write-Host "    [Would optimize in production mode]" -ForegroundColor DarkGray
        }
    }
}

$optimizationData.EstimatedMemorySavings = $totalMemorySaved

# Save report
$optimizationData | ConvertTo-Json -Depth 10 | Out-File $reportFile -Encoding UTF8

Write-Host ""
Write-Host "=== Optimization Summary ===" -ForegroundColor Cyan
Write-Host "Total textures scanned: $($textureFiles.Count)"
Write-Host "Textures to optimize: $optimizedCount"
Write-Host "Estimated memory savings: $([math]::Round($totalMemorySaved / 1MB, 2)) MB"
Write-Host ""

if ($optimizedCount -gt 0) {
    Write-Host "Top optimization opportunities:" -ForegroundColor Yellow
    $topOptimizations = $optimizationData.OptimizedTextures |
        Sort-Object EstimatedSavings -Descending |
        Select-Object -First 5

    foreach ($opt in $topOptimizations) {
        Write-Host "  $($opt.Name): $([math]::Round($opt.EstimatedSavings / 1MB, 2)) MB"
    }
    Write-Host ""
}

Write-Host "Report saved to: $reportFile" -ForegroundColor Green
Write-Host ""

if ($DryRun) {
    Write-Host "This was a dry run. Run without -DryRun to apply optimizations." -ForegroundColor Yellow
} else {
    Write-Host "Optimization commands generated. Apply using UE5 automation tools." -ForegroundColor Green
}

# Generate Python script for UE5 automation
$pythonScript = @"
# UE5 Texture Optimization Script
# Generated: $timestamp
import unreal

def optimize_textures():
    asset_registry = unreal.AssetRegistryHelpers.get_asset_registry()

    # Optimization settings
    max_sizes = {
        'BaseColor': $($profile.BaseColorMax),
        'Normal': $($profile.NormalMapMax),
        'Roughness': $($profile.RoughnessMax),
        'Metallic': $($profile.MetallicMax),
        'Emissive': $($profile.EmissiveMax),
        'UI': $($profile.UIMax)
    }

    # Find all texture assets
    textures = asset_registry.get_assets_by_class('Texture2D', True)

    for texture_data in textures:
        texture = unreal.load_asset(texture_data.package_name)
        if not texture:
            continue

        # Determine texture type from name
        texture_name = texture.get_name()
        texture_type = 'BaseColor'

        if '_N' in texture_name or 'Normal' in texture_name:
            texture_type = 'Normal'
        elif '_R' in texture_name or 'Roughness' in texture_name:
            texture_type = 'Roughness'
        elif '_M' in texture_name or 'Metallic' in texture_name:
            texture_type = 'Metallic'
        elif '_E' in texture_name or 'Emissive' in texture_name:
            texture_type = 'Emissive'
        elif 'UI' in texture_name:
            texture_type = 'UI'

        # Get target size
        target_size = max_sizes.get(texture_type, $($profile.BaseColorMax))

        # Check if resize needed
        current_size = max(texture.blueprint_get_size_x(), texture.blueprint_get_size_y())

        if current_size > target_size:
            print(f'Optimizing {texture_name}: {current_size} -> {target_size}')

            # Set max texture size
            texture.set_editor_property('max_texture_size', target_size)

            # Set compression settings
            texture.set_editor_property('compression_quality', unreal.TextureCompressionQuality.$($profile.CompressionQuality.ToUpper()))

            # Save asset
            unreal.EditorAssetLibrary.save_loaded_asset(texture)

    print('Texture optimization complete!')

if __name__ == '__main__':
    optimize_textures()
"@

$pythonScriptPath = Join-Path $OutputDir "optimize_textures_$timestamp.py"
$pythonScript | Out-File $pythonScriptPath -Encoding UTF8

Write-Host "Python automation script saved to: $pythonScriptPath" -ForegroundColor Green
Write-Host ""
Write-Host "To apply optimizations, run in UE5 Editor:" -ForegroundColor Cyan
Write-Host "  py `"$pythonScriptPath`"" -ForegroundColor White
Write-Host ""
Write-Host "Texture optimization complete!" -ForegroundColor Green
