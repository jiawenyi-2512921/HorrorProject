// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EditorSubsystem.h"
#include "AudioBatchProcessor.generated.h"

USTRUCT(BlueprintType)
struct FAudioProcessingSettings
{
	GENERATED_BODY()

	static constexpr int32 DefaultTargetSampleRate = 44100;
	static constexpr float DefaultCompressionQuality = 75.0f;

	UPROPERTY()
	bool bNormalize = true;

	UPROPERTY()
	bool bConvertToMono = false;

	UPROPERTY()
	int32 TargetSampleRate = DefaultTargetSampleRate;

	UPROPERTY()
	float CompressionQuality = DefaultCompressionQuality;

	UPROPERTY()
	bool bEnableStreaming = true;
};

UCLASS()
class HORRORPROJECTEDITOR_API UAudioBatchProcessor : public UEditorSubsystem
{
	GENERATED_BODY()

public:
	static void ProcessAudioFiles();
	static void ProcessAudioFile(USoundWave* Sound, const FAudioProcessingSettings& Settings);

private:
	static const FAudioProcessingSettings& GetDefaultSettings();
};
