# ExtractDuplicateCode.ps1 - Duplicate Code Extraction Helper
# Generates extraction suggestions for duplicate code blocks

param(
    [string]$SourcePath = "Source",
    [string]$OutputFile = "Docs/Quality/Improvement/DuplicationExtractionPlan.md"
)

$ExtractionTasks = @()

function Generate-ExtractionSuggestion {
    param($DuplicateGroup, $Locations)

    $task = [PSCustomObject]@{
        Occurrences = $Locations.Count
        Files = ($Locations | ForEach-Object { $_.File } | Select-Object -Unique)
        Priority = if ($Locations.Count -gt 3) { "P0" } elseif ($Locations.Count -eq 3) { "P1" } else { "P2" }
        EstimatedHours = $Locations.Count * 0.5
        ExtractionMethod = ""
        TargetLocation = ""
        NewFunctionName = ""
    }

    # Determine extraction method
    $uniqueFiles = $task.Files.Count
    if ($uniqueFiles -eq 1) {
        $task.ExtractionMethod = "Extract to private method in same class"
        $task.TargetLocation = $task.Files[0]
    }
    elseif ($uniqueFiles -le 3) {
        $task.ExtractionMethod = "Extract to utility class or base class"
        $task.TargetLocation = "Create shared utility class"
    }
    else {
        $task.ExtractionMethod = "Extract to global utility namespace"
        $task.TargetLocation = "Utilities/SharedHelpers.h"
    }

    return $task
}

# Main execution
Write-Host "Generating Duplication Extraction Plan..." -ForegroundColor Cyan

$report = @"
# Duplicate Code Extraction Plan
Generated: $(Get-Date -Format "yyyy-MM-dd HH:mm:ss")

## Overview
This document provides a systematic plan for extracting duplicate code into reusable functions.

## Extraction Strategies

### 1. Extract Method (Same Class)
When duplication is within the same class:
```cpp
// Before
void ClassA::Method1() {
    // Duplicate code block
    if (Condition) {
        DoSomething();
    }
}

void ClassA::Method2() {
    // Same duplicate code block
    if (Condition) {
        DoSomething();
    }
}

// After
void ClassA::Method1() {
    ExtractedHelper();
}

void ClassA::Method2() {
    ExtractedHelper();
}

void ClassA::ExtractedHelper() {
    if (Condition) {
        DoSomething();
    }
}
```

### 2. Extract to Utility Class
When duplication spans multiple related classes:
```cpp
// Before - Duplicated in multiple classes
void ClassA::ProcessData() {
    // Complex validation logic
}

void ClassB::ProcessData() {
    // Same complex validation logic
}

// After - Extracted to utility
class UDataValidator {
public:
    static bool ValidateData(const FData& Data) {
        // Complex validation logic
        return true;
    }
};

void ClassA::ProcessData() {
    if (UDataValidator::ValidateData(MyData)) {
        // Process
    }
}

void ClassB::ProcessData() {
    if (UDataValidator::ValidateData(MyData)) {
        // Process
    }
}
```

### 3. Extract to Base Class
When duplication is in related classes:
```cpp
// Before
class AEnemyA : public ACharacter {
    void TakeDamage() {
        // Common damage logic
    }
};

class AEnemyB : public ACharacter {
    void TakeDamage() {
        // Same common damage logic
    }
};

// After
class ABaseEnemy : public ACharacter {
protected:
    virtual void TakeDamage() {
        // Common damage logic
    }
};

class AEnemyA : public ABaseEnemy {
    // Inherits TakeDamage
};

class AEnemyB : public ABaseEnemy {
    // Inherits TakeDamage
};
```

### 4. Template Method Pattern
When algorithms are similar but have variations:
```cpp
// Before
void ProcessTypeA() {
    Setup();
    // Type A specific logic
    Cleanup();
}

void ProcessTypeB() {
    Setup();
    // Type B specific logic
    Cleanup();
}

// After
class Processor {
protected:
    void Process() {
        Setup();
        DoSpecificWork();
        Cleanup();
    }

    virtual void DoSpecificWork() = 0;

private:
    void Setup() { /* Common setup */ }
    void Cleanup() { /* Common cleanup */ }
};

class ProcessorA : public Processor {
protected:
    void DoSpecificWork() override {
        // Type A specific logic
    }
};
```

## Extraction Workflow

### Step 1: Identify Duplication
Run CodeDuplicationDetector.ps1 to find duplicate code blocks.

### Step 2: Analyze Context
- Are the duplicates in the same class?
- Are they in related classes?
- Are they in unrelated classes?

### Step 3: Choose Extraction Method
- Same class → Private method
- Related classes → Base class or shared component
- Unrelated classes → Utility class or namespace

### Step 4: Extract Code
1. Create new method/class
2. Move duplicate code
3. Replace all occurrences with calls
4. Test thoroughly

