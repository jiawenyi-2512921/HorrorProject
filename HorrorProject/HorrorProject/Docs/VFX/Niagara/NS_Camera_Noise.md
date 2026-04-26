# NS_Camera_Noise - Camera Film Grain/Noise Implementation Guide

## System Overview

**Purpose**: Screen-space film grain and noise particles for horror atmosphere and visual degradation effects.

**Use Cases**:
- VHS tape effect enhancement
- Low-light camera noise
- Fear/stress visual feedback
- Found footage aesthetic

**Performance Budget**:
- Max Particles: 300 concurrent
- Draw Calls: 1
- GPU Cost: ~0.2ms @ 1080p
- Memory: ~1MB

## Emitter Configuration

### Emitter Properties

**Emitter State**:
- Execution State: Active
- Execution State Source: Scalability
- Simulation Target: GPUSim
- Requires Persistent IDs: False
- Determinism: False

**Spawn Settings**:
- Spawn Rate: 100-200 particles/second
- Spawn Burst: 50 particles on activation
- Spawn Probability: 1.0

### Emitter Modules

#### 1. Initialize Particle

**Lifetime**:
- Mode: Random Range
- Minimum: 0.1 seconds
- Maximum: 0.3 seconds

**Color**:
- Mode: Random Range
- Color Min: (R: 0.0, G: 0.0, B: 0.0, A: 0.05)
- Color Max: (R: 1.0, G: 1.0, B: 1.0, A: 0.15)
- Color Mode: Linear Color

**Sprite Size**:
- Mode: Random Uniform
- Minimum: (X: 1.0, Y: 1.0)
- Maximum: (X: 3.0, Y: 3.0)
- Scale Mode: Uniform

**Sprite Rotation**:
- Mode: Random Range
- Minimum: 0.0 degrees
- Maximum: 360.0 degrees

**Mass**:
- Mass: 0.0 kg (no physics)

#### 2. Spawn Location

**Shape**: Screen Space Rectangle
- Screen Coverage: Full screen
- Distribution Mode: Random
- Surface Only: True
- Camera Relative: True

**Velocity**:
- Mode: None
- Velocity: (0, 0, 0)

#### 3. Update Modules

**Screen Space Position**:
- Position Mode: Screen Space
- Depth: 0.99 (near camera)
- Follow Camera: True

**Scale Color**:
- Alpha Curve:
  - 0.0s: 0.0
  - 0.05s: 1.0
  - Lifetime-0.05s: 1.0
  - Lifetime: 0.0

**Flicker**:
- Flicker Rate: 30 Hz
- Flicker Amount: 0.5
- Random Phase: True

**Color Variation**:
- Hue Shift: Random Range (-0.1, 0.1)
- Saturation: Random Range (0.0, 0.3)
- Value: Random Range (0.5, 1.0)

#### 4. Collision

**Collision Module**: Disabled (screen space)

#### 5. Kill Particles

**Lifetime Kill**:
- Enabled: True

**Screen Bounds Kill**:
- Kill if outside screen: True

## Particle Attributes

### Required Attributes

| Attribute | Type | Default | Description |
|-----------|------|---------|-------------|
| ScreenPosition | Vector2 | (0.5,0.5) | Screen UV position |
| Color | Linear Color | (0.5,0.5,0.5,0.1) | Particle color |
| SpriteSize | Vector2 | (2,2) | Sprite dimensions |
| SpriteRotation | Float | 0.0 | Rotation angle |
| Age | Float | 0.0 | Current age |
| Lifetime | Float | 0.2 | Max lifetime |
| FlickerPhase | Float | 0.0 | Flicker offset |

### User-Exposed Parameters

| Parameter | Type | Default | Range | Description |
|-----------|------|---------|-------|-------------|
| NoiseIntensity | Float | 1.0 | 0.0-3.0 | Overall noise strength |
| NoiseScale | Float | 1.0 | 0.5-2.0 | Particle size multiplier |
| NoiseDensity | Float | 1.0 | 0.3-3.0 | Spawn rate multiplier |
| ColoredNoise | Float | 0.0 | 0.0-1.0 | Color vs grayscale mix |
| FlickerSpeed | Float | 1.0 | 0.5-3.0 | Flicker rate multiplier |
| NoisePattern | Int | 0 | 0-2 | Pattern type (0=random, 1=scanline, 2=block) |

## Material Configuration

### Material: M_CameraNoise

**Material Domain**: Surface
**Blend Mode**: Translucent
**Shading Model**: Unlit

**Material Properties**:
- Two Sided: False
- Disable Depth Test: True
- Responsive AA: False
- Translucency Lighting Mode: Surface TranslucencyVolume
- Translucent Sort Priority: 100 (render last)

