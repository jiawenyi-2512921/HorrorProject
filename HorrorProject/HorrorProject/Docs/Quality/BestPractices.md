# Best Practices Guide - HorrorProject

Generated: 2026-04-26
AAA Game Industry Standards for Unreal Engine 5 C++

## Overview

This guide establishes coding standards and best practices for HorrorProject, aligned with AAA game industry standards and Unreal Engine conventions.

---

## 1. Code Documentation

### File Headers

Every source file must have a header comment:

```cpp
// HorrorPlayerCharacter.h
// Copyright Epic Games, Inc. All Rights Reserved.
// 
// Player character implementation for found-footage horror gameplay.
// Manages camera recording, flashlight, interaction, and fear mechanics.
//
// Key Features:
// - VHS camera recording with battery management
// - Dynamic flashlight with battery drain
// - Context-sensitive interaction system
// - Fear level tracking and effects
//
// Usage:
//   Spawn in level or possess via GameMode
//   Configure camera/flashlight settings in Blueprint
//   Bind input actions in Project Settings
```

### API Documentation

Document all public APIs with Doxygen-style comments:

```cpp
/**
 * Attempts to interact with the object the player is looking at.
 * Performs a line trace from camera to detect interactable objects.
 * 
 * @param MaxDistance Maximum interaction distance in centimeters (default: 200cm)
 * @return true if interaction was successful, false otherwise
 * 
 * @note Requires InteractionComponent to be initialized
 * @see UInteractionComponent::TryInteract
 * 
 * Example:
 *   if (TryInteract(300.0f))
 *   {
 *       UE_LOG(LogTemp, Log, TEXT("Interaction successful"));
 *   }
 */
UFUNCTION(BlueprintCallable, Category = "Horror|Interaction")
bool TryInteract(float MaxDistance = 200.0f);
```

### Inline Comments

Use inline comments for complex logic:

```cpp
// Calculate fear increase based on threat proximity and visibility
// Closer threats and direct line of sight increase fear faster
float FearIncrease = BaseFearRate * ProximityMultiplier * VisibilityMultiplier;

// Clamp fear to prevent overflow and ensure smooth transitions
FearLevel = FMath::Clamp(FearLevel + FearIncrease * DeltaTime, 0.0f, MaxFearLevel);
```

---

## 2. Naming Conventions

### Classes

Follow Unreal Engine prefixes:

```cpp
// Actor classes
class AHorrorPlayerCharacter : public ACharacter { };
class AEvidenceActor : public AActor { };

// UObject classes
class UHorrorAudioSubsystem : public UGameInstanceSubsystem { };
class UInteractionComponent : public UActorComponent { };

// Structs
struct FEvidenceData { };
struct FObjectiveState { };

// Enums
enum class EObjectiveType : uint8 { };
enum class EThreatState : uint8 { };

// Interfaces
class IInteractableInterface { };

// Templates
template<typename T>
class THorrorContainer { };
```

### Variables

```cpp
// Member variables (lowercase start)
float CurrentBattery;
int32 PhotoCount;
bool bIsRecording;  // 'b' prefix for booleans

// Constants (PascalCase)
static constexpr float MaxBatteryLevel = 100.0f;
static const FName DefaultZoneTag = TEXT("Zone.Default");

// UPROPERTY variables (follow Blueprint conventions)
UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
float BatteryDrainRate = 1.0f;

// Private members (consider 'm' prefix for clarity)
private:
    float mCachedDeltaTime;  // Optional, team preference
```

### Functions

```cpp
// Public functions (PascalCase)
void StartRecording();
bool TryInteract();
float GetBatteryLevel() const;

// Private functions (PascalCase)
void UpdateBatteryLevel(float DeltaTime);
void BroadcastRecordingEvent();

// Blueprint functions (descriptive names)
UFUNCTION(BlueprintCallable, Category = "Camera")
void StartCameraRecording();

UFUNCTION(BlueprintPure, Category = "Camera")
float GetRemainingBatteryPercent() const;
```

---

## 3. Error Handling

### Pointer Validation

Always validate pointers before use:

```cpp
// GOOD - Safe cast with validation
if (AHorrorPlayerCharacter* Player = Cast<AHorrorPlayerCharacter>(GetOwner()))
{
    Player->AddFear(10.0f);
}

// GOOD - Using ensure for expected-valid pointers
UWorld* World = GetWorld();
if (ensure(World))
{
    World->SpawnActor<AActor>(...);
}

// GOOD - Using check for must-be-valid pointers
void Initialize(UObject* RequiredObject)
{
    check(RequiredObject);  // Fatal error if null
    RequiredObject->DoSomething();
}

// BAD - No validation
AActor* Target = Cast<AActor>(Object);
Target->SetActorLocation(Loc);  // CRASH if cast fails!
```

### Chained Calls

Validate each step in chained calls:

```cpp
// BAD - Unsafe chaining
GetWorld()->GetFirstPlayerController()->GetPawn()->SetActorLocation(Loc);

// GOOD - Validated chaining
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

### Array Access

Always check bounds:

```cpp
// BAD - No bounds check
FEvidence Evidence = EvidenceArray[Index];

// GOOD - Bounds check
if (EvidenceArray.IsValidIndex(Index))
{
    FEvidence Evidence = EvidenceArray[Index];
}

// GOOD - Safe access with default
FEvidence Evidence = EvidenceArray.IsValidIndex(Index) 
    ? EvidenceArray[Index] 
    : FEvidence::Default;
```

---

## 4. Memory Management

### UObject Pointers

Always use UPROPERTY for UObject pointers:

```cpp
// GOOD - GC tracked
UPROPERTY()
AActor* TargetActor;

UPROPERTY()
UActorComponent* CachedComponent;

// BAD - Not tracked by GC, will cause crashes!
AActor* TargetActor;
```

### Smart Pointers

Use appropriate smart pointer types:

```cpp
// For shared ownership
TSharedPtr<FMyData> SharedData = MakeShared<FMyData>();

// For unique ownership
TUniquePtr<FMyData> UniqueData = MakeUnique<FMyData>();

// For weak references (doesn't prevent deletion)
TWeakPtr<FMyData> WeakData = SharedData;

// For UObject weak references
UPROPERTY()
TWeakObjectPtr<AActor> WeakActor;
```

### Avoid Raw new/delete

```cpp
// BAD - Manual memory management
FMyData* Data = new FMyData();
delete Data;

// GOOD - Use NewObject for UObjects
UMyObject* Object = NewObject<UMyObject>(this);

// GOOD - Use smart pointers for non-UObjects
TUniquePtr<FMyData> Data = MakeUnique<FMyData>();
```

---

## 5. Performance Best Practices

### Parameter Passing

```cpp
// Primitives by value
void SetHealth(float NewHealth);
void SetCount(int32 Count);
void SetEnabled(bool bEnabled);

// Large types by const reference (input)
void ProcessTag(const FGameplayTag& Tag);
void UpdateEvidence(const TArray<FEvidence>& Evidence);
void ApplySettings(const FGameSettings& Settings);

// Output parameters by pointer or reference
void GetEvidence(TArray<FEvidence>& OutEvidence) const;
void GetPlayerInfo(FPlayerInfo* OutInfo) const;
```

### Caching

Cache expensive lookups:

```cpp
// BAD - GetWorld() every frame
void Tick(float DeltaTime)
{
    GetWorld()->GetTimeSeconds();
    GetWorld()->SpawnActor(...);
}

// GOOD - Cache World pointer
void BeginPlay()
{
    Super::BeginPlay();
    CachedWorld = GetWorld();
}

void Tick(float DeltaTime)
{
    if (!CachedWorld) return;
    CachedWorld->GetTimeSeconds();
}
```

### Avoid Allocations in Hot Paths

```cpp
// BAD - String allocation in loop
for (const AActor* Actor : Actors)
{
    FString Name = Actor->GetName();  // Allocation every iteration!
}

// GOOD - Reuse string
FString Name;
for (const AActor* Actor : Actors)
{
    Name = Actor->GetName();
}

// BETTER - Use FName if possible
for (const AActor* Actor : Actors)
{
    FName Name = Actor->GetFName();  // No allocation
}
```

### Tick Optimization

```cpp
// Only tick when necessary
PrimaryComponentTick.bCanEverTick = false;  // Disable if not needed

