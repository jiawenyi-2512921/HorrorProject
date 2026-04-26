# M_Master_VHS - VHS Effect Master Material

**Material Type:** Master Material  
**Domain:** Post Process  
**Blend Mode:** Composite  
**Performance Budget:** 300-400 shader instructions

---

## Overview

M_Master_VHS is the core post-process material for creating authentic VHS/bodycam visual effects. It combines multiple analog video artifacts including chromatic aberration, scanlines, noise, distortion, and color degradation to achieve the signature horror aesthetic.

Based on: `Content/Bodycam_VHS_Effect/Materials/Masters/PostProcess/M_OldVHS.uasset`

---

## Material Graph Structure

### Scene Texture Input

```
SceneTexture:PostProcessInput0 → Base Scene Color
```

### Chromatic Aberration

```
SceneTexture → MF_Chromatic_Aberration → Separated RGB Channels
```

**Parameters:**
- ChromaticStrength (Default: 0.003, Range: 0.0-0.01)
- ChromaticOffset (Default: 0.002)

### VHS Noise Layer

```
Time → MF_VHS_Noise → Multiply(NoiseStrength) → Add to Scene
```

**Parameters:**
- NoiseStrength (Default: 0.15, Range: 0.0-1.0)
- NoiseSpeed (Default: 1.0, Range: 0.1-5.0)
- NoiseScale (Default: 100.0)

### Scanlines

```
ScreenPosition → MF_Scanlines → Multiply(ScanlineIntensity) → Overlay
```

**Parameters:**
- ScanlineCount (Default: 480, Range: 240-1080)
- ScanlineIntensity (Default: 0.3, Range: 0.0-1.0)
- ScanlineSpeed (Default: 0.1)

### Distortion/Warping

```
ScreenPosition → MF_Distortion → Offset UVs → Sample Scene
```

**Parameters:**
- DistortionStrength (Default: 0.002, Range: 0.0-0.01)
- DistortionFrequency (Default: 5.0)
- DistortionSpeed (Default: 0.5)

### Color Grading

```
Scene Color → MF_Color_Grading → Desaturate → Tint → Output
```

**Parameters:**
- Saturation (Default: 0.7, Range: 0.0-1.0)
- ColorTint (Default: (1.0, 0.95, 0.9) - warm)
- Brightness (Default: 1.0, Range: 0.5-1.5)
- Contrast (Default: 1.1, Range: 0.8-1.5)

### Vignette

```
ScreenPosition → MF_Vignette → Multiply with Scene → Output
```

**Parameters:**
- VignetteStrength (Default: 0.5, Range: 0.0-1.0)
- VignetteRadius (Default: 0.8, Range: 0.3-1.0)

### Film Grain

```
Time → MF_Film_Grain → Add to Final → Output
```

**Parameters:**
- GrainStrength (Default: 0.1, Range: 0.0-0.5)
- GrainSize (Default: 1.0, Range: 0.5-2.0)

---

## Material Parameters

### Intensity Controls

| Parameter | Default | Range | Description |
|-----------|---------|-------|-------------|
| EffectIntensity | 1.0 | 0.0-1.0 | Master intensity control |
| ChromaticStrength | 0.003 | 0.0-0.01 | Chromatic aberration amount |
| NoiseStrength | 0.15 | 0.0-1.0 | VHS noise intensity |
| ScanlineIntensity | 0.3 | 0.0-1.0 | Scanline visibility |
| DistortionStrength | 0.002 | 0.0-0.01 | Image warping amount |
| VignetteStrength | 0.5 | 0.0-1.0 | Edge darkening |
| GrainStrength | 0.1 | 0.0-0.5 | Film grain amount |

### Quality Settings

| Parameter | Default | Range | Description |
|-----------|---------|-------|-------------|
| ScanlineCount | 480 | 240-1080 | Number of horizontal lines |
| NoiseScale | 100.0 | 50-200 | Noise texture scale |
| GrainSize | 1.0 | 0.5-2.0 | Grain particle size |

### Animation Controls

| Parameter | Default | Range | Description |
|-----------|---------|-------|-------------|
| NoiseSpeed | 1.0 | 0.1-5.0 | Noise animation speed |
| ScanlineSpeed | 0.1 | 0.0-1.0 | Scanline scroll speed |
| DistortionSpeed | 0.5 | 0.0-2.0 | Warping animation speed |
| DistortionFrequency | 5.0 | 1.0-10.0 | Warping wave frequency |

### Color Parameters

| Parameter | Default | Description |
|-----------|---------|-------------|
| ColorTint | (1.0, 0.95, 0.9) | Warm analog color tint |
| Saturation | 0.7 | Color saturation (0=grayscale) |
| Brightness | 1.0 | Overall brightness multiplier |
| Contrast | 1.1 | Contrast adjustment |

