# Perception System Setup Guide

## Overview
Complete guide for setting up the AI Perception System in the Horror Project, enabling threats to see, hear, and sense the player.

## Prerequisites
- AIModule enabled in project
- AIController configured
- Perception Component available

## Core Components

### 1. AI Perception Component
The main component that handles all perception senses.

```cpp
// In AHorrorThreatAIController.h
#include "Perception/AIPerceptionComponent.h"

UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
UAIPerceptionComponent* PerceptionComponent;
```

### 2. Sense Configurations
Individual configurations for each sense type:
- Sight (UAISenseConfig_Sight)
- Hearing (UAISenseConfig_Hearing)
- Damage (UAISenseConfig_Damage)
- Touch (UAISenseConfig_Touch)
- Team (UAISenseConfig_Team)

## Basic Setup

### Step 1: Create Perception Component

```cpp
// In AHorrorThreatAIController constructor
AHorrorThreatAIController::AHorrorThreatAIController()
{
    // Create perception component
    PerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(
        TEXT("PerceptionComponent")
    );
    
    // Set as dominant sense component
    SetPerceptionComponent(*PerceptionComponent);
}
```

### Step 2: Configure Senses

```cpp
void AHorrorThreatAIController::SetupPerception()
{
    // Sight configuration
    UAISenseConfig_Sight* SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(
        TEXT("SightConfig")
    );
    ConfigureSightSense(SightConfig);
    PerceptionComponent->ConfigureSense(*SightConfig);
    
    // Hearing configuration
    UAISenseConfig_Hearing* HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(
        TEXT("HearingConfig")
    );
    ConfigureHearingSense(HearingConfig);
    PerceptionComponent->ConfigureSense(*HearingConfig);
    
    // Damage configuration
    UAISenseConfig_Damage* DamageConfig = CreateDefaultSubobject<UAISenseConfig_Damage>(
        TEXT("DamageConfig")
    );
    ConfigureDamageSense(DamageConfig);
    PerceptionComponent->ConfigureSense(*DamageConfig);
    
    // Set dominant sense
    PerceptionComponent->SetDominantSense(SightConfig->GetSenseImplementation());
}
```

### Step 3: Bind Perception Events

```cpp
void AHorrorThreatAIController::BeginPlay()
{
    Super::BeginPlay();
    
    // Bind perception update event
    if (PerceptionComponent)
    {
        PerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(
            this, 
            &AHorrorThreatAIController::OnPerceptionUpdated
        );
        
        PerceptionComponent->OnPerceptionUpdated.AddDynamic(
            this,
            &AHorrorThreatAIController::OnPerceptionInfoUpdated
        );
    }
}
```

## Perception Event Handlers

### OnTargetPerceptionUpdated
Called when a specific actor's perception status changes.

```cpp
UFUNCTION()
void OnPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);
```

**Implementation:**
```cpp
void AHorrorThreatAIController::OnPerceptionUpdated(
    AActor* Actor, 
    FAIStimulus Stimulus)
{
    if (!Actor || !Blackboard) return;
    
    // Get sense type
    TSubclassOf<UAISense> SenseClass = UAIPerceptionSystem::GetSenseClassForStimulus(
        GetWorld(), 
        Stimulus
    );
    
    // Handle sight
    if (SenseClass == UAISense_Sight::StaticClass())
    {
        HandleSightStimulus(Actor, Stimulus);
    }
    // Handle hearing
    else if (SenseClass == UAISense_Hearing::StaticClass())
    {
        HandleHearingStimulus(Actor, Stimulus);
    }
    // Handle damage
    else if (SenseClass == UAISense_Damage::StaticClass())
    {
        HandleDamageStimulus(Actor, Stimulus);
    }
}
```

### OnPerceptionInfoUpdated
Called when any perception information updates.

```cpp
UFUNCTION()
void OnPerceptionInfoUpdated(const TArray<AActor*>& UpdatedActors);
```

**Implementation:**
```cpp
void AHorrorThreatAIController::OnPerceptionInfoUpdated(
    const TArray<AActor*>& UpdatedActors)
{
    // Update blackboard with all perceived actors
    for (AActor* Actor : UpdatedActors)
    {
        FActorPerceptionBlueprintInfo Info;
        PerceptionComponent->GetActorsPerception(Actor, Info);
        
        // Process perception info
        ProcessPerceptionInfo(Actor, Info);
    }
}
```

## Stimulus Handlers

