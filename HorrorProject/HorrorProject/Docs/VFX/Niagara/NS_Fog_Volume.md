# NS_Fog_Volume - Volumetric Fog Implementation Guide

## System Overview

**Purpose**: Volumetric fog effect for atmospheric depth, horror ambiance, and environmental storytelling.

**Use Cases**:
- Corridor fog layers
- Room atmosphere
- Underwater murk
- Horror tension building

**Performance Budget**:
- Max Particles: 200 concurrent
- Draw Calls: 1
- GPU Cost: ~0.8ms @ 1080p
- Memory: ~4MB

## Emitter Configuration

### Emitter Properties

**Emitter State**:
- Execution State: Active
- Execution State Source: Scalability
- Simulation Target: GPUSim
- Requires Persistent IDs: False
- Determinism: False

**Spawn Settings**:
- Spawn Rate: 15-30 particles/second
- Spawn Burst: 20 particles on activation
- Spawn Probability: 1.0

### Emitter Modules

#### 1. Initialize Particle

**Lifetime**:
- Mode: Random Range
- Minimum: 8.0 seconds
- Maximum: 15.0 seconds

**Color**:
- Mode: Direct Set
- Color: (R: 0.6, G: 0.65, B: 0.7, A: 0.15)
- Color Mode: Linear Color

**Sprite Size**:
- Mode: Random Uniform
- Minimum: (X: 100.0, Y: 100.0)
- Maximum: (X: 250.0, Y: 250.0)
- Scale Mode: Uniform

**Sprite Rotation**:
- Mode: Random Range
- Minimum: 0.0 degrees
- Maximum: 360.0 degrees

**Mass**:
- Mass: 0.001 kg (extremely light)

#### 2. Spawn Location

**Shape**: Box
- Box Size: (X: 800, Y: 800, Z: 400)
- Distribution Mode: Volume
- Surface Only: False

**Velocity**:
- Mode: Random Range
- Velocity Range: (-10, -10, -5) to (10, 10, 5)
- Velocity Mode: Set

#### 3. Update Modules

**Add Velocity**:
- Velocity: (X: 0, Y: 0, Z: 2) - very slow rise
- Velocity Mode: Add to Existing

**Drag**:
- Drag Coefficient: 1.5 (heavy drag)
- Velocity Scale: 1.0

**Curl Noise Force**:
- Noise Strength: 30.0
- Noise Frequency: 0.2
- Noise Octaves: 3
- Noise Quality: High
- Noise Speed: 0.1

**Scale Sprite Size**:
- Scale Factor Curve:
  - 0.0s: 0.3 (small start)
  - 2.0s: 1.0 (grow)
  - Lifetime-2.0s: 1.2 (continue growing)
  - Lifetime: 1.5 (large end)

**Scale Color**:
- Alpha Curve:
  - 0.0s: 0.0
  - 1.5s: 1.0 (fade in)
  - Lifetime-3.0s: 1.0
  - Lifetime: 0.0 (fade out)

**Sprite Rotation Rate**:
- Rotation Rate: Random Range (-10, 10) degrees/second

**Camera Offset**:
- Camera Offset: -100 units
- Apply in Camera Space: True

**Depth Sorting**:
- Sort Mode: View Depth
- Sort Precision: High

#### 4. Collision

**Collision Module**: Disabled (fog passes through)

#### 5. Kill Particles

**Kill Box**:
- Box Size: (X: 1000, Y: 1000, Z: 600)
- Invert: True (kill outside box)

**Lifetime Kill**:
- Enabled: True

## Particle Attributes

### Required Attributes

| Attribute | Type | Default | Description |
|-----------|------|---------|-------------|
| Position | Vector3 | (0,0,0) | World position |
| Velocity | Vector3 | (0,0,2) | Movement velocity |
| Color | Linear Color | (0.6,0.65,0.7,0.15) | Fog tint |
| SpriteSize | Vector2 | (150,150) | Sprite dimensions |
| SpriteRotation | Float | 0.0 | Rotation angle |
| Age | Float | 0.0 | Current age |
| Lifetime | Float | 10.0 | Max lifetime |
| Mass | Float | 0.001 | Particle mass |

### User-Exposed Parameters

| Parameter | Type | Default | Range | Description |
|-----------|------|---------|-------|-------------|
| FogDensity | Float | 1.0 | 0.3-3.0 | Spawn rate multiplier |
| FogThickness | Float | 1.0 | 0.5-2.0 | Opacity multiplier |
| FogColor | Linear Color | (0.6,0.65,0.7,1) | - | Fog tint color |
| FogHeight | Float | 200.0 | 100-500 | Vertical spread |
| DriftSpeed | Float | 1.0 | 0.3-2.0 | Movement speed |
| TurbulenceStrength | Float | 30.0 | 0-100 | Noise intensity |
| FogScale | Float | 1.0 | 0.5-3.0 | Particle size multiplier |

