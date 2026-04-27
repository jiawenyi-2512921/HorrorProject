// Copyright Epic Games, Inc. All Rights Reserved.

#include "AudioLocalization.h"
#include "LocalizationSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/ConfigCacheIni.h"

namespace
{
	const TCHAR* AudioLocalizationConfigSection = TEXT("/Script/HorrorProject.AudioLocalization");
	const TCHAR* AudioLocalizationConfigKey = TEXT("AudioLocalizationDataAsset");
	const TCHAR* DefaultAudioLocalizationDataPath = TEXT("/Game/Localization/DA_AudioLocalization.DA_AudioLocalization");

	UAudioLocalizationData* LoadAudioLocalizationData()
	{
		FString DataAssetPath;
		if (GConfig)
		{
			GConfig->GetString(AudioLocalizationConfigSection, AudioLocalizationConfigKey, DataAssetPath, GGameIni);
		}

		if (DataAssetPath.IsEmpty())
		{
			DataAssetPath = DefaultAudioLocalizationDataPath;
		}

		const FSoftObjectPath AssetPath(DataAssetPath);
		if (!AssetPath.IsValid())
		{
			UE_LOG(LogTemp, Warning, TEXT("Invalid audio localization data asset path: %s"), *DataAssetPath);
			return nullptr;
		}

		return Cast<UAudioLocalizationData>(AssetPath.TryLoad());
	}
}

TSoftObjectPtr<USoundBase> UAudioLocalizationData::GetAudio(const FString& Key, const FString& LanguageCode) const
{
	for (const FLocalizedAudio& LocalizedAudio : LocalizedAudios)
	{
		if (LocalizedAudio.Key == Key)
		{
			if (LanguageCode == TEXT("en"))
				return LocalizedAudio.English;
			else if (LanguageCode == TEXT("zh-Hans"))
				return LocalizedAudio.Chinese.IsNull() ? LocalizedAudio.English : LocalizedAudio.Chinese;
			else if (LanguageCode == TEXT("ja"))
				return LocalizedAudio.Japanese.IsNull() ? LocalizedAudio.English : LocalizedAudio.Japanese;
			else if (LanguageCode == TEXT("ko"))
				return LocalizedAudio.Korean.IsNull() ? LocalizedAudio.English : LocalizedAudio.Korean;
			else if (LanguageCode == TEXT("es"))
				return LocalizedAudio.Spanish.IsNull() ? LocalizedAudio.English : LocalizedAudio.Spanish;

			return LocalizedAudio.English;
		}
	}
	return TSoftObjectPtr<USoundBase>();
}

USoundBase* UAudioLocalizationLibrary::GetLocalizedAudio(UObject* WorldContextObject, const FString& Key)
{
	if (!WorldContextObject)
	{
		return nullptr;
	}

	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(WorldContextObject);
	if (!GameInstance)
	{
		return nullptr;
	}

	ULocalizationSubsystem* LocalizationSubsystem = GameInstance->GetSubsystem<ULocalizationSubsystem>();
	if (!LocalizationSubsystem)
	{
		return nullptr;
	}

	FString LanguageCode = LocalizationSubsystem->GetLanguageCode(LocalizationSubsystem->GetCurrentLanguage());

	UAudioLocalizationData* AudioData = LoadAudioLocalizationData();
	if (!AudioData)
	{
		UE_LOG(LogTemp, Verbose, TEXT("Audio localization data asset is unavailable"));
		return nullptr;
	}

	TSoftObjectPtr<USoundBase> LocalizedAudio = AudioData->GetAudio(Key, LanguageCode);
	if (LocalizedAudio.IsNull())
	{
		UE_LOG(LogTemp, Verbose, TEXT("Localized audio key '%s' was not found for language '%s'"), *Key, *LanguageCode);
		return nullptr;
	}

	return LocalizedAudio.LoadSynchronous();
}

void UAudioLocalizationLibrary::PlayLocalizedAudio(UObject* WorldContextObject, const FString& Key, FVector Location, float VolumeMultiplier)
{
	USoundBase* Sound = GetLocalizedAudio(WorldContextObject, Key);
	if (Sound)
	{
		UGameplayStatics::PlaySoundAtLocation(WorldContextObject, Sound, Location, VolumeMultiplier);
	}
}

void UAudioLocalizationLibrary::PlayLocalizedAudio2D(UObject* WorldContextObject, const FString& Key, float VolumeMultiplier)
{
	USoundBase* Sound = GetLocalizedAudio(WorldContextObject, Key);
	if (Sound)
	{
		UGameplayStatics::PlaySound2D(WorldContextObject, Sound, VolumeMultiplier);
	}
}
