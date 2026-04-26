# Code Smells - HorrorProject

Generated: 2026-04-26
Identification and remediation guide for code smells

## What Are Code Smells?

Code smells are indicators of potential problems in code. They're not bugs, but suggest deeper issues that may lead to bugs or maintenance difficulties.

**Current Status:** Overall code quality is GOOD, but several smells detected.

---

## Detected Code Smells

### 1. Insufficient Documentation (CRITICAL)

**Smell Type:** Comments
**Severity:** Critical
**Occurrences:** 164 files
**Impact:** High maintenance cost, difficult onboarding

#### Symptoms
- Comment ratio: 3.1% (industry standard: 20-30%)
- Missing file headers
- Undocumented public APIs
- No usage examples

#### Example
```cpp
// SMELL - No documentation
class UHorrorAudioSubsystem : public UGameInstanceSubsystem
{
    UFUNCTION(BlueprintCallable)
    void PlayAmbientSound(FGameplayTag ZoneTag);
};

// FRESH - Well documented
/**
 * Manages audio playback for horror atmosphere and events.
 * Handles zone-based ambient audio, dynamic music, and spatial sound effects.
 * 
 * Usage:
 *   UHorrorAudioSubsystem* Audio = GetGameInstance()->GetSubsystem<UHorrorAudioSubsystem>();
 *   Audio->PlayAmbientSound(FGameplayTag::RequestGameplayTag("Audio.Zone.Basement"));
 */
class UHorrorAudioSubsystem : public UGameInstanceSubsystem
{
    /**
     * Plays ambient sound for the specified zone.
     * @param ZoneTag Gameplay tag identifying the audio zone (e.g., "Audio.Zone.Basement")
     */
    UFUNCTION(BlueprintCallable, Category = "Horror|Audio")
    void PlayAmbientSound(FGameplayTag ZoneTag);
};
```

#### Remediation
1. Add file-level documentation to every source file
2. Document all UFUNCTION(BlueprintCallable) with purpose, parameters, returns
3. Include usage examples for complex systems
4. Generate API docs with Doxygen

---

### 2. Magic Numbers (HIGH FREQUENCY)

**Smell Type:** Maintainability
**Severity:** Medium
**Occurrences:** 123 instances
**Impact:** Unclear intent, difficult tuning

#### Symptoms
- Hardcoded numeric literals
- Repeated values across files
- Unclear meaning
- Difficult to tune

#### Example
```cpp
// SMELL - Magic numbers everywhere
if (Distance < 800.0f)
{
    Speed = 600.0f;
    if (Health < 50.0f)
    {
        Damage = 25.0f;
    }
}

// FRESH - Named constants
static constexpr float ThreatDetectionRange = 800.0f;
static constexpr float ThreatChaseSpeed = 600.0f;
static constexpr float LowHealthThreshold = 50.0f;
static constexpr float CriticalDamage = 25.0f;

if (Distance < ThreatDetectionRange)
{
    Speed = ThreatChaseSpeed;
    if (Health < LowHealthThreshold)
    {
        Damage = CriticalDamage;
    }
}

// BEST - Designer-tunable properties
UPROPERTY(EditDefaultsOnly, Category = "AI|Detection", meta = (ClampMin = "0.0"))
float DetectionRange = 800.0f;

UPROPERTY(EditDefaultsOnly, Category = "AI|Movement", meta = (ClampMin = "0.0"))
float ChaseSpeed = 600.0f;
```

#### Remediation
1. Replace all magic numbers with named constants
2. Use UPROPERTY for designer-tunable values
3. Group related constants in dedicated headers
4. Document constant meanings

---

### 3. Unsafe Pointer Operations (CRITICAL)

**Smell Type:** Error Handling
**Severity:** Critical
**Occurrences:** 49 instances
**Impact:** Potential crashes, undefined behavior

#### Symptoms
- Cast<> without null check
- Chained calls without validation
- Missing ensure() or check()
- Array access without bounds check

#### Example
```cpp
// SMELL - Unsafe cast
AActor* Target = Cast<AActor>(Object);
Target->SetActorLocation(NewLocation);  // CRASH if cast fails!

// FRESH - Safe with validation
if (AActor* Target = Cast<AActor>(Object))
{
    Target->SetActorLocation(NewLocation);
}

// SMELL - Unsafe chained call
GetWorld()->GetFirstPlayerController()->GetPawn()->SetActorLocation(Loc);

// FRESH - Safe with validation
UWorld* World = GetWorld();
if (ensure(World))
{
    APlayerController* PC = World->GetFirstPlayerController();
    if (ensure(PC))
    {
        APawn* Pawn = PC->GetPawn();
        if (Pawn)
        {
            Pawn->SetActorLocation(Loc);
        }
    }
}
```

