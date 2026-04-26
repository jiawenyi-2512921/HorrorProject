# Checks core asset and content integrity before release.

param(
    [string]$ReportPath = ""
)

$ErrorActionPreference = "Stop"
. (Join-Path $PSScriptRoot "QualityCommon.ps1")

$ProjectRoot = Get-HorrorProjectRoot
$Report = New-HorrorQualityReport -Name "AssetIntegrity"

$ContentRoot = Join-Path $ProjectRoot "Content"
$ConfigRoot = Join-Path $ProjectRoot "Config"

Test-HorrorRequiredPath -Report $Report -Name "Content directory exists" -Path $ContentRoot
Test-HorrorRequiredPath -Report $Report -Name "Config directory exists" -Path $ConfigRoot

$Assets = @(Get-ChildItem -LiteralPath $ContentRoot -File -Recurse -Include *.uasset,*.umap -ErrorAction SilentlyContinue)
$ZeroByteAssets = @($Assets | Where-Object { $_.Length -le 0 })
$Maps = @($Assets | Where-Object { $_.Extension -eq ".umap" })

Add-HorrorQualityCheck -Report $Report -Name "Assets present" -Passed ($Assets.Count -gt 0) -Detail "$($Assets.Count) assets found" -Severity "Critical"
Add-HorrorQualityCheck -Report $Report -Name "No zero-byte assets" -Passed ($ZeroByteAssets.Count -eq 0) -Detail "$($ZeroByteAssets.Count) zero-byte assets" -Severity "Critical"
Add-HorrorQualityCheck -Report $Report -Name "Maps present" -Passed ($Maps.Count -gt 0) -Detail "$($Maps.Count) maps found" -Severity "Major"

$FailedCount = Write-HorrorQualitySummary -Report $Report
Save-HorrorQualityReport -Report $Report -ReportPath $ReportPath
exit $(if ($FailedCount -gt 0) { 1 } else { 0 })
