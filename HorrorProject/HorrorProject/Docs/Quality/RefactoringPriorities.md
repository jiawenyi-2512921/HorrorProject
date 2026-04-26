# Refactoring Priorities - HorrorProject

Generated: 2026-04-26
Based on comprehensive code quality analysis

## Executive Summary

Overall code quality is **GOOD** with excellent complexity metrics:
- Average Complexity: 2 (Target: <10) ✓
- Average Maintainability: 89.5/100 (Target: >70) ✓
- Zero Circular Dependencies ✓
- Zero High-Risk Functions ✓

**Key Issues to Address:**
1. Low comment ratio (3.1% vs target 20%)
2. 123 magic numbers requiring constants
3. 85 thread safety warnings
4. 82 performance optimization opportunities
5. 49 error handling improvements

## Priority Matrix

### P0 - Critical (Address Immediately)

#### 1. Documentation Coverage
**Impact:** High | **Effort:** Medium | **Timeline:** 1 week

Current comment ratio: 3.1% (Target: 20%)

**Action Items:**
- Add file-level documentation to all source files
- Document all public APIs (UFUNCTION BlueprintCallable)
- Add parameter and return value documentation
- Include usage examples for complex systems

**Files Requiring Documentation:**
- HorrorGameModeBase.cpp (463 lines, 0.2% comments)
- HorrorAudioSubsystem.cpp (422 lines, 0.2% comments)
- FoundFootageObjectiveInteractable.cpp (335 lines, 0.3% comments)
- CameraPhotoComponent.cpp (324 lines, 0.3% comments)
- InteractionComponent.cpp (310 lines, 0.3% comments)

#### 2. Error Handling Robustness
**Impact:** High | **Effort:** Low | **Timeline:** 3 days

49 instances of unsafe pointer operations detected.

**Action Items:**
- Add null checks before Cast<> operations
- Use ensure() or check() for chained calls
- Validate GetWorld()/GetOwner() results
- Add bounds checking for array access

**Critical Files:**
- HorrorGolemBehaviorComponent.cpp (3 unsafe casts)
- AmbientAudioComponent.cpp (3 chained calls)
- FootstepAudioComponent.cpp (2 chained calls)

### P1 - High Priority (Next Sprint)

#### 3. Magic Numbers to Constants
**Impact:** Medium | **Effort:** Low | **Timeline:** 2 days

123 magic numbers detected across codebase.

**Action Items:**
- Create constants header for each module
- Replace inline numbers with named constants
- Use UPROPERTY for tunable values
- Document constant meanings

**Example Refactoring:**
```cpp
// Before
if (Distance < 800.0f)

// After
static constexpr float ThreatDetectionRange = 800.0f;
if (Distance < ThreatDetectionRange)
```

**High-Impact Files:**
- HorrorGolemBehaviorComponent.h (17 magic numbers)
- AmbientAudioComponent.cpp (multiple timing values)
- CameraPhotoComponent.cpp (threshold values)

#### 4. Thread Safety Improvements
**Impact:** Medium | **Effort:** Medium | **Timeline:** 1 week

85 potential thread safety issues detected.

**Action Items:**
- Make static variables const where possible
- Add FScopeLock for container modifications
- Document thread safety guarantees
- Use atomic operations for counters

**Critical Areas:**
- HorrorAudioLibrary.h (12 non-const statics)
- ArchiveSubsystem.cpp (4 unprotected container mods)
- EvidenceBlueprintLibrary.h (4 non-const statics)

#### 5. Performance Optimizations
**Impact:** Medium | **Effort:** Low | **Timeline:** 2 days

82 performance issues detected (mostly parameter passing).

**Action Items:**
- Pass large types by const reference
- Cache GetWorld() calls in Tick
- Move string allocations out of loops
- Use FName instead of FString where appropriate

**Pattern to Fix:**
```cpp
// Before
void UpdateValue(FGameplayTag Tag)

// After  
void UpdateValue(const FGameplayTag& Tag)
```

### P2 - Medium Priority (Next Month)

#### 6. API Design Improvements
**Impact:** Low | **Effort:** Medium | **Timeline:** 1 week

**Action Items:**
- Add Category to all BlueprintCallable functions
- Encapsulate public members with UPROPERTY
- Use const& for input, pointer for output parameters
- Improve Blueprint-friendly naming

