# Code Examples: Achievement System

Complete code examples for implementing achievements.

---

## Example 1: Basic Achievement Unlock

```cpp
// MyGameMode.h
#pragma once
#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Achievements/AchievementSubsystem.h"
#include "MyGameMode.generated.h"

UCLASS()
class HORRORPROJECT_API AMyGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    void OnPlayerStartGame();
    void OnLevelComplete(const FString& LevelName);
};

// MyGameMode.cpp
#include "MyGameMode.h"

void AMyGameMode::OnPlayerStartGame()
{
    UAchievementSubsystem* Achievements = 
        GetGameInstance()->GetSubsystem<UAchievementSubsystem>();
    
    if (Achievements)
    {
        Achievements->UnlockAchievement(FName("ACH_FirstSteps"));
    }
}

void AMyGameMode::OnLevelComplete(const FString& LevelName)
{
    UAchievementSubsystem* Achievements = 
        GetGameInstance()->GetSubsystem<UAchievementSubsystem>();
    
    if (Achievements)
    {
        TMap<FString, FString> Params;
        Params.Add(TEXT("level_name"), LevelName);
        
        Achievements->UnlockAchievement(FName("ACH_LevelComplete"));
    }
}
```

---

## Example 2: Progressive Achievement

```cpp
// CollectibleItem.h
#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CollectibleItem.generated.h"

UCLASS()
class HORRORPROJECT_API ACollectibleItem : public AActor
{
    GENERATED_BODY()

public:
    UFUNCTION()
    void OnPickup(AActor* Collector);
};

// CollectibleItem.cpp
#include "CollectibleItem.h"
#include "Achievements/AchievementSubsystem.h"

void ACollectibleItem::OnPickup(AActor* Collector)
{
    UAchievementSubsystem* Achievements = 
        GetWorld()->GetGameInstance()->GetSubsystem<UAchievementSubsystem>();
    
    if (Achievements)
    {
        // Update collector achievement (+1 item)
        Achievements->UpdateAchievementProgress(FName("ACH_Collector"), 1.0f);
        
        // Check if achievement is complete
        float Progress = Achievements->GetAchievementProgress(FName("ACH_Collector"));
        if (Progress >= 100.0f)
        {
            UE_LOG(LogTemp, Log, TEXT("Collector achievement complete!"));
        }
    }
    
    Destroy();
}
```

---

## Example 3: Achievement Tracker Component

```cpp
// MyCharacter.h
#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Achievements/AchievementTracker.h"
#include "MyCharacter.generated.h"

UCLASS()
class HORRORPROJECT_API AMyCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    AMyCharacter();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Achievements")
    UAchievementTracker* AchievementTracker;

    void OnRoomEntered(const FString& RoomID);
    void OnEvidenceCollected(const FString& EvidenceID);
    void OnGhostEncountered();
};

// MyCharacter.cpp
#include "MyCharacter.h"

AMyCharacter::AMyCharacter()
{
    AchievementTracker = CreateDefaultSubobject<UAchievementTracker>(TEXT("AchievementTracker"));
}

void AMyCharacter::OnRoomEntered(const FString& RoomID)
{
    if (AchievementTracker)
    {
        AchievementTracker->TrackRoomDiscovered(RoomID);
    }
}

void AMyCharacter::OnEvidenceCollected(const FString& EvidenceID)
{
    if (AchievementTracker)
    {
        AchievementTracker->TrackEvidenceCollected(EvidenceID);
    }
}

void AMyCharacter::OnGhostEncountered()
{
    if (AchievementTracker)
    {
        AchievementTracker->TrackGhostEncounter();
    }
}
```

---

## Example 4: Achievement Event Handling

```cpp
// AchievementManager.h
#pragma once
#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Achievements/AchievementSubsystem.h"
#include "AchievementManager.generated.h"

UCLASS()
class HORRORPROJECT_API UAchievementManager : public UObject
{
    GENERATED_BODY()

public:
    void Initialize(UWorld* World);
    void Shutdown();

    UFUNCTION()
    void OnAchievementUnlocked(const FAchievementData& Achievement);

    UFUNCTION()
    void OnAchievementProgress(FName AchievementID, float Progress);

private:
    UAchievementSubsystem* AchievementSubsystem;
};

// AchievementManager.cpp
#include "AchievementManager.h"

void UAchievementManager::Initialize(UWorld* World)
{
    AchievementSubsystem = World->GetGameInstance()->GetSubsystem<UAchievementSubsystem>();
    
    if (AchievementSubsystem)
    {
        AchievementSubsystem->OnAchievementUnlocked.AddDynamic(
            this, &UAchievementManager::OnAchievementUnlocked);
        
        AchievementSubsystem->OnAchievementProgress.AddDynamic(
            this, &UAchievementManager::OnAchievementProgress);
    }
}

void UAchievementManager::Shutdown()
{
    if (AchievementSubsystem)
    {
        AchievementSubsystem->OnAchievementUnlocked.RemoveDynamic(
            this, &UAchievementManager::OnAchievementUnlocked);
        
        AchievementSubsystem->OnAchievementProgress.RemoveDynamic(
            this, &UAchievementManager::OnAchievementProgress);
    }
}

void UAchievementManager::OnAchievementUnlocked(const FAchievementData& Achievement)
{
    UE_LOG(LogTemp, Log, TEXT("Achievement Unlocked: %s"), *Achievement.Name.ToString());
    UE_LOG(LogTemp, Log, TEXT("Description: %s"), *Achievement.Description.ToString());
    
    // Play sound, show notification, etc.
}

void UAchievementManager::OnAchievementProgress(FName AchievementID, float Progress)
{
    UE_LOG(LogTemp, Log, TEXT("Achievement %s: %.2f%% complete"), 
        *AchievementID.ToString(), Progress);
}
```

