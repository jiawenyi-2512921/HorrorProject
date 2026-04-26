# Material Parameter Guide

**Document Type:** Technical Reference  
**Target Audience:** Artists, Technical Artists, Programmers  
**Last Updated:** 2026-04-26

---

## Overview

Comprehensive reference for all material parameters in HorrorProject master materials. Includes parameter types, ranges, defaults, and usage guidelines.

---

## Parameter Types

### Texture Parameters

**Type:** Texture2D  
**Usage:** Assign texture assets  
**Performance:** Each sample costs ~10 instructions

### Scalar Parameters

**Type:** Float  
**Usage:** Numeric values (intensity, multipliers, etc.)  
**Performance:** Minimal cost (~1 instruction per operation)

### Vector Parameters

**Type:** LinearColor (RGBA) or Vector3 (RGB)  
**Usage:** Colors, directions, multi-value data  
**Performance:** Minimal cost (~3 instructions per operation)

### Static Switch Parameters

**Type:** Boolean  
**Usage:** Enable/disable features  
**Performance:** Changes shader compilation, no runtime cost

---

## M_Master_Surface Parameters

### Texture Parameters

| Parameter | Type | Default | Compression | sRGB | Description |
|-----------|------|---------|-------------|------|-------------|
| T_BaseColor | Texture2D | None | BC7 | Yes | Albedo/diffuse map |
| T_Normal | Texture2D | FlatNormal | BC5 | No | Normal map |
| T_ORM | Texture2D | White | BC7 | No | Occlusion/Roughness/Metallic |
| T_Emissive | Texture2D | Black | BC7 | Yes | Emissive/glow map |
| T_DetailAlbedo | Texture2D | None | BC7 | Yes | Detail diffuse (optional) |
| T_DetailNormal | Texture2D | None | BC5 | No | Detail normal (optional) |

### Scalar Parameters

| Parameter | Default | Min | Max | Step | Description |
|-----------|---------|-----|-----|------|-------------|
| Tiling | 1.0 | 0.1 | 10.0 | 0.1 | UV coordinate scale |
| NormalStrength | 1.0 | 0.0 | 2.0 | 0.1 | Normal map intensity |
| RoughnessMultiplier | 1.0 | 0.0 | 1.0 | 0.05 | Roughness adjustment |
| MetallicMultiplier | 1.0 | 0.0 | 1.0 | 0.05 | Metallic adjustment |
| EmissiveStrength | 0.0 | 0.0 | 100.0 | 1.0 | Emissive intensity |
| AOStrength | 1.0 | 0.0 | 1.0 | 0.1 | Ambient occlusion intensity |
| DetailTiling | 10.0 | 5.0 | 50.0 | 5.0 | Detail texture scale |
| DetailStrength | 0.5 | 0.0 | 1.0 | 0.1 | Detail layer blend |

### Vector Parameters

| Parameter | Default | Range | Description |
|-----------|---------|-------|-------------|
| BaseColorTint | (1,1,1) | 0-1 per channel | Color tint multiplier |
| EmissiveColor | (1,1,1) | 0-1 per channel | Emissive color tint |

### Static Switches

| Parameter | Default | Cost | Description |
|-----------|---------|------|-------------|
| UseNormalMap | True | +40 inst | Enable normal mapping |
| UseEmissive | False | +30 inst | Enable emissive output |
| UseDetailLayer | False | +60 inst | Enable detail textures |
| UseVertexColor | False | +10 inst | Multiply by vertex color |
| UseParallax | False | +80 inst | Enable parallax occlusion |

---

## M_Master_VHS Parameters

### Scalar Parameters - Intensity

