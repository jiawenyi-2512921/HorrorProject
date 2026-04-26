# VFX System Blueprint Implementation Guide

## Overview

The VFX system provides three main C++ components for visual effects:
- **UPostProcessController** - 9 post-process effects
- **UParticleSpawner** - 8 Niagara particle systems
- **UScreenEffectManager** - Camera shakes and screen feedback

All components are `BlueprintSpawnableComponent` and can be added to any Actor.

## C++ Foundation

### PostProcessController Component

**Purpose**: Manages post-process effects like VHS, underwater, fear, death effects.

**Key Features**:
- 9 effect types with dynamic intensity control
- Auto-fade support with configurable duration
- Material parameter automation
- Performance-optimized effect stacking

**Effect Types**:
```cpp
enum class EPostProcessEffectType : uint8
{
    VHS,                    // VHS tape distortion
    Underwater,             // Underwater color grading
    PressureVignette,       // Depth pressure vignette
    LensDroplets,           // Water droplets on lens
    ChromaticAberration,    // Color separation
    Scanlines,              // CRT scanlines
    NoiseOverlay,           // Film grain noise
    DeathEffect,            // Death screen effect
    FearEffect              // Fear/panic effect
};
```

### ParticleSpawner Component

**Purpose**: Spawns and manages Niagara particle effects.

**Key Features**:
- 8 particle effect types
- Automatic cleanup of finished effects
- LOD and culling support
- Particle budget management (max 1000 particles)

**Particle Types**:
```cpp
enum class EParticleEffectType : uint8
{
    UnderwaterDebris,   // Floating debris particles
    DustParticles,      // Dust motes in air
    BloodSplatter,      // Blood impact effects
    WaterDrips,         // Dripping water
    SteamVents,         // Steam from vents
    EmergencyLights,    // Emergency light particles
    BubbleTrail,        // Underwater bubbles
    SparkShower         // Electrical sparks
};
```

### ScreenEffectManager Component

**Purpose**: Camera shakes and screen-space effects.

**Key Features**:
- 6 camera shake types
- Intensity-based shake scaling
- Directional impact feedback
- Pressure/fear/drowning feedback

**Shake Types**:
```cpp
enum class ECameraShakeType : uint8
{
    Impact,      // Physical impact
    Explosion,   // Explosion shake
    Pressure,    // Pressure shake
    Fear,        // Fear trembling
    Drowning,    // Drowning shake
    Death        // Death shake
};
```

## Blueprint Implementation

### Step 1: Create Post-Process Materials

**Required Materials** (create in `Content/VFX/PostProcess/`):

1. **M_VHSEffect**
   - Base Material Domain: Post Process
   - Blend Mode: Translucent
   - Parameters:
     - `Intensity` (Scalar, default 1.0)
     - `NoiseAmount` (Scalar, default 0.1)
     - `ScanlineSpeed` (Scalar, default 1.0)
     - `ChromaticAberration` (Scalar, default 0.02)

2. **M_UnderwaterEffect**
   - Parameters:
     - `Intensity` (Scalar, default 1.0)
     - `ColorTint` (Vector, default (0.1, 0.3, 0.5))
     - `CausticStrength` (Scalar, default 0.5)

3. **M_PressureVignette**
   - Parameters:
     - `Intensity` (Scalar, default 0.0)
     - `VignetteSize` (Scalar, default 0.5)
     - `VignetteColor` (Vector, default (0, 0, 0))

4. **M_LensDroplets**
   - Parameters:
     - `Intensity` (Scalar, default 0.0)
     - `DropletTexture` (Texture2D)
     - `DropletScale` (Scalar, default 1.0)

5. **M_ChromaticAberration**
   - Parameters:
     - `Amount` (Scalar, default 0.0)
     - `Direction` (Vector, default (1, 0, 0))

6. **M_Scanlines**
   - Parameters:
     - `Intensity` (Scalar, default 0.0)
     - `LineCount` (Scalar, default 480)
     - `Speed` (Scalar, default 1.0)

7. **M_NoiseOverlay**
   - Parameters:
     - `Amount` (Scalar, default 0.0)
     - `NoiseTexture` (Texture2D)
     - `NoiseScale` (Scalar, default 1.0)

8. **M_DeathEffect**
   - Parameters:
     - `Intensity` (Scalar, default 0.0)
     - `DesaturationAmount` (Scalar, default 1.0)
     - `VignetteStrength` (Scalar, default 1.0)

