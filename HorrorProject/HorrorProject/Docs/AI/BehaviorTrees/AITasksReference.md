# AI Tasks Reference

## Overview
Complete reference for all custom Behavior Tree tasks used in the Horror Project AI system.

## Task Categories

### Movement Tasks
- BTTask_MoveToTarget
- BTTask_MoveToLocation
- BTTask_PatrolAroundTarget
- BTTask_FleeFromTarget
- BTTask_StopMovement

### Combat Tasks
- BTTask_Attack
- BTTask_RangedAttack
- BTTask_MeleeAttack
- BTTask_TriggerFinalImpact

### Investigation Tasks
- BTTask_Investigate
- BTTask_SearchArea
- BTTask_LookAround

### State Management Tasks
- BTTask_UpdateState
- BTTask_SetAlertState
- BTTask_TransitionState

### Animation Tasks
- BTTask_PlayAnimation
- BTTask_PlayMontage
- BTTask_StopAnimation

## Movement Tasks

### BTTask_MoveToTarget

**Purpose:** Move AI towards a target actor

**Parameters:**
```cpp
UPROPERTY(EditAnywhere, Category = "Movement")
float MoveSpeed = 400.0f;

UPROPERTY(EditAnywhere, Category = "Movement")
float AcceptanceRadius = 100.0f;

UPROPERTY(EditAnywhere, Category = "Movement")
bool bUseNavMesh = true;

UPROPERTY(EditAnywhere, Category = "Movement")
bool bStopOnOverlap = true;

UPROPERTY(EditAnywhere, Category = "Movement")
bool bProjectGoalOnNavigation = true;

UPROPERTY(EditAnywhere, Category = "Blackboard")
FBlackboardKeySelector TargetActorKey;
```

**Implementation:**
```cpp
EBTNodeResult::Type UBTTask_MoveToTarget::ExecuteTask(
    UBehaviorTreeComponent& OwnerComp, 
    uint8* NodeMemory)
{
    AAIController* AIController = OwnerComp.GetAIOwner();
    if (!AIController) return EBTNodeResult::Failed;

    UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
    if (!Blackboard) return EBTNodeResult::Failed;

    AActor* TargetActor = Cast<AActor>(
        Blackboard->GetValueAsObject(TargetActorKey.SelectedKeyName)
    );
    if (!TargetActor) return EBTNodeResult::Failed;

    // Set movement speed
    APawn* ControlledPawn = AIController->GetPawn();
    if (ACharacter* Character = Cast<ACharacter>(ControlledPawn))
    {
        if (UCharacterMovementComponent* Movement = Character->GetCharacterMovement())
        {
            Movement->MaxWalkSpeed = MoveSpeed;
        }
    }

    // Move to target
    EPathFollowingRequestResult::Type Result;
    if (bUseNavMesh)
    {
        Result = AIController->MoveToActor(
            TargetActor, 
            AcceptanceRadius,
            bStopOnOverlap,
            true,
            bProjectGoalOnNavigation
        );
    }
    else
    {
        Result = AIController->MoveToLocation(
            TargetActor->GetActorLocation(),
            AcceptanceRadius,
            bStopOnOverlap
        );
    }

    return Result == EPathFollowingRequestResult::RequestSuccessful 
        ? EBTNodeResult::InProgress 
        : EBTNodeResult::Failed;
}
```

**Usage:**
- Chase behavior
- Approach target
- Follow player

---

### BTTask_PatrolAroundTarget

**Purpose:** Circle around target at specified distance

**Parameters:**
```cpp
UPROPERTY(EditAnywhere, Category = "Patrol")
float PatrolSpeed = 150.0f;

UPROPERTY(EditAnywhere, Category = "Patrol")
float MinDistance = 1000.0f;

UPROPERTY(EditAnywhere, Category = "Patrol")
float MaxDistance = 1500.0f;

UPROPERTY(EditAnywhere, Category = "Patrol")
float CircleDirection = 1.0f; // 1.0 = clockwise, -1.0 = counter-clockwise

UPROPERTY(EditAnywhere, Category = "Patrol")
float PauseChance = 0.01f;

UPROPERTY(EditAnywhere, Category = "Patrol")
float PauseDuration = 2.0f;

UPROPERTY(EditAnywhere, Category = "Blackboard")
FBlackboardKeySelector TargetActorKey;
```

