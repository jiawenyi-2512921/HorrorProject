# Technical Debt Inventory - HorrorProject

Generated: 2026-04-26
Comprehensive technical debt tracking and management

## Overview

Technical debt represents the implied cost of additional rework caused by choosing an easy solution now instead of a better approach that would take longer.

**Current Debt Level:** LOW-MEDIUM
**Estimated Effort to Resolve:** 6 weeks
**Risk Level:** LOW

## Debt Categories

### 1. Documentation Debt (HIGH PRIORITY)

**Severity:** High | **Effort:** 40 hours | **Interest Rate:** High

#### Description
Critically low documentation coverage (3.1% comment ratio vs 20% target).

#### Impact
- Difficult onboarding for new developers
- Increased maintenance time
- Blueprint users lack guidance
- API misuse potential

#### Debt Items
| File | Lines | Comments | Ratio | Priority |
|------|-------|----------|-------|----------|
| HorrorGameModeBase.cpp | 463 | 1 | 0.2% | P0 |
| HorrorAudioSubsystem.cpp | 422 | 1 | 0.2% | P0 |
| FoundFootageObjectiveInteractable.cpp | 335 | 1 | 0.3% | P0 |
| CameraPhotoComponent.cpp | 324 | 1 | 0.3% | P0 |
| InteractionComponent.cpp | 310 | 1 | 0.3% | P0 |

#### Resolution Plan
1. Add file-level documentation headers
2. Document all UFUNCTION(BlueprintCallable)
3. Add parameter/return documentation
4. Include usage examples
5. Generate API documentation with Doxygen

#### Payoff
- 50% reduction in onboarding time
- Fewer support questions
- Better Blueprint usability
- Easier maintenance

---

### 2. Magic Number Debt (MEDIUM PRIORITY)

**Severity:** Medium | **Effort:** 16 hours | **Interest Rate:** Medium

#### Description
123 magic numbers scattered throughout codebase.

#### Impact
- Unclear intent
- Difficult tuning
- Copy-paste errors
- Inconsistent values

#### Debt Items by Module
| Module | Count | Examples |
|--------|-------|----------|
| AI | 17 | Detection ranges, speeds, timings |
| Audio | 24 | Volume levels, fade times, distances |
| Player | 31 | Battery drain, photo limits, movement |
| Game | 28 | Objective timings, spawn delays |
| VFX | 23 | Effect intensities, durations |

#### Resolution Plan
1. Create module-specific constants headers
2. Replace inline numbers with named constants
3. Convert to UPROPERTY for designer tuning
4. Document constant meanings
5. Establish naming conventions

#### Example Refactoring
```cpp
// BEFORE - Magic numbers
if (Distance < 800.0f)
{
    Speed = 600.0f;
    DetectionTime = 2.5f;
}

// AFTER - Named constants
static constexpr float ThreatDetectionRange = 800.0f;
static constexpr float ThreatChaseSpeed = 600.0f;
static constexpr float ThreatDetectionDelay = 2.5f;

if (Distance < ThreatDetectionRange)
{
    Speed = ThreatChaseSpeed;
    DetectionTime = ThreatDetectionDelay;
}

// BEST - Designer-tunable
UPROPERTY(EditDefaultsOnly, Category = "AI|Detection")
float DetectionRange = 800.0f;

UPROPERTY(EditDefaultsOnly, Category = "AI|Movement")
float ChaseSpeed = 600.0f;

UPROPERTY(EditDefaultsOnly, Category = "AI|Detection")
float DetectionDelay = 2.5f;
```

#### Payoff
- Easier game balancing
- Self-documenting code
- Reduced bugs from typos
- Designer empowerment

---

### 3. Error Handling Debt (HIGH PRIORITY)

**Severity:** High | **Effort:** 12 hours | **Interest Rate:** High

#### Description
49 instances of unsafe pointer operations without proper validation.

#### Impact
- Potential crashes
- Undefined behavior
- Difficult debugging
- Poor user experience

#### Debt Items by Category
| Category | Count | Risk |
|----------|-------|------|
| Unsafe Cast<> | 15 | High |
| Chained calls without checks | 22 | High |
| Array access without bounds check | 8 | Medium |
| Missing null checks | 4 | High |

#### Critical Files
```
HorrorGolemBehaviorComponent.cpp:20   - Cast without check
HorrorGolemBehaviorComponent.cpp:420  - Cast without check
AmbientAudioComponent.cpp:248         - Chained GetWorld()->
FootstepAudioComponent.cpp:125        - Chained GetOwner()->
```

