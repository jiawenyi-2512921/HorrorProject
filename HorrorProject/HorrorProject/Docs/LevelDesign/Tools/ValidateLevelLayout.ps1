# Static level-layout documentation validator.

param(
    [string]$OutputPath = ""
)

$ErrorActionPreference = "Stop"
$ProjectRoot = Split-Path -Parent (Split-Path -Parent (Split-Path -Parent $PSScriptRoot))
if ([string]::IsNullOrWhiteSpace($OutputPath)) {
    $OutputPath = Join-Path $ProjectRoot "Saved\LevelDesign\LayoutValidation.md"
}

$ContentRoot = Join-Path $ProjectRoot "Content"
$Maps = @(Get-ChildItem -Path $ContentRoot -File -Recurse -Include *.umap -ErrorAction SilentlyContinue)
$Blueprints = @(Get-ChildItem -Path $ContentRoot -File -Recurse -Include *.uasset -ErrorAction SilentlyContinue | Where-Object { $_.Name -like "BP_*" })
$Passed = $Maps.Count -gt 0 -and $Blueprints.Count -gt 0

$Report = @"
# Level Layout Validation

Generated: $(Get-Date -Format 'yyyy-MM-dd HH:mm:ss')

## Static Checks

- Maps found: $($Maps.Count)
- Blueprint assets found: $($Blueprints.Count)

## Status

$(if ($Passed) { "PASS: Layout inputs are present." } else { "FAIL: Missing maps or blueprint assets." })
"@

New-Item -ItemType Directory -Force -Path (Split-Path -Parent $OutputPath) | Out-Null
Set-Content -Path $OutputPath -Value $Report -Encoding UTF8
Write-Host "Layout validation report: $OutputPath" -ForegroundColor Green
exit $(if ($Passed) { 0 } else { 1 })
