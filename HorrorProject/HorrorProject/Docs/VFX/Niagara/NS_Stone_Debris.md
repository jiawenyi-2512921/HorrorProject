# NS_Stone_Debris - Stone/Concrete Debris Implementation Guide

## System Overview

**Purpose**: Stone and concrete debris particles for destruction, impact, and environmental damage effects.

**Use Cases**:
- Wall/floor impact debris
- Structural damage effects
- Explosion debris
- Environmental destruction

**Performance Budget**:
- Max Particles: 120 per burst
- Draw Calls: 2 (particles + mesh)
- GPU Cost: ~0.6ms @ 1080p
- Memory: ~4MB

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
- Spawn Burst: 60-120 particles
- Spawn Probability: 1.0
- One Shot: True

### Emitter Modules

#### 1. Initialize Particle

**Lifetime**:
- Mode: Random Range
- Minimum: 1.0 seconds
- Maximum: 3.0 seconds

**Color**:
- Mode: Random Range
- Color Min: (R: 0.3, G: 0.3, B: 0.3, A: 1.0)
- Color Max: (R: 0.6, G: 0.6, B: 0.6, A: 1.0)
- Color Mode: Linear Color

**Mesh Scale**:
- Mode: Random Uniform
- Minimum: (X: 0.5, Y: 0.5, Z: 0.5)
- Maximum: (X: 2.0, Y: 2.0, Z: 2.0)
- Scale Mode: Uniform

**Mesh Rotation**:
- Mode: Random Range
- Minimum: (0, 0, 0) degrees
- Maximum: (360, 360, 360) degrees

**Mass**:
- Mass: Random Range (0.1, 0.5) kg

**Mesh Index**:
- Random mesh from array (0-7)

#### 2. Spawn Location

**Shape**: Sphere
- Sphere Radius: 20 units
- Distribution Mode: Volume
- Surface Only: False

**Velocity**:
- Mode: Random Range in Cone
- Cone Axis: Impact Normal
- Cone Angle: 90 degrees
- Velocity Magnitude: 300-800 units/second

#### 3. Update Modules

**Gravity**:
- Gravity Scale: 1.0 (full gravity)
- Gravity Direction: (0, 0, -980)

**Drag**:
- Drag Coefficient: 0.5 (moderate air resistance)
- Velocity Scale: 1.0

**Angular Velocity**:
- Initial Angular Velocity: Random Range
  - Pitch: (-360, 360) deg/s
  - Yaw: (-360, 360) deg/s
  - Roll: (-360, 360) deg/s

**Angular Drag**:
- Angular Drag Coefficient: 0.3

**Scale Mesh Size**:
- Scale Factor Curve:
  - 0.0s: 0.3 (small start)
  - 0.1s: 1.0 (pop in)
  - Lifetime: 1.0 (constant)

**Scale Color**:
- Alpha Curve:
  - 0.0s: 1.0
  - Lifetime-0.5s: 1.0
  - Lifetime: 0.0 (fade out)

**Color Over Life**:
- Dust color overlay:
  - 0.0s: Clean stone
  - Lifetime: Dusty/dirty

#### 4. Collision

**Collision Module**:
- Collision Mode: Scene Depth + Trace
- Radius: 3.0
- Resilience: 0.4 (moderate bounce)
- Friction: 0.7 (moderate friction)
- Kill Particles on Collision: False

**Collision Event**:
- On Collision: Play Sound
- Sound: Impact sound (volume based on velocity)
- Spawn Dust Puff: Small dust particle burst

**Collision Response**:
- First Bounce: 40% velocity retention
- Subsequent Bounces: 20% velocity retention
- Max Bounces: 3

#### 5. Kill Particles

**Lifetime Kill**:
- Enabled: True

**Velocity Kill**:
- Kill when velocity < 20 units/s
- Delay: 1.0 seconds after low velocity

**Ground Settle**:
- Kill when on ground and stationary
- Delay: 2.0 seconds

## Particle Attributes

### Required Attributes

| Attribute | Type | Default | Description |
|-----------|------|---------|-------------|
| Position | Vector3 | (0,0,0) | World position |
| Velocity | Vector3 | (0,0,0) | Movement velocity |
| AngularVelocity | Vector3 | (0,0,0) | Rotation velocity |
| Color | Linear Color | (0.45,0.45,0.45,1) | Debris color |
| MeshScale | Vector3 | (1,1,1) | Mesh scale |
| MeshRotation | Rotator | (0,0,0) | Mesh rotation |
| Age | Float | 0.0 | Current age |
| Lifetime | Float | 2.0 | Max lifetime |
| Mass | Float | 0.3 | Particle mass |
| MeshIndex | Int | 0 | Mesh variation index |
| BounceCount | Int | 0 | Number of bounces |

