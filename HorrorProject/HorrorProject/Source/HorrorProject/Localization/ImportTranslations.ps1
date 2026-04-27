# Import Translations from External Sources
# Usage: .\ImportTranslations.ps1 -InputFile "translations.csv"

param(
    [string]$InputFile,
    [string]$LocalizationPath = "",
    [switch]$Merge = $false
)

$ErrorActionPreference = "Stop"

. (Join-Path $PSScriptRoot "..\..\..\Scripts\Validation\Common.ps1")
$ProjectRoot = Get-HorrorProjectRoot -StartPath $PSScriptRoot
if ([string]::IsNullOrWhiteSpace($LocalizationPath)) { $LocalizationPath = Join-Path $ProjectRoot "Content\Localization" }

if (-not $InputFile) {
    Write-Host "Usage: .\ImportTranslations.ps1 -InputFile <path> [-Merge]"
    Write-Host ""
    Write-Host "Supported formats:"
    Write-Host "  - CSV with columns: Key, English, Chinese, Japanese, Korean, Spanish"
    Write-Host "  - JSON with language code as keys"
    Write-Host ""
    Write-Host "Options:"
    Write-Host "  -Merge: Merge with existing translations instead of replacing"
    exit 1
}

if (-not (Test-Path $InputFile)) {
    Write-Host "Error: Input file not found: $InputFile"
    exit 1
}

Write-Host "Importing translations from: $InputFile"

$Extension = [System.IO.Path]::GetExtension($InputFile).ToLower()
$ImportedData = @{}

# Load input file based on format
switch ($Extension) {
    ".csv" {
        Write-Host "Detected CSV format"
        $CsvData = Import-Csv -Path $InputFile -Encoding UTF8

        foreach ($Row in $CsvData) {
            $Key = $Row.Key
            if (-not $Key) { continue }

            $ImportedData[$Key] = @{
                "en" = $Row.English
                "zh-Hans" = $Row.Chinese
                "ja" = $Row.Japanese
                "ko" = $Row.Korean
                "es" = $Row.Spanish
            }
        }
    }
    ".json" {
        Write-Host "Detected JSON format"
        $JsonData = Get-Content -Path $InputFile -Raw -Encoding UTF8 | ConvertFrom-Json

        # Assume structure: { "key": { "en": "text", "zh-Hans": "text", ... } }
        foreach ($Property in $JsonData.PSObject.Properties) {
            $Key = $Property.Name
            $ImportedData[$Key] = @{}

            foreach ($LangProperty in $Property.Value.PSObject.Properties) {
                $ImportedData[$Key][$LangProperty.Name] = $LangProperty.Value
            }
        }
    }
    default {
        Write-Host "Error: Unsupported file format: $Extension"
        Write-Host "Supported formats: .csv, .json"
        exit 1
    }
}

Write-Host "Loaded $($ImportedData.Count) translation entries"

# Load existing master table if merging
$MasterTablePath = Join-Path $LocalizationPath "MasterLocalizationTable.csv"
$ExistingData = @{}

if ($Merge -and (Test-Path $MasterTablePath)) {
    Write-Host "Loading existing translations for merge..."
    $ExistingTable = Import-Csv -Path $MasterTablePath -Encoding UTF8

    foreach ($Row in $ExistingTable) {
        $ExistingData[$Row.Key] = @{
            "en" = $Row.English
            "zh-Hans" = $Row.Chinese
            "ja" = $Row.Japanese
            "ko" = $Row.Korean
            "es" = $Row.Spanish
        }
    }

    Write-Host "Loaded $($ExistingData.Count) existing entries"
}

