# MF_Blur - Blur Material Function

**Function Type:** Material Function  
**Category:** Post Process / Filtering  
**Performance Cost:** 80-150 shader instructions

---

## Overview

MF_Blur implements Gaussian blur filtering for post-process effects. Used for depth of field, motion blur, and screen-space effects requiring image softening.

---

## Function Graph Structure

```
SceneTexture → Multi-Sample(Kernel) → Average → Blurred Output
```

---

## Input Parameters

| Input | Type | Default | Range | Description |
|-------|------|---------|-------|-------------|
| SceneTexture | Texture | PostProcessInput0 | - | Input scene |
| UV | Vector2 | ScreenPosition | - | Screen coordinates |
| KernelSize | Scalar | 5 | 3-15 | Sample count (odd) |
| BlurStrength | Scalar | 1.0 | 0.0-10.0 | Blur radius |

---

## Output

| Output | Type | Description |
|--------|------|-------------|
| BlurredColor | Vector3 | Blurred scene color |

---

## Implementation Details

### Gaussian Kernel

```
// 5x5 Gaussian weights
float Weights[5] = {0.06, 0.24, 0.40, 0.24, 0.06}

// Sample and accumulate
float3 Result = 0
for (int i = 0; i < KernelSize; i++)
{
    float2 Offset = (i - KernelSize/2) * BlurStrength * PixelSize
    Result += Texture2DSample(SceneTexture, UV + Offset) * Weights[i]
}
```

---

## Usage Examples

### Subtle Blur

```
MF_Blur(
    SceneTexture: PostProcessInput0,
    UV: ScreenPosition,
    KernelSize: 5,
    BlurStrength: 1.0
)
```

### Heavy Blur

```
MF_Blur(
    SceneTexture: PostProcessInput0,
    UV: ScreenPosition,
    KernelSize: 9,
    BlurStrength: 3.0
)
```

---

## Performance Analysis

| Kernel Size | Instructions | Samples | Impact |
|-------------|--------------|---------|--------|
| 3x3 | 60 | 9 | Low |
| 5x5 | 80 | 25 | Medium |
| 7x7 | 110 | 49 | High |
| 9x9 | 150 | 81 | Very High |

---

**Document Owner:** Materials Team  
**Last Updated:** 2026-04-26  
**Function Location:** Content/Materials/Functions/MF_Blur.uasset
