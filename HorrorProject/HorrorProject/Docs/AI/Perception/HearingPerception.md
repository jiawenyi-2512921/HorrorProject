# Hearing Perception Configuration

## Overview
Configuration guide for AI hearing perception, enabling threats to detect and respond to sounds made by the player and environment.

## Hearing Configuration Parameters

### Basic Parameters

```cpp
UPROPERTY(EditAnywhere, Category = "Hearing")
float HearingRange = 1000.0f;  // Maximum hearing distance (cm)

UPROPERTY(EditAnywhere, Category = "Hearing")
FAISenseAffiliationFilter DetectionByAffiliation;  // What to hear

UPROPERTY(EditAnywhere, Category = "Hearing")
bool bUseLoSHearing = true;  // Require line of sight for hearing

UPROPERTY(EditAnywhere, Category = "Hearing")
float LoSHearingRange = 500.0f;  // Range for LoS hearing
```

## Configuration Presets

### Golem Configuration
Medium-range hearing for footsteps and loud noises.

```cpp
void ConfigureGolemHearing(UAISenseConfig_Hearing* HearingConfig)
{
    HearingConfig->HearingRange = 1500.0f;           // 15m hearing range
    HearingConfig->bUseLoSHearing = false;           // Can hear through walls
    HearingConfig->LoSHearingRange = 0.0f;
    
    // Detection settings
    HearingConfig->DetectionByAffiliation.bDetectEnemies = true;
    HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
    HearingConfig->DetectionByAffiliation.bDetectFriendlies = false;
    
    HearingConfig->Implementation = UAISense_Hearing::StaticClass();
}
```

### Shadow Configuration
Long-range, sensitive hearing for stealth gameplay.

```cpp
void ConfigureShadowHearing(UAISenseConfig_Hearing* HearingConfig)
{
    HearingConfig->HearingRange = 2500.0f;           // 25m hearing range
    HearingConfig->bUseLoSHearing = false;           // Supernatural hearing
    HearingConfig->LoSHearingRange = 0.0f;
    
    HearingConfig->DetectionByAffiliation.bDetectEnemies = true;
    HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
    HearingConfig->DetectionByAffiliation.bDetectFriendlies = false;
    
    HearingConfig->Implementation = UAISense_Hearing::StaticClass();
}
```

### Hunter Configuration
Balanced hearing with LoS requirement.

```cpp
void ConfigureHunterHearing(UAISenseConfig_Hearing* HearingConfig)
{
    HearingConfig->HearingRange = 2000.0f;           // 20m hearing range
    HearingConfig->bUseLoSHearing = true;            // Realistic hearing
    HearingConfig->LoSHearingRange = 800.0f;         // 8m through walls
    
    HearingConfig->DetectionByAffiliation.bDetectEnemies = true;
    HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
    HearingConfig->DetectionByAffiliation.bDetectFriendlies = false;
    
    HearingConfig->Implementation = UAISense_Hearing::StaticClass();
}
```

### Patrol Guard Configuration
Short-range, focused hearing.

```cpp
void ConfigurePatrolGuardHearing(UAISenseConfig_Hearing* HearingConfig)
{
    HearingConfig->HearingRange = 800.0f;            // 8m hearing range
    HearingConfig->bUseLoSHearing = true;            // Realistic hearing
    HearingConfig->LoSHearingRange = 300.0f;         // 3m through walls
    
    HearingConfig->DetectionByAffiliation.bDetectEnemies = true;
    HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
    HearingConfig->DetectionByAffiliation.bDetectFriendlies = false;
    
    HearingConfig->Implementation = UAISense_Hearing::StaticClass();
}
```

## Noise Generation