# Merge or replace
$FinalData = if ($Merge) {
    # Start with existing data
    $Merged = $ExistingData.Clone()

    # Add/update with imported data
    foreach ($Key in $ImportedData.Keys) {
        if (-not $Merged.ContainsKey($Key)) {
            $Merged[$Key] = @{}
        }

        foreach ($Lang in $ImportedData[$Key].Keys) {
            if ($ImportedData[$Key][$Lang]) {
                $Merged[$Key][$Lang] = $ImportedData[$Key][$Lang]
            }
        }
    }

    $Merged
} else {
    $ImportedData
}

Write-Host "Final dataset contains $($FinalData.Count) entries"

# Generate new master table
$NewMasterTable = @()
foreach ($Key in $FinalData.Keys | Sort-Object) {
    $Entry = $FinalData[$Key]
    $NewMasterTable += [PSCustomObject]@{
        Key = $Key
        English = if ($Entry["en"]) { $Entry["en"] } else { "" }
        Chinese = if ($Entry["zh-Hans"]) { $Entry["zh-Hans"] } else { "" }
        Japanese = if ($Entry["ja"]) { $Entry["ja"] } else { "" }
        Korean = if ($Entry["ko"]) { $Entry["ko"] } else { "" }
        Spanish = if ($Entry["es"]) { $Entry["es"] } else { "" }
    }
}

# Backup existing master table
if (Test-Path $MasterTablePath) {
    $BackupPath = $MasterTablePath -replace '\.csv$', "_backup_$(Get-Date -Format 'yyyyMMdd_HHmmss').csv"
    Copy-Item -Path $MasterTablePath -Destination $BackupPath
    Write-Host "Backed up existing table to: $BackupPath"
}

# Save new master table
$NewMasterTable | Export-Csv -Path $MasterTablePath -NoTypeInformation -Encoding UTF8
Write-Host "Saved master table: $MasterTablePath"

# Generate language-specific files
$Languages = @{
    "en" = "English"
    "zh-Hans" = "Chinese"
    "ja" = "Japanese"
    "ko" = "Korean"
    "es" = "Spanish"
}

foreach ($LangCode in $Languages.Keys) {
    $LangDir = Join-Path $LocalizationPath $LangCode
    if (-not (Test-Path $LangDir)) {
        New-Item -ItemType Directory -Path $LangDir -Force | Out-Null
    }

    # Generate JSON
    $LocalizationData = @{}
    foreach ($Entry in $NewMasterTable) {
        $Translation = switch ($LangCode) {
            "en" { $Entry.English }
            "zh-Hans" { $Entry.Chinese }
            "ja" { $Entry.Japanese }
            "ko" { $Entry.Korean }
            "es" { $Entry.Spanish }
        }

        if ($Translation) {
            $LocalizationData[$Entry.Key] = $Translation
        }
    }

    $JsonPath = Join-Path $LangDir "Game.json"
    $LocalizationData | ConvertTo-Json -Depth 10 | Out-File -FilePath $JsonPath -Encoding UTF8
    Write-Host "Generated: $JsonPath ($($LocalizationData.Count) entries)"
}

# Generate import summary
$Summary = @"
Translation Import Summary
==========================
Input File: $InputFile
Mode: $(if ($Merge) { "Merge" } else { "Replace" })
Imported Entries: $($ImportedData.Count)
Final Entries: $($FinalData.Count)
Output: $MasterTablePath

Language Files Generated:
"@

foreach ($LangCode in $Languages.Keys) {
    $JsonPath = Join-Path (Join-Path $LocalizationPath $LangCode) "Game.json"
    $Summary += "`n  - $JsonPath"
}

$Summary += @"

Next Steps:
1. Review the imported translations
2. Run ValidateTranslations.ps1 to check for issues
3. Test in-game with different languages
"@

Write-Host "`n$Summary"

# Save summary
$SummaryPath = Join-Path $LocalizationPath "ImportSummary_$(Get-Date -Format 'yyyyMMdd_HHmmss').txt"
$Summary | Out-File -FilePath $SummaryPath -Encoding UTF8
Write-Host "`nSummary saved to: $SummaryPath"
