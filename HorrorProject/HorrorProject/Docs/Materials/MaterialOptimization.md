# Material Optimization Guide

**Document Type:** Technical Guide  
**Target Audience:** Technical Artists, Programmers  
**Last Updated:** 2026-04-26

---

## Overview

Comprehensive guide for optimizing materials in HorrorProject to meet performance budgets while maintaining visual quality. Covers shader optimization, texture management, and runtime performance.

---

## Performance Budget

### Target Metrics

| Material Type | Max Instructions | Max Samples | Target FPS Impact |
|---------------|------------------|-------------|-------------------|
| Simple Surface | 200 | 2 | <0.1ms |
| Standard Surface | 400 | 3-5 | <0.2ms |
| Complex Surface | 600 | 5-7 | <0.3ms |
| Post Process | 300 | 3-4 | <0.5ms |
| UI | 150 | 1-2 | <0.1ms |

### Current Status (from Performance Budget)

- **GPU Total:** 18-20ms (Target: 14ms) ⚠️ Over
- **Base Pass:** 7-9ms (Target: 4-6ms) ⚠️ Over
- **Material Budget:** 256 MB (Current: 500 MB) ⚠️ Over

**Priority:** Critical optimization required

---

## Shader Optimization

### Instruction Count Reduction

#### 1. Disable Unused Features

**Impact:** -50 to -200 instructions per feature

```
Static Switches to Review:
- UseNormalMap: Disable for flat surfaces (-40 inst)
- UseEmissive: Disable if no glow needed (-30 inst)
- UseDetailLayer: Disable for distant objects (-60 inst)
- UseParallax: Disable unless hero asset (-80 inst)
```

**Example:**
```
Before: All features enabled = 580 instructions
After: Only Base + Normal = 420 instructions
Savings: 160 instructions (27% reduction)
```### 2. Simplify Math Operations

**Impact:** -5 to -20 instructions

**Avoid:**
- Complex trigonometry (sin, cos, tan)
- Division operations (use multiply by reciprocal)
- Branching (if statements)
- Unnecessary lerps

**Prefer:**
- Simple multiply/add operations
- Pre-calculated constants
- Lookup tables for complex functions

#### 3. Optimize Texture Sampling

**Impact:** -10 instructions per sample removed

**Strategies:**
- Use packed textures (ORM instead of separate R, M, AO)
- Share texture samples across multiple outputs
- Use lower mip levels for distant objects
- Disable texture sampling for invisible surfaces

**Example:**
```
Before: 
- T_Roughness (1 sample)
- T_Metallic (1 sample)
- T_AO (1 sample)
Total: 3 samples

After:
- T_ORM (1 sample, 3 channels)
Total: 1 sample

Savings: 2 samples, ~20 instructions
```

---

## Texture Optimization

### Texture Resolution

#### Resolution Guidelines

| Asset Type | Current | Target | Reduction |
|------------|---------|--------|-----------|
| Environment (8K) | 8K | 4K | 75% memory |
| Hero Props | 4K | 2K | 75% memory |
| Standard Props | 2K | 1K | 75% memory |
| Background | 1K | 512 | 75% memory |

#### Optimization Strategy

**Phase 1: Immediate (Week 1)**
- Downscale all 8K textures to 4K
- Compress uncompressed textures
- Enable texture streaming

**Phase 2: Short-term (Week 2)**
- Downscale 4K to 2K for non-hero assets
- Implement LOD texture switching
- Remove unused textures

**Phase 3: Long-term (Week 3+)**
- Create texture atlases
- Implement virtual texturing
- Optimize texture streaming

### Texture Compression

#### Compression Settings

| Map Type | Current | Target | Savings |
|----------|---------|--------|---------|
| Albedo | Uncompressed | BC7 | 75% |
| Normal | BC7 | BC5 | 33% |
| Roughness | Uncompressed | BC4 | 87% |
| Metallic | Uncompressed | BC4 | 87% |

#### Implementation

**Albedo/Diffuse:**
```
Compression: BC7
sRGB: Enabled
Mip Gen Settings: SimpleAverage
Never Stream: False
```

**Normal Maps:**
```
Compression: BC5 (Normalmap)
sRGB: Disabled
Mip Gen Settings: Blur5
Never Stream: False
```

**ORM Packed:**
```
Compression: BC7
sRGB: Disabled
Mip Gen Settings: SimpleAverage
Never Stream: False
```

