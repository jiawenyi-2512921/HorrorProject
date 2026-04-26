# MF_Scanlines - Scanlines Material Function

**Function Type:** Material Function  
**Category:** Post Process / VHS Effects  
**Performance Cost:** 30 shader instructions

---

## Overview

MF_Scanlines generates horizontal scanline patterns that simulate CRT displays and analog video output. This function creates the characteristic line structure of interlaced video, essential for authentic VHS and monitor effects.

---

## Function Graph Structure

### Scanline Pattern Generation

```
ScreenPosition.Y → Multiply(LineCount) → Frac → Step(Threshold) → Scanline Mask
```

### Animated Scanlines

```
ScreenPosition.Y → Add(Time * Speed) → Multiply(LineCount) → Pattern
```

---

## Input Parameters

| Input | Type | Default | Range | Description |
|-------|------|---------|-------|-------------|
| UV | Vector2 | ScreenPosition | - | Screen coordinates |
| LineCount | Scalar | 480 | 240-1080 | Number of horizontal lines |
| Intensity | Scalar | 0.3 | 0.0-1.0 | Line darkness |
| Thickness | Scalar | 0.5 | 0.1-0.9 | Line width (0=thin, 1=thick) |
| Speed | Scalar | 0.0 | -1.0-1.0 | Scroll speed (0=static) |
| Time | Scalar | Time | - | Current game time |

---

## Output

| Output | Type | Description |
|--------|------|-------------|
| Mask | Scalar | Scanline pattern (0-1) |

---

## Implementation Details

### Pattern Generation

```
// Calculate scanline position
float LinePosition = UV.y * LineCount + Time * Speed

// Create repeating pattern
float Pattern = frac(LinePosition)

// Generate line mask
float Mask = step(Thickness, Pattern)

// Apply intensity
float FinalMask = lerp(1.0, Mask, Intensity)
```

### Line Characteristics

**Standard (480 lines):**
- Matches SD video resolution
- Visible but not distracting
- Authentic VHS look

**High Definition (720-1080 lines):**
- Finer lines
- More subtle effect
- Modern display simulation

**Low Resolution (240 lines):**
- Thick, prominent lines
- Retro aesthetic
- Heavy CRT effect

---

## Usage Examples

### Standard VHS Scanlines

```
MF_Scanlines(
    UV: ScreenPosition,
    LineCount: 480,
    Intensity: 0.3,
    Thickness: 0.5,
    Speed: 0.0,
    Time: Time
)
```

**Result:** Static 480-line pattern

### Scrolling Monitor Effect

```
MF_Scanlines(
    UV: ScreenPosition,
    LineCount: 720,
    Intensity: 0.2,
    Thickness: 0.6,
    Speed: 0.1,
    Time: Time
)
```

**Result:** Slowly scrolling lines

### Heavy CRT Effect

```
MF_Scanlines(
    UV: ScreenPosition,
    LineCount: 240,
    Intensity: 0.5,
    Thickness: 0.4,
    Speed: 0.0,
    Time: Time
)
```

**Result:** Prominent retro lines

---

## Performance Analysis

### Instruction Count

- UV manipulation: 5 instructions
- Pattern generation: 10 instructions
- Masking: 10 instructions
- Intensity blend: 5 instructions
- **Total:** 30 instructions

### Texture Samples

- 0 samples (procedural)

### Performance Impact

| Configuration | Instructions | Impact |
|---------------|--------------|--------|
| Static | 25 | Minimal |
| Animated | 30 | Minimal |
| Complex | 35 | Low |

### Optimization Tips

1. **Use static scanlines** (Speed: 0) when possible
2. **Match LineCount to resolution** for best performance
3. **Reduce Intensity** for subtle effect
4. **Combine with other effects** efficiently

---

## Integration with Materials

### In M_Master_VHS

```
SceneColor → Multiply(MF_Scanlines) → Darkened Lines → Final
```

### Overlay Method

```
SceneColor → Lerp(Darkened, SceneColor, ScanlineMask) → Final
```

---

## Advanced Techniques

### Interlaced Scanlines

```
// Alternate lines per frame
float FrameOffset = floor(Time * 60.0) % 2.0
float InterlacedPattern = frac((UV.y * LineCount + FrameOffset) * 0.5)
```

### Curved Scanlines (CRT Curvature)

```
// Apply barrel distortion to UV before scanlines
float2 CurvedUV = ApplyBarrelDistortion(UV)
float Scanlines = MF_Scanlines(CurvedUV, ...)
```

### Flickering Scanlines

```
// Random intensity variation
float Flicker = sin(Time * 60.0 + UV.y * 100.0) * 0.1 + 0.9
float FinalIntensity = Intensity * Flicker
```

### Depth-Faded Scanlines

```
// Reduce scanlines on distant objects
float DepthFade = saturate(SceneDepth * 0.001)
float FinalIntensity = Intensity * DepthFade
```

---

## Quality Presets

### Low Quality (Performance)

```
LineCount: 240
Intensity: 0.2
Thickness: 0.6
Speed: 0.0
```

**Cost:** 25 instructions

### Medium Quality (Balanced)

```
LineCount: 480
Intensity: 0.3
Thickness: 0.5
Speed: 0.0
```

**Cost:** 30 instructions

### High Quality (Visual)

```
LineCount: 720
Intensity: 0.35
Thickness: 0.5
Speed: 0.05
```

**Cost:** 35 instructions

---

## Resolution Matching

### 1080p Display

```
LineCount: 1080 (1:1 pixel mapping)
Intensity: 0.2 (subtle)
Thickness: 0.7 (thin lines)
```

### 1440p Display

```
LineCount: 720 (2:1 mapping)
Intensity: 0.25
Thickness: 0.6
```

### 4K Display

```
LineCount: 1080 (2:1 mapping)
Intensity: 0.2
Thickness: 0.7
```

---

## Troubleshooting

**Issue:** Scanlines not visible
- Increase Intensity (0.4-0.6)
- Reduce Thickness (0.3-0.5)
- Check LineCount matches resolution
- Verify mask is being applied

**Issue:** Moiré patterns
- Adjust LineCount to match display resolution
- Use anti-aliasing
- Slightly animate with Speed
- Add subtle noise overlay

**Issue:** Lines too thick
- Increase Thickness value (0.6-0.8)
- Increase LineCount
- Reduce Intensity

**Issue:** Performance problems
- Use static scanlines (Speed: 0)
- Reduce LineCount
- Disable on low-end hardware

---

## Technical Specifications

```
Function Inputs: 6
Function Outputs: 1
Texture Samples: 0
Math Operations: 12
Estimated Cost: 30 instructions
Procedural: Yes
Screen-Space: Yes
```

---

## Combination Effects

### Scanlines + Chromatic Aberration

```
SceneColor → MF_Chromatic_Aberration → Multiply(MF_Scanlines) → Final
```

### Scanlines + Vignette

```
Scanlines → Multiply(Vignette) → Combined Mask → Apply to Scene
```

### Scanlines + Noise

```
Scanlines → Add(Noise * 0.1) → Noisy Lines → Apply
```

---

## Related Functions

- MF_VHS_Noise - For noise overlay
- MF_Vignette - For edge darkening
- MF_Chromatic_Aberration - For color fringing
- MF_Distortion - For CRT curvature

---

**Document Owner:** Materials Team  
**Last Updated:** 2026-04-26  
**Function Location:** Content/Materials/Functions/MF_Scanlines.uasset
