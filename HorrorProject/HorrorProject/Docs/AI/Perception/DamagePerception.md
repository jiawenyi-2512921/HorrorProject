# Damage Perception Configuration

## Overview
Configuration guide for AI damage perception, enabling threats to detect and respond to damage events.

## Damage Configuration

### Basic Setup

```cpp
void ConfigureDamageSense(UAISenseConfig_Damage* DamageConfig)
{
    DamageConfig->Implementation = UAISense_Damage::StaticClass();
    
    // Damage is always detected regardless of affiliation
    // No additional configuration needed
}
```

### Integration with AIController

```cpp
// In AHorrorThreatAIController constructor
void AHorrorThreatAIController::SetupPerception()
{
    // ... other sense configs ...
    
    // Damage configuration
    UAISenseConfig_Damage* DamageConfig = CreateDefaultSubobject<UAISenseConfig_Damage>(
        TEXT("DamageConfig")
    );
    DamageConfig->Implementation = UAISense_Damage::StaticClass();
    PerceptionComponent->ConfigureSense(*DamageConfig);
}
```

## Damage Reporting

### Automatic Damage Reporting

Damage is automatically reported when `TakeDamage` is called on an actor.

```cpp
// In AHorrorThreatCharacter
float AHorrorThreatCharacter::TakeDamage(
    float DamageAmount,
    FDamageEvent const& DamageEvent,
    AController* EventInstigator,
    AActor* DamageCauser)
{
    float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
    
    // Damage perception is automatically reported by the engine
    // But we can add custom logic here
    
    if (ActualDamage > 0.0f)
    {
        OnDamageTaken(ActualDamage, DamageCauser, EventInstigator);
    }
    
    return ActualDamage;
}
```

### Manual Damage Reporting

```cpp
void ReportDamageToAI(
    AActor* DamagedActor,
    AActor* Instigator,
    float DamageAmount,
    FVector DamageLocation,
    FVector HitLocation)
{
    UAISense_Damage::ReportDamageEvent(
        DamagedActor->GetWorld(),
        DamagedActor,
        Instigator,
        DamageAmount,
        DamageLocation,
        HitLocation
    );
}
```

## Damage Stimulus Handling

### Basic Damage Handler

```cpp
void AHorrorThreatAIController::HandleDamageStimulus(AActor* Actor, FAIStimulus Stimulus)
{
    if (!Stimulus.WasSuccessfullySensed()) return;
    
    // Immediately alert to combat state
    if (Blackboard)
    {
        Blackboard->SetValueAsObject("TargetActor", Actor);
        Blackboard->SetValueAsVector("LastKnownLocation", Actor ? Actor->GetActorLocation() : Stimulus.StimulusLocation);
        Blackboard->SetValueAsEnum("AlertState", static_cast<uint8>(EAlertState::Combat));
        Blackboard->SetValueAsBool("bTookDamage", true);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Damage: Took damage from %s at %s"),
        Actor ? *Actor->GetName() : TEXT("Unknown"),
        *Stimulus.StimulusLocation.ToString()
    );
}
```

### Advanced Damage Handler with Damage Types

```cpp
void AHorrorThreatAIController::HandleDamageStimulus(AActor* Actor, FAIStimulus Stimulus)
{
    if (!Stimulus.WasSuccessfullySensed()) return;
    
    // Get damage amount from stimulus strength
    float DamageAmount = Stimulus.Strength;
    
    // Classify damage severity
    EDamageSeverity Severity = ClassifyDamageSeverity(DamageAmount);
    
    // Update blackboard
    if (Blackboard)
    {
        Blackboard->SetValueAsObject("TargetActor", Actor);
        Blackboard->SetValueAsVector("LastKnownLocation", Actor ? Actor->GetActorLocation() : Stimulus.StimulusLocation);
        Blackboard->SetValueAsVector("DamageSourceLocation", Stimulus.StimulusLocation);
        Blackboard->SetValueAsFloat("LastDamageAmount", DamageAmount);
        Blackboard->SetValueAsEnum("DamageSeverity", static_cast<uint8>(Severity));
        
        // Escalate alert state
        Blackboard->SetValueAsEnum("AlertState", static_cast<uint8>(EAlertState::Combat));
        Blackboard->SetValueAsBool("bTookDamage", true);
    }
    
    // Trigger damage response behavior
    OnDamageReceived(Actor, DamageAmount, Severity, Stimulus.StimulusLocation);
    
    UE_LOG(LogTemp, Warning, TEXT("Damage: Took %.1f damage from %s (severity: %d)"),
        DamageAmount,
        Actor ? *Actor->GetName() : TEXT("Unknown"),
        static_cast<int32>(Severity)
    );
}

EDamageSeverity AHorrorThreatAIController::ClassifyDamageSeverity(float DamageAmount) const
{
    APawn* ControlledPawn = GetPawn();
    if (!ControlledPawn) return EDamageSeverity::Minor;
    
    // Get max health
    float MaxHealth = 100.0f;  // Default
    if (UHealthComponent* HealthComp = ControlledPawn->FindComponentByClass<UHealthComponent>())
    {
        MaxHealth = HealthComp->GetMaxHealth();
    }
    
    float DamagePercent = DamageAmount / MaxHealth;
    
    if (DamagePercent >= 0.5f)
        return EDamageSeverity::Critical;
    else if (DamagePercent >= 0.25f)
        return EDamageSeverity::Major;
    else if (DamagePercent >= 0.1f)
        return EDamageSeverity::Moderate;
    else
        return EDamageSeverity::Minor;
}
```

