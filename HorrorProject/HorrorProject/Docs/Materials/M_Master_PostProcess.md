# M_Master_PostProcess - Post Process Master Material

**Material Type:** Master Material  
**Domain:** Post Process  
**Blend Mode:** Composite  
**Performance Budget:** 200-300 shader instructions

---

## Overview

M_Master_PostProcess is a flexible master material for custom post-process effects beyond the VHS system. It provides a foundation for screen-space effects like blur, glow, edge detection, color correction, and custom filters.

---

## Material Graph Structure

### Scene Texture Input

```
SceneTexture:PostProcessInput0 → Base Scene Color
SceneTexture:SceneDepth → Depth Buffer
SceneTexture:CustomDepth → Custom Depth (optional)
```

### Blur Effect

```
SceneTexture → MF_Blur(KernelSize, Strength) → Blurred Scene
```

**Parameters:**
- BlurStrength (Default: 0.0, Range: 0.0-10.0)
- BlurKernelSize (Default: 5, Range: 3-15)

### Glow/Bloom

```
SceneTexture → Threshold(Brightness) → MF_Glow → Add to Scene
```

**Parameters:**
- GlowThreshold (Default: 0.8, Range: 0.0-1.0)
- GlowIntensity (Default: 1.0, Range: 0.0-5.0)
- GlowRadius (Default: 2.0, Range: 1.0-10.0)

### Edge Detection

```
SceneDepth → Sobel Filter → Edge Mask → Multiply(EdgeColor)
```

**Parameters:**
- EdgeThickness (Default: 1.0, Range: 0.5-5.0)
- EdgeColor (Default: 0,0,0)
- EdgeIntensity (Default: 0.0, Range: 0.0-1.0)

### Color Correction

```
SceneColor → MF_Color_Grading → Output
```

**Parameters:**
- Exposure (Default: 1.0, Range: 0.1-5.0)
- Saturation (Default: 1.0, Range: 0.0-2.0)
- Contrast (Default: 1.0, Range: 0.5-2.0)
- ColorFilter (Default: 1,1,1)

### Depth of Field (Custom)

```
SceneDepth → Compare(FocusDistance) → Lerp(Blurred, Sharp)
```

**Parameters:**
- FocusDistance (Default: 500.0, Range: 0-10000)
- FocusRange (Default: 200.0, Range: 50-1000)
- DOFStrength (Default: 0.0, Range: 0.0-1.0)

---

## Material Parameters

### Effect Intensity

| Parameter | Default | Range | Description |
|-----------|---------|-------|-------------|
| EffectIntensity | 1.0 | 0.0-1.0 | Master effect blend |
| BlurStrength | 0.0 | 0.0-10.0 | Blur amount |
| GlowIntensity | 1.0 | 0.0-5.0 | Glow brightness |
| EdgeIntensity | 0.0 | 0.0-1.0 | Edge detection strength |
| DOFStrength | 0.0 | 0.0-1.0 | Depth of field amount |

### Quality Settings

| Parameter | Default | Range | Description |
|-----------|---------|-------|-------------|
| BlurKernelSize | 5 | 3-15 | Blur sample count (odd numbers) |
| GlowRadius | 2.0 | 1.0-10.0 | Glow spread distance |
| EdgeThickness | 1.0 | 0.5-5.0 | Edge line width |

### Color Parameters

| Parameter | Default | Description |
|-----------|---------|-------------|
| ColorFilter | (1,1,1) | Color tint overlay |
| EdgeColor | (0,0,0) | Edge line color |
| Exposure | 1.0 | Brightness multiplier |
| Saturation | 1.0 | Color saturation |
| Contrast | 1.0 | Contrast adjustment |

### Depth Parameters

| Parameter | Default | Range | Description |
|-----------|---------|-------|-------------|
| FocusDistance | 500.0 | 0-10000 | DOF focus point (cm) |
| FocusRange | 200.0 | 50-1000 | DOF focus falloff (cm) |
| DepthFadeStart | 1000.0 | 0-10000 | Depth fade begin (cm) |
| DepthFadeEnd | 5000.0 | 0-10000 | Depth fade end (cm) |

### Static Switches

