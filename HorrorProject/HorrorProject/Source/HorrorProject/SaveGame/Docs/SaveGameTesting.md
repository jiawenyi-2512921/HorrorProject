# Save Game Testing Guide

## Overview

Comprehensive testing guide for the save game system, covering unit tests, integration tests, and manual testing procedures.

## Test Categories

### 1. Unit Tests
Test individual components in isolation.

### 2. Integration Tests
Test component interactions and full workflows.

### 3. Manual Tests
Test user-facing features and edge cases.

## Running Tests

### Unreal Automation System

Run all save game tests:
```
Session Frontend -> Automation -> Filter: "HorrorProject.SaveGame"
```

Run specific test category:
```
HorrorProject.SaveGame.SaveSlotManager.*
HorrorProject.SaveGame.CloudSave.*
HorrorProject.SaveGame.Migration.*
HorrorProject.SaveGame.AutoSave.*
```

### Command Line

Run tests from command line:
```bash
UnrealEditor.exe "HorrorProject.uproject" -ExecCmds="Automation RunTests HorrorProject.SaveGame" -unattended -nopause -testexit="Automation Test Queue Empty"
```

## Unit Test Coverage

### SaveSlotManager Tests

**TestSaveGameSlots.cpp**

1. **Basic Test**
   - Create slot manager
   - Verify all slots empty initially
   - Test slot count

2. **Save/Load Test**
   - Create save game
   - Save to slot
   - Load from slot
   - Verify data matches

3. **Multiple Slots Test**
   - Save to all 3 slots
   - Verify each slot independent
   - Load and verify each slot

4. **Delete Test**
   - Save to slot
   - Delete slot
   - Verify slot empty

5. **Metadata Test**
   - Save to slot
   - Get metadata
   - Verify metadata accuracy

### CloudSave Tests

**TestCloudSave.cpp**

1. **Init Test**
   - Create subsystem
   - Verify initial state
   - Check status

2. **Provider Detection Test**
   - Initialize subsystem
   - Verify provider detected
   - Check enabled state

3. **Upload Test**
   - Attempt upload
   - Verify callback fired
   - Check status

4. **Download Test**
   - Attempt download
   - Verify callback fired
   - Check status

5. **Sync Test**
   - Sync all saves
   - Verify all callbacks
   - Check final state

### Migration Tests

**TestSaveMigration.cpp**

1. **Basic Test**
   - Create migration system
   - Verify current version
   - Check initialization

2. **Needs Migration Test**
   - Test current version save
   - Test old version save
   - Verify detection

3. **Execute Test**
   - Create save
   - Run migration
   - Verify success

4. **Null Save Test**
   - Test with null save
   - Verify error handling

5. **Version Preservation Test**
   - Migrate current version
   - Verify no changes

### AutoSave Tests

**TestAutoSave.cpp**

1. **Basic Test**
   - Create component
   - Verify defaults
   - Check properties

2. **Enable/Disable Test**
   - Toggle auto-save
   - Verify state changes

3. **Interval Test**
   - Set various intervals
   - Verify clamping
   - Check bounds

4. **Slot Test**
   - Set different slots
   - Verify assignment

5. **Notification Test**
   - Toggle notifications
   - Verify flag

## Integration Tests

### Save/Load Cycle

Test complete save and load workflow:
```cpp
void TestSaveLoadCycle()
{
    // Setup
    USaveSlotManager* SlotManager = NewObject<USaveSlotManager>();
    UHorrorSaveSubsystem* SaveSubsystem = GetSaveSubsystem();
    
    // Save checkpoint
    bool bSaved = SaveSubsystem->SaveCheckpoint(WorldContext, FName(TEXT("TestCheckpoint")));
    check(bSaved);
    
    // Load checkpoint
    bool bLoaded = SaveSubsystem->LoadCheckpoint(WorldContext);
    check(bLoaded);
    
    // Verify state restored
    VerifyGameState();
}
```

### Cloud Sync Cycle

