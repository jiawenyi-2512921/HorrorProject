# MF_VHS_Noise - VHS Noise Material Function

**Function Type:** Material Function  
**Category:** Post Process / VHS Effects  
**Performance Cost:** 60 shader instructions

---

## Overview

MF_VHS_Noise generates animated analog video noise patterns that simulate VHS tape degradation. It combines multiple noise layers with time-based animation to create authentic video static and interference effects.

---

## Function Graph Structure

### Noise Generation

```
Time → Multiply(Speed) → Add(UV) → TextureSample(NoiseTexture) → Output
```

### Multi-Layer Noise

```
Layer1 (Fast) → Add → Layer2 (Slow) → Multiply(Strength) → Final Noise
```

---

## Input Parameters

| Input | Type | Default | Range | Description |
|-------|------|---------|-------|-------------|
| Time | Scalar | Time | - | Current game time |
| NoiseScale | Scalar | 100.0 | 50-200 | Texture tiling scale |
| NoiseSpeed | Scalar | 1.0 | 0.1-5.0 | Animation speed |
| NoiseStrength | Scalar | 0.15 | 0.0-1.0 | Overall intensity |
| UV | Vector2 | TexCoord | - | Screen UV coordinates |

---

## Output

| Output | Type | Description |
|--------|------|-------------|
| Noise | Vector3 | RGB noise pattern (0-1 range) |

---

## Implementation Details

### Noise Texture Generation

**Internal Texture:** Procedural noise pattern
- Resolution: 512x512
- Format: BC7 (sRGB)
- Tiling: Seamless
- Pattern: Perlin/Simplex noise

### Animation Method

```
// Fast layer (high frequency)
FastUV = UV * NoiseScale + Time * NoiseSpeed * 2.0
FastNoise = Texture2DSample(NoiseTexture, FastUV)

// Slow layer (low frequency)
SlowUV = UV * (NoiseScale * 0.5) + Time * NoiseSpeed * 0.3
SlowNoise = Texture2DSample(NoiseTexture, SlowUV)

// Combine layers
FinalNoise = (FastNoise * 0.6 + SlowNoise * 0.4) * NoiseStrength
```

### Noise Characteristics

- **Fast Layer:** Simulates high-frequency static
- **Slow Layer:** Simulates tape degradation
- **Combined:** Authentic VHS interference

---

## Uge Examples

### Standard VHS Noise

```
MF_VHS_Noise(
    Time: Time,
    NoiseScale: 100.0,
    NoiseSpeed: 1.0,
    NoiseStrength: 0.15,
    UV: ScreenPosition
)
```

**Result:** Moderate analog noise

### Intense Static

```
MF_VHS_Noise(
    Time: Time,
    NoiseScale: 150.0,
    NoiseSpeed: 2.5,
    NoiseStrength: 0.35,
    UV: ScreenPosition
)
```

**Result:** Heavy interference

### Subtle Grain

```
MF_VHS_Noise(
    Time: Time,
    NoiseScale: 200.0,
    NoiseSpeed: 0.5,
    NoiseStrength: 0.08,
    UV: ScreenPosition
)
```

**Result:** Light texture

---

## Performance Analysis

### Instruction Count

- Base noise generation: 40 instructions
- Multi-layer blending: 15 instructions
- UV manipulation: 5 instructions
- **Total:** 60 instructions

### Texture Samples

- 2 samples (fast and slow layers)

### Optimization Tips

1. **Reduce NoiseScale** for better texture cache performance
2. **Use single layer** for low-end hardware (30 instructions)
3. **Pre-bake noise** into texture for static patterns
4. **Lower texture resolution** to 256x256 if needed

---

## Integration with Materials

### In M_Master_VHS

```
SceneColor → Add(MF_VHS_Noise) → Final Color
```

### Dynamic Control

```cpp
// Increase noise during horror events
float NoiseAmount = FMath::Lerp(0.15f, 0.4f, HorrorIntensity);
MaterialInstance->SetScalarParameterValue("NoiseStrength", NoiseAmount);
```

---

## Advanced Techniques

### Scanline-Synced Noise

```
UV.Y → Frac(UV.Y * ScanlineCount) → Multiply(Noise) → Banded Noise
```

### Color Channel Separation

```
Noise.R → Offset(+0.002, 0)
Noise.G → No offset
Noise.B → Offset(-0.002, 0)
```

### Temporal Glitches

```
Time → Frac → If(>0.95) → Multiply(NoiseStrength * 3) → Glitch Spikes
```

---

## Troubleshooting

**Issue:** Noise too repetitive
- Increase NoiseScale
- Use higher resolution noise texture
- Add third noise layer

**Issue:** Performance problems
- Reduce to single layer
- Lower NoiseScale
- Use smaller texture resolution

**Issue:** Noise too static
- Increase NoiseSpeed
- Add more animation layers
- Use time-based variation

---

## Technical Specifications

```
Function Inputs: 5
Function Outputs: 1
Texture Samples: 2
Math Operations: 18
Estimated Cost: 60 instructions
```

---

**Document Owner:** Materials Team  
**Last Updated:** 2026-04-26  
**Function Location:** Content/Materials/Functions/MF_VHS_Noise.uasset