### Sight Stimulus Handler
```cpp
void AHorrorThreatAIController::HandleSightStimulus(
    AActor* Actor, 
    FAIStimulus Stimulus)
{
    if (Stimulus.WasSuccessfullySensed())
    {
        // Target spotted
        Blackboard->SetValueAsObject("TargetActor", Actor);
        Blackboard->SetValueAsBool("bCanSeeTarget", true);
        Blackboard->SetValueAsVector("LastKnownLocation", Actor->GetActorLocation());
        
        // Update alert state
        EAlertState CurrentState = static_cast<EAlertState>(
            Blackboard->GetValueAsEnum("AlertState")
        );
        
        if (CurrentState < EAlertState::Alert)
        {
            Blackboard->SetValueAsEnum("AlertState", static_cast<uint8>(EAlertState::Alert));
        }
        
        // Log for debugging
        UE_LOG(LogTemp, Log, TEXT("Sight: Spotted %s at %s"), 
            *Actor->GetName(), 
            *Actor->GetActorLocation().ToString()
        );
    }
    else
    {
        // Lost sight
        Blackboard->SetValueAsBool("bCanSeeTarget", false);
        
        UE_LOG(LogTemp, Log, TEXT("Sight: Lost sight of %s"), *Actor->GetName());
    }
}
```

### Hearing Stimulus Handler
```cpp
void AHorrorThreatAIController::HandleHearingStimulus(
    AActor* Actor, 
    FAIStimulus Stimulus)
{
    if (Stimulus.WasSuccessfullySensed())
    {
        // Heard something
        Blackboard->SetValueAsVector("NoiseLocation", Stimulus.StimulusLocation);
        Blackboard->SetValueAsVector("InvestigateLocation", Stimulus.StimulusLocation);
        
        // Update alert state if not already alerted
        EAlertState CurrentState = static_cast<EAlertState>(
            Blackboard->GetValueAsEnum("AlertState")
        );
        
        if (CurrentState == EAlertState::Idle)
        {
            Blackboard->SetValueAsEnum(
                "AlertState", 
                static_cast<uint8>(EAlertState::Suspicious)
            );
        }
        
        // Calculate noise strength
        float Distance = FVector::Dist(
            GetPawn()->GetActorLocation(), 
            Stimulus.StimulusLocation
        );
        float NoiseStrength = 1.0f - (Distance / 1000.0f); // Normalize to 1000cm
        
        UE_LOG(LogTemp, Log, TEXT("Hearing: Heard noise at %s (strength: %.2f)"), 
            *Stimulus.StimulusLocation.ToString(), 
            NoiseStrength
        );
    }
}
```

### Damage Stimulus Handler
```cpp
void AHorrorThreatAIController::HandleDamageStimulus(
    AActor* Actor, 
    FAIStimulus Stimulus)
{
    if (Stimulus.WasSuccessfullySensed())
    {
        // Took damage - immediately alert
        Blackboard->SetValueAsObject("TargetActor", Actor);
        Blackboard->SetValueAsVector("LastKnownLocation", Actor->GetActorLocation());
        Blackboard->SetValueAsEnum("AlertState", static_cast<uint8>(EAlertState::Combat));
        
        UE_LOG(LogTemp, Warning, TEXT("Damage: Took damage from %s"), *Actor->GetName());
    }
}
```

## Perception Configuration

### Sight Configuration
See [SightPerception.md](SightPerception.md) for detailed configuration.

**Quick Setup:**
```cpp
SightConfig->SightRadius = 2000.0f;              // 20m sight range
SightConfig->LoseSightRadius = 2500.0f;          // 25m lose sight range
SightConfig->PeripheralVisionAngleDegrees = 90.0f; // 90° FOV
SightConfig->DetectionByAffiliation.bDetectEnemies = true;
SightConfig->DetectionByAffiliation.bDetectNeutrals = false;
SightConfig->DetectionByAffiliation.bDetectFriendlies = false;
```

### Hearing Configuration
See [HearingPerception.md](HearingPerception.md) for detailed configuration.

**Quick Setup:**
```cpp
HearingConfig->HearingRange = 1000.0f;           // 10m hearing range
HearingConfig->DetectionByAffiliation.bDetectEnemies = true;
HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
HearingConfig->DetectionByAffiliation.bDetectFriendlies = false;
```

### Damage Configuration
See [DamagePerception.md](DamagePerception.md) for detailed configuration.

**Quick Setup:**
```cpp
DamageConfig->Implementation = UAISense_Damage::StaticClass();
```

## Blackboard Integration

### Required Blackboard Keys
```
TargetActor          (Object: Actor)
bCanSeeTarget        (Bool)
LastKnownLocation    (Vector)
NoiseLocation        (Vector)
InvestigateLocation  (Vector)
AlertState           (Enum: EAlertState)
```

### Update Blackboard from Perception
```cpp
void AHorrorThreatAIController::UpdateBlackboardFromPerception()
{
    if (!PerceptionComponent || !Blackboard) return;
    
    TArray<AActor*> PerceivedActors;
    PerceptionComponent->GetCurrentlyPerceivedActors(
        UAISense_Sight::StaticClass(), 
        PerceivedActors
    );
    
    if (PerceivedActors.Num() > 0)
    {
        // Update with closest perceived actor
        AActor* ClosestActor = GetClosestActor(PerceivedActors);
        Blackboard->SetValueAsObject("TargetActor", ClosestActor);
        Blackboard->SetValueAsBool("bCanSeeTarget", true);
    }
    else
    {
        Blackboard->SetValueAsBool("bCanSeeTarget", false);
    }
}
```

## Generating Stimuli

