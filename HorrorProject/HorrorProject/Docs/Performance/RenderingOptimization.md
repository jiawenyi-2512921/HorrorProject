# Rendering Optimization Guide - HorrorProject

## Overview

Rendering optimization for 60 FPS @ Epic quality with Lumen, VSM, and selective Ray Tracing on Windows PC.

---

## 1. Rendering Budget Breakdown

### Target Frame Budget: 16.67ms (60 FPS)

```
Recommended Budget Allocation:
├── Game Thread: 10ms (60%)
│   ├── Gameplay Logic: 1ms
│   ├── Animation: 2ms
│   ├── Physics: 1ms
│   ├── AI: 2ms
│   └── Misc: 4ms
│
├── Render Thread: 12ms (72%)
│   ├── Scene Traversal: 2ms
│   ├── Culling: 1ms
│   ├── Draw Call Submission: 3ms
│   └── Misc: 6ms
│
└── GPU: 14ms (84%)
    ├── Base Pass: 2ms
    ├── Lumen GI: 8ms
    ├── VSM Shadows: 3ms
    ├── Post-Process: 1ms
    └── Misc: 0ms
```

---

## 2. Lumen Optimization

### Current Configuration Analysis

**Lumen Components**:
- Screen Space Tracing: Fast, limited range
- Mesh SDF Tracing: Medium cost, good quality
- Radiance Cache: Amortized cost, excellent quality
- Hardware Ray Tracing: Expensive, best quality

### Optimized Lumen Settings

```ini
; DefaultEngine.ini - Lumen Configuration for 60 FPS

[/Script/Engine.RendererSettings]
; === Core Lumen Settings ===
r.Lumen.DiffuseIndirect.Allow=1
r.Lumen.Reflections.Allow=1
r.Lumen.TranslucencyReflections.FrontLayer.Enable=0
r.Lumen.TranslucencyVolume.Enable=0

; === Screen Probe Gather (GI) ===
r.Lumen.ScreenProbeGather.ScreenTraces=1
r.Lumen.ScreenProbeGather.TraceMeshSDFs=1
r.Lumen.ScreenProbeGather.RadianceCache.Enable=1
r.Lumen.ScreenProbeGather.RadianceCache.ProbeResolution=16
r.Lumen.ScreenProbeGather.RadianceCache.NumProbesToTraceBudget=128
r.Lumen.ScreenProbeGather.ScreenTracesThickness=1.0
r.Lumen.ScreenProbeGather.DownsampleFactor=2

; === Reflections ===
r.Lumen.Reflections.ScreenTraces=1
r.Lumen.Reflections.MaxRoughnessToTrace=0.4
r.Lumen.Reflections.BilateralFilter=1
r.Lumen.Reflections.Temporal=1
r.Lumen.Reflections.DownsampleFactor=2
r.Lumen.Reflections.MaxReflectionBounces=1

; === Scene Representation ===
r.Lumen.Scene.SurfaceCache.CardCaptureRefreshFraction=0.125
r.Lumen.Scene.SurfaceCache.MeshCardsMinSize=10
r.Lumen.Scene.SurfaceCache.AtlasSize=2048

; === Hardware Ray Tracing (if available) ===
r.Lumen.HardwareRayTracing=0  ; Disable for performance
r.Lumen.HardwareRayTracing.Reflections=0
r.Lumen.HardwareRayTracing.ScreenProbeGather=0
```

### Lumen Quality Levels

**Epic (Target)**:
```ini
r.Lumen.ScreenProbeGather.RadianceCache.NumProbesToTraceBudget=128
r.Lumen.ScreenProbeGather.DownsampleFactor=2
r.Lumen.Reflections.DownsampleFactor=2
```

**High (Fallback)**:
```ini
r.Lumen.ScreenProbeGather.RadianceCache.NumProbesToTraceBudget=96
r.Lumen.ScreenProbeGather.DownsampleFactor=4
r.Lumen.Reflections.DownsampleFactor=4
```

### Lumen Scene Optimization

1. **Mesh Distance Fields**
   - Enable for all static meshes
   - Use appropriate resolution
   - Disable for small/distant objects

2. **Surface Cache**
   - Limit card capture updates
   - Use appropriate atlas size
   - Cull distant surfaces

3. **Radiance Cache**
   - Reduce probe count in open areas
   - Increase density in complex areas
   - Use probe occlusion

---

## 3. Virtual Shadow Maps (VSM)

