// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Sound/SoundBase.h"
#include "AudioLocalization.generated.h"

USTRUCT(BlueprintType)
struct FLocalizedAudio
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Key;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<USoundBase> English;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<USoundBase> Chinese;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<USoundBase> Japanese;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<USoundBase> Korean;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<USoundBase> Spanish;
};

/**
 * Defines Audio Localization Data data used by the Localization module.
 */
UCLASS()
class HORRORPROJECT_API UAudioLocalizationData : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Localization")
	TArray<FLocalizedAudio> LocalizedAudios;

	UFUNCTION(BlueprintPure, Category = "Localization")
	TSoftObjectPtr<USoundBase> GetAudio(const FString& Key, const FString& LanguageCode) const;
};

/**
 * Exposes Blueprint helper functions for Audio Localization Library workflows.
 */
UCLASS(BlueprintType)
class HORRORPROJECT_API UAudioLocalizationLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Localization|Audio", meta = (WorldContext = "WorldContextObject"))
	static USoundBase* GetLocalizedAudio(UObject* WorldContextObject, const FString& Key);

	UFUNCTION(BlueprintCallable, Category = "Localization|Audio", meta = (WorldContext = "WorldContextObject"))
	static void PlayLocalizedAudio(UObject* WorldContextObject, const FString& Key, FVector Location, float VolumeMultiplier = 1.0f);

	UFUNCTION(BlueprintCallable, Category = "Localization|Audio", meta = (WorldContext = "WorldContextObject"))
	static void PlayLocalizedAudio2D(UObject* WorldContextObject, const FString& Key, float VolumeMultiplier = 1.0f);
};
