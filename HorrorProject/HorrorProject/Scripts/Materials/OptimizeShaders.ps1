# Generates shader optimization recommendations from a material analysis JSON file.

param(
    [string]$AnalysisPath = "",
    [string]$OutputPath = ""
)

$ErrorActionPreference = "Stop"

$ProjectRoot = Split-Path -Parent (Split-Path -Parent $PSScriptRoot)
if ([string]::IsNullOrWhiteSpace($AnalysisPath)) {
    $AnalysisPath = Join-Path $ProjectRoot "Scripts\Materials\ShaderAnalysis.json"
}
if ([string]::IsNullOrWhiteSpace($OutputPath)) {
    $OutputPath = Join-Path $ProjectRoot "Scripts\Materials\OptimizationReport.md"
}

Write-Host "=== Shader Optimization Report Generator ===" -ForegroundColor Cyan

if (Test-Path -LiteralPath $AnalysisPath) {
    $Analysis = Get-Content -LiteralPath $AnalysisPath -Raw | ConvertFrom-Json
} else {
    Write-Host "Analysis file not found. Creating a baseline recommendation report." -ForegroundColor Yellow
    $Analysis = [PSCustomObject]@{
        Summary = [PSCustomObject]@{
            TotalMaterials = 0
            PostProcessMaterials = 0
        }
        ComplexityDistribution = [PSCustomObject]@{
            VeryHigh = 0
            High = 0
        }
        CategoryDistribution = [PSCustomObject]@{
            Translucent = 0
        }
    }
}

$NeedsOptimization = [int]$Analysis.ComplexityDistribution.VeryHigh + [int]$Analysis.ComplexityDistribution.High
$Report = @"
# Shader Optimization Report

Generated: $(Get-Date -Format 'yyyy-MM-dd HH:mm:ss')

## Summary

- Total materials: $($Analysis.Summary.TotalMaterials)
- High-priority materials: $NeedsOptimization
- Post-process materials: $($Analysis.Summary.PostProcessMaterials)
- Translucent materials: $($Analysis.CategoryDistribution.Translucent)

## Targets

- Keep expensive master materials below 300 instructions where possible.
- Keep texture samples below 16 for Epic quality and lower for scalable tiers.
- Prefer material instances over duplicated master materials.
- Use masked materials instead of translucent materials where the art direction allows it.

## Recommendations

1. Build a small master-material family for opaque, masked, translucent, and water surfaces.
2. Move repeated graph logic into material functions.
3. Add static switches for quality levels and feature toggles.
4. Audit post-process materials first because they affect the full frame.
5. Profile with Shader Complexity and GPU captures after each optimization pass.
"@

$OutputDir = Split-Path -Parent $OutputPath
if ($OutputDir) {
    New-Item -ItemType Directory -Force -Path $OutputDir | Out-Null
}

Set-Content -Path $OutputPath -Value $Report -Encoding UTF8
Write-Host "Report generated: $OutputPath" -ForegroundColor Green