Test cloud upload and download:
```cpp
void TestCloudSyncCycle()
{
    UCloudSaveSubsystem* CloudSave = GetCloudSaveSubsystem();
    USaveSlotManager* SlotManager = GetSlotManager();
    
    // Create and save
    UHorrorSaveGame* SaveGame = CreateTestSave();
    SlotManager->SaveToSlot(0, SaveGame);
    
    // Upload to cloud
    CloudSave->UploadSaveToCloud(0);
    WaitForCloudOperation();
    
    // Delete local
    SlotManager->DeleteSlot(0);
    check(SlotManager->IsSlotEmpty(0));
    
    // Download from cloud
    CloudSave->DownloadSaveFromCloud(0);
    WaitForCloudOperation();
    
    // Verify restored
    UHorrorSaveGame* Restored = SlotManager->LoadFromSlot(0);
    check(Restored != nullptr);
    VerifySaveData(Restored, SaveGame);
}
```

### Migration Chain

Test multi-version migration:
```cpp
void TestMigrationChain()
{
    // Create V1 save
    UHorrorSaveGame* SaveGame = NewObject<UHorrorSaveGame>();
    SaveGame->SaveVersion = 1;
    PopulateV1Data(SaveGame);
    
    // Migrate to current
    USaveGameMigration* Migration = NewObject<USaveGameMigration>();
    bool bSuccess = Migration->MigrateSaveGame(SaveGame);
    check(bSuccess);
    
    // Verify current version
    check(SaveGame->SaveVersion == USaveGameMigration::CurrentSaveVersion);
    
    // Verify data integrity
    VerifyMigratedData(SaveGame);
}
```

### Auto-Save Integration

Test auto-save with slot manager:
```cpp
void TestAutoSaveIntegration()
{
    // Setup
    UAutoSaveComponent* AutoSave = CreateAutoSaveComponent();
    AutoSave->AutoSaveIntervalSeconds = 1.0f; // Fast for testing
    AutoSave->AutoSaveSlotIndex = 2;
    
    // Start auto-save
    AutoSave->BeginPlay();
    
    // Wait for auto-save
    WaitSeconds(1.5f);
    
    // Verify save created
    USaveSlotManager* SlotManager = GetSlotManager();
    check(!SlotManager->IsSlotEmpty(2));
    
    // Cleanup
    AutoSave->EndPlay(EEndPlayReason::Destroyed);
}
```

### Backup and Restore

Test backup system:
```cpp
void TestBackupRestore()
{
    USaveGameBackup* Backup = NewObject<USaveGameBackup>();
    USaveSlotManager* SlotManager = GetSlotManager();
    
    // Create initial save
    UHorrorSaveGame* SaveV1 = CreateTestSave();
    SaveV1->CheckpointId = FName(TEXT("Checkpoint1"));
    SlotManager->SaveToSlot(0, SaveV1);
    
    // Create backup
    bool bBackupCreated = Backup->CreateBackup(0);
    check(bBackupCreated);
    
    // Modify save
    UHorrorSaveGame* SaveV2 = CreateTestSave();
    SaveV2->CheckpointId = FName(TEXT("Checkpoint2"));
    SlotManager->SaveToSlot(0, SaveV2);
    
    // Restore backup
    TArray<FSaveGameBackupInfo> Backups = Backup->GetBackupsForSlot(0);
    check(Backups.Num() > 0);
    bool bRestored = Backup->RestoreBackup(Backups[0].BackupName, 0);
    check(bRestored);
    
    // Verify original data
    UHorrorSaveGame* Restored = SlotManager->LoadFromSlot(0);
    check(Restored->CheckpointId == FName(TEXT("Checkpoint1")));
}
```

## Manual Testing

### Basic Save/Load

1. Start new game
2. Progress to checkpoint
3. Save to slot 1
4. Continue playing
5. Load from slot 1
6. Verify state restored

### Multiple Slots

1. Create save in slot 0
2. Create save in slot 1
3. Create save in slot 2
4. Load each slot
5. Verify each independent

### Cloud Sync

