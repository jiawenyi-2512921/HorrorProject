// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Audio/HorrorAudioSubsystem.h"
#include "HorrorAudioLibrary.generated.h"

/**
 * Exposes Blueprint helper functions for Horror Audio Library workflows.
 */
UCLASS()
class HORRORPROJECT_API UHorrorAudioLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category="Horror|Audio", meta=(WorldContext="WorldContextObject"))
	static UHorrorAudioSubsystem* GetHorrorAudioSubsystem(const UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable, Category="Horror|Audio", meta=(WorldContext="WorldContextObject"))
	static UAudioComponent* PlayHorrorSound3D(const UObject* WorldContextObject, USoundBase* Sound, FVector Location, float VolumeMultiplier = 1.0f);

	UFUNCTION(BlueprintCallable, Category="Horror|Audio", meta=(WorldContext="WorldContextObject"))
	static UAudioComponent* PlayHorrorSound2D(const UObject* WorldContextObject, USoundBase* Sound, float VolumeMultiplier = 1.0f);

	UFUNCTION(BlueprintCallable, Category="Horror|Audio", meta=(WorldContext="WorldContextObject"))
	static bool PlayHorrorEventSound(const UObject* WorldContextObject, FGameplayTag EventTag, UObject* SourceObject = nullptr);

	UFUNCTION(BlueprintCallable, Category="Horror|Audio", meta=(WorldContext="WorldContextObject"))
	static bool EnterHorrorAudioZone(const UObject* WorldContextObject, FName ZoneId);

	UFUNCTION(BlueprintCallable, Category="Horror|Audio", meta=(WorldContext="WorldContextObject"))
	static bool ExitHorrorAudioZone(const UObject* WorldContextObject, FName ZoneId);

	UFUNCTION(BlueprintCallable, Category="Horror|Audio", meta=(WorldContext="WorldContextObject"))
	static void StopAllHorrorAmbient(const UObject* WorldContextObject, float FadeOutDuration = 1.0f);

	UFUNCTION(BlueprintCallable, Category="Horror|Audio", meta=(WorldContext="WorldContextObject"))
	static void SetHorrorCategoryVolume(const UObject* WorldContextObject, EHorrorAudioCategory Category, float Volume);

	UFUNCTION(BlueprintPure, Category="Horror|Audio", meta=(WorldContext="WorldContextObject"))
	static float GetHorrorCategoryVolume(const UObject* WorldContextObject, EHorrorAudioCategory Category);

	UFUNCTION(BlueprintCallable, Category="Horror|Audio", meta=(CPP_Default_VolumeMultiplier="1.0", CPP_Default_bUse3D="true", CPP_Default_AttenuationRadius="2000.0"))
	static FHorrorAudioEventMapping MakeAudioEventMapping(FGameplayTag EventTag, USoundBase* Sound, float VolumeMultiplier, bool bUse3D, float AttenuationRadius);

	UFUNCTION(BlueprintCallable, Category="Horror|Audio")
	static FHorrorAudioZoneConfig MakeAudioZoneConfig(FName ZoneId, USoundBase* AmbientSound, float Volume = 0.5f, bool bLoop = true, float FadeIn = 2.0f, float FadeOut = 1.5f);
};
