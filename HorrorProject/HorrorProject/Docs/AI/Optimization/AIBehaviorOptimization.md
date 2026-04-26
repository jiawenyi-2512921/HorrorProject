# AI Behavior Optimization Guide

## Overview
Optimization strategies specifically for AI behavior systems, including state machines, decision-making, and behavior tree execution.

## State Machine Optimization

### 1. Efficient State Transitions

**Optimized State Machine:**
```cpp
class FOptimizedStateMachine
{
private:
    EGolemEncounterState CurrentState;
    EGolemEncounterState PreviousState;
    float StateTimer;
    
    // Transition table for fast lookups
    TMap<EGolemEncounterState, TArray<FStateTransition>> TransitionTable;
    
    struct FStateTransition
    {
        EGolemEncounterState TargetState;
        TFunction<bool()> Condition;
        float Priority;
    };
    
public:
    void UpdateStateMachine(float DeltaTime)
    {
        StateTimer += DeltaTime;
        
        // Check transitions for current state
        if (TransitionTable.Contains(CurrentState))
        {
            const TArray<FStateTransition>& Transitions = TransitionTable[CurrentState];
            
            // Transitions are sorted by priority
            for (const FStateTransition& Transition : Transitions)
            {
                if (Transition.Condition())
                {
                    TransitionToState(Transition.TargetState);
                    return;
                }
            }
        }
        
        // Update current state
        UpdateCurrentState(DeltaTime);
    }
    
    void TransitionToState(EGolemEncounterState NewState)
    {
        if (CurrentState == NewState) return;
        
        PreviousState = CurrentState;
        CurrentState = NewState;
        StateTimer = 0.0f;
        
        OnStateChanged(PreviousState, CurrentState);
    }
};
```

### 2. State Update Optimization

**Distance-Based State Updates:**
```cpp
void UHorrorGolemBehaviorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!bBehaviorActive) return;
    
    // Adjust update frequency based on distance
    float DistanceToTarget = GetDistanceToTarget();
    
    if (DistanceToTarget < 0.0f) return;
    
    // Skip updates for distant AI
    if (DistanceToTarget > 5000.0f)
    {
        // Update every 5th frame
        if (GFrameCounter % 5 != 0) return;
    }
    else if (DistanceToTarget > 2000.0f)
    {
        // Update every 2nd frame
        if (GFrameCounter % 2 != 0) return;
    }
    
    StateTimer += DeltaTime;
    UpdateStateMachine(DeltaTime);
}
```

### 3. Cached Distance Calculations

```cpp
class UHorrorGolemBehaviorComponent : public UActorComponent
{
private:
    float CachedDistance = -1.0f;
    float CachedDistanceSquared = -1.0f;
    float LastDistanceUpdateTime = 0.0f;
    float DistanceCacheLifetime = 0.1f;  // Cache for 0.1s
    
public:
    float GetDistanceToTarget() const
    {
        float CurrentTime = GetWorld()->GetTimeSeconds();
        
        if (CurrentTime - LastDistanceUpdateTime > DistanceCacheLifetime)
        {
            UpdateDistanceCache();
        }
        
        return CachedDistance;
    }
    
    float GetDistanceToTargetSquared() const
    {
        float CurrentTime = GetWorld()->GetTimeSeconds();
        
        if (CurrentTime - LastDistanceUpdateTime > DistanceCacheLifetime)
        {
            UpdateDistanceCache();
        }
        
        return CachedDistanceSquared;
    }
    
private:
    void UpdateDistanceCache() const
    {
        if (!TargetActor.IsValid() || !OwnerThreat.IsValid())
        {
            CachedDistance = -1.0f;
            CachedDistanceSquared = -1.0f;
            return;
        }
        
        FVector OwnerLocation = OwnerThreat->GetActorLocation();
        FVector TargetLocation = TargetActor->GetActorLocation();
        
        CachedDistanceSquared = FVector::DistSquared(OwnerLocation, TargetLocation);
        CachedDistance = FMath::Sqrt(CachedDistanceSquared);
        
        LastDistanceUpdateTime = GetWorld()->GetTimeSeconds();
    }
};
```