| Parameter | Default | Description |
|-----------|---------|-------------|
| EnableBlur | False | Enable blur effect |
| EnableGlow | False | Enable glow/bloom |
| EnableEdges | False | Enable edge detection |
| EnableDOF | False | Enable depth of field |
| EnableColorGrade | True | Enable color correction |
| UseCustomDepth | False | Use custom depth buffer |

---

## Material Functions Used

### MF_Blur

**Purpose:** Gaussian blur filter  
**Cost:** 80-150 instructions (depends on kernel size)  
**Inputs:**
- SceneTexture (Texture)
- KernelSize (Scalar, 3-15)
- Strength (Scalar, 0-10)

**Output:** Blurred scene color

### MF_Glow

**Purpose:** Bloom/glow effect  
**Cost:** 100 instructions  
**Inputs:**
- SceneTexture (Texture)
- Threshold (Scalar, 0-1)
- Intensity (Scalar, 0-5)
- Radius (Scalar, 1-10)

**Output:** Glow contribution

### MF_Color_Grading

**Purpose:** Color correction and grading  
**Cost:** 35 instructions  
**Inputs:**
- SceneColor (Vector3)
- Exposure (Scalar)
- Saturation (Scalar)
- Contrast (Scalar)
- ColorFilter (Vector3)

**Output:** Graded color

### MF_Distortion

**Purpose:** UV distortion for effects  
**Cost:** 50 instructions  
**Inputs:**
- UV (Vector2D)
- Strength (Scalar)
- Pattern (Texture)

**Output:** Distorted UVs

---

## Performance Analysis

### Shader Complexity

**Minimal Configuration:**
- Shader Instructions: 50
- Texture Samples: 1
- Performance: Excellent

**Standard Configuration:**
- Shader Instructions: 220
- Texture Samples: 2-3
- Performance: Good

**Full Configuration:**
- Shader Instructions: 380
- Texture Samples: 4-5
- Performance: Moderate

### Performance Impact by Feature

| Feature | Instructions | Samples | Impact |
|---------|--------------|---------|--------|
| Base Pass | 50 | 1 | Minimal |
| Blur (5x5) | +80 | +4 | Medium |
| Blur (9x9) | +150 | +8 | High |
| Glow | +100 | +2 | Medium |
| Edge Detection | +60 | +1 | Low |
| DOF | +90 | +4 | Medium |
| Color Grading | +35 | 0 | Low |

---

## Quality Presets

### Performance Mode

```
EnableBlur: False
EnableGlow: False
EnableEdges: False
EnableDOF: False
EnableColorGrade: True
```

**Cost:** 85 instructions

### Balanced Mode

```
EnableBlur: True (KernelSize: 5)
EnableGlow: True (Intensity: 1.0)
EnableEdges: False
EnableDOF: False
EnableColorGrade: True
```

**Cost:** 220 instructions

### Quality Mode

```
EnableBlur: True (KernelSize: 7)
EnableGlow: True (Intensity: 1.5)
EnableEdges: True
EnableDOF: True
EnableColorGrade: True
```

**Cost:** 350 instructions

---

## Material Instance Examples

### Horror Atmosphere

**MI_PP_Horror:**
```
EnableColorGrade: True
Saturation: 0.6
Contrast: 1.2
ColorFilter: (0.9, 0.85, 0.8)
EnableEdges: True
EdgeIntensity: 0.3
EdgeColor: (0, 0, 0)
```

### Dream Sequence

**MI_PP_Dream:**
```
EnableBlur: True
BlurStrength: 2.0
EnableGlow: True
GlowIntensity: 2.0
Saturation: 1.3
ColorFilter: (1.0, 0.95, 1.05)
```

### Focus Effect

**MI_PP_Focus:**
```
EnableDOF: True
FocusDistance: 300.0
FocusRange: 150.0
DOFStrength: 0.8
BlurKernelSize: 7
```

### Night Vision

**MI_PP_NightVision:**
```
EnableColorGrade: True
ColorFilter: (0.2, 1.0, 0.3)
Saturation: 0.3
Contrast: 1.5
EnableGlow: True
GlowIntensity: 1.5
```

---

## Usage Guidelines

### Post-Process Volume Setup

1. Create Post Process Volume
2. Add material to Post Process Materials array
3. Set appropriate blend weight
4. Configure priority if using multiple volumes

### Dynamic Control