### User-Exposed Parameters

| Parameter | Type | Default | Range | Description |
|-----------|------|---------|-------|-------------|
| BurstCount | Int | 80 | 30-150 | Particles per burst |
| ExplosionForce | Float | 1.0 | 0.5-3.0 | Initial velocity multiplier |
| DebrisColor | Linear Color | (0.45,0.45,0.45,1) | - | Debris tint color |
| SprayAngle | Float | 90.0 | 45-180 | Cone spread angle |
| DebrisSize | Float | 1.0 | 0.5-2.0 | Mesh size multiplier |
| ImpactSound | Bool | True | - | Play impact sounds |

## Material Configuration

### Material: M_StoneDebris

**Material Domain**: Surface
**Blend Mode**: Opaque
**Shading Model**: Default Lit

**Material Properties**:
- Two Sided: False
- Disable Depth Test: False
- Responsive AA: True

**Texture Inputs**:

1. **Base Color**: T_Concrete_D
   - Resolution: 512x512
   - Format: BC1
   - Compression: Default
   - Content: Concrete/stone texture

2. **Normal Map**: T_Concrete_N
   - Resolution: 512x512
   - Format: BC5
   - Compression: Normal Map
   - Content: Surface detail normals

3. **ORM Map**: T_Concrete_ORM
   - Resolution: 512x512
   - Format: BC1
   - Compression: Masks
   - Content: Occlusion, Roughness, Metallic

**Material Parameters**:

| Parameter | Type | Default | Description |
|-----------|------|---------|-------------|
| BaseColor | Vector3 | (0.45,0.45,0.45) | Stone color |
| Roughness | Scalar | 0.8 | Surface roughness |
| Metallic | Scalar | 0.0 | Non-metallic |
| NormalStrength | Scalar | 1.0 | Normal intensity |

**Material Graph**:
```
Texture Sample (T_Concrete_D)
  → Multiply (Particle Color)
  → Multiply (BaseColor)
  → Base Color

Texture Sample (T_Concrete_N)
  → Multiply (NormalStrength)
  → Normal

Texture Sample (T_Concrete_ORM)
  → Split Channels
    → R: Ambient Occlusion
    → G: Roughness
    → B: Metallic
```

## Mesh Assets

### Debris Mesh Array

**Required Meshes** (8 variations):

1. **SM_Debris_Chunk01**
   - Triangles: 50-100
   - Size: 10-20 units
   - Shape: Irregular chunk

2. **SM_Debris_Chunk02**
   - Triangles: 50-100
   - Size: 10-20 units
   - Shape: Angular piece

3. **SM_Debris_Chunk03**
   - Triangles: 50-100
   - Size: 10-20 units
   - Shape: Flat shard

4. **SM_Debris_Chunk04**
   - Triangles: 50-100
   - Size: 10-20 units
   - Shape: Small rock

5. **SM_Debris_Chunk05**
   - Triangles: 50-100
   - Size: 10-20 units
   - Shape: Elongated piece

6. **SM_Debris_Chunk06**
   - Triangles: 50-100
   - Size: 10-20 units
   - Shape: Cubic chunk

7. **SM_Debris_Chunk07**
   - Triangles: 50-100
   - Size: 10-20 units
   - Shape: Thin shard

8. **SM_Debris_Chunk08**
   - Triangles: 50-100
   - Size: 10-20 units
   - Shape: Irregular fragment

**Mesh Requirements**:
- LOD0: Full detail
- LOD1: 50% triangles
- Collision: Simple box/sphere
- Pivot: Center of mass
- UV: Single channel, tiling

## Performance Optimization

### LOD Configuration

**LOD 0** (0-1000 units):
- Burst Count: 80
- Max Particles: 120
- Mesh LOD: 0
- Collision: Full trace
- Sound: Enabled

**LOD 1** (1000-2000 units):
- Burst Count: 50
- Max Particles: 70
- Mesh LOD: 1
- Collision: Depth only
- Sound: Enabled

