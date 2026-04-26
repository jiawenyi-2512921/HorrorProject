# Golem Behavior Tree Implementation

## Overview
Detailed implementation of the Golem behavior tree, mapping the 5-phase encounter system to behavior tree nodes.

## Behavior Tree Hierarchy

```
Root (Selector)
├── Sequence: Final Impact (Priority 1)
│   ├── Decorator: Distance <= 500cm
│   ├── Task: Stop Movement
│   ├── Task: Trigger Attack Animation
│   └── Task: Trigger Cutscene
│
├── Sequence: Full Chase (Priority 2)
│   ├── Decorator: Distance <= 2500cm AND Distance > 500cm
│   ├── Service: Check Environment Destruction
│   ├── Task: Move To Target (600 cm/s)
│   └── Task: Face Target
│
├── Sequence: Chase Triggered (Priority 3)
│   ├── Decorator: Distance <= 2000cm AND Distance > 1000cm
│   ├── Task: Move To Target (280 cm/s, 70% speed)
│   └── Task: Face Target
│
├── Sequence: Close Stalking (Priority 4)
│   ├── Decorator: Distance <= 1500cm AND Distance > 1000cm
│   ├── Decorator: State Timer >= 3.0s
│   ├── Service: Update Patrol Location
│   ├── Task: Patrol Around Target (150 cm/s)
│   └── Task: Random Pause
│
├── Sequence: Distant Sighting (Priority 5)
│   ├── Decorator: Distance > 1500cm
│   ├── Task: Face Target
│   └── Task: Slow Approach (50 cm/s) [Optional]
│
└── Task: Idle (Fallback)
```

## Phase Implementations

### Phase 1: Distant Sighting (30m+)

**Conditions:**
- Distance > 3000cm (30m)
- Behavior Active

**Behavior:**
```
Sequence: Distant Sighting
├── Decorator: Check Distance Range
│   └── Distance > DistantSightingMinDistance (3000cm)
├── Task: Face Target
│   └── Rotation Speed: 2.0 deg/s
└── Task: Slow Approach [Optional]
    ├── Speed: 50 cm/s
    └── Condition: !bDistantSightingStationary
```

**Blackboard Updates:**
- CurrentState = DistantSighting
- bIsChasing = false

### Phase 2: Close Stalking (10-15m)

**Conditions:**
- Distance: 1000-1500cm
- State Timer >= 3.0s

**Behavior:**
```
Sequence: Close Stalking
├── Decorator: Check Distance Range
│   ├── Distance >= CloseStalking_MinDistance (1000cm)
│   └── Distance <= CloseStalking_MaxDistance (1500cm)
├── Decorator: Check State Timer
│   └── StateTimer >= 3.0s
├── Service: Update Patrol (0.5s interval)
│   ├── Calculate Circle Position
│   ├── Update PatrolLocation
│   └── Random Pause Check (1% chance)
├── Task: Move To Patrol Location
│   └── Speed: 150 cm/s
└── Task: Handle Pause
    ├── Duration: 2.0s
    └── Face Random Direction
```

**Patrol Algorithm:**
```cpp
// Circle around target at desired distance
DesiredDistance = (MinDistance + MaxDistance) * 0.5f
CircleDirection = Cross(ToTarget, UpVector)

if (CurrentDistance > DesiredDistance + 100)
    MoveDirection = CircleDirection + ToTarget * 0.5f
else if (CurrentDistance < DesiredDistance - 100)
    MoveDirection = CircleDirection - ToTarget * 0.5f
else
    MoveDirection = CircleDirection
```

**Blackboard Updates:**
- CurrentState = CloseStalking
- PatrolLocation = Calculated position
- bIsChasing = false

### Phase 3: Chase Triggered (20m, 70% speed)

**Conditions:**
- Distance <= 2000cm
- Previous state was CloseStalking

**Behavior:**
```
Sequence: Chase Triggered
├── Decorator: Check Distance
│   └── Distance <= ChaseTriggered_StartDistance (2000cm)
├── Task: Move To Target
│   ├── Speed: 280 cm/s (400 * 0.7)
│   └── Use NavMesh: true
└── Task: Face Target
    └── Rotation Speed: 5.0 deg/s
```

**Blackboard Updates:**
- CurrentState = ChaseTriggered
- bIsChasing = true
- LastKnownLocation = Target location

### Phase 4: Full Chase (10-25m, 100% speed)

**Conditions:**
- Distance: 1000-2500cm
- Chasing active

**Behavior:**
```
Sequence: Full Chase
├── Decorator: Check Distance Range
│   ├── Distance >= FullChase_MinDistance (1000cm)
│   └── Distance <= FullChase_MaxDistance (2500cm)
├── Service: Environment Destruction (1.0s interval)
│   ├── Check: 5% chance per tick
│   ├── Raycast Forward (200cm)
│   └── Trigger Destruction Event
├── Task: Move To Target
│   ├── Speed: 600 cm/s
│   ├── Use NavMesh: false (direct movement)
│   └── Collision: Sweep
└── Task: Face Target
    └── Rotation Speed: 5.0 deg/s
```

