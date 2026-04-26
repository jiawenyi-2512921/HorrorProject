# Verifies Unreal build configuration files.

param(
    [string]$ReportPath = ""
)

$ErrorActionPreference = "Stop"
. (Join-Path $PSScriptRoot "QualityCommon.ps1")

$ProjectRoot = Get-HorrorProjectRoot
$Report = New-HorrorQualityReport -Name "VerifyBuildConfigs"

$Files = @(
    "HorrorProject.uproject",
    "Source\HorrorProject.Target.cs",
    "Source\HorrorProjectEditor.Target.cs",
    "Source\HorrorProject\HorrorProject.Build.cs",
    "Source\HorrorProjectEditor\HorrorProjectEditor.Build.cs",
    "Config\DefaultEngine.ini",
    "Config\DefaultGame.ini"
)

foreach ($File in $Files) {
    Test-HorrorRequiredPath -Report $Report -Name "Build config exists: $File" -Path (Join-Path $ProjectRoot $File)
}

$GameTarget = Get-Content -LiteralPath (Join-Path $ProjectRoot "Source\HorrorProject.Target.cs") -Raw
$EditorTarget = Get-Content -LiteralPath (Join-Path $ProjectRoot "Source\HorrorProjectEditor.Target.cs") -Raw
Add-HorrorQualityCheck -Report $Report -Name "Game target uses UE 5.6 include order" -Passed ($GameTarget -match "Unreal5_6") -Detail "HorrorProject.Target.cs" -Severity "Major"
Add-HorrorQualityCheck -Report $Report -Name "Editor target uses UE 5.6 include order" -Passed ($EditorTarget -match "Unreal5_6") -Detail "HorrorProjectEditor.Target.cs" -Severity "Major"

$FailedCount = Write-HorrorQualitySummary -Report $Report
Save-HorrorQualityReport -Report $Report -ReportPath $ReportPath
exit $(if ($FailedCount -gt 0) { 1 } else { 0 })