## Material Configuration

### Material: M_VolumetricFog

**Material Domain**: Surface
**Blend Mode**: Translucent
**Shading Model**: Unlit

**Material Properties**:
- Two Sided: True
- Disable Depth Test: False
- Responsive AA: True
- Translucency Lighting Mode: Surface ForwardShading
- Translucent Sort Priority: 10

**Texture Inputs**:

1. **Fog Texture**: T_Fog_Noise_D
   - Resolution: 512x512
   - Format: BC1
   - Compression: Default
   - Mip Gen Settings: Blur5
   - Content: Perlin noise pattern

2. **Opacity Texture**: T_Fog_Opacity
   - Resolution: 256x256
   - Format: BC4
   - Compression: Grayscale
   - Content: Soft radial gradient

**Material Parameters**:

| Parameter | Type | Default | Description |
|-----------|------|---------|-------------|
| FogColor | Vector3 | (0.6,0.65,0.7) | Base fog color |
| Opacity | Scalar | 0.15 | Base opacity |
| DepthFade | Scalar | 100.0 | Soft particle distance |
| NoiseScale | Scalar | 2.0 | Texture tiling |
| NoisePan | Vector2 | (0.02,0.01) | Texture scroll speed |
| LightAbsorption | Scalar | 0.8 | Light penetration |

**Material Graph**:
```
Texture Sample (T_Fog_Noise_D + Panner)
  → Multiply (Particle Color)
  → Lerp (FogColor, 0.5)
  → Base Color

Texture Sample (T_Fog_Opacity)
  → Multiply (Particle Alpha)
  → Multiply (DepthFade)
  → Multiply (CameraFade)
  → Opacity

SceneColor
  → Multiply (LightAbsorption)
  → Add (Base Color)
  → Final Color
```

**Depth Fade Setup**:
```
PixelDepth
  → Subtract (SceneDepth)
  → Divide (DepthFade Distance)
  → Saturate
  → Multiply (Opacity)
```

**Camera Fade Setup**:
```
CameraPosition
  → Distance (PixelWorldPosition)
  → Divide (200)
  → Saturate
  → Multiply (Opacity)
```

## Performance Optimization

### LOD Configuration

**LOD 0** (0-1500 units):
- Spawn Rate: 25/second
- Max Particles: 200
- Sprite Size: 100%
- Update Rate: 60 FPS
- Noise Octaves: 3

**LOD 1** (1500-3000 units):
- Spawn Rate: 15/second
- Max Particles: 100
- Sprite Size: 120%
- Update Rate: 30 FPS
- Noise Octaves: 2

**LOD 2** (3000+ units):
- Spawn Rate: 8/second
- Max Particles: 50
- Sprite Size: 150%
- Update Rate: 15 FPS
- Noise Octaves: 1

### Scalability Settings

**Epic/High**:
- Full spawn rate
- 3 noise octaves
- High quality sorting
- Full depth fade

**Medium**:
- 70% spawn rate
- 2 noise octaves
- Medium sorting
- Simplified depth fade

**Low**:
- 50% spawn rate
- 1 noise octave
- Low sorting
- No depth fade

**Cinematic**:
- 150% spawn rate
- 4 noise octaves
- Highest quality sorting
- Enhanced depth fade

### Culling Configuration

**Distance Culling**:
- Max Draw Distance: 3500 units
- Cull Proxy Mode: None

**Visibility Culling**:
- Visibility Tag: "VolumetricFog"
- Cull When Not Rendered: True
- Cull Delay: 3.0 seconds

**Bounds**:
- Fixed Bounds: (X: 1000, Y: 1000, Z: 600)
- Use Fixed Bounds: True

## Collision and Physics

### Collision Settings

**Collision Type**: None (fog passes through geometry)

### Physics Simulation

**Forces**:
1. Gravity: -50 units/s² (very weak)
2. Drag: 1.5 coefficient (heavy)
3. Curl Noise: 30 strength
4. Buoyancy: +2 units/s² (slow rise)

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
- Randomize noise offset

### Update Events

**On Particle Update** (every frame):
- Apply forces (buoyancy, drag, noise)
- Update rotation slowly
- Scale size over lifetime
- Scale alpha over lifetime
- Update noise panning

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

**Test 1: Fog Coverage**
- Place emitter in corridor
- Verify fog fills space evenly
- Check no gaps or holes
- Verify smooth distribution

