# Validates that major gameplay systems have source coverage.

param(
    [string]$ReportPath = ""
)

$ErrorActionPreference = "Stop"
. (Join-Path $PSScriptRoot "QualityCommon.ps1")

$ProjectRoot = Get-HorrorProjectRoot
$Report = New-HorrorQualityReport -Name "ValidateAllSystems"

$Systems = @("AI", "Audio", "Evidence", "Game", "Interaction", "Network", "Performance", "Player", "Save", "Settings", "UI", "VFX")
foreach ($System in $Systems) {
    $SystemPath = Join-Path $ProjectRoot "Source\HorrorProject\$System"
    $Files = @(Get-ChildItem -Path $SystemPath -File -Recurse -Include *.h,*.cpp -ErrorAction SilentlyContinue)
    Add-HorrorQualityCheck -Report $Report -Name "System source: $System" -Passed ($Files.Count -gt 0) -Detail "$($Files.Count) source files" -Severity "Major"
}

$FailedCount = Write-HorrorQualitySummary -Report $Report
Save-HorrorQualityReport -Report $Report -ReportPath $ReportPath
exit $(if ($FailedCount -gt 0) { 1 } else { 0 })