**Implementation:**
```cpp
void UBTTask_PatrolAroundTarget::TickTask(
    UBehaviorTreeComponent& OwnerComp, 
    uint8* NodeMemory, 
    float DeltaSeconds)
{
    AAIController* AIController = OwnerComp.GetAIOwner();
    APawn* ControlledPawn = AIController->GetPawn();
    AActor* TargetActor = GetTargetActor(OwnerComp);

    if (!ControlledPawn || !TargetActor)
    {
        FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
        return;
    }

    // Check for random pause
    if (FMath::FRand() < PauseChance)
    {
        // Pause logic
        return;
    }

    // Calculate circle movement
    FVector ToTarget = TargetActor->GetActorLocation() - ControlledPawn->GetActorLocation();
    float CurrentDistance = ToTarget.Size();
    float DesiredDistance = (MinDistance + MaxDistance) * 0.5f;

    // Tangent direction for circling
    FVector CircleDir = FVector::CrossProduct(
        ToTarget.GetSafeNormal(), 
        FVector::UpVector
    ) * CircleDirection;

    // Blend with radial movement to maintain distance
    FVector MoveDirection = CircleDir;
    if (CurrentDistance > DesiredDistance + 100.0f)
    {
        MoveDirection += ToTarget.GetSafeNormal() * 0.5f;
    }
    else if (CurrentDistance < DesiredDistance - 100.0f)
    {
        MoveDirection -= ToTarget.GetSafeNormal() * 0.5f;
    }

    MoveDirection.Normalize();

    // Apply movement
    if (ACharacter* Character = Cast<ACharacter>(ControlledPawn))
    {
        Character->GetCharacterMovement()->MaxWalkSpeed = PatrolSpeed;
        FVector NewLocation = ControlledPawn->GetActorLocation() + 
            MoveDirection * PatrolSpeed * DeltaSeconds;
        Character->SetActorLocation(NewLocation, true);

        // Face movement direction
        FRotator TargetRotation = MoveDirection.Rotation();
        FRotator NewRotation = FMath::RInterpTo(
            Character->GetActorRotation(), 
            TargetRotation, 
            DeltaSeconds, 
            3.0f
        );
        Character->SetActorRotation(NewRotation);
    }
}
```

**Usage:**
- Golem Close Stalking phase
- Circling behavior
- Maintaining distance

---

### BTTask_StopMovement

**Purpose:** Immediately stop all movement

**Parameters:**
```cpp
UPROPERTY(EditAnywhere, Category = "Movement")
bool bStopImmediately = true;

UPROPERTY(EditAnywhere, Category = "Movement")
bool bClearPath = true;
```

**Implementation:**
```cpp
EBTNodeResult::Type UBTTask_StopMovement::ExecuteTask(
    UBehaviorTreeComponent& OwnerComp, 
    uint8* NodeMemory)
{
    AAIController* AIController = OwnerComp.GetAIOwner();
    if (!AIController) return EBTNodeResult::Failed;

    if (bClearPath)
    {
        AIController->StopMovement();
    }

    APawn* ControlledPawn = AIController->GetPawn();
    if (ACharacter* Character = Cast<ACharacter>(ControlledPawn))
    {
        if (UCharacterMovementComponent* Movement = Character->GetCharacterMovement())
        {
            if (bStopImmediately)
            {
                Movement->StopMovementImmediately();
            }
            else
            {
                Movement->Velocity = FVector::ZeroVector;
            }
        }
    }

    return EBTNodeResult::Succeeded;
}
```

**Usage:**
- Final Impact phase
- Attack preparation
- State transitions