#### Remediation
1. Always check Cast<> results before use
2. Use ensure() for expected-valid pointers
3. Use check() for must-be-valid pointers
4. Validate all GetWorld()/GetOwner() calls

---

### 4. Non-Const Static Variables (THREAD SAFETY)

**Smell Type:** Thread Safety
**Severity:** Medium
**Occurrences:** 73 instances
**Impact:** Race conditions, data corruption

#### Symptoms
- Static variables without const
- Mutable shared state
- No synchronization
- Potential race conditions

#### Example
```cpp
// SMELL - Non-const static (thread-unsafe)
class UHorrorAudioLibrary
{
    static FName CurrentZone;  // Mutable shared state!
    static float GlobalVolume;
};

// FRESH - Const static (thread-safe)
class UHorrorAudioLibrary
{
    static const FName DefaultZone;
    static constexpr float DefaultVolume = 1.0f;
};

// SMELL - Unprotected container modification
TArray<FEvidence> Evidence;
void AddEvidence(const FEvidence& NewEvidence)
{
    Evidence.Add(NewEvidence);  // Race condition!
}

// FRESH - Protected with lock
TArray<FEvidence> Evidence;
FCriticalSection EvidenceLock;

void AddEvidence(const FEvidence& NewEvidence)
{
    FScopeLock Lock(&EvidenceLock);
    Evidence.Add(NewEvidence);
}
```

#### Remediation
1. Make static variables const where possible
2. Add FScopeLock for shared container access
3. Use TAtomic for counters
4. Document thread safety guarantees

---

### 5. Pass-By-Value for Large Types (PERFORMANCE)

**Smell Type:** Performance
**Severity:** Low
**Occurrences:** 68 instances
**Impact:** Unnecessary copies, memory allocations

#### Symptoms
- Passing structs by value
- Passing FString/FName by value
- Passing containers by value
- Unnecessary copies

#### Example
```cpp
// SMELL - Pass by value (copies entire struct)
void ProcessTag(FGameplayTag Tag)
{
    // Tag is copied on every call
}

void UpdateEvidence(TArray<FEvidence> Evidence)
{
    // Entire array copied!
}

// FRESH - Pass by const reference (no copy)
void ProcessTag(const FGameplayTag& Tag)
{
    // No copy, just a reference
}

void UpdateEvidence(const TArray<FEvidence>& Evidence)
{
    // No copy, just a reference
}

// For output parameters, use pointer
void GetEvidence(TArray<FEvidence>* OutEvidence)
{
    check(OutEvidence);
    *OutEvidence = CollectedEvidence;
}
```

#### Remediation
1. Pass large types by const reference for input
2. Use pointers for output parameters
3. Reserve "pass by value" for primitives (int, float, bool)
4. Use move semantics for transfers

---

### 6. GetWorld() in Tick (PERFORMANCE)

**Smell Type:** Performance
**Severity:** Medium
**Occurrences:** 8 instances
**Impact:** Unnecessary function calls every frame

#### Symptoms
- GetWorld() called multiple times in Tick
- GetOwner() called repeatedly
- Expensive lookups in hot paths

#### Example
```cpp
// SMELL - GetWorld() every frame
void UMyComponent::Tick(float DeltaTime)
{
    float Time = GetWorld()->GetTimeSeconds();
    GetWorld()->SpawnActor(...);
    GetWorld()->LineTraceSingleByChannel(...);
}

// FRESH - Cache World pointer
void UMyComponent::Tick(float DeltaTime)
{
    UWorld* World = GetWorld();
    if (!World) return;
    
    float Time = World->GetTimeSeconds();
    World->SpawnActor(...);
    World->LineTraceSingleByChannel(...);
}

// BEST - Cache in BeginPlay
void UMyComponent::BeginPlay()
{
    Super::BeginPlay();
    CachedWorld = GetWorld();
}

void UMyComponent::Tick(float DeltaTime)
{
    if (!CachedWorld) return;
    
    float Time = CachedWorld->GetTimeSeconds();
    // Use CachedWorld...
}
```

