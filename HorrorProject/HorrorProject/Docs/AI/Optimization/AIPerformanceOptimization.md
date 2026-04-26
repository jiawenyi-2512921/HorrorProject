# AI Performance Optimization Guide

## Overview
Comprehensive guide for optimizing AI performance in the Horror Project, covering behavior trees, perception, navigation, and memory management.

## Performance Targets

### Per-AI Budget
- Total AI Budget: < 2.0ms per AI
- Behavior Tree: < 0.5ms
- Perception: < 0.5ms
- Navigation: < 1.0ms
- State Machine: < 0.1ms

### System-Wide Targets
- 10 AI actors: < 20ms total
- 20 AI actors: < 40ms total
- Target: 60 FPS (16.67ms frame budget)
- AI should use < 25% of frame budget

## Behavior Tree Optimization

### 1. Update Frequency Optimization

**Distance-Based Update Rates:**
```cpp
void AHorrorThreatAIController::UpdateBehaviorTreeFrequency(float DistanceToPlayer)
{
    if (!BrainComponent) return;
    
    float UpdateInterval;
    
    if (DistanceToPlayer < 1000.0f)  // < 10m - High priority
    {
        UpdateInterval = 0.1f;  // 10 Hz
    }
    else if (DistanceToPlayer < 3000.0f)  // < 30m - Medium priority
    {
        UpdateInterval = 0.2f;  // 5 Hz
    }
    else if (DistanceToPlayer < 5000.0f)  // < 50m - Low priority
    {
        UpdateInterval = 0.5f;  // 2 Hz
    }
    else  // > 50m - Very low priority
    {
        UpdateInterval = 1.0f;  // 1 Hz
    }
    
    if (UBehaviorTreeComponent* BTComp = Cast<UBehaviorTreeComponent>(BrainComponent))
    {
        // Set update interval
        BTComp->SetUpdateInterval(UpdateInterval);
    }
}
```

**Visibility-Based Updates:**
```cpp
void AHorrorThreatAIController::UpdateBasedOnVisibility()
{
    APawn* ControlledPawn = GetPawn();
    if (!ControlledPawn) return;
    
    // Check if AI is rendered
    bool bIsRendered = ControlledPawn->WasRecentlyRendered(0.5f);
    
    if (!bIsRendered)
    {
        // Reduce update rate for non-visible AI
        if (UBehaviorTreeComponent* BTComp = Cast<UBehaviorTreeComponent>(BrainComponent))
        {
            BTComp->SetUpdateInterval(1.0f);  // 1 Hz
        }
    }
}
```

### 2. Blackboard Optimization

**Cache Frequently Accessed Values:**
```cpp
class AHorrorThreatAIController : public AAIController
{
private:
    // Cached blackboard values
    UPROPERTY()
    AActor* CachedTargetActor = nullptr;
    
    FVector CachedLastKnownLocation = FVector::ZeroVector;
    float CachedDistanceToTarget = -1.0f;
    float CacheUpdateTime = 0.0f;
    float CacheLifetime = 0.1f;  // Update cache every 0.1s
    
public:
    AActor* GetCachedTargetActor()
    {
        float CurrentTime = GetWorld()->GetTimeSeconds();
        
        if (CurrentTime - CacheUpdateTime > CacheLifetime)
        {
            if (Blackboard)
            {
                CachedTargetActor = Cast<AActor>(
                    Blackboard->GetValueAsObject("TargetActor")
                );
            }
            CacheUpdateTime = CurrentTime;
        }
        
        return CachedTargetActor;
    }
    
    float GetCachedDistanceToTarget()
    {
        float CurrentTime = GetWorld()->GetTimeSeconds();
        
        if (CurrentTime - CacheUpdateTime > CacheLifetime)
        {
            if (Blackboard)
            {
                CachedDistanceToTarget = Blackboard->GetValueAsFloat("DistanceToTarget");
            }
            CacheUpdateTime = CurrentTime;
        }
        
        return CachedDistanceToTarget;
    }
};
```

