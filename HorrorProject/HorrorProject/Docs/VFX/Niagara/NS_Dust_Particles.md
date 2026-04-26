# NS_Dust_Particles - Dust Motes Implementation Guide

## System Overview

**Purpose**: Atmospheric dust particles floating in air to enhance environmental depth and lighting visibility.

**Use Cases**:
- Indoor ambient atmosphere
- Light shaft visualization
- Abandoned/dusty environments
- Subtle environmental storytelling

**Performance Budget**:
- Max Particles: 150 concurrent
- Draw Calls: 1
- GPU Cost: ~0.3ms @ 1080p
- Memory: ~2MB

## Emitter Configuration

### Emitter Properties

**Emitter State**:
- Execution State: Active
- Execution State Source: Scalability
- Simulation Target: CPUSim
- Requires Persistent IDs: False
- Determinism: False

**Spawn Settings**:
- Spawn Rate: 20-50 particles/second
- Spawn Burst: None (continuous)
- Spawn Probability: 1.0

### Emitter Modules

#### 1. Initialize Particle

**Lifetime**:
- Mode: Random Range
- Minimum: 3.0 seconds
- Maximum: 8.0 seconds

**Color**:
- Mode: Direct Set
- Color: (R: 0.8, G: 0.8, B: 0.75, A: 0.3)
- Color Mode: Linear Color

**Sprite Size**:
- Mode: Random Uniform
- Minimum: (X: 2.0, Y: 2.0)
- Maximum: (X: 8.0, Y: 8.0)
- Scale Mode: Uniform

**Sprite Rotation**:
- Mode: Random Range
- Minimum: 0.0 degrees
- Maximum: 360.0 degrees

**Mass**:
- Mass: 0.01 kg (very light)

#### 2. Spawn Location

**Shape**: Box
- Box Size: (X: 500, Y: 500, Z: 300)
- Distribution Mode: Random
- Surface Only: False

**Velocity**:
- Mode: Random Range in Cone
- Cone Axis: (X: 0, Y: 0, Z: 1)
- Cone Angle: 45 degrees
- Velocity Magnitude: 10-30 units/second

#### 3. Update Modules

**Add Velocity**:
- Velocity: (X: 0, Y: 0, Z: 5) - slight upward drift
- Velocity Mode: Add to Existing

**Drag**:
- Drag Coefficient: 0.5
- Velocity Scale: 1.0

**Curl Noise Force**:
- Noise Strength: 50.0
- Noise Frequency: 0.5
- Noise Octaves: 2
- Noise Quality: Medium

**Scale Sprite Size**:
- Scale Factor Curve:
  - 0.0s: 0.0 (fade in)
  - 0.5s: 1.0 (full size)
  - Lifetime-0.5s: 1.0
  - Lifetime: 0.0 (fade out)

**Scale Color**:
- Alpha Curve:
  - 0.0s: 0.0
  - 0.3s: 1.0
  - Lifetime-0.5s: 1.0
  - Lifetime: 0.0

**Sprite Rotation Rate**:
- Rotation Rate: Random Range (-30, 30) degrees/second

**Camera Offset**:
- Camera Offset: -50 units (prevent near-clip)
- Apply in Camera Space: True

#### 4. Collision

**Collision Module**:
- Collision Mode: Scene Depth
- Radius: 2.0
- Resilience: 0.3 (slight bounce)
- Friction: 0.8
- Kill Particles on Collision: False

#### 5. Kill Particles

**Kill Box**:
- Box Size: (X: 600, Y: 600, Z: 400)
- Invert: True (kill outside box)

**Lifetime Kill**:
- Enabled: True

## Particle Attributes

### Required Attributes

| Attribute | Type | Default | Description |
|-----------|------|---------|-------------|
| Position | Vector3 | (0,0,0) | World position |
| Velocity | Vector3 | (0,0,0) | Movement velocity |
| Color | Linear Color | (0.8,0.8,0.75,0.3) | Particle tint |
| SpriteSize | Vector2 | (5,5) | Sprite dimensions |
| SpriteRotation | Float | 0.0 | Rotation angle |
| Age | Float | 0.0 | Current age |
| Lifetime | Float | 5.0 | Max lifetime |
| Mass | Float | 0.01 | Particle mass |

### User-Exposed Parameters

