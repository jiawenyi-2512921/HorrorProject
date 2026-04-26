// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Game/HorrorAnomalyDirector.h"
#include "Game/HorrorFoundFootageContract.h"
#include "HorrorGameModeBase.generated.h"

class ADeepWaterStationRouteKit;
class AHorrorPlayerCharacter;
class UHorrorSaveSubsystem;

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

	UFUNCTION(BlueprintPure, Category="Horror|Objectives")
	FHorrorFoundFootageProgressSnapshot BuildFoundFootageProgressSnapshot() const;

	const FHorrorFoundFootageContract& GetFoundFootageContract() const;
	FHorrorFoundFootageSaveState ExportFoundFootageSaveState() const;
	void ImportFoundFootageSaveState(const FHorrorFoundFootageSaveState& SaveState);
	void ImportPendingFirstAnomalyCandidate(FName SourceId);

	UFUNCTION(BlueprintCallable, Category="Horror|Objectives")
	void SyncFoundFootageRuntimeStateToPlayer();

	UFUNCTION(BlueprintCallable, Category="Horror|Save")
	bool SaveDay1Checkpoint(FName CheckpointId);

	UFUNCTION(BlueprintCallable, Category="Horror|Save")
	bool LoadDay1Checkpoint();

	UFUNCTION(BlueprintPure, Category="Horror|Objectives")
	bool IsLeadPlayerRecording() const;

	UFUNCTION(BlueprintPure, Category="Horror|Bootstrap")
	bool ShouldAutoGrantBodycamOnPlayerBeginPlay() const { return bAutoGrantBodycamOnPlayerBeginPlay; }

	virtual void BeginPlay() override;

private:
	bool RecordFoundFootageEvent(FGameplayTag EventTag, FName SourceId);
	AHorrorPlayerCharacter* ResolveLeadPlayerCharacter() const;
	ADeepWaterStationRouteKit* EnsureRouteKit();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Bootstrap", meta=(AllowPrivateAccess="true"))
	bool bAutoGrantBodycamOnPlayerBeginPlay = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Route", meta=(AllowPrivateAccess="true"))
	bool bAutoSpawnRouteKitOnBeginPlay = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Route", meta=(AllowPrivateAccess="true"))
	FTransform RuntimeRouteKitTransform = FTransform(FRotator::ZeroRotator, FVector(0.0f, 0.0f, 80.0f));

	UPROPERTY(Transient)
	TObjectPtr<ADeepWaterStationRouteKit> RuntimeRouteKit;

	FHorrorFoundFootageContract FoundFootageContract;
	FHorrorAnomalyDirector AnomalyDirector;
};
