# Save Game Migration Guide

## Overview

The save game migration system handles version upgrades for save files, ensuring backward compatibility when game data structures change.

## Migration Architecture

### Version System

Each save game has a version number:
```cpp
UPROPERTY(VisibleAnywhere, BlueprintReadOnly, SaveGame, Category="Horror|Save")
int32 SaveVersion = 1;
```

Current version is defined in `USaveGameMigration`:
```cpp
static constexpr int32 CurrentSaveVersion = 1;
```

### Migration Steps

Migrations are defined as steps from one version to another:
```cpp
struct FSaveGameMigrationStep
{
    int32 FromVersion;
    int32 ToVersion;
    TFunction<bool(UHorrorSaveGame*)> MigrationFunction;
};
```

## Creating Migrations

### Step 1: Increment Version

When changing save data structure, increment `CurrentSaveVersion`:
```cpp
// In SaveGameMigration.h
static constexpr int32 CurrentSaveVersion = 2; // Was 1
```

### Step 2: Define Migration Function

Create a migration function for the version change:
```cpp
bool USaveGameMigration::MigrateFromV1ToV2(UHorrorSaveGame* SaveGame)
{
    if (!SaveGame)
        return false;
    
    // Example: Add new field with default value
    // V2 adds a new array for quest data
    SaveGame->QuestData.Empty();
    
    // Example: Transform existing data
    for (FName& EvidenceId : SaveGame->CollectedEvidenceIds)
    {
        // Convert old format to new format
        FString OldFormat = EvidenceId.ToString();
        FString NewFormat = TEXT("Evidence_") + OldFormat;
        EvidenceId = FName(*NewFormat);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Successfully migrated save from V1 to V2"));
    return true;
}
```

### Step 3: Register Migration Step

Add the migration step in `RegisterMigrationSteps()`:
```cpp
void USaveGameMigration::RegisterMigrationSteps()
{
    MigrationSteps.Empty();
    
    // V1 to V2 migration
    FSaveGameMigrationStep V1ToV2;
    V1ToV2.FromVersion = 1;
    V1ToV2.ToVersion = 2;
    V1ToV2.MigrationFunction = [this](UHorrorSaveGame* SaveGame) -> bool
    {
        return MigrateFromV1ToV2(SaveGame);
    };
    MigrationSteps.Add(V1ToV2);
    
    // V2 to V3 migration (if needed)
    FSaveGameMigrationStep V2ToV3;
    V2ToV3.FromVersion = 2;
    V2ToV3.ToVersion = 3;
    V2ToV3.MigrationFunction = [this](UHorrorSaveGame* SaveGame) -> bool
    {
        return MigrateFromV2ToV3(SaveGame);
    };
    MigrationSteps.Add(V2ToV3);
    
    bMigrationStepsRegistered = true;
}
```

## Migration Execution

### Automatic Migration

Migration happens automatically when loading a save:
```cpp
USaveSlotManager* SlotManager = GetSlotManager();
UHorrorSaveGame* SaveGame = SlotManager->LoadFromSlot(0);

if (SaveGame)
{
    USaveGameMigration* Migration = NewObject<USaveGameMigration>();
    
    if (Migration->NeedsMigration(SaveGame))
    {
        if (Migration->MigrateSaveGame(SaveGame))
        {
            // Migration successful, re-save with new version
            SlotManager->SaveToSlot(0, SaveGame);
        }
        else
        {
            // Migration failed, handle error
            UE_LOG(LogTemp, Error, TEXT("Failed to migrate save game"));
        }
    }
}
```

### Manual Migration

You can also trigger migration manually:
```cpp
void MigrateSaveFile(int32 SlotIndex)
{
    USaveSlotManager* SlotManager = GetSlotManager();
    UHorrorSaveGame* SaveGame = SlotManager->LoadFromSlot(SlotIndex);
    
    if (!SaveGame)
        return;
    
    USaveGameMigration* Migration = NewObject<USaveGameMigration>();
    
    if (Migration->MigrateSaveGame(SaveGame))
    {
        // Save migrated data
        SlotManager->SaveToSlot(SlotIndex, SaveGame);
        UE_LOG(LogTemp, Log, TEXT("Save migrated to version %d"), SaveGame->SaveVersion);
    }
}
```

## Migration Patterns

