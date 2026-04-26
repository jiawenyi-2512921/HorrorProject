# Niagara System Templates and Standard Configurations

## Overview

This document provides standard configuration templates for Niagara particle systems in the Horror Project. Use these templates as starting points for creating consistent, optimized particle effects.

## Standard Emitter Template

### Basic Emitter Configuration

```
Emitter Name: NE_[EffectName]
Simulation Target: CPUSim (default) or GPUSim (for 200+ particles)
Execution State: Active
Scalability Mode: Effects Quality
```

### Required Modules (All Emitters)

1. **Emitter Properties**
2. **Emitter Spawn**
3. **Emitter Update**
4. **Particle Spawn**
5. **Initialize Particle**
6. **Particle Update**
7. **Render**

## Module Templates

### 1. Initialize Particle Template

#### Standard Sprite Particle
```
Lifetime:
  - Mode: Random Range
  - Min: 1.0s
  - Max: 3.0s

Color:
  - Mode: Direct Set or Random Range
  - Default: (0.8, 0.8, 0.8, 0.5)

Sprite Size:
  - Mode: Random Uniform
  - Min: (5, 5)
  - Max: (15, 15)

Sprite Rotation:
  - Mode: Random Range
  - Min: 0°
  - Max: 360°

Mass:
  - Default: 0.01 kg
  - Range: 0.001 - 1.0 kg
```

#### Mesh Particle
```
Lifetime:
  - Mode: Random Range
  - Min: 1.0s
  - Max: 3.0s

Color:
  - Mode: Direct Set
  - Default: (1.0, 1.0, 1.0, 1.0)

Mesh Scale:
  - Mode: Random Uniform
  - Min: (0.5, 0.5, 0.5)
  - Max: (2.0, 2.0, 2.0)

Mesh Rotation:
  - Mode: Random Range
  - Min: (0, 0, 0)
  - Max: (360, 360, 360)

Mass:
  - Default: 0.1 kg
  - Range: 0.01 - 1.0 kg
```

### 2. Spawn Location Templates

#### Point Spawn
```
Shape: Point
Location: (0, 0, 0)
```

#### Sphere Spawn
```
Shape: Sphere
Radius: 50 units
Distribution: Volume or Surface
```

#### Box Spawn
```
Shape: Box
Box Size: (500, 500, 300)
Distribution: Volume
```

#### Cone Spawn
```
Shape: Cone
Cone Angle: 45°
Cone Radius: 10 units
Cone Height: 500 units
Distribution: Volume
```

### 3. Velocity Templates

#### Static (No Movement)
```
Mode: None
Velocity: (0, 0, 0)
```

#### Directional
```
Mode: Direct Set
Velocity: (0, 0, 100)
Variation: ±10%
```

#### Explosive
```
Mode: Random Range in Cone
Cone Axis: Impact Normal
Cone Angle: 90°
Magnitude: 200-500 units/s
```

#### Drift
```
Mode: Random Range
Min: (-20, -20, -10)
Max: (20, 20, 10)
```

### 4. Force Templates

#### Gravity
```
Gravity Scale: 1.0 (full gravity)
Gravity Direction: (0, 0, -980)
```

#### Drag
```
Drag Coefficient: 0.5 (moderate)
Velocity Scale: 1.0
```

#### Curl Noise
```
Noise Strength: 50.0
Noise Frequency: 0.5
Noise Octaves: 2
Noise Quality: Medium
```

#### Point Attraction
```
Attraction Point: User Parameter
Attraction Strength: 100
Falloff Radius: 500 units
Falloff Exponent: 2.0
```

### 5. Scale Over Life Templates

#### Fade In/Out
```
Scale Factor Curve:
  - 0.0s: 0.0
  - 0.2s: 1.0
  - Lifetime-0.3s: 1.0
  - Lifetime: 0.0
```

#### Grow Over Life
```
Scale Factor Curve:
  - 0.0s: 0.3
  - 0.5s: 1.0
  - Lifetime: 1.5
```

#### Shrink Over Life
```
Scale Factor Curve:
  - 0.0s: 1.0
  - Lifetime: 0.3
```

### 6. Color Over Life Templates

#### Fade Alpha
```
Alpha Curve:
  - 0.0s: 0.0
  - 0.2s: 1.0
  - Lifetime-0.3s: 1.0
  - Lifetime: 0.0
```

#### Color Gradient (Fire)
```
Color Curve:
  - 0.0s: (1.0, 0.8, 0.2) - Bright yellow
  - 0.5s: (1.0, 0.4, 0.1) - Orange
  - Lifetime: (0.5, 0.1, 0.0) - Dark red
```

#### Desaturate Over Life
```
Color Curve:
  - 0.0s: Full color
  - Lifetime: Grayscale
```

### 7. Collision Templates

#### Simple Collision (Kill)
```
Collision Mode: Scene Depth
Radius: 2.0
Kill on Collision: True
```

#### Bounce Collision
```
Collision Mode: Scene Depth + Trace
Radius: 2.0
Resilience: 0.4
Friction: 0.7
Kill on Collision: False
```