### Making Noise
```cpp
// In player character or any actor
void AHorrorPlayerCharacter::MakeNoise(float Loudness, FVector NoiseLocation)
{
    // Report noise event
    UAISense_Hearing::ReportNoiseEvent(
        GetWorld(),
        NoiseLocation,
        Loudness,           // 0.0 - 1.0
        this,               // Instigator
        0.0f,               // Max range (0 = use config)
        FName("Footstep")   // Tag
    );
}
```

### Reporting Damage
```cpp
// Automatically reported when TakeDamage is called
// Or manually:
UAISense_Damage::ReportDamageEvent(
    GetWorld(),
    DamagedActor,
    Instigator,
    DamageAmount,
    DamageLocation,
    HitLocation
);
```

## Debug Visualization

### Enable Perception Debug
```cpp
// Console commands
showdebug ai
ai.debug [AIName]

// Or in code
#if ENABLE_DRAW_DEBUG
if (PerceptionComponent)
{
    PerceptionComponent->DrawDebugInfo(*GEngine->GetWorldContextFromWorld(GetWorld()));
}
#endif
```

### Custom Debug Drawing
```cpp
void AHorrorThreatAIController::DrawPerceptionDebug()
{
    if (!PerceptionComponent) return;
    
    APawn* ControlledPawn = GetPawn();
    if (!ControlledPawn) return;
    
    FVector PawnLocation = ControlledPawn->GetActorLocation();
    
    // Draw sight range
    DrawDebugSphere(
        GetWorld(),
        PawnLocation,
        2000.0f,  // Sight radius
        32,
        FColor::Blue,
        false,
        -1.0f,
        0,
        2.0f
    );
    
    // Draw hearing range
    DrawDebugSphere(
        GetWorld(),
        PawnLocation,
        1000.0f,  // Hearing radius
        32,
        FColor::Yellow,
        false,
        -1.0f,
        0,
        1.0f
    );
    
    // Draw sight cone
    FVector ForwardVector = ControlledPawn->GetActorForwardVector();
    float SightAngle = 90.0f;
    
    DrawDebugCone(
        GetWorld(),
        PawnLocation,
        ForwardVector,
        2000.0f,  // Length
        FMath::DegreesToRadians(SightAngle),
        FMath::DegreesToRadians(SightAngle),
        32,
        FColor::Green,
        false,
        -1.0f,
        0,
        2.0f
    );
}
```

## Performance Optimization

### Update Frequency
```cpp
// Set perception update interval
PerceptionComponent->SetSenseUpdateInterval(
    UAISense_Sight::StaticClass(),
    0.2f  // Update every 0.2 seconds
);

PerceptionComponent->SetSenseUpdateInterval(
    UAISense_Hearing::StaticClass(),
    0.5f  // Update every 0.5 seconds
);
```

### LOD System
```cpp
void AHorrorThreatAIController::UpdatePerceptionLOD(float DistanceToPlayer)
{
    if (DistanceToPlayer > 5000.0f)
    {
        // Far - reduce update rate
        PerceptionComponent->SetSenseUpdateInterval(UAISense_Sight::StaticClass(), 1.0f);
        PerceptionComponent->SetSenseUpdateInterval(UAISense_Hearing::StaticClass(), 2.0f);
    }
    else if (DistanceToPlayer > 2000.0f)
    {
        // Medium - normal update rate
        PerceptionComponent->SetSenseUpdateInterval(UAISense_Sight::StaticClass(), 0.5f);
        PerceptionComponent->SetSenseUpdateInterval(UAISense_Hearing::StaticClass(), 1.0f);
    }
    else
    {
        // Close - high update rate
        PerceptionComponent->SetSenseUpdateInterval(UAISense_Sight::StaticClass(), 0.1f);
        PerceptionComponent->SetSenseUpdateInterval(UAISense_Hearing::StaticClass(), 0.2f);
    }
}
```

## Common Issues

### Perception Not Working
1. Check AIPerceptionSystem is enabled
2. Verify sense configurations are valid
3. Ensure target has AIPerceptionStimuliSource component
4. Check team affiliations match

### Target Not Detected
1. Verify target is within range
2. Check line of sight is clear
3. Ensure target is generating stimuli
4. Verify detection affiliation settings

### Performance Issues
1. Reduce update frequency
2. Implement LOD system
3. Limit number of AI with perception
4. Use simpler collision for line traces

## Testing Checklist

- [ ] Sight detection works
- [ ] Hearing detection works
- [ ] Damage detection works
- [ ] Blackboard updates correctly
- [ ] Alert states transition properly
- [ ] Debug visualization displays
- [ ] Performance within budget
- [ ] LOD system functions
- [ ] Multiple AI don't conflict
- [ ] Edge cases handled

## See Also
- [SightPerception.md](SightPerception.md) - Sight configuration
- [HearingPerception.md](HearingPerception.md) - Hearing configuration
- [DamagePerception.md](DamagePerception.md) - Damage configuration
- [BehaviorTreeSetup.md](../BehaviorTrees/BehaviorTreeSetup.md) - Behavior tree integration
