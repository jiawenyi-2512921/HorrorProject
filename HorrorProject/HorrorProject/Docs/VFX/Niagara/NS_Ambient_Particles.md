# NS_Ambient_Particles - Ambient Environment Particles Implementation Guide

## System Overview

**Purpose**: General-purpose ambient particles for environmental atmosphere and depth enhancement.

**Use Cases**:
- Floating dust in light shafts
- Insects/fireflies
- Embers and ash
- Spores and pollen
- Underwater particles
- General atmosphere

**Performance Budget**:
- Max Particles: 200 concurrent
- Draw Calls: 1
- GPU Cost: ~0.4ms @ 1080p
- Memory: ~2MB

## Emitter Configuration

### Emitter Properties

**Emitter State**:
- Execution State: Active
- Execution State Source: Scalability
- Simulation Target: GPUSim
- Requires Persistent IDs: False
- Determinism: False

**Spawn Settings**:
- Spawn Rate: 30-60 particles/second
- Spawn Burst: 20 particles on activation
- Spawn Probability: 1.0

### Emitter Modules

#### 1. Initialize Particle

**Lifetime**:
- Mode: Random Range
- Minimum: 5.0 seconds
- Maximum: 12.0 seconds

**Color**:
- Mode: Random Range
- Color Min: (R: 0.5, G: 0.5, B: 0.5, A: 0.2)
- Color Max: (R: 1.0, G: 1.0, B: 1.0, A: 0.5)
- Color Mode: Linear Color

**Sprite Size**:
- Mode: Random Uniform
- Minimum: (X: 3.0, Y: 3.0)
- Maximum: (X: 12.0, Y: 12.0)
- Scale Mode: Uniform

**Sprite Rotation**:
- Mode: Random Range
- Minimum: 0.0 degrees
- Maximum: 360.0 degrees

**Mass**:
- Mass: Random Range (0.001, 0.01) kg

#### 2. Spawn Location

**Shape**: Box
- Box Size: (X: 600, Y: 600, Z: 400)
- Distribution Mode: Volume
- Surface Only: False

**Velocity**:
- Mode: Random Range
- Velocity Range: (-20, -20, -10) to (20, 20, 10)
- Velocity Mode: Set

#### 3. Update Modules

**Add Velocity**:
- Velocity: User Parameter (default: 0, 0, 5)
- Velocity Mode: Add to Existing

**Drag**:
- Drag Coefficient: 0.8 (moderate drag)
- Velocity Scale: 1.0

**Curl Noise Force**:
- Noise Strength: 40.0
- Noise Frequency: 0.3
- Noise Octaves: 2
- Noise Quality: Medium
- Noise Speed: 0.2

**Point Attraction Force** (optional):
- Attraction Point: User Parameter
- Attraction Strength: 0-100
- Falloff Radius: 500 units
- Falloff Exponent: 2.0

**Scale Sprite Size**:
- Scale Factor Curve:
  - 0.0s: 0.0
  - 0.5s: 1.0 (fade in)
  - Lifetime-1.0s: 1.0
  - Lifetime: 0.0 (fade out)

**Scale Color**:
- Alpha Curve:
  - 0.0s: 0.0
  - 0.8s: 1.0 (slow fade in)
  - Lifetime-1.5s: 1.0
  - Lifetime: 0.0 (fade out)

**Sprite Rotation Rate**:
- Rotation Rate: Random Range (-20, 20) degrees/second

**Camera Offset**:
- Camera Offset: -50 units
- Apply in Camera Space: True

**Color Over Life**:
- Color Curve: User Parameter
- Default: Constant color

#### 4. Collision

**Collision Module**:
- Collision Mode: Scene Depth
- Radius: 2.0
- Resilience: 0.5 (moderate bounce)
- Friction: 0.5
- Kill Particles on Collision: False

#### 5. Kill Particles

**Kill Box**:
- Box Size: (X: 700, Y: 700, Z: 500)
- Invert: True (kill outside box)

**Lifetime Kill**:
- Enabled: True

## Particle Attributes

### Required Attributes

| Attribute | Type | Default | Description |
|-----------|------|---------|-------------|
| Position | Vector3 | (0,0,0) | World position |
| Velocity | Vector3 | (0,0,0) | Movement velocity |
| Color | Linear Color | (0.8,0.8,0.8,0.3) | Particle tint |
| SpriteSize | Vector2 | (7,7) | Sprite dimensions |
| SpriteRotation | Float | 0.0 | Rotation angle |
| Age | Float | 0.0 | Current age |
| Lifetime | Float | 8.0 | Max lifetime |
| Mass | Float | 0.005 | Particle mass |