1. Enable cloud saves
2. Save game
3. Upload to cloud
4. Delete local save
5. Download from cloud
6. Verify restored

### Auto-Save

1. Enable auto-save
2. Set interval to 60 seconds
3. Play for 2 minutes
4. Check auto-save slot
5. Verify saves created

### Migration

1. Load save from previous version
2. Verify migration prompt
3. Accept migration
4. Verify game loads correctly
5. Save again
6. Verify new version

### Backup

1. Create save
2. Verify backup created
3. Corrupt save file
4. Restore from backup
5. Verify data intact

## Edge Cases

### Corrupted Save

Test handling of corrupted data:
```cpp
void TestCorruptedSave()
{
    // Create corrupted save file
    CreateCorruptedSaveFile(0);
    
    // Attempt load
    USaveSlotManager* SlotManager = GetSlotManager();
    UHorrorSaveGame* SaveGame = SlotManager->LoadFromSlot(0);
    
    // Should return null or valid default
    if (SaveGame)
    {
        // Validate
        USaveGameValidator* Validator = NewObject<USaveGameValidator>();
        ESaveGameValidationResult Result = Validator->ValidateSaveGame(SaveGame);
        check(Result == ESaveGameValidationResult::CorruptedData);
    }
}
```

### Full Slots

Test behavior when all slots full:
```cpp
void TestFullSlots()
{
    USaveSlotManager* SlotManager = GetSlotManager();
    
    // Fill all slots
    for (int32 i = 0; i < USaveSlotManager::MaxSaveSlots; ++i)
    {
        UHorrorSaveGame* SaveGame = CreateTestSave();
        SlotManager->SaveToSlot(i, SaveGame);
    }
    
    // Verify all full
    for (int32 i = 0; i < USaveSlotManager::MaxSaveSlots; ++i)
    {
        check(!SlotManager->IsSlotEmpty(i));
    }
    
    // Attempt to save to invalid slot
    UHorrorSaveGame* SaveGame = CreateTestSave();
    bool bSaved = SlotManager->SaveToSlot(99, SaveGame);
    check(!bSaved); // Should fail
}
```

### Cloud Offline

Test cloud operations when offline:
```cpp
void TestCloudOffline()
{
    // Simulate offline
    SimulateOfflineMode();
    
    UCloudSaveSubsystem* CloudSave = GetCloudSaveSubsystem();
    
    // Attempt upload
    bool bCallbackReceived = false;
    CloudSave->OnCloudSaveComplete.AddLambda([&](int32 SlotIndex, ECloudSaveStatus Status, const FString& ErrorMessage)
    {
        bCallbackReceived = true;
        check(Status == ECloudSaveStatus::Failed);
        check(!ErrorMessage.IsEmpty());
    });
    
    CloudSave->UploadSaveToCloud(0);
    check(bCallbackReceived);
}
```

### Rapid Save/Load

Test rapid operations:
```cpp
void TestRapidSaveLoad()
{
    USaveSlotManager* SlotManager = GetSlotManager();
    
    // Rapid saves
    for (int32 i = 0; i < 10; ++i)
    {
        UHorrorSaveGame* SaveGame = CreateTestSave();
        SaveGame->CheckpointId = FName(*FString::Printf(TEXT("Checkpoint_%d"), i));
        SlotManager->SaveToSlot(0, SaveGame);
    }
    
    // Load and verify last save
    UHorrorSaveGame* LoadedSave = SlotManager->LoadFromSlot(0);
    check(LoadedSave->CheckpointId == FName(TEXT("Checkpoint_9")));
}
```

## Performance Testing

### Save Time

Measure save operation time:
```cpp
void MeasureSaveTime()
{
    USaveSlotManager* SlotManager = GetSlotManager();
    UHorrorSaveGame* SaveGame = CreateLargeSave(); // Large data set
    
    double StartTime = FPlatformTime::Seconds();
    SlotManager->SaveToSlot(0, SaveGame);
    double EndTime = FPlatformTime::Seconds();
    
    double SaveTime = (EndTime - StartTime) * 1000.0; // ms
    UE_LOG(LogTemp, Log, TEXT("Save time: %.2f ms"), SaveTime);
    
    // Should be < 100ms
    check(SaveTime < 100.0);
}
```