**Batch Blackboard Updates:**
```cpp
void AHorrorThreatAIController::BatchUpdateBlackboard()
{
    if (!Blackboard) return;
    
    // Collect all updates
    TMap<FName, FBlackboardEntry> Updates;
    
    // Calculate all values
    AActor* Target = GetTargetActor();
    if (Target)
    {
        Updates.Add("TargetActor", FBlackboardEntry(Target));
        Updates.Add("LastKnownLocation", FBlackboardEntry(Target->GetActorLocation()));
        
        float Distance = FVector::Dist(
            GetPawn()->GetActorLocation(),
            Target->GetActorLocation()
        );
        Updates.Add("DistanceToTarget", FBlackboardEntry(Distance));
    }
    
    // Apply all updates at once
    for (const auto& Update : Updates)
    {
        // Set blackboard value
        // This minimizes blackboard notification overhead
    }
}
```

### 3. Task Optimization

**Use Squared Distance:**
```cpp
// Instead of:
float Distance = FVector::Dist(A, B);
if (Distance < Threshold)

// Use:
float DistanceSquared = FVector::DistSquared(A, B);
if (DistanceSquared < Threshold * Threshold)
```

**Early Exit Conditions:**
```cpp
EBTNodeResult::Type UBTTask_OptimizedTask::ExecuteTask(
    UBehaviorTreeComponent& OwnerComp, 
    uint8* NodeMemory)
{
    // Early validation checks
    AAIController* AIController = OwnerComp.GetAIOwner();
    if (!AIController) return EBTNodeResult::Failed;
    
    APawn* ControlledPawn = AIController->GetPawn();
    if (!ControlledPawn) return EBTNodeResult::Failed;
    
    // Quick blackboard check
    UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
    if (!Blackboard) return EBTNodeResult::Failed;
    
    // Early exit if conditions not met
    if (!CheckPreconditions(OwnerComp))
    {
        return EBTNodeResult::Failed;
    }
    
    // Expensive operations only if needed
    return PerformTask(OwnerComp, NodeMemory);
}
```

## Perception Optimization

### 1. LOD System for Perception

```cpp
void AHorrorThreatAIController::UpdatePerceptionLOD(float DistanceToPlayer)
{
    if (!PerceptionComponent) return;
    
    // Sight LOD
    float SightInterval;
    float SightRange;
    
    if (DistanceToPlayer < 1000.0f)  // Close
    {
        SightInterval = 0.1f;  // 10 Hz
        SightRange = 2000.0f;
    }
    else if (DistanceToPlayer < 3000.0f)  // Medium
    {
        SightInterval = 0.2f;  // 5 Hz
        SightRange = 1500.0f;
    }
    else if (DistanceToPlayer < 5000.0f)  // Far
    {
        SightInterval = 0.5f;  // 2 Hz
        SightRange = 1000.0f;
    }
    else  // Very far
    {
        SightInterval = 1.0f;  // 1 Hz
        SightRange = 500.0f;
    }
    
    PerceptionComponent->SetSenseUpdateInterval(
        UAISense_Sight::StaticClass(),
        SightInterval
    );
    
    // Update sight config range
    if (UAISenseConfig_Sight* SightConfig = 
        Cast<UAISenseConfig_Sight>(PerceptionComponent->GetSenseConfig(UAISense_Sight::StaticClass())))
    {
        SightConfig->SightRadius = SightRange;
    }
    
    // Hearing LOD
    float HearingInterval = SightInterval * 2.0f;  // Half the frequency of sight
    PerceptionComponent->SetSenseUpdateInterval(
        UAISense_Hearing::StaticClass(),
        HearingInterval
    );
}
```

### 2. Perception Culling

