// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Game/HorrorAnomalyDirector.h"
#include "Game/HorrorFoundFootageContract.h"
#include "HorrorGameModeBase.generated.h"

class ADeepWaterStationRouteKit;
class AHorrorEncounterDirector;
class AHorrorPlayerCharacter;
class UHorrorSaveSubsystem;

/**
 * Defines Horror Game Mode Base behavior for the Game module.
 */
UCLASS()
class HORRORPROJECT_API AHorrorGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public:
	AHorrorGameModeBase();

	UFUNCTION(BlueprintCallable, Category="Horror|Objectives")
	bool TryAcquireBodycam(FName SourceId, bool bEnableBodycam);

	UFUNCTION(BlueprintCallable, Category="Horror|Objectives")
	bool TryCollectFirstNote(FName SourceId);

	UFUNCTION(BlueprintCallable, Category="Horror|Objectives")
	bool BeginFirstAnomalyCandidate(FName SourceId);

	UFUNCTION(BlueprintCallable, Category="Horror|Objectives")
	bool TryRecordFirstAnomaly(bool bIsRecording);

	UFUNCTION(BlueprintCallable, Category="Horror|Objectives")
	bool TryReviewArchive(FName SourceId);

	UFUNCTION(BlueprintPure, Category="Horror|Objectives")
	bool HasBodycamAcquired() const;

	UFUNCTION(BlueprintPure, Category="Horror|Objectives")
	bool HasCollectedFirstNote() const;

	UFUNCTION(BlueprintPure, Category="Horror|Objectives")
	bool HasRecordedFirstAnomaly() const;

	UFUNCTION(BlueprintPure, Category="Horror|Objectives")
	bool HasPendingFirstAnomalyCandidate() const;

	UFUNCTION(BlueprintPure, Category="Horror|Objectives")
	FName GetPendingFirstAnomalySourceId() const;

	UFUNCTION(BlueprintPure, Category="Horror|Objectives")
	bool HasReviewedArchive() const;

	UFUNCTION(BlueprintPure, Category="Horror|Objectives")
	bool HasFoundFootageRecordedEvent(FGameplayTag EventTag) const;

	UFUNCTION(BlueprintPure, Category="Horror|Objectives")
	bool HasFoundFootageCompletedState(FGameplayTag StateTag) const;

	UFUNCTION(BlueprintPure, Category="Horror|Objectives")
	bool CanCollectFirstNote() const;

	UFUNCTION(BlueprintPure, Category="Horror|Objectives")
	bool CanBeginFirstAnomalyCandidate() const;

	UFUNCTION(BlueprintPure, Category="Horror|Objectives")
	bool CanRecordFirstAnomaly(bool bIsRecording) const;

	UFUNCTION(BlueprintPure, Category="Horror|Objectives")
	bool CanReviewArchive() const;

	UFUNCTION(BlueprintPure, Category="Horror|Objectives")
	bool IsExitUnlocked() const;

	UFUNCTION(BlueprintCallable, Category="Horror|Objectives")
	bool TryCompleteDay1(FName SourceId);

	UFUNCTION(BlueprintPure, Category="Horror|Objectives")
	bool IsDay1Complete() const { return bDay1Complete; }

	void ImportDay1CompleteState(bool bInDay1Complete);

	UFUNCTION(BlueprintPure, Category="Horror|Objectives")
	FHorrorFoundFootageProgressSnapshot BuildFoundFootageProgressSnapshot() const;

	const FHorrorFoundFootageContract& GetFoundFootageContract() const;
	FHorrorFoundFootageSaveState ExportFoundFootageSaveState() const;
	void ImportFoundFootageSaveState(const FHorrorFoundFootageSaveState& SaveState);
	void ImportPendingFirstAnomalyCandidate(FName SourceId);

	UFUNCTION(BlueprintCallable, Category="Horror|Objectives")
	void SyncFoundFootageRuntimeStateToPlayer();

	UFUNCTION(BlueprintPure, Category="Horror|Encounter")
	AHorrorEncounterDirector* GetRuntimeEncounterDirector() const { return RuntimeEncounterDirector.Get(); }

	UFUNCTION(BlueprintPure, Category="Horror|Route")
	ADeepWaterStationRouteKit* GetRuntimeRouteKit() const { return RuntimeRouteKit.Get(); }

	UFUNCTION(BlueprintCallable, Category="Horror|Save")
	bool SaveDay1Checkpoint(FName CheckpointId);

	UFUNCTION(BlueprintCallable, Category="Horror|Save")
	bool LoadDay1Checkpoint();

	UFUNCTION(BlueprintCallable, Category="Horror|Player Failure")
	bool RequestPlayerDeath(FName DeathCause);

	UFUNCTION(BlueprintCallable, Category="Horror|Player Failure")
	bool RequestPlayerFailure(FName FailureCause, FText FeedbackText);

	UFUNCTION(BlueprintPure, Category="Horror|Player Failure")
	FName GetLastPlayerFailureCause() const { return LastPlayerFailureCause; }

	UFUNCTION(BlueprintPure, Category="Horror|Player Failure")
	bool DidLastPlayerFailureRecoverFromCheckpoint() const { return bLastPlayerFailureRecoveredFromCheckpoint; }

	UFUNCTION(BlueprintPure, Category="Horror|Player Failure")
	int32 GetPlayerFailureCount() const { return PlayerFailureCount; }

	UFUNCTION(BlueprintPure, Category="Horror|Objectives")
	bool IsLeadPlayerRecording() const;

	UFUNCTION(BlueprintPure, Category="Horror|Bootstrap")
	bool ShouldAutoGrantBodycamOnPlayerBeginPlay() const { return bAutoGrantBodycamOnPlayerBeginPlay; }

	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
	virtual void BeginPlay() override;
	virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;
	virtual APawn* SpawnDefaultPawnAtTransform_Implementation(AController* NewPlayer, const FTransform& SpawnTransform) override;

