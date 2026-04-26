# Material Instance Creation Guide

**Document Type:** Implementation Guide  
**Target Audience:** Artists, Technical Artists  
**Last Updated:** 2026-04-26

---

## Overview

This guide covers the complete workflow for creating material instances in HorrorProject, from master material selection to parameter configuration and optimization.

---

## Material Instance Workflow

### Step 1: Select Master Material

Choose the appropriate master material based on use case:

| Use Case | Master Material | Domain |
|----------|----------------|--------|
| Opaque surfaces (walls, floors, props) | M_Master_Surface | Surface |
| VHS/bodycam effects | M_Master_VHS | Post Process |
| Screen effects (blur, glow, DOF) | M_Master_PostProcess | Post Process |
| UI elements (HUD, menus) | M_Master_UI | User Interface |

### Step 2: Create Material Instance

**In Unreal Editor:**

1. Navigate to Content Browser
2. Right-click on master material
3. Select "Create Material Instance"
4. Name following convention: `MI_[Category]_[Name]_[Variation]`
5. Move to appropriate folder

**Example:**
```
Master: M_Master_Surface
Instance: MI_Wall_Concrete_Dirty_01
Location: Content/Materials/Instances/Environment/
```

### Step 3: Configure Parameters

Open material instance and set parameters:

1. **Textures:** Assign required texture assets
2. **Scalars:** Adjust numeric values
3. **Vectors:** Set colors and tints
4. **Static Switches:** Enable/disable features

### Step 4: Test and Optimize

1. Apply to mesh in level
2. Test under various lighting conditions
3. Check performance (stat ShaderComplexity)
4. Optimize if needed

### Step 5: Document and Save

1. Add descriptive comment in material instance
2. Save with proper naming
3. Update material library documentation
4. Commit to version control

---

## Master Material Selection Guide

### M_Master_Surface

**Use For:**
- Architecture (walls, floors, ceilings)
- Props (furniture, objects)
- Static meshes
- Opaque materials

**Don't Use For:**
- Transparent objects (glass, water)
- UI elements
- Post-process effects
- Emissive-heavy materials

**Key Parameters:**
- T_BaseColor (required)
- T_Normal (recommended)
- T_ORM (recommended)
- Tiling, RoughnessMultiplier, MetallicMultiplier

### M_Master_VHS

**Use For:**
- Bodycam camera effects
- VHS tape simulation
- Horror atmosphere
- Analog video look

**Don't Use For:**
- Surface materials
- UI elements
- Clean/modern visuals

**Key Parameters:**
- ChromaticStrength
- NoiseStrength
- ScanlineIntensity
- DistortionStrength

### M_Master_PostProcess

**Use For:**
- Custom screen effects
- Blur effects
- Glow/bloom
- Color correction
- Depth of field

**Don't Use For:**
- Surface materials
- VHS effects (use M_Master_VHS)
- UI rendering

**Key Parameters:**
- BlurStrength
- GlowIntensity
- Saturation
- DOFStrength

### M_Master_UI

**Use For:**
- HUD elements
- Menu screens
- Widget blueprints
- Screen overlays

**Don't Use For:**
- 3D surfaces
- Post-process effects
- World-space UI

**Key Parameters:**
- T_UI_Texture
- UIColor
- OpacityMultiplier
- EmissiveStrength

---

## Parameter Configuration

### Texture Parameters

**Best Practices:**
1. Always assign BaseColor/Albedo texture
2. Use Normal maps for detail (except UI)
3. Use ORM packed textures when possible
4. Ensure correct texture compression settings

**Texture Compression:**
- Albedo/Diffuse: BC7 (sRGB)
- Normal: BC5 (Linear)
- ORM: BC7 (Linear)
- Masks: BC4 (Linear)

**Resolution Guidelines:**
- Hero assets: 2K-4K
- Standard assets: 1K-2K
- Background assets: 512-1K
- UI elements: 1K-2K

### Scalar Parameters

**Common Ranges:**

| Parameter | Min | Max | Default | Notes |
|-----------|-----|-----|---------|-------|
| Tiling | 0.1 | 10.0 | 1.0 | UV scale |
| NormalStrength | 0.0 | 2.0 | 1.0 | Normal intensity |
| RoughnessMultiplier | 0.0 | 1.0 | 1.0 | Surface roughness |
| MetallicMultiplier | 0.0 | 1.0 | 1.0 | Metallic property |
| EmissiveStrength | 0.0 | 100.0 | 0.0 | Glow intensity |
| OpacityMultiplier | 0.0 | 1.0 | 1.0 | Transparency |

**Tips:**
- Start with default values
- Adjust incrementally (0.1 steps)
- Test under various lighting
- Document final values

### Vector Parameters

**Color Tinting:**
- Use (1,1,1) for no tint
- Warm tint: (1.0, 0.95, 0.9)
- Cool tint: (0.9, 0.95, 1.0)
- Horror tint: (0.9, 0.85, 0.8)

**Emissive Colors:**
- Red warning: (1, 0.2, 0.2)
- Green safe: (0.2, 1, 0.2)
- Blue info: (0.2, 0.5, 1)
- Yellow caution: (1, 0.9, 0.2)

