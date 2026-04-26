# Material Function Library - Implementation Guide

## Overview

This directory contains reusable material functions that can be used across all materials in the HorrorProject. These functions are optimized for performance and follow industry best practices.

## Material Functions

### MF_Parallax
**File:** `MF_Parallax.uasset`
**Purpose:** Parallax Occlusion Mapping for depth illusion

**Inputs:**
- HeightMap (Scalar) - Height texture input
- UV (Vector2) - Base UV coordinates
- HeightScale (Scalar) - Parallax depth scale
- MinSamples (Scalar) - Minimum ray samples
- MaxSamples (Scalar) - Maximum ray samples

**Outputs:**
- ParallaxUV (Vector2) - Offset UV coordinates
- ParallaxHeight (Scalar) - Final height value

**Quality Levels:**
- Low: 4 samples
- Medium: 8 samples
- High: 16 samples
- Epic: 32 samples

**Performance:**
- Instructions: ~50-150 (depends on sample count)
- Texture Samples: 1 + sample count

**Implementation:**
```
Algorithm: Parallax Occlusion Mapping (POM)
1. Calculate view direction in tangent space
2. Perform ray marching along view direction
3. Sample height map at each step
4. Find intersection point
5. Return offset UV coordinates
```

**Usage Example:**
```
Inputs:
- HeightMap: HeightTexture.R
- UV: TexCoord[0]
- HeightScale: 0.05
- MinSamples: 8
- MaxSamples: 16

Output:
- Connect ParallaxUV to all texture samples
```

---

### MF_DetailNormal
**File:** `MF_DetailNormal.uasset`
**Purpose:** Blend detail normal with base normal

**Inputs:**
- BaseNormal (Vector3) - Base normal map
- DetailNormal (Vector3) - Detail normal map
- DetailUV (Vector2) - Detail UV coordinates
- DetailIntensity (Scalar) - Blend strength
- DetailTiling (Scalar) - Detail texture tiling

**Outputs:**
- BlendedNormal (Vector3) - Combined normal

**Blend Modes:**
- Linear: Simple lerp blend
- Overlay: Reoriented normal mapping (RNM)
- UDN: Unreal Developer Network method

**Performance:**
- Instructions: ~20-30
- Texture Samples: 1 (detail normal)

**Implementation:**
```
Algorithm: Reoriented Normal Mapping (RNM)
1. Unpack base normal
2. Sample and unpack detail normal
3. Blend using RNM algorithm
4. Normalize result
5. Return blended normal
```

**Usage Example:**
```
Inputs:
- BaseNormal: NormalTexture
- DetailNormal: DetailNormalTexture
- DetailUV: TexCoord[0] * 10.0
- DetailIntensity: 0.5
- DetailTiling: 10.0

Output:
- Connect BlendedNormal to Normal input
```

---

### MF_Wetness
**File:** `MF_Wetness.uasset`
**Purpose:** Unified wetness effect for rain and water

**Inputs:**
- BaseColor (Vector3) - Dry base color
- Roughness (Scalar) - Dry roughness
- Normal (Vector3) - Surface normal
- WetnessAmount (Scalar) - Wetness intensity (0-1)
- WetnessColor (Vector3) - Wet color tint

**Outputs:**
- WetBaseColor (Vector3) - Modified base color
- WetRoughness (Scalar) - Modified roughness
- WetNormal (Vector3) - Modified normal
- Puddles (Scalar) - Puddle mask

**Effects:**
- Darkens base color
- Reduces roughness
- Adds water droplet normals
- Creates puddle areas

**Performance:**
- Instructions: ~40-60
- Texture Samples: 2 (droplet normal, puddle mask)

**Implementation:**
```
Algorithm: Wetness Simulation
1. Darken base color based on wetness
2. Reduce roughness (wet = smooth)
3. Add droplet normal details
4. Generate puddle mask from world position
5. Blend all effects based on wetness amount
```

**Usage Example:**
```
Inputs:
- BaseColor: BaseColorTexture
- Roughness: RoughnessTexture
- Normal: NormalTexture
- WetnessAmount: WetnessParameter (0-1)
- WetnessColor: (0.8, 0.8, 0.9)

Outputs:
- WetBaseColor -> BaseColor
- WetRoughness -> Roughness
- WetNormal -> Normal
```

---

### MF_TriplanarMapping
**File:** `MF_TriplanarMapping.uasset`
**Purpose:** World-space triplanar texture projection

**Inputs:**
- Texture (Texture2D) - Texture to project
- WorldPosition (Vector3) - World position
- WorldNormal (Vector3) - World normal
- Tiling (Scalar) - Texture tiling scale
- BlendSharpness (Scalar) - Blend transition sharpness

