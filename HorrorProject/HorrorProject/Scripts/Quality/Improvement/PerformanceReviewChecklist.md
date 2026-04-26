# Performance Review Checklist

## Overview
Performance-focused checklist for code reviews to identify and prevent performance issues.

**Version**: 1.0
**Last Updated**: 2026-04-26
**Target**: 60 FPS on target hardware

---

## General Performance

### Algorithm Efficiency
- [ ] Appropriate algorithm complexity (O(n) vs O(n²))
- [ ] No unnecessary nested loops
- [ ] Efficient data structures used
- [ ] Early exit conditions implemented
- [ ] Caching used where beneficial
- [ ] Lazy evaluation considered

### Memory Efficiency
- [ ] No excessive memory allocations
- [ ] Memory pooling used where appropriate
- [ ] Large objects passed by reference
- [ ] No memory leaks
- [ ] Proper memory alignment
- [ ] Memory fragmentation considered

### CPU Optimization
- [ ] Hot paths identified and optimized
- [ ] Branch prediction friendly code
- [ ] Cache-friendly data structures
- [ ] SIMD opportunities identified
- [ ] Parallelization considered
- [ ] Lock contention minimized

---

## Unreal Engine Specific

### Tick Functions
- [ ] Tick disabled when not needed
- [ ] Tick interval appropriate
- [ ] No expensive operations in Tick
- [ ] Timers used instead of Tick where possible
- [ ] Tick dependencies minimized
- [ ] Tick group appropriate

**Red Flags**:
- ❌ Object creation in Tick
- ❌ World queries in Tick
- ❌ File I/O in Tick
- ❌ Complex calculations every frame
- ❌ String operations in Tick

### Component Management
- [ ] Components cached in BeginPlay
- [ ] GetComponentByClass avoided in hot paths
- [ ] Component references stored
- [ ] Unnecessary components removed
- [ ] Component tick disabled when not needed

### Actor Management
- [ ] Actor pooling used for frequently spawned actors
- [ ] GetActorOfClass avoided in hot paths
- [ ] Actor references cached
- [ ] Dormancy used for inactive actors
- [ ] Actor destruction batched

### Rendering
- [ ] Draw calls minimized
- [ ] Material complexity appropriate
- [ ] LODs implemented
- [ ] Occlusion culling used
- [ ] Instancing used where possible
- [ ] Texture sizes appropriate
- [ ] Overdraw minimized

### Physics
- [ ] Physics simulation complexity appropriate
- [ ] Collision complexity optimized
- [ ] Physics sub-stepping configured
- [ ] Sleeping objects handled
- [ ] Physics queries optimized
- [ ] Collision channels used efficiently

### Animation
- [ ] Animation blueprints optimized
- [ ] Unnecessary bone updates avoided
- [ ] Animation LODs used
- [ ] Root motion efficient
- [ ] Blend spaces optimized
- [ ] Animation notifies minimal

### Audio
- [ ] Audio occlusion efficient
- [ ] Sound attenuation appropriate
- [ ] Audio pooling used
- [ ] Unnecessary audio stopped
- [ ] Audio priority system used
- [ ] 3D audio calculations optimized

---

## Memory Management

### Allocations
- [ ] No allocations in hot paths
- [ ] TArray::Reserve used when size known
- [ ] Object pooling for frequent allocations
- [ ] Stack allocation preferred over heap
- [ ] Temporary allocations minimized
- [ ] Memory arenas considered

### Garbage Collection
- [ ] UPROPERTY used for UObject references
- [ ] No circular references
- [ ] Weak pointers used appropriately
- [ ] GC-friendly object lifetimes
- [ ] Force GC avoided in gameplay
- [ ] GC optimization settings appropriate

### Smart Pointers
- [ ] TSharedPtr used appropriately
- [ ] TWeakPtr for optional references
- [ ] TUniquePtr for exclusive ownership
- [ ] Circular references avoided
- [ ] Thread-safe variants used correctly

---

## Data Structures

### Arrays
- [ ] TArray::Reserve called when size known
- [ ] Appropriate growth strategy
- [ ] RemoveSwap used when order doesn't matter
- [ ] Range-based for with const& for large elements
- [ ] Bulk operations used (Append, AddUnique)

