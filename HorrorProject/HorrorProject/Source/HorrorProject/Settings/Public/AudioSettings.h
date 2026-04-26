// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "AudioSettings.generated.h"

class USoundClass;
class USoundMix;

/**
 * Audio Settings
 * Manages all audio-related settings
 */
UCLASS(BlueprintType, Config=GameUserSettings)
class HORRORPROJECT_API UAudioSettings : public UObject
{
	GENERATED_BODY()

public:
	UAudioSettings();

	// Volume Settings
	UPROPERTY(Config, BlueprintReadWrite, Category = "Volume")
	float MasterVolume;

	UPROPERTY(Config, BlueprintReadWrite, Category = "Volume")
	float MusicVolume;

	UPROPERTY(Config, BlueprintReadWrite, Category = "Volume")
	float SFXVolume;

	UPROPERTY(Config, BlueprintReadWrite, Category = "Volume")
	float DialogueVolume;

	UPROPERTY(Config, BlueprintReadWrite, Category = "Volume")
	float AmbienceVolume;

	UPROPERTY(Config, BlueprintReadWrite, Category = "Volume")
	float UIVolume;

	// Audio Quality
	UPROPERTY(Config, BlueprintReadWrite, Category = "Quality")
	int32 AudioQuality;

	UPROPERTY(Config, BlueprintReadWrite, Category = "Quality")
	bool bEnableReverb;

	UPROPERTY(Config, BlueprintReadWrite, Category = "Quality")
	bool bEnableOcclusion;

	UPROPERTY(Config, BlueprintReadWrite, Category = "Quality")
	int32 MaxChannels;

	// Spatial Audio
	UPROPERTY(Config, BlueprintReadWrite, Category = "Spatial")
	bool bEnableSpatialAudio;

	UPROPERTY(Config, BlueprintReadWrite, Category = "Spatial")
	bool bEnableHRTF;

	UPROPERTY(Config, BlueprintReadWrite, Category = "Spatial")
	bool bEnableAttenuation;

	// Output
	UPROPERTY(Config, BlueprintReadWrite, Category = "Output")
	FString OutputDevice;

	UPROPERTY(Config, BlueprintReadWrite, Category = "Output")
	bool bEnableSurround;

	UPROPERTY(Config, BlueprintReadWrite, Category = "Output")
	int32 SpeakerConfiguration;

	// Accessibility
	UPROPERTY(Config, BlueprintReadWrite, Category = "Accessibility")
	bool bEnableSubtitles;

	UPROPERTY(Config, BlueprintReadWrite, Category = "Accessibility")
	float SubtitleSize;

	UPROPERTY(Config, BlueprintReadWrite, Category = "Accessibility")
	bool bEnableClosedCaptions;

	UPROPERTY(Config, BlueprintReadWrite, Category = "Accessibility")
	bool bEnableAudioDescription;

	// Operations
	UFUNCTION(BlueprintCallable, Category = "Audio")
	void Apply();

	UFUNCTION(BlueprintCallable, Category = "Audio")
	void AutoDetect();

	UFUNCTION(BlueprintCallable, Category = "Audio")
	void SetMasterVolume(float Volume, bool bApplyImmediately = true);

	UFUNCTION(BlueprintCallable, Category = "Audio")
	void SetCategoryVolume(FName Category, float Volume, bool bApplyImmediately = true);

	UFUNCTION(BlueprintPure, Category = "Audio")
	TArray<FString> GetAvailableOutputDevices() const;

	UFUNCTION(BlueprintPure, Category = "Audio")
	bool IsSpatialAudioSupported() const;

private:
	void ApplyVolumeSettings();
	void ApplyQualitySettings();
	void ApplySpatialSettings();
	void DetectAudioCapabilities();

	UPROPERTY()
	TObjectPtr<USoundMix> CurrentSoundMix;
};
