// Copyright Epic Games, Inc. All Rights Reserved.

#include "AudioBatchProcessor.h"
#include "Sound/SoundWave.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Misc/MessageDialog.h"

FAudioProcessingSettings UAudioBatchProcessor::DefaultSettings;

void UAudioBatchProcessor::ProcessAudioFiles()
{
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	TArray<FAssetData> AssetDataList;
	AssetRegistryModule.Get().GetAssetsByClass(USoundWave::StaticClass()->GetClassPathName(), AssetDataList, true);

	int32 ProcessedCount = 0;

	for (const FAssetData& AssetData : AssetDataList)
	{
		USoundWave* Sound = Cast<USoundWave>(AssetData.GetAsset());
		if (Sound)
		{
			ProcessAudioFile(Sound, DefaultSettings);
			ProcessedCount++;
		}
	}

	FText Message = FText::FromString(FString::Printf(
		TEXT("Audio Batch Processing Complete\n\nProcessed %d audio files"),
		ProcessedCount
	));
	FMessageDialog::Open(EAppMsgType::Ok, Message);

	UE_LOG(LogTemp, Log, TEXT("Audio Batch Processing: Processed %d files"), ProcessedCount);
}

void UAudioBatchProcessor::ProcessAudioFile(USoundWave* Sound, const FAudioProcessingSettings& Settings)
{
	if (!Sound) return;

	// Apply streaming settings
	if (Settings.bEnableStreaming && Sound->Duration > 5.0f)
	{
		Sound->bStreaming = true;
	}

	// Apply compression quality
	Sound->CompressionQuality = Settings.CompressionQuality;

	Sound->PostEditChange();

	UE_LOG(LogTemp, Log, TEXT("Processed audio: %s"), *Sound->GetName());
}