9. **M_FearEffect**
   - Parameters:
     - `Intensity` (Scalar, default 0.0)
     - `DistortionAmount` (Scalar, default 0.1)
     - `VignetteStrength` (Scalar, default 0.5)

### Step 2: Create Niagara Systems

**Required Niagara Systems** (create in `Content/VFX/Niagara/`):

1. **NS_UnderwaterDebris**
   - Emitter: Sprite particles
   - Spawn Rate: 10-20 per second
   - Lifetime: 5-10 seconds
   - Velocity: Slow drift (50-100 units/s)
   - Size: 5-20 units
   - Parameters: `SpawnRate`, `ParticleSize`, `DriftSpeed`

2. **NS_DustParticles**
   - Emitter: Sprite particles
   - Spawn Rate: 20-50 per second
   - Lifetime: 3-8 seconds
   - Velocity: Very slow (10-30 units/s)
   - Size: 2-8 units
   - Parameters: `SpawnRate`, `ParticleSize`, `DriftSpeed`

3. **NS_BloodSplatter**
   - Emitter: Mesh particles (blood droplets)
   - Spawn Rate: Burst (50-100 particles)
   - Lifetime: 0.5-2 seconds
   - Velocity: Explosive (200-500 units/s)
   - Size: 3-10 units
   - Parameters: `BurstCount`, `ExplosionForce`, `Color`

4. **NS_WaterDrips**
   - Emitter: Sprite particles
   - Spawn Rate: 1-5 per second
   - Lifetime: 1-3 seconds
   - Velocity: Gravity-based (falling)
   - Size: 2-5 units
   - Parameters: `DripRate`, `DropSize`

5. **NS_SteamVents**
   - Emitter: Sprite particles
   - Spawn Rate: 30-60 per second
   - Lifetime: 2-5 seconds
   - Velocity: Upward (100-200 units/s)
   - Size: 20-50 units (growing)
   - Parameters: `SpawnRate`, `VentForce`, `SteamColor`

6. **NS_EmergencyLights**
   - Emitter: Light particles
   - Spawn Rate: Continuous
   - Lifetime: Infinite (looping)
   - Velocity: None (static)
   - Size: Light radius 500-1000 units
   - Parameters: `LightColor`, `LightIntensity`, `FlashRate`

7. **NS_BubbleTrail**
   - Emitter: Sprite particles
   - Spawn Rate: 10-30 per second
   - Lifetime: 2-5 seconds
   - Velocity: Upward with wobble (50-150 units/s)
   - Size: 5-15 units
   - Parameters: `BubbleRate`, `BubbleSize`, `RiseSpeed`

8. **NS_SparkShower**
   - Emitter: Sprite particles with trails
   - Spawn Rate: Burst (100-200 particles)
   - Lifetime: 0.3-1 second
   - Velocity: Explosive with gravity (300-800 units/s)
   - Size: 2-5 units
   - Parameters: `SparkCount`, `ExplosionForce`, `SparkColor`

### Step 3: Create Camera Shake Classes

**Required Camera Shakes** (create in `Content/VFX/CameraShakes/`):

1. **CS_Impact**
   - Blueprint Class → Camera Shake Base
   - Oscillation Duration: 0.3s
   - Rotation Amplitude: (Pitch: 2, Yaw: 2, Roll: 1)
   - Frequency: 20 Hz

2. **CS_Explosion**
   - Oscillation Duration: 0.8s
   - Rotation Amplitude: (Pitch: 5, Yaw: 5, Roll: 3)
   - Location Amplitude: (X: 10, Y: 10, Z: 5)
   - Frequency: 15 Hz

3. **CS_Pressure**
   - Oscillation Duration: Continuous (looping)
   - Rotation Amplitude: (Pitch: 0.5, Yaw: 0.5, Roll: 0.3)
   - Frequency: 5 Hz (slow)

4. **CS_Fear**
   - Oscillation Duration: Continuous (looping)
   - Rotation Amplitude: (Pitch: 1, Yaw: 1, Roll: 0.5)
   - Frequency: 10 Hz (trembling)

5. **CS_Drowning**
   - Oscillation Duration: Continuous (looping)
   - Rotation Amplitude: (Pitch: 2, Yaw: 2, Roll: 1.5)
   - Frequency: 3 Hz (slow, heavy)

6. **CS_Death**
   - Oscillation Duration: 2.0s
   - Rotation Amplitude: (Pitch: 10, Yaw: 5, Roll: 5)
   - Location Amplitude: (X: 20, Y: 20, Z: 10)
   - Frequency: 8 Hz
   - Blend Out: 1.0s

