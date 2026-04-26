// Copyright Epic Games, Inc. All Rights Reserved.

#include "Audio/HorrorAudioSubsystem.h"
#include "Game/HorrorEventBusSubsystem.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"

void UHorrorAudioSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	InitializeDefaultVolumes();

	if (UHorrorEventBusSubsystem* EventBus = GetWorld()->GetSubsystem<UHorrorEventBusSubsystem>())
	{
		EventBus->GetOnEventPublishedNative().AddUObject(this, &UHorrorAudioSubsystem::OnEventPublished);
	}
}

void UHorrorAudioSubsystem::Deinitialize()
{
	StopAllAmbient(0.0f);

	if (UHorrorEventBusSubsystem* EventBus = GetWorld()->GetSubsystem<UHorrorEventBusSubsystem>())
	{
		EventBus->GetOnEventPublishedNative().RemoveAll(this);
	}

	Super::Deinitialize();
}

void UHorrorAudioSubsystem::InitializeDefaultVolumes()
{
	CategoryVolumes.Add(EHorrorAudioCategory::Ambient, 0.6f);
	CategoryVolumes.Add(EHorrorAudioCategory::Anomaly, 0.8f);
	CategoryVolumes.Add(EHorrorAudioCategory::Site, 0.7f);
	CategoryVolumes.Add(EHorrorAudioCategory::Interaction, 0.75f);
	CategoryVolumes.Add(EHorrorAudioCategory::Escape, 0.9f);
	CategoryVolumes.Add(EHorrorAudioCategory::Music, 0.5f);
}

UAudioComponent* UHorrorAudioSubsystem::PlaySoundAtLocation(USoundBase* Sound, FVector Location, float VolumeMultiplier, float PitchMultiplier)
{
	if (!Sound || !GetWorld())
	{
		return nullptr;
	}

	return UGameplayStatics::SpawnSoundAtLocation(
		GetWorld(),
		Sound,
		Location,
		FRotator::ZeroRotator,
		VolumeMultiplier,
		PitchMultiplier,
		0.0f,
		nullptr,
		nullptr,
		true);
}

UAudioComponent* UHorrorAudioSubsystem::PlaySoundAttached(USoundBase* Sound, USceneComponent* AttachComponent, FName AttachPointName, float VolumeMultiplier)
{
	if (!Sound || !AttachComponent || !GetWorld())
	{
		return nullptr;
	}

	return UGameplayStatics::SpawnSoundAttached(
		Sound,
		AttachComponent,
		AttachPointName,
		FVector::ZeroVector,
		FRotator::ZeroRotator,
		EAttachLocation::KeepRelativeOffset,
		false,
		VolumeMultiplier,
		1.0f,
		0.0f,
		nullptr,
		nullptr,
		true);
}

UAudioComponent* UHorrorAudioSubsystem::PlaySound2D(USoundBase* Sound, float VolumeMultiplier)
{
	if (!Sound || !GetWorld())
	{
		return nullptr;
	}

	return UGameplayStatics::SpawnSound2D(
		GetWorld(),
		Sound,
		VolumeMultiplier,
		1.0f,
		0.0f,
		nullptr,
		true);
}

bool UHorrorAudioSubsystem::PlayEventSound(FGameplayTag EventTag, UObject* SourceObject)
{
	const FHorrorAudioEventMapping* Mapping = EventMappings.Find(EventTag);
	if (!Mapping || !Mapping->Sound)
	{
		return false;
	}

	UAudioComponent* AudioComp = nullptr;

	if (Mapping->bAttachToSource && SourceObject)
	{
		if (AActor* SourceActor = Cast<AActor>(SourceObject))
		{
			AudioComp = PlaySoundAttached(Mapping->Sound, SourceActor->GetRootComponent(), NAME_None, Mapping->VolumeMultiplier);
		}
		else if (USceneComponent* SceneComp = Cast<USceneComponent>(SourceObject))
		{
			AudioComp = PlaySoundAttached(Mapping->Sound, SceneComp, NAME_None, Mapping->VolumeMultiplier);
		}
	}
	else if (Mapping->bUse3DAttenuation && SourceObject)
	{
		FVector Location = FVector::ZeroVector;
		if (AActor* SourceActor = Cast<AActor>(SourceObject))
		{
			Location = SourceActor->GetActorLocation();
		}
		else if (USceneComponent* SceneComp = Cast<USceneComponent>(SourceObject))
		{
			Location = SceneComp->GetComponentLocation();
		}

		AudioComp = PlaySoundAtLocation(Mapping->Sound, Location, Mapping->VolumeMultiplier);
	}
	else
	{
		AudioComp = PlaySound2D(Mapping->Sound, Mapping->VolumeMultiplier);
	}

	return AudioComp != nullptr;
}

