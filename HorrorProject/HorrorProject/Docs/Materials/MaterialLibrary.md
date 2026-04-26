# Material Library - HorrorProject

**Document Type:** Asset Catalog  
**Last Updated:** 2026-04-26  
**Total Materials:** Master (4) + Functions (9) + Instances (320+)

---

## Overview

Complete catalog of all materials in HorrorProject, organized by type and usage. This document serves as the central reference for material assets.

---

## Master Materials

### Surface Materials

| Material | Domain | Blend Mode | Instructions | Samples | Status |
|----------|--------|------------|--------------|---------|--------|
| M_Master_Surface | Surface | Opaque | 420 | 3 | ✅ Production |

**Location:** `Content/Materials/Masters/M_Master_Surface.uasset`  
**Documentation:** [M_Master_Surface.md](M_Master_Surface.md)  
**Use Cases:** Walls, floors, props, architecture

### Post-Process Materials

| Material | Domain | Blend Mode | Instructions | Samples | Status |
|----------|--------|------------|--------------|---------|--------|
| M_Master_VHS | Post Process | Composite | 320-385 | 3-4 | ✅ Production |
| M_Master_PostProcess | Post Process | Composite | 220-380 | 2-5 | ✅ Production |

**VHS Location:** `Content/Bodycam_VHS_Effect/Materials/Masters/PostProcess/M_OldVHS.uasset`  
**PostProcess Location:** `Content/Materials/Masters/M_Master_PostProcess.uasset`  
**Documentation:** [M_Master_VHS.md](M_Master_VHS.md), [M_Master_PostProcess.md](M_Master_PostProcess.md)

### UI Materials

| Material | Domain | Blend Mode | Instructions | Samples | Status |
|----------|--------|------------|--------------|---------|--------|
| M_Master_UI | User Interface | Translucent | 120 | 1-2 | ✅ Production |

**Location:** `Content/Materials/Masters/M_Master_UI.uasset`  
**Documentation:** [M_Master_UI.md](M_Master_UI.md)

---

## Material Functions

### VHS/Post-Process Functions

| Function | Category | Cost (inst) | Samples | Documentation |
|----------|----------|-------------|---------|---------------|
| MF_Chromatic_Aberration | Post Process | 45 | 3 | [Link](MF_Chromatic_Aberration.md) |
| MF_VHS_Noise | Post Process | 60 | 2 | [Link](MF_VHS_Noise.md) |
| MF_Scanlines | Post Process | 30 | 0 | [Link](MF_Scanlines.md) |
| MF_Vignette | Post Process | 25 | 0 | [Link](MF_Vignette.md) |
| MF_Film_Grain | Post Process | 40 | 1 | [Link](MF_Film_Grain.md) |
| MF_Color_Grading | Post Process | 35 | 0 | [Link](MF_Color_Grading.md) |
| MF_Distortion | Post Process | 50 | 0 | [Link](MF_Distortion.md) |
| MF_Blur | Post Process | 80-150 | 9-81 | [Link](MF_Blur.md) |
| MF_Glow | Post Process | 100 | 10-20 | [Link](MF_Glow.md) |

**Location:** `Content/Materials/Functions/`

---

## Material Instances by Category

### Environment - Walls

| Instance | Master | Textures | Use Case | Status |
|----------|--------|----------|----------|--------|
| MI_Wall_Concrete_01 | M_Master_Surface | 2K | Industrial walls | ✅ |
| MI_Wall_Concrete_Dirty | M_Master_Surface | 2K | Weathered walls | ✅ |
| MI_Wall_Metal_Panels | M_Master_Surface | 2K | Metal walls | ✅ |
| MI_Wall_Brick_Red | M_Master_Surface | 2K | Brick walls | ✅ |

**Location:** `Content/Materials/Instances/Environment/Walls/`

### Environment - Floors

| Instance | Master | Textures | Use Case | Status |
|----------|--------|----------|----------|--------|
| MI_Floor_Concrete | M_Master_Surface | 2K | Concrete floors | ✅ |
| MI_Floor_Metal_Grate | M_Master_Surface | 2K | Metal grating | ✅ |
| MI_Floor_Wood_Worn | M_Master_Surface | 2K | Wooden floors | ✅ |
| MI_Floor_Tile_Dirty | M_Master_Surface | 1K | Tile floors | ✅ |

