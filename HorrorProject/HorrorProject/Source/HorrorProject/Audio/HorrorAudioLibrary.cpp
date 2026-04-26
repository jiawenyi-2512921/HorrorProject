// Copyright Epic Games, Inc. All Rights Reserved.

#include "Audio/HorrorAudioLibrary.h"
#include "Components/AudioComponent.h"
#include "Engine/World.h"

UHorrorAudioSubsystem* UHorrorAudioLibrary::GetHorrorAudioSubsystem(const UObject* WorldContextObject)
{
	if (!WorldContextObject)
	{
		return nullptr;
	}

	UWorld* World = WorldContextObject->GetWorld();
	return World ? World->GetSubsystem<UHorrorAudioSubsystem>() : nullptr;
}

UAudioComponent* UHorrorAudioLibrary::PlayHorrorSound3D(const UObject* WorldContextObject, USoundBase* Sound, FVector Location, float VolumeMultiplier)
{
	UHorrorAudioSubsystem* AudioSys = GetHorrorAudioSubsystem(WorldContextObject);
	return AudioSys ? AudioSys->PlaySoundAtLocation(Sound, Location, VolumeMultiplier) : nullptr;
}

UAudioComponent* UHorrorAudioLibrary::PlayHorrorSound2D(const UObject* WorldContextObject, USoundBase* Sound, float VolumeMultiplier)
{
	UHorrorAudioSubsystem* AudioSys = GetHorrorAudioSubsystem(WorldContextObject);
	return AudioSys ? AudioSys->PlaySound2D(Sound, VolumeMultiplier) : nullptr;
}

bool UHorrorAudioLibrary::PlayHorrorEventSound(const UObject* WorldContextObject, FGameplayTag EventTag, UObject* SourceObject)
{
	UHorrorAudioSubsystem* AudioSys = GetHorrorAudioSubsystem(WorldContextObject);
	return AudioSys ? AudioSys->PlayEventSound(EventTag, SourceObject) : false;
}

bool UHorrorAudioLibrary::EnterHorrorAudioZone(const UObject* WorldContextObject, FName ZoneId)
{
	UHorrorAudioSubsystem* AudioSys = GetHorrorAudioSubsystem(WorldContextObject);
	return AudioSys ? AudioSys->EnterAudioZone(ZoneId) : false;
}

bool UHorrorAudioLibrary::ExitHorrorAudioZone(const UObject* WorldContextObject, FName ZoneId)
{
	UHorrorAudioSubsystem* AudioSys = GetHorrorAudioSubsystem(WorldContextObject);
	return AudioSys ? AudioSys->ExitAudioZone(ZoneId) : false;
}

void UHorrorAudioLibrary::StopAllHorrorAmbient(const UObject* WorldContextObject, float FadeOutDuration)
{
	UHorrorAudioSubsystem* AudioSys = GetHorrorAudioSubsystem(WorldContextObject);
	if (AudioSys)
	{
		AudioSys->StopAllAmbient(FadeOutDuration);
	}
}

void UHorrorAudioLibrary::SetHorrorCategoryVolume(const UObject* WorldContextObject, EHorrorAudioCategory Category, float Volume)
{
	UHorrorAudioSubsystem* AudioSys = GetHorrorAudioSubsystem(WorldContextObject);
	if (AudioSys)
	{
		AudioSys->SetCategoryVolume(Category, Volume);
	}
}

float UHorrorAudioLibrary::GetHorrorCategoryVolume(const UObject* WorldContextObject, EHorrorAudioCategory Category)
{
	UHorrorAudioSubsystem* AudioSys = GetHorrorAudioSubsystem(WorldContextObject);
	return AudioSys ? AudioSys->GetCategoryVolume(Category) : 1.0f;
}

FHorrorAudioEventMapping UHorrorAudioLibrary::MakeAudioEventMapping(FGameplayTag EventTag, USoundBase* Sound, float VolumeMultiplier, bool bUse3D, float AttenuationRadius)
{
	FHorrorAudioEventMapping Mapping;
	Mapping.EventTag = EventTag;
	Mapping.Sound = Sound;
	Mapping.VolumeMultiplier = VolumeMultiplier;
	Mapping.bUse3DAttenuation = bUse3D;
	Mapping.AttenuationRadius = AttenuationRadius;
	Mapping.bAttachToSource = false;
	return Mapping;
}

FHorrorAudioZoneConfig UHorrorAudioLibrary::MakeAudioZoneConfig(FName ZoneId, USoundBase* AmbientSound, float Volume, bool bLoop, float FadeIn, float FadeOut)
{
	FHorrorAudioZoneConfig Config;
	Config.ZoneId = ZoneId;
	Config.AmbientSound = AmbientSound;
	Config.AmbientVolume = Volume;
	Config.bLoopAmbient = bLoop;
	Config.FadeInDuration = FadeIn;
	Config.FadeOutDuration = FadeOut;
	return Config;
}
