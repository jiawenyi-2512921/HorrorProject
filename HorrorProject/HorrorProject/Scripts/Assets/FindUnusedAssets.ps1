# Find Unused Assets Script
# Identifies assets that are not referenced by any other assets

param(
    [string]$ContentPath = "",
    [string]$ProjectPath = "",
    [switch]$ExcludeMaps = $false,
    [switch]$Verbose = $false
)

. (Join-Path (Split-Path -Parent $PSScriptRoot) "Validation\Common.ps1")

$ProjectRoot = Get-HorrorProjectRoot -StartPath $PSScriptRoot
if ([string]::IsNullOrWhiteSpace($ContentPath)) {
    $ContentPath = Join-Path $ProjectRoot "Content"
}
if ([string]::IsNullOrWhiteSpace($ProjectPath)) {
    $ProjectPath = Get-HorrorProjectFile -ProjectRoot $ProjectRoot
}

Write-Host "Finding unused assets in: $ContentPath" -ForegroundColor Cyan
Write-Host "This script provides estimates - verify in UE5 Reference Viewer" -ForegroundColor Yellow
Write-Host ""

# Results tracking
$results = @{
    TotalAssets = 0
    PotentiallyUnused = @()
    TotalSize = 0
}

# Folders to always exclude
$excludeFolders = @(
    'Developers',
    'Collections',
    '__ExternalActors__',
    '__ExternalObjects__'
)

# Asset types that are typically referenced
$referencedTypes = @(
    'Material',
    'MaterialInstance',
    'Texture',
    'StaticMesh',
    'SkeletalMesh',
    'Blueprint',
    'Sound'
)

function Get-AssetReferences {
    param([string]$AssetPath)

    # This is a simplified check - UE5 has complex reference tracking
    # For accurate results, use UE5 Editor's Reference Viewer

    $references = @()
    $assetName = [System.IO.Path]::GetFileNameWithoutExtension($AssetPath)

    # Search for references in other asset files
    Get-ChildItem -Path $ContentPath -Recurse -Include *.uasset,*.umap | ForEach-Object {
        if ($_.FullName -ne $AssetPath) {
            # Simple text search in binary file (not 100% accurate)
            $content = Get-Content $_.FullName -Raw -Encoding Byte -TotalCount 10000
            $contentStr = [System.Text.Encoding]::ASCII.GetString($content)

            if ($contentStr -match [regex]::Escape($assetName)) {
                $references += $_.FullName
            }
        }
    }

    return $references
}

function Get-FolderSize {
    param([string]$Path)

    $size = 0
    Get-ChildItem -Path $Path -Recurse -File -ErrorAction SilentlyContinue | ForEach-Object {
        $size += $_.Length
    }
    return $size
}

function Format-FileSize {
    param([long]$Size)

    if ($Size -gt 1GB) {
        return "{0:N2} GB" -f ($Size / 1GB)
    } elseif ($Size -gt 1MB) {
        return "{0:N2} MB" -f ($Size / 1MB)
    } elseif ($Size -gt 1KB) {
        return "{0:N2} KB" -f ($Size / 1KB)
    } else {
        return "$Size bytes"
    }
}

# Scan all assets
Write-Host "Scanning assets..." -ForegroundColor Cyan

$allAssets = Get-ChildItem -Path $ContentPath -Recurse -Include *.uasset,*.umap | Where-Object {
    $exclude = $false
    foreach ($folder in $excludeFolders) {
        if ($_.FullName -match [regex]::Escape($folder)) {
            $exclude = $true
            break
        }
    }

    # Optionally exclude maps
    if ($ExcludeMaps -and $_.Extension -eq '.umap') {
        $exclude = $true
    }

    -not $exclude
}

$results.TotalAssets = $allAssets.Count
Write-Host "Found $($allAssets.Count) assets to check" -ForegroundColor White
Write-Host ""