#### 7. Code Style Compliance
**Impact:** Low | **Effort:** Low | **Timeline:** 3 days

4 errors, 4735 warnings detected (mostly naming conventions).

**Action Items:**
- Fix class naming (ensure UE prefixes)
- Standardize member variable naming
- Ensure .generated.h is last include
- Add #pragma once to all headers

#### 8. Module Coupling Reduction
**Impact:** Low | **Effort:** High | **Timeline:** 2 weeks

2 files with high coupling (>15 dependencies).

**Action Items:**
- Refactor high-coupling files
- Extract common interfaces
- Apply dependency inversion
- Improve module boundaries

### P3 - Low Priority (Backlog)

#### 9. Function Length Reduction
**Impact:** Low | **Effort:** Medium

5 medium-risk functions (complexity 10-20).

**Action Items:**
- Break down switch statements in FoundFootageObjectiveInteractable.cpp
- Extract helper methods from complex functions
- Simplify conditional logic

#### 10. Test Coverage Expansion
**Impact:** Low | **Effort:** High

**Action Items:**
- Increase test coverage for critical systems
- Add integration tests
- Implement performance benchmarks
- Create regression test suite

## Refactoring Strategy

### Phase 1: Quick Wins (Week 1)
1. Add error handling (null checks, ensures)
2. Replace magic numbers with constants
3. Fix parameter passing (const references)
4. Add file-level documentation

**Expected Impact:** Immediate stability and readability improvements

### Phase 2: Quality Improvements (Week 2-3)
1. Complete API documentation
2. Fix thread safety issues
3. Improve code style compliance
4. Add Blueprint categories

**Expected Impact:** Better maintainability and Blueprint usability

### Phase 3: Architectural Refinements (Week 4-6)
1. Reduce module coupling
2. Refactor complex functions
3. Improve test coverage
4. Establish quality gates

**Expected Impact:** Long-term code health and scalability

## Refactoring Guidelines

### Before Refactoring
1. ✓ Ensure all tests pass
2. ✓ Create feature branch
3. ✓ Document current behavior
4. ✓ Identify affected systems

### During Refactoring
1. ✓ Make small, incremental changes
2. ✓ Run tests after each change
3. ✓ Maintain functionality (no behavior changes)
4. ✓ Add tests for edge cases

### After Refactoring
1. ✓ Verify all tests pass
2. ✓ Run static analysis
3. ✓ Performance benchmark
4. ✓ Code review
5. ✓ Update documentation

## Success Metrics

### Code Quality Targets
- [ ] Comment ratio: 3.1% → 20%
- [ ] Magic numbers: 123 → 0
- [ ] Thread safety warnings: 85 → 0
- [ ] Performance warnings: 82 → 0
- [ ] Error handling issues: 49 → 0
- [ ] Average maintainability: 89.5 → 95+

### Process Metrics
- [ ] All public APIs documented
- [ ] Zero critical static analysis errors
- [ ] 80%+ test coverage for critical systems
- [ ] CI/CD quality gates established
- [ ] Monthly quality audits scheduled

## Risk Assessment

### Low Risk Refactorings
- Adding documentation
- Replacing magic numbers
- Adding const to parameters
- Adding null checks

### Medium Risk Refactorings
- Thread safety improvements
- Function extraction
- Module restructuring

### High Risk Refactorings
- Breaking API changes
- Architectural changes
- Performance optimizations in hot paths

**Recommendation:** Start with low-risk items, build confidence, then tackle medium and high-risk items.

## Resources Required

### Team Allocation
- 1 Senior Developer (lead refactoring)
- 2 Developers (implementation)
- 1 QA Engineer (testing)

### Tools Needed
- Static analysis tools (PVS-Studio, ReSharper C++)
- Performance profilers (Unreal Insights)
- Documentation generators (Doxygen)
- CI/CD pipeline integration

### Timeline
- Phase 1: 1 week
- Phase 2: 2 weeks  
- Phase 3: 3 weeks
- **Total: 6 weeks**

## Next Steps

1. **Review this document** with the team
2. **Create GitHub issues** for each priority item
3. **Schedule refactoring sprint** for Phase 1
4. **Set up automated quality checks** in CI/CD
5. **Establish code review guidelines** for quality
6. **Schedule weekly progress reviews**

---

*This document is a living guide. Update priorities as the codebase evolves.*
