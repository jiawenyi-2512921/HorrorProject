# AI Memory Optimization Guide

## Overview
Guide for optimizing memory usage in AI systems, covering data structures, caching, pooling, and memory management strategies.

## Memory Budget

### Per-AI Memory Target
- AI Controller: < 10 KB
- Behavior Tree Component: < 5 KB
- Perception Component: < 8 KB
- Blackboard: < 2 KB
- Total per AI: < 25 KB

### System-Wide Targets
- 10 AI: < 250 KB
- 50 AI: < 1.25 MB
- 100 AI: < 2.5 MB

## Data Structure Optimization

### 1. Use Lightweight Structures

**Before (Heavy):**
```cpp
struct FHeavyAIData
{
    TArray<AActor*> PerceivedActors;           // 16 bytes + array data
    TMap<AActor*, float> ActorDistances;       // 80 bytes + map data
    TArray<FVector> PatrolPoints;              // 16 bytes + array data
    TArray<FSoundMemory> RecentSounds;         // 16 bytes + array data
    TArray<FDamageMemory> DamageHistory;       // 16 bytes + array data
    FString DebugInfo;                         // 16 bytes + string data
};
// Total: ~160 bytes + dynamic data
```

**After (Lightweight):**
```cpp
struct FLightweightAIData
{
    AActor* CurrentTarget;                     // 8 bytes
    FVector LastKnownLocation;                 // 12 bytes
    float DistanceToTarget;                    // 4 bytes
    uint8 AlertState;                          // 1 byte
    uint8 CurrentBehaviorState;                // 1 byte
    float StateTimer;                          // 4 bytes
    uint16 Flags;                              // 2 bytes (packed booleans)
};
// Total: 32 bytes
```

### 2. Bit Packing

```cpp
class AHorrorThreatAIController : public AAIController
{
private:
    // Instead of multiple bools (1 byte each)
    // bool bCanSeeTarget;
    // bool bCanHearTarget;
    // bool bTookDamage;
    // bool bIsFleeing;
    // bool bIsInCombat;
    // Total: 5 bytes
    
    // Use bit flags (1 byte total)
    uint8 AIFlags;
    
    enum EAIFlags : uint8
    {
        Flag_CanSeeTarget   = 1 << 0,  // 0x01
        Flag_CanHearTarget  = 1 << 1,  // 0x02
        Flag_TookDamage     = 1 << 2,  // 0x04
        Flag_IsFleeing      = 1 << 3,  // 0x08
        Flag_IsInCombat     = 1 << 4,  // 0x10
    };
    
public:
    FORCEINLINE bool CanSeeTarget() const { return (AIFlags & Flag_CanSeeTarget) != 0; }
    FORCEINLINE void SetCanSeeTarget(bool bValue)
    {
        if (bValue)
            AIFlags |= Flag_CanSeeTarget;
        else
            AIFlags &= ~Flag_CanSeeTarget;
    }
    
    FORCEINLINE bool IsInCombat() const { return (AIFlags & Flag_IsInCombat) != 0; }
    FORCEINLINE void SetIsInCombat(bool bValue)
    {
        if (bValue)
            AIFlags |= Flag_IsInCombat;
        else
            AIFlags &= ~Flag_IsInCombat;
    }
};
```

### 3. Fixed-Size Arrays

```cpp
// Instead of dynamic TArray
// TArray<FSoundMemory> RecentSounds;  // 16 bytes + heap allocation

// Use fixed-size array
static constexpr int32 MaxRecentSounds = 10;
FSoundMemory RecentSounds[MaxRecentSounds];
int32 RecentSoundCount = 0;

void AddSound(const FSoundMemory& Sound)
{
    if (RecentSoundCount < MaxRecentSounds)
    {
        RecentSounds[RecentSoundCount++] = Sound;
    }
    else
    {
        // Shift array and add new sound
        for (int32 i = 0; i < MaxRecentSounds - 1; ++i)
        {
            RecentSounds[i] = RecentSounds[i + 1];
        }
        RecentSounds[MaxRecentSounds - 1] = Sound;
    }
}
```

### 4. Circular Buffers

```cpp
template<typename T, int32 Size>
class TCircularBuffer
{
private:
    T Buffer[Size];
    int32 Head = 0;
    int32 Tail = 0;
    int32 Count = 0;
    
public:
    void Add(const T& Item)
    {
        Buffer[Head] = Item;
        Head = (Head + 1) % Size;
        
        if (Count < Size)
        {
            Count++;
        }
        else
        {
            Tail = (Tail + 1) % Size;
        }
    }
    
    T Get(int32 Index) const
    {
        check(Index < Count);
        return Buffer[(Tail + Index) % Size];
    }
    
    int32 Num() const { return Count; }
    
    void Clear()
    {
        Head = 0;
        Tail = 0;
        Count = 0;
    }
};

// Usage
TCircularBuffer<FSoundMemory, 10> RecentSounds;
```

