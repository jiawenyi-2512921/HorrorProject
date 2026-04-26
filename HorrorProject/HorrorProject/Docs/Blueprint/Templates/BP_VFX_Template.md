# Blueprint VFX Template

## Template: BP_VFX_Template

Use this template as a starting point for creating VFX-related Blueprints.

---

## BP_VFXManager Template

**Parent Class**: Actor

**Purpose**: Central VFX management for the level.

### Components
```
Root (SceneComponent)
├── PostProcessController
├── ParticleSpawner
└── ScreenEffectManager
```

### Variables

**Configuration**:
- `bAutoInitialize` (bool, default: true) - Auto-initialize on BeginPlay
- `bEnableVFX` (bool, default: true) - Master VFX enable/disable
- `VFXQualityLevel` (int32, default: 2) - 0=Low, 1=Medium, 2=High, 3=Ultra

**References**:
- `PostProcessVolume` (PostProcessVolume) - Level post-process volume
- `PlayerCharacter` (Character) - Player reference

**State**:
- `ActiveEffects` (Array of Name) - Currently active effects
- `EffectIntensities` (Map of Name to float) - Effect intensity values

### Functions

#### InitializeVFX
```
Called on BeginPlay
  → Find PostProcessVolume in level
  → Get Player Character
  → Configure PostProcessController
  → Configure ParticleSpawner
  → Configure ScreenEffectManager
  → Apply quality settings
```

#### ApplyPostProcessEffect
```
Input: EPostProcessEffectType EffectType, float Intensity, float Duration
  → PostProcessController → Apply Effect
  → Add to ActiveEffects
  → Update EffectIntensities
```

#### SpawnParticleEffect
```
Input: EParticleEffectType EffectType, FVector Location, FRotator Rotation
Return: UNiagaraComponent
  → ParticleSpawner → Spawn Effect At Location
  → Return component reference
```

#### ApplyCameraShake
```
Input: ECameraShakeType ShakeType, float Intensity
  → ScreenEffectManager → Apply Camera Shake
```

#### SetVFXQuality
```
Input: int32 QualityLevel
  → Set VFXQualityLevel
  → Update particle budgets
  → Update post-process quality
  → Update effect LOD settings
```

#### ClearAllEffects
```
  → PostProcessController → Clear All Effects
  → ParticleSpawner → Stop All Effects
  → ScreenEffectManager → Stop All Camera Shakes
  → Clear ActiveEffects array
```

### Event Graph

```
Event BeginPlay
  → Branch (bAutoInitialize)
    True: InitializeVFX

Event EndPlay
  → ClearAllEffects
  → Cleanup references
```

### Usage Example

```
Get VFXManager
  → Apply Post Process Effect
     - Effect Type: VHS
     - Intensity: 0.8
     - Duration: 0.0 (permanent)
```

---

## BP_CustomPostProcessEffect Template

**Parent Class**: Actor

**Purpose**: Custom post-process effect actor.

### Components
```
Root (SceneComponent)
└── PostProcessComponent
```

### Variables

**Configuration**:
- `EffectMaterial` (MaterialInterface) - Post-process material
- `EffectIntensity` (float, default: 1.0) - Effect intensity
- `BlendWeight` (float, default: 1.0) - Blend weight
- `bUnbound` (bool, default: true) - Unbound volume

**Animation**:
- `FadeInDuration` (float, default: 0.5) - Fade in time
- `FadeOutDuration` (float, default: 0.5) - Fade out time
- `CurrentBlendWeight` (float, default: 0.0) - Current blend weight

### Functions

#### FadeIn
```
Timeline (0 to 1 over FadeInDuration)
  → Lerp (0 to BlendWeight)
  → Set PostProcessComponent Blend Weight
```

#### FadeOut
```
Timeline (1 to 0 over FadeOutDuration)
  → Lerp (BlendWeight to 0)
  → Set PostProcessComponent Blend Weight
On Finished:
  → Destroy Actor (if temporary)
```

#### SetEffectIntensity
```
Input: float Intensity
  → Set EffectIntensity
  → Update material parameter "Intensity"
```

### Event Graph

```
Event BeginPlay
  → Create Dynamic Material Instance
  → Set PostProcessComponent Material
  → FadeIn
```

---

## BP_ParticleEffect Template

**Parent Class**: Actor

**Purpose**: Spawnable particle effect actor.

### Components
```
Root (SceneComponent)
└── NiagaraComponent
```

### Variables

**Configuration**:
- `ParticleSystem` (NiagaraSystem) - Niagara system to spawn
- `bAutoActivate` (bool, default: true) - Auto-activate on spawn
- `bAutoDestroy` (bool, default: true) - Auto-destroy when finished
- `Lifetime` (float, default: 5.0) - Effect lifetime