### Static Switch Parameters

**Performance Impact:**
- Enabling switches adds shader instructions
- Disable unused features for optimization
- Changes require shader recompilation
- Test performance after changes

**Common Switches:**
- UseNormalMap: Enable for detailed surfaces
- UseEmissive: Enable for glowing materials
- UseDetailLayer: Enable for hero assets only
- UseVertexColor: Enable for painted meshes

---

## Material Instance Examples

### Example 1: Concrete Wall

**Master:** M_Master_Surface  
**Name:** MI_Wall_Concrete_Dirty_01

**Parameters:**
```
Textures:
  T_BaseColor: T_Wall_Concrete_D
  T_Normal: T_Wall_Concrete_N
  T_ORM: T_Wall_Concrete_ORM

Scalars:
  Tiling: 2.0
  NormalStrength: 0.8
  RoughnessMultiplier: 0.9
  MetallicMultiplier: 0.0

Vectors:
  BaseColorTint: (0.95, 0.95, 0.95)

Static Switches:
  UseNormalMap: True
  UseEmissive: False
```

**Use Case:** Standard environment walls

### Example 2: Rusty Metal Prop

**Master:** M_Master_Surface  
**Name:** MI_Metal_Rusty_Industrial

**Parameters:**
```
Textures:
  T_BaseColor: T_Metal_Rusty_D
  T_Normal: T_Metal_Rusty_N
  T_ORM: T_Metal_Rusty_ORM

Scalars:
  Tiling: 1.0
  NormalStrength: 1.0
  RoughnessMultiplier: 0.8
  MetallicMultiplier: 0.6

Vectors:
  BaseColorTint: (0.9, 0.7, 0.6)

Static Switches:
  UseNormalMap: True
  UseEmissive: False
```

**Use Case:** Industrial props, machinery

### Example 3: Glowing Emergency Light

**Master:** M_Master_Surface  
**Name:** MI_Light_Emergency_Red

**Parameters:**
```
Textures:
  T_BaseColor: T_Light_Housing_D
  T_Normal: T_Light_Housing_N
  T_ORM: T_Light_Housing_ORM
  T_Emissive: T_Light_Emissive

Scalars:
  Tiling: 1.0
  EmissiveStrength: 10.0
  RoughnessMultiplier: 0.3
  MetallicMultiplier: 0.8

Vectors:
  EmissiveColor: (1, 0.2, 0.2)

Static Switches:
  UseNormalMap: True
  UseEmissive: True
```

**Use Case:** Emergency lighting, warning lights

### Example 4: VHS Horror Effect

**Master:** M_Master_VHS  
**Name:** MI_VHS_Horror_Intense

**Parameters:**
```
Scalars:
  EffectIntensity: 1.0
  ChromaticStrength: 0.006
  NoiseStrength: 0.25
  ScanlineIntensity: 0.5
  DistortionStrength: 0.004
  VignetteStrength: 0.7
  GrainStrength: 0.2

Vectors:
  ColorTint: (0.9, 0.85, 0.8)

Scalars:
  Saturation: 0.5
  Contrast: 1.3

Static Switches:
  EnableChromatic: True
  EnableNoise: True
  EnableScanlines: True
  EnableDistortion: True
  EnableVignette: True
  EnableGrain: True
```

**Use Case:** Intense horror sequences

### Example 5: UI Health Bar

**Master:** M_Master_UI  
**Name:** MI_UI_HealthBar_Fill

**Parameters:**
```
Textures:
  T_UI_Texture: T_HealthBar_Fill

Scalars:
  OpacityMultiplier: 1.0
  EmissiveStrength: 2.0

Vectors:
  UIColor: (1, 0.2, 0.2, 1)

Static Switches:
  UseEmissive: True
```

**Use Case:** HUD health indicator

---

## Dynamic Material Instances

### Creating at Runtime

**Blueprint Example:**
```cpp
// Create dynamic material instance
UMaterialInstanceDynamic* DynMaterial = UMaterialInstanceDynamic::Create(MI_Base, this);

// Apply to mesh
MeshComponent->SetMaterial(0, DynMaterial);

// Set parameters
DynMaterial->SetScalarParameterValue("EmissiveStrength", 5.0f);
DynMaterial->SetVectorParameterValue("BaseColorTint", FLinearColor::Red);
```

**C++ Example:**
```cpp
// Create dynamic instance
UMaterialInstanceDynamic* DynMaterial = UMaterialInstanceDynamic::Create(BaseMaterial, this);

// Set texture parameter
DynMaterial->SetTextureParameterValue(FName("T_BaseColor"), NewTexture);

// Set scalar parameter
DynMaterial->SetScalarParameterValue(FName("Tiling"), 2.0f);

// Apply to component
StaticMeshComponent->SetMaterial(0, DynMaterial);
```

### Animating Parameters

