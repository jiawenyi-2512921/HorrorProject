# MF_Film_Grain - Film Grain Material Function

**Function Type:** Material Function  
**Category:** Post Process / Texture Effects  
**Performance Cost:** 40 shader instructions

---

## Overview

MF_Film_Grain adds animated film grain texture to simulate analog film stock and photographic noise. Creates organic, time-varying grain patterns for cinematic and horror aesthetics.

---

## Function Graph Structure

```
ScreenPosition → Add(Time) → TextureSample(GrainTexture) → Multiply(Strength) → Grain
```

---

## Input Parameters

| Input | Type | Default | Range | Description |
|-------|------|---------|-------|-------------|
| UV | Vector2 | ScreenPosition | - | Screen coordinates |
| Time | Scalar | Time | - | Animation time |
| GrainSize | Scalar | 1.0 | 0.5-2.0 | Grain particle size |
| GrainStrength | Scalar | 0.1 | 0.0-0.5 | Grain intensity |
| AnimationSpeed | Scalar | 1.0 | 0.1-5.0 | Grain animation speed |

---

## Output

| Output | Type | Description |
|--------|------|-------------|
| Grain | Vector3 | RGB grain pattern |

---

## Implementation Details

### Grain Generation

```
// Animate UV
float2 AnimatedUV = UV * (1.0 / GrainSize) + Time * AnimationSpeed * 0.1

// Sample grain texture
float3 GrainTexture = Texture2DSample(T_FilmGrain, AnimatedUV)

// Center around 0.5 and apply strength
float3 FinalGrain = (GrainTexture - 0.5) * GrainStrength
```

---

## Usage Examples

### Subtle Film Grain

```
MF_Film_Grain(
    UV: ScreenPosition,
    Time: Time,
    GrainSize: 1.0,
    GrainStrength: 0.05,
    AnimationSpeed: 1.0
)
```

### Heavy Analog Grain

```
MF_Film_Grain(
    UV: ScreenPosition,
    Time: Time,
    GrainSize: 0.8,
    GrainStrength: 0.2,
    AnimationSpeed: 2.0
)
```

---

## Performance Analysis

**Instruction Count:** 40  
**Texture Samples:** 1  
**Performance Impact:** Low

---

## Related Functions

- MF_VHS_Noise - For VHS static
- MF_Vignette - For edge darkening
- MF_Color_Grading - For color adjustment

---

**Document Owner:** Materials Team  
**Last Updated:** 2026-04-26  
**Function Location:** Content/Materials/Functions/MF_Film_Grain.uasset
