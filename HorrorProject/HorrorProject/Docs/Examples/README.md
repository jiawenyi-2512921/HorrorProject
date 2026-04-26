# Interactive API Documentation Index

Quick reference for all core systems with interactive examples.

---

## Core Systems

### 1. Achievement System
**Quick Start:** [Achievements_QuickStart.md](../QuickStart/Achievements_QuickStart.md)  
**Code Examples:** [Achievements_Examples.md](Achievements_Examples.md)  
**Video Tutorial:** [01_Achievements_Tutorial_Script.md](../VideoTutorials/01_Achievements_Tutorial_Script.md)  
**Full Documentation:** `/Source/HorrorProject/Achievements/`

**Key Classes:**
- `UAchievementSubsystem` - Main achievement management
- `UAchievementTracker` - Component for tracking progress
- `FAchievementData` - Achievement definition structure

**Common Use Cases:**
```cpp
// Unlock achievement
Achievements->UnlockAchievement(FName("ACH_FirstSteps"));

// Track progress
Achievements->UpdateAchievementProgress(FName("ACH_Collector"), 1.0f);

// Listen for unlocks
Achievements->OnAchievementUnlocked.AddDynamic(this, &UMyClass::OnAchievementUnlocked);
```

---

### 2. Analytics System
**Quick Start:** [Analytics_QuickStart.md](../QuickStart/Analytics_QuickStart.md)  
**Code Examples:** [Analytics_Examples.md](Analytics_Examples.md)  
**Video Tutorial:** [02_Analytics_Tutorial_Script.md](../VideoTutorials/02_Analytics_Tutorial_Script.md)  
**Full Documentation:** `/Source/HorrorProject/Analytics/Docs/`

**Key Classes:**
- `UAnalyticsSubsystem` - Event tracking and consent management
- `UTelemetrySubsystem` - Performance and gameplay telemetry
- `UMetricsCollector` - Custom metrics collection

**Common Use Cases:**
```cpp
// Track event
Analytics->TrackEvent(TEXT("level_complete"), Parameters);

// Get performance stats
float AvgFPS = PerfTelemetry->GetAverageFPS();

// User consent
Analytics->SetUserConsent(Consent);
```

---

### 3. Accessibility System
**Quick Start:** [Accessibility_QuickStart.md](../QuickStart/Accessibility_QuickStart.md)  
**Code Examples:** [Accessibility_Examples.md](Accessibility_Examples.md)  
**Video Tutorial:** [03_Accessibility_Tutorial_Script.md](../VideoTutorials/03_Accessibility_Tutorial_Script.md)  
**Full Documentation:** `/Source/HorrorProject/Accessibility/Docs/`

**Key Classes:**
- `UAccessibilitySubsystem` - Main accessibility system
- `FAccessibilitySettings` - Settings structure
- `EColorBlindMode` - Color blind mode enum

**Common Use Cases:**
```cpp
// Display subtitle
Accessibility->DisplaySubtitle(Text, Duration, Speaker);

// Visual audio cue
Accessibility->TriggerVisualAudioCue(Location, Type, Intensity);

// Color blind mode
Accessibility->SetColorBlindMode(EColorBlindMode::Protanopia);
```

---

### 4. Network Multiplayer System
**Quick Start:** [NetworkMultiplayer_QuickStart.md](../QuickStart/NetworkMultiplayer_QuickStart.md)  
**Code Examples:** [NetworkMultiplayer_Examples.md](NetworkMultiplayer_Examples.md)  
**Video Tutorial:** [04_NetworkMultiplayer_Tutorial_Script.md](../VideoTutorials/04_NetworkMultiplayer_Tutorial_Script.md)  
**Full Documentation:** `/Source/HorrorProject/Network/`

**Key Classes:**
- `UMultiplayerSessionSubsystem` - Session management
- `AReplicatedGameState` - Game state replication
- `AReplicatedPlayerState` - Player state replication

**Common Use Cases:**
```cpp
// Create session
SessionSubsystem->CreateSession(4, false, TEXT("MyGame"));

// Find sessions
SessionSubsystem->FindSessions(10, false);

// Join session
SessionSubsystem->JoinSession(0);

// Replicate property
UPROPERTY(Replicated)
int32 Score;
```

---

### 5. Localization System
**Quick Start:** [Localization_QuickStart.md](../QuickStart/Localization_QuickStart.md)  
**Code Examples:** [Localization_Examples.md](Localization_Examples.md)  
**Video Tutorial:** [05_Localization_Tutorial_Script.md](../VideoTutorials/05_Localization_Tutorial_Script.md)  
**Full Documentation:** `/Source/HorrorProject/Localization/`

**Key Classes:**
- `ULocalizationSubsystem` - Language management
- `UUILocalizationComponent` - Auto-updating UI component
- `ELanguage` - Supported language enum

