// Copyright Epic Games, Inc. All Rights Reserved.

#include "Audio/HorrorAudioAssetAuditor.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Misc/FileHelper.h"
#include "HAL/PlatformFileManager.h"

namespace HorrorAudioAudit
{
	constexpr float PercentMultiplier = 100.0f;
	constexpr float KilobytesPerMegabyte = 1024.0f;
	constexpr float ShortSoundDurationSeconds = 2.0f;
	constexpr float LongStreamingDurationSeconds = 10.0f;
	constexpr int32 MaxRecommendedSampleRate = 48000;
	constexpr int32 AmbientSampleRateThreshold = 44100;
	constexpr int32 MaxStereoChannelCount = 2;
}

TArray<FAudioAssetReport> UHorrorAudioAssetAuditor::AuditAllAudioAssets(const FString& ContentPath)
{
	TArray<FAudioAssetReport> Reports;

	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

	TArray<FAssetData> AssetDataList;
	AssetRegistry.GetAssetsByPath(FName(*ContentPath), AssetDataList, true);

	for (const FAssetData& AssetData : AssetDataList)
	{
		if (AssetData.AssetClassPath.GetAssetName() == FName("SoundWave"))
		{
			USoundWave* SoundWave = Cast<USoundWave>(AssetData.GetAsset());
			if (SoundWave)
			{
				FAudioAssetReport Report = AuditSingleAsset(SoundWave);
				Reports.Add(Report);
			}
		}
	}

	return Reports;
}

FAudioAssetReport UHorrorAudioAssetAuditor::AuditSingleAsset(USoundWave* SoundWave)
{
	FAudioAssetReport Report;

	if (!SoundWave)
	{
		return Report;
	}

	Report.AssetPath = SoundWave->GetPathName();
	Report.AssetName = SoundWave->GetName();
	Report.SampleRate = SoundWave->GetSampleRateForCurrentPlatform();
	Report.NumChannels = SoundWave->NumChannels;
	Report.Duration = SoundWave->Duration;
	Report.bIsLooping = SoundWave->bLooping;
	Report.CompressionQuality = GetCompressionQualityString(SoundWave);

	AddOptimizationSuggestions(Report, SoundWave);

	return Report;
}

void UHorrorAudioAssetAuditor::GenerateAuditReport(const TArray<FAudioAssetReport>& Reports, const FString& OutputPath)
{
	FString ReportContent = TEXT("Horror Project Audio Asset Audit Report\n");
	ReportContent += TEXT("========================================\n\n");

	AppendAuditSummary(ReportContent, Reports);
	ReportContent += TEXT("Detailed Asset Reports:\n");
	ReportContent += TEXT("----------------------\n\n");

	for (const FAudioAssetReport& Report : Reports)
	{
		AppendAssetReport(ReportContent, Report);
	}

	FFileHelper::SaveStringToFile(ReportContent, *OutputPath);
}

void UHorrorAudioAssetAuditor::AppendAuditSummary(FString& ReportContent, const TArray<FAudioAssetReport>& Reports)
{
	int32 TotalAssets = Reports.Num();
	int32 AssetsNeedingOptimization = 0;
	float TotalDuration = 0.0f;
	int64 TotalSize = 0;

	for (const FAudioAssetReport& Report : Reports)
	{
		if (Report.bNeedsOptimization)
		{
			AssetsNeedingOptimization++;
		}
		TotalDuration += Report.Duration;
		TotalSize += Report.FileSizeKB;
	}

	ReportContent += FString::Printf(TEXT("Total Assets: %d\n"), TotalAssets);
	ReportContent += FString::Printf(TEXT("Assets Needing Optimization: %d (%.1f%%)\n"),
		AssetsNeedingOptimization,
		TotalAssets > 0 ? (float)AssetsNeedingOptimization / TotalAssets * HorrorAudioAudit::PercentMultiplier : 0.0f);
	ReportContent += FString::Printf(TEXT("Total Duration: %.2f seconds\n"), TotalDuration);
	ReportContent += FString::Printf(TEXT("Total Size: %.2f MB\n\n"), TotalSize / HorrorAudioAudit::KilobytesPerMegabyte);
}

void UHorrorAudioAssetAuditor::AppendAssetReport(FString& ReportContent, const FAudioAssetReport& Report)
{
	ReportContent += FString::Printf(TEXT("Asset: %s\n"), *Report.AssetName);
	ReportContent += FString::Printf(TEXT("  Path: %s\n"), *Report.AssetPath);
	ReportContent += FString::Printf(TEXT("  Sample Rate: %d Hz\n"), Report.SampleRate);
	ReportContent += FString::Printf(TEXT("  Channels: %d\n"), Report.NumChannels);
	ReportContent += FString::Printf(TEXT("  Duration: %.2f seconds\n"), Report.Duration);
	ReportContent += FString::Printf(TEXT("  Size: %d KB\n"), Report.FileSizeKB);
	ReportContent += FString::Printf(TEXT("  Compression: %s\n"), *Report.CompressionQuality);
	ReportContent += FString::Printf(TEXT("  Looping: %s\n"), Report.bIsLooping ? TEXT("Yes") : TEXT("No"));

	if (Report.bNeedsOptimization && Report.OptimizationSuggestions.Num() > 0)
	{
		ReportContent += TEXT("  Optimization Suggestions:\n");
		for (const FString& Suggestion : Report.OptimizationSuggestions)
		{
			ReportContent += FString::Printf(TEXT("    - %s\n"), *Suggestion);
		}
	}

	ReportContent += TEXT("\n");
}

