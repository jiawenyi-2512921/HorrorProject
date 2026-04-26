# NS_Water_Drips - Water Dripping Effect Implementation Guide

## System Overview

**Purpose**: Realistic water dripping particles for leaky pipes, ceiling drips, and wet environments.

**Use Cases**:
- Leaking pipes and ceilings
- Cave water drips
- Wet environment atmosphere
- Post-rain dripping

**Performance Budget**:
- Max Particles: 50 concurrent
- Draw Calls: 1
- GPU Cost: ~0.2ms @ 1080p
- Memory: ~1MB

## Emitter Configuration

### Emitter Properties

**Emitter State**:
- Execution State: Active
- Execution State Source: Scalability
- Simulation Target: CPUSim
- Requires Persistent IDs: False
- Determinism: True (predictable dripping)

**Spawn Settings**:
- Spawn Rate: 1-5 particles/second
- Spawn Burst: None
- Spawn Probability: 1.0

### Emitter Modules

#### 1. Initialize Particle

**Lifetime**:
- Mode: Random Range
- Minimum: 1.0 seconds
- Maximum: 3.0 seconds

**Color**:
- Mode: Direct Set
- Color: (R: 0.8, G: 0.9, B: 1.0, A: 0.6)
- Color Mode: Linear Color

**Sprite Size**:
- Mode: Random Uniform
- Minimum: (X: 2.0, Y: 2.0)
- Maximum: (X: 5.0, Y: 5.0)
- Scale Mode: Uniform

**Sprite Rotation**:
- Mode: None
- Rotation: 0.0 degrees (always vertical)

**Mass**:
- Mass: 0.01 kg

#### 2. Spawn Location

**Shape**: Point
- Location: (0, 0, 0) - drip source
- Distribution Mode: Single Point
- Surface Only: False

**Velocity**:
- Mode: Direct Set
- Velocity: (0, 0, -50) - straight down
- Velocity Variation: ±5 units/s

#### 3. Update Modules

**Gravity**:
- Gravity Scale: 1.0 (full gravity)
- Gravity Direction: (0, 0, -980)

**Drag**:
- Drag Coefficient: 0.1 (minimal air resistance)
- Velocity Scale: 1.0

**Scale Sprite Size**:
- Scale Factor Curve:
  - 0.0s: 0.5 (small droplet)
  - 0.3s: 1.0 (full size)
  - Lifetime-0.2s: 1.0
  - Lifetime: 0.3 (splash shrink)

**Scale Color**:
- Alpha Curve:
  - 0.0s: 0.0
  - 0.1s: 1.0 (fade in)
  - Lifetime-0.1s: 1.0
  - Lifetime: 0.0 (fade out)

**Sprite Stretch**:
- Stretch Factor: Velocity-based
- Max Stretch: 3.0 (elongate when falling fast)
- Stretch Direction: Velocity direction

**Camera Offset**:
- Camera Offset: -10 units
- Apply in Camera Space: False

#### 4. Collision

**Collision Module**:
- Collision Mode: Scene Depth + Trace
- Radius: 1.0
- Resilience: 0.0 (no bounce)
- Friction: 1.0
- Kill Particles on Collision: True

**Collision Event**:
- On Collision: Spawn Splash
- Splash Effect: Small water splash particles
- Sound: Drip sound (quiet)

#### 5. Kill Particles

**Lifetime Kill**:
- Enabled: True

**Collision Kill**:
- Kill on collision: True

## Particle Attributes

### Required Attributes

| Attribute | Type | Default | Description |
|-----------|------|---------|-------------|
| Position | Vector3 | (0,0,0) | World position |
| Velocity | Vector3 | (0,0,-50) | Movement velocity |
| Color | Linear Color | (0.8,0.9,1.0,0.6) | Water color |
| SpriteSize | Vector2 | (3,3) | Sprite dimensions |
| Age | Float | 0.0 | Current age |
| Lifetime | Float | 2.0 | Max lifetime |
| Mass | Float | 0.01 | Particle mass |
| StretchFactor | Float | 1.0 | Velocity-based stretch |

### User-Exposed Parameters

| Parameter | Type | Default | Range | Description |
|-----------|------|---------|-------|-------------|
| DripRate | Float | 3.0 | 0.5-10.0 | Drips per second |
| DropSize | Float | 1.0 | 0.5-2.0 | Droplet size multiplier |
| WaterColor | Linear Color | (0.8,0.9,1.0,1) | - | Water tint color |
| SpawnHeight | Float | 0.0 | -500-500 | Height offset from emitter |
| EnableSplash | Bool | True | - | Spawn splash on impact |
| EnableSound | Bool | True | - | Play drip sound |

## Material Configuration

### Material: M_WaterDrop

**Material Domain**: Surface
**Blend Mode**: Translucent
**Shading Model**: Unlit

**Material Properties**:
- Two Sided: True
- Disable Depth Test: False
- Responsive AA: True
- Translucency Lighting Mode: Surface TranslucencyVolume

**Texture Inputs**:

