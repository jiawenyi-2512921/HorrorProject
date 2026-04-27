# Extract Localizable Text from Source Files
# Usage: .\ExtractLocalizableText.ps1

param(
    [string]$SourcePath = "",
    [string]$OutputPath = ""
)

$ErrorActionPreference = "Stop"

. (Join-Path $PSScriptRoot "..\..\..\Scripts\Validation\Common.ps1")
$ProjectRoot = Get-HorrorProjectRoot -StartPath $PSScriptRoot
if ([string]::IsNullOrWhiteSpace($SourcePath)) { $SourcePath = Join-Path $ProjectRoot "Source" }
if ([string]::IsNullOrWhiteSpace($OutputPath)) { $OutputPath = Join-Path $ProjectRoot "Content\Localization\ExtractedTexts.csv" }

Write-Host "Extracting localizable text from: $SourcePath"

$ExtractedTexts = @()
$TextPatterns = @(
    'FText::FromString\(TEXT\("([^"]+)"\)\)',
    'LOCTEXT\("([^"]+)",\s*"([^"]+)"\)',
    'NSLOCTEXT\("([^"]+)",\s*"([^"]+)",\s*"([^"]+)"\)',
    'GetLocalizedText\("([^"]+)"\)'
)

function Extract-TextFromFile {
    param([string]$FilePath)

    $Content = Get-Content -LiteralPath $FilePath -Raw -ErrorAction SilentlyContinue
    if (-not $Content) { return }

    $FileTexts = @()
    foreach ($Pattern in $TextPatterns) {
        $Matches = [regex]::Matches($Content, $Pattern)
        foreach ($Match in $Matches) {
            $Text = if ($Match.Groups.Count -gt 2) { $Match.Groups[2].Value } else { $Match.Groups[1].Value }

            if ($Text -and $Text.Length -gt 0) {
                $FileTexts += [PSCustomObject]@{
                    File = $FilePath
                    Key = $Text
                    Text = $Text
                    Context = $Match.Value
                }
            }
        }
    }

    return $FileTexts
}

# Find all C++ source files
$SourceFiles = @(Get-ChildItem -LiteralPath $SourcePath -Recurse -File -Include *.cpp,*.h)

Write-Host "Found $($SourceFiles.Count) source files"

foreach ($File in $SourceFiles) {
    Write-Host "Processing: $($File.Name)"
    $ExtractedTexts += Extract-TextFromFile -FilePath $File.FullName
}

# Remove duplicates
$UniqueTexts = @($ExtractedTexts | Sort-Object -Property Key -Unique)

Write-Host "Extracted $($UniqueTexts.Count) unique text entries"

# Create output directory if it doesn't exist
$OutputDir = Split-Path $OutputPath -Parent
if (-not (Test-Path $OutputDir)) {
    New-Item -ItemType Directory -Path $OutputDir -Force | Out-Null
}

# Export to CSV
$UniqueTexts | Export-Csv -Path $OutputPath -NoTypeInformation -Encoding UTF8

Write-Host "Exported to: $OutputPath"

# Generate summary
$Summary = @"
Localization Text Extraction Summary
=====================================
Total Files Processed: $($SourceFiles.Count)
Unique Text Entries: $($UniqueTexts.Count)
Output File: $OutputPath

Next Steps:
1. Review the extracted texts in $OutputPath
2. Add missing localization keys
3. Run GenerateLocalizationFiles.ps1 to create language files
"@

Write-Host $Summary

# Save summary
$SummaryPath = Join-Path $OutputDir "ExtractionSummary.txt"
$Summary | Out-File -FilePath $SummaryPath -Encoding UTF8