#### Remediation
1. Cache GetWorld() result in BeginPlay
2. Cache GetOwner() result
3. Avoid repeated lookups in Tick
4. Profile hot paths

---

### 7. Missing Blueprint Categories (API DESIGN)

**Smell Type:** API Design
**Severity:** Low
**Occurrences:** Multiple files
**Impact:** Poor Blueprint organization

#### Symptoms
- BlueprintCallable without Category
- Inconsistent category naming
- Poor Blueprint UX

#### Example
```cpp
// SMELL - No category
UFUNCTION(BlueprintCallable)
void PlaySound();

UFUNCTION(BlueprintCallable)
void StopSound();

// FRESH - Organized with categories
UFUNCTION(BlueprintCallable, Category = "Horror|Audio|Playback")
void PlaySound();

UFUNCTION(BlueprintCallable, Category = "Horror|Audio|Playback")
void StopSound();

UFUNCTION(BlueprintCallable, Category = "Horror|Audio|Volume")
void SetVolume(float Volume);
```

#### Remediation
1. Add Category to all BlueprintCallable functions
2. Use consistent category hierarchy
3. Group related functions together
4. Follow pattern: "Game|System|Subsystem"

---

### 8. Long Functions (COMPLEXITY)

**Smell Type:** Complexity
**Severity:** Low
**Occurrences:** 5 medium-risk functions
**Impact:** Difficult to understand and test

#### Symptoms
- Functions > 50 lines
- Cyclomatic complexity > 10
- Multiple responsibilities
- Difficult to test

#### Example
```cpp
// SMELL - Long function with multiple responsibilities
void ProcessObjective(FObjective Objective)
{
    // 80 lines of code
    // Validation
    // State updates
    // Event broadcasting
    // UI updates
    // Audio playback
    // Achievement checking
}

// FRESH - Extracted into focused functions
void ProcessObjective(FObjective Objective)
{
    if (!ValidateObjective(Objective)) return;
    
    UpdateObjectiveState(Objective);
    BroadcastObjectiveEvents(Objective);
    UpdateObjectiveUI(Objective);
    PlayObjectiveAudio(Objective);
    CheckObjectiveAchievements(Objective);
}

bool ValidateObjective(const FObjective& Objective) { /* ... */ }
void UpdateObjectiveState(const FObjective& Objective) { /* ... */ }
void BroadcastObjectiveEvents(const FObjective& Objective) { /* ... */ }
// etc.
```

#### Remediation
1. Extract methods for distinct responsibilities
2. Keep functions under 50 lines
3. Single Responsibility Principle
4. Improve testability

---

### 9. Large Switch Statements (COMPLEXITY)

**Smell Type:** Complexity
**Severity:** Low
**Occurrences:** 5 instances
**Impact:** Difficult to maintain, violates OCP

#### Symptoms
- Switch statements with many cases
- Repeated switch patterns
- Difficult to extend

#### Example
```cpp
// SMELL - Large switch statement
void ProcessObjectiveType(EObjectiveType Type)
{
    switch (Type)
    {
        case EObjectiveType::Collect:
            // 20 lines
            break;
        case EObjectiveType::Interact:
            // 20 lines
            break;
        case EObjectiveType::Reach:
            // 20 lines
            break;
        // 10 more cases...
    }
}

// FRESH - Polymorphism or strategy pattern
class IObjectiveHandler
{
public:
    virtual void Process() = 0;
};

class CollectObjectiveHandler : public IObjectiveHandler
{
    void Process() override { /* ... */ }
};

// Or use TMap for simple cases
TMap<EObjectiveType, TFunction<void()>> ObjectiveHandlers;

void InitializeHandlers()
{
    ObjectiveHandlers.Add(EObjectiveType::Collect, [this]() { ProcessCollect(); });
    ObjectiveHandlers.Add(EObjectiveType::Interact, [this]() { ProcessInteract(); });
    // etc.
}

void ProcessObjectiveType(EObjectiveType Type)
{
    if (auto* Handler = ObjectiveHandlers.Find(Type))
    {
        (*Handler)();
    }
}
```

#### Remediation
1. Use polymorphism for complex cases
2. Use TMap<Enum, Function> for simple cases
3. Extract each case into a method
4. Consider Command pattern

---

### 10. Public Members Without UPROPERTY (API DESIGN)

