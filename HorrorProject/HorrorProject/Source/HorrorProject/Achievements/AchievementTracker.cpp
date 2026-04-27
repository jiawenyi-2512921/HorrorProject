#include "AchievementTracker.h"
#include "AchievementSubsystem.h"
#include "Engine/GameInstance.h"
#include "Kismet/GameplayStatics.h"

namespace HorrorAchievementTracker
{
	constexpr float PerfectionistCompletionPercent = 99.0f;
	constexpr float NervesOfSteelMinimumSanityPercent = 50.0f;
	constexpr float SecondsPerHour = 3600.0f;
}

UAchievementTracker::UAchievementTracker()
{
	PrimaryComponentTick.bCanEverTick = true;
	GhostEncounterCount = 0;
	DeathCount = 0;
	bFlashlightUsedThisChapter = false;
	MinSanityLevel = 100.0f;
	GameStartTime = 0.0f;
}

void UAchievementTracker::BeginPlay()
{
	Super::BeginPlay();

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	if (UGameInstance* GameInstance = World->GetGameInstance())
	{
		AchievementSubsystem = GameInstance->GetSubsystem<UAchievementSubsystem>();
	}
	GameStartTime = UGameplayStatics::GetTimeSeconds(World);
}

void UAchievementTracker::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UAchievementTracker::TrackRoomDiscovered(FName RoomID)
{
	if (DiscoveredRooms.Contains(RoomID))
	{
		return;
	}

	DiscoveredRooms.Add(RoomID);

	if (!AchievementSubsystem)
	{
		return;
	}

	// First room
	if (DiscoveredRooms.Num() == 1)
	{
		AchievementSubsystem->UnlockAchievement(FName("ACH_FirstSteps"));
	}

	// Explorer - 10 rooms
	AchievementSubsystem->UpdateAchievementProgress(FName("ACH_Explorer"), DiscoveredRooms.Num());

	// Master Explorer - all rooms (30)
	AchievementSubsystem->UpdateAchievementProgress(FName("ACH_MasterExplorer"), DiscoveredRooms.Num());

	CheckExplorationAchievements();
}

void UAchievementTracker::TrackEvidenceCollected(FName EvidenceID)
{
	if (CollectedEvidence.Contains(EvidenceID))
	{
		return;
	}

	CollectedEvidence.Add(EvidenceID);

	if (!AchievementSubsystem)
	{
		return;
	}

	// First clue
	if (CollectedEvidence.Num() == 1)
	{
		AchievementSubsystem->UnlockAchievement(FName("ACH_FirstClue"));
	}

	// Detective - 10 evidence
	AchievementSubsystem->UpdateAchievementProgress(FName("ACH_Detective"), CollectedEvidence.Num());

	// Master Detective - all evidence (25)
	AchievementSubsystem->UpdateAchievementProgress(FName("ACH_MasterDetective"), CollectedEvidence.Num());

	CheckCollectionAchievements();
}

void UAchievementTracker::TrackDocumentRead(FName DocumentID)
{
	if (ReadDocuments.Contains(DocumentID))
	{
		return;
	}

	ReadDocuments.Add(DocumentID);

	if (!AchievementSubsystem)
	{
		return;
	}

	// Archivist - all documents (15)
	AchievementSubsystem->UpdateAchievementProgress(FName("ACH_Archivist"), ReadDocuments.Num());
}

void UAchievementTracker::TrackGhostEncounter()
{
	GhostEncounterCount++;

	if (!AchievementSubsystem)
	{
		return;
	}

	// First encounter
	if (GhostEncounterCount == 1)
	{
		AchievementSubsystem->UnlockAchievement(FName("ACH_FirstEncounter"));
	}

	// Survivor - 5 encounters
	AchievementSubsystem->UpdateAchievementProgress(FName("ACH_Survivor"), GhostEncounterCount);

	CheckSurvivalAchievements();
}

void UAchievementTracker::TrackPlayerDeath()
{
	DeathCount++;
	CheckSurvivalAchievements();
}

void UAchievementTracker::TrackChapterComplete(int32 ChapterNumber)
{
	if (!AchievementSubsystem)
	{
		return;
	}

	switch (ChapterNumber)
	{
	case 1:
		AchievementSubsystem->UnlockAchievement(FName("ACH_TheBeginning"));
		break;
	case 2:
		AchievementSubsystem->UnlockAchievement(FName("ACH_DeeperIntoMadness"));
		break;
	case 3:
		AchievementSubsystem->UnlockAchievement(FName("ACH_TheTruth"));
		break;
	}

	// Check challenge achievements at chapter end
	if (!bFlashlightUsedThisChapter)
	{
		AchievementSubsystem->UnlockAchievement(FName("ACH_NoFlashlight"));
	}

	// Reset chapter tracking
	bFlashlightUsedThisChapter = false;
}