#### Resolution Plan
1. Add null checks before all Cast<> operations
2. Use ensure() for expected-valid pointers
3. Use check() for must-be-valid pointers
4. Add bounds checking for array access
5. Validate GetWorld()/GetOwner() results

#### Example Refactoring
```cpp
// BEFORE - Unsafe
AActor* Target = Cast<AActor>(Object);
Target->SetActorLocation(NewLocation);

// AFTER - Safe with ensure
if (AActor* Target = Cast<AActor>(Object))
{
    Target->SetActorLocation(NewLocation);
}
else
{
    UE_LOG(LogTemp, Warning, TEXT("Failed to cast to AActor"));
}

// BEFORE - Unsafe chained call
GetWorld()->GetFirstPlayerController()->GetPawn();

// AFTER - Safe with validation
UWorld* World = GetWorld();
if (ensure(World))
{
    APlayerController* PC = World->GetFirstPlayerController();
    if (ensure(PC))
    {
        APawn* Pawn = PC->GetPawn();
        // Use Pawn safely
    }
}
```

#### Payoff
- Zero crash bugs from null pointers
- Better error messages
- Easier debugging
- More robust gameplay

---

### 4. Thread Safety Debt (MEDIUM PRIORITY)

**Severity:** Medium | **Effort:** 24 hours | **Interest Rate:** Medium

#### Description
85 potential thread safety issues, primarily non-const static variables and unprotected container modifications.

#### Impact
- Race conditions
- Data corruption
- Intermittent bugs
- Difficult reproduction

#### Debt Items by Type
| Type | Count | Risk |
|------|-------|------|
| Non-const static variables | 73 | Medium |
| Unprotected container mods | 12 | High |

#### Critical Areas
```
HorrorAudioLibrary.h          - 12 non-const statics
ArchiveSubsystem.cpp          - 4 unprotected TArray mods
EvidenceBlueprintLibrary.h    - 4 non-const statics
UIAnimationLibrary.h          - 3 non-const statics
```

#### Resolution Plan
1. Make static variables const where possible
2. Add FScopeLock for shared container access
3. Use TAtomic for counters
4. Document thread safety guarantees
5. Add thread safety tests

#### Example Refactoring
```cpp
// BEFORE - Non-const static (thread-unsafe)
static FName CurrentZone;

// AFTER - Const static (thread-safe)
static const FName DefaultZone = TEXT("MainZone");

// BEFORE - Unprotected container modification
TArray<FEvidence> Evidence;
void AddEvidence(const FEvidence& NewEvidence)
{
    Evidence.Add(NewEvidence);  // Race condition!
}

// AFTER - Protected with lock
TArray<FEvidence> Evidence;
FCriticalSection EvidenceLock;

void AddEvidence(const FEvidence& NewEvidence)
{
    FScopeLock Lock(&EvidenceLock);
    Evidence.Add(NewEvidence);
}
```

#### Payoff
- Eliminate race conditions
- Safer async operations
- Better multiplayer support
- Fewer intermittent bugs

---

### 5. Performance Debt (LOW PRIORITY)

**Severity:** Low | **Effort:** 8 hours | **Interest Rate:** Low

#### Description
82 performance issues, primarily inefficient parameter passing.

#### Impact
- Unnecessary memory allocations
- Cache misses
- Reduced frame rate
- Increased memory pressure

#### Debt Items by Type
| Type | Count | Impact |
|------|-------|--------|
| Pass-by-value for large types | 68 | Low |
| GetWorld() in Tick | 8 | Medium |
| FString in loops | 4 | Medium |
| Expensive searches | 2 | High |

#### Resolution Plan
1. Pass large types by const reference
2. Cache GetWorld() results
3. Move string allocations out of loops
4. Cache search results

#### Example Refactoring
```cpp
// BEFORE - Pass by value (copies entire struct)
void ProcessTag(FGameplayTag Tag)
{
    // ...
}

// AFTER - Pass by const reference (no copy)
void ProcessTag(const FGameplayTag& Tag)
{
    // ...
}

// BEFORE - GetWorld() in Tick
void Tick(float DeltaTime)
{
    GetWorld()->GetTimeSeconds();  // Called every frame!
    GetWorld()->SpawnActor(...);
}

// AFTER - Cache World pointer
void Tick(float DeltaTime)
{
    UWorld* World = GetWorld();
    if (World)
    {
        World->GetTimeSeconds();
        World->SpawnActor(...);
    }
}
```

#### Payoff
- Reduced memory allocations
- Better frame rate
- Lower memory usage
- Improved cache efficiency

---

### 6. Code Style Debt (LOW PRIORITY)

**Severity:** Low | **Effort:** 12 hours | **Interest Rate:** Low

