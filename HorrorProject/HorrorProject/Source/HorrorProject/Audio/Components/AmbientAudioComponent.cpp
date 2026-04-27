// Copyright Epic Games, Inc. All Rights Reserved.

#include "Audio/Components/AmbientAudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"

namespace HorrorAmbientAudio
{
	constexpr float SecondsPerHour = 3600.0f;
	constexpr float HoursPerDay = 24.0f;
	constexpr float HalfDayHours = 12.0f;
	constexpr float MinimumTimeOfDayMultiplier = 0.2f;
}

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

	if (AmbientType == EAmbientAudioType::Static && !AudioLayers.IsEmpty())
	{
		const FAmbientAudioLayer* StaticLayer = AudioLayers.GetData();
		SetSound(StaticLayer->Sound);
		FadeIn(StaticLayer->FadeInTime, StaticLayer->BaseVolume);
		Play();
	}
	else if (AmbientType == EAmbientAudioType::Layered)
	{
		for (const FAmbientAudioLayer& Layer : AudioLayers)
		{
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
		UAudioComponent* LayerComp = LayerComponents.GetData()[LayerIndex];
		if (LayerComp && LayerComp->IsPlaying())
		{
			const FAmbientAudioLayer* Layer = AudioLayers.GetData() + LayerIndex;
			LayerComp->FadeOut(Layer->FadeOutTime, 0.0f);
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

	UAudioComponent* LayerComp = LayerComponents.GetData()[LayerIndex];
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

	const int32 SharedLayerCount = FMath::Min(LayerComponents.Num(), AudioLayers.Num());
	TObjectPtr<UAudioComponent>* LayerComponentCursor = LayerComponents.GetData();
	const FAmbientAudioLayer* AudioLayerCursor = AudioLayers.GetData();
	for (int32 i = 0; i < SharedLayerCount; ++i, ++LayerComponentCursor, ++AudioLayerCursor)
	{
		UAudioComponent* LayerComponent = LayerComponentCursor->Get();
		if (LayerComponent)
		{
			float TargetVolume = AudioLayerCursor->BaseVolume * TimeMultiplier * CurrentTimeOfDayIntensity;
			LayerComponent->SetVolumeMultiplier(TargetVolume);
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
		const FAmbientAudioLayer& Layer = *(AudioLayers.GetData() + RandomIndex);

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
	UWorld* World = GetWorld();
	if (!World || MaxDistance <= KINDA_SMALL_NUMBER)
	{
		return;
	}

	APlayerController* PlayerController = World->GetFirstPlayerController();
	if (!PlayerController)
	{
		return;
	}

	APawn* PlayerPawn = PlayerController->GetPawn();
	if (!PlayerPawn)
	{
		return;
	}

	float Distance = FVector::Dist(GetComponentLocation(), PlayerPawn->GetActorLocation());
	float DistanceRatio = FMath::Clamp(1.0f - (Distance / MaxDistance), 0.0f, 1.0f);

	if (!AudioLayers.IsEmpty())
	{
		const FAmbientAudioLayer* StaticLayer = AudioLayers.GetData();
		float TargetVolume = StaticLayer->BaseVolume * DistanceRatio;
		SetVolumeMultiplier(TargetVolume);
	}
}

float UAmbientAudioComponent::CalculateTimeOfDayMultiplier() const
{
	const UWorld* World = GetWorld();
	if (!World)
	{
		return 1.0f;
	}

	float CurrentHour = World->GetTimeSeconds() / HorrorAmbientAudio::SecondsPerHour;
	CurrentHour = FMath::Fmod(CurrentHour, HorrorAmbientAudio::HoursPerDay);

	float HourDifference = FMath::Abs(CurrentHour - PeakHour);
	if (HourDifference > HorrorAmbientAudio::HalfDayHours)
	{
		HourDifference = HorrorAmbientAudio::HoursPerDay - HourDifference;
	}

	return FMath::Clamp(
		1.0f - (HourDifference / HorrorAmbientAudio::HalfDayHours),
		HorrorAmbientAudio::MinimumTimeOfDayMultiplier,
		1.0f);
}