#### Stick Collision
```
Collision Mode: Scene Depth + Trace
Radius: 2.0
Resilience: 0.1
Friction: 0.9
Kill on Collision: False
Velocity Kill: < 10 units/s
```

### 8. Kill Particle Templates

#### Lifetime Only
```
Lifetime Kill: Enabled
```

#### Box Bounds
```
Kill Box:
  - Size: (600, 600, 400)
  - Invert: True (kill outside)
```

#### Velocity Kill
```
Kill when velocity < 10 units/s
Delay: 0.5s
```

## Material Templates

### Unlit Translucent Particle
```
Material Domain: Surface
Blend Mode: Translucent
Shading Model: Unlit
Two Sided: True
Disable Depth Test: False
Responsive AA: True

Required Inputs:
  - Base Color (from texture + particle color)
  - Opacity (from texture alpha + particle alpha)
  - Emissive (optional, for glowing particles)

Required Parameters:
  - Opacity (Scalar, default 0.5)
  - DepthFade (Scalar, default 50.0)
```

### Additive Particle
```
Material Domain: Surface
Blend Mode: Additive
Shading Model: Unlit
Two Sided: True
Disable Depth Test: False

Required Inputs:
  - Emissive Color (from texture + particle color)
  - Opacity (from texture alpha + particle alpha)

Required Parameters:
  - Intensity (Scalar, default 1.0)
  - DepthFade (Scalar, default 30.0)
```

### Lit Mesh Particle
```
Material Domain: Surface
Blend Mode: Opaque
Shading Model: Default Lit
Two Sided: False

Required Inputs:
  - Base Color
  - Normal
  - Roughness
  - Metallic
  - Ambient Occlusion

Required Parameters:
  - BaseColor (Vector3)
  - Roughness (Scalar, default 0.7)
  - Metallic (Scalar, default 0.0)
```

## User Parameter Templates

### Standard Exposed Parameters

```
SpawnRate (Float):
  - Default: 30.0
  - Range: 10-100
  - Description: Particles per second

ParticleSize (Float):
  - Default: 1.0
  - Range: 0.5-2.0
  - Description: Size multiplier

ParticleColor (Linear Color):
  - Default: (1, 1, 1, 1)
  - Description: Particle tint color

Intensity (Float):
  - Default: 1.0
  - Range: 0.0-3.0
  - Description: Effect intensity

EnableEffect (Bool):
  - Default: True
  - Description: Enable/disable effect
```

## LOD Templates

### Standard 3-LOD Configuration

```
LOD 0 (0-1500 units):
  - Spawn Rate: 100%
  - Max Particles: Full budget
  - Update Rate: 60 FPS
  - Quality: High

LOD 1 (1500-3000 units):
  - Spawn Rate: 70%
  - Max Particles: 60% of budget
  - Update Rate: 30 FPS
  - Quality: Medium

LOD 2 (3000+ units):
  - Spawn Rate: 50%
  - Max Particles: 30% of budget
  - Update Rate: 15 FPS
  - Quality: Low
```

## Scalability Templates

### Standard Scalability Settings

```
Epic/High:
  - Spawn Rate: 100%
  - Full features enabled
  - High quality modules
  - Collision enabled

Medium:
  - Spawn Rate: 70%
  - Essential features only
  - Medium quality modules
  - Collision enabled

Low:
  - Spawn Rate: 50%
  - Minimal features
  - Low quality modules
  - Collision disabled

Cinematic:
  - Spawn Rate: 150%
  - All features enabled
  - Highest quality
  - Enhanced effects
```

## System Naming Conventions

### Niagara Systems
```
NS_[Category]_[EffectName]

Examples:
  - NS_Dust_Particles
  - NS_Blood_Splatter
  - NS_Water_Drips
```

### Niagara Emitters
```
NE_[EffectName]Emitter

Examples:
  - NE_DustEmitter
  - NE_BloodEmitter
  - NE_SplashEmitter
```

### Materials
```
M_[EffectName]

Examples:
  - M_DustParticle
  - M_BloodParticle
  - M_WaterDrop
```

### Material Instances
```
MI_[EffectName]_Inst

Examples:
  - MI_DustParticle_Inst
  - MI_BloodParticle_Inst
```

### Textures
```
T_[EffectName]_[Type]

Types:
  - D (Diffuse/Base Color)
  - N (Normal)
  - O (Opacity/Alpha)
  - ORM (Occlusion/Roughness/Metallic)

Examples:
  - T_Dust_D
  - T_Blood_N
  - T_Water_O
```

## Common Module Combinations

### Atmospheric Particles (Dust, Fog)
```
1. Initialize Particle (random size, color, rotation)
2. Box Spawn Location (large volume)
3. Random Drift Velocity
4. Curl Noise Force
5. Light Drag
6. Scale Size Over Life (fade in/out)
7. Scale Alpha Over Life (fade in/out)
8. Slow Rotation
9. Camera Offset
10. Kill Box (outside bounds)
```

