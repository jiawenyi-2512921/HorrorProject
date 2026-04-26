# Sight Perception Configuration

## Overview
Detailed configuration guide for AI sight perception in the Horror Project. Controls how threats detect and track the player visually.

## Sight Configuration Parameters

### Basic Parameters

```cpp
UPROPERTY(EditAnywhere, Category = "Sight")
float SightRadius = 2000.0f;  // Maximum sight distance (cm)

UPROPERTY(EditAnywhere, Category = "Sight")
float LoseSightRadius = 2500.0f;  // Distance at which sight is lost (cm)

UPROPERTY(EditAnywhere, Category = "Sight")
float PeripheralVisionAngleDegrees = 90.0f;  // Field of view angle

UPROPERTY(EditAnywhere, Category = "Sight")
FAISenseAffiliationFilter DetectionByAffiliation;  // What to detect

UPROPERTY(EditAnywhere, Category = "Sight")
float AutoSuccessRangeFromLastSeenLocation = 500.0f;  // Auto-detect range
```

### Advanced Parameters

```cpp
UPROPERTY(EditAnywhere, Category = "Sight")
float PointOfViewBackwardOffset = 0.0f;  // POV offset from actor center

UPROPERTY(EditAnywhere, Category = "Sight")
float NearClippingRadius = 0.0f;  // Minimum detection distance

UPROPERTY(EditAnywhere, Category = "Sight")
float MaxAge = 5.0f;  // How long to remember last seen location
```

## Configuration Presets

### Golem Configuration
High-range, narrow FOV for focused stalking behavior.

```cpp
void ConfigureGolemSight(UAISenseConfig_Sight* SightConfig)
{
    SightConfig->SightRadius = 3000.0f;              // 30m sight range
    SightConfig->LoseSightRadius = 3500.0f;          // 35m lose sight
    SightConfig->PeripheralVisionAngleDegrees = 60.0f; // Narrow 60° FOV
    SightConfig->AutoSuccessRangeFromLastSeenLocation = 800.0f;
    SightConfig->PointOfViewBackwardOffset = 0.0f;
    SightConfig->NearClippingRadius = 100.0f;        // Don't detect too close
    SightConfig->MaxAge = 10.0f;                     // Remember for 10s
    
    // Detection settings
    SightConfig->DetectionByAffiliation.bDetectEnemies = true;
    SightConfig->DetectionByAffiliation.bDetectNeutrals = false;
    SightConfig->DetectionByAffiliation.bDetectFriendlies = false;
    
    // Set implementation
    SightConfig->Implementation = UAISense_Sight::StaticClass();
}
```

### Shadow Configuration
Medium-range, wide FOV for ambient threat.

```cpp
void ConfigureShadowSight(UAISenseConfig_Sight* SightConfig)
{
    SightConfig->SightRadius = 1500.0f;              // 15m sight range
    SightConfig->LoseSightRadius = 2000.0f;          // 20m lose sight
    SightConfig->PeripheralVisionAngleDegrees = 120.0f; // Wide 120° FOV
    SightConfig->AutoSuccessRangeFromLastSeenLocation = 500.0f;
    SightConfig->PointOfViewBackwardOffset = 0.0f;
    SightConfig->NearClippingRadius = 50.0f;
    SightConfig->MaxAge = 5.0f;                      // Remember for 5s
    
    SightConfig->DetectionByAffiliation.bDetectEnemies = true;
    SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
    SightConfig->DetectionByAffiliation.bDetectFriendlies = false;
    
    SightConfig->Implementation = UAISense_Sight::StaticClass();
}
```

### Hunter Configuration
Long-range, medium FOV for tracking behavior.

```cpp
void ConfigureHunterSight(UAISenseConfig_Sight* SightConfig)
{
    SightConfig->SightRadius = 4000.0f;              // 40m sight range
    SightConfig->LoseSightRadius = 4500.0f;          // 45m lose sight
    SightConfig->PeripheralVisionAngleDegrees = 75.0f; // Medium 75° FOV
    SightConfig->AutoSuccessRangeFromLastSeenLocation = 1000.0f;
    SightConfig->PointOfViewBackwardOffset = 0.0f;
    SightConfig->NearClippingRadius = 200.0f;
    SightConfig->MaxAge = 15.0f;                     // Remember for 15s
    
    SightConfig->DetectionByAffiliation.bDetectEnemies = true;
    SightConfig->DetectionByAffiliation.bDetectNeutrals = false;
    SightConfig->DetectionByAffiliation.bDetectFriendlies = false;
    
    SightConfig->Implementation = UAISense_Sight::StaticClass();
}
```