## Perception Memory Optimization

### 1. Limit Memory Duration

```cpp
void AHorrorThreatAIController::CleanPerceptionMemory()
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    // Clean old sounds (keep last 10 seconds)
    const float SoundMemoryDuration = 10.0f;
    RecentSounds.RemoveAll([CurrentTime, SoundMemoryDuration](const FSoundMemory& Sound)
    {
        return (CurrentTime - Sound.Timestamp) > SoundMemoryDuration;
    });
    
    // Clean old damage (keep last 30 seconds)
    const float DamageMemoryDuration = 30.0f;
    DamageHistory.RemoveAll([CurrentTime, DamageMemoryDuration](const FDamageMemory& Damage)
    {
        return (CurrentTime - Damage.Timestamp) > DamageMemoryDuration;
    });
    
    // Limit array sizes
    const int32 MaxSounds = 20;
    if (RecentSounds.Num() > MaxSounds)
    {
        RecentSounds.RemoveAt(0, RecentSounds.Num() - MaxSounds);
    }
    
    const int32 MaxDamage = 10;
    if (DamageHistory.Num() > MaxDamage)
    {
        DamageHistory.RemoveAt(0, DamageHistory.Num() - MaxDamage);
    }
}
```

### 2. Perception Data Pooling

```cpp
class FPerceptionDataPool
{
private:
    TArray<FSoundMemory*> SoundPool;
    TArray<FDamageMemory*> DamagePool;
    
public:
    FSoundMemory* AllocateSound()
    {
        if (SoundPool.Num() > 0)
        {
            return SoundPool.Pop();
        }
        return new FSoundMemory();
    }
    
    void FreeSound(FSoundMemory* Sound)
    {
        if (Sound)
        {
            SoundPool.Add(Sound);
        }
    }
    
    void Clear()
    {
        for (FSoundMemory* Sound : SoundPool)
        {
            delete Sound;
        }
        SoundPool.Empty();
        
        for (FDamageMemory* Damage : DamagePool)
        {
            delete Damage;
        }
        DamagePool.Empty();
    }
};
```

## Blackboard Optimization

### 1. Minimize Blackboard Keys

```cpp
// Before: Many individual keys
// TargetActor (Object)
// TargetLocation (Vector)
// TargetDistance (Float)
// TargetHealth (Float)
// TargetIsVisible (Bool)
// Total: 5 keys

// After: Consolidated data
struct FTargetData
{
    AActor* Actor;
    FVector Location;
    float Distance;
    float Health;
    bool bIsVisible;
};

// Blackboard: TargetData (Struct)
// Total: 1 key
```

### 2. Use Enums Instead of Strings

```cpp
// Before: String-based state
// Blackboard->SetValueAsString("CurrentState", "Patrolling");

// After: Enum-based state
UENUM(BlueprintType)
enum class EAIState : uint8
{
    Idle,
    Patrolling,
    Investigating,
    Chasing,
    Combat
};

// Blackboard->SetValueAsEnum("CurrentState", static_cast<uint8>(EAIState::Patrolling));
```

### 3. Blackboard Value Caching

```cpp
class FBlackboardCache
{
private:
    TMap<FName, FBlackboardEntry> Cache;
    float LastUpdateTime = 0.0f;
    float CacheLifetime = 0.1f;
    
public:
    template<typename T>
    T GetCachedValue(UBlackboardComponent* Blackboard, FName Key, T DefaultValue)
    {
        float CurrentTime = FPlatformTime::Seconds();
        
        if (CurrentTime - LastUpdateTime > CacheLifetime)
        {
            // Refresh cache
            Cache.Empty();
            LastUpdateTime = CurrentTime;
        }
        
        if (Cache.Contains(Key))
        {
            return Cache[Key].GetValue<T>();
        }
        
        // Get from blackboard and cache
        T Value = Blackboard->GetValueAs<T>(Key);
        Cache.Add(Key, FBlackboardEntry(Value));
        
        return Value;
    }
};
```

## AI Actor Pooling

### 1. Object Pool Implementation