#### Description
4 errors and 4735 warnings related to UE coding standards compliance.

#### Impact
- Inconsistent codebase
- Harder code reviews
- Potential UE integration issues
- Team confusion

#### Debt Items
- Naming convention violations
- Include order issues
- Missing header guards
- Inconsistent formatting

#### Resolution Plan
1. Fix critical naming issues
2. Ensure .generated.h is last include
3. Add #pragma once to all headers
4. Run clang-format on codebase
5. Set up pre-commit hooks

#### Payoff
- Consistent codebase
- Easier code reviews
- Better UE integration
- Professional appearance

---

### 7. Architectural Debt (LOW PRIORITY)

**Severity:** Low | **Effort:** 80 hours | **Interest Rate:** Medium

#### Description
2 files with high coupling (>15 dependencies), some module boundary issues.

#### Impact
- Difficult refactoring
- Tight coupling
- Reduced testability
- Limited reusability

#### Debt Items
- High coupling in core systems
- Missing abstraction layers
- Direct dependencies between modules
- Insufficient interfaces

#### Resolution Plan
1. Extract common interfaces
2. Apply dependency inversion
3. Reduce file-level coupling
4. Improve module boundaries
5. Create facade patterns

#### Payoff
- Better testability
- Easier refactoring
- Improved reusability
- Cleaner architecture

---

## Debt Tracking

### Total Debt Summary
| Category | Severity | Effort | Interest | Priority |
|----------|----------|--------|----------|----------|
| Documentation | High | 40h | High | P0 |
| Error Handling | High | 12h | High | P0 |
| Magic Numbers | Medium | 16h | Medium | P1 |
| Thread Safety | Medium | 24h | Medium | P1 |
| Performance | Low | 8h | Low | P2 |
| Code Style | Low | 12h | Low | P2 |
| Architecture | Low | 80h | Medium | P3 |
| **TOTAL** | - | **192h** | - | - |

### Debt Trend
```
Current: 192 hours
Target:  < 40 hours (maintenance level)
Reduction needed: 152 hours
```

### Interest Calculation
Technical debt "interest" is the ongoing cost of not fixing the debt:

| Debt Type | Weekly Interest | Annual Cost |
|-----------|----------------|-------------|
| Documentation | 4h/week | 208h/year |
| Error Handling | 2h/week | 104h/year |
| Magic Numbers | 1h/week | 52h/year |
| Thread Safety | 1h/week | 52h/year |
| Performance | 0.5h/week | 26h/year |
| **TOTAL** | **8.5h/week** | **442h/year** |

**ROI Analysis:** Investing 192 hours now saves 442 hours annually.

---

## Debt Management Strategy

### Prevention
1. **Code Review Checklist**
   - Documentation complete?
   - No magic numbers?
   - Error handling present?
   - Thread-safe?
   - Performance considered?

2. **Quality Gates**
   - Static analysis must pass
   - Test coverage > 80%
   - Complexity < 10
   - Documentation ratio > 20%

3. **Continuous Monitoring**
   - Weekly quality reports
   - Monthly debt reviews
   - Quarterly refactoring sprints

### Reduction
1. **Dedicated Time**
   - 20% of sprint for debt reduction
   - Monthly "quality day"
   - Quarterly refactoring sprint

2. **Boy Scout Rule**
   - Leave code better than you found it
   - Fix nearby issues when touching code
   - Add tests for legacy code

3. **Incremental Improvement**
   - Small, frequent improvements
   - Prioritize high-interest debt
   - Celebrate progress

---

## Action Plan

### Week 1-2: High Priority Debt
- [ ] Add error handling (12h)
- [ ] Start documentation (20h)
- [ ] Fix critical thread safety (8h)

### Week 3-4: Medium Priority Debt
- [ ] Replace magic numbers (16h)
- [ ] Complete documentation (20h)
- [ ] Fix remaining thread safety (16h)

### Week 5-6: Low Priority Debt
- [ ] Performance optimizations (8h)
- [ ] Code style fixes (12h)
- [ ] Set up quality gates (8h)

### Future: Architectural Debt
- [ ] Plan architectural improvements
- [ ] Schedule refactoring sprints
- [ ] Implement gradually over 3 months

---

## Success Metrics

### Quantitative
- [ ] Total debt: 192h → 40h
- [ ] Weekly interest: 8.5h → 2h
- [ ] Documentation ratio: 3.1% → 20%
- [ ] Static analysis warnings: 375 → 50

### Qualitative
- [ ] Faster onboarding
- [ ] Fewer production bugs
- [ ] Easier maintenance
- [ ] Better team morale

---

*Track debt regularly. Update this document monthly. Celebrate progress.*