| Parameter | Default | Min | Max | Step | Description |
|-----------|---------|-----|-----|------|-------------|
| EffectIntensity | 1.0 | 0.0 | 1.0 | 0.05 | Master effect blend |
| ChromaticStrength | 0.003 | 0.0 | 0.01 | 0.001 | RGB separation amount |
| NoiseStrength | 0.15 | 0.0 | 1.0 | 0.05 | VHS noise intensity |
| ScanlineIntensity | 0.3 | 0.0 | 1.0 | 0.05 | Scanline visibility |
| DistortionStrength | 0.002 | 0.0 | 0.01 | 0.001 | Image warping amount |
| VignetteStrength | 0.5 | 0.0 | 1.0 | 0.05 | Edge darkening |
| GrainStrength | 0.1 | 0.0 | 0.5 | 0.05 | Film grain amount |

### Scalar Parameters - Quality

| Parameter | Default | Min | Max | Step | Description |
|-----------|---------|-----|-----|------|-------------|
| ScanlineCount | 480 | 240 | 1080 | 60 | Number of horizontal lines |
| NoiseScale | 100.0 | 50.0 | 200.0 | 10.0 | Noise texture scale |
| GrainSize | 1.0 | 0.5 | 2.0 | 0.1 | Grain particle size |

### Scalar Parameters - Animation

| Parameter | Default | Min | Max | Step | Description |
|-----------|---------|-----|-----|------|-------------|
| NoiseSpeed | 1.0 | 0.1 | 5.0 | 0.1 | Noise animation speed |
| ScanlineSpeed | 0.1 | 0.0 | 1.0 | 0.05 | Scanline scroll speed |
| DistortionSpeed | 0.5 | 0.0 | 2.0 | 0.1 | Warping animation speed |
| DistortionFrequency | 5.0 | 1.0 | 10.0 | 0.5 | Warping wave frequency |

### Vector Parameters

| Parameter | Default | Range | Description |
|-----------|---------|-------|-------------|
| ColorTint | (1.0, 0.95, 0.9) | 0-1 per channel | Warm analog color tint |

### Scalar Parameters - Color

| Parameter | Default | Min | Max | Step | Description |
|-----------|---------|-----|-----|------|-------------|
| Saturation | 0.7 | 0.0 | 1.0 | 0.05 | Color saturation |
| Brightness | 1.0 | 0.5 | 1.5 | 0.05 | Overall brightness |
| Contrast | 1.1 | 0.8 | 1.5 | 0.05 | Contrast adjustment |

### Static Switches

| Parameter | Default | Cost | Description |
|-----------|---------|------|-------------|
| EnableChromatic | True | +45 inst | Enable chromatic aberration |
| EnableNoise | True | +60 inst | Enable VHS noise |
| EnableScanlines | True | +30 inst | Enable scanlines |
| EnableDistortion | True | +50 inst | Enable image warping |
| EnableVignette | True | +25 inst | Enable vignette |
| EnableGrain | True | +40 inst | Enable film grain |
| EnableColorGrade | True | +35 inst | Enable color grading |

---

## M_Master_PostProcess Parameters

### Scalar Parameters - Effects

| Parameter | Default | Min | Max | Step | Description |
|-----------|---------|-----|-----|------|-------------|
| EffectIntensity | 1.0 | 0.0 | 1.0 | 0.05 | Master effect blend |
| BlurStrength | 0.0 | 0.0 | 10.0 | 0.5 | Blur amount |
| GlowIntensity | 1.0 | 0.0 | 5.0 | 0.1 | Glow brightness |
| EdgeIntensity | 0.0 | 0.0 | 1.0 | 0.05 | Edge detection strength |
| DOFStrength | 0.0 | 0.0 | 1.0 | 0.05 | Depth of field amount |

### Scalar Parameters - Quality

| Parameter | Default | Min | Max | Step | Description |
|-----------|---------|-----|-----|------|-------------|
| BlurKernelSize | 5 | 3 | 15 | 2 | Blur sample count (odd) |
| GlowRadius | 2.0 | 1.0 | 10.0 | 0.5 | Glow spread distance |
| EdgeThickness | 1.0 | 0.5 | 5.0 | 0.5 | Edge line width |
| GlowThreshold | 0.8 | 0.0 | 1.0 | 0.05 | Brightness cutoff |