// Use tick interval for non-critical updates
PrimaryComponentTick.TickInterval = 0.1f;  // Update every 100ms

// Conditional ticking
void Tick(float DeltaTime)
{
    if (!bNeedsUpdate) return;  // Early exit
    
    // Expensive operations only when needed
    UpdateComplexState();
}
```

---

## 6. Thread Safety

### Const Static Variables

```cpp
// GOOD - Thread-safe
static const FName DefaultZone = TEXT("MainZone");
static constexpr float MaxDistance = 1000.0f;

// BAD - Not thread-safe
static FName CurrentZone;  // Mutable shared state!
static float GlobalVolume;
```

### Container Protection

```cpp
// Shared container with lock
class UMySubsystem
{
private:
    TArray<FEvidence> Evidence;
    FCriticalSection EvidenceLock;

public:
    void AddEvidence(const FEvidence& NewEvidence)
    {
        FScopeLock Lock(&EvidenceLock);
        Evidence.Add(NewEvidence);
    }
    
    void GetEvidence(TArray<FEvidence>& OutEvidence) const
    {
        FScopeLock Lock(&EvidenceLock);
        OutEvidence = Evidence;
    }
};
```

### Atomic Operations

```cpp
// For simple counters
TAtomic<int32> ActiveThreadCount{0};

void IncrementCounter()
{
    ActiveThreadCount++;  // Thread-safe
}
```

---

## 7. Blueprint Integration

### Blueprint-Friendly APIs

```cpp
// Clear, descriptive names
UFUNCTION(BlueprintCallable, Category = "Horror|Camera")
void StartCameraRecording();

// Pure functions for getters (no execution pin)
UFUNCTION(BlueprintPure, Category = "Horror|Camera")
float GetBatteryPercent() const;

// Proper categories for organization
UFUNCTION(BlueprintCallable, Category = "Horror|Audio|Playback")
void PlayAmbientSound(FGameplayTag ZoneTag);

// Meta specifiers for better UX
UFUNCTION(BlueprintCallable, Category = "Horror|Interaction", 
    meta = (DisplayName = "Try Interact", Keywords = "use activate"))
bool TryInteract();
```

### UPROPERTY Best Practices

```cpp
// EditAnywhere - Editable in Blueprint and instances
UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
float BatteryDrainRate = 1.0f;

// EditDefaultsOnly - Only in Blueprint, not instances
UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera")
TSubclassOf<UCameraShakeBase> RecordingShake;

// VisibleAnywhere - Read-only display
UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
float CurrentBattery;

// Meta specifiers for validation
UPROPERTY(EditAnywhere, Category = "Camera", 
    meta = (ClampMin = "0.0", ClampMax = "100.0", UIMin = "0.0", UIMax = "100.0"))
float BatteryLevel = 100.0f;

// Tooltips for designers
UPROPERTY(EditAnywhere, Category = "Camera",
    meta = (Tooltip = "Rate at which battery drains per second while recording"))
float BatteryDrainRate = 1.0f;
```

---

## 8. Code Organization

### Header File Structure

```cpp
// HorrorPlayerCharacter.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "HorrorPlayerCharacter.generated.h"  // MUST be last!

/**
 * Player character for found-footage horror gameplay.
 */
UCLASS()
class HORRORPROJECT_API AHorrorPlayerCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    // Constructor
    AHorrorPlayerCharacter();

    // AActor interface
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void SetupPlayerInputComponent(UInputComponent* Input) override;

    // Public API
    UFUNCTION(BlueprintCallable, Category = "Horror|Camera")
    void StartRecording();

protected:
    // Protected API
    virtual void OnRecordingStarted();

    // Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UCameraRecordingComponent* CameraComponent;

private:
    // Private implementation
    void UpdateBattery(float DeltaTime);
    
    // Private data
    float CurrentBattery;
    bool bIsRecording;
};
```

### Implementation File Structure

```cpp
// HorrorPlayerCharacter.cpp
#include "HorrorPlayerCharacter.h"
#include "Components/CameraRecordingComponent.h"
#include "Kismet/GameplayStatics.h"

AHorrorPlayerCharacter::AHorrorPlayerCharacter()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Component initialization
    CameraComponent = CreateDefaultSubobject<UCameraRecordingComponent>(TEXT("CameraComponent"));
}