| Parameter | Type | Default | Range | Description |
|-----------|------|---------|-------|-------------|
| SpawnRate | Float | 35.0 | 10-100 | Particles per second |
| ParticleSize | Float | 1.0 | 0.5-2.0 | Size multiplier |
| DriftSpeed | Float | 1.0 | 0.5-3.0 | Movement speed multiplier |
| DustColor | Linear Color | (0.8,0.8,0.75,1) | - | Dust tint color |
| DustDensity | Float | 1.0 | 0.5-2.0 | Spawn rate multiplier |
| NoiseStrength | Float | 50.0 | 0-200 | Turbulence intensity |

## Material Configuration

### Material: M_DustParticle

**Material Domain**: Surface
**Blend Mode**: Translucent
**Shading Model**: Unlit

**Material Properties**:
- Two Sided: True
- Disable Depth Test: False
- Responsive AA: True
- Translucency Lighting Mode: Surface TranslucencyVolume

**Texture Inputs**:

1. **Base Texture**: T_Dust_D
   - Resolution: 128x128
   - Format: BC1 (no alpha)
   - Compression: Default
   - Mip Gen Settings: SimpleAverage
   - Content: Soft circular gradient

2. **Opacity Texture**: T_Dust_O
   - Resolution: 128x128
   - Format: BC4 (grayscale)
   - Compression: Grayscale
   - Content: Soft alpha gradient

**Material Parameters**:

| Parameter | Type | Default | Description |
|-----------|------|---------|-------------|
| BaseColor | Vector3 | (0.8,0.8,0.75) | Dust color |
| Opacity | Scalar | 0.3 | Base opacity |
| EmissiveStrength | Scalar | 0.1 | Slight glow in light |
| DepthFade | Scalar | 50.0 | Soft particle fade distance |

