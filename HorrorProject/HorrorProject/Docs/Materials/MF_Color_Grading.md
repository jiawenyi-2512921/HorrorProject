# MF_Color_Grading - Color Grading Material Function

**Function Type:** Material Function  
**Category:** Post Process / Color Correction  
**Performance Cost:** 35 shader instructions

---

## Overview

MF_Color_Grading provides comprehensive color correction including saturation, brightness, contrast, and color tinting. Essential for establishing mood and visual style in horror environments.

---

## Function Graph Structure

```
SceneColor → Desaturate → Brightness → Contrast → Tint → Output
```

---

## Input Parameters

| Input | Type | Default | Range | Description |
|-------|------|---------|-------|-------------|
| SceneColor | Vector3 | (1,1,1) | - | Input color |
| Saturation | Scalar | 1.0 | 0.0-2.0 | Color saturation (0=grayscale) |
| Brightness | Scalar | 1.0 | 0.1-5.0 | Brightness multiplier |
| Contrast | Scalar | 1.0 | 0.5-2.0 | Contrast adjustment |
| ColorTint | Vector3 | (1,1,1) | - | Color tint overlay |

---

## Output

| Output | Type | Description |
|--------|------|-------------|
| Color | Vector3 | Graded color output |

---

## Implementation Details

### Color Grading Pipeline

```
// 1. Desaturate
float Luminance = dot(SceneColor, float3(0.299, 0.587, 0.114))
float3 Desaturated = lerp(Luminance, SceneColor, Saturation)

// 2. Brightness
float3 Brightened = Desaturated * Brightness

// 3. Contrast
float3 Contrasted = ((Brightened - 0.5) * Contrast) + 0.5

// 4. Tint
float3 Final = Contrasted * ColorTint
```

---

## Usage Examples

### Horror Desaturation

```
MF_Color_Grading(
    SceneColor: Input,
    Saturation: 0.6,
    Brightness: 0.9,
    Contrast: 1.2,
    ColorTint: (0.9, 0.85, 0.8)
)
```

### Warm VHS Tint

```
MF_Color_Grading(
    SceneColor: Input,
    Saturation: 0.7,
    Brightness: 1.0,
    Contrast: 1.1,
    ColorTint: (1.0, 0.95, 0.9)
)
```

---

## Performance Analysis

**Instruction Count:** 35  
**Texture Samples:** 0  
**Performance Impact:** Low

---

**Document Owner:** Materials Team  
**Last Updated:** 2026-04-26  
**Function Location:** Content/Materials/Functions/MF_Color_Grading.uasset
