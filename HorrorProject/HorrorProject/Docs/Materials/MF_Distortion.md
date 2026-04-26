# MF_Distortion - Distortion Material Function

**Function Type:** Material Function  
**Category:** Post Process / UV Manipulation  
**Performance Cost:** 50 shader instructions

---

## Overview

MF_Distortion creates wave-based UV distortion for simulating analog video warping, lens distortion, and glitch effects. Essential for VHS tape degradation and horror disorientation effects.

---

## Function Graph Structure

```
UV → Sin/Cos(Frequency) → Multiply(Strength) → Offset UV → Output
```

---

## Input Parameters

| Input | Type | Default | Range | Description |
|-------|------|---------|-------|-------------|
| UV | Vector2 | ScreenPosition | - | Input coordinates |
| Strength | Scalar | 0.002 | 0.0-0.01 | Distortion amount |
| Frequency | Scalar | 5.0 | 1.0-20.0 | Wave frequency |
| Speed | Scalar | 0.5 | 0.0-5.0 | Animation speed |
| Time | Scalar | Time | - | Current time |
| Direction | Vector2 | (1,0) | - | Distortion direction |

---

## Output

| Output | Type | Description |
|--------|------|-------------|
| DistortedUV | Vector2 | Warped UV coordinates |

---

## Implementation Details

### Wave Distortion

```
// Calculate wave offset
float Wave = sin(UV.y * Frequency + Time * Speed) * Strength

// Apply directional offset
float2 Offset = Direction * Wave

// Output distorted UV
float2 DistortedUV = UV + Offset
```

---

## Usage Examples

### VHS Tape Warping

```
MF_Distortion(
    UV: ScreenPosition,
    Strength: 0.002,
    Frequency: 5.0,
    Speed: 0.5,
    Time: Time,
    Direction: (1, 0)
)
```

### Intense Glitch

```
MF_Distortion(
    UV: ScreenPosition,
    Strength: 0.008,
    Frequency: 10.0,
    Speed: 2.0,
    Time: Time,
    Direction: (1, 0.5)
)
```

---

## Performance Analysis

**Instruction Count:** 50  
**Texture Samples:** 0  
**Performance Impact:** Low

---

**Document Owner:** Materials Team  
**Last Updated:** 2026-04-26  
**Function Location:** Content/Materials/Functions/MF_Distortion.uasset
