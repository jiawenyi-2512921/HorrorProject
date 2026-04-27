# Coding Standards

## Overview

This document defines the coding standards for HorrorProject. Following these standards ensures code consistency, readability, and maintainability.

## General Principles

1. **Clarity over cleverness** - Write code that is easy to understand
2. **Consistency** - Follow existing patterns in the codebase
3. **Simplicity** - Prefer simple solutions over complex ones
4. **Performance** - Consider performance implications
5. **Testability** - Write code that can be easily tested

## Naming Conventions

### Classes

**Unreal Prefixes:**
- `U` - UObject-derived classes (e.g., `UHorrorEventBusSubsystem`)
- `A` - AActor-derived classes (e.g., `AHorrorEncounterDirector`)
- `F` - Plain structs (e.g., `FHorrorEventMessage`)
- `E` - Enums (e.g., `EHorrorEncounterPhase`)
- `I` - Interfaces (e.g., `IHorrorInteractable`)
- `T` - Templates (e.g., `TArray`, `TMap`)

**Examples:**
```cpp
class UHorrorAudioSubsystem : public UWorldSubsystem { };
class AHorrorThreatCharacter : public ACharacter { };
struct FEvidenceData { };
enum class EEvidenceType : uint8 { };
class IHorrorInteractable { };
```

### Variables

**Member Variables:**
```cpp
class AMyActor : public AActor
{
private:
    // Private members (no prefix in modern UE)
    int32 EncounterCount;
    float RevealRadius;
    
    // UPROPERTY members
    UPROPERTY()
    TObjectPtr<AActor> TargetActor;
    
    // Boolean members (use 'b' prefix)
    bool bIsActive;
    bool bGateRouteDuringReveal;
};
```

**Local Variables:**
```cpp
void MyFunction()
{
    int32 LocalCounter = 0;
    float DeltaTime = 0.0f;
    bool bIsValid = false;
    AActor* TargetActor = nullptr;
}
```

**Constants:**
```cpp
// Global constants (use 'G' prefix)
const float GDefaultRevealRadius = 1200.0f;

// Static constants
static const FName DefaultEncounterId = TEXT("Encounter_Default");

// Constexpr
constexpr int32 MaxHistoryCapacity = 128;
```

### Functions

**PascalCase for all functions:**
```cpp
void PrimeEncounter(FName EncounterId);
bool CanTriggerReveal(const AActor* PlayerActor) const;
float GetRevealRadius() const;
void SetRevealRadius(float NewRadius);
```

**Blueprint Functions:**
```cpp
UFUNCTION(BlueprintCallable, Category="Horror|Encounter")
bool TriggerReveal(AActor* PlayerActor);

UFUNCTION(BlueprintPure, Category="Horror|Encounter")
EHorrorEncounterPhase GetEncounterPhase() const;

UFUNCTION(BlueprintImplementableEvent, Category="Horror|Encounter")
void BP_OnEncounterRevealed(FName EncounterId);
```

### Enums

```cpp
UENUM(BlueprintType)
enum class EHorrorEncounterPhase : uint8
{
    Dormant UMETA(DisplayName="Dormant"),
    Primed UMETA(DisplayName="Primed"),
    Revealed UMETA(DisplayName="Revealed"),
    Resolved UMETA(DisplayName="Resolved")
};
```

### Gameplay Tags

**Hierarchical naming:**
```
Event.Encounter.Primed
Event.Encounter.Revealed
Event.Encounter.Resolved
Event.Objective.Started
Event.Objective.Completed
Event.Evidence.Collected
State.Inactive
State.Active
State.Completed
```

## Code Organization

### File Structure

**Header File (.h):**
```cpp
// Copyright notice

#pragma once

#include "CoreMinimal.h"
#include "ParentClass.h"
#include "MyClass.generated.h"

// Forward declarations
class UHorrorEventBusSubsystem;
class AHorrorThreatCharacter;

// Delegates
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMyDelegate, int32, Value);

// Enums
UENUM(BlueprintType)
enum class EMyEnum : uint8 { };

// Structs
USTRUCT(BlueprintType)
struct FMyStruct
{
    GENERATED_BODY()
    
    UPROPERTY(BlueprintReadOnly, Category="MyCategory")
    int32 Value;
};

// Class declaration
UCLASS()
class HORRORPROJECT_API UMyClass : public UParentClass
{
    GENERATED_BODY()

public:
    // Public interface
    
protected:
    // Protected members
    
private:
    // Private implementation
};
```

