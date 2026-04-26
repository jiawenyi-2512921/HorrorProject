// Copyright Epic Games, Inc. All Rights Reserved.

#include "AudioLocalization.h"
#include "LocalizationSubsystem.h"
#include "Kismet/GameplayStatics.h"

TSoftObjectPtr<USoundBase> UAudioLocalizationData::GetAudio(const FString& Key, const FString& LanguageCode) const
{
	for (const FLocalizedAudio& LocalizedAudio : LocalizedAudios)
	{
		if (LocalizedAudio.Key == Key)
		{
			if (LanguageCode == TEXT("en"))
				return LocalizedAudio.English;
			else if (LanguageCode == TEXT("zh-Hans"))
				return LocalizedAudio.Chinese;
			else if (LanguageCode == TEXT("ja"))
				return LocalizedAudio.Japanese;
			else if (LanguageCode == TEXT("ko"))
				return LocalizedAudio.Korean;
			else if (LanguageCode == TEXT("es"))
				return LocalizedAudio.Spanish;
		}
	}
	return nullptr;
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

	// Load audio localization data asset
	// In production, this should be loaded from a data registry or asset manager
	// For now, return nullptr as placeholder

	return nullptr;
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
