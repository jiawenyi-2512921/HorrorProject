# NS_Flashlight_Beam - Flashlight Volumetric Beam Implementation Guide

## System Overview

**Purpose**: Volumetric light beam effect for flashlight with visible dust particles in beam.

**Use Cases**:
- Player flashlight beam
- Searchlight effects
- Volumetric lighting
- Atmospheric light shafts

**Performance Budget**:
- Max Particles: 80 concurrent
- Draw Calls: 1
- GPU Cost: ~0.4ms @ 1080p
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
- Spawn Rate: 40-60 particles/second
- Spawn Burst: 10 particles on activation
- Spawn Probability: 1.0

### Emitter Modules

#### 1. Initialize Particle

**Lifetime**:
- Mode: Random Range
- Minimum: 0.8 seconds
- Maximum: 1.5 seconds

**Color**:
- Mode: Direct Set
- Color: (R: 1.0, G: 0.95, B: 0.85, A: 0.4)
- Color Mode: Linear Color

**Sprite Size**:
- Mode: Random Uniform
- Minimum: (X: 3.0, Y: 3.0)
- Maximum: (X: 8.0, Y: 8.0)
- Scale Mode: Uniform

**Sprite Rotation**:
- Mode: Random Range
- Minimum: 0.0 degrees
- Maximum: 360.0 degrees

**Mass**:
- Mass: 0.005 kg

#### 2. Spawn Location

**Shape**: Cone
- Cone Angle: 15 degrees (adjustable)
- Cone Radius: 10 units (base)
- Cone Height: 800 units (beam length)
- Distribution Mode: Volume
- Surface Only: False

**Velocity**:
- Mode: Cone
- Cone Axis: (X: 1, Y: 0, Z: 0) - forward
- Cone Angle: 5 degrees
- Velocity Magnitude: 50-100 units/second

#### 3. Update Modules

**Add Velocity**:
- Velocity: (X: 200, Y: 0, Z: 0) - forward motion
- Velocity Mode: Add to Existing

**Drag**:
- Drag Coefficient: 0.3
- Velocity Scale: 1.0

**Curl Noise Force**:
- Noise Strength: 20.0
- Noise Frequency: 1.0
- Noise Octaves: 1
- Noise Quality: Low

**Scale Sprite Size**:
- Scale Factor Curve:
  - 0.0s: 0.5
  - 0.2s: 1.0
  - Lifetime-0.2s: 1.0
  - Lifetime: 0.3

**Scale Color**:
- Alpha Curve:
  - 0.0s: 0.0
  - 0.1s: 1.0
  - Lifetime-0.3s: 1.0
  - Lifetime: 0.0

**Sprite Rotation Rate**:
- Rotation Rate: Random Range (-45, 45) degrees/second

**Camera Offset**:
- Camera Offset: -20 units
- Apply in Camera Space: False

**Distance Fade**:
- Fade Start Distance: 0 units
- Fade End Distance: 800 units
- Fade Curve: Linear

#### 4. Collision

**Collision Module**:
- Collision Mode: Scene Depth
- Radius: 2.0
- Resilience: 0.0 (no bounce)
- Friction: 1.0
- Kill Particles on Collision: True

#### 5. Kill Particles

**Kill Cone**:
- Cone Angle: 20 degrees
- Cone Height: 850 units
- Invert: True (kill outside cone)

**Lifetime Kill**:
- Enabled: True

## Particle Attributes

### Required Attributes

| Attribute | Type | Default | Description |
|-----------|------|---------|-------------|
| Position | Vector3 | (0,0,0) | World position |
| Velocity | Vector3 | (200,0,0) | Movement velocity |
| Color | Linear Color | (1,0.95,0.85,0.4) | Particle tint |
| SpriteSize | Vector2 | (5,5) | Sprite dimensions |
| SpriteRotation | Float | 0.0 | Rotation angle |
| Age | Float | 0.0 | Current age |
| Lifetime | Float | 1.0 | Max lifetime |
| Mass | Float | 0.005 | Particle mass |
| DistanceFromSource | Float | 0.0 | Distance traveled |

### User-Exposed Parameters

| Parameter | Type | Default | Range | Description |
|-----------|------|---------|-------|-------------|
| BeamIntensity | Float | 1.0 | 0.3-2.0 | Brightness multiplier |
| BeamColor | Linear Color | (1,0.95,0.85,1) | - | Beam color tint |
| BeamLength | Float | 800.0 | 300-1500 | Maximum beam distance |
| BeamAngle | Float | 15.0 | 10-30 | Cone spread angle |
| ParticleDensity | Float | 1.0 | 0.5-2.0 | Spawn rate multiplier |
| BeamFlicker | Float | 0.0 | 0-0.3 | Intensity flicker amount |

