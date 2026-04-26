# Code Review Checklist

## Overview
Comprehensive checklist for code reviews to maintain high quality standards.

**Version**: 1.0
**Last Updated**: 2026-04-26

---

## Pre-Review Checklist

### Author Responsibilities
- [ ] Code compiles without errors or warnings
- [ ] All tests pass locally
- [ ] Self-review completed
- [ ] Code follows project style guide
- [ ] Documentation updated
- [ ] No debug code or commented-out code
- [ ] Commit messages are clear and descriptive

### Reviewer Responsibilities
- [ ] Understand the context and requirements
- [ ] Review related documentation
- [ ] Allocate sufficient time for thorough review
- [ ] Approach review constructively

---

## Code Quality

### Readability
- [ ] Code is easy to understand
- [ ] Variable and function names are descriptive
- [ ] No magic numbers (use named constants)
- [ ] Complex logic is commented
- [ ] Code follows consistent formatting
- [ ] No unnecessary complexity

### Maintainability
- [ ] Functions are focused and single-purpose
- [ ] Classes have clear responsibilities
- [ ] Code is DRY (Don't Repeat Yourself)
- [ ] Dependencies are minimal and clear
- [ ] Code is modular and reusable

### Correctness
- [ ] Logic is correct and handles edge cases
- [ ] No obvious bugs or errors
- [ ] Error handling is appropriate
- [ ] Null/nullptr checks where needed
- [ ] Array bounds are checked
- [ ] Resource cleanup is proper

---

## Unreal Engine Specific

### Memory Management
- [ ] UPROPERTY used for UObject references
- [ ] No raw pointers to UObjects (use TWeakObjectPtr if needed)
- [ ] Proper use of NewObject/SpawnActor
- [ ] No memory leaks (check destructors)
- [ ] Smart pointers used appropriately (TSharedPtr, TUniquePtr)

### Performance
- [ ] No expensive operations in Tick
- [ ] Appropriate use of timers vs Tick
- [ ] Component references cached in BeginPlay
- [ ] No unnecessary GetWorld() calls
- [ ] TArray::Reserve used when size is known
- [ ] Const references for large parameters

### Naming Conventions
- [ ] Classes prefixed correctly (U/A/F/I/E)
- [ ] Booleans prefixed with 'b'
- [ ] Functions use PascalCase
- [ ] Variables use PascalCase
- [ ] Functions start with verbs (Get, Set, Is, etc.)

### Blueprint Integration
- [ ] UFUNCTION used for Blueprint-callable functions
- [ ] UPROPERTY used for Blueprint-visible properties
- [ ] BlueprintReadWrite/BlueprintReadOnly used appropriately
- [ ] Category specified for organization
- [ ] Tooltips provided for Blueprint-exposed elements

### Replication (if applicable)
- [ ] UPROPERTY(Replicated) for replicated properties
- [ ] GetLifetimeReplicatedProps implemented
- [ ] Server/Client RPC functions marked correctly
- [ ] Replication conditions appropriate
- [ ] No unnecessary replication

---

## Architecture & Design

### Design Patterns
- [ ] Appropriate design patterns used
- [ ] No anti-patterns present
- [ ] SOLID principles followed
- [ ] Separation of concerns maintained
- [ ] Interfaces used where appropriate

### Dependencies
- [ ] Minimal coupling between modules
- [ ] No circular dependencies
- [ ] Dependencies are explicit and necessary
- [ ] Proper use of forward declarations
- [ ] Include order follows convention

### Extensibility
- [ ] Code is easy to extend
- [ ] Virtual functions used appropriately
- [ ] Configuration externalized where appropriate
- [ ] Hard-coded values avoided

---

## Testing

### Test Coverage
- [ ] Unit tests added for new functionality
- [ ] Edge cases tested
- [ ] Error conditions tested
- [ ] Tests are independent and repeatable
- [ ] Test names are descriptive

### Test Quality
- [ ] Tests actually test the intended behavior
- [ ] No flaky tests
- [ ] Tests run quickly
- [ ] Mocks/stubs used appropriately
- [ ] Test data is realistic

---

## Security

### Input Validation
- [ ] All user input validated
- [ ] Bounds checking on arrays/collections
- [ ] String length limits enforced
- [ ] Type checking performed

### Data Protection
- [ ] No sensitive data in logs
- [ ] No hardcoded credentials
- [ ] Proper access control
- [ ] Secure data transmission

### Resource Management
- [ ] No resource exhaustion vulnerabilities
- [ ] Proper cleanup of resources
- [ ] No infinite loops or recursion
- [ ] Rate limiting where appropriate

---

## Performance

### Algorithms
- [ ] Appropriate algorithm complexity
- [ ] No unnecessary nested loops
- [ ] Efficient data structures used
- [ ] Caching used where beneficial

### Memory
- [ ] No excessive memory allocations
- [ ] Object pooling used where appropriate
- [ ] Large objects passed by reference
- [ ] No memory leaks

### Optimization
- [ ] Premature optimization avoided
- [ ] Performance-critical code identified
- [ ] Profiling data considered
- [ ] Trade-offs documented

---

## Documentation

### Code Comments
- [ ] Complex logic explained
- [ ] Why, not what, is documented
- [ ] No outdated comments
- [ ] No commented-out code
- [ ] TODO/FIXME items tracked

### API Documentation
- [ ] Public functions documented
- [ ] Parameters explained
- [ ] Return values described
- [ ] Exceptions/errors documented
- [ ] Usage examples provided

### Design Documentation
- [ ] Architecture decisions documented
- [ ] Design patterns explained
- [ ] Trade-offs noted
- [ ] Diagrams updated if needed

---

## Error Handling

### Error Detection
- [ ] Errors are detected early
- [ ] Appropriate error checking
- [ ] Assertions used for invariants
- [ ] Logging at appropriate levels

### Error Recovery
- [ ] Graceful degradation
- [ ] Proper error propagation
- [ ] Resources cleaned up on error
- [ ] User-friendly error messages

---

## Logging & Debugging

### Logging
- [ ] Appropriate log levels used
- [ ] Sensitive data not logged
- [ ] Log messages are clear
- [ ] No excessive logging in hot paths
- [ ] Structured logging used

### Debugging Support
- [ ] Debug visualization available
- [ ] Stat commands implemented
- [ ] Console commands for testing
- [ ] Debug flags properly guarded

---

## Version Control

### Commits
- [ ] Commits are atomic and focused
- [ ] Commit messages follow convention
- [ ] No unrelated changes included
- [ ] Binary files handled appropriately

### Branch Management
- [ ] Branch naming follows convention
- [ ] Branch is up to date with main
- [ ] No merge conflicts
- [ ] Clean commit history

---

## Specific Code Patterns

### Initialization
- [ ] Members initialized in constructor
- [ ] BeginPlay used for runtime initialization
- [ ] Proper initialization order
- [ ] No initialization in header

### Cleanup
- [ ] Destructor cleans up resources
- [ ] EndPlay handles cleanup
- [ ] Timers cleared appropriately
- [ ] Delegates unbound

### State Management
- [ ] State transitions are clear
- [ ] Invalid states prevented
- [ ] State is consistent
- [ ] Thread-safe if needed

---

## Review Process

### During Review
- [ ] Understand the change before commenting
- [ ] Be specific in feedback
- [ ] Suggest improvements, don't just criticize
- [ ] Distinguish between must-fix and nice-to-have
- [ ] Ask questions if unclear

### After Review
- [ ] All comments addressed or discussed
- [ ] Follow-up items tracked
- [ ] Approval given when satisfied
- [ ] Knowledge shared with team

---

## Common Issues to Watch For

### Unreal Engine
- ❌ Using Tick when timer would suffice
- ❌ Not caching component references
- ❌ Raw pointers to UObjects
- ❌ Spawning actors in Tick
- ❌ Missing UPROPERTY on UObject references
- ❌ Incorrect replication setup
- ❌ Not checking for nullptr
- ❌ Missing GetLifetimeReplicatedProps

### C++ General
- ❌ Memory leaks
- ❌ Uninitialized variables
- ❌ Buffer overflows
- ❌ Integer overflow
- ❌ Use after free
- ❌ Dangling pointers
- ❌ Race conditions
- ❌ Deadlocks

### Code Quality
- ❌ God classes (too many responsibilities)
- ❌ Long functions (> 50 lines)
- ❌ Deep nesting (> 3 levels)
- ❌ High cyclomatic complexity (> 10)
- ❌ Code duplication
- ❌ Magic numbers
- ❌ Poor naming
- ❌ Lack of error handling

---

## Severity Levels

### Critical (P0) - Block Merge
- Security vulnerabilities
- Memory leaks
- Crashes or data corruption
- Breaking changes without migration
- Incorrect core functionality

### High (P1) - Must Fix Before Merge
- Performance issues
- Incorrect error handling
- Missing tests for critical paths
- Significant code quality issues
- Naming convention violations

### Medium (P2) - Should Fix
- Code duplication
- Missing documentation
- Minor performance issues
- Style inconsistencies
- Incomplete error handling

### Low (P3) - Nice to Have
- Code organization improvements
- Additional comments
- Minor refactoring opportunities
- Optimization suggestions

---

## Review Feedback Template

```markdown
## Summary
[Brief overview of the change]

## Strengths
- [What was done well]
- [Good practices observed]

## Issues Found

### Critical
- [ ] [Issue description] - [Location]

### High Priority
- [ ] [Issue description] - [Location]

### Medium Priority
- [ ] [Issue description] - [Location]

### Suggestions
- [Improvement suggestion]
- [Alternative approach]

## Questions
- [Clarification needed]

## Overall Assessment
[Approve / Request Changes / Comment]
```

---

## Quick Reference

### Before Approving, Verify:
1. ✅ Code compiles and tests pass
2. ✅ No critical or high-priority issues
3. ✅ Naming conventions followed
4. ✅ Performance considerations addressed
5. ✅ Memory management correct
6. ✅ Error handling appropriate
7. ✅ Documentation sufficient
8. ✅ Tests adequate

### Red Flags:
- 🚩 "This is temporary code"
- 🚩 "I'll fix this later"
- 🚩 "This works but I don't know why"
- 🚩 Excessive commented-out code
- 🚩 TODO comments without tickets
- 🚩 Copy-pasted code
- 🚩 Overly complex logic
- 🚩 Missing error handling

---

## Resources

- [Unreal Engine Coding Standard](https://docs.unrealengine.com/5.3/en-US/epic-cplusplus-coding-standard-for-unreal-engine/)
- [C++ Core Guidelines](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines)
- [Code Review Best Practices](https://google.github.io/eng-practices/review/)
- [Clean Code Principles](https://www.oreilly.com/library/view/clean-code-a/9780136083238/)

---

**Remember**: The goal of code review is to improve code quality and share knowledge, not to criticize the author. Be constructive, specific, and respectful in all feedback.
