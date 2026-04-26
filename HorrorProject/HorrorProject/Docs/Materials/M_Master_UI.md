# M_Master_UI - UI Master Material

**Material Type:** Master Material  
**Domain:** User Interface  
**Blend Mode:** Translucent  
**Performance Budget:** 100-150 shader instructions

---

## Overview

M_Master_UI is the master material for all UI elements in HorrorProject. It provides efficient rendering for widgets, HUD elements, and screen overlays with support for transparency, masking, and color manipulation.

---

## Material Graph Structure

### Base Texture

```
TextureCoordinate → TextureSample(UITexture) → Multiply(Color) → Final Color
```

**Nodes:**
- TextureCoordinate (UV0)
- Texture2D Parameter: `T_UI_Texture`
- Vector Parameter: `UIColor` (Default: 1,1,1,1)

### Opacity Control

```
TextureSample(UITexture).A → Multiply(OpacityMultiplier) → Opacity Output
```

**Nodes:**
- Scalar Parameter: `OpacityMultiplier` (Default: 1.0, Range: 0.0-1.0)
- Static Switch: `UseCustomOpacity` (Default: False)

### Mask Channel

```
TextureSample(MaskTexture) → ComponentMask(Channel) → Multiply with Opacity
```

**Nodes:**
- Texture2D Parameter: `T_Mask` (Optional)
- Static Switch: `UseMask` (Default: False)
- Scalar Parameter: `MaskChannel` (R=0, G=1, B=2, A=3)

### Color Tinting

```
BaseColor → Lerp(TintColor, BaseColor, TintStrength) → Final Color
```

**Nodes:**
- Vector Parameter: `TintColor` (Default: 1,1,1)
- Scalar Parameter: `TintStrength` (Default: 0.0, Range: 0.0-1.0)

### Emissive for Glow

```
BaseColor → Multiply(EmissiveStrength) → Emissive Output
```

**Nodes:**
- Scalar Parameter: `EmissiveStrength` (Default: 0.0, Range: 0.0-10.0)
- Static Switch: `UseEmissive` (Default: False)

---

## Material Parameters

### Texture Parameters

| Parameter | Type | Default | Description |
|-----------|------|---------|-------------|
| T_UI_Texture | Texture2D | White | Main UI texture (sRGB) |
| T_Mask | Texture2D | White | Optional mask texture |

### Scalar Parameters

| Parameter | Default | Range | Description |
|-----------|---------|-------|-------------|
| OpacityMultiplier | 1.0 | 0.0-1.0 | Overall opacity control |
| TintStrength | 0.0 | 0.0-1.0 | Color tint blend amount |
| EmissiveStrength | 0.0 | 0.0-10.0 | Glow intensity |
| MaskChannel | 3 | 0-3 | Mask texture channel (RGBA) |

### Vector Parameters

| Parameter | Default | Description |
|-----------|---------|-------------|
| UIColor | (1,1,1,1) | Base color multiplier with alpha |
| TintColor | (1,1,1) | Tint color overlay |

### Static Switches

| Parameter | Default | Description |
|-----------|---------|-------------|
| UseCustomOpacity | False | Use separate opacity control |
| UseMask | False | Enable mask texture |
| UseEmissive | False | Enable glow effect |
| UseVertexColor | False | Multiply by vertex color |

---

## Performance Analysis

### Shader Complexity

**Minimal Configuration:**
- Shader Instructions: 85
- Texture Samples: 1
- Performance: Excellent

**Standard Configuration:**
- Shader Instructions: 120
- Texture Samples: 1-2
- Performance: Excellent

**Full Configuration:**
- Shader Instructions: 150
- Texture Samples: 2
- Performance: Good

### Performance Impact

| Feature | Instructions | Samples | Impact |
|---------|--------------|---------|--------|
| Base Texture | 85 | 1 | Minimal |
| Mask Texture | +20 | +1 | Low |
| Color Tinting | +15 | 0 | Minimal |
| Emissive | +10 | 0 | Minimal |
| Vertex Color | +5 | 0 | Minimal |

### Optimization Tips

1. **Use texture atlases** to reduce draw calls
2. **Disable unused features** via static switches
3. **Use single-channel masks** when possible
4. **Avoid emissive** unless necessary for glow
5. **Batch similar UI elements** together

---

## Material Instance Examples

### Standard Button

**MI_UI_Button_Standard:**
```
T_UI_Texture: T_Button_Base
UIColor: (1, 1, 1, 1)
OpacityMultiplier: 1.0
UseEmissive: False
```

### Glowing Health Bar

