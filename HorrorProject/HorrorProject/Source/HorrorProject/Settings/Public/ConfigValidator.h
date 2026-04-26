// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "ConfigValidator.generated.h"

class UGraphicsSettings;
class UHorrorAudioSettings;
class UControlSettings;
class UGameplaySettings;

USTRUCT(BlueprintType)
struct FValidationResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Validation")
	bool bIsValid;

	UPROPERTY(BlueprintReadOnly, Category = "Validation")
	TArray<FString> Errors;

	UPROPERTY(BlueprintReadOnly, Category = "Validation")
	TArray<FString> Warnings;

	FValidationResult()
		: bIsValid(true)
	{}
};

/**
 * Config Validator
 * Validates configuration settings
 */
UCLASS()
class HORRORPROJECT_API UConfigValidator : public UObject
{
	GENERATED_BODY()

public:
	// Validation methods
	UFUNCTION(BlueprintCallable, Category = "Validation")
	bool ValidateGraphicsSettings(UGraphicsSettings* Settings);

	UFUNCTION(BlueprintCallable, Category = "Validation")
	bool ValidateAudioSettings(UHorrorAudioSettings* Settings);

	UFUNCTION(BlueprintCallable, Category = "Validation")
	bool ValidateControlSettings(UControlSettings* Settings);

	UFUNCTION(BlueprintCallable, Category = "Validation")
	bool ValidateGameplaySettings(UGameplaySettings* Settings);

	UFUNCTION(BlueprintCallable, Category = "Validation")
	FValidationResult ValidateAllSettings(UGraphicsSettings* Graphics, UHorrorAudioSettings* Audio,
		UControlSettings* Controls, UGameplaySettings* Gameplay);

	// Get last validation result
	UFUNCTION(BlueprintPure, Category = "Validation")
	FValidationResult GetLastValidationResult() const { return LastValidationResult; }

private:
	bool ValidateResolution(const FIntPoint& Resolution);
	bool ValidateVolumeRange(float Volume);
	bool ValidateKeyBindings(UControlSettings* Settings);
	bool ValidateFOV(float FOV);

	void AddError(const FString& Error);
	void AddWarning(const FString& Warning);
	void ClearResults();

	FValidationResult LastValidationResult;
};
