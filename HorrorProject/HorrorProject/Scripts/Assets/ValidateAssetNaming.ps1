# Asset Naming Validation Script
# Validates asset naming conventions for HorrorProject

param(
    [string]$ContentPath = "D:\gptzuo\HorrorProject\HorrorProject\Content",
    [switch]$FixIssues = $false,
    [switch]$Verbose = $false
)

# Valid prefixes by asset type
$validPrefixes = @{
    'uasset' = @('SM_', 'SK_', 'SKP_', 'M_', 'MI_', 'MF_', 'MPC_', 'PM_', 'T_', 'RT_', 'TC_',
                 'BP_', 'BPFL_', 'BPI_', 'BPML_', 'E_', 'F_', 'WBP_',
                 'A_', 'ABP_', 'AM_', 'BS_', 'BS1D_', 'AO_', 'AO1D_', 'AC_', 'S_',
                 'SW_', 'SC_', 'SA_', 'SCon_', 'SCl_', 'SM_', 'DV_', 'DW_',
                 'PS_', 'NS_', 'NE_', 'NF_', 'NM_', 'NPC_',
                 'PHYS_', 'DM_', 'LF_', 'LP_',
                 'L_', 'DA_', 'DT_', 'CT_', 'CF_', 'CV_', 'CLC_',
                 'MP_', 'MT_', 'MS_', 'Font_', 'FF_')
    'umap' = @('L_')
}

# Texture suffix validation
$validTextureSuffixes = @('_D', '_N', '_R', '_M', '_AO', '_E', '_O', '_ORM', '_Mask', '_H')

# Results tracking
$results = @{
    Total = 0
    Valid = 0
    Invalid = 0
    Issues = @()
}

function Test-AssetName {
    param(
        [string]$FileName,
        [string]$Extension
    )

    $issues = @()
    $baseName = [System.IO.Path]::GetFileNameWithoutExtension($FileName)

    # Check for spaces
    if ($baseName -match '\s') {
        $issues += "Contains spaces"
    }

    # Check for lowercase start (except after underscore)
    if ($baseName -match '^[a-z]') {
        $issues += "Starts with lowercase"
    }

    # Check for valid prefix
    $hasValidPrefix = $false
    if ($validPrefixes.ContainsKey($Extension)) {
        foreach ($prefix in $validPrefixes[$Extension]) {
            if ($baseName.StartsWith($prefix)) {
                $hasValidPrefix = $true
                break
            }
        }
    }

    # Allow TEMP_, PROTO_, DEPRECATED_, TEST_ prefixes
    if ($baseName -match '^(TEMP_|PROTO_|DEPRECATED_|TEST_)') {
        $hasValidPrefix = $true
    }

    if (-not $hasValidPrefix -and $Extension -eq 'uasset') {
        $issues += "Missing or invalid prefix"
    }

    # Check texture naming
    if ($baseName.StartsWith('T_') -and $baseName -match '_[A-Z]+$') {
        $suffix = $baseName -replace '.*(_[A-Z]+)$', '$1'
        if ($validTextureSuffixes -notcontains $suffix) {
            $issues += "Invalid texture suffix: $suffix"
        }
    }

    # Check for single digit variations (should be 01, 02, not 1, 2)
    if ($baseName -match '_\d$') {
        $issues += "Single digit variation (should be 01, 02, etc.)"
    }

    # Check length
    if ($baseName.Length -lt 3) {
        $issues += "Name too short (min 3 characters)"
    }
    if ($baseName.Length -gt 64) {
        $issues += "Name too long (max 64 characters)"
    }

    # Check for invalid characters
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

    # Remove spaces
    $suggested = $baseName -replace '\s', '_'

    # Fix single digit variations
    $suggested = $suggested -replace '_(\d)$', '_0$1'

    # Capitalize after underscores
    $suggested = ($suggested -split '_' | ForEach-Object {
        if ($_.Length -gt 0) {
            $_.Substring(0,1).ToUpper() + $_.Substring(1)
        }
    }) -join '_'

    # Try to add prefix based on folder
    if (-not ($suggested -match '^[A-Z]+_')) {
        $folderName = Split-Path $FolderPath -Leaf

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
        }

        foreach ($key in $prefixMap.Keys) {
            if ($FolderPath -match $key) {
                $suggested = $prefixMap[$key] + $suggested
                break
            }
        }
    }

    return $suggested + $extension
}

# Scan assets
Write-Host "Scanning assets in: $ContentPath" -ForegroundColor Cyan
Write-Host ""

Get-ChildItem -Path $ContentPath -Recurse -Include *.uasset,*.umap | ForEach-Object {
    $results.Total++
    $file = $_
    $extension = $_.Extension.TrimStart('.')

    $issues = Test-AssetName -FileName $_.Name -Extension $extension

    if ($issues.Count -eq 0) {
        $results.Valid++
        if ($Verbose) {
            Write-Host "✓ $($_.Name)" -ForegroundColor Green
        }
    } else {
        $results.Invalid++

        $issueObj = [PSCustomObject]@{
            File = $_.FullName
            Name = $_.Name
            Issues = $issues -join ', '
            Suggested = Get-SuggestedName -FileName $_.Name -FolderPath $_.DirectoryName
        }

        $results.Issues += $issueObj

        Write-Host "✗ $($_.Name)" -ForegroundColor Red
        Write-Host "  Issues: $($issues -join ', ')" -ForegroundColor Yellow
        Write-Host "  Suggested: $($issueObj.Suggested)" -ForegroundColor Cyan
        Write-Host ""
    }
}

# Summary
Write-Host ""
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "Validation Summary" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "Total Assets:   $($results.Total)" -ForegroundColor White
Write-Host "Valid:          $($results.Valid) ($([math]::Round($results.Valid/$results.Total*100, 1))%)" -ForegroundColor Green
Write-Host "Invalid:        $($results.Invalid) ($([math]::Round($results.Invalid/$results.Total*100, 1))%)" -ForegroundColor Red
Write-Host ""

# Export issues to CSV
if ($results.Invalid -gt 0) {
    $reportPath = Join-Path $ContentPath "..\Docs\Assets\NamingIssues.csv"
    $results.Issues | Export-Csv -Path $reportPath -NoTypeInformation
    Write-Host "Issues exported to: $reportPath" -ForegroundColor Yellow
    Write-Host ""
}

# Fix issues if requested
if ($FixIssues -and $results.Invalid -gt 0) {
    Write-Host "Fix mode not implemented - requires UE5 Editor integration" -ForegroundColor Yellow
    Write-Host "Please use UE5 Content Browser to rename assets" -ForegroundColor Yellow
    Write-Host "This ensures references are updated correctly" -ForegroundColor Yellow
}

# Exit code
if ($results.Invalid -gt 0) {
    exit 1
} else {
    exit 0
}