private:
	static constexpr float DefaultRuntimeRouteKitHeight = 80.0f;

	bool RecordFoundFootageEvent(FGameplayTag EventTag, FName SourceId);
	AHorrorPlayerCharacter* ResolveLeadPlayerCharacter() const;
	ADeepWaterStationRouteKit* EnsureRouteKit();
	FTransform ResolveRuntimeRouteKitSpawnTransform() const;
	AHorrorEncounterDirector* EnsureEncounterDirector();
	void RegisterDefaultObjectiveMetadata();
	void HandleObjectiveStateChange(FGameplayTag StateTag);
	void HandleEncounterObjectiveStateChange(FGameplayTag StateTag);
	void HandleAutosaveObjectiveStateChange(FGameplayTag StateTag);
	void RestoreEncounterStateFromFoundFootageCheckpoint();
	void TryAutosaveOnMilestone(FName CheckpointId);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Bootstrap", meta=(AllowPrivateAccess="true"))
	bool bAutoGrantBodycamOnPlayerBeginPlay = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Route", meta=(AllowPrivateAccess="true"))
	bool bAutoSpawnRouteKitOnBeginPlay = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Route", meta=(AllowPrivateAccess="true"))
	FTransform RuntimeRouteKitTransform = FTransform(FRotator::ZeroRotator, FVector(0.0f, 0.0f, DefaultRuntimeRouteKitHeight));

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Route", meta=(AllowPrivateAccess="true"))
	bool bAnchorRuntimeRouteKitToPlayerStart = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Spawn", meta=(AllowPrivateAccess="true"))
	FName PreferredPlayerStartTag = TEXT("DeepWaterStationBase");

	UPROPERTY(Transient)
	TObjectPtr<ADeepWaterStationRouteKit> RuntimeRouteKit;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Encounter", meta=(AllowPrivateAccess="true"))
	bool bAutoSpawnEncounterDirectorOnBeginPlay = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Encounter", meta=(AllowPrivateAccess="true"))
	FTransform RuntimeEncounterDirectorTransform = FTransform::Identity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Encounter", meta=(AllowPrivateAccess="true"))
	TSubclassOf<AHorrorEncounterDirector> RuntimeEncounterDirectorClass;

	UPROPERTY(Transient)
	TObjectPtr<AHorrorEncounterDirector> RuntimeEncounterDirector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Save", meta=(AllowPrivateAccess="true"))
	bool bAutosaveOnObjectiveMilestone = true;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Horror|Player Failure", meta=(AllowPrivateAccess="true"))
	FName LastPlayerFailureCause;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Horror|Player Failure", meta=(AllowPrivateAccess="true"))
	bool bLastPlayerFailureRecoveredFromCheckpoint = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Horror|Player Failure", meta=(AllowPrivateAccess="true"))
	int32 PlayerFailureCount = 0;

	UPROPERTY(Transient)
	bool bDay1Complete = false;

	FHorrorFoundFootageContract FoundFootageContract;
	FHorrorAnomalyDirector AnomalyDirector;
};
