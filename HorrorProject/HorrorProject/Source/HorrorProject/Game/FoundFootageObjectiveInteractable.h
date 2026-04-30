// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameFramework/Actor.h"
#include "Interaction/InteractableInterface.h"
#include "Player/Components/InventoryComponent.h"
#include "Player/Components/NoteRecorderComponent.h"
#include "FoundFootageObjectiveInteractable.generated.h"

class ADeepWaterStationRouteKit;
class AHorrorGameModeBase;
class AHorrorPlayerCharacter;
class UBoxComponent;
class UStaticMeshComponent;

UENUM(BlueprintType)
enum class EFoundFootageInteractableObjective : uint8
{
	Bodycam UMETA(DisplayName="随身摄像机"),
	FirstNote UMETA(DisplayName="第一份笔记"),
	FirstAnomalyCandidate UMETA(DisplayName="第一个异常候选点"),
	FirstAnomalyRecord UMETA(DisplayName="记录第一个异常"),
	ArchiveReview UMETA(DisplayName="档案审查"),
	ExitRouteGate UMETA(DisplayName="出口闸门")
};

/**
 * Defines Found Footage Objective Interactable behavior for the Game module.
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup=(Horror))
class HORRORPROJECT_API AFoundFootageObjectiveInteractable : public AActor, public IInteractableInterface
{
	GENERATED_BODY()

public:
	AFoundFootageObjectiveInteractable();

	virtual bool CanInteract_Implementation(AActor* InstigatorActor, const FHitResult& Hit) const override;
	virtual bool Interact_Implementation(AActor* InstigatorActor, const FHitResult& Hit) override;
	virtual void OnConstruction(const FTransform& Transform) override;

	bool CanCompleteObjective(AHorrorGameModeBase* GameMode) const;
	bool CanCompleteObjectiveForInstigator(AHorrorGameModeBase* GameMode, AActor* InstigatorActor) const;
	bool TryCompleteObjective(AHorrorGameModeBase* GameMode) const;
	bool TryCompleteObjectiveForInstigator(AHorrorGameModeBase* GameMode, AActor* InstigatorActor) const;
	FText GetInteractionPromptText(AActor* InstigatorActor) const;
	void RefreshVisualDefaults();
	UStaticMeshComponent* GetVisualMeshComponentForTests() const { return VisualMesh.Get(); }
	void RecordInstigatorProgress(AActor* InstigatorActor, FName ProgressId) const;
	void RegisterObjectiveEventMetadata(FName ProgressId) const;
	void RegisterObjectiveEventMetadataForCompletedObjective(AHorrorGameModeBase* GameMode, FName ProgressId) const;
	void UnregisterObjectiveEventMetadataForCompletedObjective(AHorrorGameModeBase* GameMode, FName ProgressId) const;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Objectives")
	EFoundFootageInteractableObjective Objective = EFoundFootageInteractableObjective::Bodycam;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Objectives")
	FName SourceId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Objectives")
	bool bEnableBodycamOnInteract = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Objectives")
	bool bIsRecordingForFirstAnomalyRecord = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Objectives")
	FHorrorEvidenceMetadata EvidenceMetadata;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Objectives")
	FHorrorNoteMetadata NoteMetadata;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Objectives")
	FName TrailerBeatId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Objectives")
	FText ObjectiveHint;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Objectives")
	FText DebugLabel;

protected:
	AHorrorGameModeBase* ResolveObjectiveGameMode() const;
	FName ResolveSourceId() const;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Horror|Objectives")
	TObjectPtr<UBoxComponent> InteractionBounds;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Horror|Objectives")
	TObjectPtr<UStaticMeshComponent> VisualMesh;

private:
	ADeepWaterStationRouteKit* ResolveOwningRouteKit() const;
	bool CanCompleteArchiveReview(AHorrorGameModeBase& GameMode, AActor* InstigatorActor) const;
	bool CanCompleteExitRouteGate(AHorrorGameModeBase& GameMode, AActor* InstigatorActor) const;
	bool TryCompleteBodycamObjective(AHorrorGameModeBase& GameMode) const;
	bool TryCompleteFirstNoteObjective(AHorrorGameModeBase& GameMode) const;
	bool TryCompleteFirstAnomalyCandidateObjective(AHorrorGameModeBase& GameMode) const;
	bool TryCompleteFirstAnomalyRecordObjective(AHorrorGameModeBase& GameMode) const;
	bool TryCompleteArchiveReviewObjective(AHorrorGameModeBase& GameMode) const;
	bool TryCompleteExitRouteGateObjective(AHorrorGameModeBase& GameMode) const;
	bool CanRecordFirstAnomalyFromCandidate(AHorrorGameModeBase& GameMode) const;
	FText BuildBlockedObjectivePrompt(AHorrorGameModeBase* GameMode, AActor* InstigatorActor) const;
	FText BuildObjectiveActionPrompt(AHorrorGameModeBase* GameMode) const;
	void RecordEvidenceProgress(AHorrorPlayerCharacter& PlayerCharacter, FName ProgressId, bool bMarkCollected) const;
	void RecordNoteProgress(AHorrorPlayerCharacter& PlayerCharacter, FName ProgressId) const;
	void RecordObjectiveHintNoteProgress(AHorrorPlayerCharacter& PlayerCharacter, FName ProgressId) const;
	void RecordArchiveSummaryProgress(AHorrorPlayerCharacter& PlayerCharacter, FName ArchiveEvidenceId) const;
	FGameplayTag ResolveCompletedObjectiveEventTag() const;
};