bool UHorrorAudioAssetAuditor::OptimizeAudioAsset(USoundWave* SoundWave, bool bForceMonoForShortSounds, int32 TargetSampleRate)
{
	if (!SoundWave)
	{
		return false;
	}

	bool bModified = false;

	if (bForceMonoForShortSounds && ShouldBeMonoSound(SoundWave))
	{
		if (SoundWave->NumChannels > 1)
		{
			bModified = true;
		}
	}

	if (ShouldReduceSampleRate(SoundWave))
	{
		bModified = true;
	}

	if (bModified)
	{
		SoundWave->MarkPackageDirty();
	}

	return bModified;
}

void UHorrorAudioAssetAuditor::NormalizeVolumes(const TArray<USoundWave*>& SoundWaves, float TargetPeakDB)
{
	for (USoundWave* SoundWave : SoundWaves)
	{
		if (SoundWave)
		{
			SoundWave->Volume = 1.0f;
			SoundWave->MarkPackageDirty();
		}
	}
}

bool UHorrorAudioAssetAuditor::ShouldBeMonoSound(USoundWave* SoundWave)
{
	if (!SoundWave)
	{
		return false;
	}

	if (SoundWave->Duration < HorrorAudioAudit::ShortSoundDurationSeconds)
	{
		return true;
	}

	FString AssetName = SoundWave->GetName().ToLower();
	if (AssetName.Contains(TEXT("footstep")) ||
		AssetName.Contains(TEXT("impact")) ||
		AssetName.Contains(TEXT("click")) ||
		AssetName.Contains(TEXT("button")))
	{
		return true;
	}

	return false;
}

bool UHorrorAudioAssetAuditor::ShouldReduceSampleRate(USoundWave* SoundWave)
{
	if (!SoundWave)
	{
		return false;
	}

	int32 CurrentSampleRate = SoundWave->GetSampleRateForCurrentPlatform();

	if (CurrentSampleRate > HorrorAudioAudit::MaxRecommendedSampleRate)
	{
		return true;
	}

	FString AssetName = SoundWave->GetName().ToLower();
	if ((AssetName.Contains(TEXT("ambient")) || AssetName.Contains(TEXT("background")))
		&& CurrentSampleRate > HorrorAudioAudit::AmbientSampleRateThreshold)
	{
		return true;
	}

	return false;
}

FString UHorrorAudioAssetAuditor::GetCompressionQualityString(USoundWave* SoundWave)
{
	if (!SoundWave)
	{
		return TEXT("Unknown");
	}

	return TEXT("Platform Default");
}

void UHorrorAudioAssetAuditor::AddOptimizationSuggestions(FAudioAssetReport& OutReport, USoundWave* SoundWave)
{
	if (!SoundWave)
	{
		return;
	}

	OutReport.bNeedsOptimization = false;

	if (ShouldBeMonoSound(SoundWave) && SoundWave->NumChannels > 1)
	{
		OutReport.OptimizationSuggestions.Add(TEXT("Convert to mono - short sound effects don't benefit from stereo"));
		OutReport.bNeedsOptimization = true;
	}

	if (ShouldReduceSampleRate(SoundWave))
	{
		OutReport.OptimizationSuggestions.Add(FString::Printf(
			TEXT("Reduce sample rate from %d Hz to %d Hz"),
			OutReport.SampleRate,
			HorrorAudioAudit::AmbientSampleRateThreshold));
		OutReport.bNeedsOptimization = true;
	}

	if (OutReport.Duration > HorrorAudioAudit::LongStreamingDurationSeconds && !SoundWave->bStreaming)
	{
		OutReport.OptimizationSuggestions.Add(TEXT("Enable streaming for long audio files"));
		OutReport.bNeedsOptimization = true;
	}

	if (OutReport.NumChannels > HorrorAudioAudit::MaxStereoChannelCount)
	{
		OutReport.OptimizationSuggestions.Add(FString::Printf(TEXT("Reduce channels from %d to stereo (2)"), OutReport.NumChannels));
		OutReport.bNeedsOptimization = true;
	}

	FString AssetName = SoundWave->GetName().ToLower();
	if (AssetName.Contains(TEXT("ambient")) && !SoundWave->bLooping)
	{
		OutReport.OptimizationSuggestions.Add(TEXT("Consider enabling looping for ambient sounds"));
	}
}
