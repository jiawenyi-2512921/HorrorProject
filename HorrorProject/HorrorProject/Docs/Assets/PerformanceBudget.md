# Performance Budget - HorrorProject

**Generated:** 2026-04-26  
**Target Platform:** PC (RTX 3060, 16GB RAM, 1080p)  
**Target Performance:** 60 FPS minimum

---

## Overall Performance Budget

### Frame Time Budget (60 FPS = 16.67ms)

| Component | Budget | Current (Est.) | Status |
|-----------|--------|----------------|--------|
| **GPU Total** | 14ms | 18-20ms | ⚠️ Over |
| Base Pass | 4-6ms | 7-9ms | ⚠️ Over |
| Lighting | 2-3ms | 3-4ms | ⚠️ Over |
| Post-Process | 2-3ms | 2-3ms | ✅ OK |
| Translucency | 1-2ms | 1-2ms | ✅ OK |
| Shadows | 2-3ms | 3-4ms | ⚠️ Over |
| Other | 2-3ms | 2-3ms | ✅ OK |
| **CPU Total** | 15ms | 12-14ms | ✅ OK |
| Game Thread | 8-10ms | 7-9ms | ✅ OK |
| Render Thread | 6-8ms | 5-7ms | ✅ OK |

**Status:** GPU-bound, requires optimization

---

## Memory Budget

### VRAM Budget (8GB Target)

| Category | Budget | Current (Est.) | Status |
|----------|--------|----------------|--------|
| **Textures** | 4-5 GB | 10-12 GB | 🔴 Critical |
| Meshes | 1-1.5 GB | 3 GB | ⚠️ Over |
| Materials | 256 MB | 500 MB | ⚠️ Over |
| Render Targets | 512 MB | 400 MB | ✅ OK |
| Other | 512 MB | 300 MB | ✅ OK |
| **Total VRAM** | 6-7 GB | 14-16 GB | 🔴 Critical |

**Status:** Severely over budget, immediate optimization required

### System RAM Budget (16GB Total, 12GB Available)

| Category | Budget | Current (Est.) | Status |
|----------|--------|----------------|--------|
| Engine | 2 GB | 2 GB | ✅ OK |
| Game Code | 500 MB | 400 MB | ✅ OK |
| Audio (Loaded) | 50 MB | 200 MB | ⚠️ Over |
| Audio (Streaming) | 256 MB | 100 MB | ✅ OK |
| Blueprints | 100 MB | 80 MB | ✅ OK |
| Physics | 256 MB | 200 MB | ✅ OK |
| Other | 1 GB | 800 MB | ✅ OK |
| **Total RAM** | 8-10 GB | 10-12 GB | ⚠️ High |

**Status:** Near budget limit, monitor closely

---

## Texture Budget

### Total Texture Memory: 4-5 GB Target

| Category | Budget | Current (Est.) | Optimization |
|----------|--------|----------------|--------------|
| **Environment** | 2.5 GB | 8 GB | 🔴 Critical |
| DeepWaterStation | 1 GB | 3 GB | Compress, LOD |
| SD_Art | 1 GB | 4 GB | Downscale 8K→4K |
| Props | 500 MB | 1 GB | Compress |
| **Characters** | 500 MB | 400 MB | ✅ OK |
| Player | 200 MB | 150 MB | ✅ OK |
| Enemies | 300 MB | 250 MB | ✅ OK |
| **UI** | 100 MB | 80 MB | ✅ OK |
| **VFX** | 200 MB | 150 MB | ✅ OK |
| **Other** | 200 MB | 150 MB | ✅ OK |

### Texture Resolution Guidelines

| Asset Type | Max Resolution | Recommended | Notes |
|------------|----------------|-------------|-------|
| Hero Character | 4K | 2K | Face can be 4K |
| Standard Character | 2K | 1K | Background NPCs |
| Large Props | 2K | 1K | >2m size |
| Small Props | 1K | 512 | <1m size |
| Architecture (Unique) | 2K | 1K | Hero pieces |
| Architecture (Tiling) | 1K | 512-1K | Repeating |
| UI Elements | 2K | 1K | HUD elements |
| VFX Textures | 512 | 256-512 | Particles |