## Decision-Making Optimization

### 1. Decision Tree Pruning

**Early Exit Conditions:**
```cpp
EGolemEncounterState UHorrorGolemBehaviorComponent::DetermineNextState()
{
    float Distance = GetDistanceToTargetSquared();  // Use squared distance
    
    // Early exits for common cases
    if (Distance < 0.0f)
    {
        return EGolemEncounterState::Dormant;
    }
    
    // Check in order of likelihood
    if (Distance <= FinalImpact_TriggerDistance * FinalImpact_TriggerDistance)
    {
        return EGolemEncounterState::FinalImpact;
    }
    
    if (Distance <= FullChase_MaxDistance * FullChase_MaxDistance)
    {
        return EGolemEncounterState::FullChase;
    }
    
    if (Distance <= ChaseTriggered_StartDistance * ChaseTriggered_StartDistance)
    {
        return EGolemEncounterState::ChaseTriggered;
    }
    
    if (Distance <= CloseStalking_MaxDistance * CloseStalking_MaxDistance)
    {
        return EGolemEncounterState::CloseStalking;
    }
    
    return EGolemEncounterState::DistantSighting;
}
```

### 2. Utility-Based Decision Making

**Optimized Utility System:**
```cpp
struct FUtilityScore
{
    EAIAction Action;
    float Score;
    
    bool operator<(const FUtilityScore& Other) const
    {
        return Score > Other.Score;  // Higher score first
    }
};

class FUtilityAI
{
private:
    TArray<FUtilityScore> Scores;
    
public:
    EAIAction SelectBestAction()
    {
        Scores.Reset();
        
        // Calculate scores (only for valid actions)
        if (CanAttack())
        {
            Scores.Add({EAIAction::Attack, CalculateAttackScore()});
        }
        
        if (CanChase())
        {
            Scores.Add({EAIAction::Chase, CalculateChaseScore()});
        }
        
        if (CanInvestigate())
        {
            Scores.Add({EAIAction::Investigate, CalculateInvestigateScore()});
        }
        
        // Sort by score (highest first)
        Scores.Sort();
        
        // Return best action
        return Scores.Num() > 0 ? Scores[0].Action : EAIAction::Idle;
    }
    
private:
    float CalculateAttackScore() const
    {
        // Fast calculation using cached values
        float DistanceScore = 1.0f - (CachedDistance / 500.0f);
        float HealthScore = GetHealthPercent();
        float VisibilityScore = bCanSeeTarget ? 1.0f : 0.5f;
        
        return (DistanceScore * 0.5f + HealthScore * 0.3f + VisibilityScore * 0.2f);
    }
};
```

## Movement Optimization

### 1. Simplified Movement

**Direct Movement for Close Targets:**
```cpp
void UHorrorGolemBehaviorComponent::MoveTowardsTarget(float Speed, float DeltaTime)
{
    if (!OwnerThreat.IsValid() || !TargetActor.IsValid()) return;
    
    ACharacter* ThreatCharacter = Cast<ACharacter>(OwnerThreat.Get());
    if (!ThreatCharacter) return;
    
    const FVector CurrentLocation = OwnerThreat->GetActorLocation();
    const FVector TargetLocation = TargetActor->GetActorLocation();
    
    // Use simple movement for close targets
    float DistanceSquared = FVector::DistSquared(CurrentLocation, TargetLocation);
    
    if (DistanceSquared < 500.0f * 500.0f)  // < 5m
    {
        // Direct movement without pathfinding
        FVector Direction = (TargetLocation - CurrentLocation).GetSafeNormal();
        FVector NewLocation = CurrentLocation + Direction * Speed * DeltaTime;
        
        ThreatCharacter->SetActorLocation(NewLocation, true);
    }
    else
    {
        // Use navigation for distant targets
        if (AAIController* AIController = Cast<AAIController>(ThreatCharacter->GetController()))
        {
            AIController->MoveToLocation(TargetLocation);
        }
    }
    
    // Face target (optimized rotation)
    FaceTargetOptimized(TargetLocation, DeltaTime);
}
```

