// Copyright Epic Games, Inc. All Rights Reserved.

#include "AudioBatchProcessor.h"
#include "Sound/SoundWave.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Misc/MessageDialog.h"
#include "UObject/UnrealType.h"

const FAudioProcessingSettings& UAudioBatchProcessor::GetDefaultSettings()
{
	static const FAudioProcessingSettings Settings;
	return Settings;
}

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
			ProcessAudioFile(Sound, GetDefaultSettings());
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
	if (FIntProperty* CompressionQualityProperty = FindFProperty<FIntProperty>(USoundWave::StaticClass(), TEXT("CompressionQuality")))
	{
		const int32 CompressionQuality = FMath::Clamp(FMath::RoundToInt(Settings.CompressionQuality), 1, 100);
		Sound->PreEditChange(CompressionQualityProperty);
		CompressionQualityProperty->SetPropertyValue_InContainer(Sound, CompressionQuality);
		FPropertyChangedEvent ChangeEvent(CompressionQualityProperty);
		Sound->PostEditChangeProperty(ChangeEvent);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Unable to find CompressionQuality property for %s"), *Sound->GetName());
	}

	Sound->PostEditChange();

	UE_LOG(LogTemp, Log, TEXT("Processed audio: %s"), *Sound->GetName());
}