## Damage Response Behaviors

### Immediate Combat Response

```cpp
void AHorrorThreatAIController::OnDamageReceived(
    AActor* Attacker,
    float DamageAmount,
    EDamageSeverity Severity,
    FVector DamageLocation)
{
    // Stop current behavior
    StopMovement();
    
    // Face attacker
    if (Attacker)
    {
        APawn* ControlledPawn = GetPawn();
        if (ControlledPawn)
        {
            FVector Direction = Attacker->GetActorLocation() - ControlledPawn->GetActorLocation();
            FRotator TargetRotation = Direction.Rotation();
            ControlledPawn->SetActorRotation(TargetRotation);
        }
    }
    
    // Trigger combat behavior based on severity
    switch (Severity)
    {
        case EDamageSeverity::Critical:
            TriggerCriticalDamageResponse(Attacker);
            break;
        case EDamageSeverity::Major:
            TriggerMajorDamageResponse(Attacker);
            break;
        case EDamageSeverity::Moderate:
            TriggerModerateDamageResponse(Attacker);
            break;
        case EDamageSeverity::Minor:
            TriggerMinorDamageResponse(Attacker);
            break;
    }
}

void AHorrorThreatAIController::TriggerCriticalDamageResponse(AActor* Attacker)
{
    // Critical damage - aggressive response or flee
    if (ShouldFleeFromDamage())
    {
        InitiateFlee(Attacker);
    }
    else
    {
        InitiateAggressiveAttack(Attacker);
    }
}

void AHorrorThreatAIController::TriggerMajorDamageResponse(AActor* Attacker)
{
    // Major damage - immediate attack
    InitiateAttack(Attacker);
}

void AHorrorThreatAIController::TriggerModerateDamageResponse(AActor* Attacker)
{
    // Moderate damage - pursue and attack
    InitiateChase(Attacker);
}

void AHorrorThreatAIController::TriggerMinorDamageResponse(AActor* Attacker)
{
    // Minor damage - investigate and pursue
    InitiateInvestigation(Attacker->GetActorLocation());
}
```

### Flee Behavior

```cpp
bool AHorrorThreatAIController::ShouldFleeFromDamage() const
{
    APawn* ControlledPawn = GetPawn();
    if (!ControlledPawn) return false;
    
    // Check health percentage
    if (UHealthComponent* HealthComp = ControlledPawn->FindComponentByClass<UHealthComponent>())
    {
        float HealthPercent = HealthComp->GetHealthPercent();
        
        // Flee if health below 25%
        if (HealthPercent < 0.25f)
        {
            return true;
        }
    }
    
    // Check if outnumbered
    int32 NearbyEnemies = CountNearbyEnemies(1000.0f);
    if (NearbyEnemies > 2)
    {
        return true;
    }
    
    return false;
}

void AHorrorThreatAIController::InitiateFlee(AActor* ThreatSource)
{
    if (!Blackboard) return;
    
    // Calculate flee location
    FVector FleeLocation = CalculateFleeLocation(ThreatSource);
    
    Blackboard->SetValueAsVector("FleeLocation", FleeLocation);
    Blackboard->SetValueAsBool("bIsFleeing", true);
    
    UE_LOG(LogTemp, Log, TEXT("Fleeing from %s to %s"),
        ThreatSource ? *ThreatSource->GetName() : TEXT("Unknown"),
        *FleeLocation.ToString()
    );
}

FVector AHorrorThreatAIController::CalculateFleeLocation(AActor* ThreatSource) const
{
    APawn* ControlledPawn = GetPawn();
    if (!ControlledPawn) return FVector::ZeroVector;
    
    FVector CurrentLocation = ControlledPawn->GetActorLocation();
    FVector ThreatLocation = ThreatSource ? ThreatSource->GetActorLocation() : CurrentLocation;
    
    // Flee in opposite direction
    FVector FleeDirection = (CurrentLocation - ThreatLocation).GetSafeNormal();
    
    // Find safe location 2000cm away
    FVector FleeLocation = CurrentLocation + FleeDirection * 2000.0f;
    
    // Project onto navmesh
    FNavLocation NavLocation;
    UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
    if (NavSys && NavSys->ProjectPointToNavigation(FleeLocation, NavLocation))
    {
        return NavLocation.Location;
    }
    
    return FleeLocation;
}
```

