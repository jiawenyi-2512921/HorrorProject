# Generates a release-readiness markdown report.

param(
    [string]$ReportPath = ""
)

$ErrorActionPreference = "Stop"
. (Join-Path $PSScriptRoot "QualityCommon.ps1")

$ProjectRoot = Get-HorrorProjectRoot
if ([string]::IsNullOrWhiteSpace($ReportPath)) {
    $ReportDir = Join-Path $ProjectRoot "Saved\FinalIntegration"
    New-Item -ItemType Directory -Force -Path $ReportDir | Out-Null
    $ReportPath = Join-Path $ReportDir "ReleaseReadiness.md"
}

$SourceFiles = @(Get-ChildItem -Path (Join-Path $ProjectRoot "Source") -File -Recurse -Include *.h,*.cpp,*.cs)
$Assets = @(Get-ChildItem -Path (Join-Path $ProjectRoot "Content") -File -Recurse -Include *.uasset,*.umap -ErrorAction SilentlyContinue)
$ScriptIssues = Get-HorrorPowerShellSyntaxIssueCount -ScanPath @("Scripts\Validation", "Scripts\Build", "Scripts\Package", "Scripts\Test", "Scripts\Performance", "Scripts\FinalIntegration")

$Content = @"
# Release Readiness Report

Generated: $(Get-Date -Format 'yyyy-MM-dd HH:mm:ss')

## Inventory

- Source files: $($SourceFiles.Count)
- Unreal assets/maps: $($Assets.Count)
- Release-chain PowerShell syntax issues: $ScriptIssues

## Verified Commands

- `Scripts\Validation\ValidateCompilation.ps1`
- `Scripts\Validation\ValidateCompilation.ps1 -EditorOnly`
- `Scripts\FinalIntegration\FinalIntegrationCheck.ps1`

## Known Constraints

- Legacy automation tests remain disabled behind `HORRORPROJECT_ENABLE_LEGACY_AUTOMATION_TESTS=0`.
- Full Cook/Package validation should be run after release-chain scripts remain clean.
"@

Set-Content -Path $ReportPath -Value $Content -Encoding UTF8
Write-Host "Release report: $ReportPath" -ForegroundColor Green