**Smell Type:** API Design
**Severity:** Low
**Occurrences:** Multiple files
**Impact:** GC issues, poor encapsulation

#### Symptoms
- Public UObject* without UPROPERTY
- Public data members
- No encapsulation
- GC tracking issues

#### Example
```cpp
// SMELL - Public members without UPROPERTY
class UMyComponent : public UActorComponent
{
public:
    AActor* TargetActor;  // Not tracked by GC!
    float Speed;          // No Blueprint access
    TArray<FName> Tags;   // No serialization
};

// FRESH - Proper encapsulation
class UMyComponent : public UActorComponent
{
private:
    UPROPERTY()
    AActor* TargetActor;  // GC tracked
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (AllowPrivateAccess = "true"))
    float Speed = 600.0f;
    
    UPROPERTY(BlueprintReadOnly, Category = "Tags", meta = (AllowPrivateAccess = "true"))
    TArray<FName> Tags;

public:
    UFUNCTION(BlueprintCallable, Category = "MyComponent")
    AActor* GetTargetActor() const { return TargetActor; }
    
    UFUNCTION(BlueprintCallable, Category = "MyComponent")
    void SetTargetActor(AActor* NewTarget) { TargetActor = NewTarget; }
};
```

#### Remediation
1. Add UPROPERTY to all UObject pointers
2. Make data members private
3. Provide public accessors
4. Use appropriate UPROPERTY specifiers

---

## Code Smell Detection Checklist

Use this checklist during code reviews:

### Documentation
- [ ] File has header comment
- [ ] All public APIs documented
- [ ] Parameters and returns documented
- [ ] Usage examples provided

### Error Handling
- [ ] All Cast<> results checked
- [ ] GetWorld()/GetOwner() validated
- [ ] Array access bounds checked
- [ ] ensure() or check() used appropriately

### Performance
- [ ] Large types passed by const reference
- [ ] No GetWorld() in Tick (cached instead)
- [ ] No string allocations in loops
- [ ] Expensive searches cached

### Thread Safety
- [ ] Static variables are const
- [ ] Container modifications protected
- [ ] Thread safety documented
- [ ] No race conditions

### Maintainability
- [ ] No magic numbers
- [ ] Functions under 50 lines
- [ ] Cyclomatic complexity < 10
- [ ] Single Responsibility Principle

### API Design
- [ ] BlueprintCallable has Category
- [ ] UObject pointers have UPROPERTY
- [ ] Data members encapsulated
- [ ] Consistent naming conventions

---

## Smell Severity Guide

### Critical (Fix Immediately)
- Unsafe pointer operations
- Missing UPROPERTY on UObject*
- Race conditions
- Memory leaks

### High (Fix This Sprint)
- Missing documentation
- Thread safety issues
- Performance problems in hot paths

### Medium (Fix Next Sprint)
- Magic numbers
- Long functions
- Large switch statements

### Low (Fix When Convenient)
- Missing Blueprint categories
- Code style issues
- Minor optimizations

---

## Prevention Strategies

### 1. Code Review Guidelines
- Use smell detection checklist
- Require documentation for all PRs
- Enforce error handling standards
- Check for magic numbers

### 2. Automated Tools
- Static analysis in CI/CD
- Complexity metrics tracking
- Documentation coverage reports
- Performance profiling

### 3. Team Education
- Regular code smell workshops
- Share refactoring examples
- Celebrate clean code
- Learn from mistakes

### 4. Quality Gates
- No PRs with critical smells
- Documentation ratio > 20%
- Complexity < 10
- Zero static analysis errors

---

## Refactoring Priorities

Based on smell severity and frequency:

1. **Week 1:** Fix unsafe pointer operations (49 instances)
2. **Week 2:** Add documentation (164 files)
3. **Week 3:** Replace magic numbers (123 instances)
4. **Week 4:** Fix thread safety (85 issues)
5. **Week 5:** Performance optimizations (82 issues)
6. **Week 6:** API improvements and cleanup

---

## Success Metrics

### Quantitative
- [ ] Zero critical smells
- [ ] < 10 high-severity smells
- [ ] Documentation ratio > 20%
- [ ] Average complexity < 5

### Qualitative
- [ ] Code is self-documenting
- [ ] Easy to understand
- [ ] Safe and robust
- [ ] Performant

---

*Remember: Code smells are not bugs, but indicators of potential problems. Address them proactively to maintain code health.*