### Call for Help

```cpp
void AHorrorThreatAIController::CallForHelp(AActor* Attacker, float Radius)
{
    TArray<AActor*> NearbyAllies;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AHorrorThreatCharacter::StaticClass(), NearbyAllies);
    
    for (AActor* Ally : NearbyAllies)
    {
        if (Ally == GetPawn()) continue;
        
        float Distance = FVector::Dist(GetPawn()->GetActorLocation(), Ally->GetActorLocation());
        if (Distance <= Radius)
        {
            // Alert ally
            if (AHorrorThreatCharacter* AllyThreat = Cast<AHorrorThreatCharacter>(Ally))
            {
                if (AHorrorThreatAIController* AllyController = Cast<AHorrorThreatAIController>(AllyThreat->GetController()))
                {
                    AllyController->OnAllyCalledForHelp(GetPawn(), Attacker);
                }
            }
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Called for help against %s"),
        Attacker ? *Attacker->GetName() : TEXT("Unknown")
    );
}

void AHorrorThreatAIController::OnAllyCalledForHelp(AActor* Ally, AActor* Attacker)
{
    if (!Blackboard) return;
    
    // Set target to attacker
    Blackboard->SetValueAsObject("TargetActor", Attacker);
    Blackboard->SetValueAsVector("LastKnownLocation", Attacker ? Attacker->GetActorLocation() : FVector::ZeroVector);
    Blackboard->SetValueAsEnum("AlertState", static_cast<uint8>(EAlertState::Combat));
    
    UE_LOG(LogTemp, Log, TEXT("Responding to help call from %s"),
        Ally ? *Ally->GetName() : TEXT("Unknown")
    );
}
```

## Damage Memory System

### Track Damage History

```cpp
struct FDamageMemory
{
    AActor* Attacker;
    float DamageAmount;
    FVector DamageLocation;
    float Timestamp;
    EDamageSeverity Severity;
};

class AHorrorThreatAIController : public AAIController
{
private:
    TArray<FDamageMemory> DamageHistory;
    float DamageMemoryDuration = 30.0f;  // Remember damage for 30 seconds
    
public:
    void AddDamageToHistory(AActor* Attacker, float DamageAmount, FVector Location, EDamageSeverity Severity)
    {
        FDamageMemory NewDamage;
        NewDamage.Attacker = Attacker;
        NewDamage.DamageAmount = DamageAmount;
        NewDamage.DamageLocation = Location;
        NewDamage.Timestamp = GetWorld()->GetTimeSeconds();
        NewDamage.Severity = Severity;
        
        DamageHistory.Add(NewDamage);
        
        CleanOldDamageMemories();
    }
    
    void CleanOldDamageMemories()
    {
        float CurrentTime = GetWorld()->GetTimeSeconds();
        
        DamageHistory.RemoveAll([CurrentTime, this](const FDamageMemory& Damage)
        {
            return (CurrentTime - Damage.Timestamp) > DamageMemoryDuration;
        });
    }
    
    float GetTotalDamageFromAttacker(AActor* Attacker) const
    {
        float TotalDamage = 0.0f;
        
        for (const FDamageMemory& Damage : DamageHistory)
        {
            if (Damage.Attacker == Attacker)
            {
                TotalDamage += Damage.DamageAmount;
            }
        }
        
        return TotalDamage;
    }
    
    AActor* GetMostDangerousAttacker() const
    {
        TMap<AActor*, float> AttackerDamage;
        
        for (const FDamageMemory& Damage : DamageHistory)
        {
            if (Damage.Attacker)
            {
                AttackerDamage.FindOrAdd(Damage.Attacker) += Damage.DamageAmount;
            }
        }
        
        AActor* MostDangerous = nullptr;
        float MaxDamage = 0.0f;
        
        for (const auto& Pair : AttackerDamage)
        {
            if (Pair.Value > MaxDamage)
            {
                MaxDamage = Pair.Value;
                MostDangerous = Pair.Key;
            }
        }
        
        return MostDangerous;
    }
    
    int32 GetDamageEventCount(float TimeWindow) const
    {
        float CurrentTime = GetWorld()->GetTimeSeconds();
        int32 Count = 0;
        
        for (const FDamageMemory& Damage : DamageHistory)
        {
            if ((CurrentTime - Damage.Timestamp) <= TimeWindow)
            {
                Count++;
            }
        }
        
        return Count;
    }
};
```