### User-Exposed Parameters

| Parameter | Type | Default | Range | Description |
|-----------|------|---------|-------|-------------|
| ParticleType | Int | 0 | 0-5 | Particle preset type |
| SpawnRate | Float | 45.0 | 10-120 | Particles per second |
| ParticleSize | Float | 1.0 | 0.3-3.0 | Size multiplier |
| DriftSpeed | Float | 1.0 | 0.3-3.0 | Movement speed multiplier |
| ParticleColor | Linear Color | (0.8,0.8,0.8,1) | - | Particle tint color |
| ParticleDensity | Float | 1.0 | 0.3-3.0 | Spawn rate multiplier |
| TurbulenceStrength | Float | 40.0 | 0-150 | Noise intensity |
| EnableAttraction | Bool | False | - | Enable point attraction |
| AttractionPoint | Vector3 | (0,0,0) | - | Attraction location |

## Material Configuration

### Material: M_AmbientParticle

**Material Domain**: Surface
**Blend Mode**: Translucent
**Shading Model**: Unlit

**Material Properties**:
- Two Sided: True
- Disable Depth Test: False
- Responsive AA: True
- Translucency Lighting Mode: Surface TranslucencyVolume

**Texture Inputs**:

1. **Particle Texture**: T_Ambient_D
   - Resolution: 256x256
   - Format: BC1
   - Compression: Default
   - Content: Soft particle shapes (4x4 flipbook)

2. **Particle Mask**: T_Ambient_O
   - Resolution: 256x256
   - Format: BC4
   - Compression: Grayscale
   - Content: Alpha masks

**Material Parameters**:

| Parameter | Type | Default | Description |
|-----------|------|---------|-------------|
| ParticleColor | Vector3 | (0.8,0.8,0.8) | Base color |
| Emissive | Scalar | 0.0 | Glow amount |
| Opacity | Scalar | 0.3 | Base opacity |
| DepthFade | Scalar | 50.0 | Soft particle distance |
| FlipbookFrame | Scalar | 0.0 | Texture frame |

**Material Graph**:
```
Texture Sample (T_Ambient_D + Flipbook)
  → Multiply (Particle Color)
  → Multiply (ParticleColor)
  → Base Color

Texture Sample (T_Ambient_O)
  → Multiply (Particle Alpha)
  → Multiply (DepthFade)
  → Opacity

Base Color
  → Multiply (Emissive)
  → Emissive Color
```

**Depth Fade Setup**:
```
PixelDepth
  → Subtract (SceneDepth)
  → Divide (DepthFade Distance)
  → Saturate
  → Multiply (Opacity)
```

## Particle Type Presets

### Type 0: Dust Motes (Default)
- Size: 3-8 units
- Color: (0.8, 0.8, 0.75)
- Opacity: 0.3
- Emissive: 0.0
- Drift: Slow upward
- Turbulence: Medium

### Type 1: Fireflies
- Size: 5-10 units
- Color: (1.0, 0.9, 0.5)
- Opacity: 0.8
- Emissive: 2.0
- Drift: Random
- Turbulence: High
- Flicker: Enabled

### Type 2: Embers
- Size: 4-12 units
- Color: (1.0, 0.4, 0.1)
- Opacity: 0.6
- Emissive: 3.0
- Drift: Upward
- Turbulence: Medium
- Fade: Darken over life

### Type 3: Ash
- Size: 3-10 units
- Color: (0.3, 0.3, 0.3)
- Opacity: 0.4
- Emissive: 0.0
- Drift: Slow downward
- Turbulence: High

### Type 4: Spores
- Size: 5-15 units
- Color: (0.6, 0.8, 0.5)
- Opacity: 0.5
- Emissive: 0.5
- Drift: Very slow
- Turbulence: Low

### Type 5: Underwater Particles
- Size: 4-10 units
- Color: (0.7, 0.8, 0.9)
- Opacity: 0.4
- Emissive: 0.2
- Drift: Slow upward
- Turbulence: Medium

## Performance Optimization

### LOD Configuration

**LOD 0** (0-1500 units):
- Spawn Rate: 45/second
- Max Particles: 200
- Sprite Size: 100%
- Update Rate: 60 FPS
- Noise Octaves: 2

**LOD 1** (1500-3000 units):
- Spawn Rate: 30/second
- Max Particles: 120
- Sprite Size: 120%
- Update Rate: 30 FPS
- Noise Octaves: 1