### Player Footsteps
```cpp
// In AHorrorPlayerCharacter
void AHorrorPlayerCharacter::MakeFootstepNoise()
{
    float Loudness = CalculateFootstepLoudness();
    FVector NoiseLocation = GetActorLocation();
    
    UAISense_Hearing::ReportNoiseEvent(
        GetWorld(),
        NoiseLocation,
        Loudness,
        this,
        0.0f,  // Max range (0 = use AI's hearing range)
        FName("Footstep")
    );
}

float AHorrorPlayerCharacter::CalculateFootstepLoudness() const
{
    float Speed = GetVelocity().Size();
    
    // Crouching
    if (bIsCrouched)
    {
        return 0.2f;  // Very quiet
    }
    // Walking
    else if (Speed < 300.0f)
    {
        return 0.5f;  // Quiet
    }
    // Running
    else if (Speed < 600.0f)
    {
        return 0.8f;  // Loud
    }
    // Sprinting
    else
    {
        return 1.0f;  // Very loud
    }
}
```

### Door Interactions
```cpp
void AHorrorDoor::OnDoorOpened()
{
    float Loudness = bIsLocked ? 0.9f : 0.6f;  // Locked doors are louder
    
    UAISense_Hearing::ReportNoiseEvent(
        GetWorld(),
        GetActorLocation(),
        Loudness,
        GetInstigator(),
        0.0f,
        FName("DoorOpen")
    );
}

void AHorrorDoor::OnDoorSlammed()
{
    UAISense_Hearing::ReportNoiseEvent(
        GetWorld(),
        GetActorLocation(),
        1.0f,  // Maximum loudness
        GetInstigator(),
        0.0f,
        FName("DoorSlam")
    );
}
```

### Object Interactions
```cpp
void AHorrorInteractable::MakeInteractionNoise(float LoudnessMultiplier)
{
    float BaseLoudness = 0.5f;
    float FinalLoudness = FMath::Clamp(BaseLoudness * LoudnessMultiplier, 0.0f, 1.0f);
    
    UAISense_Hearing::ReportNoiseEvent(
        GetWorld(),
        GetActorLocation(),
        FinalLoudness,
        GetLastInteractor(),
        0.0f,
        FName("ObjectInteraction")
    );
}
```

### Environmental Sounds
```cpp
void AHorrorEnvironmentSound::TriggerSound(FVector Location, float Loudness, FName SoundTag)
{
    UAISense_Hearing::ReportNoiseEvent(
        GetWorld(),
        Location,
        Loudness,
        nullptr,  // No instigator for environmental sounds
        0.0f,
        SoundTag
    );
}

// Examples:
// Glass breaking
TriggerSound(BreakLocation, 0.9f, FName("GlassBreak"));

// Metal clang
TriggerSound(ClanLocation, 0.7f, FName("MetalClang"));

// Distant thunder
TriggerSound(ThunderLocation, 0.4f, FName("Thunder"));
```

## Hearing Stimulus Processing

### Basic Hearing Handler
```cpp
void AHorrorThreatAIController::HandleHearingStimulus(AActor* Actor, FAIStimulus Stimulus)
{
    if (!Stimulus.WasSuccessfullySensed()) return;
    
    // Calculate noise strength based on distance
    float Distance = FVector::Dist(
        GetPawn()->GetActorLocation(),
        Stimulus.StimulusLocation
    );
    
    float NoiseStrength = Stimulus.Strength * (1.0f - (Distance / 1500.0f));
    NoiseStrength = FMath::Clamp(NoiseStrength, 0.0f, 1.0f);
    
    // Update blackboard
    if (Blackboard)
    {
        Blackboard->SetValueAsVector("NoiseLocation", Stimulus.StimulusLocation);
        Blackboard->SetValueAsFloat("NoiseStrength", NoiseStrength);
        
        // Escalate alert state based on noise strength
        EAlertState CurrentState = static_cast<EAlertState>(
            Blackboard->GetValueAsEnum("AlertState")
        );
        
        if (NoiseStrength > 0.7f && CurrentState < EAlertState::Alert)
        {
            Blackboard->SetValueAsEnum("AlertState", static_cast<uint8>(EAlertState::Alert));
        }
        else if (NoiseStrength > 0.3f && CurrentState == EAlertState::Idle)
        {
            Blackboard->SetValueAsEnum("AlertState", static_cast<uint8>(EAlertState::Suspicious));
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Hearing: Heard noise at %s (strength: %.2f)"),
        *Stimulus.StimulusLocation.ToString(),
        NoiseStrength
    );
}
```