### 2. Optimized Rotation

**Smooth Rotation with Early Exit:**
```cpp
void UHorrorGolemBehaviorComponent::FaceTargetOptimized(FVector TargetLocation, float DeltaTime)
{
    if (!OwnerThreat.IsValid()) return;
    
    FVector CurrentLocation = OwnerThreat->GetActorLocation();
    FVector Direction = TargetLocation - CurrentLocation;
    Direction.Z = 0;  // Ignore vertical component
    
    // Early exit if already facing target
    FVector CurrentForward = OwnerThreat->GetActorForwardVector();
    CurrentForward.Z = 0;
    
    float DotProduct = FVector::DotProduct(CurrentForward.GetSafeNormal(), Direction.GetSafeNormal());
    
    if (DotProduct > 0.99f)  // Already facing target (within ~8 degrees)
    {
        return;
    }
    
    // Smooth rotation
    FRotator TargetRotation = Direction.Rotation();
    FRotator CurrentRotation = OwnerThreat->GetActorRotation();
    
    float InterpSpeed = 5.0f;
    FRotator NewRotation = FMath::RInterpTo(CurrentRotation, TargetRotation, DeltaTime, InterpSpeed);
    
    OwnerThreat->SetActorRotation(NewRotation);
}
```

### 3. Patrol Optimization

**Cached Patrol Calculations:**
```cpp
void UHorrorGolemBehaviorComponent::PatrolAroundTarget(float DeltaTime)
{
    if (!OwnerThreat.IsValid() || !TargetActor.IsValid()) return;
    
    // Cache patrol calculation
    static FVector LastPatrolDirection = FVector::ZeroVector;
    static float LastPatrolUpdateTime = 0.0f;
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    if (CurrentTime - LastPatrolUpdateTime > 0.2f)  // Update every 0.2s
    {
        const FVector TargetLocation = TargetActor->GetActorLocation();
        const FVector CurrentLocation = OwnerThreat->GetActorLocation();
        
        const float DesiredDistance = (CloseStalking_MinDistance + CloseStalking_MaxDistance) * 0.5f;
        const FVector ToTarget = TargetLocation - CurrentLocation;
        const float CurrentDistance = ToTarget.Size();
        
        // Calculate patrol direction
        FVector CircleDirection = FVector::CrossProduct(ToTarget.GetSafeNormal(), FVector::UpVector);
        
        LastPatrolDirection = CircleDirection;
        
        // Adjust for distance
        if (CurrentDistance > DesiredDistance + 100.0f)
        {
            LastPatrolDirection += ToTarget.GetSafeNormal() * 0.5f;
        }
        else if (CurrentDistance < DesiredDistance - 100.0f)
        {
            LastPatrolDirection -= ToTarget.GetSafeNormal() * 0.5f;
        }
        
        LastPatrolDirection.Normalize();
        LastPatrolUpdateTime = CurrentTime;
    }
    
    // Apply cached direction
    ACharacter* ThreatCharacter = Cast<ACharacter>(OwnerThreat.Get());
    if (ThreatCharacter)
    {
        FVector NewLocation = OwnerThreat->GetActorLocation() + 
            LastPatrolDirection * CloseStalking_PatrolSpeed * DeltaTime;
        ThreatCharacter->SetActorLocation(NewLocation, true);
    }
}
```

## Behavior Tree Optimization

### 1. Decorator Optimization

