# NS_Blood_Splatter - Blood Impact Effect Implementation Guide

## System Overview

**Purpose**: Blood splatter particle effect for impact, damage, and horror visual feedback.

**Use Cases**:
- Weapon impact blood
- Character damage feedback
- Environmental blood stains
- Horror atmosphere enhancement

**Performance Budget**:
- Max Particles: 100 per burst
- Draw Calls: 2 (particles + decals)
- GPU Cost: ~0.5ms @ 1080p
- Memory: ~3MB

## Emitter Configuration

### Emitter Properties

**Emitter State**:
- Execution State: Active (burst only)
- Execution State Source: Scalability
- Simulation Target: CPUSim
- Requires Persistent IDs: False
- Determinism: False

**Spawn Settings**:
- Spawn Rate: 0 (burst only)
- Spawn Burst: 50-100 particles
- Spawn Probability: 1.0
- One Shot: True

### Emitter Modules

#### 1. Initialize Particle

**Lifetime**:
- Mode: Random Range
- Minimum: 0.5 seconds
- Maximum: 2.0 seconds

**Color**:
- Mode: Random Range
- Color Min: (R: 0.3, G: 0.05, B: 0.05, A: 1.0)
- Color Max: (R: 0.6, G: 0.1, B: 0.1, A: 1.0)
- Color Mode: Linear Color

**Sprite Size**:
- Mode: Random Uniform
- Minimum: (X: 3.0, Y: 3.0)
- Maximum: (X: 10.0, Y: 10.0)
- Scale Mode: Uniform

**Sprite Rotation**:
- Mode: Random Range
- Minimum: 0.0 degrees
- Maximum: 360.0 degrees

**Mass**:
- Mass: Random Range (0.01, 0.05) kg

#### 2. Spawn Location

**Shape**: Sphere
- Sphere Radius: 10 units
- Distribution Mode: Volume
- Surface Only: False

**Velocity**:
- Mode: Random Range in Cone
- Cone Axis: Impact Normal
- Cone Angle: 60 degrees
- Velocity Magnitude: 200-500 units/second

#### 3. Update Modules

**Gravity**:
- Gravity Scale: 1.0 (full gravity)
- Gravity Direction: (0, 0, -980)

**Drag**:
- Drag Coefficient: 2.0 (high air resistance)
- Velocity Scale: 1.0

**Scale Sprite Size**:
- Scale Factor Curve:
  - 0.0s: 0.5
  - 0.1s: 1.0
  - Lifetime-0.2s: 1.0
  - Lifetime: 0.8 (shrink slightly)

**Scale Color**:
- Alpha Curve:
  - 0.0s: 1.0
  - Lifetime-0.3s: 1.0
  - Lifetime: 0.0 (fade out)

**Sprite Rotation Rate**:
- Rotation Rate: Random Range (-180, 180) degrees/second

**Color Over Life**:
- Color Curve:
  - 0.0s: Bright red
  - 0.5s: Dark red
  - Lifetime: Very dark red (dried)

#### 4. Collision

**Collision Module**:
- Collision Mode: Scene Depth + Trace
- Radius: 2.0
- Resilience: 0.1 (minimal bounce)
- Friction: 0.9 (high friction)
- Kill Particles on Collision: False

**Collision Event**:
- On Collision: Spawn Decal
- Decal Material: M_BloodDecal
- Decal Size: Random (5-15 units)
- Decal Lifetime: 60 seconds

#### 5. Kill Particles

**Lifetime Kill**:
- Enabled: True

**Velocity Kill**:
- Kill when velocity < 10 units/s
- Delay: 0.5 seconds after collision

## Particle Attributes

### Required Attributes

| Attribute | Type | Default | Description |
|-----------|------|---------|-------------|
| Position | Vector3 | (0,0,0) | World position |
| Velocity | Vector3 | (0,0,0) | Movement velocity |
| Color | Linear Color | (0.5,0.08,0.08,1) | Blood color |
| SpriteSize | Vector2 | (6,6) | Sprite dimensions |
| SpriteRotation | Float | 0.0 | Rotation angle |
| Age | Float | 0.0 | Current age |
| Lifetime | Float | 1.0 | Max lifetime |
| Mass | Float | 0.02 | Particle mass |
| HasCollided | Bool | False | Collision flag |

### User-Exposed Parameters

| Parameter | Type | Default | Range | Description |
|-----------|------|---------|-------|-------------|
| BurstCount | Int | 75 | 20-150 | Particles per burst |
| ExplosionForce | Float | 1.0 | 0.5-2.0 | Initial velocity multiplier |
| BloodColor | Linear Color | (0.5,0.08,0.08,1) | - | Blood tint color |
| SprayAngle | Float | 60.0 | 30-120 | Cone spread angle |
| DecalSize | Float | 1.0 | 0.5-2.0 | Decal size multiplier |
| DecalLifetime | Float | 60.0 | 10-300 | Decal duration (seconds) |

## Material Configuration

### Material: M_BloodParticle

