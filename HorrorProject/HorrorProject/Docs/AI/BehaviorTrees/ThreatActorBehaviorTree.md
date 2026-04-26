# Threat Actor Behavior Tree

## Overview
Generic behavior tree implementation for all threat actors in the Horror Project. This serves as the base template that can be specialized for specific threats like Golem, Shadow, or other enemies.

## Base Behavior Tree Structure

```
Root (Selector)
├── Sequence: Handle Death
│   ├── Decorator: Is Dead
│   └── Task: Play Death Animation
│
├── Sequence: Combat
│   ├── Decorator: Can Attack Target
│   ├── Task: Attack Target
│   └── Service: Update Combat State
│
├── Sequence: Chase
│   ├── Decorator: Has Target
│   ├── Decorator: Can See Target OR Recent Sighting
│   ├── Task: Move To Target
│   └── Service: Update Last Known Location
│
├── Sequence: Investigate
│   ├── Decorator: Has Last Known Location
│   ├── Task: Move To Last Known Location
│   └── Task: Search Area
│
├── Sequence: Patrol
│   ├── Decorator: Has Patrol Route
│   ├── Task: Move To Next Patrol Point
│   └── Service: Update Patrol
│
└── Task: Idle
```

## Blackboard Keys

### Required Keys
```
TargetActor          (Object: Actor)        - Current target
LastKnownLocation    (Vector)               - Last seen target position
bCanSeeTarget        (Bool)                 - Line of sight to target
bIsInCombat          (Bool)                 - Currently fighting
bIsDead              (Bool)                 - Threat is dead
CurrentPatrolPoint   (Vector)               - Current patrol destination
PatrolPointIndex     (Int)                  - Index in patrol route
ThreatLevel          (Float)                - 0-1, affects behavior
AlertState           (Enum: EAlertState)    - Idle/Suspicious/Alert/Combat
```

### Optional Keys
```
FleeLocation         (Vector)               - Escape destination
CoverLocation        (Vector)               - Cover point
AllyActors           (Object: Array)        - Nearby allies
NoiseLocation        (Vector)               - Heard sound location
InvestigateLocation  (Vector)               - Point of interest
```

## Alert State System

### EAlertState Enum
```cpp
UENUM(BlueprintType)
enum class EAlertState : uint8
{
    Idle        UMETA(DisplayName="Idle"),
    Suspicious  UMETA(DisplayName="Suspicious"),
    Alert       UMETA(DisplayName="Alert"),
    Combat      UMETA(DisplayName="Combat")
};
```

### State Transitions
```
Idle → Suspicious:  Hear noise, see movement
Suspicious → Alert: Confirm threat presence
Alert → Combat:     Target in attack range
Combat → Alert:     Lost sight of target
Alert → Suspicious: No activity for 10s
Suspicious → Idle:  No activity for 20s
```

## Core Behaviors

### 1. Idle Behavior
**Purpose:** Default state when no threats detected

**Implementation:**
```
Task: Idle
├── Play Idle Animation
├── Random Look Around (every 5-10s)
└── Listen for Sounds
```

**Blackboard:**
- AlertState = Idle
- TargetActor = None

### 2. Patrol Behavior
**Purpose:** Move between waypoints when not alerted

**Implementation:**
```
Sequence: Patrol
├── Decorator: Alert State == Idle
├── Service: Update Patrol (1.0s)
│   ├── Get Next Patrol Point
│   └── Update CurrentPatrolPoint
├── Task: Move To Patrol Point
│   ├── Speed: Patrol Speed (200 cm/s)
│   └── Acceptance Radius: 100cm
└── Task: Pause At Waypoint
    └── Duration: 2-5s random
```

**Blackboard:**
- CurrentPatrolPoint = Next waypoint
- PatrolPointIndex = Current index

### 3. Investigate Behavior
**Purpose:** Check suspicious activity

**Implementation:**
```
Sequence: Investigate
├── Decorator: Alert State == Suspicious
├── Decorator: Has Investigate Location
├── Task: Move To Investigate Location
│   ├── Speed: Walk Speed (250 cm/s)
│   └── Acceptance Radius: 150cm
├── Task: Search Area
│   ├── Look Around (360° over 5s)
│   ├── Listen Carefully
│   └── Check Hiding Spots
└── Task: Wait And Listen
    └── Duration: 3s
```