# Check each asset for references
$progress = 0
foreach ($asset in $allAssets) {
    $progress++

    if ($progress % 50 -eq 0) {
        Write-Host "Progress: $progress / $($allAssets.Count)" -ForegroundColor Gray
    }

    # Skip certain asset types that are always "used"
    $assetName = $asset.Name

    # Skip if it's a map
    if ($asset.Extension -eq '.umap') {
        continue
    }

    # Skip if it's in a special folder
    $relativePath = $asset.FullName.Replace($ContentPath, '')

    # Check for references (simplified)
    $references = Get-AssetReferences -AssetPath $asset.FullName

    if ($references.Count -eq 0) {
        $size = $asset.Length
        $results.TotalSize += $size

        $unusedAsset = [PSCustomObject]@{
            Name = $asset.Name
            Path = $relativePath
            FullPath = $asset.FullName
            Size = $size
            SizeFormatted = Format-FileSize -Size $size
            Type = $asset.Extension
        }

        $results.PotentiallyUnused += $unusedAsset

        if ($Verbose) {
            Write-Host "Potentially unused: $($asset.Name) ($($unusedAsset.SizeFormatted))" -ForegroundColor Yellow
        }
    }
}

# Sort by size (largest first)
$results.PotentiallyUnused = $results.PotentiallyUnused | Sort-Object -Property Size -Descending

# Summary
Write-Host ""
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "Unused Assets Report" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "Total Assets Scanned: $($results.TotalAssets)" -ForegroundColor White
Write-Host "Potentially Unused:   $($results.PotentiallyUnused.Count)" -ForegroundColor Yellow
Write-Host "Potential Space Savings: $(Format-FileSize -Size $results.TotalSize)" -ForegroundColor Green
Write-Host ""

# Show top 20 largest unused assets
if ($results.PotentiallyUnused.Count -gt 0) {
    Write-Host "Top 20 Largest Potentially Unused Assets:" -ForegroundColor Cyan
    Write-Host ""

    $top20 = $results.PotentiallyUnused | Select-Object -First 20
    foreach ($asset in $top20) {
        Write-Host "  $($asset.SizeFormatted.PadRight(12)) $($asset.Name)" -ForegroundColor White
        Write-Host "    $($asset.Path)" -ForegroundColor Gray
    }

    Write-Host ""
}

# Export full report
$reportPath = Join-Path $ContentPath "..\Docs\Assets\UnusedAssets.csv"
$results.PotentiallyUnused | Export-Csv -Path $reportPath -NoTypeInformation
Write-Host "Full report exported to: $reportPath" -ForegroundColor Green
Write-Host ""

# Warnings
Write-Host "⚠️  IMPORTANT WARNINGS:" -ForegroundColor Red
Write-Host "  1. This script uses simplified reference detection" -ForegroundColor Yellow
Write-Host "  2. Some assets may be used in ways not detected by this script" -ForegroundColor Yellow
Write-Host "  3. ALWAYS verify in UE5 Editor before deleting:" -ForegroundColor Yellow
Write-Host "     - Right-click asset → Reference Viewer" -ForegroundColor Yellow
Write-Host "     - Check 'Show Referencers' to see what uses it" -ForegroundColor Yellow
Write-Host "  4. Move to Archive folder first, test thoroughly, then delete" -ForegroundColor Yellow
Write-Host ""

# Recommendations
Write-Host "Recommended Actions:" -ForegroundColor Cyan
Write-Host "  1. Open UE5 Editor" -ForegroundColor White
Write-Host "  2. Use Size Map to visualize asset sizes" -ForegroundColor White
Write-Host "  3. Use Reference Viewer to verify unused assets" -ForegroundColor White
Write-Host "  4. Move unused assets to Content/Archive/" -ForegroundColor White
Write-Host "  5. Test project thoroughly for 1 week" -ForegroundColor White
Write-Host "  6. Delete Archive folder if no issues" -ForegroundColor White
Write-Host ""

exit 0
