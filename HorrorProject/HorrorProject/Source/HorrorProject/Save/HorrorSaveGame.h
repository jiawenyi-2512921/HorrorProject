// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "GameplayTagContainer.h"
#include "HorrorSaveGame.generated.h"

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
	TArray<FName> RecordedNoteIds;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, SaveGame, Category="Horror|Save")
	FTransform PlayerTransform;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, SaveGame, Category="Horror|Save")
	FRotator PlayerControlRotation;
};
