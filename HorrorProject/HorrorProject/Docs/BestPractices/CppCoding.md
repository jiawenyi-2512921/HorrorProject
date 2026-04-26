# C++ Coding Best Practices

Best practices for writing high-quality C++ code in HorrorProject.

## Code Style

### Naming Conventions

Follow Unreal Engine naming conventions:

```cpp
// Classes: Prefix with A (Actor), U (Object), F (Struct), E (Enum), I (Interface)
class AEvidenceActor : public AActor { };
class UEvidenceCollectionComponent : public UActorComponent { };
struct FEvidenceData { };
enum class EEvidenceCategory : uint8 { };
class IInteractable { };

// Member variables: Prefix with 'b' for bool, no prefix for others
bool bIsCollected;
int32 EvidenceCount;
float FadeInTime;
FString EvidenceName;

// Private members: No special prefix (Unreal style)
private:
    TArray<FEvidenceData> CollectedEvidence;
    
// Function names: PascalCase
void CollectEvidence();
bool HasEvidence() const;
```

### Header Organization

```cpp
// EvidenceCollectionComponent.h
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EvidenceTypes.h"  // Project includes
#include "EvidenceCollectionComponent.generated.h"  // Generated last

/**
 * Component for managing evidence collection
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class HORRORPROJECT_API UEvidenceCollectionComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    // Constructors
    UEvidenceCollectionComponent();

    // Public functions
    UFUNCTION(BlueprintCallable, Category = "Evidence")
    bool CollectEvidence(AEvidenceActor* Evidence);

    // Public properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    int32 MaxCapacity;

protected:
    // Protected functions
    virtual void BeginPlay() override;

private:
    // Private functions
    void SaveToArchive();
    
    // Private properties
    TArray<FEvidenceData> CollectedEvidence;
};
```

### CPP File Organization

```cpp
// EvidenceCollectionComponent.cpp
#include "Evidence/EvidenceCollectionComponent.h"
#include "Evidence/EvidenceActor.h"
#include "Evidence/ArchiveSubsystem.h"

// Constructor
UEvidenceCollectionComponent::UEvidenceCollectionComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    MaxCapacity = 100;
}

// BeginPlay
void UEvidenceCollectionComponent::BeginPlay()
{
    Super::BeginPlay();
    LoadFromArchive();
}

// Public functions
bool UEvidenceCollectionComponent::CollectEvidence(AEvidenceActor* Evidence)
{
    if (!Evidence || CollectedEvidence.Num() >= MaxCapacity)
    {
        return false;
    }
    
    // Implementation
    return true;
}

// Private functions
void UEvidenceCollectionComponent::SaveToArchive()
{
    // Implementation
}
```

## Memory Management

### Smart Pointers

```cpp
// Use TSharedPtr for shared ownership
TSharedPtr<FMyData> SharedData = MakeShared<FMyData>();

// Use TWeakPtr to avoid circular references
TWeakPtr<FMyData> WeakData = SharedData;

// Use TUniquePtr for exclusive ownership
TUniquePtr<FMyData> UniqueData = MakeUnique<FMyData>();

// Check weak pointers before use
if (TSharedPtr<FMyData> PinnedData = WeakData.Pin())
{
    // Use PinnedData safely
}
```

### Object References

```cpp
// Use UPROPERTY for UObject references (prevents garbage collection)
UPROPERTY()
UEvidenceCollectionComponent* EvidenceComponent;

// Use TWeakObjectPtr for optional references
TWeakObjectPtr<AEvidenceActor> WeakEvidence;

// Check validity before use
if (EvidenceComponent && EvidenceComponent->IsValidLowLevel())
{
    EvidenceComponent->CollectEvidence(Evidence);
}
```

### Avoid Memory Leaks

```cpp
// Bad: Manual memory management
FMyData* Data = new FMyData();
// ... might leak if exception occurs

// Good: Use smart pointers
TUniquePtr<FMyData> Data = MakeUnique<FMyData>();
// Automatically cleaned up

// Good: Use Unreal's object system
UMyObject* Object = NewObject<UMyObject>(this);
// Garbage collected automatically
```

## Performance

### Minimize Tick Functions

```cpp
// Bad: Expensive operations in Tick
void AMyActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Expensive operation every frame!
    TArray<AActor*> Actors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), Actors);
}

// Good: Use events or timers
void AMyActor::BeginPlay()
{
    Super::BeginPlay();
    
    // Check every 0.5 seconds instead
    GetWorldTimerManager().SetTimer(
        CheckTimerHandle,
        this,
        &AMyActor::PerformCheck,
        0.5f,
        true
    );
}
```

### Cache Component References

```cpp
// Bad: Repeated lookups
void AMyCharacter::Tick(float DeltaTime)
{
    UEvidenceCollectionComponent* Evidence = FindComponentByClass<UEvidenceCollectionComponent>();
    if (Evidence)
    {
        // Use Evidence
    }
}

// Good: Cache in BeginPlay
void AMyCharacter::BeginPlay()
{
    Super::BeginPlay();
    CachedEvidenceComponent = FindComponentByClass<UEvidenceCollectionComponent>();
}

void AMyCharacter::Tick(float DeltaTime)
{
    if (CachedEvidenceComponent)
    {
        // Use cached reference
    }
}
```

### Use Const Correctness

```cpp
// Mark functions const when they don't modify state
bool HasEvidence(FName EvidenceID) const;
int32 GetEvidenceCount() const;

// Use const references for parameters
void ProcessEvidence(const FEvidenceData& Evidence);

// Use const for read-only pointers
void DisplayEvidence(const AEvidenceActor* Evidence) const;
```

### Prefer TArray over TMap for Small Collections