**Cached Decorator Results:**
```cpp
class UBTDecorator_CachedCheck : public UBTDecorator
{
private:
    mutable bool bCachedResult = false;
    mutable float LastCheckTime = 0.0f;
    float CacheLifetime = 0.1f;
    
public:
    virtual bool CalculateRawConditionValue(
        UBehaviorTreeComponent& OwnerComp, 
        uint8* NodeMemory) const override
    {
        float CurrentTime = OwnerComp.GetWorld()->GetTimeSeconds();
        
        // Use cached result if still valid
        if (CurrentTime - LastCheckTime < CacheLifetime)
        {
            return bCachedResult;
        }
        
        // Calculate new result
        bCachedResult = PerformCheck(OwnerComp);
        LastCheckTime = CurrentTime;
        
        return bCachedResult;
    }
    
protected:
    virtual bool PerformCheck(UBehaviorTreeComponent& OwnerComp) const
    {
        // Actual check logic
        return true;
    }
};
```

### 2. Service Optimization

**Batched Service Updates:**
```cpp
class UBTService_BatchedUpdate : public UBTService
{
private:
    TArray<FName> KeysToUpdate;
    
public:
    virtual void TickNode(
        UBehaviorTreeComponent& OwnerComp, 
        uint8* NodeMemory, 
        float DeltaSeconds) override
    {
        UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
        if (!Blackboard) return;
        
        // Batch all updates
        TMap<FName, FBlackboardEntry> Updates;
        
        // Collect all values to update
        for (const FName& Key : KeysToUpdate)
        {
            FBlackboardEntry Value = CalculateValue(OwnerComp, Key);
            Updates.Add(Key, Value);
        }
        
        // Apply all updates at once
        for (const auto& Update : Updates)
        {
            Blackboard->SetValue(Update.Key, Update.Value);
        }
    }
};
```

### 3. Task Optimization

**Lightweight Task Execution:**
```cpp
class UBTTask_Lightweight : public UBTTaskNode
{
public:
    virtual EBTNodeResult::Type ExecuteTask(
        UBehaviorTreeComponent& OwnerComp, 
        uint8* NodeMemory) override
    {
        // Minimize allocations
        AAIController* AIController = OwnerComp.GetAIOwner();
        if (!AIController) return EBTNodeResult::Failed;
        
        // Use stack variables
        FVector Location = AIController->GetPawn()->GetActorLocation();
        
        // Avoid string operations
        // Avoid dynamic arrays
        // Avoid complex calculations
        
        // Quick execution
        return PerformQuickTask(AIController, Location);
    }
    
private:
    FORCEINLINE EBTNodeResult::Type PerformQuickTask(AAIController* Controller, const FVector& Location)
    {
        // Inline for performance
        return EBTNodeResult::Succeeded;
    }
};
```

## Animation Optimization

### 1. Animation State Caching

```cpp
class AHorrorThreatCharacter : public ACharacter
{
private:
    EAnimationState CachedAnimState = EAnimationState::Idle;
    float LastAnimUpdateTime = 0.0f;
    
public:
    void UpdateAnimationState()
    {
        float CurrentTime = GetWorld()->GetTimeSeconds();
        
        // Update animation state less frequently
        if (CurrentTime - LastAnimUpdateTime < 0.1f)
        {
            return;
        }
        
        EAnimationState NewState = DetermineAnimationState();
        
        if (NewState != CachedAnimState)
        {
            CachedAnimState = NewState;
            ApplyAnimationState(NewState);
        }
        
        LastAnimUpdateTime = CurrentTime;
    }
};
```

### 2. LOD-Based Animation

```cpp
void AHorrorThreatCharacter::UpdateAnimationLOD(float DistanceToPlayer)
{
    USkeletalMeshComponent* Mesh = GetMesh();
    if (!Mesh) return;
    
    if (DistanceToPlayer < 1000.0f)  // Close
    {
        Mesh->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;
    }
    else if (DistanceToPlayer < 3000.0f)  // Medium
    {
        Mesh->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::OnlyTickPoseWhenRendered;
    }
    else  // Far
    {
        Mesh->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::OnlyTickMontagesWhenNotRendered;
    }
}
```

## Debug Draw Optimization

### 1. Conditional Debug Drawing