1. **Drop Texture**: T_WaterDrop_D
   - Resolution: 128x128
   - Format: BC1
   - Compression: Default
   - Content: Water droplet shape

2. **Drop Normal**: T_WaterDrop_N
   - Resolution: 128x128
   - Format: BC5
   - Compression: Normal Map
   - Content: Droplet surface normals

**Material Parameters**:

| Parameter | Type | Default | Description |
|-----------|------|---------|-------------|
| WaterColor | Vector3 | (0.8,0.9,1.0) | Base water color |
| Refraction | Scalar | 0.3 | Refraction strength |
| Opacity | Scalar | 0.6 | Base opacity |
| DepthFade | Scalar | 10.0 | Soft particle distance |

**Material Graph**:
```
Texture Sample (T_WaterDrop_D)
  → Multiply (Particle Color)
  → Multiply (WaterColor)
  → Base Color

Texture Sample (T_WaterDrop_N)
  → Normal

Particle Alpha
  → Multiply (DepthFade)
  → Opacity

Refraction
  → Multiply (Refraction Strength)
  → Refraction
```

**Depth Fade Setup**:
```
PixelDepth
  → Subtract (SceneDepth)
  → Divide (DepthFade Distance)
  → Saturate
  → Multiply (Opacity)
```

## Splash Effect Configuration

### Sub-Emitter: NE_WaterSplash

**Spawn Trigger**: On Collision Event

**Splash Properties**:
- Burst Count: 5-10 particles
- Lifetime: 0.3-0.6 seconds
- Velocity: Radial outward (50-150 units/s)
- Size: 2-5 units
- Color: Same as parent droplet

**Splash Modules**:
1. Initialize with radial velocity
2. Apply gravity
3. Fade out quickly
4. No collision

## Performance Optimization

### LOD Configuration

**LOD 0** (0-1500 units):
- Drip Rate: Full
- Max Particles: 50
- Sprite Size: 100%
- Splash: Enabled
- Sound: Enabled

**LOD 1** (1500-3000 units):
- Drip Rate: 70%
- Max Particles: 30
- Sprite Size: 80%
- Splash: Simplified
- Sound: Enabled

**LOD 2** (3000+ units):
- Drip Rate: 50%
- Max Particles: 15
- Sprite Size: 60%
- Splash: Disabled
- Sound: Disabled

### Scalability Settings

**Epic/High**:
- Full drip rate
- Splash enabled
- Sound enabled
- Refraction enabled

**Medium**:
- 70% drip rate
- Splash enabled
- Sound enabled
- No refraction

**Low**:
- 50% drip rate
- No splash
- No sound
- No refraction

**Cinematic**:
- 150% drip rate
- Enhanced splash
- Full sound
- Enhanced refraction

### Culling Configuration

**Distance Culling**:
- Max Draw Distance: 3000 units
- Cull Proxy Mode: None

**Visibility Culling**:
- Visibility Tag: "WaterDrips"
- Cull When Not Rendered: True
- Cull Delay: 2.0 seconds

**Bounds**:
- Fixed Bounds: (X: 50, Y: 50, Z: 500)
- Use Fixed Bounds: True

## Collision and Physics

### Collision Settings

**Collision Type**: Scene Depth + Line Trace
- Query Type: Both
- Collision Group: WorldStatic
- Max Collision Iterations: 1

**Collision Response**:
- Kill on Collision: True
- Spawn Splash: True
- Play Sound: True

**Collision Events**:
- On Collision: Spawn Splash Effect
- Generate Collision Events: True

### Physics Simulation

**Forces**:
1. Gravity: -980 units/s² (full gravity)
2. Drag: 0.1 coefficient (minimal)
3. Initial Velocity: -50 units/s (downward)

**Integration**:
- Integration Method: Euler
- Sub-Steps: 1
- Max Delta Time: 0.1 seconds

## Events and Lifecycle

### Spawn Events

**On Particle Spawn**:
- Set spawn position
- Apply initial downward velocity
- Set random lifetime
- Set spawn color

### Update Events

**On Particle Update** (every frame):
- Apply gravity
- Apply minimal drag
- Update stretch based on velocity
- Scale alpha by age
- Check collision

### Collision Events

**On Particle Collision**:
- Spawn splash effect (5-10 particles)
- Play drip sound (volume based on velocity)
- Kill particle

### Death Events

**On Particle Death**:
- No special handling
- Automatic cleanup

**Death Conditions**:
1. Collision with surface
2. Lifetime expired
3. Alpha < 0.01

## Testing and Validation

### Visual Tests

**Test 1: Drip Pattern**
- Verify consistent drip rate
- Check droplets fall straight
- Verify no erratic movement
- Check predictable timing

**Test 2: Droplet Appearance**
- Verify water color correct
- Check size appropriate
- Verify stretch when falling
- Check transparency correct

**Test 3: Splash Effect**
- Verify splash spawns on impact
- Check splash direction correct
- Verify splash size appropriate
- Check splash fades quickly