**Destruction Check:**
```cpp
// Every 1 second, 5% chance
if (FMath::FRand() < 0.05f)
{
    FVector DestructionLocation = 
        GolemLocation + ForwardVector * 200.0f;
    BP_OnEnvironmentDestruction(DestructionLocation);
}
```

**Blackboard Updates:**
- CurrentState = FullChase
- bIsChasing = true

### Phase 5: Final Impact (5m, attack)

**Conditions:**
- Distance <= 500cm

**Behavior:**
```
Sequence: Final Impact
├── Decorator: Check Distance
│   └── Distance <= FinalImpact_TriggerDistance (500cm)
├── Task: Stop Movement
│   └── StopMovementImmediately()
├── Task: Face Target
│   └── Instant rotation
├── Task: Play Attack Animation
│   └── Montage: AM_GolemAttack
├── Task: Trigger Cutscene [Optional]
│   └── Condition: bFinalImpact_TriggerCutscene
└── Task: Apply Damage
    ├── Damage: 100
    └── Damage Type: Melee
```

**Blackboard Updates:**
- CurrentState = FinalImpact
- bIsChasing = false

## Custom Task Implementations

### BTTask_GolemMoveToTarget.h
```cpp
#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_GolemMoveToTarget.generated.h"

UCLASS()
class HORRORPROJECT_API UBTTask_GolemMoveToTarget : public UBTTaskNode
{
    GENERATED_BODY()

public:
    UBTTask_GolemMoveToTarget();

    virtual EBTNodeResult::Type ExecuteTask(
        UBehaviorTreeComponent& OwnerComp, 
        uint8* NodeMemory
    ) override;

    virtual void TickTask(
        UBehaviorTreeComponent& OwnerComp, 
        uint8* NodeMemory, 
        float DeltaSeconds
    ) override;

    UPROPERTY(EditAnywhere, Category = "Movement")
    float MoveSpeed = 400.0f;

    UPROPERTY(EditAnywhere, Category = "Movement")
    float AcceptanceRadius = 100.0f;

    UPROPERTY(EditAnywhere, Category = "Movement")
    bool bUseNavMesh = true;

    UPROPERTY(EditAnywhere, Category = "Blackboard")
    FBlackboardKeySelector TargetActorKey;
};
```

### BTTask_GolemPatrolAroundTarget.h
```cpp
#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_GolemPatrolAroundTarget.generated.h"

UCLASS()
class HORRORPROJECT_API UBTTask_GolemPatrolAroundTarget : public UBTTaskNode
{
    GENERATED_BODY()

public:
    UBTTask_GolemPatrolAroundTarget();

    virtual EBTNodeResult::Type ExecuteTask(
        UBehaviorTreeComponent& OwnerComp, 
        uint8* NodeMemory
    ) override;

    virtual void TickTask(
        UBehaviorTreeComponent& OwnerComp, 
        uint8* NodeMemory, 
        float DeltaSeconds
    ) override;

    UPROPERTY(EditAnywhere, Category = "Patrol")
    float PatrolSpeed = 150.0f;

    UPROPERTY(EditAnywhere, Category = "Patrol")
    float MinDistance = 1000.0f;

    UPROPERTY(EditAnywhere, Category = "Patrol")
    float MaxDistance = 1500.0f;

    UPROPERTY(EditAnywhere, Category = "Patrol")
    float PauseChance = 0.01f;

    UPROPERTY(EditAnywhere, Category = "Blackboard")
    FBlackboardKeySelector TargetActorKey;

    UPROPERTY(EditAnywhere, Category = "Blackboard")
    FBlackboardKeySelector PatrolLocationKey;
};
```

### BTTask_GolemTriggerFinalImpact.h
```cpp
#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_GolemTriggerFinalImpact.generated.h"

UCLASS()
class HORRORPROJECT_API UBTTask_GolemTriggerFinalImpact : public UBTTaskNode
{
    GENERATED_BODY()

public:
    UBTTask_GolemTriggerFinalImpact();

    virtual EBTNodeResult::Type ExecuteTask(
        UBehaviorTreeComponent& OwnerComp, 
        uint8* NodeMemory
    ) override;

    UPROPERTY(EditAnywhere, Category = "Attack")
    UAnimMontage* AttackMontage;

    UPROPERTY(EditAnywhere, Category = "Attack")
    float Damage = 100.0f;

    UPROPERTY(EditAnywhere, Category = "Attack")
    bool bTriggerCutscene = true;

    UPROPERTY(EditAnywhere, Category = "Blackboard")
    FBlackboardKeySelector TargetActorKey;
};
```

## Custom Decorator Implementations