**Blackboard:**
- InvestigateLocation = Point of interest
- AlertState = Suspicious

### 4. Chase Behavior
**Purpose:** Pursue detected target

**Implementation:**
```
Sequence: Chase
├── Decorator: Alert State >= Alert
├── Decorator: Has Target
├── Service: Update Target Info (0.5s)
│   ├── Update Distance
│   ├── Update Line of Sight
│   └── Update Last Known Location
├── Task: Move To Target
│   ├── Speed: Chase Speed (400 cm/s)
│   ├── Use NavMesh: true
│   └── Acceptance Radius: Attack Range
└── Service: Update Alert State
    └── Escalate to Combat if in range
```

**Blackboard:**
- TargetActor = Player or threat
- bCanSeeTarget = true/false
- LastKnownLocation = Updated position

### 5. Combat Behavior
**Purpose:** Attack target when in range

**Implementation:**
```
Sequence: Combat
├── Decorator: Alert State == Combat
├── Decorator: Can Attack Target
│   ├── Distance <= Attack Range
│   └── Line of Sight
├── Task: Face Target
├── Task: Play Attack Animation
├── Task: Apply Damage
│   ├── Damage Amount: 20-50
│   └── Damage Type: Melee/Ranged
└── Task: Attack Cooldown
    └── Duration: 1-3s
```

**Blackboard:**
- bIsInCombat = true
- TargetActor = Attack target

## Custom Tasks

### BTTask_ThreatMoveToTarget
```cpp
UCLASS()
class UBTTask_ThreatMoveToTarget : public UBTTaskNode
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, Category = "Movement")
    float MoveSpeed = 400.0f;

    UPROPERTY(EditAnywhere, Category = "Movement")
    float AcceptanceRadius = 100.0f;

    UPROPERTY(EditAnywhere, Category = "Movement")
    bool bStopOnOverlap = true;

    UPROPERTY(EditAnywhere, Category = "Blackboard")
    FBlackboardKeySelector TargetKey;

    virtual EBTNodeResult::Type ExecuteTask(
        UBehaviorTreeComponent& OwnerComp, 
        uint8* NodeMemory
    ) override;
};
```

### BTTask_ThreatAttack
```cpp
UCLASS()
class UBTTask_ThreatAttack : public UBTTaskNode
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, Category = "Attack")
    UAnimMontage* AttackMontage;

    UPROPERTY(EditAnywhere, Category = "Attack")
    float AttackDamage = 30.0f;

    UPROPERTY(EditAnywhere, Category = "Attack")
    float AttackRange = 200.0f;

    UPROPERTY(EditAnywhere, Category = "Attack")
    float AttackCooldown = 2.0f;

    UPROPERTY(EditAnywhere, Category = "Blackboard")
    FBlackboardKeySelector TargetKey;

    virtual EBTNodeResult::Type ExecuteTask(
        UBehaviorTreeComponent& OwnerComp, 
        uint8* NodeMemory
    ) override;
};
```

### BTTask_ThreatInvestigate
```cpp
UCLASS()
class UBTTask_ThreatInvestigate : public UBTTaskNode
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, Category = "Investigation")
    float SearchRadius = 500.0f;

    UPROPERTY(EditAnywhere, Category = "Investigation")
    float SearchDuration = 5.0f;

    UPROPERTY(EditAnywhere, Category = "Investigation")
    int32 SearchPoints = 8;

    UPROPERTY(EditAnywhere, Category = "Blackboard")
    FBlackboardKeySelector InvestigateLocationKey;

    virtual EBTNodeResult::Type ExecuteTask(
        UBehaviorTreeComponent& OwnerComp, 
        uint8* NodeMemory
    ) override;
};
```

