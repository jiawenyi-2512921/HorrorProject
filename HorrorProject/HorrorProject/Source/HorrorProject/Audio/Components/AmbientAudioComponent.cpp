// Copyright Epic Games, Inc. All Rights Reserved.

#include "Audio/Components/AmbientAudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"

UAmbientAudioComponent::UAmbientAudioComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
	bAutoActivate = false;

	// Memory optimization: Pre-allocate layer components
	LayerComponents.Reserve(4);
}

void UAmbientAudioComponent::BeginPlay()
{
	Super::BeginPlay();

	if (bAutoStart)
	{
		StartAmbient();
	}
}

void UAmbientAudioComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bIsAmbientPlaying)
	{
		return;
	}

	switch (AmbientType)
	{
	case EAmbientAudioType::Layered:
		UpdateLayeredAudio(DeltaTime);
		break;
	case EAmbientAudioType::Randomized:
		UpdateRandomizedAudio(DeltaTime);
		break;
	case EAmbientAudioType::Dynamic:
		UpdateDynamicAudio(DeltaTime);
		break;
	default:
		break;
	}
}

void UAmbientAudioComponent::StartAmbient()
{
	if (bIsAmbientPlaying)
	{
		return;
	}

	bIsAmbientPlaying = true;

	if (AmbientType == EAmbientAudioType::Static && AudioLayers.Num() > 0)
	{
		SetSound(AudioLayers[0].Sound);
		FadeIn(AudioLayers[0].FadeInTime, AudioLayers[0].BaseVolume);
		Play();
	}
	else if (AmbientType == EAmbientAudioType::Layered)
	{
		for (int32 i = 0; i < AudioLayers.Num(); ++i)
		{
			const FAmbientAudioLayer& Layer = AudioLayers[i];
			if (!Layer.Sound)
			{
				continue;
			}

			UAudioComponent* LayerComp = UGameplayStatics::SpawnSoundAttached(
				Layer.Sound,
				this,
				NAME_None,
				FVector::ZeroVector,
				EAttachLocation::KeepRelativeOffset,
				false,
				Layer.BaseVolume,
				1.0f,
				0.0f,
				nullptr,
				nullptr,
				true
			);

			if (LayerComp)
			{
				LayerComp->FadeIn(Layer.FadeInTime, Layer.BaseVolume);
				LayerComponents.Add(LayerComp);
			}
		}
	}
}

void UAmbientAudioComponent::StopAmbient(float FadeOutDuration)
{
	if (!bIsAmbientPlaying)
	{
		return;
	}

	bIsAmbientPlaying = false;

	if (AmbientType == EAmbientAudioType::Static)
	{
		FadeOut(FadeOutDuration, 0.0f);
	}
	else
	{
		for (UAudioComponent* LayerComp : LayerComponents)
		{
			if (LayerComp && LayerComp->IsPlaying())
			{
				LayerComp->FadeOut(FadeOutDuration, 0.0f);
			}
		}
		LayerComponents.Empty();
	}
}

void UAmbientAudioComponent::AddLayer(const FAmbientAudioLayer& Layer)
{
	AudioLayers.Add(Layer);

	if (bIsAmbientPlaying && AmbientType == EAmbientAudioType::Layered && Layer.Sound)
	{
		UAudioComponent* LayerComp = UGameplayStatics::SpawnSoundAttached(
			Layer.Sound,
			this,
			NAME_None,
			FVector::ZeroVector,
			EAttachLocation::KeepRelativeOffset,
			false,
			Layer.BaseVolume,
			1.0f,
			0.0f,
			nullptr,
			nullptr,
			true
		);

		if (LayerComp)
		{
			LayerComp->FadeIn(Layer.FadeInTime, Layer.BaseVolume);
			LayerComponents.Add(LayerComp);
		}
	}
}

void UAmbientAudioComponent::RemoveLayer(int32 LayerIndex)
{
	if (!AudioLayers.IsValidIndex(LayerIndex))
	{
		return;
	}

	if (bIsAmbientPlaying && LayerComponents.IsValidIndex(LayerIndex))
	{
		UAudioComponent* LayerComp = LayerComponents[LayerIndex];
		if (LayerComp && LayerComp->IsPlaying())
		{
			LayerComp->FadeOut(AudioLayers[LayerIndex].FadeOutTime, 0.0f);
		}
		LayerComponents.RemoveAt(LayerIndex);
	}

	AudioLayers.RemoveAt(LayerIndex);
}