bool UHorrorAudioSubsystem::EnterAudioZone(FName ZoneId)
{
	if (ZoneId.IsNone())
	{
		return false;
	}

	const FHorrorAudioZoneConfig* Config = ZoneConfigs.Find(ZoneId);
	if (!Config || !Config->AmbientSound)
	{
		return false;
	}

	if (CurrentAmbientComponent && CurrentAmbientComponent->IsPlaying())
	{
		CurrentAmbientComponent->FadeOut(Config->FadeOutDuration, 0.0f);
	}

	CurrentZoneId = ZoneId;

	CurrentAmbientComponent = UGameplayStatics::SpawnSound2D(
		GetWorld(),
		Config->AmbientSound,
		Config->AmbientVolume,
		1.0f,
		0.0f,
		nullptr,
		Config->bLoopAmbient,
		true);

	if (CurrentAmbientComponent)
	{
		CurrentAmbientComponent->FadeIn(Config->FadeInDuration, Config->AmbientVolume);
		return true;
	}

	return false;
}

bool UHorrorAudioSubsystem::ExitAudioZone(FName ZoneId)
{
	if (CurrentZoneId != ZoneId)
	{
		return false;
	}

	const FHorrorAudioZoneConfig* Config = ZoneConfigs.Find(ZoneId);
	float FadeOutDuration = Config ? Config->FadeOutDuration : 1.5f;

	if (CurrentAmbientComponent && CurrentAmbientComponent->IsPlaying())
	{
		CurrentAmbientComponent->FadeOut(FadeOutDuration, 0.0f);
	}

	CurrentZoneId = NAME_None;
	CurrentAmbientComponent = nullptr;
	return true;
}

void UHorrorAudioSubsystem::StopAllAmbient(float FadeOutDuration)
{
	if (CurrentAmbientComponent && CurrentAmbientComponent->IsPlaying())
	{
		if (FadeOutDuration > 0.0f)
		{
			CurrentAmbientComponent->FadeOut(FadeOutDuration, 0.0f);
		}
		else
		{
			CurrentAmbientComponent->Stop();
		}
	}

	CurrentZoneId = NAME_None;
	CurrentAmbientComponent = nullptr;
}

void UHorrorAudioSubsystem::SetCategoryVolume(EHorrorAudioCategory Category, float Volume)
{
	CategoryVolumes.Add(Category, FMath::Clamp(Volume, 0.0f, 1.0f));
}

float UHorrorAudioSubsystem::GetCategoryVolume(EHorrorAudioCategory Category) const
{
	const float* Volume = CategoryVolumes.Find(Category);
	return Volume ? *Volume : 1.0f;
}

void UHorrorAudioSubsystem::RegisterEventMapping(const FHorrorAudioEventMapping& Mapping)
{
	if (Mapping.EventTag.IsValid() && Mapping.Sound)
	{
		EventMappings.Add(Mapping.EventTag, Mapping);
	}
}

void UHorrorAudioSubsystem::RegisterZoneConfig(const FHorrorAudioZoneConfig& Config)
{
	if (!Config.ZoneId.IsNone() && Config.AmbientSound)
	{
		ZoneConfigs.Add(Config.ZoneId, Config);
	}
}

void UHorrorAudioSubsystem::UnregisterEventMapping(FGameplayTag EventTag)
{
	EventMappings.Remove(EventTag);
}

void UHorrorAudioSubsystem::UnregisterZoneConfig(FName ZoneId)
{
	ZoneConfigs.Remove(ZoneId);
}

void UHorrorAudioSubsystem::OnEventPublished(const FHorrorEventMessage& Message)
{
	PlayEventSound(Message.EventTag, Message.SourceObject);
}
