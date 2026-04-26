# Code Examples: Analytics System

Complete code examples for implementing analytics and telemetry.

---

## Example 1: Basic Event Tracking

```cpp
// GameAnalytics.h
#pragma once
#include "CoreMinimal.h"
#include "Analytics/AnalyticsSubsystem.h"
#include "GameAnalytics.generated.h"

UCLASS()
class HORRORPROJECT_API UGameAnalytics : public UObject
{
    GENERATED_BODY()

public:
    static void TrackGameStart(UWorld* World);
    static void TrackLevelComplete(UWorld* World, const FString& LevelName, float CompletionTime);
    static void TrackPlayerDeath(UWorld* World, const FString& Cause);
};

// GameAnalytics.cpp
#include "GameAnalytics.h"

void UGameAnalytics::TrackGameStart(UWorld* World)
{
    UAnalyticsSubsystem* Analytics = 
        World->GetGameInstance()->GetSubsystem<UAnalyticsSubsystem>();
    
    if (Analytics && Analytics->IsAnalyticsEnabled())
    {
        Analytics->TrackEvent(TEXT("game_start"));
    }
}

void UGameAnalytics::TrackLevelComplete(UWorld* World, const FString& LevelName, float CompletionTime)
{
    UAnalyticsSubsystem* Analytics = 
        World->GetGameInstance()->GetSubsystem<UAnalyticsSubsystem>();
    
    if (Analytics && Analytics->IsAnalyticsEnabled())
    {
        TMap<FString, FString> Params;
        Params.Add(TEXT("level_name"), LevelName);
        Params.Add(TEXT("completion_time"), FString::Printf(TEXT("%.2f"), CompletionTime));
        Params.Add(TEXT("timestamp"), FDateTime::Now().ToString());
        
        Analytics->TrackEvent(TEXT("level_complete"), Params);
    }
}

void UGameAnalytics::TrackPlayerDeath(UWorld* World, const FString& Cause)
{
    UAnalyticsSubsystem* Analytics = 
        World->GetGameInstance()->GetSubsystem<UAnalyticsSubsystem>();
    
    if (Analytics && Analytics->IsAnalyticsEnabled())
    {
        TMap<FString, FString> Params;
        Params.Add(TEXT("death_cause"), Cause);
        
        Analytics->TrackEvent(TEXT("player_death"), Params);
    }
}
```

---

## Example 2: User Consent Management

```cpp
// ConsentManager.h
#pragma once
#include "CoreMinimal.h"
#include "Analytics/AnalyticsSubsystem.h"
#include "ConsentManager.generated.h"

UCLASS()
class HORRORPROJECT_API UConsentManager : public UObject
{
    GENERATED_BODY()

public:
    void ShowConsentDialog(UWorld* World);
    void AcceptConsent(UWorld* World, bool bAnalytics, bool bCrashReporting);
    void RevokeConsent(UWorld* World);
    void ExportUserData(UWorld* World);
    void DeleteUserData(UWorld* World);
};

// ConsentManager.cpp
#include "ConsentManager.h"

void UConsentManager::AcceptConsent(UWorld* World, bool bAnalytics, bool bCrashReporting)
{
    UAnalyticsSubsystem* Analytics = 
        World->GetGameInstance()->GetSubsystem<UAnalyticsSubsystem>();
    
    if (Analytics)
    {
        FUserConsent Consent;
        Consent.bAnalyticsEnabled = bAnalytics;
        Consent.bCrashReportingEnabled = bCrashReporting;
        Consent.bPersonalizedAdsEnabled = false;
        Consent.ConsentVersion = 1;
        Consent.ConsentTimestamp = FDateTime::Now();
        
        Analytics->SetUserConsent(Consent);
        
        UE_LOG(LogTemp, Log, TEXT("User consent accepted: Analytics=%d, Crash=%d"), 
            bAnalytics, bCrashReporting);
    }
}

void UConsentManager::RevokeConsent(UWorld* World)
{
    UAnalyticsSubsystem* Analytics = 
        World->GetGameInstance()->GetSubsystem<UAnalyticsSubsystem>();
    
    if (Analytics)
    {
        Analytics->RevokeConsent();
        UE_LOG(LogTemp, Log, TEXT("User consent revoked"));
    }
}

void UConsentManager::ExportUserData(UWorld* World)
{
    UAnalyticsSubsystem* Analytics = 
        World->GetGameInstance()->GetSubsystem<UAnalyticsSubsystem>();
    
    if (Analytics)
    {
        FString ExportedData = Analytics->ExportUserData();
        
        // Save to file
        FString FilePath = FPaths::ProjectSavedDir() / TEXT("UserData_Export.json");
        FFileHelper::SaveStringToFile(ExportedData, *FilePath);
        
        UE_LOG(LogTemp, Log, TEXT("User data exported to: %s"), *FilePath);
    }
}

void UConsentManager::DeleteUserData(UWorld* World)
{
    UAnalyticsSubsystem* Analytics = 
        World->GetGameInstance()->GetSubsystem<UAnalyticsSubsystem>();
    
    if (Analytics)
    {
        Analytics->DeleteUserData();
        UE_LOG(LogTemp, Log, TEXT("User data deleted"));
    }
}
```