**Location:** `Content/Materials/Instances/Environment/Floors/`

### Props - Industrial

| Instance | Master | Textures | Use Case | Status |
|----------|--------|----------|----------|--------|
| MI_Metal_Rusty | M_Master_Surface | 1K | Rusty metal props | ✅ |
| MI_Metal_Clean | M_Master_Surface | 1K | Clean metal | ✅ |
| MI_Plastic_Matte | M_Master_Surface | 512 | Plastic objects | ✅ |
| MI_Rubber_Black | M_Master_Surface | 512 | Rubber materials | ✅ |

**Location:** `Content/Materials/Instances/Props/Industrial/`

### VHS Effects

| Instance | Master | Parameters | Use Case | Status |
|----------|--------|------------|----------|--------|
| MI_VHS_Standard | M_Master_VHS | Balanced | Standard bodycam | ✅ |
| MI_VHS_Horror | M_Master_VHS | Intense | Horror sequences | ✅ |
| MI_VHS_Subtle | M_Master_VHS | Light | Subtle effect | ✅ |
| MI_VHS_Glitch | M_Master_VHS | Extreme | Glitch events | ✅ |

**Location:** `Content/Bodycam_VHS_Effect/Materials/Instances/`

### UI Elements

| Instance | Master | Use Case | Status |
|----------|--------|----------|--------|
| MI_UI_Button_Standard | M_Master_UI | Menu buttons | ✅ |
| MI_UI_HealthBar | M_Master_UI | Health indicator | ✅ |
| MI_UI_Icon_Base | M_Master_UI | UI icons | ✅ |
| MI_UI_Text_Standard | M_Master_UI | Text rendering | ✅ |

**Location:** `Content/Materials/Instances/UI/`

---

## Material Usage Statistics

### By Category

| Category | Instance Count | VRAM Usage | Avg Instructions |
|----------|----------------|------------|------------------|
| Environment | 120 | 3.5 GB | 380 |
| Props | 80 | 1.2 GB | 350 |
| Characters | 40 | 800 MB | 450 |
| VFX | 30 | 200 MB | 280 |
| UI | 25 | 150 MB | 120 |
| Post-Process | 8 | 50 MB | 300 |
| **Total** | **303** | **5.9 GB** | **363** |

### Performance Distribution

| Complexity | Count | Percentage | Target |
|------------|-------|------------|--------|
| Simple (<200 inst) | 45 | 15% | 30% |
| Standard (200-400 inst) | 180 | 59% | 50% |
| Complex (400-600 inst) | 65 | 21% | 15% |
| Heavy (>600 inst) | 13 | 5% | 5% |

**Status:** ⚠️ Need more simple materials, reduce complex materials

---

## Material Naming Convention

### Format

```
[Prefix]_[Category]_[Name]_[Variation]
```

### Prefixes

- `M_` - Master Material
- `MI_` - Material Instance
- `MF_` - Material Function
- `MPC_` - Material Parameter Collection

### Examples

```
M_Master_Surface
MI_Wall_Concrete_Dirty_01
MF_Chromatic_Aberration
MPC_Global_Environment
```

**Full Convention:** See [NamingConvention.md](../Assets/NamingConvention.md)

---

## Material Creation Workflow

### 1. Check Existing Materials

Before creating new material:
- Search library for similar materials
- Check if existing instance can be reused
- Verify no duplicates exist

### 2. Select Master Material

Choose appropriate master:
- Surface → M_Master_Surface
- VHS Effect → M_Master_VHS
- Screen Effect → M_Master_PostProcess
- UI → M_Master_UI

### 3. Create Instance

Follow guide: [MaterialInstanceCreation.md](MaterialInstanceCreation.md)

### 4. Configure Parameters

Reference: [MaterialParameterGuide.md](MaterialParameterGuide.md)

### 5. Optimize

Follow: [MaterialOptimization.md](MaterialOptimization.md)

### 6. Document

Add to this library with:
- Name and location
- Master material
- Parameter values
- Use case
- Performance metrics

