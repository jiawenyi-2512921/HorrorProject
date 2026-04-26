// Copyright Epic Games, Inc. All Rights Reserved.

#include "Audio/Components/UnderwaterAudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"

UUnderwaterAudioComponent::UUnderwaterAudioComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
	bAutoActivate = false;
}

void UUnderwaterAudioComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UUnderwaterAudioComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bIsUnderwater)
	{
		return;
	}

	UpdateUnderwaterEffects();
	UpdateBubbles(DeltaTime);
	UpdateDrowningSound();
}

void UUnderwaterAudioComponent::EnterWater(float Depth)
{
	if (bIsUnderwater)
	{
		return;
	}

	bIsUnderwater = true;
	CurrentDepth = Depth;

	if (AudioConfig.UnderwaterAmbient && GetWorld())
	{
		AmbientComponent = UGameplayStatics::SpawnSound2D(
			GetWorld(),
			AudioConfig.UnderwaterAmbient,
			AudioConfig.UnderwaterVolume,
			1.0f,
			0.0f,
			nullptr,
			true,
			true
		);

		if (AmbientComponent)
		{
			AmbientComponent->FadeIn(AudioConfig.TransitionTime, AudioConfig.UnderwaterVolume);
		}
	}

	if (AudioConfig.SurfaceBreachSound)
	{
		UGameplayStatics::PlaySound2D(GetWorld(), AudioConfig.SurfaceBreachSound, 0.8f);
	}
}

void UUnderwaterAudioComponent::ExitWater()
{
	if (!bIsUnderwater)
	{
		return;
	}

	bIsUnderwater = false;
	CurrentDepth = 0.0f;

	if (AmbientComponent && AmbientComponent->IsPlaying())
	{
		AmbientComponent->FadeOut(AudioConfig.TransitionTime, 0.0f);
	}

	if (DrowningComponent && DrowningComponent->IsPlaying())
	{
		DrowningComponent->FadeOut(0.5f, 0.0f);
	}

	if (AudioConfig.SurfaceBreachSound)
	{
		UGameplayStatics::PlaySound2D(GetWorld(), AudioConfig.SurfaceBreachSound, 0.8f);
	}
}

void UUnderwaterAudioComponent::UpdateDepth(float Depth)
{
	CurrentDepth = FMath::Clamp(Depth, 0.0f, MaxDepth);
	UpdateUnderwaterEffects();
}

void UUnderwaterAudioComponent::PlayBubbleSound()
{
	if (!AudioConfig.BubbleSound || !GetWorld())
	{
		return;
	}

	float PitchVariation = FMath::RandRange(0.9f, 1.1f);
	float VolumeVariation = FMath::RandRange(0.6f, 1.0f);

	UGameplayStatics::PlaySound2D(
		GetWorld(),
		AudioConfig.BubbleSound,
		VolumeVariation,
		PitchVariation
	);
}

void UUnderwaterAudioComponent::SetOxygenLevel(float OxygenLevel)
{
	CurrentOxygenLevel = FMath::Clamp(OxygenLevel, 0.0f, 1.0f);
}

void UUnderwaterAudioComponent::UpdateUnderwaterEffects()
{
	if (!AmbientComponent)
	{
		return;
	}

	float DepthMultiplier = CalculateDepthMultiplier();
	float TargetVolume = AudioConfig.UnderwaterVolume * DepthMultiplier;

	AmbientComponent->SetVolumeMultiplier(TargetVolume);

	float PitchMultiplier = 1.0f - (DepthMultiplier * DepthPressureMultiplier * 0.2f);
	AmbientComponent->SetPitchMultiplier(FMath::Clamp(PitchMultiplier, 0.8f, 1.0f));
}

void UUnderwaterAudioComponent::UpdateBubbles(float DeltaTime)
{
	TimeSinceLastBubble += DeltaTime;

	if (TimeSinceLastBubble >= NextBubbleInterval)
	{
		PlayBubbleSound();
		TimeSinceLastBubble = 0.0f;
		NextBubbleInterval = BubbleInterval + FMath::RandRange(-BubbleIntervalVariation, BubbleIntervalVariation);
	}
}

void UUnderwaterAudioComponent::UpdateDrowningSound()
{
	if (CurrentOxygenLevel <= OxygenWarningThreshold)
	{
		if (!DrowningComponent && AudioConfig.DrowningSound && GetWorld())
		{
			DrowningComponent = UGameplayStatics::SpawnSound2D(
				GetWorld(),
				AudioConfig.DrowningSound,
				0.0f,
				1.0f,
				0.0f,
				nullptr,
				true,
				true
			);

			if (DrowningComponent)
			{
				float TargetVolume = 1.0f - (CurrentOxygenLevel / OxygenWarningThreshold);
				DrowningComponent->FadeIn(1.0f, TargetVolume);
			}
		}
		else if (DrowningComponent)
		{
			float TargetVolume = 1.0f - (CurrentOxygenLevel / OxygenWarningThreshold);
			DrowningComponent->SetVolumeMultiplier(TargetVolume);
		}
	}
	else if (DrowningComponent && DrowningComponent->IsPlaying())
	{
		DrowningComponent->FadeOut(1.0f, 0.0f);
		DrowningComponent = nullptr;
	}
}

float UUnderwaterAudioComponent::CalculateDepthMultiplier() const
{
	return FMath::Clamp(CurrentDepth / MaxDepth, 0.5f, 1.0f);
}
