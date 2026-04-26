# Static level-performance documentation test.

param(
    [string]$OutputPath = ""
)

$ErrorActionPreference = "Stop"
$ProjectRoot = Split-Path -Parent (Split-Path -Parent (Split-Path -Parent $PSScriptRoot))
if ([string]::IsNullOrWhiteSpace($OutputPath)) {
    $OutputPath = Join-Path $ProjectRoot "Saved\LevelDesign\PerformanceReport.md"
}

$ContentRoot = Join-Path $ProjectRoot "Content"
$Maps = @(Get-ChildItem -Path $ContentRoot -File -Recurse -Include *.umap -ErrorAction SilentlyContinue)
$LargeAssets = @(Get-ChildItem -Path $ContentRoot -File -Recurse -Include *.uasset,*.umap -ErrorAction SilentlyContinue | Where-Object { $_.Length -gt 100MB })

$Passed = $Maps.Count -gt 0
$Report = @"
# Level Performance Static Report

Generated: $(Get-Date -Format 'yyyy-MM-dd HH:mm:ss')

## Inventory

- Maps: $($Maps.Count)
- Assets larger than 100 MB: $($LargeAssets.Count)

## Recommendations

- Run an in-editor GPU profile on each shipping map.
- Keep light complexity and shader complexity views clean before release.
- Use cooked build profiling for final performance decisions.
"@

New-Item -ItemType Directory -Force -Path (Split-Path -Parent $OutputPath) | Out-Null
Set-Content -Path $OutputPath -Value $Report -Encoding UTF8
Write-Host "Level performance report: $OutputPath" -ForegroundColor Green
exit $(if ($Passed) { 0 } else { 1 })