---

## Material Performance Budget

### Per-Material Budgets

| Type | Instructions | Samples | VRAM |
|------|--------------|---------|------|
| Simple Surface | <200 | 1-2 | <10 MB |
| Standard Surface | 200-400 | 3-5 | 10-20 MB |
| Complex Surface | 400-600 | 5-7 | 20-30 MB |
| Post Process | 200-400 | 2-5 | 5-10 MB |
| UI | <150 | 1-2 | <5 MB |

### Project Totals

| Metric | Current | Target | Status |
|--------|---------|--------|--------|
| Total VRAM | 5.9 GB | 4.0 GB | ⚠️ Over |
| Avg Instructions | 363 | 300 | ⚠️ Over |
| Material Count | 303 | 200 | ⚠️ Over |
| Draw Calls | 3500+ | 2500 | ⚠️ Over |

**Action Required:** Consolidation and optimization needed

---

## Optimization Priorities

### Critical (P0)

1. **Texture Compression**
   - Compress all uncompressed textures
   - Target: -2 GB VRAM
   - Timeline: Week 1

2. **Material Consolidation**
   - Merge duplicate instances
   - Target: 303 → 200 instances
   - Timeline: Week 1-2

3. **Resolution Reduction**
   - Downscale 8K → 4K textures
   - Target: -1.5 GB VRAM
   - Timeline: Week 1

### High (P1)

4. **LOD Materials**
   - Implement LOD switching
   - Target: -30% instructions
   - Timeline: Week 2

5. **Feature Optimization**
   - Disable unused features
   - Target: -50 avg instructions
   - Timeline: Week 2

### Medium (P2)

6. **Texture Atlasing**
   - Create texture atlases
   - Target: -20% draw calls
   - Timeline: Week 3

---

## Material Audit Results

### Issues Found

**Texture Issues:**
- 45 textures at 8K resolution (should be 4K)
- 120 textures uncompressed (should be BC7/BC5)
- 80 textures not streaming (should stream)

**Material Issues:**
- 67 duplicate or near-duplicate instances
- 13 materials over 600 instructions
- 25 materials with unused features enabled

**Performance Issues:**
- Total VRAM 47% over budget
- Average instructions 21% over target
- Material count 51% over target

### Recommendations

1. Immediate texture optimization
2. Material instance consolidation
3. Feature optimization pass
4. LOD material implementation
5. Regular performance monitoring

---

## Material Templates

### Standard Surface Template

```
Master: M_Master_Surface
Textures:
  - T_BaseColor (2K, BC7, sRGB)
  - T_Normal (2K, BC5, Linear)
  - T_ORM (2K, BC7, Linear)
Parameters:
  - Tiling: 1.0-2.0
  - NormalStrength: 0.8-1.0
  - RoughnessMultiplier: 0.6-0.8
Static Switches:
  - UseNormalMap: True
  - UseEmissive: False
```

### VHS Effect Template

```
Master: M_Master_VHS
Parameters:
  - EffectIntensity: 0.8-1.0
  - ChromaticStrength: 0.002-0.004
  - NoiseStrength: 0.12-0.18
  - ScanlineIntensity: 0.25-0.35
Static Switches:
  - All enabled for standard quality
```

---

## Maintenance Schedule

### Daily

- Monitor new material additions
- Check for naming compliance
- Verify no duplicates created

### Weekly

- Performance profiling
- VRAM usage check
- Material count review
- Update documentation

### Monthly

- Full material audit
- Optimization review
- Budget compliance check
- Library cleanup

---

## Related Documentation

- [MaterialInstanceCreation.md](MaterialInstanceCreation.md) - Creation guide
- [MaterialParameterGuide.md](MaterialParameterGuide.md) - Parameter reference
- [MaterialOptimization.md](MaterialOptimization.md) - Optimization guide
- [MaterialNamingConvention.md](MaterialNamingConvention.md) - Naming standards
- [MaterialPerformanceBudget.md](MaterialPerformanceBudget.md) - Performance targets

---

**Document Owner:** Materials Team  
**Last Updated:** 2026-04-26  
**Next Review:** Weekly