### Maps & Sets
- [ ] TMap used for lookups instead of linear search
- [ ] TSet used for uniqueness checks
- [ ] Appropriate hash functions
- [ ] Reserve called for known sizes
- [ ] Key types efficient

### Strings
- [ ] FName used for identifiers
- [ ] FText used for localized text
- [ ] FString used for dynamic text
- [ ] String concatenation minimized
- [ ] FString::Printf avoided in loops
- [ ] String comparisons optimized

---

## Function Optimization

### Function Calls
- [ ] Inline used for small, frequently called functions
- [ ] Virtual function overhead considered
- [ ] Function call depth reasonable
- [ ] Recursion depth limited
- [ ] Tail recursion optimized

### Parameters
- [ ] Large objects passed by const reference
- [ ] Move semantics used appropriately
- [ ] Output parameters used for multiple returns
- [ ] Default parameters used wisely
- [ ] Parameter count reasonable (< 5)

### Return Values
- [ ] Return value optimization (RVO) enabled
- [ ] Move semantics for large returns
- [ ] Optional/Result types for errors
- [ ] Avoid returning by value for large objects

---

## Loop Optimization

### Loop Structure
- [ ] Loop invariants moved outside
- [ ] Loop unrolling considered
- [ ] Early exit conditions used
- [ ] Nested loops minimized
- [ ] Loop fusion applied where possible

### Iteration
- [ ] Range-based for with const& for read-only
- [ ] Index-based for when index needed
- [ ] Iterator invalidation avoided
- [ ] Parallel iteration considered
- [ ] SIMD opportunities identified

### Common Patterns
```cpp
// ❌ Bad: Copies each element
for (auto Item : LargeArray) { }

// ✅ Good: No copy, read-only
for (const auto& Item : LargeArray) { }

// ❌ Bad: Repeated function call
for (int i = 0; i < Array.Num(); i++) { }

// ✅ Good: Cache size
const int32 Count = Array.Num();
for (int i = 0; i < Count; i++) { }

// ❌ Bad: Nested linear search
for (auto& A : ArrayA) {
    for (auto& B : ArrayB) {
        if (A.ID == B.ID) { }
    }
}

// ✅ Good: Use map for O(1) lookup
TMap<int32, FItemB> MapB;
for (auto& B : ArrayB) {
    MapB.Add(B.ID, B);
}
for (auto& A : ArrayA) {
    if (FItemB* B = MapB.Find(A.ID)) { }
}
```

---

## Math Optimization

### Vector Math
- [ ] SizeSquared used instead of Size when possible
- [ ] Dot product used for angle checks
- [ ] Normalize avoided when not needed
- [ ] Vector operations batched
- [ ] SIMD intrinsics considered

### Trigonometry
- [ ] Lookup tables for repeated calculations
- [ ] Approximations used where acceptable
- [ ] Expensive functions (sin, cos, sqrt) minimized
- [ ] Angle comparisons use dot product
- [ ] Radians vs degrees conversions minimized

### Floating Point
- [ ] Epsilon comparisons for equality
- [ ] Integer math used when possible
- [ ] Division replaced with multiplication
- [ ] Reciprocal square root used
- [ ] Denormal numbers handled

---

## Multithreading

### Thread Safety
- [ ] Shared data protected
- [ ] Lock-free algorithms considered
- [ ] Atomic operations used appropriately
- [ ] Thread-local storage used
- [ ] Race conditions prevented

### Parallelization
- [ ] ParallelFor used for independent iterations
- [ ] Task graph utilized
- [ ] Work distribution balanced
- [ ] Thread pool used
- [ ] Synchronization minimized

### Unreal Threading
- [ ] Game thread work minimized
- [ ] Render thread synchronization avoided
- [ ] Async tasks used appropriately
- [ ] Thread-safe delegates used
- [ ] FRunnable for long-running tasks

---

## I/O Operations

### File I/O
- [ ] Async loading used
- [ ] Streaming used for large files
- [ ] File operations off game thread
- [ ] Batch file operations
- [ ] File caching implemented

### Network I/O
- [ ] Async networking used
- [ ] Network calls batched
- [ ] Timeouts configured
- [ ] Bandwidth usage optimized
- [ ] Compression used