void UAmbientAudioComponent::SetLayerVolume(int32 LayerIndex, float Volume, float FadeTime)
{
	if (!AudioLayers.IsValidIndex(LayerIndex) || !LayerComponents.IsValidIndex(LayerIndex))
	{
		return;
	}

	UAudioComponent* LayerComp = LayerComponents[LayerIndex];
	if (LayerComp && LayerComp->IsPlaying())
	{
		if (FadeTime > 0.0f)
		{
			LayerComp->FadeOut(FadeTime, Volume);
			LayerComp->FadeIn(FadeTime, Volume);
		}
		else
		{
			LayerComp->SetVolumeMultiplier(Volume);
		}
	}
}

void UAmbientAudioComponent::SetTimeOfDayIntensity(float Intensity)
{
	CurrentTimeOfDayIntensity = FMath::Clamp(Intensity, 0.0f, 1.0f);
}

void UAmbientAudioComponent::UpdateLayeredAudio(float DeltaTime)
{
	if (!bUseTimeOfDay)
	{
		return;
	}

	float TimeMultiplier = CalculateTimeOfDayMultiplier();

	for (int32 i = 0; i < LayerComponents.Num(); ++i)
	{
		if (LayerComponents[i] && AudioLayers.IsValidIndex(i))
		{
			float TargetVolume = AudioLayers[i].BaseVolume * TimeMultiplier * CurrentTimeOfDayIntensity;
			LayerComponents[i]->SetVolumeMultiplier(TargetVolume);
		}
	}
}

void UAmbientAudioComponent::UpdateRandomizedAudio(float DeltaTime)
{
	TimeSinceLastRandomization += DeltaTime;

	if (TimeSinceLastRandomization >= RandomizationInterval && AudioLayers.Num() > 0)
	{
		TimeSinceLastRandomization = 0.0f;

		int32 RandomIndex = FMath::RandRange(0, AudioLayers.Num() - 1);
		const FAmbientAudioLayer& Layer = AudioLayers[RandomIndex];

		if (Layer.Sound)
		{
			float Volume = Layer.BaseVolume;
			if (Layer.bRandomizeVolume)
			{
				Volume += FMath::RandRange(-Layer.VolumeVariation, Layer.VolumeVariation);
				Volume = FMath::Clamp(Volume, 0.0f, 1.0f);
			}

			SetSound(Layer.Sound);
			SetVolumeMultiplier(Volume);
			Play();
		}
	}
}

void UAmbientAudioComponent::UpdateDynamicAudio(float DeltaTime)
{
	if (!GetWorld() || !GetWorld()->GetFirstPlayerController())
	{
		return;
	}

	APawn* PlayerPawn = GetWorld()->GetFirstPlayerController()->GetPawn();
	if (!PlayerPawn)
	{
		return;
	}

	float Distance = FVector::Dist(GetComponentLocation(), PlayerPawn->GetActorLocation());
	float DistanceRatio = FMath::Clamp(1.0f - (Distance / MaxDistance), 0.0f, 1.0f);

	if (AudioLayers.Num() > 0)
	{
		float TargetVolume = AudioLayers[0].BaseVolume * DistanceRatio;
		SetVolumeMultiplier(TargetVolume);
	}
}

float UAmbientAudioComponent::CalculateTimeOfDayMultiplier() const
{
	if (!GetWorld())
	{
		return 1.0f;
	}

	float CurrentHour = GetWorld()->GetTimeSeconds() / 3600.0f;
	CurrentHour = FMath::Fmod(CurrentHour, 24.0f);

	float HourDifference = FMath::Abs(CurrentHour - PeakHour);
	if (HourDifference > 12.0f)
	{
		HourDifference = 24.0f - HourDifference;
	}

	return FMath::Clamp(1.0f - (HourDifference / 12.0f), 0.2f, 1.0f);
}