### Step 4: Create BP_VFXManager Actor

**Purpose**: Central VFX management actor for the level.

**Creation Steps**:
1. Content Browser → Blueprint Class → Actor
2. Name: `BP_VFXManager`
3. Add Components:
   - PostProcessController (name: `PostProcessCtrl`)
   - ParticleSpawner (name: `ParticleSpawner`)
   - ScreenEffectManager (name: `ScreenEffects`)

**Component Configuration**:

**PostProcessController**:
- Find or create PostProcessVolume in level
- Assign to `PostProcessVolume` property
- Configure `EffectMaterials` map:
  - VHS → M_VHSEffect
  - Underwater → M_UnderwaterEffect
  - PressureVignette → M_PressureVignette
  - LensDroplets → M_LensDroplets
  - ChromaticAberration → M_ChromaticAberration
  - Scanlines → M_Scanlines
  - NoiseOverlay → M_NoiseOverlay
  - DeathEffect → M_DeathEffect
  - FearEffect → M_FearEffect

**ParticleSpawner**:
- Configure `ParticleSystems` map:
  - UnderwaterDebris → NS_UnderwaterDebris
  - DustParticles → NS_DustParticles
  - BloodSplatter → NS_BloodSplatter
  - WaterDrips → NS_WaterDrips
  - SteamVents → NS_SteamVents
  - EmergencyLights → NS_EmergencyLights
  - BubbleTrail → NS_BubbleTrail
  - SparkShower → NS_SparkShower
- Set `MaxActiveParticles`: 1000
- Set `ParticleLODDistance`: 2000
- Enable `bEnableParticleLOD`: true
- Enable `bEnableParticleCulling`: true

**ScreenEffectManager**:
- Configure `CameraShakeClasses` map:
  - Impact → CS_Impact
  - Explosion → CS_Explosion
  - Pressure → CS_Pressure
  - Fear → CS_Fear
  - Drowning → CS_Drowning
  - Death → CS_Death
- Set `MaxShakeIntensity`: 2.0
- Enable `bEnableScreenEffects`: true

### Step 5: Blueprint Usage Examples

#### Example 1: Apply VHS Effect on Event

**Event Graph**:
```
Event BeginPlay
  → Get Component by Class (PostProcessController)
  → Apply Effect
     - Effect Type: VHS
     - Intensity: 0.8
     - Duration: 0.0 (permanent)
```

#### Example 2: Spawn Blood Splatter on Hit

**Event Graph**:
```
Event AnyDamage
  → Get Component by Class (ParticleSpawner)
  → Spawn Effect At Location
     - Effect Type: BloodSplatter
     - Location: Hit Location
     - Rotation: Hit Normal to Rotation
```

#### Example 3: Apply Impact Shake

**Event Graph**:
```
On Impact Event
  → Get Component by Class (ScreenEffectManager)
  → Apply Camera Shake
     - Shake Type: Impact
     - Intensity: 1.0
```

#### Example 4: Pressure Effect Based on Depth

**Event Graph** (Tick):
```
Event Tick
  → Get Actor Location
  → Break Vector (get Z)
  → Map Range In Range (Z: -5000 to 0, Out: 0 to 1)
  → Get Component by Class (PostProcessController)
  → Update Pressure Effect
     - Depth: Current Depth
     - Max Depth: 5000
```

#### Example 5: Fear Effect Based on Threat Proximity

**Event Graph** (Tick):
```
Event Tick
  → Get All Actors of Class (ThreatCharacter)
  → For Each Loop
     → Get Distance To
     → Select (if < 1000, calculate fear level)
  → Get Component by Class (PostProcessController)
  → Update Fear Effect
     - Fear Level: Calculated Value (0-1)
```

#### Example 6: Underwater Ambient Particles

**Event Graph**:
```
On Enter Water Volume
  → Get Component by Class (ParticleSpawner)
  → Spawn Attached Effect
     - Effect Type: UnderwaterDebris
     - Attach To Component: Root Component
     - Socket Name: None
  → Spawn Attached Effect
     - Effect Type: BubbleTrail
     - Attach To Component: Camera Component
     - Socket Name: None
```

## Parameter Configuration

### Post-Process Effect Settings

**FPostProcessEffectSettings**:
- `Intensity` (0.0 - 2.0): Effect strength
- `FadeSpeed` (0.1 - 10.0): Fade in/out speed
- `bAutoFade` (bool): Auto-fade after duration
- `Duration` (seconds): Effect duration (0 = permanent)

