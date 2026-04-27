#include "AchievementSubsystem.h"
#include "AchievementSaveGame.h"
#include "AchievementNotification.h"
#include "Kismet/GameplayStatics.h"

namespace HorrorAchievementSave
{
	const FString SlotName(TEXT("HorrorProject_Achievements"));
	constexpr int32 UserIndex = 0;
	constexpr int32 SaveVersion = 1;
}

void UAchievementSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	bInitialized = false;
	bPlatformAvailable = false;

	InitializeAchievements();
	LoadAchievementData();

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
		const float ProgressPercent = Achievement->MaxProgress > 0.0f
			? Achievement->CurrentProgress / Achievement->MaxProgress
			: 0.0f;
		OnAchievementProgress.Broadcast(AchievementID, ProgressPercent);
		SaveAchievementData();
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
	if (Achievement && Achievement->bIsProgressive && Achievement->MaxProgress > 0.0f)
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
	UE_LOG(LogTemp, Verbose, TEXT("Steam achievement sync skipped for %s: Steam integration is not enabled."), *AchievementID.ToString());
}

void UAchievementSubsystem::SyncEpicAchievement(FName AchievementID)
{
	UE_LOG(LogTemp, Verbose, TEXT("Epic achievement sync skipped for %s: EOS integration is not enabled."), *AchievementID.ToString());
}

void UAchievementSubsystem::SaveAchievementData()
{
	UHorrorAchievementSaveGame* SaveGame = Cast<UHorrorAchievementSaveGame>(
		UGameplayStatics::CreateSaveGameObject(UHorrorAchievementSaveGame::StaticClass()));
	if (!SaveGame)
	{
		UE_LOG(LogTemp, Warning, TEXT("Unable to create achievement save game object"));
		return;
	}

	SaveGame->SaveVersion = HorrorAchievementSave::SaveVersion;
	SaveGame->SavedAtUtc = FDateTime::UtcNow();
	SaveGame->Records.Reserve(Achievements.Num());

	for (const TPair<FName, FAchievementData>& Pair : Achievements)
	{
		const FAchievementData& Achievement = Pair.Value;

		FAchievementSaveRecord Record;
		Record.AchievementID = Pair.Key;
		Record.bUnlocked = Achievement.bUnlocked || UnlockedAchievements.Contains(Pair.Key);
		Record.Progress = Achievement.CurrentProgress;
		Record.UnlockTime = Achievement.UnlockTime;
		SaveGame->Records.Add(Record);
	}

	if (!UGameplayStatics::SaveGameToSlot(SaveGame, HorrorAchievementSave::SlotName, HorrorAchievementSave::UserIndex))
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to save achievement data to slot '%s'"), *HorrorAchievementSave::SlotName);
	}
}

void UAchievementSubsystem::LoadAchievementData()
{
	if (!UGameplayStatics::DoesSaveGameExist(HorrorAchievementSave::SlotName, HorrorAchievementSave::UserIndex))
	{
		UE_LOG(LogTemp, Verbose, TEXT("No achievement save exists in slot '%s'"), *HorrorAchievementSave::SlotName);
		return;
	}

	UHorrorAchievementSaveGame* SaveGame = Cast<UHorrorAchievementSaveGame>(
		UGameplayStatics::LoadGameFromSlot(HorrorAchievementSave::SlotName, HorrorAchievementSave::UserIndex));
	if (!SaveGame)
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to load achievement data from slot '%s'"), *HorrorAchievementSave::SlotName);
		return;
	}

	if (SaveGame->SaveVersion != HorrorAchievementSave::SaveVersion)
	{
		UE_LOG(LogTemp, Warning, TEXT("Ignoring unsupported achievement save version %d"), SaveGame->SaveVersion);
		return;
	}

	ResetAchievementRuntimeState();

	for (const FAchievementSaveRecord& Record : SaveGame->Records)
	{
		ApplyAchievementSaveRecord(Record);
	}

	UE_LOG(LogTemp, Log, TEXT("Loaded %d achievement save records"), SaveGame->Records.Num());
}

void UAchievementSubsystem::ResetAchievementRuntimeState()
{
	UnlockedAchievements.Reset();

	for (TPair<FName, FAchievementData>& Pair : Achievements)
	{
		Pair.Value.bUnlocked = false;
		Pair.Value.CurrentProgress = 0.0f;
		Pair.Value.UnlockTime = FDateTime::MinValue();
		if (Pair.Value.bIsProgressive)
		{
			AchievementProgressMap.Add(Pair.Key, 0.0f);
		}
	}
}

void UAchievementSubsystem::ApplyAchievementSaveRecord(const FAchievementSaveRecord& Record)
{
	if (Record.AchievementID.IsNone())
	{
		return;
	}

	FAchievementData* Achievement = Achievements.Find(Record.AchievementID);
	if (!Achievement)
	{
		UE_LOG(LogTemp, Verbose, TEXT("Skipping stale achievement save record '%s'"), *Record.AchievementID.ToString());
		return;
	}

	const float MaxProgress = FMath::Max(Achievement->MaxProgress, 0.0f);
	Achievement->bUnlocked = Record.bUnlocked;
	Achievement->CurrentProgress = Record.bUnlocked ? MaxProgress : FMath::Clamp(Record.Progress, 0.0f, MaxProgress);
	Achievement->UnlockTime = Record.bUnlocked ? Record.UnlockTime : FDateTime::MinValue();

	if (Achievement->bIsProgressive)
	{
		AchievementProgressMap.Add(Record.AchievementID, Achievement->CurrentProgress);
	}

	if (Record.bUnlocked)
	{
		UnlockedAchievements.Add(Record.AchievementID);
	}
}