```cpp
void UHorrorGolemBehaviorComponent::DrawDebugState()
{
#if ENABLE_DRAW_DEBUG
    // Only draw if debug mode is enabled
    if (!bDrawDebugInfo) return;
    
    // Only draw if AI is close to player
    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (PC && PC->GetPawn())
    {
        float Distance = FVector::Dist(
            OwnerThreat->GetActorLocation(),
            PC->GetPawn()->GetActorLocation()
        );
        
        if (Distance > 5000.0f)  // Don't draw for distant AI
        {
            return;
        }
    }
    
    // Draw debug info
    DrawDebugSphere(GetWorld(), OwnerThreat->GetActorLocation(), 50.0f, 12, FColor::Red);
#endif
}
```

### 2. Batched Debug Drawing

```cpp
class FAIDebugDrawBatcher
{
private:
    TArray<FDebugSphere> Spheres;
    TArray<FDebugLine> Lines;
    
public:
    void AddSphere(const FVector& Location, float Radius, const FColor& Color)
    {
        Spheres.Add({Location, Radius, Color});
    }
    
    void AddLine(const FVector& Start, const FVector& End, const FColor& Color)
    {
        Lines.Add({Start, End, Color});
    }
    
    void Flush(UWorld* World)
    {
        // Draw all at once
        for (const FDebugSphere& Sphere : Spheres)
        {
            DrawDebugSphere(World, Sphere.Location, Sphere.Radius, 12, Sphere.Color);
        }
        
        for (const FDebugLine& Line : Lines)
        {
            DrawDebugLine(World, Line.Start, Line.End, Line.Color);
        }
        
        // Clear
        Spheres.Reset();
        Lines.Reset();
    }
};
```

## Optimization Checklist

### State Machine
- [ ] Cache distance calculations
- [ ] Use squared distance comparisons
- [ ] Implement frame-skipping for distant AI
- [ ] Optimize state transition checks
- [ ] Use early exit conditions

### Decision Making
- [ ] Prune decision trees
- [ ] Cache utility scores
- [ ] Use fast approximations
- [ ] Implement decision cooldowns
- [ ] Batch decision updates

### Movement
- [ ] Use simple movement for close targets
- [ ] Cache patrol calculations
- [ ] Optimize rotation updates
- [ ] Implement movement LOD
- [ ] Reduce pathfinding frequency

### Behavior Tree
- [ ] Cache decorator results
- [ ] Batch service updates
- [ ] Optimize task execution
- [ ] Minimize allocations
- [ ] Use inline functions

### Animation
- [ ] Cache animation states
- [ ] Implement animation LOD
- [ ] Reduce update frequency
- [ ] Use simpler animations for distant AI
- [ ] Disable animations when not visible

### Debug
- [ ] Conditional debug drawing
- [ ] Batch debug draws
- [ ] Disable debug in shipping
- [ ] Limit debug range
- [ ] Use debug categories

## Performance Targets

### Per-Frame Budget
- State Machine: < 0.1ms
- Decision Making: < 0.2ms
- Movement: < 0.3ms
- Animation: < 0.2ms
- Total Behavior: < 0.8ms

### Update Frequencies
- Close (< 10m): Every frame
- Medium (10-30m): Every 2 frames
- Far (30-50m): Every 5 frames
- Very Far (> 50m): Every 10 frames

## Testing

### Performance Tests
1. Single AI behavior update time
2. 10 AI simultaneous updates
3. 50 AI with mixed distances
4. Stress test with 100+ AI
5. Long-running stability test

### Optimization Validation
1. Profile before optimization
2. Apply optimization
3. Profile after optimization
4. Verify performance gain
5. Check for regressions

## See Also
- [AIPerformanceOptimization.md](AIPerformanceOptimization.md) - General performance optimization
- [AIMemoryOptimization.md](AIMemoryOptimization.md) - Memory optimization
- [AnalyzeAIPerformance.ps1](../Debug/AnalyzeAIPerformance.ps1) - Performance analysis tool