### Static Switches

| Parameter | Default | Description |
|-----------|---------|-------------|
| EnableChromatic | True | Enable chromatic aberration |
| EnableNoise | True | Enable VHS noise |
| EnableScanlines | True | Enable scanlines |
| EnableDistortion | True | Enable image warping |
| EnableVignette | True | Enable vignette |
| EnableGrain | True | Enable film grain |
| EnableColorGrade | True | Enable color grading |

---

## Material Functions Used

### MF_VHS_Noise

**Purpose:** Generate animated VHS-style noise  
**Cost:** 60 instructions  
**Inputs:**
- Time (Scalar)
- NoiseScale (Scalar)
- NoiseSpeed (Scalar)

**Output:** RGB noise pattern

### MF_Chromatic_Aberration

**Purpose:** RGB channel separation effect  
**Cost:** 45 instructions  
**Inputs:**
- SceneTexture (Texture)
- Strength (Scalar)
- Offset (Vector2D)

**Output:** Separated color channels

### MF_Scanlines

**Purpose:** Horizontal scanline overlay  
**Cost:** 30 instructions  
**Inputs:**
- ScreenPosition (Vector2D)
- LineCount (Scalar)
- Intensity (Scalar)

**Output:** Scanline mask

### MF_Distortion

**Purpose:** Wave-based image distortion  
**Cost:** 50 instructions  
**Inputs:**
- UV (Vector2D)
- Strength (Scalar)
- Frequency (Scalar)
- Time (Scalar)

**Output:** Distorted UVs

### MF_Vignette

**Purpose:** Edge darkening effect  
**Cost:** 25 instructions  
**Inputs:**
- ScreenPosition (Vector2D)
- Strength (Scalar)
- Radius (Scalar)

**Output:** Vignette mask

### MF_Film_Grain

**Purpose:** Animated film grain texture  
**Cost:** 40 instructions  
**Inputs:**
- ScreenPosition (Vector2D)
- Time (Scalar)
- GrainSize (Scalar)

**Output:** Grain pattern

### MF_Color_Grading

**Purpose:** Color adjustment and tinting  
**Cost:** 35 instructions  
**Inputs:**
- SceneColor (Vector3)
- Saturation (Scalar)
- Brightness (Scalar)
- Contrast (Scalar)
- Tint (Vector3)

**Output:** Graded color

---

## Performance Analysis

### Shader Complexity

**Minimal Configuration (All disabled):**
- Shader Instructions: 50
- Texture Samples: 1
- Performance: Excellent

**Standard Configuration:**
- Shader Instructions: 320
- Texture Samples: 3
- Performance: Good

**Full Configuration (All enabled):**
- Shader Instructions: 385
- Texture Samples: 4
- Performance: Moderate

### Performance Impact by Feature

| Feature | Instructions | Samples | Impact |
|---------|--------------|---------|--------|
| Base Pass | 50 | 1 | Minimal |
| Chromatic Aberration | +45 | +2 | Low |
| VHS Noise | +60 | +1 | Medium |
| Scanlines | +30 | 0 | Low |
| Distortion | +50 | 0 | Low |
| Vignette | +25 | 0 | Minimal |
| Film Grain | +40 | +1 | Low |
| Color Grading | +35 | 0 | Low |

### Optimization Strategies

1. **Disable unused effects** via static switches
2. **Reduce noise resolution** for distant views
3. **Lower scanline count** on lower-end hardware
4. **Use material parameter collections** for global control
5. **Implement quality presets** (Low/Medium/High/Ultra)

---

## Quality Presets

### Low Quality

```
EnableChromatic: False
EnableNoise: True (NoiseStrength: 0.1)
EnableScanlines: True (ScanlineCount: 240)
EnableDistortion: False
EnableVignette: True
EnableGrain: False
EnableColorGrade: True
```

**Performance:** 180 instructions, 2 samples

### Medium Quality

```
EnableChromatic: True (ChromaticStrength: 0.002)
EnableNoise: True (NoiseStrength: 0.12)
EnableScanlines: True (ScanlineCount: 480)
EnableDistortion: True (DistortionStrength: 0.001)
EnableVignette: True
EnableGrain: True (GrainStrength: 0.05)
EnableColorGrade: True
```

**Performance:** 320 instructions, 3 samples

### High Quality

```
EnableChromatic: True (ChromaticStrength: 0.003)
EnableNoise: True (NoiseStrength: 0.15)
EnableScanlines: True (ScanlineCount: 720)
EnableDistortion: True (DistortionStrength: 0.002)
EnableVignette: True
EnableGrain: True (GrainStrength: 0.1)
EnableColorGrade: True
```