### Asset Loading
- [ ] Async asset loading used
- [ ] Asset streaming configured
- [ ] Soft references for optional assets
- [ ] Asset bundles used
- [ ] Preloading for known assets

---

## Profiling & Measurement

### Profiling Tools Used
- [ ] Unreal Insights profiling done
- [ ] CPU profiling performed
- [ ] Memory profiling performed
- [ ] GPU profiling performed
- [ ] Network profiling done (if applicable)

### Stat Commands
- [ ] stat fps checked
- [ ] stat unit analyzed
- [ ] stat game reviewed
- [ ] stat memory checked
- [ ] stat slow identified

### Benchmarking
- [ ] Performance benchmarks run
- [ ] Before/after comparison done
- [ ] Target hardware tested
- [ ] Worst-case scenarios tested
- [ ] Performance regression checked

---

## Common Performance Anti-Patterns

### ❌ Anti-Pattern 1: Tick Abuse
```cpp
// Bad: Expensive operation every frame
void UMyComponent::Tick(float DeltaTime) {
    AActor* Player = UGameplayStatics::GetPlayerPawn(this, 0);
    // Searches world every frame
}

// Good: Cache reference
void UMyComponent::BeginPlay() {
    CachedPlayer = UGameplayStatics::GetPlayerPawn(this, 0);
}
```

### ❌ Anti-Pattern 2: Repeated Lookups
```cpp
// Bad: O(n) lookup in loop
for (auto& Item : Items) {
    AActor* Found = FindActorByID(Item.ActorID);
}

// Good: Build map once, O(1) lookups
TMap<int32, AActor*> ActorMap;
for (auto& Actor : AllActors) {
    ActorMap.Add(Actor->ID, Actor);
}
for (auto& Item : Items) {
    AActor* Found = ActorMap.FindRef(Item.ActorID);
}
```

### ❌ Anti-Pattern 3: String Operations in Loops
```cpp
// Bad: Allocations per iteration
FString Result;
for (const auto& Item : Items) {
    Result += Item.ToString() + TEXT(", ");
}

// Good: Use Join
TArray<FString> Parts;
Parts.Reserve(Items.Num());
for (const auto& Item : Items) {
    Parts.Add(Item.ToString());
}
FString Result = FString::Join(Parts, TEXT(", "));
```

### ❌ Anti-Pattern 4: Unnecessary Copies
```cpp
// Bad: Copies entire array
void ProcessData(TArray<FLargeStruct> Data) { }

// Good: Pass by const reference
void ProcessData(const TArray<FLargeStruct>& Data) { }
```

### ❌ Anti-Pattern 5: Object Creation in Hot Paths
```cpp
// Bad: Spawns actor every frame
void UMyComponent::Tick(float DeltaTime) {
    AActor* Projectile = GetWorld()->SpawnActor<AProjectile>();
}

// Good: Object pooling
class UProjectilePool {
    TArray<AProjectile*> AvailableProjectiles;
    
    AProjectile* GetProjectile() {
        if (AvailableProjectiles.Num() > 0) {
            return AvailableProjectiles.Pop();
        }
        return SpawnNewProjectile();
    }
    
    void ReturnProjectile(AProjectile* Projectile) {
        Projectile->SetActorHiddenInGame(true);
        AvailableProjectiles.Add(Projectile);
    }
};
```

---

## Performance Budgets

### Frame Time Budget (60 FPS = 16.67ms)
- Game Thread: < 10ms
- Render Thread: < 10ms
- GPU: < 16ms
- Physics: < 2ms
- Animation: < 2ms
- Audio: < 1ms

### Memory Budget
- Total Memory: < 4GB (console target)
- Texture Memory: < 1GB
- Mesh Memory: < 500MB
- Audio Memory: < 200MB
- Code/Data: < 500MB

### Network Budget
- Bandwidth: < 100 KB/s per player
- Packet Rate: < 30 packets/s
- Latency: < 100ms acceptable
- Jitter: < 20ms

---

## Performance Testing

### Test Scenarios
- [ ] Worst-case scenario tested
- [ ] Maximum player count tested
- [ ] Stress test performed
- [ ] Long-running stability tested
- [ ] Memory leak testing done

