# Master Material Templates - Implementation Guide

## Overview

This directory contains master material templates that serve as the foundation for all materials in the HorrorProject. These templates implement industry-standard PBR workflows with built-in quality scaling and performance optimization.

## Master Materials

### M_Master_Opaque
**File:** `M_Master_Opaque.uasset`
**Blend Mode:** Opaque
**Shading Model:** Default Lit

**Purpose:**
Standard opaque materials for walls, floors, props, and most static meshes.

**Inputs:**
- BaseColor (RGB)
- Normal (RGB)
- Roughness (Scalar)
- Metallic (Scalar)
- AmbientOcclusion (Scalar)
- Emissive (RGB) - Optional
- OpacityMask (Scalar) - Not used in opaque

**Texture Maps:**
- T_BaseColor (sRGB)
- T_Normal (Linear, Normal Map)
- T_ORM (Linear) - Occlusion(R), Roughness(G), Metallic(B)
- T_Emissive (sRGB) - Optional

**Parameters:**
```
Textures:
- BaseColorTexture (Texture2D)
- NormalTexture (Texture2D)
- ORMTexture (Texture2D)
- EmissiveTexture (Texture2D)

Scalars:
- Tiling (Float, Default: 1.0)
- RoughnessMultiplier (Float, Default: 1.0)
- MetallicMultiplier (Float, Default: 1.0)
- EmissiveIntensity (Float, Default: 1.0)
- DetailNormalIntensity (Float, Default: 0.5)
- ParallaxHeight (Float, Default: 0.05)

Vectors:
- BaseColorTint (LinearColor, Default: White)
- EmissiveTint (LinearColor, Default: White)

Static Switches:
- UseDetailNormal (Bool, Default: False)
- UseParallax (Bool, Default: False)
- UseEmissive (Bool, Default: False)
- QualityLevel (Enum: Low/Medium/High/Epic, Default: High)
```

**Quality Levels:**
- **Low:** Base textures only, no detail normal, no parallax
- **Medium:** + Detail normal
- **High:** + Parallax mapping
- **Epic:** + All effects, highest quality

**Performance Target:**
- Instructions: <200
- Texture Samples: <12

**Implementation Notes:**
1. Use texture parameter references for easy instance creation
2. Implement quality switches using StaticSwitchParameter
3. Pack textures efficiently (ORM format)
4. Use material functions for complex operations

---

### M_Master_Masked
**File:** `M_Master_Masked.uasset`
**Blend Mode:** Masked
**Shading Model:** Default Lit

**Purpose:**
Materials with opacity mask for foliage, fences, decals, and cutout objects.

**Inherits from:** M_Master_Opaque features

**Additional Inputs:**
- OpacityMask (Scalar)

**Additional Parameters:**
```
Scalars:
- OpacityMaskClipValue (Float, Default: 0.333)
- DitheredLODFadeStart (Float, Default: 1000.0)
- DitheredLODFadeEnd (Float, Default: 1500.0)

Static Switches:
- UseDitheredLOD (Bool, Default: True)
- TwoSided (Bool, Default: False)
```

**Performance Target:**
- Instructions: <220
- Texture Samples: <14

**Implementation Notes:**
1. Use dithered LOD for smooth transitions
2. Enable two-sided rendering only when necessary
3. Optimize opacity mask for performance
4. Use simple alpha test when possible

---

### M_Master_Translucent
**File:** `M_Master_Translucent.uasset`
**Blend Mode:** Translucent
**Shading Model:** Default Lit

**Purpose:**
Glass, water surfaces, transparent objects, and particle effects.

**Inputs:**
- BaseColor (RGB)
- Normal (RGB)
- Roughness (Scalar)
- Metallic (Scalar)
- Opacity (Scalar)
- Refraction (Scalar) - Optional

**Parameters:**
```
Textures:
- BaseColorTexture (Texture2D)
- NormalTexture (Texture2D)
- OpacityTexture (Texture2D)

Scalars:
- Opacity (Float, Default: 0.5)
- RefractionStrength (Float, Default: 1.0)
- FresnelExponent (Float, Default: 5.0)
- FresnelIntensity (Float, Default: 1.0)

Vectors:
- BaseColorTint (LinearColor, Default: White)

Static Switches:
- UseRefraction (Bool, Default: False)
- UseFresnel (Bool, Default: True)
- QualityLevel (Enum: Low/Medium/High/Epic, Default: High)
```

**Quality Levels:**
- **Low:** Simple transparency, no refraction, no fresnel
- **Medium:** + Fresnel effects
- **High:** + Refraction
- **Epic:** + All effects, highest quality

**Performance Target:**
- Instructions: <250
- Texture Samples: <14

**Implementation Notes:**
1. Minimize overdraw by reducing transparent area
2. Use simple blend modes when possible
3. Avoid complex lighting calculations
4. Consider using masked blend mode instead

---

### M_Master_Water
**File:** `M_Master_Water.uasset`
**Blend Mode:** Translucent
**Shading Model:** Default Lit

**Purpose:**
Water surfaces, puddles, and liquid effects.