**Common Use Cases:**
```cpp
// Change language
Localization->SetLanguage(ELanguage::Chinese);

// Get localized text
FText Text = Localization->GetLocalizedText(TEXT("UI.MainMenu.Start"));

// Format string
FText Formatted = UTextLocalizationLibrary::FormatLocalizedText(Key, Args);
```

---

## Additional Systems

### AI System
**Documentation:** `/Docs/AI/`
- Behavior Trees
- Perception System
- AI Optimization

### Audio System
**Documentation:** `/Docs/Audio/`
- Audio Implementation
- Sound Design
- Audio Optimization

### Blueprint System
**Documentation:** `/Docs/Blueprint/`
- Interaction System
- UI Templates
- Blueprint Best Practices

### Performance Optimization
**Documentation:** `/Docs/Performance/`
- Profiling Guide
- Optimization Techniques
- Memory Management

---

## FAQ
**General Questions:** [FAQ.md](../FAQ.md)

---

## Video Tutorial Playlist

1. [Achievement System](../VideoTutorials/01_Achievements_Tutorial_Script.md) - 8-10 min
2. [Analytics System](../VideoTutorials/02_Analytics_Tutorial_Script.md) - 10-12 min
3. [Accessibility System](../VideoTutorials/03_Accessibility_Tutorial_Script.md) - 9-11 min
4. [Network Multiplayer](../VideoTutorials/04_NetworkMultiplayer_Tutorial_Script.md) - 10-12 min
5. [Localization System](../VideoTutorials/05_Localization_Tutorial_Script.md) - 8-10 min

---

## Quick Reference Cards

### Achievement System
```cpp
// Get subsystem
UAchievementSubsystem* Achievements = 
    GetGameInstance()->GetSubsystem<UAchievementSubsystem>();

// Unlock
Achievements->UnlockAchievement(FName("ACH_ID"));

// Progress
Achievements->UpdateAchievementProgress(FName("ACH_ID"), 1.0f);

// Query
TArray<FAchievementData> All = Achievements->GetAllAchievements();
float Completion = Achievements->GetCompletionPercentage();
```

### Analytics System
```cpp
// Get subsystem
UAnalyticsSubsystem* Analytics = 
    GetGameInstance()->GetSubsystem<UAnalyticsSubsystem>();

// Track event
TMap<FString, FString> Params;
Params.Add(TEXT("key"), TEXT("value"));
Analytics->TrackEvent(TEXT("event_name"), Params);

// Consent
FUserConsent Consent;
Consent.bAnalyticsEnabled = true;
Analytics->SetUserConsent(Consent);
```

### Accessibility System
```cpp
// Get subsystem
UAccessibilitySubsystem* Accessibility = 
    GetGameInstance()->GetSubsystem<UAccessibilitySubsystem>();

// Subtitle
Accessibility->DisplaySubtitle(Text, Duration, Speaker);

// Visual cue
Accessibility->TriggerVisualAudioCue(Location, Type, Intensity);

// Settings
FAccessibilitySettings Settings;
Settings.bSubtitlesEnabled = true;
Accessibility->ApplyAccessibilitySettings(Settings);
```

### Network Multiplayer
```cpp
// Get subsystem
UMultiplayerSessionSubsystem* Session = 
    GetGameInstance()->GetSubsystem<UMultiplayerSessionSubsystem>();

// Create
Session->CreateSession(4, false, TEXT("Name"));

// Find
Session->FindSessions(10, false);

// Join
Session->JoinSession(0);

// Replicate
UPROPERTY(Replicated)
int32 MyVariable;
```

### Localization System
```cpp
// Get subsystem
ULocalizationSubsystem* Localization = 
    GetGameInstance()->GetSubsystem<ULocalizationSubsystem>();

// Change language
Localization->SetLanguage(ELanguage::Chinese);

// Get text
FText Text = Localization->GetLocalizedText(TEXT("Key"));

// Format
TArray<FString> Args = {TEXT("Value1"), TEXT("Value2")};
FText Formatted = UTextLocalizationLibrary::FormatLocalizedText(TEXT("Key"), Args);
```

---

## Testing

All systems include comprehensive automated tests:

```
Session Frontend → Automation → HorrorProject.[System]
```

Test categories:
- `HorrorProject.Achievements` - Achievement system tests
- `HorrorProject.Analytics` - Analytics system tests
- `HorrorProject.Accessibility` - Accessibility tests
- `HorrorProject.Network` - Multiplayer tests
- `HorrorProject.Localization` - Localization tests

---

## Support

- **Documentation:** Check system-specific docs in `/Docs/` and `/Source/HorrorProject/`
- **FAQ:** [FAQ.md](../FAQ.md)
- **Examples:** Code examples in this directory
- **Video Tutorials:** Step-by-step video scripts in `/Docs/VideoTutorials/`