```cpp
bool AHorrorThreatAIController::ShouldUpdatePerception() const
{
    APawn* ControlledPawn = GetPawn();
    if (!ControlledPawn) return false;
    
    // Don't update if not rendered
    if (!ControlledPawn->WasRecentlyRendered(1.0f))
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
    
    // Don't update if in dormant state
    if (Blackboard)
    {
        EAlertState AlertState = static_cast<EAlertState>(
            Blackboard->GetValueAsEnum("AlertState")
        );
        
        if (AlertState == EAlertState::Idle)
        {
            // Reduce updates for idle AI
            return FMath::FRand() < 0.5f;  // 50% chance
        }
    }
    
    return true;
}
```

### 3. Line of Sight Optimization

**Simple LOS Check:**
```cpp
bool AHorrorThreatAIController::HasLineOfSightToFast(AActor* Target) const
{
    if (!Target) return false;
    
    APawn* ControlledPawn = GetPawn();
    if (!ControlledPawn) return false;
    
    // Use simple channel for LOS
    FCollisionQueryParams Params(SCENE_QUERY_STAT(LineOfSight), true, ControlledPawn);
    Params.AddIgnoredActor(Target);
    
    FVector Start = ControlledPawn->GetActorLocation() + FVector(0, 0, 100);
    FVector End = Target->GetActorLocation() + FVector(0, 0, 100);
    
    // Use simple line trace
    return !GetWorld()->LineTraceTestByChannel(
        Start,
        End,
        ECC_Visibility,
        Params
    );
}
```

**Cached LOS Results:**
```cpp
class AHorrorThreatAIController : public AAIController
{
private:
    TMap<AActor*, FCachedLOSResult> LOSCache;
    float LOSCacheLifetime = 0.2f;  // Cache for 0.2s
    
    struct FCachedLOSResult
    {
        bool bHasLOS;
        float Timestamp;
    };
    
public:
    bool HasLineOfSightToCached(AActor* Target)
    {
        if (!Target) return false;
        
        float CurrentTime = GetWorld()->GetTimeSeconds();
        
        // Check cache
        if (LOSCache.Contains(Target))
        {
            FCachedLOSResult& CachedResult = LOSCache[Target];
            
            if (CurrentTime - CachedResult.Timestamp < LOSCacheLifetime)
            {
                return CachedResult.bHasLOS;
            }
        }
        
        // Calculate new result
        bool bHasLOS = HasLineOfSightToFast(Target);
        
        // Update cache
        FCachedLOSResult NewResult;
        NewResult.bHasLOS = bHasLOS;
        NewResult.Timestamp = CurrentTime;
        LOSCache.Add(Target, NewResult);
        
        return bHasLOS;
    }
    
    void CleanLOSCache()
    {
        float CurrentTime = GetWorld()->GetTimeSeconds();
        
        TArray<AActor*> ToRemove;
        for (const auto& Pair : LOSCache)
        {
            if (CurrentTime - Pair.Value.Timestamp > LOSCacheLifetime * 2.0f)
            {
                ToRemove.Add(Pair.Key);
            }
        }
        
        for (AActor* Actor : ToRemove)
        {
            LOSCache.Remove(Actor);
        }
    }
};
```

## Navigation Optimization

### 1. Path Caching

```cpp
class AHorrorThreatAIController : public AAIController
{
private:
    FNavPathSharedPtr CachedPath;
    FVector CachedPathDestination;
    float CachedPathTime = 0.0f;
    float PathCacheLifetime = 1.0f;
    
public:
    bool MoveToLocationCached(FVector Destination, float AcceptanceRadius)
    {
        float CurrentTime = GetWorld()->GetTimeSeconds();
        
        // Check if we can reuse cached path
        if (CachedPath.IsValid() && 
            FVector::Dist(CachedPathDestination, Destination) < 100.0f &&
            CurrentTime - CachedPathTime < PathCacheLifetime)
        {
            // Reuse cached path
            return FollowPath(CachedPath);
        }
        
        // Calculate new path
        FPathFindingQuery Query;
        Query.StartLocation = GetPawn()->GetActorLocation();
        Query.EndLocation = Destination;
        Query.NavData = GetNavData();
        
        FPathFindingResult Result = GetWorld()->GetNavigationSystem()->FindPathSync(Query);
        
        if (Result.IsSuccessful())
        {
            CachedPath = Result.Path;
            CachedPathDestination = Destination;
            CachedPathTime = CurrentTime;
            
            return FollowPath(CachedPath);
        }
        
        return false;
    }
};
```

