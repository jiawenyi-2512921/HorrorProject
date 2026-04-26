#include "AchievementSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "AchievementNotification.h"

void UAchievementSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	bInitialized = false;
	bPlatformAvailable = false;

	InitializeAchievements();
	LoadAchievementData();

	// Check platform availability
	#if PLATFORM_WINDOWS
	bPlatformAvailable = true; // Steam/Epic available
	#endif

	bInitialized = true;

	UE_LOG(LogTemp, Log, TEXT("AchievementSubsystem initialized with %d achievements"), Achievements.Num());
}

void UAchievementSubsystem::Deinitialize()
{
	SaveAchievementData();
	Super::Deinitialize();
}

void UAchievementSubsystem::InitializeAchievements()
{
	// Load all achievement definitions
	TArray<FHorrorAchievementDefinition> Definitions = UAchievementDefinitionCatalog::GetAllAchievementDefinitions();

	for (const FHorrorAchievementDefinition& Def : Definitions)
	{
		FAchievementData Data;
		Data.ID = Def.ID;
		Data.Name = Def.Name;
		Data.Description = Def.Description;
		Data.Icon = Def.Icon;
		Data.Category = Def.Category;
		Data.bIsHidden = Def.bIsHidden;
		Data.bIsProgressive = Def.bIsProgressive;
		Data.MaxProgress = Def.MaxProgress;
		Data.Points = Def.Points;
		Data.bUnlocked = false;
		Data.CurrentProgress = 0.0f;
		Data.UnlockTime = FDateTime::MinValue();

		Achievements.Add(Def.ID, Data);

		if (Def.bIsProgressive)
		{
			AchievementProgressMap.Add(Def.ID, 0.0f);
		}
	}
}

void UAchievementSubsystem::UnlockAchievement(FName AchievementID)
{
	if (!bInitialized || !Achievements.Contains(AchievementID))
	{
		return;
	}

	if (UnlockedAchievements.Contains(AchievementID))
	{
		return; // Already unlocked
	}

	UnlockAchievementInternal(AchievementID, true);
}

void UAchievementSubsystem::UnlockAchievementInternal(FName AchievementID, bool bSyncPlatform)
{
	FAchievementData* Achievement = Achievements.Find(AchievementID);
	if (!Achievement)
	{
		return;
	}

	Achievement->bUnlocked = true;
	Achievement->UnlockTime = FDateTime::Now();
	Achievement->CurrentProgress = Achievement->MaxProgress;

	UnlockedAchievements.Add(AchievementID);

	// Sync with platform
	if (bSyncPlatform && bPlatformAvailable)
	{
		SyncSteamAchievement(AchievementID);
		SyncEpicAchievement(AchievementID);
	}

	// Broadcast event
	OnAchievementUnlocked.Broadcast(*Achievement);

	// Show notification
	UAchievementNotification::ShowAchievementNotification(GetWorld(), *Achievement);

	// Save
	SaveAchievementData();

	UE_LOG(LogTemp, Log, TEXT("Achievement unlocked: %s"), *Achievement->Name.ToString());
}

void UAchievementSubsystem::UpdateAchievementProgress(FName AchievementID, float Progress)
{
	if (!bInitialized || !Achievements.Contains(AchievementID))
	{
		return;
	}

	FAchievementData* Achievement = Achievements.Find(AchievementID);
	if (!Achievement || !Achievement->bIsProgressive || Achievement->bUnlocked)
	{
		return;
	}

	float OldProgress = Achievement->CurrentProgress;
	Achievement->CurrentProgress = FMath::Clamp(Progress, 0.0f, Achievement->MaxProgress);

	AchievementProgressMap.Add(AchievementID, Achievement->CurrentProgress);

	// Broadcast progress
	if (Achievement->CurrentProgress != OldProgress)
	{
		float ProgressPercent = Achievement->CurrentProgress / Achievement->MaxProgress;
		OnAchievementProgress.Broadcast(AchievementID, ProgressPercent);
	}

	// Check if completed
	if (Achievement->CurrentProgress >= Achievement->MaxProgress)
	{
		UnlockAchievementInternal(AchievementID, true);
	}
}

bool UAchievementSubsystem::IsAchievementUnlocked(FName AchievementID) const
{
	return UnlockedAchievements.Contains(AchievementID);
}

float UAchievementSubsystem::GetAchievementProgress(FName AchievementID) const
{
	const FAchievementData* Achievement = Achievements.Find(AchievementID);
	if (Achievement && Achievement->bIsProgressive)
	{
		return Achievement->CurrentProgress / Achievement->MaxProgress;
	}
	return 0.0f;
}

TArray<FAchievementData> UAchievementSubsystem::GetAllAchievements() const
{
	TArray<FAchievementData> Result;
	Achievements.GenerateValueArray(Result);
	return Result;
}

TArray<FAchievementData> UAchievementSubsystem::GetUnlockedAchievements() const
{
	TArray<FAchievementData> Result;
	for (const auto& Pair : Achievements)
	{
		if (Pair.Value.bUnlocked)
		{
			Result.Add(Pair.Value);
		}
	}
	return Result;
}

TArray<FAchievementData> UAchievementSubsystem::GetLockedAchievements() const
{
	TArray<FAchievementData> Result;
	for (const auto& Pair : Achievements)
	{
		if (!Pair.Value.bUnlocked)
		{
			Result.Add(Pair.Value);
		}
	}
	return Result;
}

int32 UAchievementSubsystem::GetUnlockedCount() const
{
	return UnlockedAchievements.Num();
}

float UAchievementSubsystem::GetCompletionPercentage() const
{
	if (Achievements.Num() == 0)
	{
		return 0.0f;
	}
	return (float)UnlockedAchievements.Num() / (float)Achievements.Num() * 100.0f;
}

void UAchievementSubsystem::SyncWithPlatform()
{
	if (!bPlatformAvailable)
	{
		return;
	}

	// Sync all unlocked achievements
	for (FName AchievementID : UnlockedAchievements)
	{
		SyncSteamAchievement(AchievementID);
		SyncEpicAchievement(AchievementID);
	}
}

bool UAchievementSubsystem::IsPlatformAchievementSystemAvailable() const
{
	return bPlatformAvailable;
}

void UAchievementSubsystem::SyncSteamAchievement(FName AchievementID)
{
	// TODO: Integrate with Steam API
	// ISteamUserStats* SteamUserStats = SteamUserStats();
	// if (SteamUserStats)
	// {
	//     SteamUserStats->SetAchievement(TCHAR_TO_ANSI(*AchievementID.ToString()));
	//     SteamUserStats->StoreStats();
	// }
}

void UAchievementSubsystem::SyncEpicAchievement(FName AchievementID)
{
	// TODO: Integrate with Epic Online Services
	// EOS_Achievements_UnlockAchievementsOptions Options = {};
	// Options.ApiVersion = EOS_ACHIEVEMENTS_UNLOCKACHIEVEMENTS_API_LATEST;
	// Options.UserId = LocalUserId;
	// Options.AchievementIds = &AchievementId;
	// Options.AchievementsCount = 1;
}

void UAchievementSubsystem::SaveAchievementData()
{
	// TODO: Implement save to file or save game system
	UE_LOG(LogTemp, Log, TEXT("Saving achievement data..."));
}

void UAchievementSubsystem::LoadAchievementData()
{
	// TODO: Implement load from file or save game system
	UE_LOG(LogTemp, Log, TEXT("Loading achievement data..."));
}