### BTTask_ThreatPatrol
```cpp
UCLASS()
class UBTTask_ThreatPatrol : public UBTTaskNode
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, Category = "Patrol")
    TArray<FVector> PatrolPoints;

    UPROPERTY(EditAnywhere, Category = "Patrol")
    bool bLoopPatrol = true;

    UPROPERTY(EditAnywhere, Category = "Patrol")
    float PatrolSpeed = 200.0f;

    UPROPERTY(EditAnywhere, Category = "Patrol")
    float WaitTimeAtPoint = 3.0f;

    UPROPERTY(EditAnywhere, Category = "Blackboard")
    FBlackboardKeySelector PatrolIndexKey;

    UPROPERTY(EditAnywhere, Category = "Blackboard")
    FBlackboardKeySelector PatrolPointKey;

    virtual EBTNodeResult::Type ExecuteTask(
        UBehaviorTreeComponent& OwnerComp, 
        uint8* NodeMemory
    ) override;
};
```

## Custom Decorators

### BTDecorator_CheckAlertState
```cpp
UCLASS()
class UBTDecorator_CheckAlertState : public UBTDecorator
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, Category = "Alert")
    EAlertState RequiredState;

    UPROPERTY(EditAnywhere, Category = "Alert")
    bool bCheckGreaterOrEqual = false;

    UPROPERTY(EditAnywhere, Category = "Blackboard")
    FBlackboardKeySelector AlertStateKey;

    virtual bool CalculateRawConditionValue(
        UBehaviorTreeComponent& OwnerComp, 
        uint8* NodeMemory
    ) const override;
};
```

### BTDecorator_CanSeeTarget
```cpp
UCLASS()
class UBTDecorator_CanSeeTarget : public UBTDecorator
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, Category = "Perception")
    float SightRange = 2000.0f;

    UPROPERTY(EditAnywhere, Category = "Perception")
    float SightAngle = 90.0f;

    UPROPERTY(EditAnywhere, Category = "Perception")
    bool bRequireLineOfSight = true;

    UPROPERTY(EditAnywhere, Category = "Blackboard")
    FBlackboardKeySelector TargetKey;

    virtual bool CalculateRawConditionValue(
        UBehaviorTreeComponent& OwnerComp, 
        uint8* NodeMemory
    ) const override;
};
```

### BTDecorator_CheckDistance
```cpp
UCLASS()
class UBTDecorator_CheckDistance : public UBTDecorator
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, Category = "Distance")
    float DistanceThreshold = 500.0f;

    UPROPERTY(EditAnywhere, Category = "Distance")
    bool bCheckLessThan = true;

    UPROPERTY(EditAnywhere, Category = "Blackboard")
    FBlackboardKeySelector TargetKey;

    virtual bool CalculateRawConditionValue(
        UBehaviorTreeComponent& OwnerComp, 
        uint8* NodeMemory
    ) const override;
};
```

## Custom Services

### BTService_UpdateThreatState
```cpp
UCLASS()
class UBTService_UpdateThreatState : public UBTService
{
    GENERATED_BODY()

public:
    UBTService_UpdateThreatState();

    UPROPERTY(EditAnywhere, Category = "Update")
    float UpdateInterval = 0.5f;

    UPROPERTY(EditAnywhere, Category = "Blackboard")
    FBlackboardKeySelector TargetKey;

    UPROPERTY(EditAnywhere, Category = "Blackboard")
    FBlackboardKeySelector CanSeeTargetKey;

    UPROPERTY(EditAnywhere, Category = "Blackboard")
    FBlackboardKeySelector LastKnownLocationKey;

    UPROPERTY(EditAnywhere, Category = "Blackboard")
    FBlackboardKeySelector AlertStateKey;

    virtual void TickNode(
        UBehaviorTreeComponent& OwnerComp, 
        uint8* NodeMemory, 
        float DeltaSeconds
    ) override;

protected:
    void UpdateLineOfSight(UBehaviorTreeComponent& OwnerComp);
    void UpdateAlertState(UBehaviorTreeComponent& OwnerComp);
};
```

### BTService_UpdatePerception
```cpp
UCLASS()
class UBTService_UpdatePerception : public UBTService
{
    GENERATED_BODY()

public:
    UBTService_UpdatePerception();

    UPROPERTY(EditAnywhere, Category = "Perception")
    float HearingRange = 1000.0f;

    UPROPERTY(EditAnywhere, Category = "Perception")
    float SightRange = 2000.0f;

    UPROPERTY(EditAnywhere, Category = "Blackboard")
    FBlackboardKeySelector NoiseLocationKey;

    UPROPERTY(EditAnywhere, Category = "Blackboard")
    FBlackboardKeySelector InvestigateLocationKey;

    virtual void TickNode(
        UBehaviorTreeComponent& OwnerComp, 
        uint8* NodeMemory, 
        float DeltaSeconds
    ) override;

protected:
    void CheckForNoises(UBehaviorTreeComponent& OwnerComp);
    void CheckForVisualStimuli(UBehaviorTreeComponent& OwnerComp);
};
```