### Adding New Fields

When adding new fields, provide sensible defaults:
```cpp
bool MigrateFromV1ToV2(UHorrorSaveGame* SaveGame)
{
    // New field: PlayerHealth
    SaveGame->PlayerHealth = 100.0f; // Default full health
    
    // New field: Difficulty
    SaveGame->Difficulty = EDifficulty::Normal; // Default difficulty
    
    return true;
}
```

### Removing Fields

When removing fields, no action needed (old data is ignored):
```cpp
bool MigrateFromV2ToV3(UHorrorSaveGame* SaveGame)
{
    // V3 removes "TemporaryData" field
    // No migration needed, field will be ignored on load
    return true;
}
```

### Renaming Fields

When renaming fields, copy data to new field:
```cpp
bool MigrateFromV3ToV4(UHorrorSaveGame* SaveGame)
{
    // Rename: CollectedEvidenceIds -> EvidenceInventory
    SaveGame->EvidenceInventory = SaveGame->CollectedEvidenceIds;
    SaveGame->CollectedEvidenceIds.Empty(); // Clear old field
    
    return true;
}
```

### Restructuring Data

When changing data structure, transform the data:
```cpp
bool MigrateFromV4ToV5(UHorrorSaveGame* SaveGame)
{
    // V4: Array of FNames
    // V5: Map of FName to FEvidenceData
    
    for (const FName& EvidenceId : SaveGame->CollectedEvidenceIds)
    {
        FEvidenceData Data;
        Data.Id = EvidenceId;
        Data.CollectionTime = FDateTime::Now();
        Data.bAnalyzed = false;
        
        SaveGame->EvidenceMap.Add(EvidenceId, Data);
    }
    
    SaveGame->CollectedEvidenceIds.Empty();
    return true;
}
```

### Complex Migrations

For complex migrations, break into steps:
```cpp
bool MigrateFromV5ToV6(UHorrorSaveGame* SaveGame)
{
    // Step 1: Validate existing data
    if (!ValidateV5Data(SaveGame))
    {
        UE_LOG(LogTemp, Error, TEXT("V5 data validation failed"));
        return false;
    }
    
    // Step 2: Transform data
    if (!TransformV5ToV6(SaveGame))
    {
        UE_LOG(LogTemp, Error, TEXT("V5 to V6 transformation failed"));
        return false;
    }
    
    // Step 3: Validate new data
    if (!ValidateV6Data(SaveGame))
    {
        UE_LOG(LogTemp, Error, TEXT("V6 data validation failed"));
        return false;
    }
    
    return true;
}
```

## Migration Chain

The system supports multi-version migrations:
```
V1 -> V2 -> V3 -> V4 (Current)
```

If a V1 save is loaded, it will:
1. Migrate V1 -> V2
2. Migrate V2 -> V3
3. Migrate V3 -> V4

Each step is executed in order.

## Testing Migrations

### Unit Tests

Test each migration step:
```cpp
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMigrationV1ToV2Test, "HorrorProject.SaveGame.Migration.V1ToV2", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FMigrationV1ToV2Test::RunTest(const FString& Parameters)
{
    // Create V1 save
    UHorrorSaveGame* SaveGame = NewObject<UHorrorSaveGame>();
    SaveGame->SaveVersion = 1;
    SaveGame->CheckpointId = FName(TEXT("TestCheckpoint"));
    
    // Migrate
    USaveGameMigration* Migration = NewObject<USaveGameMigration>();
    TestTrue(TEXT("Migration should succeed"), Migration->MigrateSaveGame(SaveGame));
    
    // Verify version updated
    TestEqual(TEXT("Version should be 2"), SaveGame->SaveVersion, 2);
    
    // Verify data integrity
    TestEqual(TEXT("Checkpoint should be preserved"), SaveGame->CheckpointId, FName(TEXT("TestCheckpoint")));
    
    return true;
}
```

### Integration Tests

Test full migration chain:
```cpp
void TestFullMigrationChain()
{
    // Create V1 save
    UHorrorSaveGame* SaveGame = CreateV1Save();
    
    // Migrate to current
    USaveGameMigration* Migration = NewObject<USaveGameMigration>();
    bool bSuccess = Migration->MigrateSaveGame(SaveGame);
    
    check(bSuccess);
    check(SaveGame->SaveVersion == USaveGameMigration::CurrentSaveVersion);
    
    // Validate all data
    ValidateSaveGame(SaveGame);
}
```

