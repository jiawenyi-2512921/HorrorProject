# Asset Optimization Guide - HorrorProject

## Overview

Asset optimization strategy for 60 FPS @ Epic quality with DeepWaterStation (15.6GB) and SD_Art assets.

---

## 1. Texture Optimization

### Current Asset Analysis

**DeepWaterStation**: 15.6GB total
- Estimated texture memory: ~8-10GB uncompressed
- Likely using 4K textures throughout
- Potential for significant optimization

### Optimization Strategy

#### Texture Resolution Guidelines

| Asset Type | Max Resolution | Compression | Mipmaps |
|------------|----------------|-------------|---------|
| Hero Props (close-up) | 2K (2048x2048) | BC7 | Yes |
| Standard Props | 1K (1024x1024) | BC7 | Yes |
| Background Props | 512x512 | BC7 | Yes |
| Trim Sheets | 2K | BC7 | Yes |
| Normal Maps | Same as albedo | BC5 | Yes |
| Roughness/Metallic | Half of albedo | BC4/BC5 | Yes |
| UI Textures | 512x512 | BC7 | No |

#### Texture Streaming

```ini
; DefaultEngine.ini
[/Script/Engine.RendererSettings]
r.Streaming.PoolSize=3000
r.Streaming.MaxEffectiveScreenSize=0
r.Streaming.FullyLoadUsedTextures=1
r.Streaming.UseFixedPoolSize=1
```

#### Virtual Texturing

Enable for large terrain/environment textures:
- Reduces memory footprint
- Improves streaming performance
- Ideal for DeepWaterStation's large surfaces

**Implementation**:
1. Enable Virtual Texture Support in Project Settings
2. Convert large textures (>2K) to Runtime Virtual Textures
3. Use RVT for terrain, walls, floors

### Expected Gains

- Memory reduction: 3-4GB
- Streaming performance: +15%
- Load times: -30%

---

## 2. Mesh Optimization

### LOD Strategy

#### Automatic LOD Generation

```cpp
// LOD Settings for Static Meshes
LOD 0: 100% triangles (0-5m)
LOD 1: 50% triangles (5-15m)
LOD 2: 25% triangles (15-30m)
LOD 3: 10% triangles (30m+)
```

#### Nanite Enablement

**Candidates for Nanite**:
- Static environment meshes (walls, floors, ceilings)
- High-poly props (>10K triangles)
- Repeated instances (pipes, cables, debris)

**NOT for Nanite**:
- Skeletal meshes
- Translucent materials
- Meshes with World Position Offset
- Small props (<1K triangles)

**Implementation**:
1. Select high-poly static meshes
2. Enable Nanite in mesh settings
3. Test performance impact
4. Verify no visual artifacts

### Triangle Budget

| Category | Budget | Notes |
|----------|--------|-------|
| Environment (total) | 3M triangles | Use Nanite for high-poly |
| Characters | 50K per character | LODs required |
| Props (visible) | 1M triangles | Aggressive LODs |
| Particles | 100K triangles | Use simple geometry |
| **Total** | **5M triangles** | Per frame target |

### Mesh Optimization Checklist

- [ ] Generate LODs for all static meshes
- [ ] Enable Nanite for environment meshes
- [ ] Remove unnecessary vertices
- [ ] Merge small meshes into single draw calls
- [ ] Use instanced static meshes where possible
- [ ] Optimize collision meshes (use simple collision)

---

## 3. Material Optimization

### Material Complexity Budget

**Target**: <200 instructions per material (Epic quality)

| Material Type | Instruction Budget | Notes |
|---------------|-------------------|-------|
| Simple (walls, floors) | 100-150 | Basic PBR |
| Standard (props) | 150-200 | PBR + detail |
| Complex (hero props) | 200-300 | Full features |
| Translucent | 100-150 | Expensive, minimize |
| UI | 50-100 | Keep simple |

### Optimization Techniques

#### 1. Material Instances
- Use Material Instances instead of unique materials
- Share base materials across assets
- Reduces shader compilation and memory

#### 2. Texture Sampling Reduction
```
Current: 8-10 texture samples per material
Target: 4-6 texture samples per material

Optimization:
- Pack textures (R=Metallic, G=Roughness, B=AO)
- Use texture atlases
- Remove unnecessary detail maps
```

