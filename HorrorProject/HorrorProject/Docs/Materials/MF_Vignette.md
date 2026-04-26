# MF_Vignette - Vignette Material Function

**Function Type:** Material Function  
**Category:** Post Process / Screen Effects  
**Performance Cost:** 25 shader instructions

---

## Overview

MF_Vignette creates edge darkening effects that simulate lens vignetting and focus the viewer's attention toward the screen center. Essential for cinematic effects and horror atmosphere.

---

## Function Graph Structure

### Radial Gradient

```
ScreenPosition → Distance(Center) → Power(Falloff) → Multiply(Strength) → Mask
```

---

## Input Parameters

| Input | Type | Default | Range | Description |
|-------|------|---------|-------|-------------|
| UV | Vector2 | ScreenPosition | - | Screen coordinates |
| Strength | Scalar | 0.5 | 0.0-1.0 | Edge darkening amount |
| Radius | Scalar | 0.8 | 0.3-1.5 | Vignette size |
| Falloff | Scalar | 2.0 | 1.0-5.0 | Edge softness |
| Center | Vector2 | (0.5, 0.5) | - | Vignette center point |

---

## Output

| Output | Type | Description |
|--------|------|-------------|
| Mask | Scalar | Vignette mask (0=dark, 1=bright) |

---

## Implementation Details

### Mask Calculation

```
// Calculate distance from center
float2 CenterOffset = UV - Center
float Distance = length(CenterOffset)

// Apply radius and falloff
float Vignette = saturate(1.0 - pow(Distance / Radius, Falloff))

// Apply strength
float FinalMask = lerp(1.0, Vignette, Strength)
```

---

## Usage Examples

### Standard Vignette

```
MF_Vignette(
    UV: ScreenPosition,
    Strength: 0.5,
    Radius: 0.8,
    Falloff: 2.0,
    Center: (0.5, 0.5)
)
```

### Heavy Horror Vignette

```
MF_Vignette(
    UV: ScreenPosition,
    Strength: 0.8,
    Radius: 0.6,
    Falloff: 3.0,
    Center: (0.5, 0.5)
)
```

### Subtle Cinematic

```
MF_Vignette(
    UV: ScreenPosition,
    Strength: 0.3,
    Radius: 1.0,
    Falloff: 1.5,
    Center: (0.5, 0.5)
)
```

---

## Performance Analysis

**Instruction Count:** 25  
**Texture Samples:** 0  
**Performance Impact:** Minimal

---

## Advanced Techniques

### Animated Vignette

```cpp
// Pulse vignette with heartbeat
float Pulse = sin(Time * 2.0) * 0.2 + 0.5;
MaterialInstance->SetScalarParameterValue("VignetteStrength", Pulse);
```

### Directional Vignette

```
// Offset center based on look direction
Center = (0.5, 0.5) + LookOffset * 0.1
```

---

## Related Functions

- MF_Scanlines - For scanline overlay
- MF_Film_Grain - For grain texture
- MF_Color_Grading - For color adjustment

---

**Document Owner:** Materials Team  
**Last Updated:** 2026-04-26  
**Function Location:** Content/Materials/Functions/MF_Vignette.uasset