### Scalar Parameters - Color

| Parameter | Default | Min | Max | Step | Description |
|-----------|---------|-----|-----|------|-------------|
| Exposure | 1.0 | 0.1 | 5.0 | 0.1 | Brightness multiplier |
| Saturation | 1.0 | 0.0 | 2.0 | 0.1 | Color saturation |
| Contrast | 1.0 | 0.5 | 2.0 | 0.1 | Contrast adjustment |

### Scalar Parameters - Depth

| Parameter | Default | Min | Max | Step | Description |
|-----------|---------|-----|-----|------|-------------|
| FocusDistance | 500.0 | 0.0 | 10000.0 | 50.0 | DOF focus point (cm) |
| FocusRange | 200.0 | 50.0 | 1000.0 | 25.0 | DOF focus falloff (cm) |

### Vector Parameters

| Parameter | Default | Range | Description |
|-----------|---------|-------|-------------|
| ColorFilter | (1,1,1) | 0-1 per channel | Color tint overlay |
| EdgeColor | (0,0,0) | 0-1 per channel | Edge line color |

### Static Switches

| Parameter | Default | Cost | Description |
|-----------|---------|------|-------------|
| EnableBlur | False | +80 inst | Enable blur effect |
| EnableGlow | False | +100 inst | Enable glow/bloom |
| EnableEdges | False | +60 inst | Enable edge detection |
| EnableDOF | False | +90 inst | Enable depth of field |
| EnableColorGrade | True | +35 inst | Enable color correction |

---

## M_Master_UI Parameters

### Texture Parameters

| Parameter | Type | Default | Compression | sRGB | Description |
|-----------|------|---------|-------------|------|-------------|
| T_UI_Texture | Texture2D | White | BC7 | Yes | Main UI texture |
| T_Mask | Texture2D | White | BC4 | No | Optional mask texture |

### Scalar Parameters

| Parameter | Default | Min | Max | Step | Description |
|-----------|---------|-----|-----|------|-------------|
| OpacityMultiplier | 1.0 | 0.0 | 1.0 | 0.05 | Overall opacity control |
| TintStrength | 0.0 | 0.0 | 1.0 | 0.05 | Color tint blend amount |
| EmissiveStrength | 0.0 | 0.0 | 10.0 | 0.5 | Glow intensity |
| MaskChannel | 3 | 0 | 3 | 1 | Mask texture channel (RGBA) |

### Vector Parameters

| Parameter | Default | Range | Description |
|-----------|---------|-------|-------------|
| UIColor | (1,1,1,1) | 0-1 per channel | Base color with alpha |
| TintColor | (1,1,1) | 0-1 per channel | Tint color overlay |

### Static Switches

| Parameter | Default | Cost | Description |
|-----------|---------|------|-------------|
| UseCustomOpacity | False | +10 inst | Use separate opacity control |
| UseMask | False | +20 inst | Enable mask texture |
| UseEmissive | False | +10 inst | Enable glow effect |
| UseVertexColor | False | +5 inst | Multiply by vertex color |

---

## Parameter Usage Guidelines

### Tiling

**Purpose:** Scale UV coordinates  
**Common Values:**
- 0.5: Half size (larger texture appearance)
- 1.0: Original size
- 2.0: Double size (smaller texture appearance)
- 4.0: Quad size (for tiling textures)

**Tips:**
- Use powers of 2 for best texture cache performance
- Match tiling to physical object size
- Test at various distances

### Normal Strength

**Purpose:** Control normal map intensity  
**Common Values:**
- 0.0: Flat surface (no normal)
- 0.5: Subtle detail
- 1.0: Full strength (default)
- 1.5: Enhanced detail
- 2.0: Exaggerated (use sparingly)

**Tips:**
- Reduce for distant objects
- Increase for hero assets
- Match to lighting conditions

### Roughness Multiplier

