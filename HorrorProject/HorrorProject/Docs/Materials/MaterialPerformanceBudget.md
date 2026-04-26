# Material Performance Budget

**Document Type:** Performance Standards  
**Target Audience:** Technical Artists, Programmers  
**Last Updated:** 2026-04-26

---

## Overview

Performance budgets and optimization targets for materials in HorrorProject. Based on target hardware (RTX 3060, 16GB RAM, 1080p @ 60 FPS).

---

## Overall Material Budget

### Current Status

| Metric | Current | Target | Status |
|--------|---------|--------|--------|
| Total VRAM | 5.9 GB | 4.0 GB | 🔴 47% Over |
| Material Count | 303 | 200 | 🔴 51% Over |
| Avg Instructions | 363 | 300 | ⚠️ 21% Over |
| Base Pass Time | 7-9ms | 4-6ms | 🔴 Critical |

**Priority:** Critical optimization required

---

## Per-Material Budgets

### Surface Materials

| Complexity | Instructions | Samples | VRAM | Use Case |
|------------|--------------|---------|------|----------|
| Simple | <200 | 1-2 | <10 MB | Background, distant |
| Standard | 200-400 | 3-5 | 10-20 MB | Most surfaces |
| Complex | 400-600 | 5-7 | 20-30 MB | Hero assets |
| Heavy | >600 | >7 | >30 MB | Avoid if possible |

**Target Distribution:**
- Simple: 30% of materials
- Standard: 50% of materials
- Complex: 15% of materials
- Heavy: 5% of materials

**Current Distribution:**
- Simple: 15% ⚠️ Too few
- Standard: 59% ✅ Good
- Complex: 21% ⚠️ Too many
- Heavy: 5% ✅ Acceptable

### Post-Process Materials

| Effect Type | Instructions | Samples | GPU Time | Notes |
|-------------|--------------|---------|----------|-------|
| Simple Filter | <150 | 1-2 | <0.3ms | Color grading |
| Standard Effect | 150-300 | 2-4 | 0.3-0.5ms | VHS, blur |
| Complex Effect | 300-500 | 4-8 | 0.5-1.0ms | Multi-effect |
| Heavy Effect | >500 | >8 | >1.0ms | Avoid |

**Total Post-Process Budget:** 2-3ms per frame

### UI Materials

| Type | Instructions | Samples | Notes |
|------|--------------|---------|-------|
| Simple UI | <100 | 1 | Text, icons |
| Standard UI | 100-150 | 1-2 | Buttons, panels |
| Complex UI | 150-200 | 2-3 | Animated elements |

**Total UI Budget:** <0.5ms per frame

---

## Texture Budget

### Resolution Guidelines

| Asset Type | Max Resolution | Recommended | Memory (BC7) |
|------------|----------------|-------------|--------------|
| Hero Character | 4K | 2K | 21 MB → 5 MB |
| Standard Character | 2K | 1K | 5 MB → 1.3 MB |
| Large Props (>2m) | 2K | 1K | 5 MB → 1.3 MB |
| Small Props (<1m) | 1K | 512 | 1.3 MB → 340 KB |
| Architecture (Unique) | 2K | 1K | 5 MB → 1.3 MB |
| Architecture (Tiling) | 1K | 512-1K | 1.3 MB |
| UI Elements | 2K | 1K | 5 MB → 1.3 MB |
| VFX Textures | 512 | 256-512 | 340 KB |

### Texture Memory Budget

| Category | Budget | Current | Status |
|----------|--------|---------|--------|
| Environment | 2.0 GB | 3.5 GB | 🔴 75% Over |
| Props | 800 MB | 1.2 GB | ⚠️ 50% Over |
| Characters | 600 MB | 800 MB | ⚠️ 33% Over |
| VFX | 200 MB | 200 MB | ✅ OK |
| UI | 150 MB | 150 MB | ✅ OK |
| Post-Process | 50 MB | 50 MB | ✅ OK |
| **Total** | **3.8 GB** | **5.9 GB** | 🔴 55% Over |

### Compression Settings

| Map Type | Format | Quality | Memory Savings |
|----------|--------|---------|----------------|
| Albedo/Diffuse | BC7 | High | 75% vs uncompressed |
| Normal | BC5 | High | 50% vs BC7 |
| ORM (packed) | BC7 | High | 67% vs 3 separate |
| Roughness | BC4 | Medium | 87% vs uncompressed |
| Metallic | BC4 | Medium | 87% vs uncompressed |
| Emissive | BC7 | High | 75% vs uncompressed |
| Masks | BC4 | Medium | 87% vs uncompressed |

---

## Shader Complexity Budget

### Instruction Count Targets

**By Material Type:**

| Material Type | Target | Max | Current Avg | Status |
|---------------|--------|-----|-------------|--------|
| Simple Surface | 150 | 200 | 180 | ✅ OK |
| Standard Surface | 300 | 400 | 380 | ⚠️ High |
| Complex Surface | 450 | 600 | 520 | ✅ OK |
| VHS Effect | 250 | 350 | 320 | ✅ OK |
| Post-Process | 200 | 300 | 250 | ✅ OK |
| UI | 100 | 150 | 120 | ✅ OK |

### Feature Costs

