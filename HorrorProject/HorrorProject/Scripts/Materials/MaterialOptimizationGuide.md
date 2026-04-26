# Material System Optimization Guide - HorrorProject

## Analysis Summary

**Total Materials:** 310
- Master Materials: 49
- Material Instances: 193
- Material Functions: 67
- PostProcess Materials: 16

## Complexity Distribution

**Critical Issues:**
- **Very High Complexity (>100KB):** 160 materials ⚠️
- **High Complexity (50-100KB):** 17 materials ⚠️
- Medium Complexity (20-50KB): 13 materials
- Low Complexity (<20KB): 120 materials

**177 materials need optimization** (57% of total)

## Category Breakdown

- PostProcess: 16 (High Priority)
- Architecture: 55
- Lighting: 20
- Translucent: 86 (High Priority)
- Props: 10
- Character: 69

## Optimization Priorities

### 🔴 Priority 1: PostProcess Materials (16 materials)
PostProcess materials have the highest performance impact.

**Optimization Strategies:**
- Reduce SceneTexture sampling (target: <8 samples)
- Avoid complex loops and branches
- Use LUT textures instead of complex math
- Implement quality level switches
- Consider Compute Shader alternatives

**Target Performance:**
- Instructions: <300
- Texture Samples: <16
- Quality Levels: Low/Medium/High/Epic

### 🟡 Priority 2: Translucent Materials (86 materials)
Translucent materials cause overdraw issues.

**Optimization Strategies:**
- Use Masked blend mode instead of Translucent where possible
- Reduce translucent layer count
- Optimize sorting and render order
- Use simple blend modes
- Consider Dithered Opacity for distance fading

**Target Performance:**
- Instructions: <250
- Texture Samples: <14
- Minimize overdraw

### 🟠 Priority 3: Very High Complexity Materials (160 materials)
These materials are too complex and need immediate optimization.

**Optimization Strategies:**
- Remove redundant nodes and unused inputs
- Merge duplicate math operations
- Use Material Functions for reusable logic
- Split into multiple simpler materials
- Enable material quality switches
- Reduce texture sample count

**Target Performance:**
- Instructions: <300
- Texture Samples: <16
- Parameters: <32

## Master Material System

Create standardized master material templates in `Content/_SM13/Materials/Master/`:

### 1. M_Master_Opaque
**Purpose:** Standard opaque materials (walls, floors, props)

**Features:**
- BaseColor, Normal, Roughness, Metallic, AO
- Optional: Emissive, DetailNormal, Parallax
- Quality switches for Low/Medium/High/Epic
- Optimized for <200 instructions

**Parameters:**
- Texture maps (BaseColor, Normal, ORM)
- Tiling and offset
- Detail texture intensity
- Parallax height scale
- Quality level switch

### 2. M_Master_Masked
**Purpose:** Materials with opacity mask (foliage, fences)

**Features:**
- Inherits from Opaque features
- Optimized Opacity Mask
- Dithered LOD support
- Two-sided option

**Target:** <220 instructions

### 3. M_Master_Translucent
**Purpose:** Glass, water, transparent objects

**Features:**
- Simplified lighting model
- Optimized transparency calculation
- Optional refraction
- Fresnel effects

**Target:** <250 instructions

### 4. M_Master_Water
**Purpose:** Water surfaces and effects

**Features:**
- Optimized water surface
- Configurable waves and reflection
- Performance-friendly caustics
- Flow map support

**Target:** <280 instructions

## Material Function Library

Create reusable material functions in `Content/_SM13/Materials/Functions/`:

### MF_Parallax
- Configurable sample count
- Quality level support
- Height map input
- Optimized for performance

### MF_DetailNormal
- Optimized normal blending
- Configurable intensity
- Multiple blend modes

### MF_Wetness
- Unified wetness calculation
- Performance-optimized reflection
- Rain/water interaction

### MF_TriplanarMapping
- Optimized sampling strategy
- Configurable blending
- World-space projection

## Quality Level System

Implement quality scaling using Static Switch Parameters:

| Quality | Instructions | Texture Samples | Features |
|---------|-------------|-----------------|----------|
| Low     | <150        | <8              | Basic PBR only |
| Medium  | <200        | <12             | + Detail Normal |
| High    | <250        | <14             | + Parallax |
| Epic    | <300        | <16             | + All Effects |

**Implementation:**
1. Add StaticSwitchParameter "QualityLevel" to master materials
2. Use Quality Switch nodes to control features
3. Set quality level in material instances
4. Auto-switch via Scalability settings

## Optimization Workflow

### Phase 1: Audit (Completed ✓)
- ✓ Scan all material files
- ✓ Classify material types and usage
- ✓ Identify high complexity materials

### Phase 2: Create Master System
1. Create 4 master material templates
2. Implement quality level system
3. Create material function library
4. Test and validate

### Phase 3: Migrate Existing Materials
1. Convert existing materials to instances
2. Optimize high complexity materials
3. Standardize naming conventions
4. Update references

### Phase 4: Performance Validation
1. Use Shader Complexity view in editor
2. Test different quality levels
3. Performance profiling
4. Optimize bottlenecks

## UE5 Editor Commands

```
# Show shader complexity
viewmode shadercomplexity

# Show material instruction count
stat shadercompiling

# Show texture sampling
stat textures

# Recompile all materials
recompileshaders changed

# Show material stats
stat materials
```

## Performance Targets

| Material Type | Instructions | Texture Samples | Quality Levels |
|--------------|--------------|-----------------|----------------|
| Opaque       | <200         | <12             | Low/Med/High/Epic |
| Masked       | <220         | <14             | Low/Med/High/Epic |
| Translucent  | <250         | <14             | Low/Med/High/Epic |
| PostProcess  | <300         | <16             | Low/Med/High/Epic |

## Current Status vs Targets

**Current:**
- Very High Complexity: 160 materials (CRITICAL)
- High Complexity: 17 materials (WARNING)
- Medium Complexity: 13 materials (OK)
- Low Complexity: 120 materials (GOOD)

**Target:**
- Very High Complexity: 0 materials
- High Complexity: <10 materials
- Medium Complexity: <50 materials
- Low Complexity: >250 materials

## Next Steps

### Immediate Actions
1. ✓ Create material analysis tools
2. ✓ Generate optimization report
3. Create master material templates
4. Create material function library
5. Optimize top 10 most complex materials

### Short-term Goals (1-2 days)
1. Migrate all materials to new system
2. Implement quality level system
3. Performance testing and validation
4. Update material documentation

### Long-term Goals (1 week)
1. Complete material documentation
2. Material creation guidelines
3. Automated optimization tools
4. Continuous performance monitoring

## Tools Created

1. **AnalyzeShaders.ps1** - Analyzes all materials and generates complexity report
2. **MaterialOptimizationGuide.md** - This comprehensive optimization guide
3. **Directory Structure** - Standardized material organization

## Directory Structure

```
Content/_SM13/Materials/
├── Master/
│   ├── M_Master_Opaque.uasset
│   ├── M_Master_Masked.uasset
│   ├── M_Master_Translucent.uasset
│   └── M_Master_Water.uasset
├── Functions/
│   ├── MF_Parallax.uasset
│   ├── MF_DetailNormal.uasset
│   ├── MF_Wetness.uasset
│   └── MF_TriplanarMapping.uasset
└── Instances/
    └── (Material instances organized by category)
```

## References

- UE5 Material Best Practices
- GPU Performance Optimization Guide
- Shader Complexity Guidelines
- Material Quality Scaling Documentation

---

**Report Generated:** 2026-04-26
**Project:** HorrorProject
**Team:** SM13_Materials (Team 43)