---

## Combat Tasks

### BTTask_Attack

**Purpose:** Execute attack on target

**Parameters:**
```cpp
UPROPERTY(EditAnywhere, Category = "Attack")
UAnimMontage* AttackMontage;

UPROPERTY(EditAnywhere, Category = "Attack")
float AttackDamage = 30.0f;

UPROPERTY(EditAnywhere, Category = "Attack")
float AttackRange = 200.0f;

UPROPERTY(EditAnywhere, Category = "Attack")
float AttackCooldown = 2.0f;

UPROPERTY(EditAnywhere, Category = "Attack")
TSubclassOf<UDamageType> DamageTypeClass;

UPROPERTY(EditAnywhere, Category = "Attack")
bool bRequireLineOfSight = true;

UPROPERTY(EditAnywhere, Category = "Blackboard")
FBlackboardKeySelector TargetActorKey;
```

**Implementation:**
```cpp
EBTNodeResult::Type UBTTask_Attack::ExecuteTask(
    UBehaviorTreeComponent& OwnerComp, 
    uint8* NodeMemory)
{
    AAIController* AIController = OwnerComp.GetAIOwner();
    APawn* ControlledPawn = AIController->GetPawn();
    AActor* TargetActor = GetTargetActor(OwnerComp);

    if (!ControlledPawn || !TargetActor) 
        return EBTNodeResult::Failed;

    // Check range
    float Distance = FVector::Dist(
        ControlledPawn->GetActorLocation(), 
        TargetActor->GetActorLocation()
    );
    if (Distance > AttackRange) 
        return EBTNodeResult::Failed;

    // Check line of sight
    if (bRequireLineOfSight)
    {
        FHitResult HitResult;
        FCollisionQueryParams Params;
        Params.AddIgnoredActor(ControlledPawn);

        bool bHit = GetWorld()->LineTraceSingleByChannel(
            HitResult,
            ControlledPawn->GetActorLocation(),
            TargetActor->GetActorLocation(),
            ECC_Visibility,
            Params
        );

        if (bHit && HitResult.GetActor() != TargetActor)
            return EBTNodeResult::Failed;
    }

    // Face target
    FVector Direction = TargetActor->GetActorLocation() - 
        ControlledPawn->GetActorLocation();
    FRotator TargetRotation = Direction.Rotation();
    ControlledPawn->SetActorRotation(TargetRotation);

    // Play attack animation
    if (AttackMontage)
    {
        if (ACharacter* Character = Cast<ACharacter>(ControlledPawn))
        {
            if (UAnimInstance* AnimInstance = Character->GetMesh()->GetAnimInstance())
            {
                AnimInstance->Montage_Play(AttackMontage);
            }
        }
    }

    // Apply damage
    FDamageEvent DamageEvent;
    DamageEvent.DamageTypeClass = DamageTypeClass;
    TargetActor->TakeDamage(
        AttackDamage, 
        DamageEvent, 
        AIController, 
        ControlledPawn
    );

    // Set cooldown
    FTaskMemory* Memory = CastInstanceNodeMemory<FTaskMemory>(NodeMemory);
    Memory->CooldownEndTime = GetWorld()->GetTimeSeconds() + AttackCooldown;

    return EBTNodeResult::Succeeded;
}
```

**Usage:**
- Melee attacks
- Final Impact
- Combat behavior

---

### BTTask_TriggerFinalImpact

**Purpose:** Execute Golem's final attack sequence

**Parameters:**
```cpp
UPROPERTY(EditAnywhere, Category = "Attack")
UAnimMontage* ImpactMontage;

UPROPERTY(EditAnywhere, Category = "Attack")
float ImpactDamage = 100.0f;

UPROPERTY(EditAnywhere, Category = "Attack")
float ImpactRadius = 300.0f;

UPROPERTY(EditAnywhere, Category = "Attack")
bool bTriggerCutscene = true;

UPROPERTY(EditAnywhere, Category = "Attack")
TSubclassOf<UCameraShakeBase> CameraShakeClass;

UPROPERTY(EditAnywhere, Category = "Blackboard")
FBlackboardKeySelector TargetActorKey;
```