### Texture Compression

| Map Type | Format | Quality | Notes |
|----------|--------|---------|-------|
| Albedo/Diffuse | BC7 | High | sRGB enabled |
| Normal | BC5 | High | 2-channel |
| ORM (packed) | BC7 | High | 3-channel |
| Roughness | BC4 | Medium | 1-channel |
| Metallic | BC4 | Medium | 1-channel |
| Emissive | BC7 | High | sRGB enabled |
| Masks | BC4 | Medium | 1-channel |

---

## Mesh Budget

### Total Mesh Memory: 1-1.5 GB Target

| Category | Budget | Triangle Budget | LOD Levels |
|----------|--------|-----------------|------------|
| **Environment** | 800 MB | 2M on-screen | 4 LODs |
| Architecture | 500 MB | 1.2M | 4 LODs |
| Props | 300 MB | 800K | 3-4 LODs |
| **Characters** | 400 MB | 500K on-screen | 3-4 LODs |
| Player | 150 MB | 150K | 4 LODs |
| Enemies | 250 MB | 350K | 3 LODs |
| **VFX Meshes** | 50 MB | 50K | 2 LODs |
| **Other** | 250 MB | 200K | Variable |

### Triangle Budget by Asset

| Asset Type | LOD0 | LOD1 | LOD2 | LOD3 |
|------------|------|------|------|------|
| **Hero Character** | 50K | 25K | 10K | 3K |
| **Standard Character** | 30K | 15K | 7K | 2K |
| **Large Prop** | 10K | 5K | 2K | 500 |
| **Medium Prop** | 5K | 2.5K | 1K | 300 |
| **Small Prop** | 2K | 1K | 500 | 200 |
| **Architecture (Modular)** | 3K | 1.5K | 750 | 250 |

### LOD Distance Settings

| Asset Type | LOD0 | LOD1 | LOD2 | LOD3 |
|------------|------|------|------|------|
| Hero Assets | 0-15m | 15-30m | 30-60m | 60m+ |
| Standard Assets | 0-10m | 10-25m | 25-50m | 50m+ |
| Background Assets | 0-5m | 5-15m | 15-30m | 30m+ |

---

## Material Budget

### Total Material Memory: 256 MB Target

| Category | Budget | Shader Instructions | Notes |
|----------|--------|---------------------|-------|
| **Master Materials** | 50 MB | 400-600 | Complex |
| M_Master_PBR | 20 MB | 500 | Standard PBR |
| M_Master_Emissive | 15 MB | 400 | Glowing |
| M_Master_Transparent | 15 MB | 450 | Glass, water |
| **Material Instances** | 150 MB | 100-200 | Lightweight |
| Environment | 80 MB | 150 | Most used |
| Characters | 40 MB | 180 | Higher quality |
| Props | 30 MB | 120 | Standard |
| **Post-Process** | 30 MB | 300-500 | Screen effects |
| **Other** | 26 MB | Variable | Misc |

### Material Complexity Targets

| Quality Level | Shader Instructions | Texture Samples | Notes |
|---------------|---------------------|-----------------|-------|
| Simple | <100 | 1-2 | Basic color |
| Standard | 100-200 | 3-5 | Full PBR |
| Complex | 200-400 | 5-7 | Advanced effects |
| Hero | 400-600 | 7-10 | Maximum quality |

### Material Instance Consolidation

**Current:** 320+ material instances  
**Target:** 150 unique instances  
**Reduction:** 53% fewer instances

**Strategy:**
- Consolidate duplicate materials
- Use material parameter collections
- Create parent-child hierarchies
- Share materials across similar assets

---

## Audio Budget

### Total Audio Memory: 306 MB Target