**Purpose:** Adjust surface roughness  
**Common Values:**
- 0.0: Mirror smooth (avoid)
- 0.3: Polished metal
- 0.5: Clean surface
- 0.7: Standard surface
- 0.9: Rough/matte surface
- 1.0: Very rough

**Tips:**
- Most surfaces: 0.5-0.8
- Avoid extremes (0.0, 1.0)
- Test under various lighting

### Metallic Multiplier

**Purpose:** Define metallic property  
**Common Values:**
- 0.0: Non-metal (dielectric)
- 0.5: Partially metallic (rare)
- 1.0: Pure metal

**Tips:**
- Use 0.0 or 1.0 for most materials
- Avoid intermediate values unless specific need
- Metals have low roughness (0.2-0.5)

### Emissive Strength

**Purpose:** Control glow intensity  
**Common Values:**
- 0.0: No glow
- 1.0: Subtle glow
- 5.0: Moderate glow
- 10.0: Strong glow
- 50.0+: Very bright (light source)

**Tips:**
- Scale with scene brightness
- Use with EmissiveColor for tinting
- Consider performance impact

---

## Dynamic Parameter Control

### Blueprint Examples

**Fade Material:**
```cpp
// Smooth opacity fade
float CurrentOpacity = MaterialInstance->K2_GetScalarParameterValue("OpacityMultiplier");
float NewOpacity = FMath::FInterpTo(CurrentOpacity, TargetOpacity, DeltaTime, 2.0f);
MaterialInstance->SetScalarParameterValue("OpacityMultiplier", NewOpacity);
```

**Pulse Emissive:**
```cpp
// Pulsing glow effect
float Time = GetWorld()->GetTimeSeconds();
float Pulse = (FMath::Sin(Time * 3.0f) + 1.0f) * 0.5f;
float EmissiveValue = FMath::Lerp(1.0f, 10.0f, Pulse);
MaterialInstance->SetScalarParameterValue("EmissiveStrength", EmissiveValue);
```

**Change Color:**
```cpp
// Smooth color transition
FLinearColor NewColor = FMath::CInterpTo(CurrentColor, TargetColor, DeltaTime, 1.0f);
MaterialInstance->SetVectorParameterValue("BaseColorTint", NewColor);
```

**Adjust VHS Intensity:**
```cpp
// Increase VHS effect during horror event
float Intensity = FMath::Lerp(0.5f, 1.0f, HorrorLevel);
VHSMaterial->SetScalarParameterValue("EffectIntensity", Intensity);
VHSMaterial->SetScalarParameterValue("DistortionStrength", 0.002f * Intensity);
```

---

## Performance Considerations

### Parameter Cost

| Parameter Type | Runtime Cost | Compilation Cost |
|----------------|--------------|------------------|
| Scalar | Minimal | None |
| Vector | Minimal | None |
| Texture | Low-Medium | None |
| Static Switch | None | High (recompile) |

### Optimization Tips

1. **Use scalar parameters** for values that change frequently
2. **Use static switches** for features that rarely change
3. **Minimize texture samples** (target: 3-5 per material)
4. **Use packed textures** (ORM) to reduce samples
5. **Disable unused features** via static switches

---

## Troubleshooting

### Parameter Not Visible

**Cause:** Parameter not exposed in material instance  
**Solution:** Open master material, check "Expose to Material Instance"

### Parameter Has No Effect

**Cause:** Static switch disabled or parameter not connected  
**Solution:** Verify material graph connections and switch states

### Performance Issues

**Cause:** Too many features enabled  
**Solution:** Disable unused static switches, reduce texture samples

---

## Related Documentation

- [MaterialInstanceCreation.md](MaterialInstanceCreation.md) - Instance creation guide
- [MaterialOptimization.md](MaterialOptimization.md) - Optimization techniques
- [M_Master_Surface.md](M_Master_Surface.md) - Surface material reference
- [M_Master_VHS.md](M_Master_VHS.md) - VHS material reference

---

**Document Owner:** Materials Team  
**Last Updated:** 2026-04-26  
**Next Review:** When adding new parameters or master materials
