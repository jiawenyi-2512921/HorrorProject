# Statistics Tracking Guide

Guide for implementing and using the statistics tracking system.

## System Overview

The statistics system tracks player gameplay metrics and generates detailed reports.

### Core Components

1. **StatisticsSubsystem** - Main statistics management
2. **PlayerStatistics** - Player-specific statistics
3. **GameplayMetrics** - Performance and gameplay metrics
4. **StatisticsTracker** - Component for automatic tracking

## Statistics Categories

### Exploration Statistics

- **RoomsExplored** - Number of unique rooms discovered
- **SecretsFound** - Number of secrets found
- **DistanceTraveled** - Total distance traveled (in cm)

### Collection Statistics

- **EvidenceCollected** - Number of evidence pieces collected
- **DocumentsRead** - Number of documents read

### Survival Statistics

- **GhostEncounters** - Number of ghost encounters
- **Deaths** - Number of player deaths
- **TimeInDarkness** - Time spent in darkness (seconds)
- **SanityLost** - Total sanity lost

### Gameplay Statistics

- **DoorsOpened** - Number of doors opened
- **ItemsUsed** - Number of items used
- **JumpScares** - Number of jump scares triggered

### Performance Metrics

- **AverageFPS** - Average frames per second
- **MinFPS** - Minimum FPS recorded
- **MaxFPS** - Maximum FPS recorded

### Session Statistics

- **TotalPlayTime** - Total time played (seconds)
- **SessionDuration** - Current session duration
- **SaveCount** - Number of saves
- **LoadCount** - Number of loads

## Usage

### Initialize Subsystem

```cpp
UStatisticsSubsystem* StatisticsSubsystem = 
    GetWorld()->GetGameInstance()->GetSubsystem<UStatisticsSubsystem>();
```

### Start/End Session

```cpp
// Start tracking session
StatisticsSubsystem->StartSession();

// End session (automatically saves)
StatisticsSubsystem->EndSession();
```

### Track Statistics

```cpp
// Increment statistic
StatisticsSubsystem->IncrementStatistic(FName("RoomsExplored"));

// Set statistic to specific value
StatisticsSubsystem->SetStatistic(FName("Deaths"), 5.0f);

// Get statistic value
float Deaths = StatisticsSubsystem->GetStatistic(FName("Deaths"));
```

### Use Statistics Tracker Component

Add to player character:

```cpp
// In constructor
StatisticsTrackerComponent = CreateDefaultSubobject<UStatisticsTracker>(TEXT("StatisticsTracker"));
```

Track events:

```cpp
// Track movement (automatic in Tick)
TrackerComponent->TrackMovement(Distance);

// Track door opened
TrackerComponent->TrackDoorOpened();

// Track item used
TrackerComponent->TrackItemUsed(ItemID);

// Track sanity change
TrackerComponent->TrackSanityChange(OldSanity, NewSanity);

// Track jump scare
TrackerComponent->TrackJumpScare(Intensity);

// Track save/load
TrackerComponent->TrackSaveGame();
TrackerComponent->TrackLoadGame();

// Track FPS
TrackerComponent->TrackFPS(CurrentFPS);

// Track interaction
TrackerComponent->TrackInteraction(bSuccess);

// Update darkness time
TrackerComponent->UpdateDarknessTime(DeltaTime, bInDarkness);
```

## Gameplay Metrics

### Create Metrics

```cpp
FGameplayMetrics Metrics = UGameplayMetrics::CreateMetrics();
```

### Update Metrics

```cpp
// Update FPS metrics
UGameplayMetrics::UpdateFPSMetrics(Metrics, CurrentFPS);

// Update sanity metrics
UGameplayMetrics::UpdateSanityMetrics(Metrics, CurrentSanity);

// Record jump scare
UGameplayMetrics::RecordJumpScare(Metrics, Intensity);

// Record interaction
UGameplayMetrics::RecordInteraction(Metrics, bSuccess);
```

### Generate Metrics Report

```cpp
FString Report = UGameplayMetrics::GenerateMetricsReport(Metrics);
UE_LOG(LogTemp, Log, TEXT("%s"), *Report);
```

## Report Generation

### Statistics Report

```cpp
FString Report = StatisticsSubsystem->GenerateStatisticsReport();
```

Example output:

```
=== PLAYER STATISTICS REPORT ===

Total Play Time: 2.50 hours

--- EXPLORATION ---
Rooms Explored: 15
Distance Traveled: 1250.00 meters
Secrets Found: 3

--- COLLECTION ---
Evidence Collected: 8
Documents Read: 12

--- SURVIVAL ---
Ghost Encounters: 5
Deaths: 2
Time in Darkness: 450.00 seconds

--- OTHER STATISTICS ---
Doors Opened: 25
Items Used: 15
Sanity Lost: 45
Jump Scares: 8

================================
```

### Session Report

```cpp
FString Report = StatisticsTrackerComponent->GenerateSessionReport();
```

Includes both statistics and metrics.

## Event Handling

Subscribe to statistic updates:

```cpp
StatisticsSubsystem->OnStatisticUpdated.AddDynamic(
    this, &AMyClass::OnStatisticUpdated);

void AMyClass::OnStatisticUpdated(FName StatName, float NewValue)
{
    UE_LOG(LogTemp, Log, TEXT("Statistic %s updated to %.2f"), 
        *StatName.ToString(), NewValue);
}
```