| Category | Loaded Budget | Streaming Budget | File Count |
|----------|---------------|------------------|------------|
| **Ambient** | 10 MB | 100 MB | 120 files |
| Loops | 5 MB | 80 MB | 80 files |
| One-shots | 5 MB | 20 MB | 40 files |
| **Music** | 5 MB | 80 MB | 20 files |
| Tension | 2 MB | 40 MB | 10 files |
| Chase | 2 MB | 30 MB | 8 files |
| Stingers | 1 MB | 10 MB | 2 files |
| **SFX** | 25 MB | 50 MB | 180 files |
| Footsteps | 5 MB | 10 MB | 30 files |
| Doors | 3 MB | 5 MB | 20 files |
| Mechanical | 7 MB | 15 MB | 50 files |
| Horror | 10 MB | 20 MB | 80 files |
| **Voice** | 10 MB | 26 MB | 34 files |
| **Total** | 50 MB | 256 MB | 354 files |

### Audio Compression Settings

| Type | Format | Quality | Sample Rate | Streaming |
|------|--------|---------|-------------|-----------|
| Ambient Loops | OGG | 0.6 | 44.1 kHz | Yes |
| Music | OGG | 0.8 | 44.1 kHz | Yes |
| SFX | OGG | 0.7 | 44.1 kHz | >5s |
| Voice | OGG | 0.75 | 22.05 kHz | >3s |

### Concurrent Sound Limits

| Category | Max Concurrent | Priority |
|----------|----------------|----------|
| Ambient | 8 | Low |
| Music | 2 | High |
| SFX | 32 | Medium |
| Voice | 4 | Highest |
| **Total** | 46 | - |

---

## Draw Call Budget

### Total Draw Calls: 4000 Target (60 FPS)

| Category | Budget | Current (Est.) | Status |
|----------|--------|----------------|--------|
| **Opaque Meshes** | 2000-3000 | 3500-4500 | ⚠️ Over |
| Static Meshes | 1800-2700 | 3200-4000 | ⚠️ Over |
| Skeletal Meshes | 200-300 | 300-500 | ⚠️ Over |
| **Translucent** | 200-300 | 250-350 | ⚠️ High |
| Particles | 150-200 | 200-250 | ⚠️ High |
| Glass/Water | 50-100 | 50-100 | ✅ OK |
| **Shadows** | 1000-1500 | 1500-2000 | ⚠️ Over |
| **UI** | 50-100 | 80-120 | ⚠️ High |
| **Total** | 3250-4900 | 5380-7070 | 🔴 Critical |

**Status:** Significantly over budget, requires optimization

### Draw Call Reduction Strategies

1. **Mesh Instancing:** Reduce repeated meshes
2. **LOD Optimization:** Aggressive culling
3. **Material Consolidation:** Fewer material switches
4. **Shadow Optimization:** Reduce shadow-casting objects
5. **Occlusion Culling:** Hide occluded objects

---

## Particle Budget

### Total Particle Budget

| Category | Max Particles | Emitters | GPU Time |
|----------|---------------|----------|----------|
| **Ambient** | 500-1000 | 5-8 | 0.5ms |
| Dust | 300-500 | 3-5 | 0.3ms |
| Fog | 200-500 | 2-3 | 0.2ms |
| **Environmental** | 1000-2000 | 8-12 | 1.0ms |
| Water Drips | 300-500 | 4-6 | 0.3ms |
| Steam | 400-800 | 3-5 | 0.4ms |
| Sparks | 300-700 | 1-3 | 0.3ms |
| **Gameplay** | 500-1000 | 3-5 | 0.5ms |
| **Total** | 2000-4000 | 16-25 | 2.0ms |

### Particle Optimization

- Use GPU particles (Niagara)
- Implement distance culling
- Reduce max particle counts
- Use particle LODs
- Optimize material complexity

---

## Blueprint Budget

### Blueprint Performance Budget

| Category | Tick Budget | Event Budget | Notes |
|----------|-------------|--------------|-------|
| **Player Character** | 0.5ms | 0.2ms | Optimize tick |
| **AI Characters** | 0.3ms each | 0.1ms each | Max 5 active |
| **Interactables** | 0.1ms each | 0.05ms each | Disable tick |
| **Managers** | 0.2ms | 0.1ms | Singleton |
| **Total** | 2.5ms | 1.0ms | Game thread |

### Blueprint Optimization

- Minimize tick usage
- Use timers instead of tick
- Cache component references
- Optimize collision checks
- Use nativization for critical BPs

---

## Lighting Budget

### Dynamic Lights Budget

