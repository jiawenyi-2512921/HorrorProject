// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "DefaultConfigs.generated.h"

class UGraphicsSettings;
class UAudioSettings;
class UControlSettings;
class UGameplaySettings;

/**
 * Default Configs
 * Provides default configuration values
 */
UCLASS()
class HORRORPROJECT_API UDefaultConfigs : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// Apply default settings
	UFUNCTION(BlueprintCallable, Category = "Defaults")
	static void ApplyDefaultGraphicsSettings(UGraphicsSettings* Settings);

	UFUNCTION(BlueprintCallable, Category = "Defaults")
	static void ApplyDefaultAudioSettings(UAudioSettings* Settings);

	UFUNCTION(BlueprintCallable, Category = "Defaults")
	static void ApplyDefaultControlSettings(UControlSettings* Settings);

	UFUNCTION(BlueprintCallable, Category = "Defaults")
	static void ApplyDefaultGameplaySettings(UGameplaySettings* Settings);

	// Get recommended settings based on hardware
	UFUNCTION(BlueprintCallable, Category = "Defaults")
	static void ApplyRecommendedGraphicsSettings(UGraphicsSettings* Settings);

	UFUNCTION(BlueprintCallable, Category = "Defaults")
	static void ApplyRecommendedAudioSettings(UAudioSettings* Settings);

private:
	static int32 DetectRecommendedQualityLevel();
};