void UAchievementTracker::TrackGameComplete(float CompletionTime)
{
	if (!AchievementSubsystem)
	{
		return;
	}

	// Game complete
	AchievementSubsystem->UnlockAchievement(FName("ACH_Escape"));

	// Check speedrun achievements
	CheckSpeedrunAchievements(CompletionTime);

	// Check survival achievements
	CheckSurvivalAchievements();

	// Check challenge achievements
	CheckChallengeAchievements();

	// Check perfectionist
	if (AchievementSubsystem->GetCompletionPercentage() >= HorrorAchievementTracker::PerfectionistCompletionPercent)
	{
		AchievementSubsystem->UnlockAchievement(FName("ACH_Perfectionist"));
	}
}

void UAchievementTracker::TrackSecretFound(FName SecretID)
{
	if (FoundSecrets.Contains(SecretID))
	{
		return;
	}

	FoundSecrets.Add(SecretID);

	if (!AchievementSubsystem)
	{
		return;
	}

	// Secret passage
	if (SecretID == FName("SecretPassage"))
	{
		AchievementSubsystem->UnlockAchievement(FName("ACH_SecretPassage"));
	}

	CheckSecretAchievements();
}

void UAchievementTracker::TrackFlashlightUsage(bool bUsed)
{
	if (bUsed)
	{
		bFlashlightUsedThisChapter = true;
	}
}

void UAchievementTracker::TrackSanityLevel(float SanityPercent)
{
	MinSanityLevel = FMath::Min(MinSanityLevel, SanityPercent);
}

void UAchievementTracker::CheckAllAchievements()
{
	CheckExplorationAchievements();
	CheckCollectionAchievements();
	CheckSurvivalAchievements();
	CheckChallengeAchievements();
	CheckSecretAchievements();
}

void UAchievementTracker::CheckExplorationAchievements()
{
	// Implemented via progress tracking in TrackRoomDiscovered
}

void UAchievementTracker::CheckCollectionAchievements()
{
	// Implemented via progress tracking in TrackEvidenceCollected and TrackDocumentRead
}

void UAchievementTracker::CheckSurvivalAchievements()
{
	if (!AchievementSubsystem)
	{
		return;
	}

	// Fearless - no deaths
	if (DeathCount == 0 && AchievementSubsystem->IsAchievementUnlocked(FName("ACH_Escape")))
	{
		AchievementSubsystem->UnlockAchievement(FName("ACH_Fearless"));
	}

	// Nerves of Steel - sanity above 50%
	if (MinSanityLevel >= HorrorAchievementTracker::NervesOfSteelMinimumSanityPercent
		&& AchievementSubsystem->IsAchievementUnlocked(FName("ACH_Escape")))
	{
		AchievementSubsystem->UnlockAchievement(FName("ACH_NervesOfSteel"));
	}
}

void UAchievementTracker::CheckSpeedrunAchievements(float CompletionTime)
{
	if (!AchievementSubsystem)
	{
		return;
	}

	const float CompletionHours = CompletionTime / HorrorAchievementTracker::SecondsPerHour;

	// Speed Runner - under 2 hours
	if (CompletionHours < 2.0f)
	{
		AchievementSubsystem->UnlockAchievement(FName("ACH_SpeedRunner"));
	}

	// Lightning Fast - under 1 hour
	if (CompletionHours < 1.0f)
	{
		AchievementSubsystem->UnlockAchievement(FName("ACH_LightningFast"));
	}
}

void UAchievementTracker::CheckChallengeAchievements()
{
	if (!AchievementSubsystem)
	{
		return;
	}

	// Minimalist - check if only essential items were used
	// This would need additional tracking logic
}

void UAchievementTracker::CheckSecretAchievements()
{
	if (!AchievementSubsystem)
	{
		return;
	}

	// The Watcher - specific secret condition
	if (FoundSecrets.Contains(FName("TheWatcher")))
	{
		AchievementSubsystem->UnlockAchievement(FName("ACH_TheWatcher"));
	}

	// Broken Mirror - specific secret condition
	if (FoundSecrets.Contains(FName("BrokenMirror")))
	{
		AchievementSubsystem->UnlockAchievement(FName("ACH_BrokenMirror"));
	}

	// Midnight Visitor - specific secret condition
	if (FoundSecrets.Contains(FName("MidnightVisitor")))
	{
		AchievementSubsystem->UnlockAchievement(FName("ACH_MidnightVisitor"));
	}
}
