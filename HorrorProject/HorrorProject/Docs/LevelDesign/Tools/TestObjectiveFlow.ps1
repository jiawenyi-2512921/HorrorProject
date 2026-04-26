# Static objective-flow documentation test.

param(
    [string]$OutputPath = ""
)

$ErrorActionPreference = "Stop"
$ProjectRoot = Split-Path -Parent (Split-Path -Parent (Split-Path -Parent $PSScriptRoot))
if ([string]::IsNullOrWhiteSpace($OutputPath)) {
    $OutputPath = Join-Path $ProjectRoot "Saved\LevelDesign\ObjectiveFlowReport.md"
}

$SourceRoot = Join-Path $ProjectRoot "Source\HorrorProject"
$ObjectiveFiles = @(Get-ChildItem -Path $SourceRoot -File -Recurse -Include *.h,*.cpp | Select-String -Pattern "Objective" | Select-Object -ExpandProperty Path -Unique)

$Report = @"
# Objective Flow Report

Generated: $(Get-Date -Format 'yyyy-MM-dd HH:mm:ss')

## Static Checks

- Objective-related source files: $($ObjectiveFiles.Count)
- Source root: $SourceRoot

## Status

$(if ($ObjectiveFiles.Count -gt 0) { "PASS: Objective code paths are present." } else { "FAIL: No objective code paths found." })
"@

New-Item -ItemType Directory -Force -Path (Split-Path -Parent $OutputPath) | Out-Null
Set-Content -Path $OutputPath -Value $Report -Encoding UTF8
Write-Host "Objective flow report: $OutputPath" -ForegroundColor Green
exit $(if ($ObjectiveFiles.Count -gt 0) { 0 } else { 1 })