**LOD 2** (2000+ units):
- Burst Count: 25
- Max Particles: 35
- Mesh LOD: 1
- Collision: Disabled
- Sound: Disabled

### Scalability Settings

**Epic/High**:
- Full burst count
- Collision + sound enabled
- High quality meshes
- Dust puffs on impact

**Medium**:
- 70% burst count
- Collision enabled, no dust
- Medium quality meshes
- Sound enabled

**Low**:
- 50% burst count
- No collision, no sound
- Low quality meshes
- No dust puffs

**Cinematic**:
- 150% burst count
- Full collision + enhanced sound
- Highest quality meshes
- Enhanced dust effects

### Culling Configuration

**Distance Culling**:
- Max Draw Distance: 2500 units
- Cull Proxy Mode: None

**Visibility Culling**:
- Visibility Tag: "DebrisEffects"
- Cull When Not Rendered: True
- Cull Delay: 1.0 seconds

**Bounds**:
- Fixed Bounds: (X: 300, Y: 300, Z: 300)
- Use Fixed Bounds: True

## Collision and Physics

### Collision Settings

**Collision Type**: Scene Depth + Line Trace
- Query Type: Both
- Collision Group: PhysicsBody
- Max Collision Iterations: 3

**Collision Response**:
- Resilience: 0.4 (moderate bounce)
- Friction: 0.7 (moderate friction)
- Velocity Retention: 40% first bounce, 20% subsequent

**Collision Events**:
- On Collision: Play Impact Sound
- Spawn Dust Puff: Small particle burst
- Generate Collision Events: True

### Physics Simulation

**Forces**:
1. Gravity: -980 units/s² (full gravity)
2. Drag: 0.5 coefficient (moderate)
3. Angular Drag: 0.3 coefficient
4. Initial Explosion: 300-800 units/s

**Integration**:
- Integration Method: Verlet
- Sub-Steps: 2
- Max Delta Time: 0.05 seconds

**Rotation**:
- Angular Velocity: Random tumbling
- Angular Drag: Gradual slowdown
- Rotation Mode: Full 3D

## Events and Lifecycle

### Spawn Events

**On Particle Spawn**:
- Initialize random rotation
- Set random lifetime
- Apply explosion velocity
- Set random angular velocity
- Select random mesh
- Set spawn color

### Update Events

**On Particle Update** (every frame):
- Apply gravity
- Apply drag (linear and angular)
- Update rotation
- Scale alpha by age
- Check collision
- Update color (dust accumulation)

### Collision Events

**On Particle Collision**:
- Play impact sound (volume based on velocity)
- Spawn small dust puff
- Reduce velocity by resilience
- Increment bounce count
- Apply friction to angular velocity

### Death Events

**On Particle Death**:
- Optional: Leave static mesh decal
- Automatic cleanup

**Death Conditions**:
1. Lifetime expired
2. Velocity near zero after settling
3. Max bounces exceeded
4. Alpha < 0.01

## Testing and Validation

### Visual Tests

**Test 1: Debris Pattern**
- Spawn at various angles
- Verify realistic spread
- Check cone angle correct
- Verify random distribution

**Test 2: Physics Behavior**
- Verify gravity works correctly
- Check bouncing realistic
- Verify rotation natural
- Check settling behavior

**Test 3: Mesh Variation**
- Verify all 8 meshes used
- Check random selection
- Verify size variation
- Check no duplicate patterns

**Test 4: Collision**
- Test on various surfaces
- Verify bounce appropriate
- Check sound triggers
- Verify dust spawns

**Test 5: Performance**
- Test multiple simultaneous bursts
- Check particle count limits
- Verify cleanup works
- Check no memory leaks

### Performance Tests

**Test 1: Particle Count**
- Monitor with `stat particles`
- Verify max 120 per burst
- Check cleanup after lifetime
- Verify no accumulation

**Test 2: GPU Cost**
- Use `profilegpu` command
- Target: <0.6ms per burst
- Check mesh rendering cost
- Verify scalability works

**Test 3: Memory Usage**
- Check with `stat memory`
- Target: <4MB per effect
- Verify mesh cleanup
- Check texture memory

**Test 4: Physics Cost**
- Monitor with `stat physics`
- Check collision cost
- Verify acceptable overhead
- Check no physics spikes

### Integration Tests

**Test 1: Impact Integration**
- Trigger on impact events
- Verify spawn location correct
- Check impact normal used
- Verify consistent behavior

