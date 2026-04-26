# Video Tutorial Script: Achievement System

**Duration:** 8-10 minutes  
**Target Audience:** Developers integrating achievements

---

## Introduction (0:00 - 0:30)

"Welcome to the HorrorProject Achievement System tutorial. In this video, you'll learn how to integrate achievements into your game in just a few minutes. We'll cover unlocking achievements, tracking progress, and displaying notifications."

**Show:** Title card with achievement icons

---

## Part 1: Getting Started (0:30 - 2:00)

"First, let's get the Achievement Subsystem. This is the core component that manages all achievements."

**Show:** Code editor with C++ file

```cpp
UAchievementSubsystem* Achievements = 
    GetGameInstance()->GetSubsystem<UAchievementSubsystem>();
```

"The subsystem is automatically initialized when your game starts, so you can access it from anywhere."

**Show:** Blueprint equivalent

"In Blueprint, simply use the 'Get Achievement Subsystem' node."

---

## Part 2: Unlocking Achievements (2:00 - 4:00)

"Now let's unlock our first achievement. It's as simple as calling UnlockAchievement with the achievement ID."

**Show:** Code example

```cpp
Achievements->UnlockAchievement(FName("ACH_FirstSteps"));
```

**Show:** Game running, achievement notification appearing

"Notice the notification that appears automatically. The system handles all the UI for you."

"You can also listen for unlock events:"

```cpp
Achievements->OnAchievementUnlocked.AddDynamic(this, &UMyClass::OnAchievementUnlocked);
```

**Show:** Callback function being called

---

## Part 3: Progressive Achievements (4:00 - 6:00)

"Progressive achievements track player progress over time. Let's implement a 'Collector' achievement that requires collecting 10 items."

**Show:** Code example

```cpp
// When player collects an item
Achievements->UpdateAchievementProgress(FName("ACH_Collector"), 1.0f);
```

**Show:** Progress bar updating in game

"The system automatically unlocks the achievement when progress reaches 100%."

"You can check current progress:"

```cpp
float Progress = Achievements->GetAchievementProgress(FName("ACH_Collector"));
```

---

## Part 4: Achievement Tracker Component (6:00 - 7:30)

"For convenience, use the Achievement Tracker component. It automatically tracks common game events."

**Show:** Adding component to Character

```cpp
AchievementTracker = CreateDefaultSubobject<UAchievementTracker>(TEXT("AchievementTracker"));
```

"Now you can track events with simple method calls:"

```cpp
AchievementTracker->TrackRoomDiscovered(TEXT("Room_01"));
AchievementTracker->TrackEvidenceCollected(TEXT("Evidence_Key"));
AchievementTracker->TrackGhostEncounter();
```

**Show:** Multiple achievements unlocking as player progresses

---

## Part 5: Testing (7:30 - 8:30)

"During development, you'll want to test achievements. Reset them easily:"

```cpp
Achievements->ResetAchievement(FName("ACH_FirstSteps"));
```

**Show:** Achievement being reset and unlocked again

"View all achievements:"

```cpp
TArray<FAchievementData> AllAchievements = Achievements->GetAllAchievements();
```

**Show:** Achievement list UI

---

## Conclusion (8:30 - 10:00)

"That's it! You now know how to:
- Get the Achievement Subsystem
- Unlock achievements
- Track progressive achievements
- Use the Achievement Tracker component
- Test your achievements"

**Show:** Montage of achievements unlocking

"For more information, check out the full documentation in the Docs folder. Thanks for watching!"

**Show:** End card with links to documentation

---

## Visual Notes

- Use split screen: code on left, game on right
- Highlight important code lines
- Show achievement notifications prominently
- Use smooth transitions between sections
- Include background music (subtle, non-intrusive)
- Add text overlays for key points

## B-Roll Suggestions

- Achievement icons
- Progress bars filling
- Notification animations
- Player performing achievement actions
- Achievement list UI