### Optimized VSM Configuration

```ini
; Virtual Shadow Maps Settings

[/Script/Engine.RendererSettings]
; === Core VSM ===
r.Shadow.Virtual.Enable=1
r.Shadow.Virtual.MaxPhysicalPages=4096
r.Shadow.Virtual.Cache.StaticSeparate=1

; === Resolution & LOD ===
r.Shadow.Virtual.ResolutionLodBiasDirectional=-1.5
r.Shadow.Virtual.ResolutionLodBiasLocal=-1.0
r.Shadow.Virtual.ResolutionLodBiasLocalLightsPerspective=-0.5

; === SMRT (Shadow Map Ray Tracing) ===
r.Shadow.Virtual.SMRT.RayCountDirectional=4
r.Shadow.Virtual.SMRT.RayCountLocal=4
r.Shadow.Virtual.SMRT.SamplesPerRayDirectional=2
r.Shadow.Virtual.SMRT.SamplesPerRayLocal=2
r.Shadow.Virtual.SMRT.MaxRayAngleFromLight=0.03

; === Clipmap ===
r.Shadow.Virtual.Clipmap.FirstLevel=5
r.Shadow.Virtual.Clipmap.LastLevel=15
r.Shadow.Virtual.Clipmap.FirstCoarseLevel=10

; === Caching ===
r.Shadow.Virtual.Cache.InvalidateInstancePagesOnly=1
r.Shadow.Virtual.Cache.DrawBounds=0
r.Shadow.Virtual.NonNanite.IncludeInCoarsePages=1
```

### VSM Performance Tips

1. **Light Configuration**
   - Use VSM for directional light only
   - Local lights: Use traditional shadow maps
   - Limit shadow-casting lights to 3-4

2. **Geometry Optimization**
   - Enable Nanite for VSM benefits
   - Use appropriate LODs
   - Cull shadow casters aggressively

3. **Cache Management**
   - Enable static separate cache
   - Invalidate only changed pages
   - Use appropriate clipmap levels

---

## 4. Culling & Visibility

### Occlusion Culling

```ini
; Occlusion Culling Settings
r.HZBOcclusion=1
r.HZBOcclusion.Async=1
r.AllowOcclusionQueries=1
r.DownsampledOcclusionQueries=1
```

### Distance Culling

```cpp
// Per-actor cull distances (cm)
Small Props: 2000 (20m)
Medium Props: 5000 (50m)
Large Props: 10000 (100m)
Environment: 20000 (200m)
```

### Frustum Culling

- Enable per-object bounds
- Use hierarchical culling
- Implement portal-based culling for rooms

### Precomputed Visibility

For static areas:
1. Generate precomputed visibility volumes
2. Place in key locations (doorways, corridors)
3. Reduces runtime culling cost

---

## 5. Draw Call Optimization

### Target: <3000 Draw Calls

**Current Bottlenecks**:
- Unique materials per object
- Non-instanced meshes
- Overdraw from transparency

### Optimization Strategies

#### 1. Instanced Static Meshes

```cpp
// Use ISM for repeated objects
- Pipes, cables, debris: 100+ instances
- Lights, fixtures: 50+ instances
- Small props: 20+ instances

Expected Reduction: 500-1000 draw calls
```

#### 2. Material Merging

```cpp
// Merge materials with texture atlases
Before: 50 unique materials = 50 draw calls
After: 5 atlased materials = 5 draw calls

Expected Reduction: 200-400 draw calls
```

#### 3. Mesh Merging

```cpp
// Merge static environment meshes
- Room geometry: Merge per room
- Trim pieces: Merge into single mesh
- Background details: Merge aggressively

Expected Reduction: 300-500 draw calls
```

#### 4. Nanite

```cpp
// Nanite reduces draw calls automatically
Traditional: 1 draw call per LOD per instance
Nanite: Clustered rendering, massive reduction

Expected Reduction: 1000+ draw calls
```

---

## 6. Overdraw Reduction

### Overdraw Budget: <3x

**High Overdraw Sources**:
1. Translucent particles
2. Overlapping decals
3. Complex materials
4. UI elements

### Optimization Techniques

#### 1. Translucency Sorting

```ini
r.TranslucencySortPolicy=2  ; Sort by distance
r.TranslucencySortAxis=1    ; Use camera forward
```

#### 2. Decal Optimization

