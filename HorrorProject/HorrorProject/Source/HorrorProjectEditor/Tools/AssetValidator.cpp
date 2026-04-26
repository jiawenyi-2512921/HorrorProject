// Copyright Epic Games, Inc. All Rights Reserved.

#include "AssetValidator.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Engine/Texture2D.h"
#include "Engine/StaticMesh.h"
#include "Materials/Material.h"
#include "Sound/SoundWave.h"
#include "Engine/Blueprint.h"
#include "Misc/MessageDialog.h"

TArray<FAssetValidationResult> UAssetValidator::ValidationResults;

void UAssetValidator::ValidateAllAssets()
{
	ValidationResults.Empty();

	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	TArray<FAssetData> AssetDataList;
	AssetRegistryModule.Get().GetAssetsByPath(FName("/Game"), AssetDataList, true);

	int32 TotalAssets = AssetDataList.Num();
	int32 ValidAssets = 0;
	int32 InvalidAssets = 0;

	for (const FAssetData& AssetData : AssetDataList)
	{
		FAssetValidationResult Result;
		Result.AssetPath = AssetData.GetObjectPathString();

		UObject* Asset = AssetData.GetAsset();
		if (!Asset)
		{
			Result.Errors.Add("Failed to load asset");
			Result.bIsValid = false;
			ValidationResults.Add(Result);
			InvalidAssets++;
			continue;
		}

		bool bValid = true;

		if (Asset->IsA<UTexture2D>())
			bValid = ValidateTexture(Asset, Result);
		else if (Asset->IsA<UStaticMesh>())
			bValid = ValidateStaticMesh(Asset, Result);
		else if (Asset->IsA<UMaterial>())
			bValid = ValidateMaterial(Asset, Result);
		else if (Asset->IsA<USoundWave>())
			bValid = ValidateSound(Asset, Result);
		else if (Asset->IsA<UBlueprint>())
			bValid = ValidateBlueprint(Asset, Result);

		Result.bIsValid = bValid;
		ValidationResults.Add(Result);

		if (bValid)
			ValidAssets++;
		else
			InvalidAssets++;
	}

	FText Message = FText::FromString(FString::Printf(
		TEXT("Asset Validation Complete\n\nTotal: %d\nValid: %d\nInvalid: %d"),
		TotalAssets, ValidAssets, InvalidAssets
	));
	FMessageDialog::Open(EAppMsgType::Ok, Message);

	UE_LOG(LogTemp, Log, TEXT("Asset Validation: %d valid, %d invalid out of %d total"), ValidAssets, InvalidAssets, TotalAssets);
}

void UAssetValidator::ValidateAsset(const FString& AssetPath)
{
	UObject* Asset = LoadObject<UObject>(nullptr, *AssetPath);
	if (!Asset)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to load asset: %s"), *AssetPath);
		return;
	}

	FAssetValidationResult Result;
	Result.AssetPath = AssetPath;

	if (Asset->IsA<UTexture2D>())
		ValidateTexture(Asset, Result);
	else if (Asset->IsA<UStaticMesh>())
		ValidateStaticMesh(Asset, Result);
	else if (Asset->IsA<UMaterial>())
		ValidateMaterial(Asset, Result);
	else if (Asset->IsA<USoundWave>())
		ValidateSound(Asset, Result);
	else if (Asset->IsA<UBlueprint>())
		ValidateBlueprint(Asset, Result);
}

bool UAssetValidator::ValidateTexture(UObject* Asset, FAssetValidationResult& Result)
{
	UTexture2D* Texture = Cast<UTexture2D>(Asset);
	if (!Texture) return false;

	bool bValid = true;

	// Check texture size
	if (Texture->GetSizeX() > 4096 || Texture->GetSizeY() > 4096)
	{
		Result.Warnings.Add(FString::Printf(TEXT("Texture size exceeds 4096: %dx%d"), Texture->GetSizeX(), Texture->GetSizeY()));
	}

	// Check power of two
	if (!FMath::IsPowerOfTwo(Texture->GetSizeX()) || !FMath::IsPowerOfTwo(Texture->GetSizeY()))
	{
		Result.Warnings.Add("Texture dimensions are not power of two");
	}

	// Check mipmap generation
	if (!Texture->NeverStream && Texture->GetNumMips() <= 1)
	{
		Result.Warnings.Add("Texture has no mipmaps");
	}

	return bValid;
}

bool UAssetValidator::ValidateStaticMesh(UObject* Asset, FAssetValidationResult& Result)
{
	UStaticMesh* Mesh = Cast<UStaticMesh>(Asset);
	if (!Mesh) return false;

	bool bValid = true;

	// Check triangle count
	if (Mesh->GetNumTriangles(0) > 50000)
	{
		Result.Warnings.Add(FString::Printf(TEXT("High triangle count: %d"), Mesh->GetNumTriangles(0)));
	}

	// Check LODs
	if (Mesh->GetNumLODs() < 3)
	{
		Result.Warnings.Add(FString::Printf(TEXT("Insufficient LODs: %d (recommended: 3+)"), Mesh->GetNumLODs()));
	}

	// Check collision
	if (!Mesh->GetBodySetup())
	{
		Result.Errors.Add("Missing collision data");
		bValid = false;
	}

	return bValid;
}

bool UAssetValidator::ValidateMaterial(UObject* Asset, FAssetValidationResult& Result)
{
	UMaterial* Material = Cast<UMaterial>(Asset);
	if (!Material) return false;

	bool bValid = true;

	// Check shader complexity
	if (Material->GetExpressions().Num() > 100)
	{
		Result.Warnings.Add(FString::Printf(TEXT("High shader complexity: %d nodes"), Material->GetExpressions().Num()));
	}

	return bValid;
}

bool UAssetValidator::ValidateSound(UObject* Asset, FAssetValidationResult& Result)
{
	USoundWave* Sound = Cast<USoundWave>(Asset);
	if (!Sound) return false;

	bool bValid = true;

	// Check sample rate
	if (Sound->GetSampleRateForCurrentPlatform() > 48000)
	{
		Result.Warnings.Add(FString::Printf(TEXT("High sample rate: %d Hz"), Sound->GetSampleRateForCurrentPlatform()));
	}

	// Check duration
	if (Sound->Duration > 60.0f)
	{
		Result.Warnings.Add(FString::Printf(TEXT("Long audio duration: %.2f seconds"), Sound->Duration));
	}

	return bValid;
}

bool UAssetValidator::ValidateBlueprint(UObject* Asset, FAssetValidationResult& Result)
{
	UBlueprint* Blueprint = Cast<UBlueprint>(Asset);
	if (!Blueprint) return false;

	bool bValid = true;

	// Check compilation status
	if (Blueprint->Status == BS_Error)
	{
		Result.Errors.Add("Blueprint has compilation errors");
		bValid = false;
	}
	else if (Blueprint->Status == BS_UpToDateWithWarnings)
	{
		Result.Warnings.Add("Blueprint has compilation warnings");
	}

	return bValid;
}

TArray<FAssetValidationResult> UAssetValidator::GetValidationResults()
{
	return ValidationResults;
}