**LOD 2** (3000+ units):
- Spawn Rate: 15/second
- Max Particles: 60
- Sprite Size: 150%
- Update Rate: 15 FPS
- Noise Octaves: 1

### Scalability Settings

**Epic/High**:
- Full spawn rate
- 2 noise octaves
- Collision enabled
- Attraction enabled

**Medium**:
- 70% spawn rate
- 1 noise octave
- Collision enabled
- No attraction

**Low**:
- 50% spawn rate
- 1 noise octave
- No collision
- No attraction

**Cinematic**:
- 150% spawn rate
- 3 noise octaves
- Full collision
- Enhanced attraction

### Culling Configuration

**Distance Culling**:
- Max Draw Distance: 3500 units
- Cull Proxy Mode: None

**Visibility Culling**:
- Visibility Tag: "AmbientParticles"
- Cull When Not Rendered: True
- Cull Delay: 2.0 seconds

**Bounds**:
- Fixed Bounds: (X: 700, Y: 700, Z: 500)
- Use Fixed Bounds: True

## Collision and Physics

### Collision Settings

**Collision Type**: Scene Depth Buffer
- Query Type: Depth Buffer
- Collision Group: Environment
- Max Collision Iterations: 1

**Collision Response**:
- Resilience: 0.5 (moderate bounce)
- Friction: 0.5
- Velocity Retention: 0.5

**Collision Events**:
- On Collision: None
- Generate Collision Events: False

### Physics Simulation

**Forces**:
1. Gravity: User Parameter (default: -50 units/s²)
2. Drag: 0.8 coefficient
3. Curl Noise: 40 strength
4. Point Attraction: Optional

**Integration**:
- Integration Method: Euler
- Sub-Steps: 1
- Max Delta Time: 0.1 seconds

## Events and Lifecycle

### Spawn Events

**On Particle Spawn**:
- Initialize random rotation
- Set random lifetime
- Apply initial velocity
- Set spawn color
- Randomize flipbook frame

### Update Events

**On Particle Update** (every frame):
- Apply forces (gravity, drag, noise, attraction)
- Update rotation
- Scale size/alpha by age
- Check collision
- Update color over life

### Death Events

**On Particle Death**:
- No special handling
- Automatic cleanup

**Death Conditions**:
1. Lifetime expired
2. Outside kill box
3. Alpha < 0.01

## Testing and Validation

### Visual Tests

**Test 1: Particle Distribution**
- Verify even distribution
- Check no clustering
- Verify random placement
- Check coverage area

**Test 2: Movement Behavior**
- Observe drift patterns
- Verify turbulence natural
- Check velocity appropriate
- Verify no stuck particles

**Test 3: Particle Types**
- Test all 6 presets
- Verify colors correct
- Check sizes appropriate
- Verify emissive works

**Test 4: Collision**
- Test near walls/floor
- Verify bounce behavior
- Check no penetration
- Verify friction works

**Test 5: Performance**
- Monitor particle count
- Check GPU cost
- Verify LOD transitions
- Check culling works

### Performance Tests

**Test 1: Particle Count**
- Monitor with `stat particles`
- Verify max 200 particles
- Check spawn rate consistency
- Verify cleanup works

**Test 2: GPU Cost**
- Use `profilegpu` command
- Target: <0.4ms per emitter
- Check at different LODs
- Verify scalability works

**Test 3: Memory Usage**
- Check with `stat memory`
- Target: <2MB per emitter
- Verify no memory leaks
- Check texture memory

**Test 4: Multiple Emitters**
- Place 5+ emitters in level
- Verify total particle budget
- Check performance impact
- Verify no interference

### Integration Tests

**Test 1: Environment Integration**
- Place in various environments
- Verify particles fit atmosphere
- Check lighting interaction
- Verify proper placement

**Test 2: Dynamic Parameters**
- Change particle type at runtime
- Adjust spawn rate
- Modify color
- Verify smooth transitions

**Test 3: Level Streaming**
- Test with level streaming
- Verify emitters load correctly
- Check no orphaned particles
- Verify cleanup on unload

## Troubleshooting

### Common Issues

**Issue: Particles not visible**
- Check material assigned
- Verify spawn rate > 0
- Check opacity > 0
- Verify emitter active

**Issue: Too many/few particles**
- Adjust SpawnRate parameter
- Check lifetime settings
- Verify kill box size
- Check LOD settings

