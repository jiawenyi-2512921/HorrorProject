// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "SaveGameMetadata.generated.h"

USTRUCT(BlueprintType)
struct FSaveSlotMetadata
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category="SaveGame")
	int32 SlotIndex = 0;

	UPROPERTY(BlueprintReadOnly, Category="SaveGame")
	FString SlotName;

	UPROPERTY(BlueprintReadOnly, Category="SaveGame")
	FDateTime SaveTime;

	UPROPERTY(BlueprintReadOnly, Category="SaveGame")
	int32 SaveVersion = 1;

	UPROPERTY(BlueprintReadOnly, Category="SaveGame")
	FName CheckpointId;

	UPROPERTY(BlueprintReadOnly, Category="SaveGame")
	float PlayTimeSeconds = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category="SaveGame")
	FString LevelName;

	UPROPERTY(BlueprintReadOnly, Category="SaveGame")
	bool bIsCloudSynced = false;

	UPROPERTY(BlueprintReadOnly, Category="SaveGame")
	bool bIsCorrupted = false;

	FSaveSlotMetadata() = default;
};

/**
 * Defines Save Game Metadata data used by the SaveGame module.
 */
UCLASS()
class HORRORPROJECT_API USaveGameMetadata : public USaveGame
{
	GENERATED_BODY()

public:
	UPROPERTY(SaveGame)
	TArray<FSaveSlotMetadata> SaveSlots;

	UPROPERTY(SaveGame)
	int32 LastUsedSlot = 0;

	UPROPERTY(SaveGame)
	int32 MetadataVersion = 1;

	FSaveSlotMetadata* FindSlotMetadata(int32 SlotIndex);
	const FSaveSlotMetadata* FindSlotMetadata(int32 SlotIndex) const;
	void UpdateSlotMetadata(const FSaveSlotMetadata& Metadata);
	void RemoveSlotMetadata(int32 SlotIndex);
};