**Single Channel (R, M, AO):**
```
Compression: BC4
sRGB: Disabled
Mip Gen Settings: SimpleAverage
Never Stream: False
```

### Texture Streaming

**Enable Streaming:**
```cpp
// In texture settings
Never Stream: False
LOD Group: World
Streaming Priority: 1.0
```

**Streaming Budget:**
```
Target: 4-5 GB VRAM
Current: 10-12 GB VRAM
Reduction Needed: 50-60%
```

---

## Material Instance Optimization

### Instance Consolidation

**Current Problem:**
- 320+ material instances
- Many duplicates or near-duplicates
- Excessive draw calls

**Target:**
- 150 unique instances
- Shared materials where possible
- Reduced draw calls

#### Consolidation Strategy

**Step 1: Identify Duplicates**
```
Find materials with:
- Same textures
- Similar parameters (within 10%)
- Same master material
```

**Step 2: Merge Similar Instances**
```
Example:
- MI_Wall_Concrete_01 (Tiling: 2.0)
- MI_Wall_Concrete_02 (Tiling: 2.1)
- MI_Wall_Concrete_03 (Tiling: 1.9)

Merge to:
- MI_Wall_Concrete (Tiling: 2.0)
```

**Step 3: Use Material Parameter Collections**
```cpp
// Global material parameters
MPC_Global_Environment:
- GlobalTiling
- GlobalRoughness
- GlobalBrightness
```

### LOD Material Switching

**Implementation:**
```cpp
// In static mesh LOD settings
LOD0 (0-15m): MI_Detailed (600 inst, 7 samples)
LOD1 (15-30m): MI_Standard (400 inst, 5 samples)
LOD2 (30-60m): MI_Simple (200 inst, 3 samples)
LOD3 (60m+): MI_Minimal (100 inst, 1 sample)
```

**Savings:**
- Average instruction count: -50%
- Texture samples: -40%
- GPU time: -30%

---

## Runtime Optimization

### Dynamic Material Instances

**Problem:** Creating too many dynamic instances

**Solution:** Reuse and pool instances

```cpp
// Bad: Create new instance every time
UMaterialInstanceDynamic* NewMat = UMaterialInstanceDynamic::Create(BaseMat, this);

// Good: Reuse existing instance
if (!CachedDynamicMaterial)
{
    CachedDynamicMaterial = UMaterialInstanceDynamic::Create(BaseMat, this);
}
// Reuse CachedDynamicMaterial
```

### Parameter Updates

**Minimize Updates:**
```cpp
// Bad: Update every frame
void Tick(float DeltaTime)
{
    Material->SetScalarParameterValue("Time", GetWorld()->GetTimeSeconds());
}

// Good: Update only when needed
void OnValueChanged(float NewValue)
{
    if (FMath::Abs(NewValue - CachedValue) > Threshold)
    {
        Material->SetScalarParameterValue("Value", NewValue);
        CachedValue = NewValue;
    }
}
```

### Batch Parameter Changes

```cpp
// Bad: Multiple individual updates
Material->SetScalarParameterValue("Param1", Value1);
Material->SetScalarParameterValue("Param2", Value2);
Material->SetScalarParameterValue("Param3", Value3);

// Good: Batch update (if possible)
TArray<FMaterialParameterInfo> Params;
TArray<float> Values;
// ... populate arrays ...
Material->SetScalarParameterValues(Params, Values);
```

---

## Post-Process Optimization

### VHS Effect Optimization

**Current Cost:** 385 instructions (all features)  
**Target Cost:** 250 instructions

**Optimization:**

**Quality Presets:**
```
Low (180 inst):
- Disable Chromatic Aberration
- Disable Distortion
- Reduce Noise quality
- Static scanlines

Medium (250 inst):
- Enable Chromatic (reduced)
- Enable Noise (reduced)
- Enable Scanlines
- Disable Distortion

High (320 inst):
- Enable most features
- Reduce quality slightly
- Optimize shader code
```

**Dynamic Quality Scaling:**
```cpp
// Adjust quality based on performance
float CurrentFPS = 1.0f / DeltaTime;
if (CurrentFPS < 50.0f)
{
    // Drop to low quality
    VHSMaterial->SetStaticSwitchParameterValue("EnableChromatic", false);
    VHSMaterial->SetStaticSwitchParameterValue("EnableDistortion", false);
}
```