**Test 2: Surface Types**
- Test on various surfaces
- Verify collision works on all
- Check bounce appropriate
- Verify sound correct

**Test 3: Dynamic Spawning**
- Spawn/destroy rapidly
- Verify no crashes
- Check memory cleanup
- Verify parameter changes work

## Troubleshooting

### Common Issues

**Issue: Debris not visible**
- Check meshes assigned
- Verify burst count > 0
- Check spawn location valid
- Verify material assigned

**Issue: Debris doesn't fall**
- Enable gravity module
- Check gravity scale
- Verify mass > 0
- Check drag not too high

**Issue: Debris doesn't bounce**
- Enable collision module
- Check resilience > 0
- Verify collision channel
- Check scene depth available

**Issue: No rotation**
- Enable angular velocity
- Check angular velocity range
- Verify angular drag not too high
- Check rotation mode

**Issue: Performance problems**
- Reduce burst count
- Lower max particles
- Disable collision
- Use simpler meshes

**Issue: Debris penetrates walls**
- Enable collision trace
- Increase collision radius
- Check collision channel
- Verify mesh collision

## Best Practices

1. **Burst Size**: Keep under 120 particles per burst
2. **Direction**: Always use impact normal for cone axis
3. **Meshes**: Use 8+ variations for variety
4. **Collision**: Always enable for realism
5. **Sound**: Scale volume by impact velocity
6. **Performance**: Pool and reuse systems
7. **Cleanup**: Ensure particles cleanup after settling
8. **Scalability**: Reduce on low settings
9. **Rotation**: Natural tumbling is key
10. **Testing**: Test on various surface types

## Asset Requirements

### Textures

| Asset | Resolution | Format | Size | Description |
|-------|------------|--------|------|-------------|
| T_Concrete_D | 512x512 | BC1 | ~170KB | Concrete diffuse |
| T_Concrete_N | 512x512 | BC5 | ~340KB | Concrete normals |
| T_Concrete_ORM | 512x512 | BC1 | ~170KB | ORM packed |

### Materials

| Asset | Type | Complexity | Description |
|-------|------|------------|-------------|
| M_StoneDebris | Default Lit | Low | Debris material |
| MI_StoneDebris_Inst | Instance | - | Runtime instance |

### Meshes

| Asset | Triangles | LODs | Description |
|-------|-----------|------|-------------|
| SM_Debris_Chunk01-08 | 50-100 | 2 | Debris variations |

### Niagara Assets

| Asset | Type | Description |
|-------|------|-------------|
| NS_Stone_Debris | System | Main system |
| NE_DebrisEmitter | Emitter | Debris emitter |

### Audio Assets

| Asset | Type | Description |
|-------|------|-------------|
| SFX_StoneImpact_01-05 | Sound Cue | Impact sounds |

## Parameter Presets

### Preset: Light Impact (Default)
- BurstCount: 60
- ExplosionForce: 0.8
- SprayAngle: 75
- DebrisSize: 0.8
- ImpactSound: True

### Preset: Heavy Impact
- BurstCount: 120
- ExplosionForce: 1.8
- SprayAngle: 120
- DebrisSize: 1.5
- ImpactSound: True

### Preset: Explosion
- BurstCount: 150
- ExplosionForce: 3.0
- SprayAngle: 180
- DebrisSize: 1.2
- ImpactSound: True

### Preset: Minimal Debris
- BurstCount: 30
- ExplosionForce: 0.6
- SprayAngle: 60
- DebrisSize: 0.6
- ImpactSound: False

## Advanced Features

### Dust Puff on Impact

**Sub-Emitter**: NE_DustPuff
- Spawn on collision event
- Burst: 10-20 particles
- Lifetime: 0.5-1.0 seconds
- Size: 20-40 units
- Color: Dusty gray

### Impact Sound System

**Sound Selection**:
- Velocity < 200: Soft impact (SFX_01)
- Velocity 200-500: Medium impact (SFX_02-03)
- Velocity > 500: Hard impact (SFX_04-05)

**Volume Scaling**:
```cpp
float Volume = FMath::Clamp(Velocity / 1000.0f, 0.1f, 1.0f);
```

### Surface-Specific Materials

**Material Instances**:
- MI_ConcreteDebris (gray)
- MI_BrickDebris (red-brown)
- MI_StoneDebris (gray-tan)
- MI_PlasterDebris (white)
