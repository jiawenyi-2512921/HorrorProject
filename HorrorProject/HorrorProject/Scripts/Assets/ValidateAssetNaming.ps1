# Asset Naming Validation Script
# Validates project-owned asset naming conventions for HorrorProject.

param(
    [string]$ContentPath = "",
    [switch]$FixIssues = $false,
    [switch]$Verbose = $false,
    [switch]$Detailed = $false,
    [switch]$IncludeExternalContent = $false,
    [string[]]$ExternalFolderNames = @(
        'Bodycam_VHS_Effect',
        'Fab',
        'Grimytheus_Vol_2',
        'IndustrialPipesM',
        'SD_Art',
        'SoundsOfHorror',
        'Stone_Golem',
        '_SM13',
        '__ExternalActors__',
        '__ExternalObjects__',
        'Collections',
        'Developers',
        'DeepWaterStation',
        'Variant_Horror'
    )
)

. (Join-Path (Split-Path -Parent $PSScriptRoot) "Validation\Common.ps1")

$ProjectRoot = Get-HorrorProjectRoot -StartPath $PSScriptRoot
if ([string]::IsNullOrWhiteSpace($ContentPath)) {
    $ContentPath = Join-Path $ProjectRoot "Content"
}

$ContentRootFullPath = [System.IO.Path]::GetFullPath($ContentPath).TrimEnd('\')

# Valid prefixes by asset type. The list intentionally includes common UE5,
# Enhanced Input, audio, animation, and marketplace-import conventions.
$validPrefixes = @{
    'uasset' = @(
        'SM_', 'SK_', 'SKM_', 'SKP_',
        'M_', 'MI_', 'MF_', 'ML_', 'MPC_', 'PM_',
        'T_', 'RT_', 'TC_',
        'BP_', 'BPFL_', 'BPI_', 'BPML_', 'WBP_',
        'E_', 'F_', 'DA_', 'DT_', 'CT_', 'CF_', 'CV_', 'CLC_',
        'A_', 'ABP_', 'AM_', 'BS_', 'BS1D_', 'AO_', 'AO1D_', 'AC_',
        'S_', 'SW_', 'SC_', 'SA_', 'SCon_', 'SCl_', 'CUE_', 'WAV_',
        'DV_', 'DW_', 'P_', 'PS_', 'NS_', 'NE_', 'NF_', 'NM_', 'NPC_',
        'PHYS_', 'DM_', 'LF_', 'LP_',
        'MP_', 'MT_', 'MS_', 'Font_', 'FF_',
        'IA_', 'IMC_', 'GM_', 'MM_', 'CA_', 'CR_', 'PA_', 'UI_', 'LVL_'
    )
    'umap' = @('L_', 'LVL_')
}

$validTextureSuffixes = @(
    '_D', '_BC', '_BaseColor',
    '_N', '_BN', '_Normal',
    '_R', '_Roughness',
    '_M', '_Metallic',
    '_AO',
    '_E', '_Emissive',
    '_O', '_Opacity',
    '_A', '_Alpha',
    '_ORM', '_MRA', '_MSK', '_Mask',
    '_H', '_Height'
)

$results = @{
    Total = 0
    Valid = 0
    Invalid = 0
    SkippedExternal = 0
    Issues = @()
}

function Get-ContentTopLevelFolder {
    param(
        [string]$FilePath
    )

    $fullPath = [System.IO.Path]::GetFullPath($FilePath)
    if (-not $fullPath.StartsWith($ContentRootFullPath, [System.StringComparison]::OrdinalIgnoreCase)) {
        return ""
    }

    $relativePath = $fullPath.Substring($ContentRootFullPath.Length).TrimStart([char[]]@('\', '/'))
    if ([string]::IsNullOrWhiteSpace($relativePath)) {
        return ""
    }

    return ($relativePath -split '[\\/]')[0]
}

function Test-IsExternalContentAsset {
    param(
        [System.IO.FileInfo]$File
    )

    $topLevelFolder = Get-ContentTopLevelFolder -FilePath $File.FullName
    foreach ($externalFolder in $ExternalFolderNames) {
        if ($topLevelFolder.Equals($externalFolder, [System.StringComparison]::OrdinalIgnoreCase)) {
            return $true
        }
    }

    return $false
}

function Test-AssetName {
    param(
        [string]$FileName,
        [string]$Extension
    )

    $issues = @()
    $baseName = [System.IO.Path]::GetFileNameWithoutExtension($FileName)

    if ($baseName -match '\s') {
        $issues += "Contains spaces"
    }

    # PowerShell -match is case-insensitive by default; use -cmatch here.
    if ($baseName -cmatch '^[a-z]') {
        $issues += "Starts with lowercase"
    }

    $hasValidPrefix = $false
    if ($validPrefixes.ContainsKey($Extension)) {
        foreach ($prefix in $validPrefixes[$Extension]) {
            if ($baseName.StartsWith($prefix, [System.StringComparison]::Ordinal)) {
                $hasValidPrefix = $true
                break
            }
        }
    }

    if ($baseName -cmatch '^(TEMP_|PROTO_|DEPRECATED_|TEST_)') {
        $hasValidPrefix = $true
    }

    if (-not $hasValidPrefix -and $validPrefixes.ContainsKey($Extension)) {
        $issues += "Missing or invalid prefix"
    }

    if ($baseName.StartsWith('T_', [System.StringComparison]::Ordinal) -and $baseName -cmatch '_[A-Z]+$') {
        $suffix = $baseName -replace '.*(_[A-Z]+)$', '$1'
        if ($validTextureSuffixes -notcontains $suffix) {
            $issues += "Invalid texture suffix: $suffix"
        }
    }

    if ($baseName -match '_\d$') {
        $issues += "Single digit variation (should be 01, 02, etc.)"
    }

    if ($baseName.Length -lt 3) {
        $issues += "Name too short (min 3 characters)"
    }
    if ($baseName.Length -gt 64) {
        $issues += "Name too long (max 64 characters)"
    }

    if ($baseName -match '[^a-zA-Z0-9_-]') {
        $issues += "Contains invalid characters"
    }

    return $issues
}

function Get-SuggestedName {
    param(
        [string]$FileName,
        [string]$FolderPath
    )

    $baseName = [System.IO.Path]::GetFileNameWithoutExtension($FileName)
    $extension = [System.IO.Path]::GetExtension($FileName)

    $suggested = $baseName -replace '\s', '_'
    $suggested = $suggested -replace '_(\d)$', '_0$1'

    $suggested = ($suggested -split '_' | ForEach-Object {
        if ($_.Length -gt 0) {
            $_.Substring(0, 1).ToUpperInvariant() + $_.Substring(1)
        }
    }) -join '_'

    if (-not ($suggested -cmatch '^[A-Z][A-Z0-9]*_')) {
        $prefixMap = @{
            'Meshes' = 'SM_'
            'Materials' = 'MI_'
            'Textures' = 'T_'
            'Blueprints' = 'BP_'
            'Audio' = 'SW_'
            'Sounds' = 'SW_'
            'Animations' = 'A_'
            'Particles' = 'PS_'
            'Niagara' = 'NS_'
            'Input' = 'IA_'
            'Maps' = 'L_'
            'Levels' = 'L_'
        }

        foreach ($key in $prefixMap.Keys) {
            if ($FolderPath -match [regex]::Escape($key)) {
                $suggested = $prefixMap[$key] + $suggested
                break
            }
        }
    }

    return $suggested + $extension
}

Write-Host "Scanning assets in: $ContentPath" -ForegroundColor Cyan
if (-not $IncludeExternalContent) {
    Write-Host "External/vendor content folders are skipped by default. Use -IncludeExternalContent for a full audit." -ForegroundColor DarkGray
}
Write-Host ""

$assets = Get-ChildItem -Path $ContentPath -Recurse -File -Include *.uasset,*.umap
foreach ($file in $assets) {
    if (-not $IncludeExternalContent -and (Test-IsExternalContentAsset -File $file)) {
        $results.SkippedExternal++
        continue
    }

    $results.Total++
    $extension = $file.Extension.TrimStart('.')
    $issues = Test-AssetName -FileName $file.Name -Extension $extension

    if ($issues.Count -eq 0) {
        $results.Valid++
        if ($Verbose -or $Detailed) {
            Write-Host "[OK] $($file.Name)" -ForegroundColor Green
        }
        continue
    }

    $results.Invalid++
    $issueObj = [PSCustomObject]@{
        File = $file.FullName
        Name = $file.Name
        Issues = $issues -join ', '
        Suggested = Get-SuggestedName -FileName $file.Name -FolderPath $file.DirectoryName
    }

    $results.Issues += $issueObj

    if ($Verbose -or $Detailed) {
        Write-Host "[ISSUE] $($file.Name)" -ForegroundColor Red
        Write-Host "  Issues: $($issues -join ', ')" -ForegroundColor Yellow
        Write-Host "  Suggested: $($issueObj.Suggested)" -ForegroundColor Cyan
        Write-Host ""
    }
}

Write-Host ""
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "Validation Summary" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "Total Assets:   $($results.Total)" -ForegroundColor White
Write-Host "Skipped External/Vendor: $($results.SkippedExternal)" -ForegroundColor DarkGray

$validPercent = if ($results.Total -gt 0) { [math]::Round($results.Valid / $results.Total * 100, 1) } else { 100 }
$invalidPercent = if ($results.Total -gt 0) { [math]::Round($results.Invalid / $results.Total * 100, 1) } else { 0 }
Write-Host "Valid:          $($results.Valid) ($validPercent%)" -ForegroundColor Green
Write-Host "Invalid:        $($results.Invalid) ($invalidPercent%)" -ForegroundColor Red
Write-Host ""

$reportPath = Join-Path $ContentPath "..\Docs\Assets\NamingIssues.csv"
if ($results.Invalid -gt 0) {
    $reportDir = Split-Path -Parent $reportPath
    if (-not (Test-Path $reportDir)) {
        New-Item -ItemType Directory -Path $reportDir -Force | Out-Null
    }

    $results.Issues | Export-Csv -Path $reportPath -NoTypeInformation
    Write-Host "Issues exported to: $reportPath" -ForegroundColor Yellow
    if (-not ($Verbose -or $Detailed)) {
        Write-Host "Run with -Detailed to print individual asset issues." -ForegroundColor DarkGray
    }
    Write-Host ""
} elseif (Test-Path $reportPath) {
    Remove-Item -Path $reportPath -Force
}

$renamePlanPath = Join-Path $ContentPath "..\Docs\Assets\NamingRenamePlan.json"
if ($FixIssues -and $results.Invalid -gt 0) {
    $renamePlan = $results.Issues | ForEach-Object {
        [PSCustomObject]@{
            SourceFile = $_.File
            CurrentName = $_.Name
            SuggestedName = $_.Suggested
            Issues = $_.Issues
            RequiresEditorRename = $true
        }
    }

    $renamePlan | ConvertTo-Json -Depth 4 | Set-Content -Path $renamePlanPath -Encoding UTF8
    Write-Host "Rename plan generated: $renamePlanPath" -ForegroundColor Yellow
    Write-Host "Apply the plan through UE Editor asset rename APIs so references are updated safely." -ForegroundColor Yellow
} elseif ($FixIssues -and (Test-Path $renamePlanPath)) {
    Remove-Item -Path $renamePlanPath -Force
}

if ($results.Invalid -gt 0) {
    exit 1
}

exit 0