## Specialization Examples

### Golem Specialization
```
Override:
- Chase Speed: 600 cm/s (Full Chase)
- Attack Range: 500 cm
- No Patrol (uses custom behavior)
- Custom state machine (5 phases)
```

### Shadow Specialization
```
Override:
- Stealth Movement
- Teleport ability
- Darkness preference
- Sound-based detection
```

### Hunter Specialization
```
Override:
- Ranged attacks
- Cover usage
- Flanking behavior
- Group coordination
```

## Integration with Perception System

### Perception Component Setup
```cpp
// In AHorrorThreatAIController constructor
PerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("PerceptionComp"));

// Sight config
UAISenseConfig_Sight* SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
SightConfig->SightRadius = 2000.0f;
SightConfig->LoseSightRadius = 2500.0f;
SightConfig->PeripheralVisionAngleDegrees = 90.0f;
SightConfig->DetectionByAffiliation.bDetectEnemies = true;
PerceptionComponent->ConfigureSense(*SightConfig);

// Hearing config
UAISenseConfig_Hearing* HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));
HearingConfig->HearingRange = 1000.0f;
HearingConfig->DetectionByAffiliation.bDetectEnemies = true;
PerceptionComponent->ConfigureSense(*HearingConfig);

// Bind perception events
PerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &AHorrorThreatAIController::OnPerceptionUpdated);
```

### Perception Event Handling
```cpp
void AHorrorThreatAIController::OnPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
    if (!Blackboard) return;

    if (Stimulus.Type == UAISense::GetSenseID<UAISense_Sight>())
    {
        if (Stimulus.WasSuccessfullySensed())
        {
            Blackboard->SetValueAsObject("TargetActor", Actor);
            Blackboard->SetValueAsBool("bCanSeeTarget", true);
            Blackboard->SetValueAsVector("LastKnownLocation", Actor->GetActorLocation());
            Blackboard->SetValueAsEnum("AlertState", static_cast<uint8>(EAlertState::Alert));
        }
        else
        {
            Blackboard->SetValueAsBool("bCanSeeTarget", false);
        }
    }
    else if (Stimulus.Type == UAISense::GetSenseID<UAISense_Hearing>())
    {
        Blackboard->SetValueAsVector("NoiseLocation", Stimulus.StimulusLocation);
        Blackboard->SetValueAsVector("InvestigateLocation", Stimulus.StimulusLocation);
        
        if (Blackboard->GetValueAsEnum("AlertState") < static_cast<uint8>(EAlertState::Suspicious))
        {
            Blackboard->SetValueAsEnum("AlertState", static_cast<uint8>(EAlertState::Suspicious));
        }
    }
}
```

## Performance Considerations

### Update Frequencies
- Behavior Tree: 10 Hz (0.1s)
- Services: 2 Hz (0.5s)
- Perception: 5 Hz (0.2s)
- Decorators: On demand

### Optimization Tips
1. Use blackboard queries instead of actor queries
2. Cache perception results
3. Reduce service tick rates for distant threats
4. Use LOD system for AI complexity
5. Batch blackboard updates

## Testing

### Test Scenarios
1. Idle → Suspicious (hear noise)
2. Suspicious → Alert (see player)
3. Alert → Combat (player in range)
4. Combat → Alert (lose sight)
5. Alert → Idle (timeout)
6. Patrol → Investigate → Patrol
7. Chase → Lost target → Search

### Debug Commands
```
showdebug ai
ai.debug [ThreatName]
ai.showblackboard
```

## See Also
- [GolemBehaviorTree.md](GolemBehaviorTree.md) - Golem-specific implementation
- [PerceptionSystemSetup.md](../Perception/PerceptionSystemSetup.md) - Perception configuration
- [AITasksReference.md](AITasksReference.md) - Task details