## Damage Types and Responses

### Damage Type Classification

```cpp
UENUM(BlueprintType)
enum class EDamageType : uint8
{
    Melee,
    Ranged,
    Explosive,
    Fire,
    Poison,
    Fall,
    Environmental
};

EDamageType AHorrorThreatAIController::ClassifyDamageType(const FDamageEvent& DamageEvent) const
{
    if (DamageEvent.IsOfType(FPointDamageEvent::ClassID))
    {
        return EDamageType::Ranged;
    }
    else if (DamageEvent.IsOfType(FRadialDamageEvent::ClassID))
    {
        return EDamageType::Explosive;
    }
    else
    {
        return EDamageType::Melee;
    }
}
```

### Type-Specific Responses

```cpp
void AHorrorThreatAIController::RespondToDamageType(EDamageType DamageType, AActor* Attacker)
{
    switch (DamageType)
    {
        case EDamageType::Melee:
            // Close range threat - counter attack
            InitiateCounterAttack(Attacker);
            break;
            
        case EDamageType::Ranged:
            // Distant threat - take cover or charge
            if (ShouldTakeCover())
                InitiateTakeCover(Attacker);
            else
                InitiateCharge(Attacker);
            break;
            
        case EDamageType::Explosive:
            // Area damage - flee from area
            InitiateFleeFromArea(GetPawn()->GetActorLocation());
            break;
            
        case EDamageType::Fire:
            // Fire damage - panic response
            InitiatePanicResponse();
            break;
            
        default:
            // Default aggressive response
            InitiateAttack(Attacker);
            break;
    }
}
```

## Debug Visualization

### Draw Damage Events

```cpp
void AHorrorThreatAIController::DrawDamageDebug() const
{
    APawn* ControlledPawn = GetPawn();
    if (!ControlledPawn) return;
    
    // Draw damage history
    for (const FDamageMemory& Damage : DamageHistory)
    {
        float Age = GetWorld()->GetTimeSeconds() - Damage.Timestamp;
        float Alpha = 1.0f - (Age / DamageMemoryDuration);
        
        FColor DamageColor = FColor::Red;
        DamageColor.A = static_cast<uint8>(Alpha * 255);
        
        // Draw damage location
        DrawDebugSphere(
            GetWorld(),
            Damage.DamageLocation,
            30.0f,
            12,
            DamageColor,
            false,
            -1.0f,
            0,
            2.0f
        );
        
        // Draw line from AI to damage location
        DrawDebugLine(
            GetWorld(),
            ControlledPawn->GetActorLocation(),
            Damage.DamageLocation,
            DamageColor,
            false,
            -1.0f,
            0,
            1.0f
        );
        
        // Draw damage amount text
        FString DamageText = FString::Printf(TEXT("%.1f"), Damage.DamageAmount);
        DrawDebugString(
            GetWorld(),
            Damage.DamageLocation + FVector(0, 0, 50),
            DamageText,
            nullptr,
            DamageColor,
            0.0f,
            true
        );
    }
}
```

## Performance Considerations

Damage perception has minimal performance impact as it's event-driven rather than polled.

### Optimization Tips
1. Limit damage history size
2. Clean old damage memories regularly
3. Batch damage responses
4. Use damage thresholds to ignore minor damage

## Testing

### Test Scenarios
1. Single damage event
2. Multiple rapid damage events
3. Damage from different sources
4. Critical damage response
5. Flee behavior trigger
6. Call for help
7. Damage memory persistence

### Debug Commands
```
showdebug ai
ai.debug [ThreatName]
```

## See Also
- [PerceptionSystemSetup.md](PerceptionSystemSetup.md) - Main perception setup
- [SightPerception.md](SightPerception.md) - Sight configuration
- [HearingPerception.md](HearingPerception.md) - Hearing configuration
