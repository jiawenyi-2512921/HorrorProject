# Save System Architecture

## Overview

The HorrorProject save system provides a comprehensive solution for game state persistence with support for multiple save slots, cloud synchronization, automatic backups, and version migration.

## Core Components

### 1. SaveSlotManager
Manages multiple save game slots with metadata tracking.

**Features:**
- Up to 3 save slots
- Metadata tracking (save time, version, checkpoint, etc.)
- Last used slot tracking
- Slot validation

**Key Methods:**
- `SaveToSlot(int32 SlotIndex, UHorrorSaveGame* SaveGame)` - Save to specific slot
- `LoadFromSlot(int32 SlotIndex)` - Load from specific slot
- `DeleteSlot(int32 SlotIndex)` - Delete save slot
- `GetSlotMetadata(int32 SlotIndex)` - Get slot information

### 2. CloudSaveSubsystem
Handles cloud save synchronization with Steam and Epic platforms.

**Features:**
- Automatic platform detection (Steam/Epic)
- Upload/download save files to cloud
- Sync all saves at once
- Status tracking and callbacks

**Key Methods:**
- `UploadSaveToCloud(int32 SlotIndex)` - Upload specific slot
- `DownloadSaveFromCloud(int32 SlotIndex)` - Download specific slot
- `SyncAllSaves()` - Sync all slots
- `IsCloudSaveEnabled()` - Check if cloud saves are available

### 3. AutoSaveComponent
Actor component for automatic periodic saves.

**Features:**
- Configurable save interval (30-600 seconds)
- Enable/disable at runtime
- Configurable target slot
- Optional notifications

**Key Properties:**
- `bAutoSaveEnabled` - Enable/disable auto-save
- `AutoSaveIntervalSeconds` - Time between saves
- `AutoSaveSlotIndex` - Target save slot
- `bShowAutoSaveNotification` - Show save notifications

### 4. SaveGameMigration
Handles version migration for save files.

**Features:**
- Automatic version detection
- Step-by-step migration chain
- Validation before/after migration
- Extensible migration system

**Key Methods:**
- `MigrateSaveGame(UHorrorSaveGame* SaveGame)` - Migrate to current version
- `NeedsMigration(const UHorrorSaveGame* SaveGame)` - Check if migration needed

### 5. SaveGameValidator
Validates save game integrity and structure.

**Features:**
- Version validation
- Required data validation
- Data integrity checks
- Corruption detection

**Validation Results:**
- Valid
- CorruptedData
- InvalidVersion
- MissingRequiredData
- ChecksumMismatch

### 6. SaveGameBackup
Automatic backup system for save files.

**Features:**
- Automatic backup on save
- Up to 5 backups per slot
- Restore from backup
- Automatic cleanup of old backups

**Key Methods:**
- `CreateBackup(int32 SlotIndex)` - Create backup
- `RestoreBackup(const FString& BackupName, int32 SlotIndex)` - Restore backup
- `GetBackupsForSlot(int32 SlotIndex)` - List backups

## Data Flow

### Save Flow
1. Game calls `SaveSlotManager::SaveToSlot()`
2. SaveSlotManager validates slot index
3. Save game serialized to disk
4. Metadata updated with save information
5. Backup created automatically
6. Cloud sync triggered (if enabled)
7. Callbacks fired for UI updates

### Load Flow
1. Game calls `SaveSlotManager::LoadFromSlot()`
2. SaveSlotManager validates slot exists
3. Save game loaded from disk
4. SaveGameValidator checks integrity
5. SaveGameMigration checks version
6. Migration performed if needed
7. Save game returned to caller

### Cloud Sync Flow
1. CloudSaveSubsystem detects platform
2. Upload/download requested
3. Platform-specific API called
4. Status callbacks fired
5. Metadata updated with sync status

## Integration with Existing System

The new save system extends the existing `HorrorSaveSubsystem` and `HorrorSaveGame` classes:

- `HorrorSaveSubsystem` - Handles checkpoint-based saves
- `HorrorSaveGame` - Contains game state data
- `SaveSlotManager` - Adds multi-slot support
- `CloudSaveSubsystem` - Adds cloud functionality

## Usage Examples

### Basic Save/Load
```cpp
// Get slot manager
USaveSlotManager* SlotManager = NewObject<USaveSlotManager>();

// Save to slot 0
UHorrorSaveGame* SaveGame = CreateSaveGame();
SlotManager->SaveToSlot(0, SaveGame);

// Load from slot 0
UHorrorSaveGame* LoadedSave = SlotManager->LoadFromSlot(0);
```

### Cloud Sync
```cpp
// Get cloud save subsystem
UCloudSaveSubsystem* CloudSave = GameInstance->GetSubsystem<UCloudSaveSubsystem>();

// Upload to cloud
CloudSave->UploadSaveToCloud(0);

// Download from cloud
CloudSave->DownloadSaveFromCloud(0);
```

### Auto-Save
```cpp
// Add component to player
UAutoSaveComponent* AutoSave = Player->CreateDefaultSubobject<UAutoSaveComponent>(TEXT("AutoSave"));
AutoSave->AutoSaveIntervalSeconds = 180.0f; // 3 minutes
AutoSave->AutoSaveSlotIndex = 2; // Use slot 2 for auto-saves
```

## File Structure

```
SaveGame/
├── SaveSlotManager.h/cpp          - Multi-slot management
├── CloudSaveSubsystem.h/cpp       - Cloud synchronization
├── AutoSaveComponent.h/cpp        - Automatic saves
├── SaveGameMetadata.h/cpp         - Metadata tracking
├── SaveGameMigration.h/cpp        - Version migration
├── SaveGameValidator.h/cpp        - Save validation
├── SaveGameBackup.h/cpp           - Backup system
├── Tests/
│   ├── TestSaveGameSlots.cpp      - Slot tests
│   ├── TestCloudSave.cpp          - Cloud tests
│   ├── TestSaveMigration.cpp      - Migration tests
│   └── TestAutoSave.cpp           - Auto-save tests
└── Docs/
    ├── SaveSystemArchitecture.md  - This file
    ├── CloudSaveSetup.md          - Cloud setup guide
    ├── SaveGameMigration.md       - Migration guide
    └── SaveGameTesting.md         - Testing guide
```

## Performance Considerations

- Save operations are synchronous but fast (< 100ms typical)
- Cloud operations are asynchronous with callbacks
- Backups are created on background thread
- Metadata is cached in memory
- Auto-save uses timer system (no tick overhead)

## Thread Safety

- SaveSlotManager is not thread-safe (use on game thread)
- CloudSaveSubsystem callbacks fire on game thread
- File operations use platform file manager (thread-safe)
- Backup operations can run on background threads

## Error Handling

All save operations return success/failure status:
- Invalid slot indices return false
- Corrupted saves are detected and reported
- Cloud errors trigger callbacks with error messages
- Migration failures preserve original save

## Future Enhancements

- Compression for save files
- Encryption for sensitive data
- Differential saves (only changed data)
- Save game preview screenshots
- Cross-platform cloud sync
- Save game sharing/export