### Manual Testing

1. Create save with old version
2. Modify code to new version
3. Load old save
4. Verify migration succeeds
5. Verify game functions correctly
6. Save again and verify new version

## Best Practices

### Always Provide Defaults
```cpp
// Good
SaveGame->NewField = DefaultValue;

// Bad
// SaveGame->NewField is uninitialized
```

### Validate Before and After
```cpp
bool MigrateFromVXToVY(UHorrorSaveGame* SaveGame)
{
    if (!ValidateVXData(SaveGame))
        return false;
    
    // Perform migration
    
    if (!ValidateVYData(SaveGame))
        return false;
    
    return true;
}
```

### Log Migration Steps
```cpp
UE_LOG(LogTemp, Log, TEXT("Migrating save from V%d to V%d"), FromVersion, ToVersion);
UE_LOG(LogTemp, Log, TEXT("Transformed %d evidence entries"), NumTransformed);
```

### Handle Errors Gracefully
```cpp
bool MigrateFromVXToVY(UHorrorSaveGame* SaveGame)
{
    if (!SaveGame)
    {
        UE_LOG(LogTemp, Error, TEXT("Null save game"));
        return false;
    }
    
    try
    {
        // Migration logic
    }
    catch (...)
    {
        UE_LOG(LogTemp, Error, TEXT("Migration exception"));
        return false;
    }
    
    return true;
}
```

### Preserve Critical Data
```cpp
// Always preserve player progress
FName OriginalCheckpoint = SaveGame->CheckpointId;
TArray<FName> OriginalEvidence = SaveGame->CollectedEvidenceIds;

// Perform migration

// Verify critical data preserved
check(SaveGame->CheckpointId == OriginalCheckpoint);
check(SaveGame->CollectedEvidenceIds.Num() >= OriginalEvidence.Num());
```

## Rollback Strategy

If migration fails, preserve original save:
```cpp
void SafeMigration(int32 SlotIndex)
{
    // Create backup before migration
    USaveGameBackup* Backup = NewObject<USaveGameBackup>();
    Backup->CreateBackup(SlotIndex);
    
    // Load and migrate
    USaveSlotManager* SlotManager = GetSlotManager();
    UHorrorSaveGame* SaveGame = SlotManager->LoadFromSlot(SlotIndex);
    
    USaveGameMigration* Migration = NewObject<USaveGameMigration>();
    if (Migration->MigrateSaveGame(SaveGame))
    {
        // Save migrated version
        SlotManager->SaveToSlot(SlotIndex, SaveGame);
    }
    else
    {
        // Restore backup on failure
        UE_LOG(LogTemp, Error, TEXT("Migration failed, restoring backup"));
        Backup->RestoreBackup(Backup->GetBackupsForSlot(SlotIndex)[0].BackupName, SlotIndex);
    }
}
```

## Version History Template

Document all version changes:
```cpp
/*
 * Save Game Version History
 * 
 * V1 (Initial Release)
 * - Basic save data
 * - Checkpoint system
 * - Evidence collection
 * 
 * V2 (Update 1.1)
 * - Added quest system
 * - Added player stats
 * - Renamed evidence fields
 * 
 * V3 (Update 1.2)
 * - Added difficulty setting
 * - Added playtime tracking
 * - Removed temporary data
 * 
 * V4 (Current)
 * - Restructured evidence as map
 * - Added cloud sync metadata
 * - Added backup support
 */
```

## Common Pitfalls

### Forgetting to Increment Version
Always increment `CurrentSaveVersion` when changing save structure.

### Breaking Migration Chain
Ensure each migration step is registered in order.

### Not Testing Old Saves
Always test with saves from previous versions.

### Losing Data
Always preserve player progress during migration.

### Not Handling Nulls
Always check for null pointers and invalid data.

## Migration Checklist

- [ ] Increment `CurrentSaveVersion`
- [ ] Create migration function
- [ ] Register migration step
- [ ] Add unit tests
- [ ] Test with old saves
- [ ] Document version changes
- [ ] Create backup before migration
- [ ] Validate data after migration
- [ ] Log migration steps
- [ ] Handle errors gracefully
