# M_Master_Surface - Surface Master Material

**Material Type:** Master Material  
**Domain:** Surface  
**Blend Mode:** Opaque  
**Shading Model:** Default Lit  
**Performance Budget:** 400-500 shader instructions

---

## Overview

M_Master_Surface is the primary master material for all opaque surfaces in HorrorProject. It provides a complete PBR workflow with support for standard texture maps, detail layers, and performance-optimized features.

---

## Material Graph Structure

### Base Color Network

```
TextureCoordinate → Multiply(Tiling) → TextureSample(BaseColor) → Multiply(Tint) → BaseColor Output
```

**Nodes:**
- TextureCoordinate (UV0)
- Scalar Parameter: `Tiling` (Default: 1.0, Range: 0.1-10.0)
- Texture2D Parameter: `T_BaseColor`
- Vector Parameter: `BaseColorTint` (Default: 1,1,1)

### Normal Map Network

```
TextureCoordinate → Multiply(Tiling) → TextureSample(Normal) → FlattenNormal → Normal Output
```

**Nodes:**
- TextureSample: `T_Normal` (Compression: TC_Normalmap)
- Static Switch: `UseNormalMap` (Default: True)
- FlattenNormal: Strength controlled by `NormalStrength` (Default: 1.0)

### ORM Network (Occlusion/Roughness/Metallic)

```
TextureCoordinate → TextureSample(ORM) → ComponentMask → Individual Outputs
```

**Channels:**
- R: Ambient Occlusion → AO Output
- G: Roughness → Multiply(RoughnessMultiplier) → Roughness Output
- B: Metallic → Multiply(MetallicMultiplier) → Metallic Output

### Emissive Network

```
TextureSample(Emissive) → Multiply(EmissiveColor) → Multiply(EmissiveStrength) → Emissive Output
```

**Nodes:**
- Texture2D Parameter: `T_Emissive` (Optional)
- Vector Parameter: `EmissiveColor` (Default: 1,1,1)
- Scalar Parameter: `EmissiveStrength` (Default: 0.0, Range: 0-100)
- Static Switch: `UseEmissive` (Default: False)

---

## Material Parameters

### Texture Parameters

| Parameter | Type | Default | Description |
|-----------|------|---------|-------------|
| T_BaseColor | Texture2D | None | Albedo/diffuse texture (sRGB) |
| T_Normal | Texture2D | FlatNormal | Normal map (Linear, BC5) |
| T_ORM | Texture2D | White | Packed ORM texture (Linear, BC7) |
| T_Emissive | Texture2D | Black | Emissive texture (sRGB) |

### Scalar Parameters

| Parameter | Default | Range | Description |
|-----------|---------|-------|-------------|
| Tiling | 1.0 | 0.1-10.0 | UV tiling multiplier |
| NormalStrength | 1.0 | 0.0-2.0 | Normal map intensity |
| RoughnessMultiplier | 1.0 | 0.0-1.0 | Roughness adjustment |
| MetallicMultiplier | 1.0 | 0.0-1.0 | Metallic adjustment |
| EmissiveStrength | 0.0 | 0.0-100.0 | Emissive intensity |
| AOStrength | 1.0 | 0.0-1.0 | Ambient occlusion intensity |

### Vector Parameters

| Parameter | Default | Description |
|-----------|---------|-------------|
| BaseColorTint | (1,1,1) | Color tint multiplier |
| EmissiveColor | (1,1,1) | Emissive color tint |

### Static Switch Parameters

| Parameter | Default | Description |
|-----------|---------|-------------|
| UseNormalMap | True | Enable normal mapping |
| UseEmissive | False | Enable emissive output |
| UseDetailLayer | False | Enable detail textures |
| UseVertexColor | False | Multiply base color by vertex color |

---

## Material Functions Used

### MF_Parallax (Optional)

**Purpose:** Parallax occlusion mapping for depth  
**Cost:** +80 instructions  
**Parameters:**
- HeightMap (Texture2D)
- ParallaxStrength (Scalar, 0.0-0.1)
- ParallaxSteps (Scalar, 8-32)

### MF_DetailLayer (Optional)

**Purpose:** Add detail textures for close-up viewing  
**Cost:** +60 instructions  
**Parameters:**
- DetailAlbedo (Texture2D)
- DetailNormal (Texture2D)
- DetailTiling (Scalar, 10-50)
- DetailStrength (Scalar, 0-1)

---

## Performance Analysis

### Shader Complexity

**Base Configuration:**
- Shader Instructions: 420
- Texture Samples: 3 (BaseColor, Normal, ORM)
- Math Operations: 25