void AHorrorPlayerCharacter::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialization logic
    CurrentBattery = 100.0f;
}

void AHorrorPlayerCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (bIsRecording)
    {
        UpdateBattery(DeltaTime);
    }
}

void AHorrorPlayerCharacter::StartRecording()
{
    if (CurrentBattery <= 0.0f) return;
    
    bIsRecording = true;
    OnRecordingStarted();
}

void AHorrorPlayerCharacter::OnRecordingStarted()
{
    // Override point for subclasses
}

void AHorrorPlayerCharacter::UpdateBattery(float DeltaTime)
{
    CurrentBattery = FMath::Max(0.0f, CurrentBattery - DeltaTime);
}
```

---

## 9. Constants and Configuration

### Named Constants

```cpp
// In header file
class UHorrorGameSettings
{
public:
    // Gameplay constants
    static constexpr float MaxBatteryLevel = 100.0f;
    static constexpr float MinBatteryLevel = 0.0f;
    static constexpr float DefaultBatteryDrain = 1.0f;
    
    // AI constants
    static constexpr float ThreatDetectionRange = 800.0f;
    static constexpr float ThreatChaseSpeed = 600.0f;
    static constexpr float ThreatPatrolSpeed = 200.0f;
    
    // Audio constants
    static constexpr float MaxAudioDistance = 5000.0f;
    static constexpr float DefaultVolume = 1.0f;
};
```

### Designer-Tunable Values

```cpp
// Use UPROPERTY for values designers should tune
UPROPERTY(EditDefaultsOnly, Category = "Camera|Battery",
    meta = (ClampMin = "0.1", ClampMax = "10.0", 
            Tooltip = "Battery drain rate per second while recording"))
float BatteryDrainRate = 1.0f;

UPROPERTY(EditDefaultsOnly, Category = "AI|Detection",
    meta = (ClampMin = "100.0", ClampMax = "5000.0",
            Tooltip = "Maximum distance at which threat can detect player"))
float DetectionRange = 800.0f;
```

---

## 10. Testing Best Practices

### Unit Tests

```cpp
// HorrorPlayerCharacterTests.cpp
#include "Misc/AutomationTest.h"
#include "Player/HorrorPlayerCharacter.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FHorrorPlayerCharacterTest,
    "HorrorProject.Player.HorrorPlayerCharacter.BatteryDrain",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter
)

bool FHorrorPlayerCharacterTest::RunTest(const FString& Parameters)
{
    // Arrange
    AHorrorPlayerCharacter* Player = NewObject<AHorrorPlayerCharacter>();
    Player->SetBatteryLevel(100.0f);
    
    // Act
    Player->StartRecording();
    Player->Tick(1.0f);  // 1 second
    
    // Assert
    TestTrue("Battery should drain", Player->GetBatteryLevel() < 100.0f);
    TestEqual("Battery drain rate", Player->GetBatteryLevel(), 99.0f, 0.1f);
    
    return true;
}
```

### Integration Tests

```cpp
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FInteractionSystemTest,
    "HorrorProject.Integration.InteractionSystem",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter
)

bool FInteractionSystemTest::RunTest(const FString& Parameters)
{
    // Test full interaction flow
    // Player -> Interaction Component -> Interactable -> Event Bus
    
    return true;
}
```

---

## 11. Logging and Debugging

### Log Categories

```cpp
// HorrorProject.h
DECLARE_LOG_CATEGORY_EXTERN(LogHorrorPlayer, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogHorrorAI, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogHorrorAudio, Log, All);

// HorrorProject.cpp
DEFINE_LOG_CATEGORY(LogHorrorPlayer);
DEFINE_LOG_CATEGORY(LogHorrorAI);
DEFINE_LOG_CATEGORY(LogHorrorAudio);
```

### Logging Best Practices

```cpp
// Informational
UE_LOG(LogHorrorPlayer, Log, TEXT("Recording started"));

// Warning
UE_LOG(LogHorrorPlayer, Warning, TEXT("Battery low: %.1f%%"), BatteryPercent);

// Error
UE_LOG(LogHorrorPlayer, Error, TEXT("Failed to start recording: %s"), *Reason);