## Material Configuration

### Material: M_FlashlightBeam

**Material Domain**: Surface
**Blend Mode**: Additive
**Shading Model**: Unlit

**Material Properties**:
- Two Sided: True
- Disable Depth Test: False
- Responsive AA: True
- Translucency Lighting Mode: Surface TranslucencyVolume

**Texture Inputs**:

1. **Particle Texture**: T_Dust_Soft
   - Resolution: 128x128
   - Format: BC1
   - Compression: Default
   - Content: Soft circular gradient

**Material Parameters**:

| Parameter | Type | Default | Description |
|-----------|------|---------|-------------|
| BeamColor | Vector3 | (1,0.95,0.85) | Beam color |
| Intensity | Scalar | 1.0 | Brightness |
| DepthFade | Scalar | 30.0 | Soft particle distance |
| DistanceFade | Scalar | 800.0 | Beam length fade |

**Material Graph**:
```
Texture Sample (T_Dust_Soft)
  → Multiply (Particle Color)
  → Multiply (BeamColor)
  → Multiply (Intensity)
  → Multiply (DepthFade)
  → Multiply (DistanceFade)
  → Emissive Color

Particle Alpha
  → Multiply (DepthFade)
  → Multiply (DistanceFade)
  → Opacity
```

**Depth Fade Setup**:
```
PixelDepth
  → Subtract (SceneDepth)
  → Divide (DepthFade Distance)
  → Saturate
  → Power (2.0)
```

**Distance Fade Setup**:
```
DistanceFromSource (Particle Attribute)
  → Divide (BeamLength)
  → OneMinus
  → Saturate
  → Power (1.5)
```

## Performance Optimization

### LOD Configuration

**LOD 0** (0-1000 units):
- Spawn Rate: 50/second
- Max Particles: 80
- Sprite Size: 100%
- Update Rate: 60 FPS

**LOD 1** (1000-2000 units):
- Spawn Rate: 30/second
- Max Particles: 50
- Sprite Size: 120%
- Update Rate: 30 FPS

**LOD 2** (2000+ units):
- Spawn Rate: 15/second
- Max Particles: 25
- Sprite Size: 150%
- Update Rate: 15 FPS

### Scalability Settings

**Epic/High**:
- Full spawn rate
- Collision enabled
- Curl noise enabled

**Medium**:
- 70% spawn rate
- Collision enabled
- No curl noise

**Low**:
- 50% spawn rate
- Collision disabled
- No curl noise

**Cinematic**:
- 150% spawn rate
- Full collision
- Enhanced curl noise

### Culling Configuration

**Distance Culling**:
- Max Draw Distance: 2500 units
- Cull Proxy Mode: None

**Visibility Culling**:
- Visibility Tag: "FlashlightBeam"
- Cull When Not Rendered: True
- Cull Delay: 0.5 seconds

**Bounds**:
- Fixed Bounds: Cone shape (dynamic)
- Use Fixed Bounds: False

## Collision and Physics

### Collision Settings

**Collision Type**: Scene Depth Buffer
- Query Type: Depth Buffer
- Collision Group: Environment
- Max Collision Iterations: 1

**Collision Response**:
- Kill on Collision: True
- Generate Collision Events: False

### Physics Simulation

**Forces**:
1. Forward Velocity: 200 units/s
2. Drag: 0.3 coefficient
3. Curl Noise: 20 strength (subtle)

**Integration**:
- Integration Method: Euler
- Sub-Steps: 1
- Max Delta Time: 0.1 seconds

## Events and Lifecycle

### Spawn Events

**On Particle Spawn**:
- Initialize random rotation
- Set random lifetime
- Apply forward velocity
- Set spawn color
- Initialize distance tracker

### Update Events

**On Particle Update** (every frame):
- Apply forward velocity
- Apply drag
- Update rotation
- Scale size/alpha by age
- Update distance from source
- Check collision
- Apply distance fade

### Death Events

**On Particle Death**:
- No special handling
- Automatic cleanup

**Death Conditions**:
1. Lifetime expired
2. Collision with geometry
3. Outside cone bounds
4. Alpha < 0.01

## Testing and Validation

### Visual Tests

**Test 1: Beam Shape**
- Verify cone shape correct
- Check beam angle matches setting
- Verify smooth distribution
- Check no gaps in beam