**Issue: Particles moving wrong**
- Check drift velocity
- Verify turbulence strength
- Check gravity setting
- Verify drag coefficient

**Issue: Performance problems**
- Reduce spawn rate
- Lower max particles
- Disable collision
- Reduce noise octaves

**Issue: Particles look flat**
- Increase turbulence
- Add point attraction
- Vary particle sizes
- Check depth sorting

**Issue: Particles disappear suddenly**
- Check kill box size
- Verify lifetime appropriate
- Check culling distance
- Verify bounds correct

## Best Practices

1. **Placement**: Attach to player or key locations
2. **Density**: Start subtle, increase gradually
3. **Color**: Match environment lighting
4. **Type**: Choose appropriate preset for environment
5. **Performance**: Always enable LOD and culling
6. **Turbulence**: Keep moderate for natural motion
7. **Collision**: Disable if not needed
8. **Scalability**: Test on low-end hardware
9. **Bounds**: Keep tight to visible area
10. **Cleanup**: Ensure proper destruction

## Asset Requirements

### Textures

| Asset | Resolution | Format | Size | Description |
|-------|------------|--------|------|-------------|
| T_Ambient_D | 256x256 | BC1 | ~65KB | Particle shapes flipbook |
| T_Ambient_O | 256x256 | BC4 | ~65KB | Alpha masks |

### Materials

| Asset | Type | Complexity | Description |
|-------|------|------------|-------------|
| M_AmbientParticle | Unlit | Low | Main particle material |
| MI_AmbientParticle_Inst | Instance | - | Runtime instance |

### Niagara Assets

| Asset | Type | Description |
|-------|------|-------------|
| NS_Ambient_Particles | System | Main system |
| NE_AmbientEmitter | Emitter | Ambient emitter |

## Parameter Presets

### Preset: Subtle Atmosphere (Default)
- ParticleType: 0 (Dust)
- SpawnRate: 30
- ParticleSize: 1.0
- DriftSpeed: 1.0
- ParticleDensity: 1.0
- TurbulenceStrength: 40

### Preset: Dense Atmosphere
- ParticleType: 0 (Dust)
- SpawnRate: 60
- ParticleSize: 1.2
- DriftSpeed: 0.8
- ParticleDensity: 1.5
- TurbulenceStrength: 60

### Preset: Firefly Swarm
- ParticleType: 1 (Fireflies)
- SpawnRate: 40
- ParticleSize: 1.0
- DriftSpeed: 1.5
- ParticleDensity: 1.2
- TurbulenceStrength: 80
- EnableAttraction: True

### Preset: Fire Embers
- ParticleType: 2 (Embers)
- SpawnRate: 50
- ParticleSize: 1.3
- DriftSpeed: 2.0
- ParticleDensity: 1.5
- TurbulenceStrength: 50

### Preset: Falling Ash
- ParticleType: 3 (Ash)
- SpawnRate: 45
- ParticleSize: 1.0
- DriftSpeed: 0.6
- ParticleDensity: 1.3
- TurbulenceStrength: 70

### Preset: Underwater Ambiance
- ParticleType: 5 (Underwater)
- SpawnRate: 35
- ParticleSize: 1.0
- DriftSpeed: 0.5
- ParticleDensity: 1.0
- TurbulenceStrength: 30

## Advanced Features

### Firefly Flicker Effect

**Implementation**:
```cpp
// Sine wave flicker for fireflies
float FlickerRate = 3.0f;
float FlickerAmount = 0.5f;
float Flicker = (sin(Age * FlickerRate) * 0.5f + 0.5f) * FlickerAmount + (1.0f - FlickerAmount);
Emissive *= Flicker;
```

### Point Attraction System

**Use Cases**:
- Fireflies attracted to light
- Particles swirling around point
- Vortex effects

**Implementation**:
```cpp
Vector3 ToAttractor = AttractionPoint - Position;
float Distance = ToAttractor.Length();
float Strength = AttractionStrength / (Distance * Distance);
Vector3 Force = ToAttractor.Normalized() * Strength;
Velocity += Force * DeltaTime;
```

### Color Gradient Over Life

**Ember Example**:
- 0.0s: Bright orange (1.0, 0.6, 0.2)
- 0.5s: Orange-red (1.0, 0.4, 0.1)
- 1.0s: Dark red (0.5, 0.1, 0.0)

### Seasonal Variations

**Spring**: Pollen/spores (green-yellow)
**Summer**: Fireflies (yellow-green)
**Autumn**: Falling leaves (orange-brown)
**Winter**: Snow particles (white-blue)