### Load Time

Measure load operation time:
```cpp
void MeasureLoadTime()
{
    USaveSlotManager* SlotManager = GetSlotManager();
    
    double StartTime = FPlatformTime::Seconds();
    UHorrorSaveGame* SaveGame = SlotManager->LoadFromSlot(0);
    double EndTime = FPlatformTime::Seconds();
    
    double LoadTime = (EndTime - StartTime) * 1000.0; // ms
    UE_LOG(LogTemp, Log, TEXT("Load time: %.2f ms"), LoadTime);
    
    // Should be < 100ms
    check(LoadTime < 100.0);
}
```

### Memory Usage

Monitor memory during operations:
```cpp
void MeasureMemoryUsage()
{
    FPlatformMemoryStats StartStats = FPlatformMemory::GetStats();
    
    // Perform operations
    USaveSlotManager* SlotManager = GetSlotManager();
    for (int32 i = 0; i < 100; ++i)
    {
        UHorrorSaveGame* SaveGame = CreateTestSave();
        SlotManager->SaveToSlot(0, SaveGame);
        SlotManager->LoadFromSlot(0);
    }
    
    FPlatformMemoryStats EndStats = FPlatformMemory::GetStats();
    
    uint64 MemoryDelta = EndStats.UsedPhysical - StartStats.UsedPhysical;
    UE_LOG(LogTemp, Log, TEXT("Memory delta: %llu bytes"), MemoryDelta);
}
```

## Test Data Helpers

### Create Test Save

```cpp
UHorrorSaveGame* CreateTestSave()
{
    UHorrorSaveGame* SaveGame = NewObject<UHorrorSaveGame>();
    SaveGame->SaveVersion = USaveGameMigration::GetCurrentSaveVersion();
    SaveGame->CheckpointId = FName(TEXT("TestCheckpoint"));
    SaveGame->CollectedEvidenceIds.Add(FName(TEXT("Evidence1")));
    SaveGame->CollectedEvidenceIds.Add(FName(TEXT("Evidence2")));
    SaveGame->PlayerTransform = FTransform::Identity;
    return SaveGame;
}
```

### Verify Save Data

```cpp
void VerifySaveData(const UHorrorSaveGame* Actual, const UHorrorSaveGame* Expected)
{
    check(Actual->SaveVersion == Expected->SaveVersion);
    check(Actual->CheckpointId == Expected->CheckpointId);
    check(Actual->CollectedEvidenceIds.Num() == Expected->CollectedEvidenceIds.Num());
    check(Actual->PlayerTransform.Equals(Expected->PlayerTransform));
}
```

## Continuous Integration

### Automated Test Run

Add to CI pipeline:
```yaml
- name: Run Save Game Tests
  run: |
    UnrealEditor.exe "HorrorProject.uproject" \
      -ExecCmds="Automation RunTests HorrorProject.SaveGame" \
      -unattended -nopause \
      -testexit="Automation Test Queue Empty" \
      -ReportOutputPath="TestResults"
```

### Test Report

Parse test results:
```python
import json

with open('TestResults/index.json') as f:
    results = json.load(f)
    
total = results['TotalTests']
passed = results['SucceededTests']
failed = results['FailedTests']

print(f"Tests: {passed}/{total} passed")
if failed > 0:
    exit(1)
```

## Test Checklist

- [ ] All unit tests pass
- [ ] All integration tests pass
- [ ] Manual save/load works
- [ ] Multiple slots work independently
- [ ] Cloud sync works (if enabled)
- [ ] Auto-save triggers correctly
- [ ] Migration handles old saves
- [ ] Backups create and restore
- [ ] Corrupted saves handled
- [ ] Performance acceptable
- [ ] Memory usage reasonable
- [ ] Edge cases covered