### Advanced Hearing with Sound Classification
```cpp
void AHorrorThreatAIController::HandleHearingStimulus(AActor* Actor, FAIStimulus Stimulus)
{
    if (!Stimulus.WasSuccessfullySensed()) return;
    
    // Get sound tag
    FName SoundTag = Stimulus.Tag;
    
    // Classify sound type
    ESoundType SoundType = ClassifySound(SoundTag);
    
    // Calculate response based on sound type
    float ThreatLevel = GetSoundThreatLevel(SoundType);
    float InvestigationPriority = GetSoundInvestigationPriority(SoundType);
    
    // Update blackboard
    if (Blackboard)
    {
        Blackboard->SetValueAsVector("NoiseLocation", Stimulus.StimulusLocation);
        Blackboard->SetValueAsFloat("NoiseThreatLevel", ThreatLevel);
        Blackboard->SetValueAsFloat("InvestigationPriority", InvestigationPriority);
        
        // Set investigation location if priority is high enough
        if (InvestigationPriority > 0.5f)
        {
            Blackboard->SetValueAsVector("InvestigateLocation", Stimulus.StimulusLocation);
        }
        
        // Update alert state
        UpdateAlertStateFromSound(ThreatLevel);
    }
    
    // Log with sound type
    UE_LOG(LogTemp, Log, TEXT("Hearing: Heard %s at %s (threat: %.2f, priority: %.2f)"),
        *SoundTag.ToString(),
        *Stimulus.StimulusLocation.ToString(),
        ThreatLevel,
        InvestigationPriority
    );
}

ESoundType AHorrorThreatAIController::ClassifySound(FName SoundTag) const
{
    if (SoundTag == "Footstep") return ESoundType::Footstep;
    if (SoundTag == "DoorOpen") return ESoundType::DoorOpen;
    if (SoundTag == "DoorSlam") return ESoundType::DoorSlam;
    if (SoundTag == "GlassBreak") return ESoundType::GlassBreak;
    if (SoundTag == "MetalClang") return ESoundType::MetalClang;
    if (SoundTag == "ObjectInteraction") return ESoundType::ObjectInteraction;
    
    return ESoundType::Unknown;
}

float AHorrorThreatAIController::GetSoundThreatLevel(ESoundType SoundType) const
{
    switch (SoundType)
    {
        case ESoundType::Footstep:          return 0.5f;
        case ESoundType::DoorOpen:          return 0.6f;
        case ESoundType::DoorSlam:          return 0.9f;
        case ESoundType::GlassBreak:        return 0.8f;
        case ESoundType::MetalClang:        return 0.7f;
        case ESoundType::ObjectInteraction: return 0.5f;
        default:                            return 0.3f;
    }
}

float AHorrorThreatAIController::GetSoundInvestigationPriority(ESoundType SoundType) const
{
    switch (SoundType)
    {
        case ESoundType::Footstep:          return 0.7f;
        case ESoundType::DoorOpen:          return 0.8f;
        case ESoundType::DoorSlam:          return 0.9f;
        case ESoundType::GlassBreak:        return 0.9f;
        case ESoundType::MetalClang:        return 0.6f;
        case ESoundType::ObjectInteraction: return 0.7f;
        default:                            return 0.5f;
    }
}
```

## Sound Occlusion

### Basic Occlusion
```cpp
float AHorrorThreatAIController::CalculateSoundOcclusion(FVector SoundLocation) const
{
    APawn* ControlledPawn = GetPawn();
    if (!ControlledPawn) return 1.0f;
    
    FVector ListenerLocation = ControlledPawn->GetActorLocation();
    
    FHitResult HitResult;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(ControlledPawn);
    
    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        ListenerLocation,
        SoundLocation,
        ECC_Visibility,
        Params
    );
    
    if (!bHit)
    {
        return 1.0f;  // No occlusion
    }
    
    // Check material type for occlusion amount
    if (HitResult.PhysMaterial.IsValid())
    {
        // Different materials block sound differently
        // This is simplified - actual implementation would use material properties
        return 0.5f;  // 50% occlusion
    }
    
    return 0.7f;  // Default occlusion
}
```

