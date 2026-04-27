# GenerateOptimizationReport.ps1
# Optimization Report Generator
# Creates comprehensive optimization status report

param(
    [string]$ProjectPath = "",
    [string]$OutputDir = ""
)

$ErrorActionPreference = "Stop"

. (Join-Path $PSScriptRoot "..\..\Validation\Common.ps1")
$ProjectRoot = Get-HorrorProjectRoot -StartPath $PSScriptRoot
if ([string]::IsNullOrWhiteSpace($ProjectPath)) { $ProjectPath = $ProjectRoot }
if ([string]::IsNullOrWhiteSpace($OutputDir)) { $OutputDir = Join-Path $ProjectRoot "Saved\Reports" }

Write-Host "=== Optimization Report Generator ===" -ForegroundColor Cyan
Write-Host ""

# Create output directory
New-Item -ItemType Directory -Force -Path $OutputDir | Out-Null

$timestamp = Get-Date -Format "yyyyMMdd_HHmmss"
$reportFile = Join-Path $OutputDir "optimization_report_$timestamp.html"

Write-Host "Gathering optimization data..." -ForegroundColor Yellow

# Scan for optimization results
$optimizationDir = Join-Path $ProjectPath "Saved\Optimization"
$validationDir = Join-Path $ProjectPath "Saved\Validation"

$textureOptFiles = Get-ChildItem -Path (Join-Path $optimizationDir "Textures") -Filter "texture_optimization_*.json" -ErrorAction SilentlyContinue | Sort-Object LastWriteTime -Descending
$materialOptFiles = Get-ChildItem -Path (Join-Path $optimizationDir "Materials") -Filter "material_optimization_*.json" -ErrorAction SilentlyContinue | Sort-Object LastWriteTime -Descending
$meshOptFiles = Get-ChildItem -Path (Join-Path $optimizationDir "Meshes") -Filter "mesh_optimization_*.json" -ErrorAction SilentlyContinue | Sort-Object LastWriteTime -Descending
$lightingOptFiles = Get-ChildItem -Path (Join-Path $optimizationDir "Lighting") -Filter "lighting_optimization_*.json" -ErrorAction SilentlyContinue | Sort-Object LastWriteTime -Descending
$validationFiles = Get-ChildItem -Path $validationDir -Filter "validation_report_*.json" -ErrorAction SilentlyContinue | Sort-Object LastWriteTime -Descending

$latestTexture = if ($textureOptFiles.Count -gt 0) { Get-Content $textureOptFiles[0].FullName | ConvertFrom-Json } else { $null }
$latestMaterial = if ($materialOptFiles.Count -gt 0) { Get-Content $materialOptFiles[0].FullName | ConvertFrom-Json } else { $null }
$latestMesh = if ($meshOptFiles.Count -gt 0) { Get-Content $meshOptFiles[0].FullName | ConvertFrom-Json } else { $null }
$latestLighting = if ($lightingOptFiles.Count -gt 0) { Get-Content $lightingOptFiles[0].FullName | ConvertFrom-Json } else { $null }
$latestValidation = if ($validationFiles.Count -gt 0) { Get-Content $validationFiles[0].FullName | ConvertFrom-Json } else { $null }

Write-Host "Generating HTML report..." -ForegroundColor Green