**Test 4: Physics Behavior**
- Verify gravity acceleration
- Check terminal velocity
- Verify straight fall path
- Check collision detection

**Test 5: Sound**
- Verify drip sound plays
- Check volume appropriate
- Verify no sound spam
- Check sound timing correct

### Performance Tests

**Test 1: Particle Count**
- Monitor with `stat particles`
- Verify max 50 particles
- Check drip rate consistency
- Verify cleanup works

**Test 2: GPU Cost**
- Use `profilegpu` command
- Target: <0.2ms per emitter
- Check translucency cost
- Verify scalability works

**Test 3: Memory Usage**
- Check with `stat memory`
- Target: <1MB per emitter
- Verify no memory leaks
- Check texture memory

**Test 4: Multiple Drips**
- Place 10+ drip emitters
- Check total particle budget
- Verify performance acceptable
- Check no interference

### Integration Tests

**Test 1: Environment Integration**
- Place in various locations
- Verify drips look natural
- Check lighting interaction
- Verify proper placement

**Test 2: Dynamic Parameters**
- Change drip rate at runtime
- Adjust drop size
- Modify color
- Verify smooth transitions

**Test 3: On/Off Toggle**
- Toggle drips on/off
- Verify no crashes
- Check cleanup works
- Verify no orphaned particles

## Troubleshooting

### Common Issues

**Issue: Drips not visible**
- Check material assigned
- Verify drip rate > 0
- Check spawn location valid
- Verify emitter active

**Issue: Drips too fast/slow**
- Adjust DripRate parameter
- Check gravity scale
- Verify initial velocity
- Check drag coefficient

**Issue: Drips don't fall straight**
- Check initial velocity direction
- Verify no external forces
- Check spawn location stable
- Verify no wind forces

**Issue: No splash on impact**
- Enable splash module
- Check collision enabled
- Verify collision channel
- Check splash emitter assigned

**Issue: Performance problems**
- Reduce drip rate
- Lower max particles
- Disable splash
- Disable sound

**Issue: Drips penetrate floor**
- Enable collision module
- Check collision channel
- Verify scene depth available
- Increase collision radius

## Best Practices

1. **Placement**: Attach to ceiling/pipe locations
2. **Rate**: Keep slow (1-5 drips/second) for realism
3. **Sound**: Use quiet, subtle drip sounds
4. **Splash**: Keep splash small and brief
5. **Performance**: Very cheap, safe to use many
6. **Collision**: Always enable for realism
7. **Color**: Match environment water color
8. **Stretch**: Velocity-based stretch adds realism
9. **Cleanup**: Automatic, no special handling needed
10. **Testing**: Test collision on various surfaces

## Asset Requirements

### Textures

| Asset | Resolution | Format | Size | Description |
|-------|------------|--------|------|-------------|
| T_WaterDrop_D | 128x128 | BC1 | ~10KB | Water droplet texture |
| T_WaterDrop_N | 128x128 | BC5 | ~20KB | Droplet normals |

### Materials

| Asset | Type | Complexity | Description |
|-------|------|------------|-------------|
| M_WaterDrop | Unlit | Low | Droplet material |
| MI_WaterDrop_Inst | Instance | - | Runtime instance |

### Niagara Assets

| Asset | Type | Description |
|-------|------|-------------|
| NS_Water_Drips | System | Main system |
| NE_DripEmitter | Emitter | Drip emitter |
| NE_SplashEmitter | Emitter | Splash sub-emitter |

### Audio Assets

| Asset | Type | Description |
|-------|------|-------------|
| SFX_WaterDrip_01-03 | Sound Cue | Drip sounds |

## Parameter Presets

### Preset: Slow Drip (Default)
- DripRate: 2.0
- DropSize: 1.0
- SpawnHeight: 0.0
- EnableSplash: True
- EnableSound: True

### Preset: Fast Drip
- DripRate: 6.0
- DropSize: 0.8
- SpawnHeight: 0.0
- EnableSplash: True
- EnableSound: True

### Preset: Heavy Leak
- DripRate: 10.0
- DropSize: 1.5
- SpawnHeight: 0.0
- EnableSplash: True
- EnableSound: True

### Preset: Minimal Drip
- DripRate: 0.5
- DropSize: 0.7
- SpawnHeight: 0.0
- EnableSplash: False
- EnableSound: False

## Advanced Features

### Randomized Drip Timing

**Implementation**:
```cpp
// Add slight randomness to drip timing
float RandomDelay = FMath::RandRange(0.0f, 0.3f);
SpawnRate = BaseSpawnRate + RandomDelay;
```

### Surface-Specific Splash

**Splash Variations**:
- Hard Surface: Small, tight splash
- Water Surface: Ripple effect
- Soft Surface: Absorbed, no splash

### Drip Sound Variation

**Sound Selection**:
- Random selection from 3 drip sounds
- Slight pitch variation (0.9-1.1)
- Volume based on drop size

### Puddle Formation

**Optional Feature**:
- Track drip impacts over time
- Spawn puddle decal after N drips
- Grow puddle size gradually
- Fade puddle after drips stop
