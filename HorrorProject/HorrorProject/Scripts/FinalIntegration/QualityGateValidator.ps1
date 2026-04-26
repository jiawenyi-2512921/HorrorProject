# Aggregates fast quality gates for release readiness.

param(
    [string]$ReportPath = "",
    [switch]$StrictMode
)

$ErrorActionPreference = "Stop"
. (Join-Path $PSScriptRoot "QualityCommon.ps1")

$ProjectRoot = Get-HorrorProjectRoot
$Report = New-HorrorQualityReport -Name "QualityGateValidator"

$CriticalScripts = @("Scripts\Validation", "Scripts\Build", "Scripts\Package", "Scripts\Test", "Scripts\Performance", "Scripts\FinalIntegration")
$CriticalSyntaxIssues = Get-HorrorPowerShellSyntaxIssueCount -ScanPath $CriticalScripts
Add-HorrorQualityCheck -Report $Report -Name "Critical script syntax" -Passed ($CriticalSyntaxIssues -eq 0) -Detail "$CriticalSyntaxIssues syntax errors" -Severity "Critical"

$RuntimeBuild = Get-Content -LiteralPath (Join-Path $ProjectRoot "Source\HorrorProject\HorrorProject.Build.cs") -Raw
Add-HorrorQualityCheck -Report $Report -Name "Legacy tests intentionally gated" -Passed ($RuntimeBuild -match "HORRORPROJECT_ENABLE_LEGACY_AUTOMATION_TESTS=0") -Detail "Legacy tests are quarantined until migrated" -Severity "Major"

$RequiredDirs = @("Source\HorrorProject", "Source\HorrorProjectEditor", "Content", "Config", "Scripts\Validation")
foreach ($Dir in $RequiredDirs) {
    Test-HorrorRequiredPath -Report $Report -Name "Required path: $Dir" -Path (Join-Path $ProjectRoot $Dir) -Severity "Critical"
}

if ($StrictMode) {
    $AllScriptIssues = Get-HorrorPowerShellSyntaxIssueCount -ScanPath @("Scripts", "Source", "Docs")
    Add-HorrorQualityCheck -Report $Report -Name "Strict all-script syntax" -Passed ($AllScriptIssues -eq 0) -Detail "$AllScriptIssues syntax errors" -Severity "Major"
}

$FailedCount = Write-HorrorQualitySummary -Report $Report
Save-HorrorQualityReport -Report $Report -ReportPath $ReportPath
exit $(if ($FailedCount -gt 0) { 1 } else { 0 })