# Generate HTML
$html = @"
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>HorrorProject Optimization Report</title>
    <style>
        body {
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            margin: 0;
            padding: 20px;
            background: #1a1a1a;
            color: #e0e0e0;
        }
        .container {
            max-width: 1400px;
            margin: 0 auto;
            background: #2a2a2a;
            padding: 30px;
            border-radius: 10px;
            box-shadow: 0 4px 6px rgba(0,0,0,0.3);
        }
        h1 {
            color: #ff6b6b;
            border-bottom: 3px solid #ff6b6b;
            padding-bottom: 10px;
        }
        h2 {
            color: #4ecdc4;
            margin-top: 30px;
            border-left: 4px solid #4ecdc4;
            padding-left: 15px;
        }
        h3 {
            color: #95e1d3;
        }
        .section {
            margin: 30px 0;
            padding: 20px;
            background: #333;
            border-radius: 8px;
        }
        .metric-grid {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(200px, 1fr));
            gap: 20px;
            margin: 20px 0;
        }
        .metric-card {
            background: #3a3a3a;
            padding: 20px;
            border-radius: 8px;
            border-left: 4px solid #4ecdc4;
        }
        .metric-label {
            font-size: 0.9em;
            color: #999;
            display: block;
            margin-bottom: 8px;
        }
        .metric-value {
            font-size: 2em;
            font-weight: bold;
            color: #4ecdc4;
        }
        .status-good { color: #51cf66; }
        .status-warn { color: #ffd43b; }
        .status-bad { color: #ff6b6b; }
        table {
            width: 100%;
            border-collapse: collapse;
            margin: 20px 0;
            background: #3a3a3a;
        }
        th {
            background: #444;
            color: #4ecdc4;
            padding: 12px;
            text-align: left;
        }
        td {
            padding: 10px 12px;
            border-bottom: 1px solid #444;
        }
        tr:hover {
            background: #404040;
        }
        .progress-bar {
            width: 100%;
            height: 30px;
            background: #444;
            border-radius: 15px;
            overflow: hidden;
            margin: 10px 0;
        }
        .progress-fill {
            height: 100%;
            background: linear-gradient(90deg, #4ecdc4, #51cf66);
            display: flex;
            align-items: center;
            justify-content: center;
            color: white;
            font-weight: bold;
            transition: width 0.3s ease;
        }
        .recommendation {
            background: #3a3a3a;
            padding: 15px;
            margin: 10px 0;
            border-left: 4px solid #ffd43b;
            border-radius: 4px;
        }
        .timestamp {
            color: #999;
            font-size: 0.9em;
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>🎮 HorrorProject Optimization Report</h1>
        <p class="timestamp">Generated: $(Get-Date -Format "yyyy-MM-dd HH:mm:ss")</p>
"@

# Validation summary
if ($latestValidation) {
    $totalChecks = $latestValidation.Checks.Count
    $passedChecks = $latestValidation.PassedChecks
    $passRate = if ($totalChecks -gt 0) { ($passedChecks / $totalChecks) * 100 } else { 0 }

    $html += @"
        <div class="section">
            <h2>✅ Validation Status</h2>
            <div class="metric-grid">
                <div class="metric-card">
                    <span class="metric-label">Total Checks</span>
                    <span class="metric-value">$totalChecks</span>
                </div>
                <div class="metric-card">
                    <span class="metric-label">Passed</span>
                    <span class="metric-value status-good">$passedChecks</span>
                </div>
                <div class="metric-card">
                    <span class="metric-label">Warnings</span>
                    <span class="metric-value status-warn">$($latestValidation.Warnings)</span>
                </div>
                <div class="metric-card">
                    <span class="metric-label">Failed</span>
                    <span class="metric-value status-bad">$($latestValidation.FailedChecks)</span>
                </div>
            </div>
            <div class="progress-bar">
                <div class="progress-fill" style="width: $([math]::Round($passRate, 0))%">
                    $([math]::Round($passRate, 0))% Pass Rate
                </div>
            </div>
        </div>
"@
}

# Texture optimization
if ($latestTexture) {
    $memorySavingsMB = [math]::Round($latestTexture.EstimatedMemorySavings / 1MB, 2)
    $optimizedCount = $latestTexture.OptimizedTextures.Count

    $html += @"
        <div class="section">
            <h2>🖼️ Texture Optimization</h2>
            <div class="metric-grid">
                <div class="metric-card">
                    <span class="metric-label">Total Textures</span>
                    <span class="metric-value">$($latestTexture.TotalTextures)</span>
                </div>
                <div class="metric-card">
                    <span class="metric-label">Optimized</span>
                    <span class="metric-value status-warn">$optimizedCount</span>
                </div>
                <div class="metric-card">
                    <span class="metric-label">Memory Savings</span>
                    <span class="metric-value status-good">$memorySavingsMB MB</span>
                </div>
            </div>
"@

    if ($optimizedCount -gt 0) {
        $html += @"
            <h3>Top Optimization Opportunities</h3>
            <table>
                <tr>
                    <th>Texture</th>
                    <th>Type</th>
                    <th>Current Size</th>
                    <th>Target Size</th>
                    <th>Savings</th>
                </tr>
"@

        $topTextures = $latestTexture.OptimizedTextures | Sort-Object EstimatedSavings -Descending | Select-Object -First 10
        foreach ($tex in $topTextures) {
            $savingsMB = [math]::Round($tex.EstimatedSavings / 1MB, 2)
            $html += @"
                <tr>
                    <td>$($tex.Name)</td>
                    <td>$($tex.Type)</td>
                    <td>$($tex.CurrentResolution)</td>
                    <td>$($tex.TargetResolution)</td>
                    <td class="status-good">$savingsMB MB</td>
                </tr>
"@
        }

        $html += @"
            </table>
"@
    }

    $html += @"
        </div>
"@
}

# Material optimization
if ($latestMaterial) {
    $complexCount = $latestMaterial.ComplexMaterials.Count
    $criticalCount = ($latestMaterial.ComplexMaterials | Where-Object { $_.Severity -eq "Critical" }).Count

    $html += @"
        <div class="section">
            <h2>🎨 Material Optimization</h2>
            <div class="metric-grid">
                <div class="metric-card">
                    <span class="metric-label">Total Materials</span>
                    <span class="metric-value">$($latestMaterial.TotalMaterials)</span>
                </div>
                <div class="metric-card">
                    <span class="metric-label">Need Optimization</span>
                    <span class="metric-value status-warn">$complexCount</span>
                </div>
                <div class="metric-card">
                    <span class="metric-label">Critical Issues</span>
                    <span class="metric-value status-bad">$criticalCount</span>
                </div>
            </div>
"@

    if ($complexCount -gt 0) {
        $html += @"
            <h3>Most Complex Materials</h3>
            <table>
                <tr>
                    <th>Material</th>
                    <th>Shader Instructions</th>
                    <th>Texture Samples</th>
                    <th>Severity</th>
                </tr>
"@

        $topMaterials = $latestMaterial.ComplexMaterials | Sort-Object EstimatedInstructions -Descending | Select-Object -First 10
        foreach ($mat in $topMaterials) {
            $severityClass = if ($mat.Severity -eq "Critical") { "status-bad" } else { "status-warn" }
            $html += @"
                <tr>
                    <td>$($mat.Name)</td>
                    <td>~$($mat.EstimatedInstructions)</td>
                    <td>~$($mat.EstimatedSamples)</td>
                    <td class="$severityClass">$($mat.Severity)</td>
                </tr>
"@
        }

        $html += @"
            </table>
"@
    }

    $html += @"
        </div>
"@
}

# Mesh optimization
if ($latestMesh) {
    $complexMeshCount = $latestMesh.ComplexMeshes.Count
    $criticalMeshCount = ($latestMesh.ComplexMeshes | Where-Object { $_.Severity -eq "Critical" }).Count

    $html += @"
        <div class="section">
            <h2>🔷 Mesh Optimization</h2>
            <div class="metric-grid">
                <div class="metric-card">
                    <span class="metric-label">Total Meshes</span>
                    <span class="metric-value">$($latestMesh.TotalMeshes)</span>
                </div>
                <div class="metric-card">
                    <span class="metric-label">Need Optimization</span>
                    <span class="metric-value status-warn">$complexMeshCount</span>
                </div>
                <div class="metric-card">
                    <span class="metric-label">Critical Issues</span>
                    <span class="metric-value status-bad">$criticalMeshCount</span>
                </div>
            </div>
"@

    if ($complexMeshCount -gt 0) {
        $html += @"
            <h3>Most Complex Meshes</h3>
            <table>
                <tr>
                    <th>Mesh</th>
                    <th>Estimated Triangles</th>
                    <th>File Size</th>
                    <th>Severity</th>
                </tr>
"@

        $topMeshes = $latestMesh.ComplexMeshes | Sort-Object EstimatedTriangles -Descending | Select-Object -First 10
        foreach ($mesh in $topMeshes) {
            $severityClass = if ($mesh.Severity -eq "Critical") { "status-bad" } else { "status-warn" }
            $fileSizeKB = [math]::Round($mesh.FileSize / 1KB, 2)
            $html += @"
                <tr>
                    <td>$($mesh.Name)</td>
                    <td>~$($mesh.EstimatedTriangles)</td>
                    <td>$fileSizeKB KB</td>
                    <td class="$severityClass">$($mesh.Severity)</td>
                </tr>
"@
        }

        $html += @"
            </table>
"@
    }

    $html += @"
        </div>
"@
}

# Recommendations
$html += @"
        <div class="section">
            <h2>💡 Next Steps</h2>
            <h3>Priority Actions</h3>
"@

$recommendations = @()

if ($latestValidation -and $latestValidation.FailedChecks -gt 0) {
    $recommendations += "Address $($latestValidation.FailedChecks) failed validation checks"
}

if ($latestTexture -and $latestTexture.OptimizedTextures.Count -gt 0) {
    $recommendations += "Apply texture optimizations to save $([math]::Round($latestTexture.EstimatedMemorySavings / 1MB, 0)) MB"
}

if ($latestMaterial) {
    $criticalMats = ($latestMaterial.ComplexMaterials | Where-Object { $_.Severity -eq "Critical" }).Count
    if ($criticalMats -gt 0) {
        $recommendations += "Optimize $criticalMats critical materials"
    }
}

if ($latestMesh) {
    $criticalMeshes = ($latestMesh.ComplexMeshes | Where-Object { $_.Severity -eq "Critical" }).Count
    if ($criticalMeshes -gt 0) {
        $recommendations += "Generate LODs for $criticalMeshes complex meshes"
    }
}

$recommendations += "Run performance benchmarks to measure improvements"
$recommendations += "Compare before/after results using ComparePerformance.ps1"

foreach ($rec in $recommendations) {
    $html += @"
            <div class="recommendation">$rec</div>
"@
}

$html += @"
        </div>

        <div class="section">
            <h2>📋 Optimization Workflow</h2>
            <ol style="line-height: 2;">
                <li>Run validation: <code>.\ValidateOptimizations.ps1</code></li>
                <li>Benchmark baseline: <code>.\BenchmarkFramerate.ps1</code></li>
                <li>Apply optimizations:
                    <ul>
                        <li><code>.\OptimizeTextures.ps1</code></li>
                        <li><code>.\OptimizeMaterials.ps1</code></li>
                        <li><code>.\OptimizeMeshes.ps1</code></li>
                        <li><code>.\OptimizeLighting.ps1</code></li>
                    </ul>
                </li>
                <li>Benchmark after: <code>.\BenchmarkFramerate.ps1</code></li>
                <li>Compare results: <code>.\ComparePerformance.ps1</code></li>
                <li>Generate report: <code>.\GenerateOptimizationReport.ps1</code></li>
            </ol>
        </div>
    </div>
</body>
</html>
"@

# Save report
$html | Out-File $reportFile -Encoding UTF8

Write-Host ""
Write-Host "Optimization report generated successfully!" -ForegroundColor Green
Write-Host "Location: $reportFile" -ForegroundColor Cyan
Write-Host ""

# Open in browser
Start-Process $reportFile

Write-Host "Report opened in default browser" -ForegroundColor Green