## Save/Load System

Statistics are automatically saved when:
- Session ends
- `SaveStatistics()` is called manually

```cpp
// Manual save
StatisticsSubsystem->SaveStatistics();

// Manual load
StatisticsSubsystem->LoadStatistics();
```

Implement save/load:

```cpp
void UStatisticsSubsystem::SaveStatistics()
{
    UStatisticsSaveGame* SaveGame = Cast<UStatisticsSaveGame>(
        UGameplayStatics::CreateSaveGameObject(UStatisticsSaveGame::StaticClass()));
    
    SaveGame->PlayerStats = PlayerStats;
    SaveGame->CustomStatistics = CustomStatistics;
    SaveGame->TotalPlayTime = TotalPlayTime;
    
    UGameplayStatics::SaveGameToSlot(SaveGame, TEXT("Statistics"), 0);
}

void UStatisticsSubsystem::LoadStatistics()
{
    UStatisticsSaveGame* SaveGame = Cast<UStatisticsSaveGame>(
        UGameplayStatics::LoadGameFromSlot(TEXT("Statistics"), 0));
    
    if (SaveGame)
    {
        PlayerStats = SaveGame->PlayerStats;
        CustomStatistics = SaveGame->CustomStatistics;
        TotalPlayTime = SaveGame->TotalPlayTime;
    }
}
```

## Custom Statistics

Add custom statistics:

```cpp
// Add new statistic
CustomStatistics.Add(FName("CustomStat"), 0.0f);

// Track custom statistic
StatisticsSubsystem->IncrementStatistic(FName("CustomStat"));
```

## Integration with Achievements

Statistics can trigger achievements:

```cpp
// Check if statistic threshold reached
float RoomsExplored = StatisticsSubsystem->GetStatistic(FName("RoomsExplored"));
if (RoomsExplored >= 10.0f)
{
    AchievementSubsystem->UnlockAchievement(FName("ACH_Explorer"));
}
```

## UI Integration

### Display Statistics

```cpp
// Get player statistics
FPlayerStatistics PlayerStats = StatisticsSubsystem->GetPlayerStatistics();

// Display in UI
RoomsExploredText->SetText(FText::AsNumber(PlayerStats.RoomsExplored));
EvidenceCollectedText->SetText(FText::AsNumber(PlayerStats.EvidenceCollected));
DeathsText->SetText(FText::AsNumber(PlayerStats.Deaths));

// Format play time
float Hours = PlayerStats.TotalPlayTime / 3600.0f;
PlayTimeText->SetText(FText::Format(
    LOCTEXT("PlayTime", "{0} hours"), 
    FText::AsNumber(Hours)));
```

### Display Metrics

```cpp
// Get current metrics
FGameplayMetrics Metrics = StatisticsTrackerComponent->CurrentMetrics;

// Display in UI
AvgFPSText->SetText(FText::AsNumber(FMath::RoundToInt(Metrics.AverageFPS)));
MinSanityText->SetText(FText::Format(
    LOCTEXT("Sanity", "{0}%"), 
    FText::AsNumber(FMath::RoundToInt(Metrics.MinSanity))));
```

## Testing

Run automated tests:

```
Session Frontend → Automation → HorrorProject.Statistics
```

Test categories:
- StatisticsSubsystem - Core system tests
- Session - Session tracking tests
- Report - Report generation tests
- Events - Event system tests

## Best Practices

1. **Start Session Early**: Call StartSession() at game start
2. **End Session Properly**: Call EndSession() before game exit
3. **Track Continuously**: Use StatisticsTracker component for automatic tracking
4. **Save Regularly**: Statistics are saved on session end
5. **Use Events**: Subscribe to events for real-time UI updates
6. **Generate Reports**: Use reports for debugging and analytics
7. **Custom Statistics**: Add game-specific statistics as needed

## Performance Considerations

- Statistics updates are lightweight
- Automatic tracking in Tick is optimized
- Save operations are async-safe
- Report generation is on-demand
- Event broadcasting is efficient

## Analytics Integration

Statistics can be exported for analytics:

```cpp
// Export to analytics service
FPlayerStatistics Stats = StatisticsSubsystem->GetPlayerStatistics();

AnalyticsProvider->RecordEvent(TEXT("GameplayStats"), {
    {TEXT("RoomsExplored"), FString::FromInt(Stats.RoomsExplored)},
    {TEXT("EvidenceCollected"), FString::FromInt(Stats.EvidenceCollected)},
    {TEXT("Deaths"), FString::FromInt(Stats.Deaths)},
    {TEXT("PlayTime"), FString::SanitizeFloat(Stats.TotalPlayTime)}
});
```

## Common Issues

### Statistics Not Updating

- Check if subsystem is initialized
- Verify session is started
- Check statistic name matches
- Review tracker component registration

### Session Duration Incorrect

- Ensure StartSession() is called
- Check for multiple session starts
- Verify time calculation logic

### Save/Load Failing

- Implement save/load methods
- Check save game class exists
- Verify save slot name
- Test with save game system

## Future Enhancements

- Cloud statistics sync
- Leaderboard integration
- Heatmap generation
- Advanced analytics
- Machine learning insights