**Implementation:**
```cpp
EBTNodeResult::Type UBTTask_TriggerFinalImpact::ExecuteTask(
    UBehaviorTreeComponent& OwnerComp, 
    uint8* NodeMemory)
{
    AAIController* AIController = OwnerComp.GetAIOwner();
    APawn* ControlledPawn = AIController->GetPawn();
    AActor* TargetActor = GetTargetActor(OwnerComp);

    if (!ControlledPawn || !TargetActor) 
        return EBTNodeResult::Failed;

    // Stop movement
    if (ACharacter* Character = Cast<ACharacter>(ControlledPawn))
    {
        Character->GetCharacterMovement()->StopMovementImmediately();
    }

    // Face target
    FVector Direction = TargetActor->GetActorLocation() - 
        ControlledPawn->GetActorLocation();
    ControlledPawn->SetActorRotation(Direction.Rotation());

    // Play impact animation
    if (ImpactMontage)
    {
        if (ACharacter* Character = Cast<ACharacter>(ControlledPawn))
        {
            if (UAnimInstance* AnimInstance = Character->GetMesh()->GetAnimInstance())
            {
                AnimInstance->Montage_Play(ImpactMontage);
            }
        }
    }

    // Apply radial damage
    TArray<AActor*> IgnoredActors;
    IgnoredActors.Add(ControlledPawn);

    UGameplayStatics::ApplyRadialDamage(
        GetWorld(),
        ImpactDamage,
        ControlledPawn->GetActorLocation(),
        ImpactRadius,
        UDamageType::StaticClass(),
        IgnoredActors,
        ControlledPawn,
        AIController
    );

    // Camera shake
    if (CameraShakeClass)
    {
        UGameplayStatics::PlayWorldCameraShake(
            GetWorld(),
            CameraShakeClass,
            ControlledPawn->GetActorLocation(),
            0.0f,
            ImpactRadius * 2.0f
        );
    }

    // Trigger cutscene
    if (bTriggerCutscene)
    {
        // Call Blueprint event or trigger cutscene system
        if (UHorrorGolemBehaviorComponent* GolemBehavior = 
            ControlledPawn->FindComponentByClass<UHorrorGolemBehaviorComponent>())
        {
            // Trigger cutscene through behavior component
        }
    }

    return EBTNodeResult::Succeeded;
}
```

**Usage:**
- Golem Final Impact phase
- Boss attacks
- Scripted sequences

---

## Investigation Tasks

### BTTask_Investigate

**Purpose:** Move to and investigate a location

**Parameters:**
```cpp
UPROPERTY(EditAnywhere, Category = "Investigation")
float InvestigationSpeed = 250.0f;

UPROPERTY(EditAnywhere, Category = "Investigation")
float SearchRadius = 500.0f;

UPROPERTY(EditAnywhere, Category = "Investigation")
float SearchDuration = 5.0f;

UPROPERTY(EditAnywhere, Category = "Investigation")
int32 SearchPoints = 8;

UPROPERTY(EditAnywhere, Category = "Blackboard")
FBlackboardKeySelector InvestigateLocationKey;

UPROPERTY(EditAnywhere, Category = "Blackboard")
FBlackboardKeySelector AlertStateKey;
```