**Material Graph**:
```
Texture Sample (T_Dust_D)
  → Multiply (Particle Color)
  → Base Color

Texture Sample (T_Dust_O)
  → Multiply (Particle Alpha)
  → Multiply (DepthFade)
  → Opacity

Particle Color (RGB)
  → Multiply (EmissiveStrength)
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

## Performance Optimization

### LOD Configuration

**LOD 0** (0-1000 units):
- Spawn Rate: 35/second
- Max Particles: 150
- Sprite Size: 100%
- Update Rate: 60 FPS

**LOD 1** (1000-2000 units):
- Spawn Rate: 20/second
- Max Particles: 80
- Sprite Size: 80%
- Update Rate: 30 FPS

**LOD 2** (2000+ units):
- Spawn Rate: 10/second
- Max Particles: 40
- Sprite Size: 60%
- Update Rate: 15 FPS

### Scalability Settings

**Epic/High**:
- Full spawn rate
- Curl noise enabled
- Collision enabled

**Medium**:
- 70% spawn rate
- Curl noise simplified (1 octave)
- Collision enabled

**Low**:
- 50% spawn rate
- No curl noise
- Collision disabled

**Cinematic**:
- 150% spawn rate
- Enhanced curl noise (3 octaves)
- Full collision

### Culling Configuration

**Distance Culling**:
- Max Draw Distance: 2500 units
- Cull Proxy Mode: None

**Visibility Culling**:
- Visibility Tag: "DustParticles"
- Cull When Not Rendered: True
- Cull Delay: 2.0 seconds

**Bounds**:
- Fixed Bounds: (X: 600, Y: 600, Z: 400)
- Use Fixed Bounds: True

## Collision and Physics

### Collision Settings

**Collision Type**: Scene Depth Buffer
- Query Type: Depth Buffer
- Collision Group: Environment
- Max Collision Iterations: 2

**Collision Response**:
- Resilience: 0.3 (slight bounce)
- Friction: 0.8 (high friction)
- Velocity Retention: 0.5

**Collision Events**:
- On Collision: None
- Generate Collision Events: False

### Physics Simulation

**Forces**:
1. Gravity: -980 units/s² (Z-axis)
2. Drag: 0.5 coefficient
3. Curl Noise: 50 strength
4. Wind: User parameter (optional)

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

### Update Events

**On Particle Update** (every frame):
- Apply forces (gravity, drag, noise)
- Update rotation
- Scale size/alpha by age
- Check collision
- Check bounds

### Death Events

**On Particle Death**:
- No special handling
- Automatic cleanup

**Death Conditions**:
1. Lifetime expired
2. Outside kill box
3. Alpha < 0.01 (fully faded)

## Testing and Validation

### Visual Tests

**Test 1: Spawn Verification**
- Place emitter in empty room
- Verify particles spawn continuously
- Check spawn rate matches setting
- Verify random distribution

**Test 2: Movement Behavior**
- Observe drift patterns
- Verify curl noise creates natural motion
- Check upward drift is subtle
- Verify no particles stuck

**Test 3: Lighting Interaction**
- Place in light shaft
- Verify particles visible in light
- Check color matches environment
- Verify no over-brightness

**Test 4: Collision**
- Place near walls/floor
- Verify particles bounce correctly
- Check no particles penetrate geometry
- Verify friction works

### Performance Tests

**Test 1: Particle Count**
- Monitor with `stat particles`
- Verify max 150 particles
- Check spawn rate consistency
- Verify cleanup works

**Test 2: GPU Cost**
- Use `profilegpu` command
- Target: <0.3ms per emitter
- Check at different LODs
- Verify scalability works

**Test 3: Memory Usage**
- Check with `stat memory`
- Target: <2MB per emitter
- Verify no memory leaks
- Check texture memory

**Test 4: Distance Culling**
- Move camera away from emitter
- Verify culling at 2500 units
- Check no pop-in/pop-out
- Verify performance improvement

### Integration Tests

**Test 1: Multiple Emitters**
- Place 5+ emitters in level
- Verify total particle budget
- Check performance impact
- Verify no interference

**Test 2: Dynamic Spawning**
- Spawn/destroy emitters at runtime
- Verify no crashes
- Check memory cleanup
- Verify parameter changes work

**Test 3: Level Streaming**
- Test with level streaming
- Verify emitters load correctly
- Check no orphaned particles
- Verify cleanup on unload

## Troubleshooting

### Common Issues

**Issue: Particles not visible**
- Check material is assigned
- Verify opacity > 0
- Check camera is in range
- Verify emitter is active

**Issue: Too many/few particles**
- Adjust SpawnRate parameter
- Check lifetime settings
- Verify kill box size
- Check LOD settings

**Issue: Particles moving too fast/slow**
- Adjust DriftSpeed parameter
- Check velocity settings
- Verify drag coefficient
- Check noise strength

**Issue: Performance problems**
- Reduce spawn rate
- Lower max particles
- Disable collision
- Reduce curl noise octaves

**Issue: Particles penetrating walls**
- Enable collision module
- Increase collision radius
- Check collision channel
- Verify scene depth available

**Issue: Unnatural movement**
- Adjust curl noise settings
- Add more drag
- Reduce velocity
- Check force balance

## Best Practices

1. **Placement**: Attach to player camera for consistent coverage
2. **Density**: Use sparingly - less is more for atmosphere
3. **Color**: Match environment lighting temperature
4. **Size**: Vary size for depth perception
5. **Performance**: Always enable LOD and culling
6. **Lighting**: Works best in areas with visible light shafts
7. **Collision**: Disable if not needed for performance
8. **Scalability**: Test on low-end hardware
9. **Bounds**: Keep tight to emitter area
10. **Cleanup**: Ensure proper destruction on level unload

## Asset Requirements

### Textures

| Asset | Resolution | Format | Size | Description |
|-------|------------|--------|------|-------------|
| T_Dust_D | 128x128 | BC1 | ~10KB | Dust diffuse |
| T_Dust_O | 128x128 | BC4 | ~10KB | Dust opacity |

### Materials

| Asset | Type | Complexity | Description |
|-------|------|------------|-------------|
| M_DustParticle | Unlit | Low | Main dust material |
| MI_DustParticle_Inst | Instance | - | Runtime instance |

### Niagara Assets

| Asset | Type | Description |
|-------|------|-------------|
| NS_Dust_Particles | System | Main system |
| NE_DustEmitter | Emitter | Dust emitter |

## Parameter Presets

### Preset: Light Dust (Default)
- SpawnRate: 35
- ParticleSize: 1.0
- DriftSpeed: 1.0
- DustDensity: 1.0
- NoiseStrength: 50

### Preset: Heavy Dust
- SpawnRate: 60
- ParticleSize: 1.5
- DriftSpeed: 0.8
- DustDensity: 1.5
- NoiseStrength: 70

### Preset: Minimal Dust
- SpawnRate: 15
- ParticleSize: 0.8
- DriftSpeed: 1.2
- DustDensity: 0.5
- NoiseStrength: 30

### Preset: Cinematic Dust
- SpawnRate: 80
- ParticleSize: 1.2
- DriftSpeed: 0.6
- DustDensity: 2.0
- NoiseStrength: 100