### Step 5: Refine
- Improve naming
- Add documentation
- Optimize if needed

## Common Extraction Patterns

### Pattern 1: Validation Logic
```cpp
// Utility class for common validations
class UValidationHelpers {
public:
    static bool IsValidActor(const AActor* Actor) {
        return Actor != nullptr && !Actor->IsPendingKill();
    }

    static bool IsInRange(const FVector& A, const FVector& B, float Range) {
        return FVector::DistSquared(A, B) <= Range * Range;
    }
};
```

### Pattern 2: Initialization Sequences
```cpp
// Base class with common initialization
class UBaseComponent : public UActorComponent {
protected:
    virtual void BeginPlay() override {
        Super::BeginPlay();
        InitializeCommon();
        InitializeSpecific();
    }

    void InitializeCommon() {
        // Common initialization
    }

    virtual void InitializeSpecific() {
        // Override in derived classes
    }
};
```

### Pattern 3: Error Handling
```cpp
// Centralized error handling
class UErrorHandler {
public:
    static void HandleError(const FString& Context, const FString& Message) {
        UE_LOG(LogTemp, Error, TEXT("%s: %s"), *Context, *Message);
        // Common error handling logic
    }
};
```

### Pattern 4: Resource Management
```cpp
// RAII wrapper for common resource patterns
class FScopedResourceLock {
public:
    FScopedResourceLock(UResource* InResource) : Resource(InResource) {
        if (Resource) Resource->Lock();
    }

    ~FScopedResourceLock() {
        if (Resource) Resource->Unlock();
    }

private:
    UResource* Resource;
};
```

## Best Practices

### 1. Naming
- Use descriptive names that explain purpose
- Follow Unreal naming conventions
- Avoid generic names like "Helper" or "Utility" alone

### 2. Parameters
- Keep parameter count low (< 4)
- Use const references for large objects
- Consider parameter objects for related data

### 3. Documentation
- Document extracted functions
- Explain why extraction was done
- Note any assumptions or constraints

### 4. Testing
- Write tests for extracted code
- Verify all call sites work correctly
- Test edge cases

### 5. Gradual Refactoring
- Extract one duplication at a time
- Commit after each successful extraction
- Don't mix extraction with other changes

## Automated Extraction Tools

### Visual Studio
1. Select duplicate code
2. Right-click → Quick Actions
3. Choose "Extract Method"
4. Review and apply

### ReSharper C++
1. Select code block
2. Ctrl+Shift+R → Extract Method
3. Configure parameters
4. Apply to all occurrences

## Quality Metrics

Track these metrics to measure improvement:

| Metric | Before | Target | Current |
|--------|--------|--------|---------|
| Duplication Groups | TBD | < 10 | TBD |
| Duplicated Lines | TBD | < 500 | TBD |
| Code Reuse Score | TBD | > 95% | TBD |

## Sprint Planning

### Sprint 1: High-Frequency Duplications
Focus on code duplicated 4+ times
- Highest impact on maintainability
- Most error-prone
- Estimated: 12-16 hours

### Sprint 2: Medium-Frequency Duplications
Address code duplicated 3 times
- Moderate impact
- Good refactoring practice
- Estimated: 8-12 hours

### Sprint 3: Low-Frequency Duplications
Clean up remaining duplications
- Complete the cleanup
- Establish patterns
- Estimated: 4-8 hours

## Success Criteria

- [ ] All high-frequency duplications extracted
- [ ] Utility classes documented
- [ ] Tests added for extracted code
- [ ] Code review approved
- [ ] No new duplications introduced

## Maintenance

### Preventing Future Duplication

1. **Code Reviews**: Check for similar code during reviews
2. **Pair Programming**: Share knowledge of existing utilities
3. **Documentation**: Maintain list of common utilities
4. **Training**: Educate team on extraction patterns
5. **Automated Checks**: Run duplication detector in CI/CD

### Regular Audits
- Run duplication detector weekly
- Review new duplications in sprint retrospectives
- Update utility libraries as patterns emerge

## Resources

- [Refactoring: Improving the Design of Existing Code](https://martinfowler.com/books/refactoring.html)
- [Extract Method Refactoring](https://refactoring.guru/extract-method)
- [DRY Principle](https://en.wikipedia.org/wiki/Don%27t_repeat_yourself)

---
*Generated by ExtractDuplicateCode.ps1*
"@

$outputDir = Split-Path $OutputFile -Parent
if (!(Test-Path $outputDir)) {
    New-Item -ItemType Directory -Path $outputDir -Force | Out-Null
}

$report | Out-File $OutputFile -Encoding UTF8

Write-Host "`nDuplication Extraction Plan Generated!" -ForegroundColor Green
Write-Host "Report saved to: $OutputFile" -ForegroundColor Cyan
