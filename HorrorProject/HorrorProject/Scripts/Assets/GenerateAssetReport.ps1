# Asset Report Generator
# Generates comprehensive asset reports for HorrorProject

param(
    [string]$ContentPath = "",
    [string]$OutputPath = "",
    [switch]$IncludeDetails = $false
)

. (Join-Path (Split-Path -Parent $PSScriptRoot) "Validation\Common.ps1")

$ProjectRoot = Get-HorrorProjectRoot -StartPath $PSScriptRoot
if ([string]::IsNullOrWhiteSpace($ContentPath)) {
    $ContentPath = Join-Path $ProjectRoot "Content"
}
if ([string]::IsNullOrWhiteSpace($OutputPath)) {
    $OutputPath = Join-Path $ProjectRoot "Docs\Assets\Reports"
}

# Create output directory
New-Item -ItemType Directory -Force -Path $OutputPath | Out-Null

Write-Host "Generating Asset Reports..." -ForegroundColor Cyan
Write-Host "Content Path: $ContentPath" -ForegroundColor White
Write-Host "Output Path: $OutputPath" -ForegroundColor White
Write-Host ""

# Helper functions
function Format-FileSize {
    param([long]$Size)
    if ($Size -gt 1GB) { return "{0:N2} GB" -f ($Size / 1GB) }
    elseif ($Size -gt 1MB) { return "{0:N2} MB" -f ($Size / 1MB) }
    elseif ($Size -gt 1KB) { return "{0:N2} KB" -f ($Size / 1KB) }
    else { return "$Size bytes" }
}

function Get-FolderStats {
    param([string]$Path)

    $files = Get-ChildItem -Path $Path -Recurse -File -ErrorAction SilentlyContinue
    $totalSize = ($files | Measure-Object -Property Length -Sum).Sum
    $assetCount = ($files | Where-Object { $_.Extension -in @('.uasset', '.umap') }).Count

    return @{
        TotalSize = $totalSize
        AssetCount = $assetCount
        FileCount = $files.Count
    }
}

# Report 1: Package Summary
Write-Host "Generating Package Summary..." -ForegroundColor Cyan

$packages = @()
Get-ChildItem -Path $ContentPath -Directory | ForEach-Object {
    $stats = Get-FolderStats -Path $_.FullName

    $packages += [PSCustomObject]@{
        Package = $_.Name
        Size = Format-FileSize -Size $stats.TotalSize
        SizeBytes = $stats.TotalSize
        Assets = $stats.AssetCount
        TotalFiles = $stats.FileCount
        Path = $_.FullName
    }
}

$packages = $packages | Sort-Object -Property SizeBytes -Descending
$packages | Export-Csv -Path (Join-Path $OutputPath "PackageSummary.csv") -NoTypeInformation

Write-Host "  ✓ Package Summary saved" -ForegroundColor Green

# Report 2: Asset Type Breakdown
Write-Host "Generating Asset Type Breakdown..." -ForegroundColor Cyan

$assetTypes = @{}
Get-ChildItem -Path $ContentPath -Recurse -Include *.uasset,*.umap | ForEach-Object {
    $name = $_.Name
    $type = "Unknown"

    # Detect type by prefix
    if ($name -match '^(SM_|SK_|SKP_)') { $type = "Mesh" }
    elseif ($name -match '^(M_|MI_|MF_|MPC_)') { $type = "Material" }
    elseif ($name -match '^(T_|RT_|TC_)') { $type = "Texture" }
    elseif ($name -match '^(BP_|BPFL_|BPI_)') { $type = "Blueprint" }
    elseif ($name -match '^(A_|ABP_|AM_|BS_)') { $type = "Animation" }
    elseif ($name -match '^(SW_|SC_|SA_)') { $type = "Audio" }
    elseif ($name -match '^(NS_|NE_|PS_)') { $type = "VFX" }
    elseif ($name -match '^(WBP_)') { $type = "UI" }
    elseif ($_.Extension -eq '.umap') { $type = "Map" }

    if (-not $assetTypes.ContainsKey($type)) {
        $assetTypes[$type] = @{
            Count = 0
            Size = 0
        }
    }

    $assetTypes[$type].Count++
    $assetTypes[$type].Size += $_.Length
}

$typeBreakdown = @()
foreach ($type in $assetTypes.Keys) {
    $typeBreakdown += [PSCustomObject]@{
        Type = $type
        Count = $assetTypes[$type].Count
        Size = Format-FileSize -Size $assetTypes[$type].Size
        SizeBytes = $assetTypes[$type].Size
    }
}

