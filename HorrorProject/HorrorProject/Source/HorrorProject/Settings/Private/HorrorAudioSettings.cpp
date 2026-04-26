// Copyright Epic Games, Inc. All Rights Reserved.

#include "HorrorAudioSettings.h"
#include "Sound/SoundClass.h"
#include "Sound/SoundMix.h"
#include "AudioDevice.h"
#include "Engine/Engine.h"

UHorrorAudioSettings::UHorrorAudioSettings()
{
	MasterVolume = 1.0f;
	MusicVolume = 0.8f;
	SFXVolume = 1.0f;
	DialogueVolume = 1.0f;
	AmbienceVolume = 0.7f;
	UIVolume = 0.9f;

	AudioQuality = 3;
	bEnableReverb = true;
	bEnableOcclusion = true;
	MaxChannels = 32;

	bEnableSpatialAudio = true;
	bEnableHRTF = false;
	bEnableAttenuation = true;

	OutputDevice = TEXT("Default");
	bEnableSurround = false;
	SpeakerConfiguration = 2;

	bEnableSubtitles = true;
	SubtitleSize = 1.0f;
	bEnableClosedCaptions = false;
	bEnableAudioDescription = false;
}

void UHorrorAudioSettings::Apply()
{
	ApplyVolumeSettings();
	ApplyQualitySettings();
	ApplySpatialSettings();
}

void UHorrorAudioSettings::AutoDetect()
{
	DetectAudioCapabilities();
}

void UHorrorAudioSettings::SetMasterVolume(float Volume, bool bApplyImmediately)
{
	MasterVolume = FMath::Clamp(Volume, 0.0f, 1.0f);

	if (bApplyImmediately)
	{
		ApplyVolumeSettings();
	}
}

void UHorrorAudioSettings::SetCategoryVolume(FName Category, float Volume, bool bApplyImmediately)
{
	Volume = FMath::Clamp(Volume, 0.0f, 1.0f);

	if (Category == TEXT("Music"))
	{
		MusicVolume = Volume;
	}
	else if (Category == TEXT("SFX"))
	{
		SFXVolume = Volume;
	}
	else if (Category == TEXT("Dialogue"))
	{
		DialogueVolume = Volume;
	}
	else if (Category == TEXT("Ambience"))
	{
		AmbienceVolume = Volume;
	}
	else if (Category == TEXT("UI"))
	{
		UIVolume = Volume;
	}

	if (bApplyImmediately)
	{
		ApplyVolumeSettings();
	}
}

TArray<FString> UHorrorAudioSettings::GetAvailableOutputDevices() const
{
	TArray<FString> Devices;
	Devices.Add(TEXT("Default"));

	if (GEngine)
	{
		FAudioDeviceHandle AudioDevice = GEngine->GetMainAudioDevice();
		if (AudioDevice)
		{
			AudioDevice->GetAudioDeviceList(Devices);
			Devices.Insert(TEXT("Default"), 0);
		}
	}

	return Devices;
}

bool UHorrorAudioSettings::IsSpatialAudioSupported() const
{
	if (GEngine)
	{
		FAudioDeviceHandle AudioDevice = GEngine->GetMainAudioDevice();
		if (AudioDevice)
		{
			return AudioDevice->IsSpatializationPluginEnabled();
		}
	}
	return false;
}

void UHorrorAudioSettings::ApplyVolumeSettings()
{
	if (GEngine)
	{
		FAudioDeviceHandle AudioDevice = GEngine->GetMainAudioDevice();
		if (AudioDevice)
		{
			AudioDevice->SetTransientPrimaryVolume(MasterVolume);
		}
	}
}

void UHorrorAudioSettings::ApplyQualitySettings()
{
	if (GEngine)
	{
		FAudioDeviceHandle AudioDevice = GEngine->GetMainAudioDevice();
		if (AudioDevice)
		{
			AudioDevice->SetMaxChannels(MaxChannels);
		}
	}
}

void UHorrorAudioSettings::ApplySpatialSettings()
{
	if (GEngine)
	{
		FAudioDeviceHandle AudioDevice = GEngine->GetMainAudioDevice();
		if (AudioDevice)
		{
			AudioDevice->SetSpatializationInterfaceEnabled(bEnableSpatialAudio);
		}
	}
}

void UHorrorAudioSettings::DetectAudioCapabilities()
{
	bEnableSpatialAudio = IsSpatialAudioSupported();
}