| Light Type | Max Count | Shadow Budget | Notes |
|------------|-----------|---------------|-------|
| **Directional** | 1 | 2-3ms | Sun/moon |
| **Point Lights** | 8-12 | 0.5ms total | Local lights |
| **Spot Lights** | 4-6 | 0.5ms total | Focused |
| **Rect Lights** | 2-4 | 0.3ms total | Area lights |
| **Total** | 15-23 | 3-4ms | - |

### Shadow Budget

| Shadow Type | Budget | Resolution | Notes |
|-------------|--------|------------|-------|
| Directional (CSM) | 2ms | 2048x2048 | 4 cascades |
| Dynamic Shadows | 1ms | 512-1024 | Point/spot |
| Contact Shadows | 0.5ms | Screen-space | Close detail |
| **Total** | 3.5ms | - | - |

---

## Post-Processing Budget

### Post-Process Effects: 2-3ms Target

| Effect | Budget | Quality | Notes |
|--------|--------|---------|-------|
| **Bodycam VHS** | 1.5ms | High | Core style |
| Film Grain | 0.3ms | Medium | Texture overlay |
| Chromatic Aberration | 0.2ms | Medium | Edge distortion |
| Vignette | 0.1ms | Low | Simple multiply |
| **Standard Effects** | 1.0ms | Medium | - |
| Bloom | 0.3ms | Medium | Threshold-based |
| Tone Mapping | 0.2ms | High | ACES |
| Color Grading | 0.3ms | High | LUT-based |
| Motion Blur | 0.2ms | Low | Optional |
| **Total** | 2.5ms | - | Within budget |

---

## Optimization Priority Matrix

### Critical (Week 1)

| Issue | Impact | Effort | Priority |
|-------|--------|--------|----------|
| Texture compression | -6 GB VRAM | 3h | 🔴 P0 |
| LOD generation | +20 FPS | 5h | 🔴 P1 |
| Material consolidation | -30% shaders | 4h | 🔴 P2 |

### High (Week 2)

| Issue | Impact | Effort | Priority |
|-------|--------|--------|----------|
| Audio compression | -600 MB | 5h | 🟠 P3 |
| Texture streaming | -3 GB VRAM | 2h | 🟠 P4 |
| Lightmap optimization | -1 GB | 3h | 🟠 P5 |

### Medium (Week 3)

| Issue | Impact | Effort | Priority |
|-------|--------|--------|----------|
| Nanite conversion | +15 FPS | 7h | 🟡 P6 |
| Blueprint optimization | +10 FPS | 5h | 🟡 P7 |
| Collision optimization | +5 FPS | 4h | 🟡 P8 |

---

## Validation & Monitoring

### Performance Profiling Commands

```
GPU Profiling:
  stat GPU
  stat RHI
  profilegpu

CPU Profiling:
  stat Game
  stat SceneRendering
  stat Engine
  stat Unit

Memory Profiling:
  stat Memory
  stat Streaming
  stat StreamingDetails
  memreport

Draw Calls:
  stat SceneRendering
  stat RHI
```

### Target Metrics

**Minimum Spec (RTX 3060, 1080p):**
- FPS: 60+ (High settings)
- Frame Time: <16.67ms
- VRAM: <6 GB
- RAM: <10 GB
- Load Time: <30s

**Recommended Spec (RTX 3070, 1080p):**
- FPS: 90+ (Epic settings)
- Frame Time: <11ms
- VRAM: <7 GB
- RAM: <12 GB
- Load Time: <20s

---

## Budget Compliance Tracking

### Weekly Review Checklist

- [ ] Run performance profiling
- [ ] Check VRAM usage (stat Memory)
- [ ] Verify FPS targets (stat FPS)
- [ ] Review draw calls (stat SceneRendering)
- [ ] Test on target hardware
- [ ] Update budget document
- [ ] Document any overages
- [ ] Plan optimization work

### Monthly Audit

- [ ] Full performance audit
- [ ] Asset size review
- [ ] Optimization effectiveness
- [ ] Budget adjustments
- [ ] Team review meeting

---

**Document Owner:** Asset Manager Agent  
**Last Updated:** 2026-04-26  
**Next Review:** Weekly during optimization phase