### Patrol Guard Configuration
Short-range, wide FOV for area coverage.

```cpp
void ConfigurePatrolGuardSight(UAISenseConfig_Sight* SightConfig)
{
    SightConfig->SightRadius = 1000.0f;              // 10m sight range
    SightConfig->LoseSightRadius = 1200.0f;          // 12m lose sight
    SightConfig->PeripheralVisionAngleDegrees = 180.0f; // Very wide 180° FOV
    SightConfig->AutoSuccessRangeFromLastSeenLocation = 300.0f;
    SightConfig->PointOfViewBackwardOffset = 0.0f;
    SightConfig->NearClippingRadius = 0.0f;
    SightConfig->MaxAge = 3.0f;                      // Remember for 3s
    
    SightConfig->DetectionByAffiliation.bDetectEnemies = true;
    SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
    SightConfig->DetectionByAffiliation.bDetectFriendlies = false;
    
    SightConfig->Implementation = UAISense_Sight::StaticClass();
}
```

## Line of Sight System

### Basic Line Trace
```cpp
bool AHorrorThreatAIController::HasLineOfSightTo(AActor* Target) const
{
    if (!Target) return false;
    
    APawn* ControlledPawn = GetPawn();
    if (!ControlledPawn) return false;
    
    FVector StartLocation = ControlledPawn->GetActorLocation() + FVector(0, 0, 100); // Eye height
    FVector EndLocation = Target->GetActorLocation() + FVector(0, 0, 100);
    
    FHitResult HitResult;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(ControlledPawn);
    Params.AddIgnoredActor(Target);
    
    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        StartLocation,
        EndLocation,
        ECC_Visibility,
        Params
    );
    
    return !bHit || HitResult.GetActor() == Target;
}
```

### Advanced Line Trace with Partial Cover
```cpp
bool AHorrorThreatAIController::CanSeeTarget(AActor* Target, float& VisibilityPercentage) const
{
    if (!Target) return false;
    
    APawn* ControlledPawn = GetPawn();
    if (!ControlledPawn) return false;
    
    // Multiple trace points on target
    TArray<FVector> TracePoints;
    FVector TargetLocation = Target->GetActorLocation();
    
    // Head, chest, legs
    TracePoints.Add(TargetLocation + FVector(0, 0, 180));  // Head
    TracePoints.Add(TargetLocation + FVector(0, 0, 100));  // Chest
    TracePoints.Add(TargetLocation + FVector(0, 0, 50));   // Waist
    TracePoints.Add(TargetLocation + FVector(0, 0, 20));   // Legs
    
    FVector EyeLocation = ControlledPawn->GetActorLocation() + FVector(0, 0, 100);
    
    int32 VisiblePoints = 0;
    for (const FVector& TracePoint : TracePoints)
    {
        FHitResult HitResult;
        FCollisionQueryParams Params;
        Params.AddIgnoredActor(ControlledPawn);
        
        bool bHit = GetWorld()->LineTraceSingleByChannel(
            HitResult,
            EyeLocation,
            TracePoint,
            ECC_Visibility,
            Params
        );
        
        if (!bHit || HitResult.GetActor() == Target)
        {
            VisiblePoints++;
        }
    }
    
    VisibilityPercentage = static_cast<float>(VisiblePoints) / TracePoints.Num();
    return VisibilityPercentage > 0.25f;  // At least 25% visible
}
```

## Field of View Calculations

### Check if Target in FOV
```cpp
bool AHorrorThreatAIController::IsInFieldOfView(AActor* Target, float FOVAngle) const
{
    if (!Target) return false;
    
    APawn* ControlledPawn = GetPawn();
    if (!ControlledPawn) return false;
    
    FVector ToTarget = Target->GetActorLocation() - ControlledPawn->GetActorLocation();
    ToTarget.Normalize();
    
    FVector Forward = ControlledPawn->GetActorForwardVector();
    
    float DotProduct = FVector::DotProduct(Forward, ToTarget);
    float Angle = FMath::Acos(DotProduct);
    float AngleDegrees = FMath::RadiansToDegrees(Angle);
    
    return AngleDegrees <= (FOVAngle * 0.5f);
}
```