**Outputs:**
- TriplanarColor (Vector3) - Projected color
- TriplanarAlpha (Scalar) - Projected alpha

**Performance:**
- Instructions: ~60-80
- Texture Samples: 3 (X, Y, Z projections)

**Implementation:**
```
Algorithm: Triplanar Projection
1. Calculate blend weights from world normal
2. Sample texture from X, Y, Z planes
3. Apply world position as UV
4. Blend samples based on normal direction
5. Return blended result
```

**Usage Example:**
```
Inputs:
- Texture: T_Rock_BaseColor
- WorldPosition: Absolute World Position
- WorldNormal: Vertex Normal WS
- Tiling: 0.1
- BlendSharpness: 4.0

Output:
- Connect TriplanarColor to BaseColor
```

---

### MF_FresnelEffect
**File:** `MF_FresnelEffect.uasset`
**Purpose:** Optimized Fresnel calculation

**Inputs:**
- Normal (Vector3) - Surface normal
- Exponent (Scalar) - Fresnel power
- BaseReflectFraction (Scalar) - Base reflectivity
- Invert (Bool) - Invert fresnel

**Outputs:**
- FresnelValue (Scalar) - Fresnel result

**Performance:**
- Instructions: ~10-15
- Texture Samples: 0

**Implementation:**
```
Algorithm: Schlick's Approximation
1. Calculate view direction
2. Compute dot product with normal
3. Apply power function
4. Lerp with base reflectivity
5. Optional invert
```

**Usage Example:**
```
Inputs:
- Normal: Pixel Normal WS
- Exponent: 5.0
- BaseReflectFraction: 0.04
- Invert: False

Output:
- Use for rim lighting, glass, water
```

---

### MF_POMShadows
**File:** `MF_POMShadows.uasset`
**Purpose:** Self-shadowing for parallax mapping

**Inputs:**
- HeightMap (Scalar) - Height texture
- UV (Vector2) - UV coordinates
- LightDirection (Vector3) - Light direction in tangent space
- ShadowSoftness (Scalar) - Shadow blur amount
- ShadowIntensity (Scalar) - Shadow strength

**Outputs:**
- ShadowMask (Scalar) - Shadow occlusion value

**Performance:**
- Instructions: ~80-120
- Texture Samples: 8-16

**Implementation:**
```
Algorithm: Parallax Occlusion Shadow Mapping
1. Convert light direction to tangent space
2. Ray march from surface toward light
3. Sample height map along ray
4. Accumulate occlusion
5. Return shadow mask
```

**Usage Example:**
```
Inputs:
- HeightMap: HeightTexture.R
- UV: ParallaxUV (from MF_Parallax)
- LightDirection: Light Vector
- ShadowSoftness: 0.5
- ShadowIntensity: 0.8

Output:
- Multiply with BaseColor or AO
```

---

### MF_VertexAnimation
**File:** `MF_VertexAnimation.uasset`
**Purpose:** Wind and vertex animation effects

**Inputs:**
- WorldPosition (Vector3) - Vertex world position
- VertexColor (Vector4) - Vertex color (R = weight)
- WindSpeed (Scalar) - Animation speed
- WindStrength (Scalar) - Animation intensity
- WindDirection (Vector2) - Wind direction

**Outputs:**
- AnimatedPosition (Vector3) - Offset position
- AnimatedNormal (Vector3) - Offset normal

**Performance:**
- Instructions: ~30-40
- Texture Samples: 0

**Implementation:**
```
Algorithm: Sine Wave Vertex Animation
1. Use vertex color R as animation weight
2. Calculate wave based on world position + time
3. Apply wind direction and strength
4. Output position offset
5. Calculate normal offset
```

**Usage Example:**
```
Inputs:
- WorldPosition: Absolute World Position
- VertexColor: Vertex Color
- WindSpeed: 1.0
- WindStrength: 10.0
- WindDirection: (1.0, 0.0)

Output:
- Connect to World Position Offset
```

---

### MF_DistanceFade
**File:** `MF_DistanceFade.uasset`
**Purpose:** Distance-based effect fading

**Inputs:**
- FadeStartDistance (Scalar) - Start fade distance
- FadeEndDistance (Scalar) - End fade distance
- InvertFade (Bool) - Reverse fade direction

**Outputs:**
- FadeAlpha (Scalar) - Fade value (0-1)

**Performance:**
- Instructions: ~5-10
- Texture Samples: 0