### Advanced Occlusion with Material Properties
```cpp
float AHorrorThreatAIController::CalculateAdvancedSoundOcclusion(
    FVector SoundLocation,
    float& OutOcclusionFactor) const
{
    APawn* ControlledPawn = GetPawn();
    if (!ControlledPawn)
    {
        OutOcclusionFactor = 1.0f;
        return 1.0f;
    }
    
    FVector ListenerLocation = ControlledPawn->GetActorLocation();
    
    // Multiple raycasts for better accuracy
    TArray<FVector> TraceOffsets;
    TraceOffsets.Add(FVector(0, 0, 0));      // Center
    TraceOffsets.Add(FVector(0, 0, 100));    // High
    TraceOffsets.Add(FVector(0, 0, -50));    // Low
    TraceOffsets.Add(FVector(50, 0, 0));     // Right
    TraceOffsets.Add(FVector(-50, 0, 0));    // Left
    
    float TotalOcclusion = 0.0f;
    int32 HitCount = 0;
    
    for (const FVector& Offset : TraceOffsets)
    {
        FHitResult HitResult;
        FCollisionQueryParams Params;
        Params.AddIgnoredActor(ControlledPawn);
        
        bool bHit = GetWorld()->LineTraceSingleByChannel(
            HitResult,
            ListenerLocation + Offset,
            SoundLocation + Offset,
            ECC_Visibility,
            Params
        );
        
        if (bHit)
        {
            HitCount++;
            TotalOcclusion += GetMaterialSoundOcclusion(HitResult.PhysMaterial.Get());
        }
    }
    
    if (HitCount == 0)
    {
        OutOcclusionFactor = 1.0f;
        return 1.0f;
    }
    
    float AverageOcclusion = TotalOcclusion / HitCount;
    OutOcclusionFactor = 1.0f - AverageOcclusion;
    
    return OutOcclusionFactor;
}

float AHorrorThreatAIController::GetMaterialSoundOcclusion(UPhysicalMaterial* Material) const
{
    if (!Material) return 0.5f;
    
    // Material-based occlusion values
    FName MaterialName = Material->GetFName();
    
    if (MaterialName.ToString().Contains("Wood"))
        return 0.3f;  // Wood blocks 30%
    if (MaterialName.ToString().Contains("Concrete"))
        return 0.7f;  // Concrete blocks 70%
    if (MaterialName.ToString().Contains("Metal"))
        return 0.8f;  // Metal blocks 80%
    if (MaterialName.ToString().Contains("Glass"))
        return 0.2f;  // Glass blocks 20%
    if (MaterialName.ToString().Contains("Fabric"))
        return 0.4f;  // Fabric blocks 40%
    
    return 0.5f;  // Default 50%
}
```

## Sound Memory System

### Remember Recent Sounds
```cpp
struct FSoundMemory
{
    FVector Location;
    float Loudness;
    FName SoundTag;
    float Timestamp;
    AActor* Instigator;
};

class AHorrorThreatAIController : public AAIController
{
private:
    TArray<FSoundMemory> RecentSounds;
    float SoundMemoryDuration = 10.0f;  // Remember sounds for 10 seconds
    
public:
    void AddSoundToMemory(FVector Location, float Loudness, FName Tag, AActor* Instigator)
    {
        FSoundMemory NewSound;
        NewSound.Location = Location;
        NewSound.Loudness = Loudness;
        NewSound.SoundTag = Tag;
        NewSound.Timestamp = GetWorld()->GetTimeSeconds();
        NewSound.Instigator = Instigator;
        
        RecentSounds.Add(NewSound);
        
        // Clean old sounds
        CleanOldSounds();
    }
    
    void CleanOldSounds()
    {
        float CurrentTime = GetWorld()->GetTimeSeconds();
        
        RecentSounds.RemoveAll([CurrentTime, this](const FSoundMemory& Sound)
        {
            return (CurrentTime - Sound.Timestamp) > SoundMemoryDuration;
        });
    }
    
    TArray<FSoundMemory> GetRecentSoundsInArea(FVector Location, float Radius) const
    {
        TArray<FSoundMemory> NearSounds;
        
        for (const FSoundMemory& Sound : RecentSounds)
        {
            float Distance = FVector::Dist(Sound.Location, Location);
            if (Distance <= Radius)
            {
                NearSounds.Add(Sound);
            }
        }
        
        return NearSounds;
    }
    
    FSoundMemory GetLoudestRecentSound() const
    {
        FSoundMemory LoudestSound;
        float MaxLoudness = 0.0f;
        
        for (const FSoundMemory& Sound : RecentSounds)
        {
            if (Sound.Loudness > MaxLoudness)
            {
                MaxLoudness = Sound.Loudness;
                LoudestSound = Sound;
            }
        }
        
        return LoudestSound;
    }
};
```