**Test 2: Fog Movement**
- Observe drift patterns
- Verify slow, natural motion
- Check curl noise creates wisps
- Verify no sudden movements

**Test 3: Depth Perception**
- View fog from multiple angles
- Verify depth sorting works
- Check no flickering
- Verify proper layering

**Test 4: Lighting Interaction**
- Test with various lights
- Verify fog receives lighting
- Check color blending
- Verify no over-brightness

**Test 5: Camera Distance**
- Move camera through fog
- Verify camera fade works
- Check no harsh transitions
- Verify depth fade smooth

### Performance Tests

**Test 1: Particle Count**
- Monitor with `stat particles`
- Verify max 200 particles
- Check spawn rate consistency
- Verify cleanup works

**Test 2: GPU Cost**
- Use `profilegpu` command
- Target: <0.8ms per emitter
- Check translucency cost
- Verify scalability works

**Test 3: Memory Usage**
- Check with `stat memory`
- Target: <4MB per emitter
- Verify no memory leaks
- Check texture memory

**Test 4: Overdraw**
- Use `viewmode shadercomplexity`
- Check for excessive overdraw
- Verify particle size appropriate
- Check sorting efficiency

### Integration Tests

**Test 1: Multiple Fog Volumes**
- Place 3+ emitters in level
- Verify blending looks natural
- Check performance impact
- Verify no z-fighting

**Test 2: Dynamic Parameters**
- Change density at runtime
- Adjust color dynamically
- Modify turbulence
- Verify smooth transitions

**Test 3: Level Streaming**
- Test with level streaming
- Verify fog loads correctly
- Check no orphaned particles
- Verify cleanup on unload

## Troubleshooting

### Common Issues

**Issue: Fog too transparent**
- Increase FogThickness parameter
- Raise base opacity in material
- Increase spawn rate
- Check alpha curve

**Issue: Fog too opaque**
- Decrease FogThickness parameter
- Lower base opacity
- Reduce spawn rate
- Check particle overlap

**Issue: Fog looks flat**
- Increase noise octaves
- Adjust turbulence strength
- Vary particle sizes more
- Check depth sorting enabled

**Issue: Performance problems**
- Reduce spawn rate
- Lower max particles
- Decrease noise octaves
- Reduce texture resolution

**Issue: Fog flickering**
- Enable depth sorting
- Increase sort precision
- Check z-fighting with geometry
- Verify camera fade working

**Issue: Fog disappears at distance**
- Increase max draw distance
- Adjust LOD distances
- Check culling settings
- Verify bounds correct

## Best Practices

1. **Placement**: Use multiple small volumes instead of one large
2. **Density**: Start subtle, increase gradually
3. **Color**: Match environment lighting
4. **Height**: Keep fog low for ground fog effect
5. **Performance**: Always use GPU simulation for fog
6. **Sorting**: Enable for proper depth perception
7. **Lighting**: Use translucent lighting mode
8. **Scalability**: Test on low-end hardware
9. **Bounds**: Keep tight to visible area
10. **Cleanup**: Ensure proper destruction

## Asset Requirements

### Textures

| Asset | Resolution | Format | Size | Description |
|-------|------------|--------|------|-------------|
| T_Fog_Noise_D | 512x512 | BC1 | ~170KB | Fog noise pattern |
| T_Fog_Opacity | 256x256 | BC4 | ~65KB | Fog opacity mask |

### Materials

| Asset | Type | Complexity | Description |
|-------|------|------------|-------------|
| M_VolumetricFog | Unlit | Medium | Main fog material |
| MI_VolumetricFog_Inst | Instance | - | Runtime instance |

### Niagara Assets

| Asset | Type | Description |
|-------|------|-------------|
| NS_Fog_Volume | System | Main system |
| NE_FogEmitter | Emitter | Fog emitter |

## Parameter Presets

### Preset: Light Fog (Default)
- FogDensity: 1.0
- FogThickness: 1.0
- FogHeight: 200
- DriftSpeed: 1.0
- TurbulenceStrength: 30

### Preset: Heavy Fog
- FogDensity: 2.0
- FogThickness: 1.8
- FogHeight: 300
- DriftSpeed: 0.6
- TurbulenceStrength: 50

### Preset: Ground Fog
- FogDensity: 1.5
- FogThickness: 1.2
- FogHeight: 100
- DriftSpeed: 0.8
- TurbulenceStrength: 20

### Preset: Toxic Fog
- FogDensity: 1.8
- FogThickness: 2.0
- FogHeight: 250
- DriftSpeed: 1.2
- TurbulenceStrength: 70
- FogColor: (0.4, 0.6, 0.3)