**Parameters**:
- `ParticleColor` (LinearColor, default: White) - Particle color
- `ParticleScale` (float, default: 1.0) - Particle scale
- `SpawnRate` (float, default: 1.0) - Spawn rate multiplier

### Functions

#### ActivateEffect
```
  → NiagaraComponent → Activate
  → Branch (bAutoDestroy)
    True:
      → Delay (Lifetime)
      → Destroy Actor
```

#### DeactivateEffect
```
Input: bool bImmediate
  → Branch (bImmediate)
    True: NiagaraComponent → Deactivate Immediate
    False: NiagaraComponent → Deactivate
```

#### SetParticleParameter
```
Input: FName ParameterName, float Value
  → NiagaraComponent → Set Float Parameter
```

#### SetParticleColor
```
Input: FLinearColor Color
  → Set ParticleColor
  → NiagaraComponent → Set Color Parameter ("ParticleColor", Color)
```

### Event Graph

```
Event BeginPlay
  → Set NiagaraComponent System (ParticleSystem)
  → Apply initial parameters
  → Branch (bAutoActivate)
    True: ActivateEffect
```

---

## BP_CameraShake Template

**Parent Class**: CameraShakeBase

**Purpose**: Custom camera shake.

### Variables

**Oscillation**:
- `OscillationDuration` (float, default: 0.5) - Shake duration
- `RotationAmplitude` (Rotator, default: (2, 2, 1)) - Rotation shake amount
- `LocationAmplitude` (Vector, default: (0, 0, 0)) - Location shake amount
- `FOVAmplitude` (float, default: 0.0) - FOV shake amount

**Frequency**:
- `RotationFrequency` (Rotator, default: (10, 10, 10)) - Rotation frequency
- `LocationFrequency` (Vector, default: (10, 10, 10)) - Location frequency
- `FOVFrequency` (float, default: 10.0) - FOV frequency

**Blend**:
- `BlendInTime` (float, default: 0.1) - Blend in time
- `BlendOutTime` (float, default: 0.2) - Blend out time

### Configuration

Set these values in the Blueprint defaults panel:
- Oscillation Duration
- Rotation/Location/FOV Amplitude
- Rotation/Location/FOV Frequency
- Blend In/Out Time

### Usage Example

```
Get Player Camera Manager
  → Start Camera Shake
     - Shake Class: BP_CameraShake_Impact
     - Scale: 1.0
     - Play Space: Camera Local
```

---

## Best Practices

### Post-Process Effects
1. Always use material instances for runtime parameters
2. Fade effects in/out, never instant transitions
3. Limit active post-process effects to 3-4
4. Use unbound volumes for global effects
5. Test effects in all lighting conditions

### Particle Effects
1. Set reasonable particle budgets
2. Enable LOD and culling
3. Use GPU particles for large counts
4. Auto-destroy finished effects
5. Pool frequently used effects

### Camera Shakes
1. Keep shake durations short (< 1 second)
2. Use appropriate frequencies (5-20 Hz)
3. Don't stack multiple shakes
4. Scale shake intensity with impact strength
5. Provide option to disable for accessibility

### Performance
1. Profile VFX regularly (ProfileGPU)
2. Monitor particle counts (Stat Particles)
3. Check post-process cost (Stat RHI)
4. Optimize material complexity
5. Use LOD aggressively

### Organization
1. Name effects descriptively (BP_VFX_BloodSplatter)
2. Group related effects in folders
3. Use consistent parameter names
4. Document effect usage
5. Create reusable templates

---

## Testing Checklist

- [ ] Effect spawns correctly
- [ ] Effect parameters work
- [ ] Effect cleans up properly
- [ ] Effect performs well
- [ ] Effect looks good in all lighting
- [ ] Effect integrates with other systems
- [ ] Effect can be disabled for accessibility

---

## Common Issues

### Effect Not Visible
- Check material is assigned
- Verify blend weight > 0
- Ensure post-process volume is unbound
- Check effect is in camera view

### Particles Not Spawning
- Verify Niagara system is assigned
- Check particle budget not exceeded
- Ensure spawn location is valid
- Verify system is activated

### Performance Issues
- Reduce particle count
- Lower material complexity
- Enable LOD and culling
- Limit active effects
- Use simpler shaders

---

## Example Implementation

See these files for reference:
- `/Content/VFX/BP_VFXManager`
- `/Content/VFX/PostProcess/BP_VHSEffect`
- `/Content/VFX/Particles/BP_BloodSplatter`
- `/Content/VFX/CameraShakes/BP_ImpactShake`