| Feature | Instruction Cost | When to Use |
|---------|------------------|-------------|
| Base PBR | 150 | Always |
| Normal Mapping | +40 | Most surfaces |
| Detail Layer | +60 | Hero assets only |
| Parallax Mapping | +80 | Hero assets only |
| Emissive | +30 | When needed |
| Vertex Color | +10 | Painted meshes |
| Chromatic Aberration | +45 | VHS effects |
| VHS Noise | +60 | VHS effects |
| Scanlines | +30 | VHS effects |
| Blur (5x5) | +80 | Post-process |
| Glow | +100 | Post-process |

---

## Draw Call Budget

### Material Batching

**Target:** Reduce from 3500+ to 2500 draw calls

**Strategies:**

1. **Material Consolidation**
   - Current: 303 instances
   - Target: 200 instances
   - Savings: ~100 draw calls

2. **Texture Atlasing**
   - Combine similar materials
   - Share texture atlases
   - Savings: ~200 draw calls

3. **Instancing**
   - Use instanced static meshes
   - Batch similar objects
   - Savings: ~500 draw calls

4. **LOD Optimization**
   - Aggressive culling
   - Simplified distant materials
   - Savings: ~200 draw calls

---

## Performance Targets by Hardware

### Minimum Spec (RTX 3060, 1080p)

| Metric | Target | Notes |
|--------|--------|-------|
| FPS | 60+ | High settings |
| Frame Time | <16.67ms | Consistent |
| VRAM | <6 GB | With streaming |
| Base Pass | <6ms | Material rendering |
| Post-Process | <3ms | All effects |

### Recommended Spec (RTX 3070, 1080p)

| Metric | Target | Notes |
|--------|--------|-------|
| FPS | 90+ | Epic settings |
| Frame Time | <11ms | Consistent |
| VRAM | <7 GB | With streaming |
| Base Pass | <4ms | Material rendering |
| Post-Process | <2ms | All effects |

---

## Optimization Priorities

### Phase 1: Critical (Week 1)

**Texture Optimization**
- Compress all uncompressed textures
- Downscale 8K → 4K
- Enable texture streaming
- **Target:** -2.5 GB VRAM
- **Effort:** 8 hours

**Material Consolidation**
- Merge duplicate instances
- Remove unused materials
- **Target:** 303 → 250 instances
- **Effort:** 6 hours

### Phase 2: High (Week 2)

**Feature Optimization**
- Disable unused static switches
- Optimize shader code
- **Target:** -50 avg instructions
- **Effort:** 8 hours

**LOD Materials**
- Implement LOD switching
- Create simplified materials
- **Target:** -30% distant rendering
- **Effort:** 10 hours

### Phase 3: Medium (Week 3)

**Texture Atlasing**
- Create texture atlases
- Consolidate similar materials
- **Target:** -200 draw calls
- **Effort:** 12 hours

**Advanced Optimization**
- Material layering
- Virtual texturing
- **Target:** -500 MB VRAM
- **Effort:** 16 hours

---

## Monitoring and Validation

### Profiling Commands

**Shader Complexity:**
```
stat ShaderComplexity
```

**GPU Performance:**
```
stat GPU
stat RHI
profilegpu
```

**Memory Usage:**
```
stat Memory
stat Streaming
stat StreamingDetails
memreport
```

**Material Stats:**
```
stat Materials
stat SceneRendering
```

### Performance Metrics

**Check Weekly:**
- [ ] VRAM usage (target: <4 GB)
- [ ] FPS (target: 60+)
- [ ] Base pass time (target: <6ms)
- [ ] Material count (target: <200)
- [ ] Draw calls (target: <2500)

**Check Monthly:**
- [ ] Full performance audit
- [ ] Budget compliance
- [ ] Optimization effectiveness
- [ ] Hardware testing

---

## Quality vs Performance

### Quality Presets

**Low (Performance):**
- Simple materials only
- Minimal post-process
- Reduced texture resolution
- Target: 60 FPS on GTX 1660

**Medium (Balanced):**
- Standard materials
- Basic post-process
- Standard texture resolution
- Target: 60 FPS on RTX 3060

**High (Quality):**
- Complex materials allowed
- Full post-process
- High texture resolution
- Target: 60 FPS on RTX 3070

**Ultra (Maximum):**
- All features enabled
- Maximum quality
- Highest texture resolution
- Target: 60 FPS on RTX 4070+

---

## Budget Compliance

### Validation Checklist

**Per Material:**
- [ ] Instructions within budget
- [ ] Texture samples < 8
- [ ] Proper compression
- [ ] Streaming enabled
- [ ] LODs configured

**Project-Wide:**
- [ ] Total VRAM < 4 GB
- [ ] Material count < 200
- [ ] Draw calls < 2500
- [ ] FPS > 60
- [ ] Base pass < 6ms

### Enforcement

**Pre-Commit:**
```powershell
.\Scripts\Materials\ValidateMaterialBudget.ps1
```

**CI/CD:**
```yaml
- name: Validate Material Budget
  run: .\Scripts\Materials\ValidateMaterialBudget.ps1
  failOnError: true
```

---

## Related Documentation

- [MaterialOptimization.md](MaterialOptimization.md) - Optimization techniques
- [MaterialLibrary.md](MaterialLibrary.md) - Material catalog
- [PerformanceBudget.md](../Assets/PerformanceBudget.md) - Overall performance budget

---

**Document Owner:** Materials Team  
**Last Updated:** 2026-04-26  
**Next Review:** Weekly during optimization phase
