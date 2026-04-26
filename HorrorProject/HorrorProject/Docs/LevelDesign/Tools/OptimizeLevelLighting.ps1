# Optimize Level Lighting Tool
# Optimizes lighting setup for performance

param(
    [Parameter(Mandatory=$false)]
    [string]$LevelName = "SM13",

    [Parameter(Mandatory=$false)]
    [ValidateSet("Preview", "Medium", "High", "Production")]
    [string]$BuildQuality = "Production",

    [Parameter(Mandatory=$false)]
    [string]$OutputPath = "D:/gptzuo/HorrorProject/HorrorProject/Content/Maps"
)

$ErrorActionPreference = "Stop"

Write-Host "=== Optimize Level Lighting ===" -ForegroundColor Cyan
Write-Host "Level: $LevelName"
Write-Host "Build Quality: $BuildQuality"
Write-Host ""

# Lightmass settings based on quality
$lightmassSettings = @{
    Preview = @{
        StaticLightingLevelScale = 1.0
        NumIndirectLightingBounces = 1
        NumSkyLightingBounces = 1
        IndirectLightingQuality = 1
        IndirectLightingSmoothness = 0.8
        BuildTime = "2-5 minutes"
    }
    Medium = @{
        StaticLightingLevelScale = 0.7
        NumIndirectLightingBounces = 2
        NumSkyLightingBounces = 1
        IndirectLightingQuality = 2
        IndirectLightingSmoothness = 0.9
        BuildTime = "10-20 minutes"
    }
    High = @{
        StaticLightingLevelScale = 0.6
        NumIndirectLightingBounces = 3
        NumSkyLightingBounces = 1
        IndirectLightingQuality = 3
        IndirectLightingSmoothness = 1.0
        BuildTime = "30-60 minutes"
    }
    Production = @{
        StaticLightingLevelScale = 0.5
        NumIndirectLightingBounces = 3
        NumSkyLightingBounces = 1
        IndirectLightingQuality = 4
        IndirectLightingSmoothness = 1.0
        BuildTime = "1-3 hours"
    }
}

$settings = $lightmassSettings[$BuildQuality]

Write-Host "Lightmass Settings:" -ForegroundColor Yellow
Write-Host "  - Static Lighting Level Scale: $($settings.StaticLightingLevelScale)" -ForegroundColor Gray
Write-Host "  - Indirect Lighting Bounces: $($settings.NumIndirectLightingBounces)" -ForegroundColor Gray
Write-Host "  - Sky Lighting Bounces: $($settings.NumSkyLightingBounces)" -ForegroundColor Gray
Write-Host "  - Indirect Lighting Quality: $($settings.IndirectLightingQuality)" -ForegroundColor Gray
Write-Host "  - Indirect Lighting Smoothness: $($settings.IndirectLightingSmoothness)" -ForegroundColor Gray
Write-Host "  - Estimated Build Time: $($settings.BuildTime)" -ForegroundColor Gray

Write-Host ""
Write-Host "Optimization recommendations:" -ForegroundColor Yellow
Write-Host "  1. Convert 90% of lights to static (baked)" -ForegroundColor Gray
Write-Host "  2. Use stationary lights for key dramatic lights (< 15)" -ForegroundColor Gray
Write-Host "  3. Limit dynamic lights to < 8 visible simultaneously" -ForegroundColor Gray
Write-Host "  4. Set appropriate lightmap resolution per asset type" -ForegroundColor Gray
Write-Host "  5. Use lightmass importance volume to focus quality" -ForegroundColor Gray
Write-Host "  6. Enable light functions for flickering instead of dynamic lights" -ForegroundColor Gray

# Generate optimization config
$configPath = Join-Path $OutputPath "${LevelName}_LightingOptimization.json"

$config = @{
    LevelName = $LevelName
    BuildQuality = $BuildQuality
    LightmassSettings = $settings
    LightmapResolutions = @{
        Walls = "128-256"
        Floors = "64-128"
        Props = "32-64"
        SmallObjects = "16-32"
    }
    LightLimits = @{
        StaticLights = "Unlimited"
        StationaryLights = "10-15"
        DynamicLights = "5-8"
        ShadowCastingDynamics = "3-5"
    }
    Timestamp = Get-Date -Format "yyyy-MM-dd HH:mm:ss"
}

$config | ConvertTo-Json -Depth 10 | Set-Content -Path $configPath -Encoding UTF8

Write-Host ""
Write-Host "Configuration saved: $configPath" -ForegroundColor Green
Write-Host ""
Write-Host "=== Lighting Optimization Complete ===" -ForegroundColor Cyan
