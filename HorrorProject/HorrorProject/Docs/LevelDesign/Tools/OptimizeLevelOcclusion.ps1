# Optimize Level Occlusion Tool
# Optimizes occlusion culling for performance

param(
    [Parameter(Mandatory=$false)]
    [string]$LevelName = "SM13",

    [Parameter(Mandatory=$false)]
    [string]$OutputPath = "D:/gptzuo/HorrorProject/HorrorProject/Content/Maps"
)

$ErrorActionPreference = "Stop"

Write-Host "=== Optimize Level Occlusion ===" -ForegroundColor Cyan
Write-Host "Level: $LevelName"
Write-Host ""

Write-Host "Occlusion optimization recommendations:" -ForegroundColor Yellow
Write-Host ""

Write-Host "1. Precomputed Visibility Volumes:" -ForegroundColor Cyan
Write-Host "   - Place volumes in each major zone" -ForegroundColor Gray
Write-Host "   - Cover entire playable area" -ForegroundColor Gray
Write-Host "   - Build visibility data in production builds" -ForegroundColor Gray

Write-Host ""
Write-Host "2. Cull Distance Volumes:" -ForegroundColor Cyan
Write-Host "   - Small props: 20m cull distance" -ForegroundColor Gray
Write-Host "   - Medium objects: 40m cull distance" -ForegroundColor Gray
Write-Host "   - Large structures: 80m cull distance" -ForegroundColor Gray

Write-Host ""
Write-Host "3. Hierarchical LOD (HLOD):" -ForegroundColor Cyan
Write-Host "   - Group static meshes per zone" -ForegroundColor Gray
Write-Host "   - Generate proxy meshes for distant views" -ForegroundColor Gray
Write-Host "   - Reduces draw calls at distance" -ForegroundColor Gray

Write-Host ""
Write-Host "4. Portal Occlusion:" -ForegroundColor Cyan
Write-Host "   - Use doorways as natural occlusion portals" -ForegroundColor Gray
Write-Host "   - Close doors to occlude entire rooms" -ForegroundColor Gray
Write-Host "   - Manually place occlusion planes in corridors" -ForegroundColor Gray

# Generate occlusion config
$configPath = Join-Path $OutputPath "${LevelName}_OcclusionOptimization.json"

$config = @{
    LevelName = $LevelName
    PrecomputedVisibility = @{
        Enabled = $true
        CellSize = 100
        NumCells = 130  # Approximate for 1300m²
    }
    CullDistances = @{
        SmallProps = 2000  # cm
        MediumObjects = 4000
        LargeStructures = 8000
        Particles = 3000
        Lights = 5000
    }
    HLODSettings = @{
        Enabled = $true
        LODLevels = 3
        TransitionScreenSize = 0.3
    }
    OcclusionVolumes = @(
        @{ Zone = "MainCorridor"; Position = @{X=1500;Y=0;Z=175}; Size = @{X=4000;Y=500;Z=350} },
        @{ Zone = "WestWing"; Position = @{X=800;Y=800;Z=150}; Size = @{X=2500;Y=1000;Z=300} },
        @{ Zone = "EastWing"; Position = @{X=1800;Y=-800;Z=150}; Size = @{X=2500;Y=1000;Z=300} },
        @{ Zone = "Basement"; Position = @{X=1000;Y=0;Z=-125}; Size = @{X=2000;Y=1000;Z=250} }
    )
    Timestamp = Get-Date -Format "yyyy-MM-dd HH:mm:ss"
}

$config | ConvertTo-Json -Depth 10 | Set-Content -Path $configPath -Encoding UTF8

Write-Host ""
Write-Host "Configuration saved: $configPath" -ForegroundColor Green
Write-Host ""
Write-Host "=== Occlusion Optimization Complete ===" -ForegroundColor Cyan
