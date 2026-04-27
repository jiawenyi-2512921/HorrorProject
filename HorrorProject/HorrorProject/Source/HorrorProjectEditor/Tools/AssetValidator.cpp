// Copyright Epic Games, Inc. All Rights Reserved.

#include "AssetValidator.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Engine/Texture2D.h"
#include "Engine/StaticMesh.h"
#include "Materials/Material.h"
#include "Sound/SoundWave.h"
#include "Engine/Blueprint.h"
#include "HAL/CriticalSection.h"
#include "Misc/MessageDialog.h"

namespace
{
	constexpr int32 MaxRecommendedTextureDimension = 4096;
	constexpr int32 MaxRecommendedTriangleCount = 50000;
	constexpr int32 MaxRecommendedSampleRate = 48000;
	constexpr float MaxRecommendedSoundDurationSeconds = 60.0f;

	TArray<FAssetValidationResult>& GetValidationResultsStorage()
	{
		static auto Results = TArray<FAssetValidationResult>();
		return Results;
	}

	FCriticalSection& GetValidationResultsMutex()
	{
		static auto Mutex = FCriticalSection();
		return Mutex;
	}

	void StoreValidationResults(TArray<FAssetValidationResult>&& NewResults)
	{
		FScopeLock Lock(&GetValidationResultsMutex());
		GetValidationResultsStorage() = MoveTemp(NewResults);
	}
}

void UAssetValidator::ValidateAllAssets()
{
	TArray<FAssetValidationResult> NewResults;

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
			NewResults.Add(Result);
			InvalidAssets++;
			continue;
		}

		Result.bIsValid = ValidateLoadedAsset(Asset, Result);
		NewResults.Add(Result);

		if (Result.bIsValid)
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

	StoreValidationResults(MoveTemp(NewResults));
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
	Result.bIsValid = ValidateLoadedAsset(Asset, Result);

	{
		FScopeLock Lock(&GetValidationResultsMutex());
		GetValidationResultsStorage().Add(Result);
	}
}

bool UAssetValidator::ValidateLoadedAsset(UObject* Asset, FAssetValidationResult& Result)
{
	if (Asset->IsA<UTexture2D>())
		return ValidateTexture(Asset, Result);
	else if (Asset->IsA<UStaticMesh>())
		return ValidateStaticMesh(Asset, Result);
	else if (Asset->IsA<UMaterial>())
		return ValidateMaterial(Asset, Result);
	else if (Asset->IsA<USoundWave>())
		return ValidateSound(Asset, Result);
	else if (Asset->IsA<UBlueprint>())
		return ValidateBlueprint(Asset, Result);

	return true;
}

bool UAssetValidator::ValidateTexture(UObject* Asset, FAssetValidationResult& Result)
{
	UTexture2D* Texture = Cast<UTexture2D>(Asset);
	if (!Texture) return false;

	bool bValid = true;

	// Check texture size
	if (Texture->GetSizeX() > MaxRecommendedTextureDimension || Texture->GetSizeY() > MaxRecommendedTextureDimension)
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
	if (Mesh->GetNumTriangles(0) > MaxRecommendedTriangleCount)
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
	const float SampleRate = Sound->GetSampleRateForCurrentPlatform();
	if (SampleRate > MaxRecommendedSampleRate)
	{
		Result.Warnings.Add(FString::Printf(TEXT("High sample rate: %.0f Hz"), SampleRate));
	}

	// Check duration
	if (Sound->Duration > MaxRecommendedSoundDurationSeconds)
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
	FScopeLock Lock(&GetValidationResultsMutex());
	return GetValidationResultsStorage();
}