**Performance:** 360 instructions, 4 samples

### Ultra Quality

```
All features enabled at maximum quality
ScanlineCount: 1080
All strengths at default values
```

**Performance:** 385 instructions, 4 samples

---

## Material Instance Creation

### Standard VHS Instance

**MI_VHS_Standard:**
```
EffectIntensity: 1.0
ChromaticStrength: 0.003
NoiseStrength: 0.15
ScanlineIntensity: 0.3
DistortionStrength: 0.002
VignetteStrength: 0.5
GrainStrength: 0.1
ColorTint: (1.0, 0.95, 0.9)
Saturation: 0.7
```

### Intense Horror Instance

**MI_VHS_Horror:**
```
EffectIntensity: 1.0
ChromaticStrength: 0.006
NoiseStrength: 0.25
ScanlineIntensity: 0.5
DistortionStrength: 0.004
VignetteStrength: 0.7
GrainStrength: 0.2
ColorTint: (0.9, 0.85, 0.8)
Saturation: 0.5
Contrast: 1.3
```

### Subtle Bodycam Instance

**MI_VHS_Subtle:**
```
EffectIntensity: 0.6
ChromaticStrength: 0.001
NoiseStrength: 0.08
ScanlineIntensity: 0.15
DistortionStrength: 0.001
VignetteStrength: 0.3
GrainStrength: 0.05
ColorTint: (1.0, 0.98, 0.95)
Saturation: 0.85
```

---

## Usage Guidelines

### Post-Process Volume Setup

1. Create Post Process Volume in level
2. Set to Unbound (affects entire level)
3. Add M_Master_VHS to Post Process Materials array
4. Set Blend Weight to 1.0
5. Adjust parameters via Material Instance

### Dynamic Intensity Control

**Blueprint Example:**
```cpp
// Get post process material instance
UMaterialInstanceDynamic* VHSMaterial = PostProcessVolume->GetMaterialInstance(0);

// Adjust intensity based on gameplay
float Intensity = FMath::Lerp(0.5f, 1.0f, TensionLevel);
VHSMaterial->SetScalarParameterValue("EffectIntensity", Intensity);

// Increase distortion during horror events
VHSMaterial->SetScalarParameterValue("DistortionStrength", 0.005f);
```

### Layering with Other Effects

**Recommended Order:**
1. Base scene rendering
2. M_Master_VHS (this material)
3. Additional post-process (bloom, DOF)
4. UI rendering

---

## Troubleshooting

### Common Issues

**Issue:** Effect too subtle
- Increase EffectIntensity
- Boost individual effect strengths
- Check post-process volume is unbound
- Verify blend weight is 1.0

**Issue:** Performance problems
- Disable expensive features (Chromatic, Noise)
- Reduce ScanlineCount
- Lower NoiseScale
- Use quality presets

**Issue:** Chromatic aberration too strong
- Reduce ChromaticStrength (try 0.001-0.002)
- Check screen resolution (scales with resolution)

**Issue:** Scanlines not visible
- Increase ScanlineIntensity
- Check ScanlineCount matches resolution
- Verify EnableScanlines is True

**Issue:** Color looks wrong
- Reset ColorTint to (1,1,1)
- Adjust Saturation (0.7-0.9 range)
- Check Brightness and Contrast values

---

## Technical Specifications

### Material Properties

```
Material Domain: Post Process
Blend Mode: Composite
Shading Model: Unlit
Blendable Location: After Tonemapping
Blendable Priority: 0
Output Alpha: False
```

### Performance Settings

```
Use Full Precision: False
Use High Quality Normals: N/A
Dithered LOD Transition: False
Mobile Optimized: False (PC-only effect)
```

---

## Advanced Techniques

### Time-Based Glitches

Add random glitch events:
```
Time → Frac → If(>0.98) → Multiply(DistortionStrength * 5)
```

### Screen Shake Integration

Combine with camera shake for enhanced effect:
```
CameraShakeAmount → Add to DistortionStrength
```

### Audio-Reactive Effects

Sync with audio intensity:
```
AudioIntensity → Multiply(NoiseStrength)
```

---

## Related Materials

- M_ChromaticAbberation - Standalone chromatic effect
- M_Noise - Standalone noise effect
- M_FishEye - Lens distortion effect
- M_Emboss - Edge detection effect
- M_Sharpen - Sharpening filter

---

**Document Owner:** Materials Team  
**Last Updated:** 2026-04-26  
**Material Location:** Content/Bodycam_VHS_Effect/Materials/Masters/PostProcess/M_OldVHS.uasset