**Implementation:**
```cpp
EBTNodeResult::Type UBTTask_Investigate::ExecuteTask(
    UBehaviorTreeComponent& OwnerComp, 
    uint8* NodeMemory)
{
    FTaskMemory* Memory = CastInstanceNodeMemory<FTaskMemory>(NodeMemory);
    Memory->SearchTimer = 0.0f;
    Memory->CurrentSearchPoint = 0;
    Memory->bReachedLocation = false;

    // Move to investigate location
    AAIController* AIController = OwnerComp.GetAIOwner();
    UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();

    FVector InvestigateLocation = Blackboard->GetValueAsVector(
        InvestigateLocationKey.SelectedKeyName
    );

    AIController->MoveToLocation(InvestigateLocation, 100.0f);

    return EBTNodeResult::InProgress;
}

void UBTTask_Investigate::TickTask(
    UBehaviorTreeComponent& OwnerComp, 
    uint8* NodeMemory, 
    float DeltaSeconds)
{
    FTaskMemory* Memory = CastInstanceNodeMemory<FTaskMemory>(NodeMemory);
    AAIController* AIController = OwnerComp.GetAIOwner();
    APawn* ControlledPawn = AIController->GetPawn();

    if (!Memory->bReachedLocation)
    {
        // Check if reached location
        FVector InvestigateLocation = OwnerComp.GetBlackboardComponent()->
            GetValueAsVector(InvestigateLocationKey.SelectedKeyName);
        
        float Distance = FVector::Dist(
            ControlledPawn->GetActorLocation(), 
            InvestigateLocation
        );

        if (Distance < 100.0f)
        {
            Memory->bReachedLocation = true;
            AIController->StopMovement();
        }
    }
    else
    {
        // Search behavior
        Memory->SearchTimer += DeltaSeconds;

        if (Memory->SearchTimer >= SearchDuration)
        {
            // Search complete
            FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
            return;
        }

        // Look around at search points
        float Progress = Memory->SearchTimer / SearchDuration;
        int32 TargetPoint = FMath::FloorToInt(Progress * SearchPoints);

        if (TargetPoint != Memory->CurrentSearchPoint)
        {
            Memory->CurrentSearchPoint = TargetPoint;

            // Calculate look direction
            float Angle = (360.0f / SearchPoints) * TargetPoint;
            FRotator LookRotation = FRotator(0, Angle, 0);
            ControlledPawn->SetActorRotation(LookRotation);
        }
    }
}
```

**Usage:**
- Suspicious behavior
- Noise investigation
- Search patterns

---

## State Management Tasks

### BTTask_UpdateState

**Purpose:** Update AI state in blackboard

**Parameters:**
```cpp
UPROPERTY(EditAnywhere, Category = "State")
EGolemEncounterState NewState;

UPROPERTY(EditAnywhere, Category = "Blackboard")
FBlackboardKeySelector CurrentStateKey;

UPROPERTY(EditAnywhere, Category = "Blackboard")
FBlackboardKeySelector StateTimerKey;
```

**Implementation:**
```cpp
EBTNodeResult::Type UBTTask_UpdateState::ExecuteTask(
    UBehaviorTreeComponent& OwnerComp, 
    uint8* NodeMemory)
{
    UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
    if (!Blackboard) return EBTNodeResult::Failed;

    // Update state
    Blackboard->SetValueAsEnum(
        CurrentStateKey.SelectedKeyName, 
        static_cast<uint8>(NewState)
    );

    // Reset state timer
    Blackboard->SetValueAsFloat(StateTimerKey.SelectedKeyName, 0.0f);

    // Update behavior component if present
    AAIController* AIController = OwnerComp.GetAIOwner();
    if (APawn* ControlledPawn = AIController->GetPawn())
    {
        if (UHorrorGolemBehaviorComponent* GolemBehavior = 
            ControlledPawn->FindComponentByClass<UHorrorGolemBehaviorComponent>())
        {
            GolemBehavior->ForceStateTransition(NewState);
        }
    }

    return EBTNodeResult::Succeeded;
}
```

**Usage:**
- State transitions
- Behavior synchronization
- Event triggers

---

## Animation Tasks

### BTTask_PlayMontage

**Purpose:** Play animation montage

**Parameters:**
```cpp
UPROPERTY(EditAnywhere, Category = "Animation")
UAnimMontage* Montage;

UPROPERTY(EditAnywhere, Category = "Animation")
float PlayRate = 1.0f;

UPROPERTY(EditAnywhere, Category = "Animation")
FName StartSection = NAME_None;

UPROPERTY(EditAnywhere, Category = "Animation")
bool bWaitForCompletion = true;
```