**Source File (.cpp):**
```cpp
// Copyright notice

#include "MyClass.h"
#include "OtherIncludes.h"

// Static variables

// Constructor
UMyClass::UMyClass()
{
    // Initialization
}

// Public functions

// Protected functions

// Private functions
```

### Include Order

1. Own header file
2. Unreal Engine headers
3. Project headers
4. Third-party headers

```cpp
#include "MyClass.h"

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "Game/HorrorEventBusSubsystem.h"
#include "Player/HorrorProjectCharacter.h"

#include "ThirdParty/SomeLibrary.h"
```

## Formatting

### Indentation

- Use tabs (not spaces)
- Tab size: 4 spaces equivalent
- Indent each level consistently

### Braces

**K&R style (opening brace on same line):**
```cpp
void MyFunction() {
    if (bCondition) {
        DoSomething();
    } else {
        DoSomethingElse();
    }
}
```

**Exception: Class and function definitions:**
```cpp
class UMyClass : public UObject
{
    GENERATED_BODY()
    
public:
    void MyFunction()
    {
        // Implementation
    }
};
```

### Spacing

**Operators:**
```cpp
int32 Result = A + B * C;
bool bIsValid = (Value > 0) && (Value < 100);
```

**Function calls:**
```cpp
MyFunction(Param1, Param2, Param3);
```

**Pointers and references:**
```cpp
AActor* Actor = nullptr;
const FString& Name = GetName();
```

### Line Length

- Maximum 120 characters per line
- Break long lines logically

```cpp
EventBus->Publish(
    FGameplayTag::RequestGameplayTag("Event.Encounter.Revealed"),
    ActiveEncounterId,
    FGameplayTag::RequestGameplayTag("State.Active"),
    this
);
```

## Comments

### Header Comments

```cpp
/**
 * Manages horror encounters and threat reveals.
 * 
 * The Encounter Director controls the state machine for horror encounters,
 * spawning threats, playing audio, and publishing events to the Event Bus.
 */
UCLASS()
class HORRORPROJECT_API AHorrorEncounterDirector : public AActor
{
    GENERATED_BODY()
};
```

### Function Comments

```cpp
/**
 * Triggers the reveal phase of the encounter.
 * 
 * @param PlayerActor The player actor that triggered the reveal
 * @return true if reveal was triggered successfully, false otherwise
 */
UFUNCTION(BlueprintCallable, Category="Horror|Encounter")
bool TriggerReveal(AActor* PlayerActor);
```

### Inline Comments

```cpp
// Check if player is within reveal radius
const float Distance = FVector::Dist(GetActorLocation(), PlayerActor->GetActorLocation());
if (Distance <= RevealRadius) {
    // Trigger reveal sequence
    TriggerReveal(PlayerActor);
}
```

### Tracked Follow-Up Comments

```cpp
// FOLLOWUP(UE-12345): Implement network replication for multiplayer
// DEFECT(UE-12410): Resolve leak in evidence collection
// TEMPORARY(UE-12422): Workaround for engine regression in editor preview
// NOTE: This must be called before BeginPlay
```

Every follow-up comment must include an issue ID and an owner in the tracking system. Raw reminder markers are not allowed in committed runtime code.

## Best Practices

### Memory Management

**Use smart pointers:**
```cpp
// For UObjects
UPROPERTY()
TObjectPtr<AActor> MyActor;

// For weak references
TWeakObjectPtr<AActor> WeakActor;

// For shared ownership (rare)
TSharedPtr<FMyData> SharedData;
```

**Avoid raw pointers:**
```cpp
// Bad
AActor* Actor = new AActor();

// Good
AActor* Actor = GetWorld()->SpawnActor<AActor>();
```

### Const Correctness

```cpp
// Const member functions
float GetRevealRadius() const { return RevealRadius; }

// Const parameters
void ProcessActor(const AActor* Actor) const;

// Const references
void ProcessName(const FString& Name);
```

### Null Checks

```cpp
// Always check pointers before use
if (AActor* Actor = GetTargetActor()) {
    Actor->DoSomething();
}

// Use ensure for unexpected nulls
if (ensure(EventBus != nullptr)) {
    EventBus->Publish(...);
}

// Use check for critical nulls (crashes in development)
check(World != nullptr);
```

