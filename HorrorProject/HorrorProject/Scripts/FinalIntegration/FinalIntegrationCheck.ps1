# Runs the fast final-integration checks that do not compile the project.

param(
    [string]$ReportPath = ""
)

$ErrorActionPreference = "Stop"
. (Join-Path $PSScriptRoot "QualityCommon.ps1")

$ProjectRoot = Get-HorrorProjectRoot
$Report = New-HorrorQualityReport -Name "FinalIntegrationCheck"

& (Join-Path $ProjectRoot "Scripts\Validation\CheckDependencies.ps1")
Add-HorrorQualityCheck -Report $Report -Name "Dependency check" -Passed ($LASTEXITCODE -eq 0) -Detail "ExitCode=$LASTEXITCODE" -Severity "Critical"

& (Join-Path $ProjectRoot "Scripts\Validation\CheckIncludes.ps1")
Add-HorrorQualityCheck -Report $Report -Name "Include check" -Passed ($LASTEXITCODE -eq 0) -Detail "ExitCode=$LASTEXITCODE" -Severity "Major"

$SyntaxIssues = Get-HorrorPowerShellSyntaxIssueCount -ScanPath @("Scripts\Validation", "Scripts\Build", "Scripts\Package", "Scripts\Test", "Scripts\Performance", "Scripts\FinalIntegration")
Add-HorrorQualityCheck -Report $Report -Name "Release script syntax" -Passed ($SyntaxIssues -eq 0) -Detail "$SyntaxIssues syntax errors" -Severity "Critical"

$FailedCount = Write-HorrorQualitySummary -Report $Report
Save-HorrorQualityReport -Report $Report -ReportPath $ReportPath
exit $(if ($FailedCount -gt 0) { 1 } else { 0 })
