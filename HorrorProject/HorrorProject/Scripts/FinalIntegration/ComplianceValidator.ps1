# Performs static compliance checks for project structure and configuration.

param(
    [string]$ReportPath = ""
)

$ErrorActionPreference = "Stop"
. (Join-Path $PSScriptRoot "QualityCommon.ps1")

$ProjectRoot = Get-HorrorProjectRoot
$ProjectFile = Get-HorrorProjectFile -ProjectRoot $ProjectRoot
$Report = New-HorrorQualityReport -Name "ComplianceValidator"

$ProjectJson = Get-Content -LiteralPath $ProjectFile -Raw | ConvertFrom-Json
$RuntimeBuild = Join-Path $ProjectRoot "Source\HorrorProject\HorrorProject.Build.cs"
$EditorBuild = Join-Path $ProjectRoot "Source\HorrorProjectEditor\HorrorProjectEditor.Build.cs"
$DefaultEngine = Join-Path $ProjectRoot "Config\DefaultEngine.ini"

Add-HorrorQualityCheck -Report $Report -Name "uproject JSON is readable" -Passed ($null -ne $ProjectJson) -Detail $ProjectFile -Severity "Critical"
Add-HorrorQualityCheck -Report $Report -Name "Engine association is UE 5.6" -Passed ($ProjectJson.EngineAssociation -eq "5.6") -Detail "EngineAssociation=$($ProjectJson.EngineAssociation)" -Severity "Major"
Test-HorrorRequiredPath -Report $Report -Name "Runtime Build.cs exists" -Path $RuntimeBuild
Test-HorrorRequiredPath -Report $Report -Name "Editor Build.cs exists" -Path $EditorBuild
Test-HorrorRequiredPath -Report $Report -Name "DefaultEngine.ini exists" -Path $DefaultEngine

$RuntimeBuildContent = Get-Content -LiteralPath $RuntimeBuild -Raw
foreach ($Module in @("Niagara", "UMG", "OnlineSubsystem", "NavigationSystem", "JsonUtilities")) {
    Add-HorrorQualityCheck -Report $Report -Name "Runtime module declared: $Module" -Passed ($RuntimeBuildContent -match [regex]::Escape($Module)) -Detail $Module -Severity "Major"
}

$FailedCount = Write-HorrorQualitySummary -Report $Report
Save-HorrorQualityReport -Report $Report -ReportPath $ReportPath
exit $(if ($FailedCount -gt 0) { 1 } else { 0 })