### 2. Simple Movement for Close Targets

```cpp
void AHorrorThreatAIController::MoveToTargetOptimized(AActor* Target, float AcceptanceRadius)
{
    if (!Target) return;
    
    APawn* ControlledPawn = GetPawn();
    if (!ControlledPawn) return;
    
    float Distance = FVector::Dist(
        ControlledPawn->GetActorLocation(),
        Target->GetActorLocation()
    );
    
    // Use simple movement for close targets
    if (Distance < 500.0f)
    {
        // Direct movement without pathfinding
        FVector Direction = (Target->GetActorLocation() - ControlledPawn->GetActorLocation()).GetSafeNormal();
        
        if (ACharacter* Character = Cast<ACharacter>(ControlledPawn))
        {
            Character->AddMovementInput(Direction, 1.0f);
        }
    }
    else
    {
        // Use pathfinding for distant targets
        MoveToActor(Target, AcceptanceRadius);
    }
}
```

### 3. Reduce Pathfinding Frequency

```cpp
void AHorrorThreatAIController::UpdatePathfindingFrequency(float DistanceToTarget)
{
    float RepathInterval;
    
    if (DistanceToTarget < 500.0f)
    {
        RepathInterval = 0.5f;  // Repath every 0.5s when close
    }
    else if (DistanceToTarget < 2000.0f)
    {
        RepathInterval = 1.0f;  // Repath every 1s at medium distance
    }
    else
    {
        RepathInterval = 2.0f;  // Repath every 2s when far
    }
    
    // Set repath interval
    if (UPathFollowingComponent* PathFollowing = GetPathFollowingComponent())
    {
        // Configure path following
    }
}
```

## Memory Optimization

### 1. Limit Perception Memory

```cpp
void AHorrorThreatAIController::CleanPerceptionMemory()
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    float MaxMemoryAge = 10.0f;  // Remember for 10 seconds
    
    // Clean sound memory
    RecentSounds.RemoveAll([CurrentTime, MaxMemoryAge](const FSoundMemory& Sound)
    {
        return (CurrentTime - Sound.Timestamp) > MaxMemoryAge;
    });
    
    // Clean damage memory
    DamageHistory.RemoveAll([CurrentTime, MaxMemoryAge](const FDamageMemory& Damage)
    {
        return (CurrentTime - Damage.Timestamp) > MaxMemoryAge;
    });
    
    // Clean LOS cache
    CleanLOSCache();
}
```

### 2. AI Pooling

```cpp
class UAIPoolSubsystem : public UWorldSubsystem
{
private:
    TMap<TSubclassOf<AHorrorThreatCharacter>, TArray<AHorrorThreatCharacter*>> AIPool;
    
public:
    AHorrorThreatCharacter* SpawnAI(TSubclassOf<AHorrorThreatCharacter> AIClass, FVector Location)
    {
        // Check pool for available AI
        if (AIPool.Contains(AIClass) && AIPool[AIClass].Num() > 0)
        {
            AHorrorThreatCharacter* AI = AIPool[AIClass].Pop();
            AI->SetActorLocation(Location);
            AI->SetActorHiddenInGame(false);
            AI->SetActorEnableCollision(true);
            AI->ActivateThreat();
            
            return AI;
        }
        
        // Spawn new AI if pool is empty
        AHorrorThreatCharacter* NewAI = GetWorld()->SpawnActor<AHorrorThreatCharacter>(
            AIClass,
            Location,
            FRotator::ZeroRotator
        );
        
        return NewAI;
    }
    
    void ReturnAIToPool(AHorrorThreatCharacter* AI)
    {
        if (!AI) return;
        
        // Deactivate AI
        AI->DeactivateThreat();
        AI->SetActorHiddenInGame(true);
        AI->SetActorEnableCollision(false);
        
        // Add to pool
        TSubclassOf<AHorrorThreatCharacter> AIClass = AI->GetClass();
        if (!AIPool.Contains(AIClass))
        {
            AIPool.Add(AIClass, TArray<AHorrorThreatCharacter*>());
        }
        
        AIPool[AIClass].Add(AI);
    }
};
```