**With All Features:**
- Shader Instructions: 580
- Texture Samples: 5 (+ Emissive, Detail)
- Math Operations: 45

### Performance Impact

| Configuration | Instructions | Samples | Performance |
|---------------|--------------|---------|-------------|
| Minimal (No Normal) | 280 | 1 | Excellent |
| Standard (Base+Normal+ORM) | 420 | 3 | Good |
| Full (All Features) | 580 | 5 | Moderate |

### Optimization Tips

1. **Disable unused features** via static switches
2. **Use ORM packed textures** instead of separate maps
3. **Avoid emissive** unless necessary
4. **Limit detail layers** to hero assets only
5. **Use lower tiling values** to reduce texture cache misses

---

## Material Instance Creation

### Standard Instance Setup

1. Create Material Instance from M_Master_Surface
2. Name using convention: `MI_[Category]_[Name]_[Variation]`
3. Set required textures (BaseColor, Normal, ORM)
4. Adjust parameters as needed
5. Test in-game lighting conditions

### Example Instances

**MI_Wall_Concrete_Dirty:**
```
T_BaseColor: T_Wall_Concrete_D
T_Normal: T_Wall_Concrete_N
T_ORM: T_Wall_Concrete_ORM
Tiling: 2.0
RoughnessMultiplier: 0.8
```

**MI_Metal_Rusty:**
```
T_BaseColor: T_Metal_Rusty_D
T_Normal: T_Metal_Rusty_N
T_ORM: T_Metal_Rusty_ORM
BaseColorTint: (0.9, 0.7, 0.6)
RoughnessMultiplier: 0.9
MetallicMultiplier: 0.6
```

**MI_Floor_Wood_Worn:**
```
T_BaseColor: T_Wood_Floor_D
T_Normal: T_Wood_Floor_N
T_ORM: T_Wood_Floor_ORM
Tiling: 1.5
NormalStrength: 0.8
RoughnessMultiplier: 0.7
```

---

## Usage Guidelines

### When to Use

- All opaque surfaces (walls, floors, props)
- Standard PBR materials
- Non-transparent objects
- Static and dynamic meshes

### When NOT to Use

- Transparent/translucent materials (use M_Master_Transparent)
- Emissive-heavy materials (use M_Master_Emissive)
- Post-process effects (use M_Master_PostProcess)
- UI elements (use M_Master_UI)

---

## Quality Levels

### LOD Material Switching

**LOD0 (0-15m):**
- Full quality, all features enabled
- Detail layers active
- High-res textures

**LOD1 (15-30m):**
- Disable detail layers
- Standard quality

**LOD2 (30-60m):**
- Simplified shading
- Lower texture mips

**LOD3 (60m+):**
- Minimal features
- Aggressive mip streaming

---

## Troubleshooting

### Common Issues

**Issue:** Material appears too dark
- Check BaseColorTint is (1,1,1)
- Verify lighting setup
- Check AO strength (reduce if too strong)

**Issue:** Normal map not visible
- Ensure UseNormalMap is enabled
- Check NormalStrength value
- Verify texture compression is TC_Normalmap

**Issue:** Material too shiny/rough
- Adjust RoughnessMultiplier
- Check ORM texture green channel
- Verify texture is linear color space

**Issue:** Performance problems
- Disable unused static switches
- Reduce texture resolution
- Remove detail layers
- Check shader complexity (stat ShaderComplexity)

---

## Technical Specifications

### Material Properties

```
Material Domain: Surface
Blend Mode: Opaque
Shading Model: Default Lit
Two Sided: False
Dithered LOD Transition: True
Use Material Attributes: False
```

### Texture Sampling

```
Sampler Type: Color
Mip Value Mode: Derivative
Texture Filtering: Trilinear
Texture Addressing: Wrap
```

### Performance Settings

```
Fully Rough: False (allows specular)
Use Lightmap Directionality: True
Use Full Precision: False
Use High Quality Normals: False (mobile optimization)
```

---

## Version History

**v1.0** - Initial implementation
- Basic PBR workflow
- ORM texture support
- Standard parameters

**v1.1** - Feature additions
- Detail layer support
- Parallax mapping option
- Vertex color support

**v1.2** - Performance optimization
- Static switch optimization
- Reduced instruction count
- Improved texture sampling

---

## Related Materials

- M_Master_Emissive - For glowing surfaces
- M_Master_Transparent - For glass/water
- M_Master_VHS - For VHS-affected surfaces
- MF_DetailLayer - Detail texture function
- MF_Parallax - Parallax mapping function

---

**Document Owner:** Materials Team  
**Last Updated:** 2026-04-26  
**Material Location:** Content/Materials/Masters/M_Master_Surface.uasset