**Implementation:**
```
Algorithm: Linear Distance Fade
1. Calculate camera distance
2. Remap to fade range
3. Clamp to 0-1
4. Optional invert
```

**Usage Example:**
```
Inputs:
- FadeStartDistance: 1000.0
- FadeEndDistance: 2000.0
- InvertFade: False

Output:
- Use for LOD transitions, detail fading
```

---

### MF_ColorGrading
**File:** `MF_ColorGrading.uasset`
**Purpose:** Per-material color grading

**Inputs:**
- InputColor (Vector3) - Original color
- Brightness (Scalar) - Brightness adjustment
- Contrast (Scalar) - Contrast adjustment
- Saturation (Scalar) - Saturation adjustment
- ColorTint (Vector3) - Color tint overlay

**Outputs:**
- GradedColor (Vector3) - Adjusted color

**Performance:**
- Instructions: ~20-30
- Texture Samples: 0

**Implementation:**
```
Algorithm: Standard Color Grading
1. Apply brightness offset
2. Apply contrast around midpoint
3. Adjust saturation
4. Apply color tint
5. Clamp result
```

**Usage Example:**
```
Inputs:
- InputColor: BaseColorTexture
- Brightness: 0.0
- Contrast: 1.0
- Saturation: 1.0
- ColorTint: (1.0, 1.0, 1.0)

Output:
- Connect to BaseColor
```

---

## Usage Guidelines

### When to Use Material Functions

**Use material functions when:**
- Logic is reused across multiple materials
- Complex calculations need optimization
- Standardization is required
- Quality scaling is needed

**Don't use material functions when:**
- Simple operations (single node)
- Material-specific logic
- Performance-critical unique code

### Performance Considerations

1. **Minimize function calls** - Each call has overhead
2. **Use static switches** - Enable/disable features
3. **Optimize texture samples** - Reuse samples when possible
4. **Avoid nested functions** - Flatten when possible
5. **Profile regularly** - Check instruction count

### Quality Scaling

Implement quality switches in functions:
```
StaticSwitchParameter: QualityLevel
- Low: Minimal features, <50 instructions
- Medium: Standard features, <100 instructions
- High: Enhanced features, <150 instructions
- Epic: All features, <200 instructions
```

### Testing Checklist

- [ ] Function compiles without errors
- [ ] Instruction count within target
- [ ] Texture samples optimized
- [ ] Quality levels implemented
- [ ] Documentation complete
- [ ] Example usage provided
- [ ] Performance tested in-game

## Creating New Material Functions

### Step 1: Plan
1. Define purpose and inputs/outputs
2. Set performance targets
3. Design algorithm
4. Plan quality levels

### Step 2: Implement
1. Create new Material Function asset
2. Add input/output nodes
3. Implement core logic
4. Add quality switches
5. Optimize instruction count

### Step 3: Test
1. Test in material editor
2. Check instruction count
3. Verify in Shader Complexity view
4. Profile in-game performance
5. Test all quality levels

### Step 4: Document
1. Add to this README
2. Document inputs/outputs
3. Provide usage examples
4. Note performance characteristics

## Naming Conventions

- Prefix: `MF_`
- Format: `MF_[FunctionName]`
- Examples: `MF_Parallax`, `MF_DetailNormal`

## Performance Targets

| Function Type | Instructions | Texture Samples |
|--------------|--------------|-----------------|
| Simple       | <20          | 0-1             |
| Standard     | <50          | 1-2             |
| Complex      | <100         | 2-4             |
| Advanced     | <150         | 4-8             |

## Common Patterns

### Pattern 1: Quality Switch
```
StaticSwitch (QualityLevel)
- Low: Simple calculation
- Medium: Standard calculation
- High: Enhanced calculation
- Epic: Full calculation
```

### Pattern 2: Optional Feature
```
StaticSwitch (UseFeature)
- True: Apply effect
- False: Pass through
```

### Pattern 3: Blend Mode
```
StaticSwitch (BlendMode)
- Mode1: Algorithm A
- Mode2: Algorithm B
- Mode3: Algorithm C
```

## Troubleshooting

### High Instruction Count
- Simplify math operations
- Remove redundant calculations
- Use cheaper approximations
- Add quality switches

### Texture Sample Issues
- Reuse texture samples
- Use texture packing
- Share textures across functions
- Cache sample results

### Compilation Errors
- Check input/output types
- Verify node connections
- Remove circular dependencies
- Update function references

## References

- UE5 Material Function Documentation
- Shader Optimization Guide
- PBR Material Guide
- Performance Best Practices

---

**Last Updated:** 2026-04-26
**Team:** SM13_Materials (Team 43)
**Project:** HorrorProject
