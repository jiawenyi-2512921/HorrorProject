# HorrorAudioSubsystem

Main audio management subsystem for the HorrorProject. Handles ambient audio, music, sound effects, and audio zones.

## Class Declaration

```cpp
UCLASS()
class HORRORPROJECT_API UHorrorAudioSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()
};
```

**Header**: `Audio/HorrorAudioSubsystem.h`  
**Module**: HorrorProject  
**Inherits**: UWorldSubsystem

## Overview

The HorrorAudioSubsystem provides centralized audio management for the entire game. It handles:
- Ambient sound playback and transitions
- Music system with dynamic intensity
- Audio zone management
- Sound effect pooling and optimization
- Audio state persistence

## Public Functions

### Initialize

```cpp
virtual void Initialize(FSubsystemCollectionBase& Collection) override;
```

Initializes the audio subsystem. Called automatically by the engine.

**Parameters**: None  
**Returns**: void

---

### PlayAmbientSound

```cpp
UFUNCTION(BlueprintCallable, Category = "Horror Audio")
void PlayAmbientSound(USoundBase* Sound, float FadeInTime = 2.0f);
```

Plays an ambient sound with optional fade-in.

**Parameters**:
- `Sound` [Required] - The sound asset to play
- `FadeInTime` [Optional] - Fade-in duration in seconds (default: 2.0)

**Returns**: void

**Example**:
```cpp
UHorrorAudioSubsystem* AudioSys = GetWorld()->GetSubsystem<UHorrorAudioSubsystem>();
AudioSys->PlayAmbientSound(AmbientSound, 3.0f);
```

---

### StopAmbientSound

```cpp
UFUNCTION(BlueprintCallable, Category = "Horror Audio")
void StopAmbientSound(float FadeOutTime = 2.0f);
```

Stops the currently playing ambient sound.

**Parameters**:
- `FadeOutTime` [Optional] - Fade-out duration in seconds (default: 2.0)

**Returns**: void

---

### SetMusicIntensity

```cpp
UFUNCTION(BlueprintCallable, Category = "Horror Audio")
void SetMusicIntensity(float Intensity);
```

Sets the music intensity level (0.0 to 1.0).

**Parameters**:
- `Intensity` [Required] - Intensity value (0.0 = calm, 1.0 = maximum tension)

**Returns**: void

**Example**:
```cpp
// Increase tension when threat is nearby
AudioSubsystem->SetMusicIntensity(0.8f);
```

---

### RegisterAudioZone

```cpp
UFUNCTION(BlueprintCallable, Category = "Horror Audio")
void RegisterAudioZone(AHorrorAudioZoneActor* Zone);
```

Registers an audio zone with the subsystem.

**Parameters**:
- `Zone` [Required] - The audio zone actor to register

**Returns**: void

---

### UnregisterAudioZone

```cpp
UFUNCTION(BlueprintCallable, Category = "Horror Audio")
void UnregisterAudioZone(AHorrorAudioZoneActor* Zone);
```

Unregisters an audio zone from the subsystem.

**Parameters**:
- `Zone` [Required] - The audio zone actor to unregister

**Returns**: void

---

### GetCurrentAudioZone

```cpp
UFUNCTION(BlueprintPure, Category = "Horror Audio")
AHorrorAudioZoneActor* GetCurrentAudioZone() const;
```

Gets the currently active audio zone.

**Parameters**: None  
**Returns**: AHorrorAudioZoneActor* - Current audio zone or nullptr

---

### PlaySoundAtLocation

```cpp
UFUNCTION(BlueprintCallable, Category = "Horror Audio")
UAudioComponent* PlaySoundAtLocation(USoundBase* Sound, FVector Location, float VolumeMultiplier = 1.0f);
```

Plays a sound at a specific world location.

**Parameters**:
- `Sound` [Required] - The sound to play
- `Location` [Required] - World location for the sound
- `VolumeMultiplier` [Optional] - Volume multiplier (default: 1.0)

**Returns**: UAudioComponent* - The spawned audio component

---

## Public Properties

### MaxAmbientSounds

```cpp
UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Audio Settings")
int32 MaxAmbientSounds = 3;
```

Maximum number of simultaneous ambient sounds.

---

### DefaultAttenuationSettings

```cpp
UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Audio Settings")
USoundAttenuation* DefaultAttenuationSettings;
```

Default attenuation settings for 3D sounds.

---

## Events

### OnAudioZoneChanged

```cpp
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAudioZoneChanged, 
    AHorrorAudioZoneActor*, OldZone, 
    AHorrorAudioZoneActor*, NewZone);

UPROPERTY(BlueprintAssignable, Category = "Horror Audio")
FOnAudioZoneChanged OnAudioZoneChanged;
```

Fired when the player enters a new audio zone.

**Parameters**:
- `OldZone` - Previous audio zone (can be nullptr)
- `NewZone` - New audio zone (can be nullptr)

---

## Usage Examples

### Basic Ambient Sound

```cpp
void AMyGameMode::BeginPlay()
{
    Super::BeginPlay();
    
    UHorrorAudioSubsystem* AudioSys = GetWorld()->GetSubsystem<UHorrorAudioSubsystem>();
    if (AudioSys && AmbientForestSound)
    {
        AudioSys->PlayAmbientSound(AmbientForestSound, 5.0f);
    }
}
```

### Dynamic Music Intensity

```cpp
void AMyThreatAI::OnPlayerDetected()
{
    UHorrorAudioSubsystem* AudioSys = GetWorld()->GetSubsystem<UHorrorAudioSubsystem>();
    if (AudioSys)
    {
        // Ramp up tension
        AudioSys->SetMusicIntensity(0.9f);
    }
}
```

### Audio Zone Setup

```cpp
void AMyLevel::SetupAudioZones()
{
    UHorrorAudioSubsystem* AudioSys = GetWorld()->GetSubsystem<UHorrorAudioSubsystem>();
    
    for (AHorrorAudioZoneActor* Zone : AudioZones)
    {
        AudioSys->RegisterAudioZone(Zone);
    }
}
```

## Blueprint Usage

![Audio Subsystem Blueprint](../../_images/api/audio_subsystem_bp.png)

1. Get Horror Audio Subsystem node
2. Call desired function (Play Ambient Sound, Set Music Intensity, etc.)
3. Connect execution pins

## Performance Considerations

- Limit simultaneous ambient sounds using `MaxAmbientSounds`
- Use audio zones to reduce active sound count
- Pool frequently used sound effects
- Use appropriate attenuation settings for 3D sounds

## Related Classes

- [HorrorAudioLibrary](HorrorAudioLibrary.md)
- [HorrorAudioZoneActor](HorrorAudioZoneActor.md)
- [AmbientAudioComponent](AmbientAudioComponent.md)

## See Also

- [Audio System Overview](../../Architecture/SystemOverview.md#audio-system)
- [Audio Blueprint Guide](../../Blueprint/Audio_Blueprint_Guide.md)
- [Audio Performance Optimization](../../Performance/AudioOptimization.md)
