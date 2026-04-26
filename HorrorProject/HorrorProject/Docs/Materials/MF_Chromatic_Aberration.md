# MF_Chromatic_Aberration - Chromatic Aberration Material Function

**Function Type:** Material Function  
**Category:** Post Process / VHS Effects  
**Performance Cost:** 45 shader instructions

---

## Overview

MF_Chromatic_Aberration simulates lens chromatic aberration by separating RGB color channels. This creates the characteristic color fringing seen in analog video equipment and low-quality optics, essential for authentic VHS and bodycam effects.

---

## Function Graph Structure

### RGB Channel Separation

```
UV → Offset(Red) → Sample(R)
UV → No Offset → Sample(G)
UV → Offset(Blue) → Sample(B)
→ Combine(R,G,B) → Output
```

---

## Input Parameters

| Input | Type | Default | Range | Description |
|-------|------|---------|-------|-------------|
| SceneTexture | Texture | PostProcessInput0 | - | Input scene color |
| UV | Vector2 | ScreenPosition | - | Screen coordinates |
| Strength | Scalar | 0.003 | 0.0-0.01 | Separation amount |
| Direction | Vector2 | (1,0) | - | Aberration direction |
| RadialAmount | Scalar | 0.0 | 0.0-1.0 | Radial distortion |

---

## Output

| Output | Type | Description |
|--------|------|-------------|
| Color | Vector3 | RGB with separated channels |

---

## Implementation Details

### Channel Offset Calculation

```
// Calculate radial offset from screen center
CenterOffset = UV - 0.5
RadialStrength = length(CenterOffset) * RadialAmount

// Red channel (positive offset)
RedUV = UV + Direction * Strength * (1.0 + RadialStrength)
RedChannel = SceneTexture.Sample(RedUV).r

// Green channel (no offset)
GreenChannel = SceneTexture.Sample(UV).g

// Blue channel (negative offset)
BlueUV = UV - Direction * Strength * (1.0 + RadialStrength)
BlueChannel = SceneTexture.Sample(BlueUV).b

// Combine
FinalColor = float3(RedChannel, GreenChannel, BlueChannel)
```

### Aberration Patterns

**Horizontal (Default):**
- Direction: (1, 0)
- Effect: Left-right color separation

**Vertical:**
- Direction: (0, 1)
- Effect: Up-down color separation

**Radial:**
- RadialAmount: 1.0
- Effect: Outward from center

**Diagonal:**
- Direction: (0.707, 0.707)
- Effect: 45-degree separation

---

## Usage Examples

### Standard VHS Aberration

```
MF_Chromatic_Aberration(
    SceneTexture: PostProcessInput0,
    UV: ScreenPosition,
    Strength: 0.003,
    Direction: (1, 0),
    RadialAmount: 0.0
)
```

**Result:** Subtle horizontal fringing

### Intense Horror Effect

```
MF_Chromatic_Aberration(
    SceneTexture: PostProcessInput0,
    UV: ScreenPosition,
    Strength: 0.008,
    Direction: (1, 0),
    RadialAmount: 0.5
)
```

**Result:** Strong radial aberration

### Lens Distortion

```
MF_Chromatic_Aberration(
    SceneTexture: PostProcessInput0,
    UV: ScreenPosition,
    Strength: 0.005,
    Direction: normalize(UV - 0.5),
    RadialAmount: 1.0
)
```

**Result:** Realistic lens aberration

---

## Performance Analysis

### Instruction Count

- UV offset calculation: 10 instructions
- Texture samples (3x): 30 instructions
- Channel combination: 5 instructions
- **Total:** 45 instructions

### Texture Samples

- 3 samples (R, G, B channels)

### Performance Impact

| Configuration | Instructions | Samples | Impact |
|---------------|--------------|---------|--------|
| Simple (no radial) | 35 | 3 | Low |
| Standard | 45 | 3 | Low |
| Radial | 55 | 3 | Medium |

### Optimization Tips

1. **Reduce Strength** to minimize visible offset
2. **Disable radial** for better performance
3. **Use lower resolution** for distant effects
4. **Combine with other effects** to share samples

---

## Integration with Materials

### In M_Master_VHS

```
SceneTexture → MF_Chromatic_Aberration → Add(Noise) → Final
```

### Dynamic Control

```cpp
// Increase aberration during impact
float ImpactStrength = FMath::Lerp(0.003f, 0.01f, ImpactAmount);
MaterialInstance->SetScalarParameterValue("ChromaticStrength", ImpactStrength);

// Animate direction for disorientation
FVector2D Direction = FVector2D(
    FMath::Sin(Time * 2.0f),
    FMath::Cos(Time * 2.0f)
);
MaterialInstance->SetVectorParameterValue("ChromaticDirection", Direction);
```

---

## Advanced Techniques

### Animated Aberration

```
Time → Sin → Multiply(Strength) → Pulsing Effect
```

### Edge-Only Aberration

```
ScreenPosition → Distance(0.5) → Remap → Multiply(Strength)
```

### Depth-Based Aberration

```
SceneDepth → Remap(Near, Far) → Multiply(Strength) → Depth Fringing
```

### Directional Glitch

```
Random(Time) → If(>0.95) → Spike Direction → Glitch Burst
```

---

## Quality Presets

### Low Quality

```
Strength: 0.001
RadialAmount: 0.0
Direction: (1, 0)
```

**Cost:** 35 instructions

### Medium Quality

```
Strength: 0.003
RadialAmount: 0.0
Direction: (1, 0)
```

**Cost:** 45 instructions

### High Quality

```
Strength: 0.005
RadialAmount: 0.3
Direction: (1, 0)
```

**Cost:** 55 instructions

---

## Troubleshooting

**Issue:** Effect too subtle
- Increase Strength (0.005-0.01)
- Add RadialAmount for edge emphasis
- Check scene has high-contrast edges

**Issue:** Performance problems
- Reduce Strength
- Disable RadialAmount
- Use quality presets

**Issue:** Color banding
- Ensure scene texture is high precision
- Check texture filtering is enabled
- Reduce Strength if too extreme

**Issue:** Artifacts at screen edges
- Clamp UV coordinates
- Reduce Strength near edges
- Use vignette to hide edges

---

## Technical Specifications

```
Function Inputs: 5
Function Outputs: 1
Texture Samples: 3
Math Operations: 15
Estimated Cost: 45 instructions
UV Manipulation: Yes
Screen-Space: Yes
```

---

## Related Functions

- MF_Distortion - For UV warping
- MF_VHS_Noise - For noise overlay
- MF_Vignette - For edge darkening
- MF_Blur - For motion blur

---

**Document Owner:** Materials Team  
**Last Updated:** 2026-04-26  
**Function Location:** Content/Materials/Functions/MF_Chromatic_Aberration.uasset