### Get Angle to Target
```cpp
float AHorrorThreatAIController::GetAngleToTarget(AActor* Target) const
{
    if (!Target) return 0.0f;
    
    APawn* ControlledPawn = GetPawn();
    if (!ControlledPawn) return 0.0f;
    
    FVector ToTarget = Target->GetActorLocation() - ControlledPawn->GetActorLocation();
    ToTarget.Z = 0;  // Ignore vertical component
    ToTarget.Normalize();
    
    FVector Forward = ControlledPawn->GetActorForwardVector();
    Forward.Z = 0;
    Forward.Normalize();
    
    float DotProduct = FVector::DotProduct(Forward, ToTarget);
    float Angle = FMath::Acos(DotProduct);
    
    // Determine if target is on left or right
    FVector CrossProduct = FVector::CrossProduct(Forward, ToTarget);
    if (CrossProduct.Z < 0)
    {
        Angle = -Angle;
    }
    
    return FMath::RadiansToDegrees(Angle);
}
```

## Sight Modifiers

### Distance-Based Detection
```cpp
float AHorrorThreatAIController::GetSightModifier(AActor* Target) const
{
    if (!Target) return 0.0f;
    
    APawn* ControlledPawn = GetPawn();
    if (!ControlledPawn) return 0.0f;
    
    float Distance = FVector::Dist(
        ControlledPawn->GetActorLocation(), 
        Target->GetActorLocation()
    );
    
    // Base modifier from distance
    float DistanceModifier = 1.0f - (Distance / 3000.0f);  // Normalize to 30m
    DistanceModifier = FMath::Clamp(DistanceModifier, 0.0f, 1.0f);
    
    // Angle modifier (easier to see in center of vision)
    float Angle = FMath::Abs(GetAngleToTarget(Target));
    float AngleModifier = 1.0f - (Angle / 90.0f);  // Normalize to 90°
    AngleModifier = FMath::Clamp(AngleModifier, 0.3f, 1.0f);  // Min 30%
    
    // Movement modifier (moving targets easier to spot)
    float MovementModifier = 1.0f;
    if (ACharacter* TargetCharacter = Cast<ACharacter>(Target))
    {
        float Speed = TargetCharacter->GetVelocity().Size();
        if (Speed > 100.0f)  // Walking
        {
            MovementModifier = 1.2f;
        }
        if (Speed > 400.0f)  // Running
        {
            MovementModifier = 1.5f;
        }
    }
    
    return DistanceModifier * AngleModifier * MovementModifier;
}
```

### Light Level Modifier
```cpp
float AHorrorThreatAIController::GetLightLevelModifier(AActor* Target) const
{
    if (!Target) return 1.0f;
    
    // Raycast to check light level at target location
    // This is a simplified version - actual implementation would query lighting system
    
    FVector TargetLocation = Target->GetActorLocation();
    
    // Check if in shadow
    FHitResult HitResult;
    FVector LightDirection = FVector(0, 0, 1);  // Assume light from above
    
    bool bInShadow = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        TargetLocation,
        TargetLocation + LightDirection * 1000.0f,
        ECC_Visibility,
        FCollisionQueryParams()
    );
    
    if (bInShadow)
    {
        return 0.5f;  // 50% detection in shadow
    }
    
    return 1.0f;  // 100% detection in light
}
```

## Sight Stimulus Source

### Setup on Player Character
```cpp
// In AHorrorPlayerCharacter.h
#include "Perception/AIPerceptionStimuliSourceComponent.h"

UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
UAIPerceptionStimuliSourceComponent* StimuliSource;

// In AHorrorPlayerCharacter.cpp
AHorrorPlayerCharacter::AHorrorPlayerCharacter()
{
    // Create stimuli source
    StimuliSource = CreateDefaultSubobject<UAIPerceptionStimuliSourceComponent>(
        TEXT("StimuliSource")
    );
    
    // Register for sight
    StimuliSource->RegisterForSense(UAISense_Sight::StaticClass());
    StimuliSource->RegisterAsSourceForSenses({UAISense_Sight::StaticClass()});
}
```

### Dynamic Visibility Control
```cpp
void AHorrorPlayerCharacter::SetVisibilityToAI(bool bVisible)
{
    if (StimuliSource)
    {
        if (bVisible)
        {
            StimuliSource->RegisterForSense(UAISense_Sight::StaticClass());
        }
        else
        {
            StimuliSource->UnregisterFromSense(UAISense_Sight::StaticClass());
        }
    }
}
```

## Debug Visualization

