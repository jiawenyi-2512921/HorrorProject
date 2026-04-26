# Achievement Implementation Guide

Guide for implementing and integrating the achievement system.

## System Architecture

### Core Components

1. **AchievementSubsystem** - Main achievement management system
2. **AchievementDefinition** - Achievement data definitions
3. **AchievementTracker** - Component for tracking achievement progress
4. **AchievementNotification** - UI notification system

### Data Flow

```
Game Event → AchievementTracker → AchievementSubsystem → Platform API
                                          ↓
                                  Achievement Unlocked
                                          ↓
                                  Notification Display
```

## Integration Steps

### 1. Initialize Subsystem

The AchievementSubsystem is automatically initialized as a GameInstance subsystem.

```cpp
// Get subsystem in any gameplay code
UAchievementSubsystem* AchievementSubsystem = 
    GetWorld()->GetGameInstance()->GetSubsystem<UAchievementSubsystem>();
```

### 2. Add Tracker Component

Add the AchievementTracker component to your player character or game mode:

```cpp
// In character constructor
AchievementTrackerComponent = CreateDefaultSubobject<UAchievementTracker>(TEXT("AchievementTracker"));
```

### 3. Track Game Events

Call tracker methods when relevant game events occur:

```cpp
// When player discovers a room
AchievementTrackerComponent->TrackRoomDiscovered(RoomID);

// When player collects evidence
AchievementTrackerComponent->TrackEvidenceCollected(EvidenceID);

// When player encounters ghost
AchievementTrackerComponent->TrackGhostEncounter();

// When player dies
AchievementTrackerComponent->TrackPlayerDeath();

// When chapter completes
AchievementTrackerComponent->TrackChapterComplete(ChapterNumber);

// When game completes
AchievementTrackerComponent->TrackGameComplete(CompletionTime);
```

### 4. Manual Achievement Unlock

You can also unlock achievements directly:

```cpp
UAchievementSubsystem* AchievementSubsystem = 
    GetWorld()->GetGameInstance()->GetSubsystem<UAchievementSubsystem>();

// Unlock achievement
AchievementSubsystem->UnlockAchievement(FName("ACH_FirstSteps"));

// Update progressive achievement
AchievementSubsystem->UpdateAchievementProgress(FName("ACH_Explorer"), 5.0f);
```

## Platform Integration

### Steam Integration

To integrate with Steam achievements:

1. Add Steam Online Subsystem to your project
2. Configure achievements in Steamworks
3. Implement Steam API calls in `AchievementSubsystem::SyncSteamAchievement()`

```cpp
void UAchievementSubsystem::SyncSteamAchievement(FName AchievementID)
{
    ISteamUserStats* SteamUserStats = SteamUserStats();
    if (SteamUserStats)
    {
        FString AchievementName = AchievementID.ToString();
        SteamUserStats->SetAchievement(TCHAR_TO_ANSI(*AchievementName));
        SteamUserStats->StoreStats();
    }
}
```

### Epic Games Store Integration

To integrate with Epic Online Services:

1. Add EOS plugin to your project
2. Configure achievements in Epic Dev Portal
3. Implement EOS API calls in `AchievementSubsystem::SyncEpicAchievement()`

```cpp
void UAchievementSubsystem::SyncEpicAchievement(FName AchievementID)
{
    EOS_Achievements_UnlockAchievementsOptions Options = {};
    Options.ApiVersion = EOS_ACHIEVEMENTS_UNLOCKACHIEVEMENTS_API_LATEST;
    Options.UserId = LocalUserId;
    
    const char* AchievementId = TCHAR_TO_ANSI(*AchievementID.ToString());
    Options.AchievementIds = &AchievementId;
    Options.AchievementsCount = 1;
    
    EOS_Achievements_UnlockAchievements(AchievementsHandle, &Options, nullptr, nullptr);
}
```

## Offline Support

The system supports offline achievement unlocking:

1. Achievements are tracked locally
2. When platform becomes available, sync is triggered
3. Call `SyncWithPlatform()` to manually sync

```cpp
AchievementSubsystem->SyncWithPlatform();
```

## Save/Load System

Implement save/load in `AchievementSubsystem`:

```cpp
void UAchievementSubsystem::SaveAchievementData()
{
    // Create save game object
    UAchievementSaveGame* SaveGame = Cast<UAchievementSaveGame>(
        UGameplayStatics::CreateSaveGameObject(UAchievementSaveGame::StaticClass()));
    
    // Store achievement data
    SaveGame->UnlockedAchievements = UnlockedAchievements.Array();
    SaveGame->AchievementProgress = AchievementProgressMap;
    
    // Save to slot
    UGameplayStatics::SaveGameToSlot(SaveGame, TEXT("Achievements"), 0);
}

void UAchievementSubsystem::LoadAchievementData()
{
    // Load save game
    UAchievementSaveGame* SaveGame = Cast<UAchievementSaveGame>(
        UGameplayStatics::LoadGameFromSlot(TEXT("Achievements"), 0));
    
    if (SaveGame)
    {
        UnlockedAchievements = TSet<FName>(SaveGame->UnlockedAchievements);
        AchievementProgressMap = SaveGame->AchievementProgress;
        
        // Update achievement data
        for (FName AchievementID : UnlockedAchievements)
        {
            if (FAchievementData* Achievement = Achievements.Find(AchievementID))
            {
                Achievement->bUnlocked = true;
            }
        }
    }
}
```

## Event Handling

Subscribe to achievement events:

```cpp
// Listen for achievement unlocks
AchievementSubsystem->OnAchievementUnlocked.AddDynamic(
    this, &AMyClass::OnAchievementUnlocked);

void AMyClass::OnAchievementUnlocked(const FAchievementData& Achievement)
{
    UE_LOG(LogTemp, Log, TEXT("Achievement unlocked: %s"), *Achievement.Name.ToString());
}

// Listen for progress updates
AchievementSubsystem->OnAchievementProgress.AddDynamic(
    this, &AMyClass::OnAchievementProgress);

void AMyClass::OnAchievementProgress(FName AchievementID, float Progress)
{
    UE_LOG(LogTemp, Log, TEXT("Achievement %s progress: %.2f%%"), 
        *AchievementID.ToString(), Progress * 100.0f);
}
```

## UI Integration

### Achievement Notification Widget

Create a Blueprint widget based on `UAchievementNotification`:

1. Add TextBlocks for name and description
2. Add Image for icon
3. Create fade in/out animations
4. The system will automatically display notifications

### Achievement List UI

Query achievements for display:

```cpp
// Get all achievements
TArray<FAchievementData> AllAchievements = AchievementSubsystem->GetAllAchievements();

// Get unlocked achievements
TArray<FAchievementData> UnlockedAchievements = AchievementSubsystem->GetUnlockedAchievements();

// Get locked achievements
TArray<FAchievementData> LockedAchievements = AchievementSubsystem->GetLockedAchievements();

// Get completion percentage
float CompletionPercentage = AchievementSubsystem->GetCompletionPercentage();
```

## Testing

Run automated tests:

```
Session Frontend → Automation → HorrorProject.Achievements
```

Test categories:
- AchievementSystem - Core system tests
- AchievementDefinition - Definition tests
- Events - Event system tests
- Tracker - Tracker component tests
- Progressive - Progressive achievement tests
- Secret - Secret achievement tests
- Survival - Survival achievement tests
- Speedrun - Speedrun achievement tests

## Best Practices

1. **Track Early**: Call tracker methods as soon as events occur
2. **Validate Data**: Check for null subsystems before calling methods
3. **Save Often**: Call SaveAchievementData() after important unlocks
4. **Test Offline**: Test achievement system without platform connection
5. **Progressive Updates**: Update progressive achievements incrementally
6. **Event Listeners**: Use events for UI updates and feedback
7. **Platform Sync**: Sync with platform regularly, especially on game start/end

## Common Issues

### Achievement Not Unlocking

- Check if subsystem is initialized
- Verify achievement ID matches definition
- Check if achievement is already unlocked
- Review tracker component registration

### Progress Not Updating

- Ensure achievement is marked as progressive
- Check MaxProgress value is correct
- Verify progress value is within range
- Check if achievement is already unlocked

### Platform Sync Failing

- Verify platform SDK is integrated
- Check platform availability flag
- Review platform API implementation
- Test with platform development tools

## Performance Considerations

- Achievement checks are lightweight
- Progress updates are cached
- Save operations are async-safe
- Platform syncs are non-blocking
- Notification display is optimized