---

## Example 5: Time-Based Achievement

```cpp
// SpeedrunTracker.h
#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SpeedrunTracker.generated.h"

UCLASS()
class HORRORPROJECT_API USpeedrunTracker : public UActorComponent
{
    GENERATED_BODY()

public:
    USpeedrunTracker();

    void StartTimer();
    void StopTimer();

private:
    float StartTime;
    float TargetTime;
};

// SpeedrunTracker.cpp
#include "SpeedrunTracker.h"
#include "Achievements/AchievementSubsystem.h"

USpeedrunTracker::USpeedrunTracker()
{
    TargetTime = 300.0f; // 5 minutes
}

void USpeedrunTracker::StartTimer()
{
    StartTime = GetWorld()->GetTimeSeconds();
}

void USpeedrunTracker::StopTimer()
{
    float ElapsedTime = GetWorld()->GetTimeSeconds() - StartTime;
    
    if (ElapsedTime <= TargetTime)
    {
        UAchievementSubsystem* Achievements = 
            GetWorld()->GetGameInstance()->GetSubsystem<UAchievementSubsystem>();
        
        if (Achievements)
        {
            Achievements->UnlockAchievement(FName("ACH_Speedrunner"));
            
            UE_LOG(LogTemp, Log, TEXT("Speedrun complete in %.2f seconds!"), ElapsedTime);
        }
    }
}
```

---

## Example 6: Secret Achievement

```cpp
// SecretDiscovery.cpp
#include "SecretDiscovery.h"
#include "Achievements/AchievementSubsystem.h"

void ASecretDiscovery::OnSecretFound()
{
    UAchievementSubsystem* Achievements = 
        GetWorld()->GetGameInstance()->GetSubsystem<UAchievementSubsystem>();
    
    if (Achievements)
    {
        // Secret achievements don't show progress
        Achievements->UnlockAchievement(FName("ACH_SecretRoom"));
        
        // Log for debugging only
        UE_LOG(LogTemp, Log, TEXT("Secret achievement unlocked!"));
    }
}
```

---

## Example 7: Achievement Query

```cpp
// AchievementUI.cpp
#include "AchievementUI.h"
#include "Achievements/AchievementSubsystem.h"

void UAchievementUI::PopulateAchievementList()
{
    UAchievementSubsystem* Achievements = 
        GetWorld()->GetGameInstance()->GetSubsystem<UAchievementSubsystem>();
    
    if (Achievements)
    {
        // Get all achievements
        TArray<FAchievementData> AllAchievements = Achievements->GetAllAchievements();
        
        // Get unlocked achievements
        TArray<FAchievementData> Unlocked = Achievements->GetUnlockedAchievements();
        
        // Get locked achievements
        TArray<FAchievementData> Locked = Achievements->GetLockedAchievements();
        
        // Get completion percentage
        float Completion = Achievements->GetCompletionPercentage();
        
        UE_LOG(LogTemp, Log, TEXT("Achievement Completion: %.2f%%"), Completion);
        
        // Display in UI
        for (const FAchievementData& Achievement : AllAchievements)
        {
            if (Achievement.bUnlocked)
            {
                // Show unlocked achievement
            }
            else if (!Achievement.bSecret)
            {
                // Show locked achievement (hide secrets)
                float Progress = Achievements->GetAchievementProgress(Achievement.ID);
                // Show progress bar
            }
        }
    }
}
```

---

## Blueprint Examples

### Unlock Achievement
```
Event BeginPlay
  → Get Achievement Subsystem
  → Unlock Achievement (Name: "ACH_FirstSteps")
```

### Track Progress
```
On Item Collected
  → Get Achievement Subsystem
  → Update Achievement Progress (Name: "ACH_Collector", Progress: 1.0)
```

### Listen for Unlock
```
Event BeginPlay
  → Get Achievement Subsystem
  → Bind Event to OnAchievementUnlocked
  
On Achievement Unlocked
  → Print String (Achievement Name)
  → Play Sound
  → Show Notification
```
