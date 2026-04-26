# EvidenceCollectionComponent

Component for managing evidence collection and inventory for player characters.

## Class Declaration

```cpp
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class HORRORPROJECT_API UEvidenceCollectionComponent : public UActorComponent
{
    GENERATED_BODY()
};
```

**Header**: `Evidence/EvidenceCollectionComponent.h`  
**Module**: HorrorProject  
**Inherits**: UActorComponent

## Overview

The EvidenceCollectionComponent manages the player's evidence inventory, including:
- Collecting evidence items
- Storing evidence metadata
- Tracking collection progress
- Triggering collection events
- Save/load functionality

## Public Functions

### CollectEvidence

```cpp
UFUNCTION(BlueprintCallable, Category = "Evidence")
bool CollectEvidence(AEvidenceActor* Evidence);
```

Attempts to collect an evidence item.

**Parameters**:
- `Evidence` [Required] - The evidence actor to collect

**Returns**: bool - True if collection succeeded, false otherwise

**Example**:
```cpp
bool bSuccess = EvidenceComponent->CollectEvidence(EvidenceActor);
if (bSuccess)
{
    UE_LOG(LogTemp, Log, TEXT("Evidence collected!"));
}
```

---

### HasEvidence

```cpp
UFUNCTION(BlueprintPure, Category = "Evidence")
bool HasEvidence(FName EvidenceID) const;
```

Checks if a specific evidence item has been collected.

**Parameters**:
- `EvidenceID` [Required] - Unique identifier for the evidence

**Returns**: bool - True if evidence is in collection

---

### GetCollectedEvidence

```cpp
UFUNCTION(BlueprintPure, Category = "Evidence")
TArray<FEvidenceData> GetCollectedEvidence() const;
```

Returns all collected evidence data.

**Parameters**: None  
**Returns**: TArray<FEvidenceData> - Array of collected evidence

---

### GetEvidenceCount

```cpp
UFUNCTION(BlueprintPure, Category = "Evidence")
int32 GetEvidenceCount() const;
```

Gets the total number of collected evidence items.

**Parameters**: None  
**Returns**: int32 - Number of evidence items

---

### RemoveEvidence

```cpp
UFUNCTION(BlueprintCallable, Category = "Evidence")
bool RemoveEvidence(FName EvidenceID);
```

Removes an evidence item from the collection.

**Parameters**:
- `EvidenceID` [Required] - ID of evidence to remove

**Returns**: bool - True if removal succeeded

---

### ClearAllEvidence

```cpp
UFUNCTION(BlueprintCallable, Category = "Evidence")
void ClearAllEvidence();
```

Removes all evidence from the collection.

**Parameters**: None  
**Returns**: void

---

### GetEvidenceByCategory

```cpp
UFUNCTION(BlueprintPure, Category = "Evidence")
TArray<FEvidenceData> GetEvidenceByCategory(EEvidenceCategory Category) const;
```

Gets all evidence items of a specific category.

**Parameters**:
- `Category` [Required] - Evidence category to filter by

**Returns**: TArray<FEvidenceData> - Filtered evidence array

---

### SaveToArchive

```cpp
UFUNCTION(BlueprintCallable, Category = "Evidence")
void SaveToArchive();
```

Saves the current evidence collection to the archive subsystem.

**Parameters**: None  
**Returns**: void

---

### LoadFromArchive

```cpp
UFUNCTION(BlueprintCallable, Category = "Evidence")
void LoadFromArchive();
```

Loads evidence collection from the archive subsystem.

**Parameters**: None  
**Returns**: void

---

## Public Properties

### MaxEvidenceCapacity

```cpp
UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Evidence Settings")
int32 MaxEvidenceCapacity = 100;
```

Maximum number of evidence items that can be collected.

---

### bAutoSave

```cpp
UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Evidence Settings")
bool bAutoSave = true;
```

Automatically save to archive when evidence is collected.

---

### CollectionSound

```cpp
UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Evidence Settings")
USoundBase* CollectionSound;
```

Sound played when evidence is collected.

---

## Events

### OnEvidenceCollected

```cpp
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEvidenceCollected, 
    const FEvidenceData&, Evidence);

UPROPERTY(BlueprintAssignable, Category = "Evidence Events")
FOnEvidenceCollected OnEvidenceCollected;
```

