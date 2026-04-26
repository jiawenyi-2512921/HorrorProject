# Performs static stability checks before running expensive tests.

param(
    [string]$ReportPath = ""
)

$ErrorActionPreference = "Stop"
. (Join-Path $PSScriptRoot "QualityCommon.ps1")

$ProjectRoot = Get-HorrorProjectRoot
$Report = New-HorrorQualityReport -Name "StabilityValidator"

$SourceFiles = @(Get-ChildItem -Path (Join-Path $ProjectRoot "Source") -File -Recurse -Include *.h,*.cpp,*.cs)
$EmptySourceFiles = @($SourceFiles | Where-Object { $_.Length -eq 0 })
$TargetFiles = @("Source\HorrorProject.Target.cs", "Source\HorrorProjectEditor.Target.cs")

Add-HorrorQualityCheck -Report $Report -Name "Source files present" -Passed ($SourceFiles.Count -gt 0) -Detail "$($SourceFiles.Count) files" -Severity "Critical"
Add-HorrorQualityCheck -Report $Report -Name "No empty source files" -Passed ($EmptySourceFiles.Count -eq 0) -Detail "$($EmptySourceFiles.Count) empty files" -Severity "Critical"
foreach ($TargetFile in $TargetFiles) {
    Test-HorrorRequiredPath -Report $Report -Name "Target exists: $TargetFile" -Path (Join-Path $ProjectRoot $TargetFile)
}

$FailedCount = Write-HorrorQualitySummary -Report $Report
Save-HorrorQualityReport -Report $Report -ReportPath $ReportPath
exit $(if ($FailedCount -gt 0) { 1 } else { 0 })