**Implementation:**
```cpp
EBTNodeResult::Type UBTTask_PlayMontage::ExecuteTask(
    UBehaviorTreeComponent& OwnerComp, 
    uint8* NodeMemory)
{
    if (!Montage) return EBTNodeResult::Failed;

    AAIController* AIController = OwnerComp.GetAIOwner();
    APawn* ControlledPawn = AIController->GetPawn();
    ACharacter* Character = Cast<ACharacter>(ControlledPawn);

    if (!Character) return EBTNodeResult::Failed;

    UAnimInstance* AnimInstance = Character->GetMesh()->GetAnimInstance();
    if (!AnimInstance) return EBTNodeResult::Failed;

    // Play montage
    float Duration = AnimInstance->Montage_Play(Montage, PlayRate);
    
    if (StartSection != NAME_None)
    {
        AnimInstance->Montage_JumpToSection(StartSection, Montage);
    }

    if (bWaitForCompletion)
    {
        FTaskMemory* Memory = CastInstanceNodeMemory<FTaskMemory>(NodeMemory);
        Memory->MontageEndTime = GetWorld()->GetTimeSeconds() + Duration;
        return EBTNodeResult::InProgress;
    }

    return EBTNodeResult::Succeeded;
}

void UBTTask_PlayMontage::TickTask(
    UBehaviorTreeComponent& OwnerComp, 
    uint8* NodeMemory, 
    float DeltaSeconds)
{
    FTaskMemory* Memory = CastInstanceNodeMemory<FTaskMemory>(NodeMemory);
    
    if (GetWorld()->GetTimeSeconds() >= Memory->MontageEndTime)
    {
        FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
    }
}
```

**Usage:**
- Attack animations
- Idle animations
- Scripted sequences

---

## Task Memory Structures

### Base Task Memory
```cpp
struct FBTTaskMemory
{
    float Timer = 0.0f;
    bool bInitialized = false;
};
```

### Movement Task Memory
```cpp
struct FBTMoveTaskMemory : public FBTTaskMemory
{
    FVector TargetLocation = FVector::ZeroVector;
    float AcceptanceRadius = 100.0f;
    bool bReachedDestination = false;
};
```

### Combat Task Memory
```cpp
struct FBTCombatTaskMemory : public FBTTaskMemory
{
    float CooldownEndTime = 0.0f;
    int32 AttackCount = 0;
    bool bAttackInProgress = false;
};
```

### Investigation Task Memory
```cpp
struct FBTInvestigateTaskMemory : public FBTTaskMemory
{
    float SearchTimer = 0.0f;
    int32 CurrentSearchPoint = 0;
    bool bReachedLocation = false;
    TArray<FVector> SearchPoints;
};
```

## Performance Guidelines

### Task Execution Budget
- Simple tasks: < 0.1ms
- Movement tasks: < 0.3ms
- Combat tasks: < 0.5ms
- Investigation tasks: < 0.2ms

### Optimization Tips
1. Cache frequently accessed components
2. Use squared distance when possible
3. Limit raycasts per frame
4. Batch blackboard updates
5. Use task memory efficiently

## Testing Checklist

- [ ] Task executes without errors
- [ ] Task completes successfully
- [ ] Task fails gracefully
- [ ] Task memory is initialized
- [ ] Task cleans up properly
- [ ] Performance within budget
- [ ] Blackboard updates correctly
- [ ] Animation plays correctly
- [ ] Movement is smooth
- [ ] Combat damage applies

## See Also
- [BehaviorTreeSetup.md](BehaviorTreeSetup.md) - Setup guide
- [GolemBehaviorTree.md](GolemBehaviorTree.md) - Golem implementation
- [ThreatActorBehaviorTree.md](ThreatActorBehaviorTree.md) - Generic threat AI