---

## Example 3: Performance Telemetry

```cpp
// PerformanceMonitor.h
#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Analytics/TelemetrySubsystem.h"
#include "PerformanceMonitor.generated.h"

UCLASS()
class HORRORPROJECT_API APerformanceMonitor : public AActor
{
    GENERATED_BODY()

public:
    APerformanceMonitor();
    virtual void Tick(float DeltaTime) override;

    void LogPerformanceStats();

private:
    float UpdateInterval;
    float TimeSinceLastUpdate;
};

// PerformanceMonitor.cpp
#include "PerformanceMonitor.h"

APerformanceMonitor::APerformanceMonitor()
{
    PrimaryActorTick.bCanEverTick = true;
    UpdateInterval = 5.0f;
    TimeSinceLastUpdate = 0.0f;
}

void APerformanceMonitor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    TimeSinceLastUpdate += DeltaTime;
    
    if (TimeSinceLastUpdate >= UpdateInterval)
    {
        LogPerformanceStats();
        TimeSinceLastUpdate = 0.0f;
    }
}

void APerformanceMonitor::LogPerformanceStats()
{
    UTelemetrySubsystem* Telemetry = 
        GetWorld()->GetGameInstance()->GetSubsystem<UTelemetrySubsystem>();
    
    if (Telemetry)
    {
        UPerformanceTelemetry* PerfTelemetry = Telemetry->GetPerformanceTelemetry();
        
        if (PerfTelemetry)
        {
            float AvgFPS = PerfTelemetry->GetAverageFPS();
            float MinFPS = PerfTelemetry->GetMinFPS();
            float MaxFPS = PerfTelemetry->GetMaxFPS();
            float AvgFrameTime = PerfTelemetry->GetAverageFrameTime();
            float MemoryUsage = PerfTelemetry->GetMemoryUsageMB();
            
            UE_LOG(LogTemp, Log, TEXT("Performance Stats:"));
            UE_LOG(LogTemp, Log, TEXT("  FPS: %.2f (Min: %.2f, Max: %.2f)"), AvgFPS, MinFPS, MaxFPS);
            UE_LOG(LogTemp, Log, TEXT("  Frame Time: %.2f ms"), AvgFrameTime);
            UE_LOG(LogTemp, Log, TEXT("  Memory: %.2f MB"), MemoryUsage);
        }
    }
}
```

---

## Example 4: Gameplay Telemetry

```cpp
// GameplayTracker.h
#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Analytics/TelemetrySubsystem.h"
#include "GameplayTracker.generated.h"

UCLASS()
class HORRORPROJECT_API UGameplayTracker : public UActorComponent
{
    GENERATED_BODY()

public:
    void TrackLevelStart(const FString& LevelName);
    void TrackLevelComplete(const FString& LevelName);
    void TrackPlayerDeath(const FString& Cause);
    void TrackItemCollected(const FString& ItemID);
    void TrackEnemyDefeated(const FString& EnemyType);
};

// GameplayTracker.cpp
#include "GameplayTracker.h"

void UGameplayTracker::TrackLevelStart(const FString& LevelName)
{
    UTelemetrySubsystem* Telemetry = 
        GetWorld()->GetGameInstance()->GetSubsystem<UTelemetrySubsystem>();
    
    if (Telemetry)
    {
        UGameplayTelemetry* GameplayTelemetry = Telemetry->GetGameplayTelemetry();
        if (GameplayTelemetry)
        {
            // Track level start time
            UE_LOG(LogTemp, Log, TEXT("Level Started: %s"), *LevelName);
        }
    }
}

void UGameplayTracker::TrackLevelComplete(const FString& LevelName)
{
    UTelemetrySubsystem* Telemetry = 
        GetWorld()->GetGameInstance()->GetSubsystem<UTelemetrySubsystem>();
    
    if (Telemetry)
    {
        UGameplayTelemetry* GameplayTelemetry = Telemetry->GetGameplayTelemetry();
        if (GameplayTelemetry)
        {
            GameplayTelemetry->RecordLevelCompleted(LevelName);
        }
    }
}

void UGameplayTracker::TrackPlayerDeath(const FString& Cause)
{
    UTelemetrySubsystem* Telemetry = 
        GetWorld()->GetGameInstance()->GetSubsystem<UTelemetrySubsystem>();
    
    if (Telemetry)
    {
        UGameplayTelemetry* GameplayTelemetry = Telemetry->GetGameplayTelemetry();
        if (GameplayTelemetry)
        {
            GameplayTelemetry->RecordDeath(Cause);
        }
    }
}

void UGameplayTracker::TrackItemCollected(const FString& ItemID)
{
    UTelemetrySubsystem* Telemetry = 
        GetWorld()->GetGameInstance()->GetSubsystem<UTelemetrySubsystem>();
    
    if (Telemetry)
    {
        UGameplayTelemetry* GameplayTelemetry = Telemetry->GetGameplayTelemetry();
        if (GameplayTelemetry)
        {
            GameplayTelemetry->RecordItemCollected(ItemID);
        }
    }
}

void UGameplayTracker::TrackEnemyDefeated(const FString& EnemyType)
{
    UTelemetrySubsystem* Telemetry = 
        GetWorld()->GetGameInstance()->GetSubsystem<UTelemetrySubsystem>();
    
    if (Telemetry)
    {
        UGameplayTelemetry* GameplayTelemetry = Telemetry->GetGameplayTelemetry();
        if (GameplayTelemetry)
        {
            GameplayTelemetry->RecordEnemyDefeated(EnemyType);
        }
    }
}
```