### Impact Effects (Blood, Debris)
```
1. Initialize Particle (random size, color, rotation)
2. Sphere/Point Spawn Location
3. Explosive Velocity (cone)
4. Full Gravity
5. Moderate Drag
6. Collision (bounce or stick)
7. Scale Size Over Life (pop in, fade out)
8. Scale Alpha Over Life (fade out)
9. Fast Rotation
10. Velocity Kill (after settling)
```

### Dripping Effects (Water, Blood)
```
1. Initialize Particle (small size)
2. Point Spawn Location
3. Downward Velocity
4. Full Gravity
5. Minimal Drag
6. Velocity-Based Stretch
7. Collision (kill + spawn splash)
8. Scale Alpha Over Life (fade in/out)
9. No Rotation
10. Collision Kill
```

### Ambient Effects (Fireflies, Embers)
```
1. Initialize Particle (varied size, emissive color)
2. Box Spawn Location
3. Random Drift Velocity
4. Curl Noise Force
5. Light Gravity (or negative for rising)
6. Moderate Drag
7. Scale Size Over Life (fade in/out)
8. Color Over Life (brightness variation)
9. Slow Rotation
10. Optional Point Attraction
```

## Blueprint Integration Template

### Spawning Particle Effect

```cpp
// Blueprint pseudocode
UNiagaraComponent* SpawnedEffect = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
    GetWorld(),
    NS_ParticleSystem,
    SpawnLocation,
    SpawnRotation,
    FVector(1.0f),
    true,  // Auto-destroy
    true,  // Auto-activate
    ENCPoolMethod::None
);

// Set parameters
SpawnedEffect->SetFloatParameter("Intensity", 1.5f);
SpawnedEffect->SetVectorParameter("ParticleColor", FVector(1.0f, 0.5f, 0.0f));
```

### Attached Particle Effect

```cpp
// Blueprint pseudocode
UNiagaraComponent* AttachedEffect = UNiagaraFunctionLibrary::SpawnSystemAttached(
    NS_ParticleSystem,
    AttachComponent,
    AttachSocketName,
    FVector::ZeroVector,
    FRotator::ZeroRotator,
    EAttachLocation::SnapToTarget,
    true  // Auto-destroy
);
```

## Quality Presets

### Performance Mode (Low-End Hardware)
```
- Spawn Rate: 50%
- Max Particles: 50% of budget
- Update Rate: 30 FPS
- Collision: Disabled
- Curl Noise: Simplified or disabled
- Shadows: Disabled
- LOD Distance: Reduced by 30%
```

### Balanced Mode (Mid-Range Hardware)
```
- Spawn Rate: 70%
- Max Particles: 70% of budget
- Update Rate: 60 FPS
- Collision: Enabled (depth only)
- Curl Noise: Medium quality
- Shadows: Disabled
- LOD Distance: Standard
```

### Quality Mode (High-End Hardware)
```
- Spawn Rate: 100%
- Max Particles: Full budget
- Update Rate: 60 FPS
- Collision: Full trace
- Curl Noise: High quality
- Shadows: Enabled (if applicable)
- LOD Distance: Extended by 30%
```

### Cinematic Mode (Offline Rendering)
```
- Spawn Rate: 150%
- Max Particles: 150% of budget
- Update Rate: 60 FPS
- Collision: Full trace + events
- Curl Noise: Highest quality
- Shadows: Enabled
- LOD Distance: Extended by 50%
```

## Testing Checklist Template

### Visual Tests
- [ ] Particles spawn correctly
- [ ] Movement looks natural
- [ ] Colors match design
- [ ] Size appropriate
- [ ] Fade in/out smooth
- [ ] Rotation natural
- [ ] Lighting interaction correct
- [ ] Visible in all conditions

### Performance Tests
- [ ] Particle count within budget
- [ ] GPU cost acceptable (<1ms)
- [ ] Memory usage acceptable
- [ ] No memory leaks
- [ ] LOD transitions smooth
- [ ] Culling works correctly
- [ ] Scalability settings work

### Integration Tests
- [ ] Spawns at correct location
- [ ] Attaches correctly
- [ ] Parameters work
- [ ] Cleanup works
- [ ] No crashes
- [ ] Works with level streaming
- [ ] Multiple instances work

### Collision Tests (if applicable)
- [ ] Collision detection works
- [ ] Bounce behavior correct
- [ ] No penetration
- [ ] Friction works
- [ ] Events trigger correctly

## Common Pitfalls to Avoid

1. **Too Many Particles**: Always respect particle budgets
2. **No LOD**: Always implement LOD for distance optimization
3. **No Culling**: Always enable visibility culling
4. **Expensive Collision**: Use scene depth when possible
5. **No Depth Fade**: Always use depth fade for soft particles
6. **Fixed Bounds Too Large**: Keep bounds tight to visible area
7. **No Scalability**: Always implement scalability settings
8. **Forgot Cleanup**: Ensure auto-destroy or manual cleanup
9. **No Parameter Exposure**: Expose key parameters for designers
10. **Untested Performance**: Always profile on target hardware