**Fade Effect:**
```cpp
// Smooth fade in/out
float CurrentOpacity = DynMaterial->K2_GetScalarParameterValue("OpacityMultiplier");
float TargetOpacity = bFadeIn ? 1.0f : 0.0f;
float NewOpacity = FMath::FInterpTo(CurrentOpacity, TargetOpacity, DeltaTime, FadeSpeed);
DynMaterial->SetScalarParameterValue("OpacityMultiplier", NewOpacity);
```

**Pulsing Glow:**
```cpp
// Pulsing emissive effect
float Time = GetWorld()->GetTimeSeconds();
float Pulse = (FMath::Sin(Time * PulseSpeed) + 1.0f) * 0.5f;
float EmissiveValue = FMath::Lerp(MinGlow, MaxGlow, Pulse);
DynMaterial->SetScalarParameterValue("EmissiveStrength", EmissiveValue);
```

**Color Transition:**
```cpp
// Smooth color change
FLinearColor CurrentColor = DynMaterial->K2_GetVectorParameterValue("BaseColorTint");
FLinearColor NewColor = FMath::CInterpTo(CurrentColor, TargetColor, DeltaTime, ColorSpeed);
DynMaterial->SetVectorParameterValue("BaseColorTint", NewColor);
```

---

## Optimization Checklist

### Before Creating Instance

- [ ] Verify master material is appropriate
- [ ] Check if similar instance already exists
- [ ] Plan parameter values
- [ ] Prepare texture assets

### During Creation

- [ ] Use proper naming convention
- [ ] Disable unused static switches
- [ ] Set appropriate texture compression
- [ ] Use packed textures (ORM) when possible

### After Creation

- [ ] Test in-game lighting
- [ ] Check shader complexity (stat ShaderComplexity)
- [ ] Verify texture streaming
- [ ] Test on target hardware
- [ ] Document parameters

### Performance Validation

- [ ] Shader instructions < 500 (surface materials)
- [ ] Texture samples < 8
- [ ] No unnecessary features enabled
- [ ] Proper LOD material switching
- [ ] Texture memory within budget

---

## Common Mistakes to Avoid

### ❌ Wrong Master Material

**Problem:** Using M_Master_Surface for UI elements  
**Solution:** Use M_Master_UI for all UI materials

### ❌ Excessive Features

**Problem:** Enabling all static switches  
**Solution:** Only enable needed features

### ❌ Wrong Texture Compression

**Problem:** Normal map using BC7 instead of BC5  
**Solution:** Set compression to TC_Normalmap

### ❌ Extreme Parameter Values

**Problem:** Tiling set to 100.0  
**Solution:** Use reasonable ranges (0.1-10.0)

### ❌ Missing Textures

**Problem:** No normal map assigned  
**Solution:** Always assign at minimum BaseColor and Normal

### ❌ Poor Naming

**Problem:** "Material_01", "NewMaterial"  
**Solution:** Use descriptive names: MI_Wall_Concrete_Dirty_01

---

## Troubleshooting

### Material Appears Black

**Causes:**
- Missing BaseColor texture
- Texture not set to sRGB
- Metallic value too high
- No lighting in scene

**Solutions:**
- Assign BaseColor texture
- Check texture sRGB setting
- Reduce Metallic to 0.0-0.5
- Add lights to scene

### Material Too Shiny

**Causes:**
- Roughness too low
- Metallic too high
- Wrong ORM channel mapping

**Solutions:**
- Increase RoughnessMultiplier (0.7-0.9)
- Reduce MetallicMultiplier
- Verify ORM texture channels

### Material Too Dark

**Causes:**
- BaseColor too dark
- AO too strong
- Lighting insufficient

**Solutions:**
- Lighten BaseColor texture
- Reduce AOStrength
- Adjust scene lighting

### Performance Issues

**Causes:**
- Too many texture samples
- Complex shader instructions
- Unnecessary features enabled

**Solutions:**
- Use packed textures (ORM)
- Disable unused static switches
- Simplify material graph
- Use LOD materials

---

## Material Library Integration

### Adding to Library

1. Create material instance
2. Test thoroughly
3. Document parameters
4. Add to MaterialLibrary.md
5. Commit to version control

### Sharing with Team

1. Place in shared content folder
2. Update documentation
3. Notify team via communication channel
4. Provide usage examples

---

## Version Control

### Committing Material Instances

**Good Commit Message:**
```
Add MI_Wall_Concrete_Dirty_01 material instance

- Based on M_Master_Surface
- Configured for industrial environment
- Optimized for performance (420 instructions)
- Textures: 2K resolution, BC7/BC5 compression
```

**Files to Commit:**
- Material instance (.uasset)
- Associated textures (if new)
- Updated documentation

---

## Related Documentation

- [MaterialParameterGuide.md](MaterialParameterGuide.md) - Parameter reference
- [MaterialOptimization.md](MaterialOptimization.md) - Optimization techniques
- [MaterialLibrary.md](MaterialLibrary.md) - Complete material catalog
- [NamingConvention.md](../Assets/NamingConvention.md) - Naming standards

---

**Document Owner:** Materials Team  
**Last Updated:** 2026-04-26  
**Next Review:** Monthly or when adding new master materials