Fired when evidence is successfully collected.

**Parameters**:
- `Evidence` - Data for the collected evidence

---

### OnEvidenceRemoved

```cpp
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEvidenceRemoved, 
    FName, EvidenceID);

UPROPERTY(BlueprintAssignable, Category = "Evidence Events")
FOnEvidenceRemoved OnEvidenceRemoved;
```

Fired when evidence is removed from collection.

**Parameters**:
- `EvidenceID` - ID of removed evidence

---

### OnCollectionFull

```cpp
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCollectionFull);

UPROPERTY(BlueprintAssignable, Category = "Evidence Events")
FOnCollectionFull OnCollectionFull;
```

Fired when evidence collection reaches maximum capacity.

---

## Usage Examples

### Basic Collection

```cpp
void APlayerCharacter::InteractWithEvidence(AEvidenceActor* Evidence)
{
    UEvidenceCollectionComponent* Collection = FindComponentByClass<UEvidenceCollectionComponent>();
    if (Collection)
    {
        if (Collection->CollectEvidence(Evidence))
        {
            // Show collection UI
            ShowEvidenceCollectedWidget(Evidence->GetEvidenceData());
        }
    }
}
```

### Check Collection Progress

```cpp
void AMyHUD::UpdateEvidenceCounter()
{
    UEvidenceCollectionComponent* Collection = GetPlayerCollection();
    if (Collection)
    {
        int32 Count = Collection->GetEvidenceCount();
        int32 Max = Collection->MaxEvidenceCapacity;
        
        EvidenceCounterText->SetText(FText::Format(
            LOCTEXT("EvidenceCount", "{0}/{1}"),
            Count, Max
        ));
    }
}
```

### Event Binding

```cpp
void APlayerCharacter::BeginPlay()
{
    Super::BeginPlay();
    
    UEvidenceCollectionComponent* Collection = FindComponentByClass<UEvidenceCollectionComponent>();
    if (Collection)
    {
        Collection->OnEvidenceCollected.AddDynamic(this, &APlayerCharacter::HandleEvidenceCollected);
        Collection->OnCollectionFull.AddDynamic(this, &APlayerCharacter::HandleCollectionFull);
    }
}

void APlayerCharacter::HandleEvidenceCollected(const FEvidenceData& Evidence)
{
    UE_LOG(LogTemp, Log, TEXT("Collected: %s"), *Evidence.DisplayName.ToString());
}
```

### Category Filtering

```cpp
void AArchiveWidget::ShowDocuments()
{
    UEvidenceCollectionComponent* Collection = GetPlayerCollection();
    if (Collection)
    {
        TArray<FEvidenceData> Documents = Collection->GetEvidenceByCategory(
            EEvidenceCategory::Document
        );
        
        PopulateDocumentList(Documents);
    }
}
```

## Blueprint Usage

![Evidence Collection Blueprint](../../_images/api/evidence_collection_bp.png)

### Collecting Evidence
1. Get Evidence Collection Component
2. Call "Collect Evidence" with target evidence actor
3. Check return value for success

### Checking Collection
1. Get Evidence Collection Component
2. Call "Has Evidence" with evidence ID
3. Use boolean result for logic

## Performance Considerations

- Use `HasEvidence()` for quick lookups instead of iterating
- Cache component references instead of repeated `FindComponentByClass()`
- Limit `MaxEvidenceCapacity` to reasonable values
- Use category filtering to reduce iteration overhead

## Save/Load

Evidence collection automatically integrates with the save system when `bAutoSave` is enabled. Manual save/load:

```cpp
// Manual save
EvidenceComponent->SaveToArchive();

// Manual load
EvidenceComponent->LoadFromArchive();
```

## Related Classes

- [EvidenceActor](EvidenceActor.md)
- [ArchiveSubsystem](ArchiveSubsystem.md)
- [EvidenceTypes](EvidenceTypes.md)
- [EvidenceBlueprintLibrary](EvidenceBlueprintLibrary.md)

## See Also

- [Evidence System Overview](../../Architecture/SystemOverview.md#evidence-system)
- [Evidence Blueprint Guide](../../Blueprint/Evidence_Blueprint_Guide.md)
- [Evidence Collection Tutorial](../../Tutorials/EvidenceCollection.md)