**Material Domain**: Surface
**Blend Mode**: Translucent
**Shading Model**: Unlit

**Material Properties**:
- Two Sided: True
- Disable Depth Test: False
- Responsive AA: True
- Translucency Lighting Mode: Surface TranslucencyVolume

**Texture Inputs**:

1. **Blood Texture**: T_Blood_D
   - Resolution: 256x256
   - Format: BC1
   - Compression: Default
   - Content: Blood droplet shapes (4x4 flipbook)

2. **Blood Normal**: T_Blood_N
   - Resolution: 256x256
   - Format: BC5
   - Compression: Normal Map
   - Content: Droplet normals

**Material Parameters**:

| Parameter | Type | Default | Description |
|-----------|------|---------|-------------|
| BloodColor | Vector3 | (0.5,0.08,0.08) | Base blood color |
| Metallic | Scalar | 0.1 | Slight wetness |
| Roughness | Scalar | 0.7 | Surface roughness |
| DepthFade | Scalar | 20.0 | Soft particle distance |

**Material Graph**:
```
Texture Sample (T_Blood_D + Flipbook)
  → Multiply (Particle Color)
  → Multiply (BloodColor)
  → Base Color

Texture Sample (T_Blood_N)
  → Normal

Particle Alpha
  → Multiply (DepthFade)
  → Opacity
```

### Material: M_BloodDecal

**Material Domain**: Deferred Decal
**Blend Mode**: Translucent
**Decal Blend Mode**: DBuffer Translucent Color Normal Roughness

**Texture Inputs**:

1. **Decal Texture**: T_BloodSplat_D
   - Resolution: 512x512
   - Format: BC1
   - Compression: Default
   - Content: Blood splatter patterns (8 variations)

2. **Decal Normal**: T_BloodSplat_N
   - Resolution: 512x512
   - Format: BC5
   - Compression: Normal Map

**Material Parameters**:

| Parameter | Type | Default | Description |
|-----------|------|---------|-------------|
| BloodColor | Vector3 | (0.3,0.05,0.05) | Decal color |
| Opacity | Scalar | 0.8 | Decal opacity |
| Roughness | Scalar | 0.6 | Surface roughness |
| FadeTime | Scalar | 60.0 | Fade duration |

## Performance Optimization

### LOD Configuration

**LOD 0** (0-1000 units):
- Burst Count: 75
- Max Particles: 100
- Sprite Size: 100%
- Collision: Full trace

**LOD 1** (1000-2000 units):
- Burst Count: 50
- Max Particles: 60
- Sprite Size: 80%
- Collision: Depth only

**LOD 2** (2000+ units):
- Burst Count: 25
- Max Particles: 30
- Sprite Size: 60%
- Collision: Disabled

### Scalability Settings

**Epic/High**:
- Full burst count
- Collision + decals enabled
- High quality flipbook

**Medium**:
- 70% burst count
- Collision enabled, simplified decals
- Medium quality flipbook

**Low**:
- 50% burst count
- No collision, no decals
- Low quality flipbook

**Cinematic**:
- 150% burst count
- Full collision + enhanced decals
- Highest quality

### Culling Configuration

**Distance Culling**:
- Max Draw Distance: 2500 units
- Cull Proxy Mode: None

**Visibility Culling**:
- Visibility Tag: "BloodEffects"
- Cull When Not Rendered: True
- Cull Delay: 1.0 seconds

**Bounds**:
- Fixed Bounds: (X: 200, Y: 200, Z: 200)
- Use Fixed Bounds: True

## Collision and Physics

### Collision Settings

**Collision Type**: Scene Depth + Line Trace
- Query Type: Both
- Collision Group: WorldDynamic
- Max Collision Iterations: 3

**Collision Response**:
- Resilience: 0.1 (minimal bounce)
- Friction: 0.9 (stick to surfaces)
- Velocity Retention: 0.2

**Collision Events**:
- On Collision: Spawn Decal
- Generate Collision Events: True

### Physics Simulation

**Forces**:
1. Gravity: -980 units/s² (full gravity)
2. Drag: 2.0 coefficient (high)
3. Initial Explosion: 200-500 units/s

**Integration**:
- Integration Method: Verlet
- Sub-Steps: 2
- Max Delta Time: 0.05 seconds

## Events and Lifecycle

### Spawn Events

**On Particle Spawn**:
- Initialize random rotation
- Set random lifetime
- Apply explosion velocity
- Set spawn color
- Randomize flipbook frame

### Update Events

**On Particle Update** (every frame):
- Apply gravity
- Apply drag
- Update rotation
- Scale size/alpha by age
- Check collision
- Update color (drying)

### Collision Events

**On Particle Collision**:
- Spawn blood decal at impact point
- Reduce velocity by 80%
- Set HasCollided flag
- Play impact sound (optional)

### Death Events

**On Particle Death**:
- No special handling
- Automatic cleanup

**Death Conditions**:
1. Lifetime expired
2. Velocity near zero after collision
3. Alpha < 0.01

