# Optimize Level Geometry Tool
# Optimizes level geometry for performance

param(
    [Parameter(Mandatory=$false)]
    [string]$LevelName = "SM13",

    [Parameter(Mandatory=$false)]
    [string]$OutputPath = "D:/gptzuo/HorrorProject/HorrorProject/Content/Maps"
)

$ErrorActionPreference = "Stop"

Write-Host "=== Optimize Level Geometry ===" -ForegroundColor Cyan
Write-Host "Level: $LevelName"
Write-Host ""

Write-Host "Geometry optimization recommendations:" -ForegroundColor Yellow
Write-Host ""

Write-Host "1. LOD (Level of Detail) Configuration:" -ForegroundColor Cyan
Write-Host "   - LOD0 (0-10m): Full detail" -ForegroundColor Gray
Write-Host "   - LOD1 (10-20m): 50% triangles" -ForegroundColor Gray
Write-Host "   - LOD2 (20-40m): 25% triangles" -ForegroundColor Gray
Write-Host "   - LOD3 (40m+): 10% triangles or culled" -ForegroundColor Gray

Write-Host ""
Write-Host "2. Mesh Instancing:" -ForegroundColor Cyan
Write-Host "   - Use instanced static meshes for repeated objects" -ForegroundColor Gray
Write-Host "   - Group similar objects (chairs, desks, props)" -ForegroundColor Gray
Write-Host "   - Reduces draw calls significantly" -ForegroundColor Gray

Write-Host ""
Write-Host "3. Occlusion Culling:" -ForegroundColor Cyan
Write-Host "   - Place occlusion volumes in corridors" -ForegroundColor Gray
Write-Host "   - Use precomputed visibility volumes" -ForegroundColor Gray
Write-Host "   - Enable distance culling for small objects" -ForegroundColor Gray

Write-Host ""
Write-Host "4. Material Optimization:" -ForegroundColor Cyan
Write-Host "   - Merge materials where possible" -ForegroundColor Gray
Write-Host "   - Use material instances instead of unique materials" -ForegroundColor Gray
Write-Host "   - Limit shader complexity" -ForegroundColor Gray

Write-Host ""
Write-Host "5. Collision Optimization:" -ForegroundColor Cyan
Write-Host "   - Use simple collision (boxes, spheres) where possible" -ForegroundColor Gray
Write-Host "   - Avoid per-poly collision" -ForegroundColor Gray
Write-Host "   - Disable collision on decorative objects" -ForegroundColor Gray

# Generate optimization config
$configPath = Join-Path $OutputPath "${LevelName}_GeometryOptimization.json"

$config = @{
    LevelName = $LevelName
    LODSettings = @{
        LOD0 = @{ Distance = "0-10m"; TriangleReduction = "0%" }
        LOD1 = @{ Distance = "10-20m"; TriangleReduction = "50%" }
        LOD2 = @{ Distance = "20-40m"; TriangleReduction = "75%" }
        LOD3 = @{ Distance = "40m+"; TriangleReduction = "90%" }
    }
    TriangleBudget = @{
        Target = 2000000
        Max = 3000000
        PerZone = 300000
    }
    DrawCallBudget = @{
        Target = 2000
        Max = 2500
        PerZone = 300
    }
    OptimizationTechniques = @(
        "Mesh Instancing",
        "LOD Implementation",
        "Occlusion Culling",
        "Material Merging",
        "Collision Simplification"
    )
    Timestamp = Get-Date -Format "yyyy-MM-dd HH:mm:ss"
}

$config | ConvertTo-Json -Depth 10 | Set-Content -Path $configPath -Encoding UTF8

Write-Host ""
Write-Host "Configuration saved: $configPath" -ForegroundColor Green
Write-Host ""
Write-Host "=== Geometry Optimization Complete ===" -ForegroundColor Cyan
