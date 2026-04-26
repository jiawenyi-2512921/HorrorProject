# Analytics - Quick Start Guide

Start tracking game data in 5 minutes.

## Step 1: Get User Consent (1 min)

```cpp
UAnalyticsSubsystem* Analytics = 
    GetGameInstance()->GetSubsystem<UAnalyticsSubsystem>();

// Request consent (GDPR/CCPA compliant)
FUserConsent Consent;
Consent.bAnalyticsEnabled = true;
Consent.bCrashReportingEnabled = true;
Consent.bPersonalizedAdsEnabled = false;
Analytics->SetUserConsent(Consent);
```

## Step 2: Track Events (2 min)

```cpp
// Simple event
Analytics->TrackEvent(TEXT("level_start"));

// Event with parameters
TMap<FString, FString> Params;
Params.Add(TEXT("level_name"), TEXT("Chapter_1"));
Params.Add(TEXT("difficulty"), TEXT("Normal"));
Params.Add(TEXT("player_level"), TEXT("5"));
Analytics->TrackEvent(TEXT("level_complete"), Params);
```

## Step 3: Track Performance (1 min)

```cpp
UTelemetrySubsystem* Telemetry = 
    GetGameInstance()->GetSubsystem<UTelemetrySubsystem>();

// Performance tracked automatically
UPerformanceTelemetry* PerfTelemetry = Telemetry->GetPerformanceTelemetry();
float AvgFPS = PerfTelemetry->GetAverageFPS();
float AvgFrameTime = PerfTelemetry->GetAverageFrameTime();
```

## Step 4: Track Gameplay (1 min)

```cpp
UGameplayTelemetry* GameplayTelemetry = Telemetry->GetGameplayTelemetry();

// Track gameplay events
GameplayTelemetry->RecordLevelCompleted(TEXT("Chapter_1"));
GameplayTelemetry->RecordDeath(TEXT("Ghost"));
GameplayTelemetry->RecordAchievement(TEXT("ACH_FirstSteps"));
GameplayTelemetry->RecordItemCollected(TEXT("Key"));
GameplayTelemetry->RecordEnemyDefeated(TEXT("Golem"));
```

## Blueprint Quick Start

1. Get Analytics Subsystem
2. Set User Consent (Analytics: true, Crash: true)
3. Track Event (Name: "game_start")
4. Track Event with Parameters (Name: "level_complete", Params: level_name="Chapter_1")

## Common Events to Track

```cpp
// Game flow
Analytics->TrackEvent(TEXT("game_start"));
Analytics->TrackEvent(TEXT("game_end"));
Analytics->TrackEvent(TEXT("level_start"));
Analytics->TrackEvent(TEXT("level_complete"));

// Player actions
Analytics->TrackEvent(TEXT("item_collected"));
Analytics->TrackEvent(TEXT("door_opened"));
Analytics->TrackEvent(TEXT("puzzle_solved"));
Analytics->TrackEvent(TEXT("player_death"));

// UI interactions
Analytics->TrackEvent(TEXT("menu_opened"));
Analytics->TrackEvent(TEXT("settings_changed"));
Analytics->TrackEvent(TEXT("achievement_unlocked"));
```

## Generate Reports

Run PowerShell scripts in `/Source/HorrorProject/Analytics/Scripts/`:

```powershell
# Aggregate data
.\DataAggregator.ps1 -IncludeTelemetry

# Analyze data
.\DataAnalyzer.ps1

# Generate HTML report
.\ReportGenerator.ps1 -Format HTML

# Update dashboard
.\DashboardUpdater.ps1 -AutoRefresh
```

## View Dashboard

Open: `Saved/Analytics/Dashboard/index.html`

Dashboard shows:
- Real-time FPS and performance
- Session statistics
- Top events
- Gameplay metrics
- Error reports

## Privacy Compliance

The system is GDPR/CCPA compliant:
- Requires explicit consent
- Anonymizes user IDs
- Supports data export
- Supports data deletion
- No PII collection

```cpp
// Export user data
FString ExportedData = Analytics->ExportUserData();

// Delete user data
Analytics->DeleteUserData();

// Revoke consent
Analytics->RevokeConsent();
```

## Testing

```cpp
// Check if analytics enabled
bool bEnabled = Analytics->IsAnalyticsEnabled();

// Get event count
int32 EventCount = Analytics->GetEventCount();

// Flush events immediately (for testing)
Analytics->FlushEvents();
```

## Data Storage

Analytics data saved to:
- `Saved/Analytics/events.jsonl` - Event log
- `Saved/Analytics/sessions.json` - Session history
- `Saved/Analytics/Telemetry/` - Telemetry reports
- `Saved/Analytics/Reports/` - Generated reports

## Next Steps

- Define custom events
- Set up automated reporting
- Integrate with backend
- Monitor dashboard regularly