```cpp
class UAIActorPool : public UObject
{
private:
    TMap<TSubclassOf<AActor>, TArray<AActor*>> Pool;
    int32 MaxPoolSize = 50;
    
public:
    AActor* Acquire(TSubclassOf<AActor> ActorClass, UWorld* World)
    {
        if (Pool.Contains(ActorClass) && Pool[ActorClass].Num() > 0)
        {
            AActor* Actor = Pool[ActorClass].Pop();
            Actor->SetActorHiddenInGame(false);
            Actor->SetActorEnableCollision(true);
            Actor->SetActorTickEnabled(true);
            return Actor;
        }
        
        // Spawn new actor
        return World->SpawnActor<AActor>(ActorClass);
    }
    
    void Release(AActor* Actor)
    {
        if (!Actor) return;
        
        TSubclassOf<AActor> ActorClass = Actor->GetClass();
        
        // Check pool size limit
        if (!Pool.Contains(ActorClass))
        {
            Pool.Add(ActorClass, TArray<AActor*>());
        }
        
        if (Pool[ActorClass].Num() < MaxPoolSize)
        {
            // Reset actor state
            Actor->SetActorHiddenInGame(true);
            Actor->SetActorEnableCollision(false);
            Actor->SetActorTickEnabled(false);
            
            // Add to pool
            Pool[ActorClass].Add(Actor);
        }
        else
        {
            // Pool is full, destroy actor
            Actor->Destroy();
        }
    }
    
    void Clear()
    {
        for (auto& Pair : Pool)
        {
            for (AActor* Actor : Pair.Value)
            {
                if (Actor)
                {
                    Actor->Destroy();
                }
            }
        }
        Pool.Empty();
    }
};
```

### 2. Component Pooling

```cpp
class UComponentPool : public UObject
{
private:
    TMap<TSubclassOf<UActorComponent>, TArray<UActorComponent*>> Pool;
    
public:
    template<typename T>
    T* AcquireComponent(AActor* Owner)
    {
        TSubclassOf<UActorComponent> ComponentClass = T::StaticClass();
        
        if (Pool.Contains(ComponentClass) && Pool[ComponentClass].Num() > 0)
        {
            T* Component = Cast<T>(Pool[ComponentClass].Pop());
            Component->SetActive(true);
            return Component;
        }
        
        // Create new component
        T* NewComponent = NewObject<T>(Owner);
        NewComponent->RegisterComponent();
        return NewComponent;
    }
    
    void ReleaseComponent(UActorComponent* Component)
    {
        if (!Component) return;
        
        Component->SetActive(false);
        
        TSubclassOf<UActorComponent> ComponentClass = Component->GetClass();
        if (!Pool.Contains(ComponentClass))
        {
            Pool.Add(ComponentClass, TArray<UActorComponent*>());
        }
        
        Pool[ComponentClass].Add(Component);
    }
};
```

## Memory Profiling

### 1. Memory Tracking

```cpp
class FAIMemoryTracker
{
private:
    struct FMemorySnapshot
    {
        int32 AICount;
        SIZE_T TotalMemory;
        SIZE_T PerAIMemory;
        float Timestamp;
    };
    
    TArray<FMemorySnapshot> Snapshots;
    
public:
    void TakeSnapshot(int32 AICount)
    {
        FMemorySnapshot Snapshot;
        Snapshot.AICount = AICount;
        Snapshot.TotalMemory = GetAIMemoryUsage();
        Snapshot.PerAIMemory = AICount > 0 ? Snapshot.TotalMemory / AICount : 0;
        Snapshot.Timestamp = FPlatformTime::Seconds();
        
        Snapshots.Add(Snapshot);
    }
    
    SIZE_T GetAIMemoryUsage() const
    {
        // Calculate total AI memory usage
        SIZE_T TotalMemory = 0;
        
        // Add controller memory
        // Add component memory
        // Add blackboard memory
        // etc.
        
        return TotalMemory;
    }
    
    void LogMemoryReport() const
    {
        if (Snapshots.Num() == 0) return;
        
        const FMemorySnapshot& Latest = Snapshots.Last();
        
        UE_LOG(LogTemp, Log, TEXT("AI Memory Report:"));
        UE_LOG(LogTemp, Log, TEXT("  AI Count: %d"), Latest.AICount);
        UE_LOG(LogTemp, Log, TEXT("  Total Memory: %.2f KB"), Latest.TotalMemory / 1024.0f);
        UE_LOG(LogTemp, Log, TEXT("  Per-AI Memory: %.2f KB"), Latest.PerAIMemory / 1024.0f);
    }
};
```

### 2. Memory Budget Enforcement