## Testing and Validation

### Visual Tests

**Test 1: Splatter Pattern**
- Spawn at various angles
- Verify realistic spread
- Check cone angle correct
- Verify random distribution

**Test 2: Blood Color**
- Check color matches setting
- Verify darkening over time
- Check decal color matches
- Verify no oversaturation

**Test 3: Physics Behavior**
- Verify gravity works
- Check drag appropriate
- Verify bounce minimal
- Check particles stick to surfaces

**Test 4: Decal Spawning**
- Verify decals spawn on collision
- Check decal orientation correct
- Verify decal size appropriate
- Check decal lifetime works

**Test 5: Performance**
- Test multiple simultaneous bursts
- Check particle count limits
- Verify cleanup works
- Check no memory leaks

### Performance Tests

**Test 1: Particle Count**
- Monitor with `stat particles`
- Verify max 100 per burst
- Check cleanup after lifetime
- Verify no accumulation

**Test 2: GPU Cost**
- Use `profilegpu` command
- Target: <0.5ms per burst
- Check collision cost
- Verify scalability works

**Test 3: Memory Usage**
- Check with `stat memory`
- Target: <3MB per effect
- Verify decal cleanup
- Check texture memory

**Test 4: Multiple Bursts**
- Spawn 5+ bursts simultaneously
- Check total particle budget
- Verify performance acceptable
- Check no crashes

### Integration Tests

**Test 1: Damage Integration**
- Trigger on damage events
- Verify spawn location correct
- Check impact normal used
- Verify consistent behavior

**Test 2: Surface Types**
- Test on various surfaces
- Verify decals work on all
- Check orientation correct
- Verify no penetration

**Test 3: Dynamic Spawning**
- Spawn/destroy rapidly
- Verify no crashes
- Check memory cleanup
- Verify parameter changes work

## Troubleshooting

### Common Issues

**Issue: Blood not visible**
- Check material assigned
- Verify burst count > 0
- Check spawn location valid
- Verify emitter triggered

**Issue: Too much/little blood**
- Adjust BurstCount parameter
- Check particle size
- Verify spawn rate
- Check lifetime settings

**Issue: Blood doesn't fall**
- Enable gravity module
- Check gravity scale
- Verify mass > 0
- Check drag not too high

**Issue: Decals not spawning**
- Enable collision module
- Check decal material assigned
- Verify collision channel
- Check surface allows decals

**Issue: Performance problems**
- Reduce burst count
- Lower max particles
- Disable collision
- Reduce decal count

**Issue: Blood penetrates walls**
- Enable collision trace
- Increase collision radius
- Check collision channel
- Verify scene depth available

## Best Practices

1. **Burst Size**: Keep under 100 particles per burst
2. **Direction**: Always use impact normal for cone axis
3. **Color**: Darken over time for realism
4. **Decals**: Limit lifetime to prevent accumulation
5. **Performance**: Pool and reuse systems
6. **Collision**: Always enable for realism
7. **Cleanup**: Ensure decals fade and cleanup
8. **Scalability**: Reduce on low settings
9. **Sound**: Pair with impact sound effects
10. **Testing**: Test on various surface types

## Asset Requirements

### Textures

| Asset | Resolution | Format | Size | Description |
|-------|------------|--------|------|-------------|
| T_Blood_D | 256x256 | BC1 | ~65KB | Blood droplet flipbook |
| T_Blood_N | 256x256 | BC5 | ~85KB | Droplet normals |
| T_BloodSplat_D | 512x512 | BC1 | ~170KB | Decal patterns |
| T_BloodSplat_N | 512x512 | BC5 | ~340KB | Decal normals |

### Materials

| Asset | Type | Complexity | Description |
|-------|------|------------|-------------|
| M_BloodParticle | Unlit | Low | Particle material |
| M_BloodDecal | Decal | Low | Decal material |
| MI_BloodParticle_Inst | Instance | - | Runtime instance |
| MI_BloodDecal_Inst | Instance | - | Runtime instance |

### Niagara Assets

| Asset | Type | Description |
|-------|------|-------------|
| NS_Blood_Splatter | System | Main system |
| NE_BloodEmitter | Emitter | Blood particle emitter |

## Parameter Presets

### Preset: Light Wound (Default)
- BurstCount: 50
- ExplosionForce: 0.8
- SprayAngle: 45
- DecalSize: 0.8
- DecalLifetime: 60

### Preset: Heavy Wound
- BurstCount: 100
- ExplosionForce: 1.5
- SprayAngle: 75
- DecalSize: 1.5
- DecalLifetime: 120

### Preset: Arterial Spray
- BurstCount: 150
- ExplosionForce: 2.0
- SprayAngle: 30
- DecalSize: 1.0
- DecalLifetime: 90

### Preset: Minimal Blood
- BurstCount: 25
- ExplosionForce: 0.6
- SprayAngle: 60
- DecalSize: 0.5
- DecalLifetime: 30
