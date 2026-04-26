// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "Evidence/EvidenceTypes.h"
#include "EvidenceSaveGame.generated.h"

USTRUCT(BlueprintType)
struct HORRORPROJECT_API FEvidenceSaveData
{
	GENERATED_BODY()

	UPROPERTY(SaveGame, BlueprintReadWrite, Category="Evidence")
	TArray<FName> CollectedEvidenceIds;

	UPROPERTY(SaveGame, BlueprintReadWrite, Category="Evidence")
	TMap<FName, FExtendedEvidenceMetadata> EvidenceMetadata;

	UPROPERTY(SaveGame, BlueprintReadWrite, Category="Evidence")
	TArray<FName> ViewedArchiveEntries;

	UPROPERTY(SaveGame, BlueprintReadWrite, Category="Evidence")
	int32 PhotoCount = 0;

	UPROPERTY(SaveGame, BlueprintReadWrite, Category="Evidence")
	int32 VideoCount = 0;
};

UCLASS()
class HORRORPROJECT_API UEvidenceSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UPROPERTY(SaveGame, BlueprintReadWrite, Category="Evidence")
	FEvidenceSaveData EvidenceData;

	UFUNCTION(BlueprintCallable, Category="Evidence|Save")
	static UEvidenceSaveGame* CreateEvidenceSaveGame();

	UFUNCTION(BlueprintCallable, Category="Evidence|Save")
	static bool SaveEvidenceData(const FString& SlotName, const FEvidenceSaveData& Data);

	UFUNCTION(BlueprintCallable, Category="Evidence|Save")
	static bool LoadEvidenceData(const FString& SlotName, FEvidenceSaveData& OutData);
};