### Post-Process Volume Optimization

**Reduce Overlapping Volumes:**
```
Current: 15+ post-process volumes
Target: 5-8 volumes
Strategy: Merge similar volumes, use priority system
```

**Optimize Blend Weights:**
```cpp
// Avoid unnecessary blending
if (BlendWeight < 0.01f)
{
    // Skip this volume
    continue;
}
```

---

## Profiling and Measurement

### Shader Complexity Visualization

**Console Commands:**
```
stat ShaderComplexity
stat RHI
stat SceneRendering
profilegpu
```

**Interpretation:**
- Green: <200 instructions (good)
- Yellow: 200-400 instructions (acceptable)
- Red: 400-600 instructions (high)
- Pink: >600 instructions (critical)

### Performance Profiling

**GPU Profiling:**
```
profilegpu

Look for:
- BasePass time (target: <6ms)
- Material shader time
- Texture sampling time
```

**Material Stats:**
```
stat Materials

Shows:
- Material count
- Shader compilation
- Parameter updates
```

### Texture Memory Profiling

**Console Commands:**
```
stat Streaming
stat StreamingDetails
memreport

Check:
- Texture pool size
- Streaming texture memory
- Non-streaming texture memory
```

---

## Optimization Checklist

### Per-Material Checklist

- [ ] Shader instructions < budget
- [ ] Texture samples < 8
- [ ] Unused features disabled
- [ ] Proper texture compression
- [ ] Textures set to stream
- [ ] LOD materials configured
- [ ] Tested on target hardware

### Project-Wide Checklist

- [ ] Material instances consolidated
- [ ] Duplicate materials removed
- [ ] Texture resolutions optimized
- [ ] All textures compressed
- [ ] Streaming enabled
- [ ] Performance profiled
- [ ] Budget compliance verified

---

## Optimization Results

### Expected Improvements

**Texture Memory:**
```
Before: 10-12 GB VRAM
After: 4-5 GB VRAM
Reduction: 50-60%
```

**Shader Performance:**
```
Before: 7-9ms base pass
After: 4-6ms base pass
Improvement: 30-40%
```

**Material Count:**
```
Before: 320+ instances
After: 150 instances
Reduction: 53%
```

**Overall FPS:**
```
Before: 45-50 FPS
After: 60+ FPS
Improvement: 20-30%
```

---

## Advanced Techniques

### Material Layering

**Use Material Layers for Reusability:**
```
Base Layer: Common PBR functionality
Detail Layer: Surface-specific details
Blend Layer: Weathering, dirt, damage
```

**Benefits:**
- Reduced duplication
- Easier maintenance
- Better performance

### Virtual Texturing

**For Large Environments:**
```
Enable Runtime Virtual Texturing (RVT):
- Reduces memory usage
- Improves streaming
- Better texture quality at distance
```

### Nanite Materials

**For High-Poly Assets:**
```
Use Nanite-compatible materials:
- Simplified shading
- Automatic LOD
- Better performance
```

---

## Common Optimization Mistakes

### ❌ Over-Optimization

**Problem:** Removing too much detail  
**Solution:** Balance quality and performance

### ❌ Premature Optimization

**Problem:** Optimizing before profiling  
**Solution:** Profile first, optimize bottlenecks

### ❌ Ignoring LODs

**Problem:** Same material quality at all distances  
**Solution:** Implement LOD material switching

### ❌ Texture Bloat

**Problem:** Using maximum resolution everywhere  
**Solution:** Match resolution to screen size

---

## Monitoring and Maintenance

### Weekly Performance Review

1. Run profiling commands
2. Check VRAM usage
3. Verify FPS targets
4. Review new materials
5. Update optimization plan

### Monthly Audit

1. Full performance audit
2. Material instance review
3. Texture memory analysis
4. Optimization effectiveness
5. Budget compliance check

---

## Related Documentation

- [PerformanceBudget.md](../Assets/PerformanceBudget.md) - Overall performance targets
- [MaterialParameterGuide.md](MaterialParameterGuide.md) - Parameter reference
- [MaterialInstanceCreation.md](MaterialInstanceCreation.md) - Instance creation guide
- [MaterialLibrary.md](MaterialLibrary.md) - Material catalog

---

**Document Owner:** Materials Team  
**Last Updated:** 2026-04-26  
**Next Review:** Weekly during optimization phase