```cpp
class FAIMemoryBudget
{
private:
    SIZE_T MaxTotalMemory = 10 * 1024 * 1024;  // 10 MB
    SIZE_T MaxPerAIMemory = 50 * 1024;         // 50 KB
    
public:
    bool CanSpawnAI(int32 CurrentAICount) const
    {
        SIZE_T CurrentMemory = GetCurrentAIMemory();
        SIZE_T ProjectedMemory = CurrentMemory + MaxPerAIMemory;
        
        return ProjectedMemory <= MaxTotalMemory;
    }
    
    void EnforceMemoryBudget()
    {
        SIZE_T CurrentMemory = GetCurrentAIMemory();
        
        if (CurrentMemory > MaxTotalMemory)
        {
            UE_LOG(LogTemp, Warning, TEXT("AI memory budget exceeded: %.2f MB / %.2f MB"),
                CurrentMemory / (1024.0f * 1024.0f),
                MaxTotalMemory / (1024.0f * 1024.0f)
            );
            
            // Take action: despawn distant AI, reduce memory usage, etc.
            ReduceMemoryUsage();
        }
    }
    
    void ReduceMemoryUsage()
    {
        // Clean perception memory
        // Despawn distant AI
        // Clear caches
        // etc.
    }
};
```

## String Optimization

### 1. Avoid String Allocations

```cpp
// Before: String allocations
FString DebugMessage = FString::Printf(TEXT("AI State: %s, Distance: %.2f"), 
    *StateName, Distance);

// After: Use FName or enum
FName StateName = GetStateName();  // No allocation
float Distance = GetDistance();

// Or use static strings
static const FString IdleState = TEXT("Idle");
static const FString PatrolState = TEXT("Patrol");
```

### 2. String Pooling

```cpp
class FStringPool
{
private:
    TMap<FString, FName> StringToName;
    
public:
    FName GetOrCreateName(const FString& String)
    {
        if (StringToName.Contains(String))
        {
            return StringToName[String];
        }
        
        FName Name(*String);
        StringToName.Add(String, Name);
        return Name;
    }
};
```

## Garbage Collection Optimization

### 1. Reduce GC Pressure

```cpp
// Use UPROPERTY() for all UObject references
UPROPERTY()
AActor* TargetActor;  // Properly tracked by GC

// Not: AActor* TargetActor;  // Can cause crashes

// Use TWeakObjectPtr for optional references
TWeakObjectPtr<AActor> OptionalTarget;

// Use raw pointers only for non-UObject types
float* RawFloatPtr;
```

### 2. Object Lifetime Management

```cpp
class AHorrorThreatAIController : public AAIController
{
private:
    // Use UPROPERTY for GC tracking
    UPROPERTY()
    TArray<AActor*> TrackedActors;
    
    // Use TWeakObjectPtr for weak references
    TArray<TWeakObjectPtr<AActor>> WeakReferences;
    
public:
    void CleanupInvalidReferences()
    {
        // Remove null weak references
        WeakReferences.RemoveAll([](const TWeakObjectPtr<AActor>& Ref)
        {
            return !Ref.IsValid();
        });
        
        // Remove null strong references
        TrackedActors.RemoveAll([](AActor* Actor)
        {
            return Actor == nullptr || !IsValid(Actor);
        });
    }
};
```

## Memory Optimization Checklist

### High Priority
- [ ] Use bit packing for boolean flags
- [ ] Implement AI actor pooling
- [ ] Limit perception memory duration
- [ ] Use fixed-size arrays where possible
- [ ] Minimize blackboard keys

### Medium Priority
- [ ] Implement component pooling
- [ ] Use circular buffers for history
- [ ] Cache blackboard values
- [ ] Avoid string allocations
- [ ] Clean up invalid references regularly

### Low Priority
- [ ] Implement memory tracking
- [ ] Enforce memory budgets
- [ ] Use string pooling
- [ ] Optimize struct layouts
- [ ] Profile memory usage

## Memory Profiling Tools

### Unreal Engine Tools
```
stat memory          - Show memory statistics
stat memoryplatform  - Platform-specific memory stats
memreport            - Generate memory report
obj list             - List all objects
obj dump             - Dump object details
```

### External Tools
- Visual Studio Memory Profiler
- Intel VTune
- Windows Performance Analyzer
- Valgrind (Linux)

## Testing

### Memory Test Scenarios
1. Spawn 10 AI and measure memory
2. Spawn 50 AI and measure memory
3. Spawn 100 AI and measure memory
4. Run for 1 hour and check for leaks
5. Stress test with rapid spawn/despawn

### Target Metrics
- No memory leaks
- < 25 KB per AI
- < 2.5 MB for 100 AI
- Stable memory usage over time

## See Also
- [AIPerformanceOptimization.md](AIPerformanceOptimization.md) - Performance optimization
- [AIBehaviorOptimization.md](AIBehaviorOptimization.md) - Behavior optimization
- [AnalyzeAIPerformance.ps1](../Debug/AnalyzeAIPerformance.ps1) - Performance analysis tool