### Particle Spawn Settings

**FParticleSpawnSettings**:
- `Location` (Vector): Spawn location
- `Rotation` (Rotator): Spawn rotation
- `Scale` (Vector): Particle scale
- `Lifetime` (seconds): Effect lifetime
- `bAutoDestroy` (bool): Auto-cleanup when finished
- `bAttachToParent` (bool): Attach to spawning actor

### Camera Shake Settings

**FCameraShakeSettings**:
- `Intensity` (0.0 - 2.0): Shake strength
- `Duration` (seconds): Shake duration
- `Frequency` (Hz): Oscillation frequency
- `bOrientToCamera` (bool): Orient to camera direction

## Event Binding

### PostProcessController Events

No events (uses direct function calls).

### ParticleSpawner Events

No events (uses direct function calls, returns UNiagaraComponent for control).

### ScreenEffectManager Events

No events (uses direct function calls).

## Common Patterns

### Pattern 1: Temporary Effect

```
Apply Effect (VHS, 1.0, 5.0)  // 5 second duration
  → Delay (5.0)
  → Remove Effect (VHS, true)  // Fade out
```

### Pattern 2: Intensity Ramp

```
Timeline (0 to 1 over 2 seconds)
  → Set Effect Intensity (FearEffect, Timeline Value)
```

### Pattern 3: Particle Burst

```
Spawn Effect At Location (SparkShower, Location)
  → Set Effect Parameter (Result, "SparkCount", 200)
  → Set Effect Color (Result, Orange Color)
```

### Pattern 4: Continuous Shake

```
On Enter Danger Zone
  → Apply Camera Shake (Fear, 0.5)

On Exit Danger Zone
  → Stop All Camera Shakes
```

## Performance Optimization

### Post-Process Effects
- Limit active effects to 3-4 simultaneously
- Use lower intensity for distant effects
- Disable effects when not visible
- Use material LOD for complex effects

### Particle Systems
- Respect `MaxActiveParticles` budget (1000)
- Enable LOD and culling
- Use lower spawn rates at distance
- Cleanup finished effects promptly
- Use GPU particles for large counts

### Camera Shakes
- Limit shake intensity (max 2.0)
- Don't stack multiple shakes
- Use shorter durations for frequent shakes
- Stop shakes when not needed

## Troubleshooting

### Post-Process Not Visible
- Check PostProcessVolume is set and enabled
- Verify material is assigned in EffectMaterials map
- Ensure PostProcessVolume is Unbound
- Check material parameters are exposed

### Particles Not Spawning
- Verify Niagara system is assigned in ParticleSystems map
- Check particle budget not exceeded
- Ensure spawn location is valid
- Check Niagara system is not disabled

### Camera Shake Not Working
- Verify camera shake class is assigned
- Check PlayerCameraManager is valid
- Ensure shake intensity > 0
- Verify shake duration > 0

### Performance Issues
- Check particle count (Stat Particles)
- Monitor post-process cost (ProfileGPU)
- Reduce active effect count
- Enable LOD and culling
- Lower particle spawn rates

## Best Practices

1. **Centralize VFX Management**: Use single BP_VFXManager per level
2. **Pool Particle Effects**: Reuse Niagara components when possible
3. **Fade Effects**: Always fade in/out, never instant transitions
4. **Test Performance**: Profile on target hardware early
5. **Use LOD**: Enable distance-based quality reduction
6. **Respect Budgets**: Stay within particle/effect limits
7. **Clean Up**: Always stop/destroy finished effects
8. **Parameter Exposure**: Expose key parameters for designer control
9. **Visual Consistency**: Match effect intensity to gameplay state
10. **Accessibility**: Provide options to reduce intense effects

## Testing Checklist

- [ ] All post-process materials created and assigned
- [ ] All Niagara systems created and assigned
- [ ] All camera shake classes created and assigned
- [ ] BP_VFXManager placed in level
- [ ] PostProcessVolume configured correctly
- [ ] Effects apply and remove correctly
- [ ] Particles spawn and cleanup properly
- [ ] Camera shakes trigger correctly
- [ ] Performance within budget (60 FPS)
- [ ] No memory leaks (check Stat Memory)
- [ ] Effects visible in all lighting conditions
- [ ] LOD working at distance
- [ ] Culling working correctly

## Next Steps

1. Create all required materials and Niagara systems
2. Set up BP_VFXManager in main level
3. Test each effect individually
4. Integrate with gameplay systems
5. Profile and optimize performance
6. Add designer-friendly controls
7. Document custom parameters