**Inputs:**
- BaseColor (RGB)
- Normal (RGB)
- Roughness (Scalar)
- Opacity (Scalar)
- WaveHeight (Scalar)

**Parameters:**
```
Textures:
- WaterNormalTexture (Texture2D)
- FoamTexture (Texture2D)
- CausticTexture (Texture2D)

Scalars:
- WaterDepth (Float, Default: 100.0)
- WaveSpeed (Float, Default: 0.1)
- WaveScale (Float, Default: 1.0)
- FoamIntensity (Float, Default: 1.0)
- CausticIntensity (Float, Default: 0.5)
- RefractionStrength (Float, Default: 1.0)

Vectors:
- WaterColor (LinearColor, Default: (0.0, 0.3, 0.5, 1.0))
- DeepWaterColor (LinearColor, Default: (0.0, 0.1, 0.2, 1.0))

Static Switches:
- UseFoam (Bool, Default: True)
- UseCaustics (Bool, Default: True)
- UseFlowMap (Bool, Default: False)
- QualityLevel (Enum: Low/Medium/High/Epic, Default: High)
```

**Quality Levels:**
- **Low:** Simple water color, basic normal
- **Medium:** + Foam effects
- **High:** + Caustics
- **Epic:** + Flow map, highest quality

**Performance Target:**
- Instructions: <280
- Texture Samples: <16

**Implementation Notes:**
1. Use panning normals for wave animation
2. Optimize caustics calculation
3. Use depth fade for shore blending
4. Consider using vertex animation for waves

---

## Creating Material Instances

### Step 1: Create Instance
1. Right-click master material in Content Browser
2. Select "Create Material Instance"
3. Name following convention: `MI_[Category]_[Name]`

### Step 2: Configure Parameters
1. Open material instance
2. Enable parameters you want to override
3. Set texture references
4. Adjust scalar and vector values
5. Set quality level switches

### Step 3: Optimize
1. Disable unused features via static switches
2. Use appropriate quality level
3. Test in Shader Complexity view
4. Verify instruction count

### Example: Creating a Brick Wall Material

```
Instance Name: MI_Architecture_BrickWall01
Parent: M_Master_Opaque

Parameters:
- BaseColorTexture: T_Brick_BaseColor
- NormalTexture: T_Brick_Normal
- ORMTexture: T_Brick_ORM
- Tiling: 2.0
- RoughnessMultiplier: 1.2
- UseDetailNormal: True
- DetailNormalIntensity: 0.3
- QualityLevel: High
```

## Naming Conventions

### Master Materials
- Prefix: `M_Master_`
- Format: `M_Master_[Type]`
- Examples: `M_Master_Opaque`, `M_Master_Masked`

### Material Instances
- Prefix: `MI_`
- Format: `MI_[Category]_[Name]`
- Examples: `MI_Architecture_Wall01`, `MI_Props_Chair01`

### Material Functions
- Prefix: `MF_`
- Format: `MF_[Function]`
- Examples: `MF_Parallax`, `MF_DetailNormal`

## Performance Guidelines

### Instruction Count Targets
- Opaque: <200 instructions
- Masked: <220 instructions
- Translucent: <250 instructions
- Water: <280 instructions
- PostProcess: <300 instructions

### Texture Sample Targets
- Low Quality: <8 samples
- Medium Quality: <12 samples
- High Quality: <14 samples
- Epic Quality: <16 samples

### Optimization Tips
1. Use texture packing (ORM format)
2. Minimize texture samples
3. Use material functions for reusable logic
4. Enable quality switches for scalability
5. Avoid complex math operations
6. Use LUT textures for complex calculations
7. Minimize dynamic branches
8. Use static switches instead of dynamic branches

## Testing and Validation

### Shader Complexity View
1. Open level in editor
2. Press `Alt+8` or use viewport menu
3. Select "Shader Complexity"
4. Green = Good, Yellow = OK, Red = Bad

### Material Stats
1. Open material editor
2. Window > Statistics
3. Check instruction count
4. Check texture sample count
5. Verify against targets

### Performance Profiling
```
# Console commands
stat shadercompiling
stat materials
stat textures
profilegpu
```

## Quality Level Configuration

### Project Settings
1. Edit > Project Settings
2. Engine > Rendering > Quality
3. Configure quality levels
4. Set material quality switch values

### Scalability Settings
```
[MaterialQualityLevel]
Low=0
Medium=1
High=2
Epic=3
```

## Troubleshooting

### High Instruction Count
- Remove unused nodes
- Simplify math operations
- Use material functions
- Reduce texture samples
- Lower quality level

### Texture Sampling Issues
- Use texture packing
- Reduce unique textures
- Share textures between materials
- Use texture atlases

### Performance Issues
- Check shader complexity view
- Profile with GPU profiler
- Reduce material complexity
- Use LOD materials
- Enable quality switches

## References

- UE5 Material Documentation
- PBR Texture Guide
- Material Optimization Guide
- Quality Scaling Best Practices

---

**Last Updated:** 2026-04-26
**Team:** SM13_Materials (Team 43)
**Project:** HorrorProject