**Texture Inputs**:

1. **Noise Texture**: T_FilmGrain
   - Resolution: 256x256
   - Format: BC1
   - Compression: Default
   - Content: Film grain pattern

**Material Parameters**:

| Parameter | Type | Default | Description |
|-----------|------|---------|-------------|
| NoiseIntensity | Scalar | 1.0 | Brightness multiplier |
| ColoredNoise | Scalar | 0.0 | Color amount |
| BlendMode | Scalar | 0.5 | Additive vs overlay |

**Material Graph**:
```
Texture Sample (T_FilmGrain)
  → Multiply (Particle Color)
  → Multiply (NoiseIntensity)
  → Lerp (Grayscale, Color, ColoredNoise)
  → Base Color

Particle Alpha
  → Multiply (NoiseIntensity)
  → Opacity

SceneColor
  → Lerp (Add, Overlay, BlendMode)
  → Final Color
```

**Blend Mode Setup**:
```
// Additive
SceneColor + (NoiseColor * Opacity)

// Overlay
if (SceneColor < 0.5)
  SceneColor * (2 * NoiseColor)
else
  1 - (2 * (1 - SceneColor) * (1 - NoiseColor))
```

## Performance Optimization

### LOD Configuration

**LOD 0** (Always):
- Spawn Rate: 150/second
- Max Particles: 300
- Sprite Size: 100%
- Update Rate: 60 FPS

**LOD 1** (Medium):
- Spawn Rate: 100/second
- Max Particles: 200
- Sprite Size: 100%
- Update Rate: 60 FPS

**LOD 2** (Low):
- Spawn Rate: 50/second
- Max Particles: 100
- Sprite Size: 100%
- Update Rate: 30 FPS

### Scalability Settings

**Epic/High**:
- Full spawn rate
- Colored noise enabled
- Flicker enabled

**Medium**:
- 70% spawn rate
- Grayscale only
- Flicker enabled

**Low**:
- 50% spawn rate
- Grayscale only
- No flicker

**Cinematic**:
- 150% spawn rate
- Full colored noise
- Enhanced flicker

### Culling Configuration

**Distance Culling**: Disabled (screen space)

**Visibility Culling**:
- Always render when active
- No culling delay

**Bounds**:
- Screen space bounds
- No fixed bounds needed

## Collision and Physics

### Collision Settings

**Collision Type**: None (screen space effect)

### Physics Simulation

**Forces**: None (static screen space)

**Integration**: Not applicable

## Events and Lifecycle

### Spawn Events

**On Particle Spawn**:
- Randomize screen position
- Set random color
- Set random lifetime
- Initialize flicker phase

### Update Events

**On Particle Update** (every frame):
- Update flicker
- Scale alpha by age
- Update color variation

### Death Events

**On Particle Death**:
- No special handling
- Automatic cleanup

**Death Conditions**:
1. Lifetime expired
2. Outside screen bounds

## Testing and Validation

### Visual Tests

**Test 1: Noise Coverage**
- Verify full screen coverage
- Check no gaps or patterns
- Verify random distribution
- Check density appropriate

**Test 2: Noise Intensity**
- Test at various intensities
- Verify not too distracting
- Check visibility in dark/light
- Verify smooth scaling

**Test 3: Color Variation**
- Test colored vs grayscale
- Verify color blending
- Check no oversaturation
- Verify subtle effect

**Test 4: Flicker**
- Observe flicker pattern
- Verify not too fast/slow
- Check random phase works
- Verify no strobing effect

**Test 5: Performance**
- Check frame rate impact
- Verify GPU cost low
- Test with other effects
- Check no stuttering

### Performance Tests

**Test 1: Particle Count**
- Monitor with `stat particles`
- Verify max 300 particles
- Check spawn rate consistency
- Verify cleanup works

**Test 2: GPU Cost**
- Use `profilegpu` command
- Target: <0.2ms
- Check translucency cost
- Verify scalability works

**Test 3: Memory Usage**
- Check with `stat memory`
- Target: <1MB
- Verify no memory leaks
- Check texture memory

**Test 4: Frame Rate**
- Test at 60 FPS
- Verify no drops
- Check with other effects
- Verify consistent performance

### Integration Tests

**Test 1: VFX Stacking**
- Combine with post-process
- Test with other particles
- Verify proper layering
- Check no conflicts

**Test 2: Dynamic Intensity**
- Change intensity at runtime
- Adjust density dynamically
- Modify color blend
- Verify smooth transitions

**Test 3: On/Off Toggle**
- Toggle effect on/off
- Verify no crashes
- Check cleanup works
- Verify no artifacts

## Troubleshooting

### Common Issues

