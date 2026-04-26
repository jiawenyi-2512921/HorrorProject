// Copyright Epic Games, Inc. All Rights Reserved.

#include "AudioSettings.h"
#include "Sound/SoundClass.h"
#include "Sound/SoundMix.h"
#include "AudioDevice.h"
#include "Engine/Engine.h"

UAudioSettings::UAudioSettings()
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

void UAudioSettings::Apply()
{
	ApplyVolumeSettings();
	ApplyQualitySettings();
	ApplySpatialSettings();
}

void UAudioSettings::AutoDetect()
{
	DetectAudioCapabilities();
}

void UAudioSettings::SetMasterVolume(float Volume, bool bApplyImmediately)
{
	MasterVolume = FMath::Clamp(Volume, 0.0f, 1.0f);

	if (bApplyImmediately)
	{
		ApplyVolumeSettings();
	}
}

void UAudioSettings::SetCategoryVolume(FName Category, float Volume, bool bApplyImmediately)
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

TArray<FString> UAudioSettings::GetAvailableOutputDevices() const
{
	TArray<FString> Devices;
	Devices.Add(TEXT("Default"));

	if (FAudioDevice* AudioDevice = GEngine->GetMainAudioDevice())
	{
		// Get available audio devices from the audio device
		// This is platform-specific implementation
	}

	return Devices;
}

bool UAudioSettings::IsSpatialAudioSupported() const
{
	if (FAudioDevice* AudioDevice = GEngine->GetMainAudioDevice())
	{
		return AudioDevice->IsSpatializationPluginEnabled();
	}
	return false;
}

void UAudioSettings::ApplyVolumeSettings()
{
	if (FAudioDevice* AudioDevice = GEngine->GetMainAudioDevice())
	{
		AudioDevice->SetTransientMasterVolume(MasterVolume);
	}
}

void UAudioSettings::ApplyQualitySettings()
{
	if (FAudioDevice* AudioDevice = GEngine->GetMainAudioDevice())
	{
		AudioDevice->SetMaxChannels(MaxChannels);
	}
}

void UAudioSettings::ApplySpatialSettings()
{
	if (FAudioDevice* AudioDevice = GEngine->GetMainAudioDevice())
	{
		AudioDevice->EnableSpatializationPlugin(bEnableSpatialAudio);
	}
}

void UAudioSettings::DetectAudioCapabilities()
{
	bEnableSpatialAudio = IsSpatialAudioSupported();
}