### Performance Metrics
- [ ] Frame rate measured
- [ ] Frame time analyzed
- [ ] Memory usage tracked
- [ ] Load times measured
- [ ] Network performance tested

### Regression Testing
- [ ] Performance baseline established
- [ ] Automated performance tests
- [ ] Performance CI/CD integration
- [ ] Performance alerts configured

---

## Optimization Priority

### Critical (P0) - Fix Immediately
- Frame rate < 30 FPS
- Memory leaks
- Crashes due to performance
- Unplayable sections
- Shipping blockers

### High (P1) - Fix This Sprint
- Frame rate 30-45 FPS
- Noticeable hitches
- High memory usage
- Long load times
- Multiplayer lag

### Medium (P2) - Fix Next Sprint
- Frame rate 45-55 FPS
- Minor hitches
- Moderate memory usage
- Optimization opportunities
- Code quality issues

### Low (P3) - Nice to Have
- Frame rate 55-60 FPS
- Micro-optimizations
- Future-proofing
- Code cleanup
- Documentation

---

## Performance Review Process

### Before Review
- [ ] Profile the code
- [ ] Identify hot paths
- [ ] Measure baseline performance
- [ ] Review performance requirements

### During Review
- [ ] Check for common anti-patterns
- [ ] Verify algorithm complexity
- [ ] Review memory allocations
- [ ] Check loop efficiency
- [ ] Verify caching strategies

### After Review
- [ ] Document performance characteristics
- [ ] Set performance budgets
- [ ] Create optimization tasks
- [ ] Schedule follow-up profiling

---

## Performance Checklist by System

### AI System
- [ ] Behavior tree complexity reasonable
- [ ] Perception updates optimized
- [ ] Pathfinding async
- [ ] AI tick rate appropriate
- [ ] LOD system for AI

### Physics System
- [ ] Collision meshes simplified
- [ ] Physics materials optimized
- [ ] Constraint count reasonable
- [ ] Physics sub-stepping configured
- [ ] Sleeping objects handled

### Rendering System
- [ ] Draw calls < 2000
- [ ] Triangles < 2M visible
- [ ] Material instructions < 300
- [ ] Texture memory < budget
- [ ] Overdraw < 2x

### Animation System
- [ ] Bone count < 150
- [ ] Animation blueprint optimized
- [ ] Blend node count reasonable
- [ ] Animation LODs used
- [ ] Update rate appropriate

---

## Tools & Commands

### Unreal Insights
```
UnrealInsights.exe
```

### Stat Commands
```
stat fps          - Frame rate
stat unit         - Frame time breakdown
stat game         - Game thread stats
stat slow         - Slow operations
stat memory       - Memory usage
stat streaming    - Asset streaming
stat audio        - Audio stats
stat particles    - Particle stats
```

### Console Commands
```
t.MaxFPS 60                    - Cap frame rate
r.ScreenPercentage 100         - Resolution scale
r.VSync 0                      - Disable VSync
stat startfile                 - Start stat capture
stat stopfile                  - Stop stat capture
Obj List Class=Actor           - List all actors
Obj Dump ClassName             - Dump object info
```

### Profiling Scopes
```cpp
SCOPE_CYCLE_COUNTER(STAT_MyFunction);
QUICK_SCOPE_CYCLE_COUNTER(STAT_MyQuickFunction);
```

---

## Resources

- [Unreal Engine Performance Guidelines](https://docs.unrealengine.com/5.3/en-US/performance-guidelines-for-unreal-engine/)
- [Unreal Insights](https://docs.unrealengine.com/5.3/en-US/unreal-insights-in-unreal-engine/)
- [Optimization Techniques](https://www.unrealengine.com/en-US/tech-blog)
- [C++ Performance Tips](https://www.agner.org/optimize/)

---

## Performance Review Sign-Off

**Reviewer**: _______________
**Date**: _______________
**Performance Issues Found**: Critical: ___ High: ___ Medium: ___ Low: ___
**Frame Rate**: ___ FPS (Target: 60 FPS)
**Memory Usage**: ___ MB (Budget: ___ MB)
**Recommendation**: ☐ Approve ☐ Optimize ☐ Reject

**Notes**:
_______________________________________________
_______________________________________________

---

**Remember**: Profile first, optimize second. Don't optimize prematurely, but design with performance in mind.
