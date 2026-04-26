# Achievements - Quick Start Guide

Unlock your first achievement in 5 minutes.

## Step 1: Get Achievement Subsystem (30 sec)

```cpp
UAchievementSubsystem* Achievements = 
    GetGameInstance()->GetSubsystem<UAchievementSubsystem>();
```

## Step 2: Unlock Achievement (1 min)

```cpp
// Simple unlock
Achievements->UnlockAchievement(FName("ACH_FirstSteps"));

// With callback
Achievements->OnAchievementUnlocked.AddDynamic(this, &UMyClass::OnAchievementUnlocked);

void UMyClass::OnAchievementUnlocked(const FAchievementData& Achievement)
{
    UE_LOG(LogTemp, Log, TEXT("Unlocked: %s"), *Achievement.Name.ToString());
}
```

## Step 3: Track Progress (1 min)

```cpp
// Progressive achievement (e.g., collect 10 items)
Achievements->UpdateAchievementProgress(FName("ACH_Collector"), 1.0f); // +1 item

// Check progress
float Progress = Achievements->GetAchievementProgress(FName("ACH_Collector"));
```

## Step 4: Add Tracker Component (2 min)

Add to your Character or GameMode:

```cpp
// In constructor
AchievementTracker = CreateDefaultSubobject<UAchievementTracker>(TEXT("AchievementTracker"));

// Track events
AchievementTracker->TrackRoomDiscovered(TEXT("Room_01"));
AchievementTracker->TrackEvidenceCollected(TEXT("Evidence_Key"));
AchievementTracker->TrackGhostEncounter();
```

## Blueprint Quick Start

1. Get Achievement Subsystem
2. Unlock Achievement (Name: "ACH_FirstSteps")
3. Display notification automatically

## Available Achievements

See `/Source/HorrorProject/Achievements/AchievementList.md` for full list.

Common achievements:
- `ACH_FirstSteps` - Start the game
- `ACH_Explorer` - Discover 10 rooms
- `ACH_Collector` - Collect 5 evidence items
- `ACH_Survivor` - Complete Chapter 1

## Testing

```cpp
// Reset achievement for testing
Achievements->ResetAchievement(FName("ACH_FirstSteps"));

// Get all achievements
TArray<FAchievementData> AllAchievements = Achievements->GetAllAchievements();
```

## Next Steps

- Create custom achievements
- Integrate with Steam/Epic
- Design achievement UI
- Add secret achievements
