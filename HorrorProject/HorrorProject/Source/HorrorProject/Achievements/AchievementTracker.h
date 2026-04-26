#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AchievementTracker.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class HORRORPROJECT_API UAchievementTracker : public UActorComponent
{
	GENERATED_BODY()

public:
	UAchievementTracker();

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// Tracking Methods
	UFUNCTION(BlueprintCallable, Category = "Achievement Tracker")
	void TrackRoomDiscovered(FName RoomID);

	UFUNCTION(BlueprintCallable, Category = "Achievement Tracker")
	void TrackEvidenceCollected(FName EvidenceID);

	UFUNCTION(BlueprintCallable, Category = "Achievement Tracker")
	void TrackDocumentRead(FName DocumentID);

	UFUNCTION(BlueprintCallable, Category = "Achievement Tracker")
	void TrackGhostEncounter();

	UFUNCTION(BlueprintCallable, Category = "Achievement Tracker")
	void TrackPlayerDeath();

	UFUNCTION(BlueprintCallable, Category = "Achievement Tracker")
	void TrackChapterComplete(int32 ChapterNumber);

	UFUNCTION(BlueprintCallable, Category = "Achievement Tracker")
	void TrackGameComplete(float CompletionTime);

	UFUNCTION(BlueprintCallable, Category = "Achievement Tracker")
	void TrackSecretFound(FName SecretID);

	UFUNCTION(BlueprintCallable, Category = "Achievement Tracker")
	void TrackFlashlightUsage(bool bUsed);

	UFUNCTION(BlueprintCallable, Category = "Achievement Tracker")
	void TrackSanityLevel(float SanityPercent);

	UFUNCTION(BlueprintCallable, Category = "Achievement Tracker")
	void CheckAllAchievements();

private:
	UPROPERTY()
	class UAchievementSubsystem* AchievementSubsystem;

	// Tracking Data
	TSet<FName> DiscoveredRooms;
	TSet<FName> CollectedEvidence;
	TSet<FName> ReadDocuments;
	TSet<FName> FoundSecrets;

	int32 GhostEncounterCount;
	int32 DeathCount;
	bool bFlashlightUsedThisChapter;
	float MinSanityLevel;
	float GameStartTime;

	void CheckExplorationAchievements();
	void CheckCollectionAchievements();
	void CheckSurvivalAchievements();
	void CheckSpeedrunAchievements(float CompletionTime);
	void CheckChallengeAchievements();
	void CheckSecretAchievements();
};