```cpp
// Bad: TMap for small collections (overhead)
TMap<FName, FEvidenceData> SmallCollection;  // < 10 items

// Good: TArray with Find
TArray<FEvidenceData> SmallCollection;
FEvidenceData* Found = SmallCollection.FindByPredicate([&](const FEvidenceData& Data) {
    return Data.ID == SearchID;
});
```

## Error Handling

### Validate Input

```cpp
bool UEvidenceCollectionComponent::CollectEvidence(AEvidenceActor* Evidence)
{
    // Validate input
    if (!Evidence)
    {
        UE_LOG(LogEvidence, Warning, TEXT("CollectEvidence: Evidence is null"));
        return false;
    }
    
    if (!Evidence->IsValidLowLevel())
    {
        UE_LOG(LogEvidence, Error, TEXT("CollectEvidence: Evidence is invalid"));
        return false;
    }
    
    // Proceed with collection
    return true;
}
```

### Use Check Macros

```cpp
// check() - Fatal error in development builds
check(Evidence != nullptr);

// checkSlow() - Only in debug builds
checkSlow(EvidenceCount >= 0);

// ensure() - Non-fatal, logs and continues
if (!ensure(EvidenceComponent))
{
    return;  // Handle gracefully
}

// verify() - Like ensure but evaluates in shipping
if (!verify(LoadData()))
{
    // Handle error
}
```

### Logging

```cpp
// Define log category in .h
DECLARE_LOG_CATEGORY_EXTERN(LogEvidence, Log, All);

// Define in .cpp
DEFINE_LOG_CATEGORY(LogEvidence);

// Use appropriate log levels
UE_LOG(LogEvidence, Verbose, TEXT("Detailed debug info"));
UE_LOG(LogEvidence, Log, TEXT("General information"));
UE_LOG(LogEvidence, Warning, TEXT("Warning: %s"), *WarningMessage);
UE_LOG(LogEvidence, Error, TEXT("Error: %s"), *ErrorMessage);
UE_LOG(LogEvidence, Fatal, TEXT("Fatal error - will crash"));
```

## Blueprint Integration

### Expose to Blueprint

```cpp
// Functions
UFUNCTION(BlueprintCallable, Category = "Evidence")
bool CollectEvidence(AEvidenceActor* Evidence);

UFUNCTION(BlueprintPure, Category = "Evidence")
int32 GetEvidenceCount() const;

// Properties
UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
int32 MaxCapacity;

UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
int32 CurrentCount;

// Events
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEvidenceCollected, const FEvidenceData&, Evidence);

UPROPERTY(BlueprintAssignable, Category = "Events")
FOnEvidenceCollected OnEvidenceCollected;
```

### Blueprint-Friendly Types

```cpp
// Use Blueprint-compatible types
UFUNCTION(BlueprintCallable)
void SetData(
    int32 IntValue,           // Good
    float FloatValue,         // Good
    FString StringValue,      // Good
    FName NameValue,          // Good
    TArray<int32> ArrayValue  // Good
);

// Avoid non-Blueprint types
void SetData(
    std::vector<int> Data,    // Bad - not Blueprint compatible
    std::string Text          // Bad - use FString instead
);
```

## Thread Safety

### Mark Thread-Safe Functions

```cpp
// Use appropriate thread safety specifiers
UFUNCTION(BlueprintCallable, Category = "Evidence", meta = (ThreadSafe))
int32 GetEvidenceCount() const;
```

### Use Locks for Shared Data

```cpp
class UMySubsystem : public UWorldSubsystem
{
private:
    FCriticalSection DataLock;
    TArray<FData> SharedData;

public:
    void AddData(const FData& Data)
    {
        FScopeLock Lock(&DataLock);
        SharedData.Add(Data);
    }
    
    TArray<FData> GetData() const
    {
        FScopeLock Lock(&DataLock);
        return SharedData;
    }
};
```

## Documentation

### Comment Guidelines

```cpp
/**
 * Collects an evidence item and adds it to the player's inventory
 * 
 * @param Evidence The evidence actor to collect
 * @return true if collection succeeded, false if failed (null, full, etc.)
 */
UFUNCTION(BlueprintCallable, Category = "Evidence")
bool CollectEvidence(AEvidenceActor* Evidence);

// Use inline comments for complex logic
void ProcessEvidence()
{
    // Sort by priority before processing
    Evidence.Sort([](const FEvidenceData& A, const FEvidenceData& B) {
        return A.Priority > B.Priority;
    });
    
    // Process high-priority items first
    for (const FEvidenceData& Data : Evidence)
    {
        // ...
    }
}
```

## Common Pitfalls

### Avoid Circular Dependencies

```cpp
// Bad: Circular include
// A.h includes B.h
// B.h includes A.h

// Good: Forward declare
// A.h
class UClassB;  // Forward declaration

class UClassA
{
    UClassB* BReference;
};

// A.cpp
#include "ClassB.h"  // Include in CPP
```

### Initialize Variables

```cpp
// Bad: Uninitialized
int32 Count;
float Time;

// Good: Initialize
int32 Count = 0;
float Time = 0.0f;

// Good: Constructor initialization list
UMyComponent::UMyComponent()
    : Count(0)
    , Time(0.0f)
    , bIsActive(false)
{
}
```

### Use Override Keyword

```cpp
// Good: Explicit override
virtual void BeginPlay() override;
virtual void Tick(float DeltaTime) override;

// Prevents errors if base signature changes
```

## Related Documentation
- [Blueprint Best Practices](BlueprintDesign.md)
- [Performance Best Practices](PerformanceOptimization.md)
- [API Reference](../API/README.md)
- [Coding Standards](CodingStandards.md)