## Profiling and Monitoring

### 1. Performance Counters

```cpp
class AHorrorThreatAIController : public AAIController
{
private:
    // Performance counters
    float TotalBehaviorTreeTime = 0.0f;
    float TotalPerceptionTime = 0.0f;
    float TotalNavigationTime = 0.0f;
    int32 FrameCount = 0;
    
public:
    void LogPerformanceMetrics()
    {
        if (FrameCount > 0)
        {
            float AvgBTTime = TotalBehaviorTreeTime / FrameCount;
            float AvgPerceptionTime = TotalPerceptionTime / FrameCount;
            float AvgNavigationTime = TotalNavigationTime / FrameCount;
            
            UE_LOG(LogAIPerformance, Log, 
                TEXT("AI Performance - BT: %.2fms, Perception: %.2fms, Navigation: %.2fms"),
                AvgBTTime, AvgPerceptionTime, AvgNavigationTime
            );
            
            // Reset counters
            TotalBehaviorTreeTime = 0.0f;
            TotalPerceptionTime = 0.0f;
            TotalNavigationTime = 0.0f;
            FrameCount = 0;
        }
    }
};
```

### 2. Stat Commands

```cpp
// In HorrorProject.cpp
DECLARE_STATS_GROUP(TEXT("HorrorAI"), STATGROUP_HorrorAI, STATCAT_Advanced);

DECLARE_CYCLE_STAT(TEXT("AI Behavior Tree"), STAT_AIBehaviorTree, STATGROUP_HorrorAI);
DECLARE_CYCLE_STAT(TEXT("AI Perception"), STAT_AIPerception, STATGROUP_HorrorAI);
DECLARE_CYCLE_STAT(TEXT("AI Navigation"), STAT_AINavigation, STATGROUP_HorrorAI);

// Usage in code:
void AHorrorThreatAIController::UpdateBehaviorTree()
{
    SCOPE_CYCLE_COUNTER(STAT_AIBehaviorTree);
    
    // Behavior tree update code
}
```

## Optimization Checklist

### High Priority
- [ ] Implement perception LOD system
- [ ] Cache blackboard queries
- [ ] Use squared distance calculations
- [ ] Reduce update frequencies for distant AI
- [ ] Implement simple movement for close targets

### Medium Priority
- [ ] Implement AI pooling
- [ ] Cache pathfinding results
- [ ] Batch blackboard updates
- [ ] Clean perception memory regularly
- [ ] Use visibility-based updates

### Low Priority
- [ ] Implement occlusion culling
- [ ] Optimize debug draw calls
- [ ] Add performance counters
- [ ] Profile with Unreal Insights
- [ ] Optimize material complexity on AI

## Performance Testing

### Test Scenarios
1. 10 AI in combat
2. 20 AI patrolling
3. 50 AI idle
4. Mixed states (10 combat, 20 patrol, 20 idle)
5. Stress test (100+ AI)

### Target Metrics
- 60 FPS with 10 active AI
- 30 FPS with 20 active AI
- No hitches > 100ms
- Memory usage < 500MB for AI

## See Also
- [AIMemoryOptimization.md](AIMemoryOptimization.md) - Memory optimization details
- [AIBehaviorOptimization.md](AIBehaviorOptimization.md) - Behavior-specific optimization
- [AnalyzeAIPerformance.ps1](../Debug/AnalyzeAIPerformance.ps1) - Performance analysis tool