// Verbose (only in debug builds)
UE_LOG(LogHorrorPlayer, Verbose, TEXT("Tick: DeltaTime=%.3f"), DeltaTime);

// With object context
UE_LOG(LogHorrorPlayer, Log, TEXT("%s: Recording started"), *GetName());
```

### Debug Drawing

```cpp
#if ENABLE_DRAW_DEBUG
void DrawDebugInfo()
{
    if (UWorld* World = GetWorld())
    {
        // Draw detection range
        DrawDebugSphere(World, GetActorLocation(), DetectionRange, 32, 
            FColor::Yellow, false, 0.0f, 0, 2.0f);
        
        // Draw line trace
        DrawDebugLine(World, Start, End, FColor::Red, false, 1.0f, 0, 2.0f);
    }
}
#endif
```

---

## 12. Code Review Checklist

Before submitting code for review:

### Documentation
- [ ] File header present
- [ ] All public APIs documented
- [ ] Complex logic has inline comments
- [ ] Usage examples provided

### Error Handling
- [ ] All pointers validated before use
- [ ] Cast<> results checked
- [ ] Array bounds checked
- [ ] ensure() or check() used appropriately

### Performance
- [ ] Large types passed by const reference
- [ ] Expensive calls cached
- [ ] No allocations in hot paths
- [ ] Tick optimized or disabled

### Memory Management
- [ ] UObject pointers have UPROPERTY
- [ ] Smart pointers used for non-UObjects
- [ ] No raw new/delete
- [ ] No memory leaks

### Thread Safety
- [ ] Static variables are const
- [ ] Shared data protected
- [ ] Thread safety documented

### Blueprint Integration
- [ ] BlueprintCallable has Category
- [ ] UPROPERTY has appropriate specifiers
- [ ] Meta tags for better UX
- [ ] Tooltips for designers

### Code Quality
- [ ] No magic numbers
- [ ] Functions under 50 lines
- [ ] Complexity under 10
- [ ] Follows naming conventions

### Testing
- [ ] Unit tests written
- [ ] Tests pass
- [ ] Edge cases covered

---

## 13. Common Pitfalls to Avoid

### Don't
```cpp
// DON'T use raw pointers for UObjects without UPROPERTY
AActor* MyActor;  // Will be garbage collected!

// DON'T ignore Cast<> results
Cast<AActor>(Object)->DoSomething();  // CRASH!

// DON'T use magic numbers
if (Distance < 800.0f) { }  // What is 800?

// DON'T pass large types by value
void Process(TArray<FData> Data) { }  // Copies entire array!

// DON'T use non-const statics
static FName CurrentState;  // Thread-unsafe!

// DON'T call GetWorld() repeatedly in Tick
void Tick(float DT) { GetWorld()->...; GetWorld()->...; }
```

### Do
```cpp
// DO use UPROPERTY for UObject pointers
UPROPERTY()
AActor* MyActor;

// DO validate Cast<> results
if (AActor* Actor = Cast<AActor>(Object))
{
    Actor->DoSomething();
}

// DO use named constants
static constexpr float DetectionRange = 800.0f;
if (Distance < DetectionRange) { }

// DO pass large types by const reference
void Process(const TArray<FData>& Data) { }

// DO use const statics
static const FName DefaultState = TEXT("Idle");

// DO cache GetWorld() results
UWorld* World = GetWorld();
if (World) { World->...; World->...; }
```

---

## 14. Resources

### Official Documentation
- [Unreal Engine Coding Standard](https://docs.unrealengine.com/5.3/en-US/epic-cplusplus-coding-standard-for-unreal-engine/)
- [Unreal Engine API Reference](https://docs.unrealengine.com/5.3/en-US/API/)
- [C++ Core Guidelines](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines)

### Recommended Reading
- Effective Modern C++ by Scott Meyers
- Game Programming Patterns by Robert Nystrom
- Clean Code by Robert C. Martin
- Unreal Engine 5 C++ Developer Course (Udemy)

### Tools
- Visual Studio / Rider for Unreal Engine
- ReSharper C++ for code analysis
- Unreal Insights for profiling
- PVS-Studio for static analysis

---

*Follow these best practices to maintain AAA-quality code. Update this guide as the team learns and evolves.*