### BTDecorator_CheckGolemDistance.h
```cpp
#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "BTDecorator_CheckGolemDistance.generated.h"

UENUM(BlueprintType)
enum class EDistanceCheck : uint8
{
    LessThan,
    LessThanOrEqual,
    GreaterThan,
    GreaterThanOrEqual,
    InRange
};

UCLASS()
class HORRORPROJECT_API UBTDecorator_CheckGolemDistance : public UBTDecorator
{
    GENERATED_BODY()

public:
    UBTDecorator_CheckGolemDistance();

    virtual bool CalculateRawConditionValue(
        UBehaviorTreeComponent& OwnerComp, 
        uint8* NodeMemory
    ) const override;

    UPROPERTY(EditAnywhere, Category = "Distance")
    EDistanceCheck CheckType = EDistanceCheck::LessThanOrEqual;

    UPROPERTY(EditAnywhere, Category = "Distance")
    float DistanceThreshold = 1000.0f;

    UPROPERTY(EditAnywhere, Category = "Distance")
    float MaxDistance = 2000.0f; // For InRange check

    UPROPERTY(EditAnywhere, Category = "Blackboard")
    FBlackboardKeySelector TargetActorKey;
};
```

### BTDecorator_CheckGolemState.h
```cpp
#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "AI/HorrorGolemBehaviorComponent.h"
#include "BTDecorator_CheckGolemState.generated.h"

UCLASS()
class HORRORPROJECT_API UBTDecorator_CheckGolemState : public UBTDecorator
{
    GENERATED_BODY()

public:
    UBTDecorator_CheckGolemState();

    virtual bool CalculateRawConditionValue(
        UBehaviorTreeComponent& OwnerComp, 
        uint8* NodeMemory
    ) const override;

    UPROPERTY(EditAnywhere, Category = "State")
    EGolemEncounterState RequiredState;

    UPROPERTY(EditAnywhere, Category = "Blackboard")
    FBlackboardKeySelector CurrentStateKey;
};
```

## Custom Service Implementations

### BTService_UpdateGolemBlackboard.h
```cpp
#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTService_UpdateGolemBlackboard.generated.h"

UCLASS()
class HORRORPROJECT_API UBTService_UpdateGolemBlackboard : public UBTService
{
    GENERATED_BODY()

public:
    UBTService_UpdateGolemBlackboard();

    virtual void TickNode(
        UBehaviorTreeComponent& OwnerComp, 
        uint8* NodeMemory, 
        float DeltaSeconds
    ) override;

    UPROPERTY(EditAnywhere, Category = "Blackboard")
    FBlackboardKeySelector TargetActorKey;

    UPROPERTY(EditAnywhere, Category = "Blackboard")
    FBlackboardKeySelector DistanceToTargetKey;

    UPROPERTY(EditAnywhere, Category = "Blackboard")
    FBlackboardKeySelector StateTimerKey;

    UPROPERTY(EditAnywhere, Category = "Blackboard")
    FBlackboardKeySelector CanSeeTargetKey;

    UPROPERTY(EditAnywhere, Category = "Blackboard")
    FBlackboardKeySelector LastKnownLocationKey;
};
```

## State Transition Logic

### Transition Rules
```cpp
// Distance-based transitions
if (Distance > 3000)        → DistantSighting
if (Distance <= 1500)       → CloseStalking
if (Distance <= 2000 && StateTimer >= 3.0) → ChaseTriggered
if (Distance <= 2500)       → FullChase
if (Distance <= 500)        → FinalImpact

// Reverse transitions
if (FullChase && Distance > 2500)  → ChaseTriggered
if (ChaseTriggered && Distance > 2000) → CloseStalking
if (CloseStalking && Distance > 1500) → DistantSighting
```

## Debug Visualization

### Enable in Editor
```cpp
// Console command
showdebug behaviortree

// Or in code
if (GEngine)
{
    GEngine->AddOnScreenDebugMessage(
        -1, 0.0f, FColor::Yellow,
        FString::Printf(TEXT("State: %s, Distance: %.0f"), 
            *UEnum::GetValueAsString(CurrentState),
            DistanceToTarget)
    );
}
```

### Visual Indicators
- Blue sphere: Distant Sighting
- Yellow sphere: Close Stalking
- Orange sphere: Chase Triggered
- Red sphere: Full Chase
- Magenta sphere: Final Impact

## Performance Metrics

### Target Frame Budget
- Behavior Tree Evaluation: < 0.5ms
- Movement Calculation: < 0.2ms
- State Update: < 0.1ms
- Total AI Budget: < 1.0ms per Golem

### Optimization Tips
1. Cache distance calculations
2. Use squared distance when possible
3. Reduce service update frequency
4. Batch blackboard updates
5. Use simple collision for movement

## Testing Checklist

- [ ] All 5 phases transition correctly
- [ ] Distance thresholds work as expected
- [ ] Patrol behavior circles smoothly
- [ ] Chase speed matches specifications
- [ ] Final impact triggers attack
- [ ] State timer increments properly
- [ ] Blackboard updates correctly
- [ ] Debug visualization displays
- [ ] Performance within budget
- [ ] No navigation errors

## See Also
- [BehaviorTreeSetup.md](BehaviorTreeSetup.md) - Setup guide
- [AITasksReference.md](AITasksReference.md) - Task implementations
- [ThreatActorBehaviorTree.md](ThreatActorBehaviorTree.md) - Generic threat AI
