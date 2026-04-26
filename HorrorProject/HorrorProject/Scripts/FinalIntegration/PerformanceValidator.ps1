# Performs static performance-readiness checks.

param(
    [string]$ReportPath = ""
)

$ErrorActionPreference = "Stop"
. (Join-Path $PSScriptRoot "QualityCommon.ps1")

$ProjectRoot = Get-HorrorProjectRoot
$Report = New-HorrorQualityReport -Name "PerformanceValidator"

Test-HorrorRequiredPath -Report $Report -Name "DefaultScalability.ini exists" -Path (Join-Path $ProjectRoot "Config\DefaultScalability.ini") -Severity "Major"
Test-HorrorRequiredPath -Report $Report -Name "Performance source exists" -Path (Join-Path $ProjectRoot "Source\HorrorProject\Performance") -Severity "Major"
Test-HorrorRequiredPath -Report $Report -Name "Performance scripts exist" -Path (Join-Path $ProjectRoot "Scripts\Performance") -Severity "Minor"

$PerformanceSyntaxIssues = Get-HorrorPowerShellSyntaxIssueCount -ScanPath @("Scripts\Performance")
Add-HorrorQualityCheck -Report $Report -Name "Performance script syntax" -Passed ($PerformanceSyntaxIssues -eq 0) -Detail "$PerformanceSyntaxIssues syntax errors" -Severity "Major"

$FailedCount = Write-HorrorQualitySummary -Report $Report
Save-HorrorQualityReport -Report $Report -ReportPath $ReportPath
exit $(if ($FailedCount -gt 0) { 1 } else { 0 })
