// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "SaveGameValidator.generated.h"

class UHorrorSaveGame;

UENUM(BlueprintType)
enum class ESaveGameValidationResult : uint8
{
	Valid,
	CorruptedData,
	InvalidVersion,
	MissingRequiredData,
	ChecksumMismatch
};

/**
 * Defines Save Game Validator behavior for the SaveGame module.
 */
UCLASS()
class HORRORPROJECT_API USaveGameValidator : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category="SaveGame")
	ESaveGameValidationResult ValidateSaveGame(const UHorrorSaveGame* SaveGame) const;

	UFUNCTION(BlueprintPure, Category="SaveGame")
	bool IsSaveGameValid(const UHorrorSaveGame* SaveGame) const;

	UFUNCTION(BlueprintCallable, Category="SaveGame")
	FString GetValidationErrorMessage(ESaveGameValidationResult Result) const;

private:
	bool ValidateVersion(const UHorrorSaveGame* SaveGame) const;
	bool ValidateRequiredData(const UHorrorSaveGame* SaveGame) const;
	bool ValidateDataIntegrity(const UHorrorSaveGame* SaveGame) const;
};
