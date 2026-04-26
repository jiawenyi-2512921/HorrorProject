// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Sound/SoundWave.h"
#include "HorrorAudioAssetAuditor.generated.h"

USTRUCT(BlueprintType)
struct HORRORPROJECT_API FAudioAssetReport
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category="Audio Audit")
	FString AssetPath;

	UPROPERTY(BlueprintReadOnly, Category="Audio Audit")
	FString AssetName;

	UPROPERTY(BlueprintReadOnly, Category="Audio Audit")
	int32 SampleRate = 0;

	UPROPERTY(BlueprintReadOnly, Category="Audio Audit")
	int32 NumChannels = 0;

	UPROPERTY(BlueprintReadOnly, Category="Audio Audit")
	float Duration = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category="Audio Audit")
	int32 FileSizeKB = 0;

	UPROPERTY(BlueprintReadOnly, Category="Audio Audit")
	FString CompressionQuality;

	UPROPERTY(BlueprintReadOnly, Category="Audio Audit")
	bool bIsLooping = false;

	UPROPERTY(BlueprintReadOnly, Category="Audio Audit")
	bool bNeedsOptimization = false;

	UPROPERTY(BlueprintReadOnly, Category="Audio Audit")
	TArray<FString> OptimizationSuggestions;
};

UCLASS()
class HORRORPROJECT_API UHorrorAudioAssetAuditor : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category="Audio Audit")
	static TArray<FAudioAssetReport> AuditAllAudioAssets(const FString& ContentPath);

	UFUNCTION(BlueprintCallable, Category="Audio Audit")
	static FAudioAssetReport AuditSingleAsset(USoundWave* SoundWave);

	UFUNCTION(BlueprintCallable, Category="Audio Audit")
	static void GenerateAuditReport(const TArray<FAudioAssetReport>& Reports, const FString& OutputPath);

	UFUNCTION(BlueprintCallable, Category="Audio Audit")
	static bool OptimizeAudioAsset(USoundWave* SoundWave, bool bForceMonoForShortSounds = true, int32 TargetSampleRate = 44100);

	UFUNCTION(BlueprintCallable, Category="Audio Audit")
	static void NormalizeVolumes(const TArray<USoundWave*>& SoundWaves, float TargetPeakDB = -3.0f);

private:
	static bool ShouldBeMonoSound(USoundWave* SoundWave);
	static bool ShouldReduceSampleRate(USoundWave* SoundWave);
	static FString GetCompressionQualityString(USoundWave* SoundWave);
	static void AddOptimizationSuggestions(FAudioAssetReport& Report, USoundWave* SoundWave);
};
