# MF_Glow - Glow Material Function

**Function Type:** Material Function  
**Category:** Post Process / Bloom  
**Performance Cost:** 100 shader instructions

---

## Overview

MF_Glow creates bloom/glow effects by extracting bright areas and blurring them. Used for emissive highlights, light sources, and atmospheric effects.

---

## Function Graph Structure

```
SceneTexture → Threshold(Brightness) → Blur → Multiply(Intensity) → Glow
```

---

## Input Parameters

| Input | Type | Default | Range | Description |
|-------|------|---------|-------|-------------|
| SceneTexture | Texture | PostProcessInput0 | - | Input scene |
| UV | Vector2 | ScreenPosition | - | Screen coordinates |
| Threshold | Scalar | 0.8 | 0.0-1.0 | Brightness cutoff |
| Intensity | Scalar | 1.0 | 0.0-5.0 | Glow strength |
| Radius | Scalar | 2.0 | 1.0-10.0 | Glow spread |

---

## Output

| Output | Type | Description |
|--------|------|-------------|
| Glow | Vector3 | Glow contribution |

---

## Implementation Details

### Glow Extraction

```
// Sample scene
float3 SceneColor = Texture2DSample(SceneTexture, UV)

// Extract bright areas
float Brightness = max(SceneColor.r, max(SceneColor.g, SceneColor.b))
float3 BrightAreas = SceneColor * saturate((Brightness - Threshold) / (1.0 - Threshold))

// Blur bright areas
float3 Blurred = ApplyBlur(BrightAreas, Radius)

// Apply intensity
float3 FinalGlow = Blurred * Intensity
```

---

## Usage Examples

### Subtle Glow

```
MF_Glow(
    SceneTexture: PostProcessInput0,
    UV: ScreenPosition,
    Threshold: 0.8,
    Intensity: 1.0,
    Radius: 2.0
)
```

### Intense Bloom

```
MF_Glow(
    SceneTexture: PostProcessInput0,
    UV: ScreenPosition,
    Threshold: 0.6,
    Intensity: 2.5,
    Radius: 5.0
)
```

---

## Performance Analysis

**Instruction Count:** 100  
**Texture Samples:** 10-20 (depends on blur radius)  
**Performance Impact:** Medium

---

**Document Owner:** Materials Team  
**Last Updated:** 2026-04-26  
**Function Location:** Content/Materials/Functions/MF_Glow.uasset