**Test 2: Beam Length**
- Verify beam reaches target distance
- Check fade at end is smooth
- Verify collision stops beam
- Check no particles beyond collision

**Test 3: Beam Movement**
- Attach to moving object
- Verify particles follow correctly
- Check no lag or trailing
- Verify smooth motion

**Test 4: Lighting Interaction**
- Test in dark environment
- Verify beam is visible
- Check color matches setting
- Verify proper brightness

**Test 5: Collision**
- Point at walls/objects
- Verify beam stops at surface
- Check no penetration
- Verify smooth cutoff

### Performance Tests

**Test 1: Particle Count**
- Monitor with `stat particles`
- Verify max 80 particles
- Check spawn rate consistency
- Verify cleanup works

**Test 2: GPU Cost**
- Use `profilegpu` command
- Target: <0.4ms per beam
- Check additive blend cost
- Verify scalability works

**Test 3: Memory Usage**
- Check with `stat memory`
- Target: <2MB per beam
- Verify no memory leaks
- Check texture memory

**Test 4: Multiple Beams**
- Test with 3+ flashlights
- Check performance impact
- Verify no interference
- Check total particle budget

### Integration Tests

**Test 1: Flashlight Attachment**
- Attach to player camera
- Verify beam follows aim
- Check rotation smooth
- Verify no jitter

**Test 2: Dynamic Parameters**
- Change intensity at runtime
- Adjust beam angle
- Modify color
- Verify smooth transitions

**Test 3: On/Off Toggle**
- Toggle beam on/off rapidly
- Verify no crashes
- Check cleanup works
- Verify no orphaned particles

## Troubleshooting

### Common Issues

**Issue: Beam not visible**
- Check material is assigned
- Verify intensity > 0
- Check beam color not black
- Verify emitter is active

**Issue: Beam too short/long**
- Adjust BeamLength parameter
- Check particle lifetime
- Verify forward velocity
- Check kill cone size

**Issue: Beam too wide/narrow**
- Adjust BeamAngle parameter
- Check cone spawn settings
- Verify particle distribution
- Check kill cone angle

**Issue: Beam penetrates walls**
- Enable collision module
- Check scene depth available
- Verify collision channel
- Increase collision radius

**Issue: Performance problems**
- Reduce spawn rate
- Lower max particles
- Disable collision
- Reduce beam length

**Issue: Beam flickers**
- Check spawn rate sufficient
- Verify particle lifetime
- Check no z-fighting
- Verify depth fade working

## Best Practices

1. **Attachment**: Always attach to camera or light source
2. **Direction**: Use forward vector of parent
3. **Color**: Match light source color
4. **Intensity**: Adjust based on environment brightness
5. **Performance**: Limit to 1-2 active beams
6. **Collision**: Always enable for realism
7. **Length**: Keep reasonable (500-1000 units)
8. **Angle**: Narrow for flashlight (10-20°)
9. **Cleanup**: Destroy when light turns off
10. **Scalability**: Test on low-end hardware

## Asset Requirements

### Textures

| Asset | Resolution | Format | Size | Description |
|-------|------------|--------|------|-------------|
| T_Dust_Soft | 128x128 | BC1 | ~10KB | Soft particle texture |

### Materials

| Asset | Type | Complexity | Description |
|-------|------|------------|-------------|
| M_FlashlightBeam | Unlit | Low | Main beam material |
| MI_FlashlightBeam_Inst | Instance | - | Runtime instance |

### Niagara Assets

| Asset | Type | Description |
|-------|------|-------------|
| NS_Flashlight_Beam | System | Main system |
| NE_BeamEmitter | Emitter | Beam emitter |

## Parameter Presets

### Preset: Standard Flashlight (Default)
- BeamIntensity: 1.0
- BeamLength: 800
- BeamAngle: 15
- ParticleDensity: 1.0
- BeamFlicker: 0.0

### Preset: Weak Flashlight
- BeamIntensity: 0.6
- BeamLength: 500
- BeamAngle: 12
- ParticleDensity: 0.7
- BeamFlicker: 0.1

### Preset: Searchlight
- BeamIntensity: 1.5
- BeamLength: 1500
- BeamAngle: 25
- ParticleDensity: 1.5
- BeamFlicker: 0.0

### Preset: Dying Battery
- BeamIntensity: 0.4
- BeamLength: 300
- BeamAngle: 10
- ParticleDensity: 0.5
- BeamFlicker: 0.3