**Issue: Noise too visible**
- Reduce NoiseIntensity
- Lower NoiseDensity
- Decrease particle alpha
- Check blend mode

**Issue: Noise not visible**
- Increase NoiseIntensity
- Raise NoiseDensity
- Check material assigned
- Verify emitter active

**Issue: Noise looks patterned**
- Increase spawn randomness
- Check texture tiling
- Verify random distribution
- Adjust flicker phase

**Issue: Performance problems**
- Reduce spawn rate
- Lower max particles
- Disable flicker
- Use grayscale only

**Issue: Noise flickers too much**
- Reduce FlickerSpeed
- Lower flicker amount
- Increase particle lifetime
- Check spawn rate

**Issue: Color noise looks wrong**
- Adjust ColoredNoise parameter
- Check color ranges
- Verify saturation limits
- Check hue shift range

## Best Practices

1. **Intensity**: Keep subtle (0.3-0.8 for normal use)
2. **Density**: Less is more for realism
3. **Color**: Use sparingly, grayscale usually better
4. **Flicker**: Match camera frame rate feel
5. **Performance**: Very cheap, safe to use always
6. **Layering**: Render last (high sort priority)
7. **Scalability**: Reduce density on low settings
8. **Context**: Increase in dark/stressful situations
9. **Cleanup**: No special cleanup needed
10. **Testing**: Test with various backgrounds

## Asset Requirements

### Textures

| Asset | Resolution | Format | Size | Description |
|-------|------------|--------|------|-------------|
| T_FilmGrain | 256x256 | BC1 | ~65KB | Film grain pattern |

### Materials

| Asset | Type | Complexity | Description |
|-------|------|------------|-------------|
| M_CameraNoise | Unlit | Very Low | Main noise material |
| MI_CameraNoise_Inst | Instance | - | Runtime instance |

### Niagara Assets

| Asset | Type | Description |
|-------|------|-------------|
| NS_Camera_Noise | System | Main system |
| NE_NoiseEmitter | Emitter | Noise emitter |

## Parameter Presets

### Preset: Subtle Grain (Default)
- NoiseIntensity: 0.5
- NoiseScale: 1.0
- NoiseDensity: 1.0
- ColoredNoise: 0.0
- FlickerSpeed: 1.0
- NoisePattern: 0

### Preset: VHS Tape
- NoiseIntensity: 1.2
- NoiseScale: 1.5
- NoiseDensity: 1.5
- ColoredNoise: 0.3
- FlickerSpeed: 2.0
- NoisePattern: 1

### Preset: Low Light Camera
- NoiseIntensity: 1.8
- NoiseScale: 0.8
- NoiseDensity: 2.0
- ColoredNoise: 0.5
- FlickerSpeed: 1.5
- NoisePattern: 2

### Preset: Fear Effect
- NoiseIntensity: 2.5
- NoiseScale: 1.2
- NoiseDensity: 2.5
- ColoredNoise: 0.2
- FlickerSpeed: 3.0
- NoisePattern: 0

### Preset: Clean (Minimal)
- NoiseIntensity: 0.2
- NoiseScale: 1.0
- NoiseDensity: 0.5
- ColoredNoise: 0.0
- FlickerSpeed: 0.5
- NoisePattern: 0

## Integration with Post-Process

### Recommended Combinations

**With VHS Effect**:
- NoiseIntensity: 1.0-1.5
- ColoredNoise: 0.3
- NoisePattern: 1 (scanline)

**With Fear Effect**:
- NoiseIntensity: 1.5-2.5
- ColoredNoise: 0.0-0.2
- FlickerSpeed: 2.0-3.0

**With Death Effect**:
- NoiseIntensity: 2.0-3.0
- ColoredNoise: 0.0
- NoiseDensity: 2.0-3.0

**With Underwater Effect**:
- NoiseIntensity: 0.8-1.2
- ColoredNoise: 0.5
- NoisePattern: 2 (block)

## Advanced Techniques

### Dynamic Intensity Based on Light Level

```cpp
// Blueprint example
float LightLevel = GetAverageLuminance();
float NoiseAmount = FMath::Lerp(2.0f, 0.3f, LightLevel);
NiagaraComponent->SetFloatParameter("NoiseIntensity", NoiseAmount);
```

### Stress-Based Noise Increase

```cpp
// Blueprint example
float StressLevel = PlayerState->GetStressLevel(); // 0-1
float NoiseAmount = 0.5f + (StressLevel * 2.0f);
NiagaraComponent->SetFloatParameter("NoiseIntensity", NoiseAmount);
```

### Scanline Pattern

```cpp
// Material function
float Scanline = frac(ScreenUV.y * 480.0 + Time * 10.0);
Scanline = step(0.5, Scanline);
NoiseColor *= Scanline;
```