#### 3. Shader Complexity
- Avoid complex math in pixel shader
- Move calculations to vertex shader when possible
- Use static switches for features
- Disable features not visible at distance

#### 4. Translucency Optimization
```ini
; Reduce translucency cost
r.Translucency.MaxGridSize=128
r.Translucency.GridPixelSize=32
```

### Material LOD System

Implement material quality scaling:
```cpp
// Material Quality Levels
Epic: Full features (close range)
High: Reduced detail (medium range)
Medium: Basic PBR (far range)
Low: Unlit (very far range)
```

---

## 4. Lighting Optimization

### Lumen Configuration

**Target**: 8ms GPU budget for Lumen

```ini
; Lumen Settings for 60 FPS
[/Script/Engine.RendererSettings]
r.Lumen.DiffuseIndirect.Allow=1
r.Lumen.Reflections.Allow=1
r.Lumen.ScreenProbeGather.RadianceCache.Enable=1

; Performance tuning
r.Lumen.ScreenProbeGather.ScreenTraces=1
r.Lumen.ScreenProbeGather.TraceMeshSDFs=1
r.Lumen.Reflections.ScreenTraces=1
r.Lumen.Reflections.MaxRoughnessToTrace=0.4
r.Lumen.TranslucencyReflections.FrontLayer.Enable=0

; Quality vs Performance
r.Lumen.ScreenProbeGather.ScreenTracesThickness=1.0
r.Lumen.Reflections.BilateralFilter=1
r.Lumen.Reflections.Temporal=1
```

### Virtual Shadow Maps (VSM)

**Target**: 3ms GPU budget for VSM

```ini
; VSM Settings
r.Shadow.Virtual.Enable=1
r.Shadow.Virtual.MaxPhysicalPages=4096
r.Shadow.Virtual.ResolutionLodBiasDirectional=-1.5
r.Shadow.Virtual.ResolutionLodBiasLocal=-1.0
r.Shadow.Virtual.SMRT.RayCountDirectional=4
r.Shadow.Virtual.SMRT.RayCountLocal=4
r.Shadow.Virtual.SMRT.SamplesPerRayDirectional=2
r.Shadow.Virtual.SMRT.SamplesPerRayLocal=2
```

### Light Count Budget

| Light Type | Max Count | Notes |
|------------|-----------|-------|
| Directional | 1 | Main light only |
| Stationary Point | 10 | Key area lights |
| Movable Point | 5 | Dynamic only |
| Spot Lights | 8 | Flashlight + effects |
| Rect Lights | 4 | Hero lighting only |

### Optimization Checklist

- [ ] Limit overlapping light sources
- [ ] Use stationary lights where possible
- [ ] Disable shadows on small lights
- [ ] Set appropriate light attenuation radius
- [ ] Use IES profiles instead of complex geometry
- [ ] Bake lighting for static elements (if possible)

---

## 5. Ray Tracing Optimization

### RT Configuration

**Selective Ray Tracing**: Enable only where needed

```ini
; Ray Tracing Settings
r.RayTracing.Shadows=1
r.RayTracing.Reflections=0  ; Use Lumen instead
r.RayTracing.AmbientOcclusion=0  ; Use Lumen instead
r.RayTracing.GlobalIllumination=0  ; Use Lumen instead

; Shadow quality
r.RayTracing.Shadows.SamplesPerPixel=1
r.RayTracing.Shadows.MaxBounces=1
```

### RT Geometry Budget

- Limit RT-enabled meshes to visible surfaces
- Disable RT on small/distant objects
- Use RT LODs for complex geometry

---

## 6. Particle System Optimization

### VFX Budget

| Effect Type | Max Particles | Max Emitters |
|-------------|---------------|--------------|
| Ambient (fog, dust) | 500 | 5 |
| Interactive | 200 | 3 |
| Impact effects | 100 | 2 |
| **Total Active** | **1000** | **10** |

### Optimization Techniques

1. **GPU Particles**: Use for large counts
2. **LOD System**: Reduce particles at distance
3. **Bounds Culling**: Aggressive culling
4. **Pooling**: Reuse particle systems
5. **Fixed Bounds**: Avoid dynamic bounds calculation

---

## 7. Audio Optimization

### Audio Asset Guidelines