**Blueprint Example:**
```cpp
// Get material instance
UMaterialInstanceDynamic* PPMaterial = PostProcessVolume->GetMaterialInstance(0);

// Fade in blur during stun
PPMaterial->SetScalarParameterValue("BlurStrength", StunAmount * 5.0f);

// Adjust focus distance to player's look target
float Distance = (LookTarget - CameraLocation).Size();
PPMaterial->SetScalarParameterValue("FocusDistance", Distance);

// Pulse glow during horror event
float Pulse = FMath::Sin(Time * 2.0f) * 0.5f + 1.0f;
PPMaterial->SetScalarParameterValue("GlowIntensity", Pulse * 2.0f);
```

### Layering Multiple Effects

**Recommended Order:**
1. Base scene rendering
2. M_Master_PostProcess (this material)
3. M_Master_VHS (if used)
4. Engine post-process (bloom, tone mapping)
5. UI rendering

---

## Advanced Techniques

### Depth-Based Fog

```
SceneDepth → Remap(FadeStart, FadeEnd) → Lerp(FogColor, SceneColor)
```

### Radial Blur (Impact Effect)

```
ScreenPosition → Distance(ImpactPoint) → Offset UVs → Sample Scene
```

### Chromatic Aberration (Custom)

```
ScreenPosition → Offset RGB separately → Combine Channels
```

### Screen-Space Reflections Enhancement

```
SceneColor → Threshold(Metallic) → Blur → Add Reflection
```

---

## Troubleshooting

### Common Issues

**Issue:** Effect not visible
- Check EffectIntensity > 0
- Verify static switches are enabled
- Check post-process volume is unbound
- Ensure blend weight is 1.0

**Issue:** Performance problems
- Reduce BlurKernelSize
- Disable expensive features (Blur, DOF)
- Lower GlowRadius
- Use quality presets

**Issue:** Blur looks blocky
- Increase BlurKernelSize (odd numbers only)
- Check BlurStrength isn't too high
- Verify texture filtering is enabled

**Issue:** Glow too bright
- Reduce GlowIntensity
- Increase GlowThreshold
- Check scene brightness values

**Issue:** DOF not working
- Enable EnableDOF static switch
- Check FocusDistance matches scene scale
- Verify depth buffer is available
- Adjust FocusRange for smoother transition

---

## Technical Specifications

### Material Properties

```
Material Domain: Post Process
Blend Mode: Composite
Shading Model: Unlit
Blendable Location: Before Tonemapping
Blendable Priority: 0
Output Alpha: False
```

### Scene Texture Access

```
PostProcessInput0: Current scene color
SceneDepth: Depth buffer (0-1 range)
CustomDepth: Custom depth stencil
SceneColor: Pre-tonemapped color
```

### Performance Settings

```
Use Full Precision: False
Dithered LOD Transition: False
Mobile Optimized: False
```

---

## Integration with Game Systems

### Health System

```cpp
// Blur and desaturate when low health
float HealthPercent = CurrentHealth / MaxHealth;
PPMaterial->SetScalarParameterValue("BlurStrength", (1.0f - HealthPercent) * 3.0f);
PPMaterial->SetScalarParameterValue("Saturation", FMath::Lerp(0.3f, 1.0f, HealthPercent));
```

### Sanity System

```cpp
// Increase edge detection and distortion when insane
float SanityPercent = CurrentSanity / MaxSanity;
PPMaterial->SetScalarParameterValue("EdgeIntensity", 1.0f - SanityPercent);
PPMaterial->SetScalarParameterValue("DistortionStrength", (1.0f - SanityPercent) * 0.01f);
```

### Environmental Effects

```cpp
// Underwater effect
if (bIsUnderwater)
{
    PPMaterial->SetScalarParameterValue("BlurStrength", 1.5f);
    PPMaterial->SetVectorParameterValue("ColorFilter", FLinearColor(0.5f, 0.7f, 1.0f));
}
```

---

## Related Materials

- M_Master_VHS - VHS/bodycam effects
- M_Emboss - Edge emboss effect
- M_Sharpen - Sharpening filter
- M_FishEye - Lens distortion
- MF_Blur - Blur function
- MF_Glow - Glow function

---

**Document Owner:** Materials Team  
**Last Updated:** 2026-04-26  
**Material Location:** Content/Materials/Masters/M_Master_PostProcess.uasset
