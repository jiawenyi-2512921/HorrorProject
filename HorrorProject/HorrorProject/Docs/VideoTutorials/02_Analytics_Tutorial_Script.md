# Video Tutorial Script: Analytics System

**Duration:** 10-12 minutes  
**Target Audience:** Developers implementing analytics

---

## Introduction (0:00 - 0:30)

"Welcome to the HorrorProject Analytics System tutorial. Learn how to track player behavior, monitor performance, and generate insightful reports - all while staying GDPR and CCPA compliant."

**Show:** Dashboard with live analytics data

---

## Part 1: User Consent (0:30 - 2:00)

"Before tracking anything, we must get user consent. This is required by GDPR and CCPA."

**Show:** Consent dialog in game

```cpp
UAnalyticsSubsystem* Analytics = 
    GetGameInstance()->GetSubsystem<UAnalyticsSubsystem>();

FUserConsent Consent;
Consent.bAnalyticsEnabled = true;
Consent.bCrashReportingEnabled = true;
Consent.bPersonalizedAdsEnabled = false;
Analytics->SetUserConsent(Consent);
```

**Show:** Consent being saved

"The system automatically anonymizes user IDs and respects privacy settings."

---

## Part 2: Tracking Events (2:00 - 4:30)

"Events are the foundation of analytics. Track any player action:"

**Show:** Code examples

```cpp
// Simple event
Analytics->TrackEvent(TEXT("level_start"));

// Event with parameters
TMap<FString, FString> Params;
Params.Add(TEXT("level_name"), TEXT("Chapter_1"));
Params.Add(TEXT("difficulty"), TEXT("Normal"));
Analytics->TrackEvent(TEXT("level_complete"), Params);
```

**Show:** Events appearing in log file

"Events are queued and flushed every 30 seconds, so there's minimal performance impact."

**Show:** Common events being tracked in gameplay

---

## Part 3: Performance Telemetry (4:30 - 6:30)

"The Telemetry Subsystem automatically tracks performance metrics."

**Show:** Performance graphs

```cpp
UTelemetrySubsystem* Telemetry = 
    GetGameInstance()->GetSubsystem<UTelemetrySubsystem>();

UPerformanceTelemetry* PerfTelemetry = Telemetry->GetPerformanceTelemetry();
float AvgFPS = PerfTelemetry->GetAverageFPS();
float AvgFrameTime = PerfTelemetry->GetAverageFrameTime();
float MemoryUsage = PerfTelemetry->GetMemoryUsageMB();
```

**Show:** Real-time FPS counter and memory usage

"Performance is tracked automatically every frame, with statistics calculated in real-time."

---

## Part 4: Gameplay Telemetry (6:30 - 8:00)

"Track gameplay-specific metrics to understand player behavior."

**Show:** Gameplay footage with telemetry overlay

```cpp
UGameplayTelemetry* GameplayTelemetry = Telemetry->GetGameplayTelemetry();

GameplayTelemetry->RecordLevelCompleted(TEXT("Chapter_1"));
GameplayTelemetry->RecordDeath(TEXT("Ghost"));
GameplayTelemetry->RecordAchievement(TEXT("ACH_FirstSteps"));
GameplayTelemetry->RecordItemCollected(TEXT("Key"));
```

**Show:** Statistics updating in real-time

---

## Part 5: Generating Reports (8:00 - 10:00)

"Now let's generate reports from our collected data."

**Show:** PowerShell terminal

```powershell
# Aggregate data
.\DataAggregator.ps1 -IncludeTelemetry

# Analyze data
.\DataAnalyzer.ps1

# Generate HTML report
.\ReportGenerator.ps1 -Format HTML
```

**Show:** Report being generated

**Show:** Opening HTML report in browser

"The report shows comprehensive analytics: performance metrics, top events, session statistics, and gameplay data."

**Show:** Scrolling through report sections

---

## Part 6: Live Dashboard (10:00 - 11:00)

"For real-time monitoring, use the analytics dashboard."

**Show:** Dashboard in browser

```powershell
.\DashboardUpdater.ps1 -AutoRefresh
```

**Show:** Dashboard auto-refreshing with live data

"The dashboard updates automatically, showing current FPS, active sessions, and recent events."

---

## Part 7: Privacy Features (11:00 - 11:45)

"The system includes full privacy compliance features."

**Show:** Privacy UI

```cpp
// Export user data
FString ExportedData = Analytics->ExportUserData();

// Delete user data
Analytics->DeleteUserData();

// Revoke consent
Analytics->RevokeConsent();
```

**Show:** Data being exported and deleted

---

## Conclusion (11:45 - 12:00)

"You now know how to:
- Get user consent
- Track events and metrics
- Monitor performance
- Generate reports
- Use the live dashboard
- Respect user privacy"

**Show:** Dashboard with full analytics

"Check the documentation for advanced features. Thanks for watching!"

---

## Visual Notes

- Show dashboard prominently
- Use real data examples
- Highlight privacy features
- Demonstrate report generation live
- Use graphs and charts
- Show before/after comparisons

## B-Roll Suggestions

- Dashboard animations
- Report generation process
- Graphs updating
- Event tracking in gameplay
- Performance metrics
- Privacy consent UI