- Limit overlapping decals to 3
- Use decal atlases
- Cull distant decals aggressively
- Use DBuffer decals for static

#### 3. Particle Overdraw

- Use GPU particles with depth buffer collision
- Reduce particle size at distance
- Limit particle layers
- Use additive blending where possible

---

## 7. Texture Streaming

### Streaming Pool Configuration

```ini
[/Script/Engine.RendererSettings]
r.Streaming.PoolSize=3000
r.Streaming.MaxEffectiveScreenSize=0
r.Streaming.FullyLoadUsedTextures=1
r.Streaming.UseFixedPoolSize=1
r.Streaming.HLODStrategy=2
r.Streaming.LimitPoolSizeToVRAM=1
r.Streaming.MaxTempMemoryAllowed=50
```

### Mipmap Bias

```ini
; Adjust for quality vs performance
r.Streaming.MipBias=0  ; Epic quality
r.Streaming.MipBias=1  ; High quality (saves memory)
```

### Virtual Texturing

```ini
; Enable for large textures
r.VT.Enable=1
r.VT.MaxUploadsPerFrame=16
r.VT.MaxPagesProducedPerFrame=32
r.VT.MaxAnisotropy=8
```

---

## 8. Post-Process Optimization

### Post-Process Budget: 1-2ms

```ini
; Optimized Post-Process Settings

; === Motion Blur ===
r.MotionBlurQuality=3
r.MotionBlur.Max=5
r.MotionBlur.Amount=0.5

; === Depth of Field ===
r.DepthOfFieldQuality=2
r.DOF.Gather.AccumulatorQuality=0
r.DOF.Gather.PostfilterMethod=1
r.DOF.Gather.RingCount=4
r.DOF.Scatter.MaxSpriteRatio=0.04

; === Bloom ===
r.BloomQuality=4
r.Bloom.Cross=0
r.FastBlurThreshold=0

; === Lens Flares ===
r.LensFlareQuality=2

; === Eye Adaptation ===
r.EyeAdaptationQuality=2
r.EyeAdaptation.ExponentialTransitionDistance=1.5

; === Ambient Occlusion ===
r.AmbientOcclusionLevels=2
r.AmbientOcclusionRadiusScale=1.0

; === Disable Expensive Effects ===
r.SSR.Quality=0  ; Use Lumen instead
r.SSAO.Enable=0  ; Use Lumen instead
r.ContactShadows=0  ; Use VSM instead
```

### Selective Post-Process

Use Post-Process Volumes to:
- Disable effects in simple areas
- Reduce quality at distance
- Enable expensive effects only in hero moments

---

## 9. Scalability System

### Custom Scalability Settings

```ini
; DefaultScalability.ini

[ViewDistanceQuality@3]  ; Epic
r.SkeletalMeshLODBias=0
r.ViewDistanceScale=1.0
r.MeshLODDistanceScale=1.0

[AntiAliasingQuality@3]  ; Epic
r.PostProcessAAQuality=6  ; TSR
r.TemporalAA.Quality=2
r.TemporalAA.Upsampling=1

[ShadowQuality@3]  ; Epic
r.Shadow.MaxResolution=2048
r.Shadow.MaxCSMResolution=2048
r.Shadow.RadiusThreshold=0.01
r.Shadow.DistanceScale=1.0
r.Shadow.CSM.MaxCascades=4

[GlobalIlluminationQuality@3]  ; Epic
r.Lumen.DiffuseIndirect.Allow=1
r.Lumen.Reflections.Allow=1
r.LumenScene.Radiosity.ProbeSpacing=4
r.LumenScene.Radiosity.HemisphereProbeResolution=4

[ReflectionQuality@3]  ; Epic
r.Lumen.Reflections.MaxRoughnessToTrace=0.4
r.Lumen.Reflections.ScreenTraces=1

[PostProcessQuality@3]  ; Epic
r.MotionBlurQuality=3
r.AmbientOcclusionLevels=2
r.DepthOfFieldQuality=2
r.BloomQuality=4
r.LensFlareQuality=2
r.EyeAdaptationQuality=2

[TextureQuality@3]  ; Epic
r.Streaming.MipBias=0
r.Streaming.AmortizeCPUToGPUCopy=1
r.Streaming.MaxNumTexturesToStreamPerFrame=0
r.Streaming.Boost=1

[EffectsQuality@3]  ; Epic
r.TranslucencyLightingVolumeDim=64
r.RefractionQuality=2
r.SSR.Quality=0
r.SSS.Scale=1
r.SSS.SampleSet=2
r.SSGI.Enable=0

[FoliageQuality@3]  ; Epic
foliage.DensityScale=1.0
grass.DensityScale=1.0
foliage.DiscardDataOnLoad=0
```