---

## Example 5: Error Telemetry

```cpp
// ErrorReporter.h
#pragma once
#include "CoreMinimal.h"
#include "Analytics/TelemetrySubsystem.h"
#include "ErrorReporter.generated.h"

UCLASS()
class HORRORPROJECT_API UErrorReporter : public UObject
{
    GENERATED_BODY()

public:
    static void ReportError(UWorld* World, const FString& Message, const FString& StackTrace);
    static void ReportWarning(UWorld* World, const FString& Message);
    static void ReportCritical(UWorld* World, const FString& Message, const FString& StackTrace);
};

// ErrorReporter.cpp
#include "ErrorReporter.h"

void UErrorReporter::ReportError(UWorld* World, const FString& Message, const FString& StackTrace)
{
    UTelemetrySubsystem* Telemetry = 
        World->GetGameInstance()->GetSubsystem<UTelemetrySubsystem>();
    
    if (Telemetry)
    {
        UErrorTelemetry* ErrorTelemetry = Telemetry->GetErrorTelemetry();
        if (ErrorTelemetry)
        {
            ErrorTelemetry->RecordError(EErrorSeverity::Error, Message, StackTrace);
        }
    }
    
    UE_LOG(LogTemp, Error, TEXT("Error: %s"), *Message);
}

void UErrorReporter::ReportWarning(UWorld* World, const FString& Message)
{
    UTelemetrySubsystem* Telemetry = 
        World->GetGameInstance()->GetSubsystem<UTelemetrySubsystem>();
    
    if (Telemetry)
    {
        UErrorTelemetry* ErrorTelemetry = Telemetry->GetErrorTelemetry();
        if (ErrorTelemetry)
        {
            ErrorTelemetry->RecordError(EErrorSeverity::Warning, Message, TEXT(""));
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Warning: %s"), *Message);
}

void UErrorReporter::ReportCritical(UWorld* World, const FString& Message, const FString& StackTrace)
{
    UTelemetrySubsystem* Telemetry = 
        World->GetGameInstance()->GetSubsystem<UTelemetrySubsystem>();
    
    if (Telemetry)
    {
        UErrorTelemetry* ErrorTelemetry = Telemetry->GetErrorTelemetry();
        if (ErrorTelemetry)
        {
            ErrorTelemetry->RecordError(EErrorSeverity::Critical, Message, StackTrace);
        }
    }
    
    UE_LOG(LogTemp, Fatal, TEXT("Critical Error: %s"), *Message);
}
```

---

## Example 6: Custom Metrics

```cpp
// CustomMetrics.cpp
#include "CustomMetrics.h"
#include "Analytics/MetricsCollector.h"

void UCustomMetrics::TrackLoadTime(UWorld* World, const FString& AssetName, float LoadTime)
{
    UTelemetrySubsystem* Telemetry = 
        World->GetGameInstance()->GetSubsystem<UTelemetrySubsystem>();
    
    if (Telemetry)
    {
        UMetricsCollector* Metrics = Telemetry->GetMetricsCollector();
        if (Metrics)
        {
            TMap<FString, FString> Tags;
            Tags.Add(TEXT("asset_name"), AssetName);
            
            Metrics->RecordMetric(TEXT("asset_load_time"), LoadTime, Tags);
        }
    }
}

void UCustomMetrics::TrackPlayerAction(UWorld* World, const FString& ActionName)
{
    UTelemetrySubsystem* Telemetry = 
        World->GetGameInstance()->GetSubsystem<UTelemetrySubsystem>();
    
    if (Telemetry)
    {
        UMetricsCollector* Metrics = Telemetry->GetMetricsCollector();
        if (Metrics)
        {
            TMap<FString, FString> Tags;
            Tags.Add(TEXT("action"), ActionName);
            
            Metrics->IncrementCounter(TEXT("player_actions"), Tags);
        }
    }
}
```

---

## Blueprint Examples

### Track Event
```
On Level Complete
  → Get Analytics Subsystem
  → Track Event (Name: "level_complete", Params: level_name="Chapter_1")
```

### Get Performance Stats
```
Event Tick
  → Get Telemetry Subsystem
  → Get Performance Telemetry
  → Get Average FPS
  → Display on HUD
```

### User Consent
```
On Accept Button Clicked
  → Get Analytics Subsystem
  → Set User Consent (Analytics: true, Crash: true)
```
