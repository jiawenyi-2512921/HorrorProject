// Copyright Epic Games, Inc. All Rights Reserved.

#include "Audio/HorrorAudioZoneActor.h"
#include "Audio/HorrorAudioZoneTrigger.h"
#include "Audio/HorrorAudioSubsystem.h"
#include "Engine/World.h"

AHorrorAudioZoneActor::AHorrorAudioZoneActor()
{
	PrimaryActorTick.bCanEverTick = false;

	ZoneTrigger = CreateDefaultSubobject<UHorrorAudioZoneTrigger>(TEXT("ZoneTrigger"));
}

void AHorrorAudioZoneActor::BeginPlay()
{
	Super::BeginPlay();

	if (ZoneTrigger)
	{
		ZoneTrigger->ZoneId = ZoneId;
	}

	if (bAutoRegister)
	{
		RegisterZoneConfig();
	}
}

void AHorrorAudioZoneActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UnregisterZoneConfig();
	Super::EndPlay(EndPlayReason);
}

void AHorrorAudioZoneActor::RegisterZoneConfig()
{
	if (UWorld* World = GetWorld())
	{
		if (UHorrorAudioSubsystem* AudioSys = World->GetSubsystem<UHorrorAudioSubsystem>())
		{
			FHorrorAudioZoneConfig Config;
			Config.ZoneId = ZoneId;
			Config.AmbientSound = AmbientSound;
			Config.AmbientVolume = AmbientVolume;
			Config.bLoopAmbient = bLoopAmbient;
			Config.FadeInDuration = FadeInDuration;
			Config.FadeOutDuration = FadeOutDuration;

			AudioSys->RegisterZoneConfig(Config);
		}
	}
}

void AHorrorAudioZoneActor::UnregisterZoneConfig()
{
	if (UWorld* World = GetWorld())
	{
		if (UHorrorAudioSubsystem* AudioSys = World->GetSubsystem<UHorrorAudioSubsystem>())
		{
			AudioSys->UnregisterZoneConfig(ZoneId);
		}
	}
}