---

## 10. GPU Profiling & Optimization

### Profiling Commands

```
; In-game console commands
stat fps
stat unit
stat gpu
profilegpu
r.ScreenPercentage 100
```

### GPU Bottleneck Identification

**Base Pass Heavy**:
- Too many draw calls
- Complex materials
- High triangle count
→ Solution: Reduce draw calls, simplify materials, use Nanite

**Lighting Heavy**:
- Too many lights
- High shadow resolution
- Expensive light functions
→ Solution: Reduce lights, optimize shadows, simplify light functions

**Lumen Heavy**:
- High probe count
- Too many mesh SDFs
- Large radiance cache
→ Solution: Reduce probe budget, optimize scene, tune settings

**Post-Process Heavy**:
- Expensive effects enabled
- High resolution buffers
- Multiple passes
→ Solution: Disable expensive effects, reduce quality, optimize passes

---

## 11. Nanite Configuration

### Nanite Settings

```ini
; Nanite Configuration
r.Nanite.MaxPixelsPerEdge=1.0
r.Nanite.MinPixelsPerEdgeHW=32
r.Nanite.ViewMeshLODBias.Offset=0.0
r.Nanite.ViewMeshLODBias.Min=-2.0
r.Nanite.ViewMeshLODBias.Max=0.0
r.Nanite.Streaming.MaxPendingPages=128
r.Nanite.Streaming.MaxPageInstallsPerFrame=128
```

### Nanite Best Practices

1. **Enable for**:
   - Static environment meshes
   - High-poly props (>10K triangles)
   - Repeated instances

2. **Disable for**:
   - Skeletal meshes
   - Translucent materials
   - Meshes with WPO
   - Very small meshes (<1K triangles)

3. **Optimization**:
   - Use Nanite-compatible materials
   - Avoid pixel depth offset
   - Enable VSM for best results

---

## 12. Temporal Super Resolution (TSR)

### TSR Configuration

```ini
; TSR Settings for 60 FPS
r.TemporalAA.Algorithm=1  ; Use TSR
r.TemporalAA.Quality=2    ; High quality
r.TemporalAA.Upsampling=1 ; Enable upsampling
r.TSR.ShadingRejection.Flickering=1
r.TSR.Velocity.Extrapolation=1
r.ScreenPercentage=67     ; Render at 67%, upscale to 100%
```

### TSR Benefits

- Render at lower resolution (67-80%)
- Upscale to native with high quality
- Gain: 30-40% performance improvement
- Minimal quality loss

---

## 13. Rendering Optimization Checklist

### Immediate Actions
- [ ] Configure Lumen for 60 FPS
- [ ] Optimize VSM settings
- [ ] Enable TSR with 67% screen percentage
- [ ] Disable expensive post-process effects
- [ ] Configure texture streaming

### Short-term Actions
- [ ] Enable Nanite for environment
- [ ] Implement instanced static meshes
- [ ] Merge materials with atlases
- [ ] Optimize draw calls
- [ ] Configure culling distances

### Long-term Actions
- [ ] Implement custom scalability
- [ ] Setup precomputed visibility
- [ ] Optimize overdraw
- [ ] Virtual texture implementation
- [ ] Portal-based culling system

---

## 14. Performance Targets

### Frame Time Breakdown (Target)

```
Total: 16.67ms (60 FPS)

Game Thread: 8ms
├── Tick: 0.5ms
├── Animation: 2ms
├── Physics: 1ms
├── AI: 1.5ms
└── Misc: 3ms

Render Thread: 10ms
├── Scene: 2ms
├── Culling: 1ms
├── Draw Calls: 4ms
└── Misc: 3ms

GPU: 14ms
├── Base Pass: 2ms
├── Lumen: 8ms
├── VSM: 3ms
├── Post-Process: 1ms
└── Misc: 0ms
```

### Success Criteria

- Maintain 60 FPS in all areas
- GPU time <14ms
- Draw calls <3000
- Overdraw <3x
- Memory <4GB
- Stable frame times (no hitches)

---

**Next Steps**: Apply rendering optimizations, profile with Unreal Insights, iterate based on measurements.