| Asset Type | Format | Compression | Notes |
|------------|--------|-------------|-------|
| Music | Streaming WAV | Vorbis | Stream from disk |
| Ambience | Streaming WAV | Vorbis | Loop, stream |
| SFX (long) | WAV | Vorbis | >5s duration |
| SFX (short) | WAV | PCM | <5s, load to memory |
| Voice | WAV | Vorbis | Stream |

### Concurrency Limits

```cpp
// Audio concurrency settings
Max Ambient Sounds: 8
Max SFX Sounds: 16
Max Voice Lines: 2
Total Active Sounds: 32
```

### Attenuation

- Use appropriate attenuation curves
- Set realistic max distance
- Enable spatialization only when needed
- Use sound classes for global control

---

## 8. Animation Optimization

### Skeletal Mesh LODs

```cpp
LOD 0: Full skeleton (0-5m)
LOD 1: 75% bones (5-10m)
LOD 2: 50% bones (10-20m)
LOD 3: 25% bones (20m+)
```

### Animation Compression

- Use Automatic Compression
- Remove unnecessary tracks
- Reduce keyframe density
- Use additive animations where possible

### Animation Budget

- Max active skeletal meshes: 10
- Max bones per character: 150
- Animation update rate: 30 Hz at distance

---

## 9. Post-Process Optimization

### Post-Process Budget: 2ms

```ini
; Post-process settings
r.MotionBlurQuality=3
r.DepthOfFieldQuality=2
r.BloomQuality=4
r.LensFlareQuality=2
r.EyeAdaptationQuality=2
r.AmbientOcclusionLevels=2
r.AmbientOcclusionRadiusScale=1.0
```

### Disable Expensive Effects

- Screen Space Reflections (use Lumen)
- SSAO (use Lumen)
- Contact Shadows (use VSM)

---

## 10. Asset Streaming

### Streaming Configuration

```ini
[/Script/Engine.StreamingSettings]
s.AsyncLoadingThreadEnabled=True
s.EventDrivenLoaderEnabled=True
s.WarnIfTimeLimitExceeded=True
s.TimeLimitExceededMultiplier=1.5
s.TimeLimitExceededMinTime=0.005
s.UseBackgroundLevelStreaming=True
s.PriorityAsyncLoadingExtraTime=15.0
s.LevelStreamingActorsUpdateTimeLimit=5.0
s.PriorityLevelStreamingActorsUpdateExtraTime=5.0
s.LevelStreamingComponentsRegistrationGranularity=10
s.UnregisterComponentsTimeLimit=1.0
s.LevelStreamingComponentsUnregistrationGranularity=5
```

### Level Streaming Strategy

1. **Divide DeepWaterStation into zones**
   - Main areas: Always loaded
   - Side rooms: Stream on demand
   - Distant areas: Unload when far

2. **Streaming Volumes**
   - Place at doorways/transitions
   - Preload adjacent areas
   - Unload distant areas

3. **Asset Prioritization**
   - High: Player-visible assets
   - Medium: Adjacent areas
   - Low: Background/distant assets

---

## 11. Implementation Priority

### Phase 1 (Week 1) - Quick Wins
- [ ] Texture resolution reduction
- [ ] Enable texture streaming
- [ ] Generate mesh LODs
- [ ] Material instance conversion
- [ ] Disable expensive post-process

**Expected Gain**: 5-8ms per frame

### Phase 2 (Week 2) - Medium Effort
- [ ] Enable Nanite for environment
- [ ] Implement material LOD system
- [ ] Optimize Lumen settings
- [ ] Configure VSM properly
- [ ] Audio compression and streaming

**Expected Gain**: 3-5ms per frame

### Phase 3 (Week 3) - High Effort
- [ ] Virtual texturing implementation
- [ ] Level streaming setup
- [ ] Particle system optimization
- [ ] Animation LOD system
- [ ] RT selective enablement

**Expected Gain**: 2-4ms per frame

---

## 12. Measurement & Validation

### Profiling Commands

```
stat fps
stat unit
stat gpu
stat scenerendering
stat streaming
stat memory
profilegpu
```

### Target Metrics

- Frame time: <16.67ms (60 FPS)
- GPU time: <14ms
- Draw calls: <3000
- Triangles: <5M
- Texture memory: <3GB
- Total memory: <4GB

---

**Next Steps**: Begin Phase 1 optimizations, profile after each change, document actual gains.