**MI_UI_HealthBar_Glow:**
```
T_UI_Texture: T_HealthBar_Fill
UIColor: (1, 0.2, 0.2, 1)
EmissiveStrength: 2.0
UseEmissive: True
```

### Masked Icon

**MI_UI_Icon_Masked:**
```
T_UI_Texture: T_Icon_Base
T_Mask: T_Icon_Mask
UseMask: True
MaskChannel: 3 (Alpha)
```

### Tinted Text

**MI_UI_Text_Warning:**
```
T_UI_Texture: T_Text_Base
TintColor: (1, 0.8, 0)
TintStrength: 0.5
```

---

## Usage Guidelines

### Widget Blueprint Integration

1. Create Material Instance from M_Master_UI
2. Assign to Image widget's Brush Material
3. Set texture in material instance
4. Adjust color and opacity as needed

### Dynamic Material Instances

**Blueprint Example:**
```cpp
// Create dynamic material instance
UMaterialInstanceDynamic* UIMaterial = UMaterialInstanceDynamic::Create(MI_UI_Base, this);

// Set on widget
ImageWidget->SetBrushFromMaterial(UIMaterial);

// Animate opacity
UIMaterial->SetScalarParameterValue("OpacityMultiplier", FadeValue);

// Change color
UIMaterial->SetVectorParameterValue("UIColor", NewColor);
```

### Animation Support

**Fade In/Out:**
```cpp
// Smooth fade
float Alpha = FMath::FInterpTo(CurrentAlpha, TargetAlpha, DeltaTime, FadeSpeed);
UIMaterial->SetScalarParameterValue("OpacityMultiplier", Alpha);
```

**Color Pulse:**
```cpp
// Pulsing glow effect
float Pulse = (FMath::Sin(Time * PulseSpeed) + 1.0f) * 0.5f;
UIMaterial->SetScalarParameterValue("EmissiveStrength", Pulse * MaxGlow);
```

---

## UI Element Types

### HUD Elements

**Health Bar:**
- Use emissive for low health warning
- Animate opacity for damage flash
- Tint red when critical

**Ammo Counter:**
- Standard text rendering
- Glow when reloading
- Flash when empty

**Crosshair:**
- Simple texture with opacity
- Tint based on target type
- Scale with accuracy

### Menu Elements

**Buttons:**
- Base texture with hover state
- Emissive glow on hover
- Opacity fade on disabled

**Panels:**
- Background with opacity
- Optional vignette mask
- Tinted for different states

**Icons:**
- Masked textures
- Color tinting for states
- Emissive for notifications

---

## Troubleshooting

### Common Issues

**Issue:** UI appears too bright
- Reduce EmissiveStrength
- Check UIColor alpha channel
- Verify texture is not HDR

**Issue:** Transparency not working
- Check blend mode is Translucent
- Verify OpacityMultiplier > 0
- Check texture has alpha channel

**Issue:** Mask not visible
- Enable UseMask static switch
- Check MaskChannel value (0-3)
- Verify mask texture is assigned

**Issue:** Performance problems
- Reduce number of unique materials
- Use texture atlases
- Disable unused features
- Check overdraw (stat RHI)

---

## Technical Specifications

### Material Properties

```
Material Domain: User Interface
Blend Mode: Translucent
Shading Model: Unlit
Two Sided: False
Dithered LOD Transition: False
Use Material Attributes: False
```

### Texture Sampling

```
Sampler Type: Color
Mip Value Mode: None (UI uses LOD0)
Texture Filtering: Bilinear
Texture Addressing: Clamp
```

### Performance Settings

```
Fully Rough: True (no specular needed)
Use Full Precision: False
Disable Depth Test: False
Output Velocity: False
```

---

## Advanced Techniques

### Gradient Overlay

```
ScreenPosition.Y → Lerp(ColorA, ColorB) → Multiply with Base
```

### Radial Fill (Progress Bar)

```
ScreenPosition → Atan2 → Compare(FillAmount) → Mask
```

### Distortion Effect

```
Time → Sin → Offset UVs → Sample Texture
```

### Multi-Layer Blending

```
Layer1 → Lerp(Layer2, Layer1, Mask) → Final
```

---

## Related Materials

- M_Master_PostProcess - For screen-space effects
- M_Master_VHS - For VHS overlay on UI
- MF_Glow - Glow function for highlights
- MF_Blur - Blur function for backgrounds

---

**Document Owner:** Materials Team  
**Last Updated:** 2026-04-26  
**Material Location:** Content/Materials/Masters/M_Master_UI.uasset