### Draw Sight Cone
```cpp
void AHorrorThreatAIController::DrawSightDebug() const
{
    APawn* ControlledPawn = GetPawn();
    if (!ControlledPawn) return;
    
    FVector PawnLocation = ControlledPawn->GetActorLocation();
    FVector ForwardVector = ControlledPawn->GetActorForwardVector();
    
    // Get sight config
    float SightRadius = 2000.0f;
    float FOVAngle = 90.0f;
    
    // Draw sight cone
    DrawDebugCone(
        GetWorld(),
        PawnLocation + FVector(0, 0, 100),  // Eye height
        ForwardVector,
        SightRadius,
        FMath::DegreesToRadians(FOVAngle * 0.5f),
        FMath::DegreesToRadians(FOVAngle * 0.5f),
        32,
        FColor::Green,
        false,
        -1.0f,
        0,
        2.0f
    );
    
    // Draw sight radius
    DrawDebugSphere(
        GetWorld(),
        PawnLocation,
        SightRadius,
        32,
        FColor::Blue,
        false,
        -1.0f,
        0,
        1.0f
    );
}
```

### Draw Line of Sight
```cpp
void AHorrorThreatAIController::DrawLineOfSightDebug(AActor* Target) const
{
    if (!Target) return;
    
    APawn* ControlledPawn = GetPawn();
    if (!ControlledPawn) return;
    
    FVector StartLocation = ControlledPawn->GetActorLocation() + FVector(0, 0, 100);
    FVector EndLocation = Target->GetActorLocation() + FVector(0, 0, 100);
    
    bool bHasLOS = HasLineOfSightTo(Target);
    FColor LineColor = bHasLOS ? FColor::Green : FColor::Red;
    
    DrawDebugLine(
        GetWorld(),
        StartLocation,
        EndLocation,
        LineColor,
        false,
        -1.0f,
        0,
        3.0f
    );
    
    // Draw hit point if blocked
    if (!bHasLOS)
    {
        FHitResult HitResult;
        FCollisionQueryParams Params;
        Params.AddIgnoredActor(ControlledPawn);
        
        GetWorld()->LineTraceSingleByChannel(
            HitResult,
            StartLocation,
            EndLocation,
            ECC_Visibility,
            Params
        );
        
        if (HitResult.bBlockingHit)
        {
            DrawDebugSphere(
                GetWorld(),
                HitResult.Location,
                20.0f,
                12,
                FColor::Red,
                false,
                -1.0f,
                0,
                2.0f
            );
        }
    }
}
```

## Performance Optimization

### Update Frequency by Distance
```cpp
void AHorrorThreatAIController::UpdateSightFrequency(float DistanceToPlayer)
{
    if (!PerceptionComponent) return;
    
    float UpdateInterval;
    
    if (DistanceToPlayer < 1000.0f)  // < 10m
    {
        UpdateInterval = 0.1f;  // 10 Hz
    }
    else if (DistanceToPlayer < 3000.0f)  // < 30m
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
        UAISense_Sight::StaticClass(),
        UpdateInterval
    );
}
```

### Occlusion Culling
```cpp
bool AHorrorThreatAIController::ShouldUpdateSight() const
{
    APawn* ControlledPawn = GetPawn();
    if (!ControlledPawn) return false;
    
    // Don't update if AI is not rendered
    if (!ControlledPawn->WasRecentlyRendered(0.5f))
    {
        return false;
    }
    
    // Don't update if too far from player
    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (PC && PC->GetPawn())
    {
        float Distance = FVector::Dist(
            ControlledPawn->GetActorLocation(),
            PC->GetPawn()->GetActorLocation()
        );
        
        if (Distance > 10000.0f)  // > 100m
        {
            return false;
        }
    }
    
    return true;
}
```

## Testing

### Test Scenarios
1. Target enters sight range
2. Target exits sight range
3. Target moves through FOV
4. Target behind obstacle
5. Target in peripheral vision
6. Target at maximum range
7. Multiple targets
8. Rapid movement

### Debug Commands
```
showdebug ai
ai.debug [ThreatName]
```

### Unit Tests
```cpp
// Test sight detection
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FSightDetectionTest,
    "HorrorProject.AI.Perception.SightDetection",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter
)

bool FSightDetectionTest::RunTest(const FString& Parameters)
{
    // Test implementation
    return true;
}
```

## See Also
- [PerceptionSystemSetup.md](PerceptionSystemSetup.md) - Main perception setup
- [HearingPerception.md](HearingPerception.md) - Hearing configuration
- [DamagePerception.md](DamagePerception.md) - Damage configuration
