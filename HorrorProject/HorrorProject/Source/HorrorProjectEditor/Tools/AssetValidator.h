// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EditorSubsystem.h"
#include "AssetValidator.generated.h"

USTRUCT(BlueprintType)
struct FAssetValidationResult
{
	GENERATED_BODY()

	UPROPERTY()
	FString AssetPath;

	UPROPERTY()
	TArray<FString> Errors;

	UPROPERTY()
	TArray<FString> Warnings;

	UPROPERTY()
	bool bIsValid = true;
};

UCLASS()
class HORRORPROJECTEDITOR_API UAssetValidator : public UEditorSubsystem
{
	GENERATED_BODY()

public:
	static void ValidateAllAssets();
	static void ValidateAsset(const FString& AssetPath);

	UFUNCTION()
	static TArray<FAssetValidationResult> GetValidationResults();

private:
	static bool ValidateLoadedAsset(UObject* Asset, FAssetValidationResult& Result);
	static bool ValidateTexture(UObject* Asset, FAssetValidationResult& Result);
	static bool ValidateStaticMesh(UObject* Asset, FAssetValidationResult& Result);
	static bool ValidateMaterial(UObject* Asset, FAssetValidationResult& Result);
	static bool ValidateSound(UObject* Asset, FAssetValidationResult& Result);
	static bool ValidateBlueprint(UObject* Asset, FAssetValidationResult& Result);
};