### Error Handling

```cpp
// Return bool for success/failure
bool TriggerReveal(AActor* PlayerActor)
{
    if (!PlayerActor) {
        UE_LOG(LogHorrorProject, Warning, TEXT("TriggerReveal: Invalid PlayerActor"));
        return false;
    }
    
    if (EncounterPhase != EHorrorEncounterPhase::Primed) {
        UE_LOG(LogHorrorProject, Warning, TEXT("TriggerReveal: Encounter not primed"));
        return false;
    }
    
    // Success path
    return true;
}
```

### Logging

```cpp
// Define log category in module header
DECLARE_LOG_CATEGORY_EXTERN(LogHorrorProject, Log, All);

// Use appropriate log level
UE_LOG(LogHorrorProject, Log, TEXT("Encounter primed: %s"), *EncounterId.ToString());
UE_LOG(LogHorrorProject, Warning, TEXT("Invalid encounter state"));
UE_LOG(LogHorrorProject, Error, TEXT("Failed to spawn threat actor"));

// Use verbose for detailed logging
UE_LOG(LogHorrorProject, Verbose, TEXT("Processing event: %s"), *EventTag.ToString());
```

### Performance

**Avoid expensive operations in Tick:**
```cpp
// Bad
void Tick(float DeltaTime) override
{
    TArray<AActor*> Actors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), Actors);
}

// Good - cache results, use timers
void BeginPlay() override
{
    GetWorldTimerManager().SetTimer(UpdateTimerHandle, this, &AMyActor::UpdateActors, 1.0f, true);
}
```

**Use const references for large types:**
```cpp
// Bad
void ProcessMessage(FHorrorEventMessage Message);

// Good
void ProcessMessage(const FHorrorEventMessage& Message);
```

**Prefer forward declarations:**
```cpp
// In header - forward declare
class UHorrorEventBusSubsystem;

// In cpp - include
#include "Game/HorrorEventBusSubsystem.h"
```

## Blueprint Integration

### Exposing to Blueprint

```cpp
// Callable functions
UFUNCTION(BlueprintCallable, Category="Horror|Encounter")
bool TriggerReveal(AActor* PlayerActor);

// Pure functions (no side effects)
UFUNCTION(BlueprintPure, Category="Horror|Encounter")
float GetRevealRadius() const;

// Implementable events
UFUNCTION(BlueprintImplementableEvent, Category="Horror|Encounter")
void BP_OnEncounterRevealed(FName EncounterId);

// Native events (C++ + Blueprint)
UFUNCTION(BlueprintNativeEvent, Category="Horror|Encounter")
void OnRevealSequenceStart(AActor* PlayerActor);
```

### Properties

```cpp
// Read-only in Blueprint
UPROPERTY(BlueprintReadOnly, Category="Horror|Encounter")
EHorrorEncounterPhase EncounterPhase;

// Read-write in Blueprint
UPROPERTY(BlueprintReadWrite, Category="Horror|Encounter")
float RevealRadius;

// Editable in editor
UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Encounter")
TSubclassOf<AHorrorThreatCharacter> ThreatClass;

// Visible but not editable
UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Horror|Encounter")
FName ActiveEncounterId;
```

## Testing

### Unit Test Structure

```cpp
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FHorrorEncounterDirectorTest,
    "HorrorProject.Game.EncounterDirector.PrimeEncounter",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter
)

bool FHorrorEncounterDirectorTest::RunTest(const FString& Parameters)
{
    // Arrange
    UWorld* World = CreateTestWorld();
    AHorrorEncounterDirector* Director = World->SpawnActor<AHorrorEncounterDirector>();
    
    // Act
    bool bResult = Director->PrimeEncounter(TEXT("Test_Encounter"));
    
    // Assert
    TestTrue("Encounter primed successfully", bResult);
    TestEqual("Encounter phase is Primed", Director->GetEncounterPhase(), EHorrorEncounterPhase::Primed);
    
    return true;
}
```

## Code Review Checklist

- [ ] Follows naming conventions
- [ ] Properly formatted
- [ ] Includes appropriate comments
- [ ] No memory leaks
- [ ] Const correctness
- [ ] Null checks
- [ ] Error handling
- [ ] Performance considerations
- [ ] Unit tests included
- [ ] Blueprint integration tested

---
Last Updated: 2026-04-26