## Debug Visualization

### Draw Hearing Range
```cpp
void AHorrorThreatAIController::DrawHearingDebug() const
{
    APawn* ControlledPawn = GetPawn();
    if (!ControlledPawn) return;
    
    FVector PawnLocation = ControlledPawn->GetActorLocation();
    
    // Draw hearing radius
    DrawDebugSphere(
        GetWorld(),
        PawnLocation,
        1500.0f,  // Hearing range
        32,
        FColor::Yellow,
        false,
        -1.0f,
        0,
        2.0f
    );
    
    // Draw recent sounds
    for (const FSoundMemory& Sound : RecentSounds)
    {
        float Age = GetWorld()->GetTimeSeconds() - Sound.Timestamp;
        float Alpha = 1.0f - (Age / SoundMemoryDuration);
        
        FColor SoundColor = FColor::Orange;
        SoundColor.A = static_cast<uint8>(Alpha * 255);
        
        DrawDebugSphere(
            GetWorld(),
            Sound.Location,
            50.0f * Sound.Loudness,
            12,
            SoundColor,
            false,
            -1.0f,
            0,
            2.0f
        );
        
        // Draw line to sound
        DrawDebugLine(
            GetWorld(),
            PawnLocation,
            Sound.Location,
            SoundColor,
            false,
            -1.0f,
            0,
            1.0f
        );
    }
}
```

## Performance Optimization

### Update Frequency by Distance
```cpp
void AHorrorThreatAIController::UpdateHearingFrequency(float DistanceToPlayer)
{
    if (!PerceptionComponent) return;
    
    float UpdateInterval;
    
    if (DistanceToPlayer < 2000.0f)  // < 20m
    {
        UpdateInterval = 0.2f;  // 5 Hz
    }
    else if (DistanceToPlayer < 5000.0f)  // < 50m
    {
        UpdateInterval = 0.5f;  // 2 Hz
    }
    else
    {
        UpdateInterval = 1.0f;  // 1 Hz
    }
    
    PerceptionComponent->SetSenseUpdateInterval(
        UAISense_Hearing::StaticClass(),
        UpdateInterval
    );
}
```

### Batch Sound Processing
```cpp
void AHorrorThreatAIController::ProcessSoundBatch(const TArray<FAIStimulus>& Stimuli)
{
    if (Stimuli.Num() == 0) return;
    
    // Find loudest sound in batch
    const FAIStimulus* LoudestStimulus = nullptr;
    float MaxLoudness = 0.0f;
    
    for (const FAIStimulus& Stimulus : Stimuli)
    {
        if (Stimulus.Strength > MaxLoudness)
        {
            MaxLoudness = Stimulus.Strength;
            LoudestStimulus = &Stimulus;
        }
    }
    
    // Process only the loudest sound
    if (LoudestStimulus)
    {
        HandleHearingStimulus(nullptr, *LoudestStimulus);
    }
}
```

## Testing

### Test Scenarios
1. Footstep detection at various distances
2. Door sounds through walls
3. Multiple simultaneous sounds
4. Sound occlusion by materials
5. Sound memory persistence
6. Alert state escalation
7. Investigation behavior

### Debug Commands
```
showdebug ai
ai.debug [ThreatName]
```

## See Also
- [PerceptionSystemSetup.md](PerceptionSystemSetup.md) - Main perception setup
- [SightPerception.md](SightPerception.md) - Sight configuration
- [DamagePerception.md](DamagePerception.md) - Damage configuration
