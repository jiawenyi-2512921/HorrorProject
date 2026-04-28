// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "Game/HorrorEncounterDirector.h"
#include "GameplayTagContainer.h"
#include "Player/Components/InventoryComponent.h"
#include "Player/Components/NoteRecorderComponent.h"
#include "HorrorSaveGame.generated.h"

/**
 * Stores persistent Horror Save Game data for save and restore workflows.
 */
UCLASS(BlueprintType)
class HORRORPROJECT_API UHorrorSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UHorrorSaveGame();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, SaveGame, Category="Horror|Save")
	int32 SaveVersion = 1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, SaveGame, Category="Horror|Save")
	FName CheckpointId;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, SaveGame, Category="Horror|Save")
	TArray<FGameplayTag> RecordedObjectiveEvents;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, SaveGame, Category="Horror|Save")
	TArray<FGameplayTag> CompletedObjectiveStates;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, SaveGame, Category="Horror|Save")
	FName PendingFirstAnomalySourceId;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, SaveGame, Category="Horror|Save")
	TArray<FName> CollectedEvidenceIds;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, SaveGame, Category="Horror|Save")
	TArray<FHorrorEvidenceMetadata> CollectedEvidenceMetadata;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, SaveGame, Category="Horror|Save")
	TArray<FName> RecordedNoteIds;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, SaveGame, Category="Horror|Save")
	TArray<FHorrorNoteMetadata> RecordedNoteMetadata;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, SaveGame, Category="Horror|Save")
	TMap<FName, bool> InteractableStateFlags;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, SaveGame, Category="Horror|Save")
	bool bHasEncounterState = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, SaveGame, Category="Horror|Save")
	EHorrorEncounterPhase EncounterPhase = EHorrorEncounterPhase::Dormant;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, SaveGame, Category="Horror|Save")
	FName EncounterId;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, SaveGame, Category="Horror|Save")
	bool bDay1Complete = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, SaveGame, Category="Horror|Save")
	FTransform PlayerTransform;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, SaveGame, Category="Horror|Save")
	FRotator PlayerControlRotation;
};