$typeBreakdown = $typeBreakdown | Sort-Object -Property SizeBytes -Descending
$typeBreakdown | Export-Csv -Path (Join-Path $OutputPath "AssetTypeBreakdown.csv") -NoTypeInformation

Write-Host "  ✓ Asset Type Breakdown saved" -ForegroundColor Green

# Report 3: Large Assets
Write-Host "Generating Large Assets Report..." -ForegroundColor Cyan

$largeAssets = @()
Get-ChildItem -Path $ContentPath -Recurse -Include *.uasset,*.umap |
    Where-Object { $_.Length -gt 10MB } |
    ForEach-Object {
        $relativePath = $_.FullName.Replace($ContentPath, '').TrimStart('\')

        $largeAssets += [PSCustomObject]@{
            Name = $_.Name
            Size = Format-FileSize -Size $_.Length
            SizeBytes = $_.Length
            Path = $relativePath
            Modified = $_.LastWriteTime
        }
    }

$largeAssets = $largeAssets | Sort-Object -Property SizeBytes -Descending
$largeAssets | Export-Csv -Path (Join-Path $OutputPath "LargeAssets.csv") -NoTypeInformation

Write-Host "  ✓ Large Assets Report saved ($(($largeAssets).Count) assets >10MB)" -ForegroundColor Green

# Report 4: Texture Analysis
Write-Host "Generating Texture Analysis..." -ForegroundColor Cyan

$textures = @()
Get-ChildItem -Path $ContentPath -Recurse -Include *.uasset |
    Where-Object { $_.Name -match '^T_' } |
    ForEach-Object {
        $relativePath = $_.FullName.Replace($ContentPath, '').TrimStart('\')
        $suffix = "Unknown"

        if ($_.Name -match '_([A-Z]+)\.uasset$') {
            $suffix = $matches[1]
        }

        $textures += [PSCustomObject]@{
            Name = $_.Name
            Suffix = $suffix
            Size = Format-FileSize -Size $_.Length
            SizeBytes = $_.Length
            Path = $relativePath
        }
    }

$textures = $textures | Sort-Object -Property SizeBytes -Descending
$textures | Export-Csv -Path (Join-Path $OutputPath "TextureAnalysis.csv") -NoTypeInformation

# Texture suffix summary
$textureSuffixes = $textures | Group-Object -Property Suffix | ForEach-Object {
    $totalSize = ($_.Group | Measure-Object -Property SizeBytes -Sum).Sum
    [PSCustomObject]@{
        Suffix = $_.Name
        Count = $_.Count
        TotalSize = Format-FileSize -Size $totalSize
        TotalSizeBytes = $totalSize
    }
} | Sort-Object -Property TotalSizeBytes -Descending

$textureSuffixes | Export-Csv -Path (Join-Path $OutputPath "TextureSuffixSummary.csv") -NoTypeInformation

Write-Host "  ✓ Texture Analysis saved ($(($textures).Count) textures)" -ForegroundColor Green

# Report 5: Material Analysis
Write-Host "Generating Material Analysis..." -ForegroundColor Cyan

$materials = @()
$materialInstances = @()

Get-ChildItem -Path $ContentPath -Recurse -Include *.uasset | ForEach-Object {
    $relativePath = $_.FullName.Replace($ContentPath, '').TrimStart('\')

    if ($_.Name -match '^M_') {
        $materials += [PSCustomObject]@{
            Name = $_.Name
            Size = Format-FileSize -Size $_.Length
            SizeBytes = $_.Length
            Path = $relativePath
        }
    }
    elseif ($_.Name -match '^MI_') {
        $materialInstances += [PSCustomObject]@{
            Name = $_.Name
            Size = Format-FileSize -Size $_.Length
            SizeBytes = $_.Length
            Path = $relativePath
        }
    }
}

$materials | Sort-Object -Property SizeBytes -Descending |
    Export-Csv -Path (Join-Path $OutputPath "Materials.csv") -NoTypeInformation

$materialInstances | Sort-Object -Property SizeBytes -Descending |
    Export-Csv -Path (Join-Path $OutputPath "MaterialInstances.csv") -NoTypeInformation

Write-Host "  ✓ Material Analysis saved ($($materials.Count) materials, $($materialInstances.Count) instances)" -ForegroundColor Green

# Report 6: Audio Analysis
Write-Host "Generating Audio Analysis..." -ForegroundColor Cyan

$audioAssets = @()
Get-ChildItem -Path $ContentPath -Recurse -Include *.uasset |
    Where-Object { $_.Name -match '^(SW_|SC_)' } |
    ForEach-Object {
        $relativePath = $_.FullName.Replace($ContentPath, '').TrimStart('\')
        $type = if ($_.Name -match '^SW_') { "Sound Wave" } else { "Sound Cue" }

        $audioAssets += [PSCustomObject]@{
            Name = $_.Name
            Type = $type
            Size = Format-FileSize -Size $_.Length
            SizeBytes = $_.Length
            Path = $relativePath
        }
    }

$audioAssets | Sort-Object -Property SizeBytes -Descending |
    Export-Csv -Path (Join-Path $OutputPath "AudioAssets.csv") -NoTypeInformation

Write-Host "  ✓ Audio Analysis saved ($(($audioAssets).Count) audio assets)" -ForegroundColor Green

# Generate Summary Report
Write-Host ""
Write-Host "Generating Summary Report..." -ForegroundColor Cyan

$totalStats = Get-FolderStats -Path $ContentPath

$summary = @"
# Asset Report Summary - HorrorProject
Generated: $(Get-Date -Format "yyyy-MM-dd HH:mm:ss")

## Overall Statistics
- Total Size: $(Format-FileSize -Size $totalStats.TotalSize)
- Total Assets: $($totalStats.AssetCount)
- Total Files: $($totalStats.FileCount)

## Top 5 Packages by Size
$($packages | Select-Object -First 5 | ForEach-Object { "- $($_.Package): $($_.Size) ($($_.Assets) assets)" } | Out-String)

## Asset Type Distribution
$($typeBreakdown | ForEach-Object { "- $($_.Type): $($_.Count) assets ($($_.Size))" } | Out-String)

## Large Assets (>10MB)
- Count: $(($largeAssets).Count)
- Total Size: $(Format-FileSize -Size (($largeAssets | Measure-Object -Property SizeBytes -Sum).Sum))

## Texture Statistics
- Total Textures: $(($textures).Count)
- Total Size: $(Format-FileSize -Size (($textures | Measure-Object -Property SizeBytes -Sum).Sum))
- Suffix Breakdown:
$($textureSuffixes | ForEach-Object { "  - $($_.Suffix): $($_.Count) textures ($($_.TotalSize))" } | Out-String)

## Material Statistics
- Master Materials: $($materials.Count)
- Material Instances: $($materialInstances.Count)
- Total Size: $(Format-FileSize -Size ((($materials + $materialInstances) | Measure-Object -Property SizeBytes -Sum).Sum))

## Audio Statistics
- Total Audio Assets: $(($audioAssets).Count)
- Total Size: $(Format-FileSize -Size (($audioAssets | Measure-Object -Property SizeBytes -Sum).Sum))

## Detailed Reports
All detailed reports have been exported to: $OutputPath

- PackageSummary.csv
- AssetTypeBreakdown.csv
- LargeAssets.csv
- TextureAnalysis.csv
- TextureSuffixSummary.csv
- Materials.csv
- MaterialInstances.csv
- AudioAssets.csv

## Recommendations
1. Review large assets (>10MB) for optimization opportunities
2. Compress textures using BC7/BC5 formats
3. Consolidate duplicate material instances
4. Generate LODs for meshes
5. Enable texture streaming for large textures

---
Generated by Asset Manager Agent
"@

$summary | Out-File -FilePath (Join-Path $OutputPath "Summary.md") -Encoding UTF8

Write-Host "  ✓ Summary Report saved" -ForegroundColor Green
Write-Host ""

# Display summary
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "Asset Report Generation Complete" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""
Write-Host "Total Size:   $(Format-FileSize -Size $totalStats.TotalSize)" -ForegroundColor White
Write-Host "Total Assets: $($totalStats.AssetCount)" -ForegroundColor White
Write-Host ""
Write-Host "Reports saved to: $OutputPath" -ForegroundColor Green
Write-Host ""
Write-Host "Top 5 Packages:" -ForegroundColor Cyan
$packages | Select-Object -First 5 | ForEach-Object {
    Write-Host "  $($_.Package.PadRight(30)) $($_.Size.PadLeft(12)) ($($_.Assets) assets)" -ForegroundColor White
}
Write-Host ""

exit 0
