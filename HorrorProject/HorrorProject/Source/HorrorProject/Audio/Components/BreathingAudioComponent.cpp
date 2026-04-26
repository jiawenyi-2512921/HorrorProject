// Copyright Epic Games, Inc. All Rights Reserved.

#include "Audio/Components/BreathingAudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"

UBreathingAudioComponent::UBreathingAudioComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
	bAutoActivate = false;
}

void UBreathingAudioComponent::BeginPlay()
{
	Super::BeginPlay();

	if (bAutoStart)
	{
		StartBreathing();
	}
}

void UBreathingAudioComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bIsBreathing || bIsHoldingBreath)
	{
		return;
	}

	if (bUseAutoStateTransition)
	{
		UpdateBreathingState();
	}

	TimeSinceLastBreath += DeltaTime;

	const FBreathingStateConfig* Config = StateConfigs.Find(CurrentState);
	if (Config && TimeSinceLastBreath >= Config->BreathInterval)
	{
		PlayBreathSound();
		TimeSinceLastBreath = 0.0f;
	}
}

void UBreathingAudioComponent::SetBreathingState(EBreathingState NewState)
{
	if (CurrentState != NewState)
	{
		TransitionToState(NewState);
	}
}

void UBreathingAudioComponent::SetFearLevel(float FearLevel)
{
	CurrentFearLevel = FMath::Clamp(FearLevel, 0.0f, 1.0f);
}

void UBreathingAudioComponent::SetStaminaLevel(float StaminaLevel)
{
	CurrentStaminaLevel = FMath::Clamp(StaminaLevel, 0.0f, 1.0f);
}

void UBreathingAudioComponent::StartBreathing()
{
	if (bIsBreathing)
	{
		return;
	}

	bIsBreathing = true;
	bIsHoldingBreath = false;
	TimeSinceLastBreath = 0.0f;

	const FBreathingStateConfig* Config = StateConfigs.Find(CurrentState);
	if (Config && Config->BreathingSound)
	{
		SetSound(Config->BreathingSound);
		SetVolumeMultiplier(Config->Volume);
		SetPitchMultiplier(Config->PitchMultiplier);
		Play();
	}
}

void UBreathingAudioComponent::StopBreathing(float FadeOutTime)
{
	if (!bIsBreathing)
	{
		return;
	}

	bIsBreathing = false;
	FadeOut(FadeOutTime, 0.0f);
}

void UBreathingAudioComponent::HoldBreath()
{
	bIsHoldingBreath = true;
	FadeOut(0.5f, 0.0f);
}

void UBreathingAudioComponent::ReleaseBreath()
{
	if (!bIsHoldingBreath)
	{
		return;
	}

	bIsHoldingBreath = false;

	const FBreathingStateConfig* Config = StateConfigs.Find(EBreathingState::Exhausted);
	if (Config && Config->BreathingSound)
	{
		SetSound(Config->BreathingSound);
		SetVolumeMultiplier(Config->Volume);
		SetPitchMultiplier(Config->PitchMultiplier);
		FadeIn(0.3f, Config->Volume);
		Play();
	}

	TransitionToState(EBreathingState::Exhausted);
}

void UBreathingAudioComponent::RegisterStateConfig(EBreathingState State, const FBreathingStateConfig& Config)
{
	StateConfigs.Add(State, Config);
}

void UBreathingAudioComponent::UpdateBreathingState()
{
	EBreathingState NewState = CalculateAutoState();
	if (NewState != CurrentState)
	{
		TransitionToState(NewState);
	}
}

void UBreathingAudioComponent::PlayBreathSound()
{
	const FBreathingStateConfig* Config = StateConfigs.Find(CurrentState);
	if (!Config || !Config->BreathingSound || !GetWorld())
	{
		return;
	}

	UGameplayStatics::PlaySound2D(
		GetWorld(),
		Config->BreathingSound,
		Config->Volume,
		Config->PitchMultiplier
	);
}

void UBreathingAudioComponent::TransitionToState(EBreathingState NewState)
{
	const FBreathingStateConfig* OldConfig = StateConfigs.Find(CurrentState);
	const FBreathingStateConfig* NewConfig = StateConfigs.Find(NewState);

	if (!NewConfig)
	{
		return;
	}

	float TransitionTime = OldConfig ? OldConfig->TransitionTime : 1.0f;

	CurrentState = NewState;

	if (bIsBreathing && NewConfig->BreathingSound)
	{
		FadeOut(TransitionTime * 0.5f, 0.0f);

		FTimerHandle TransitionTimer;
		GetWorld()->GetTimerManager().SetTimer(TransitionTimer, [this, NewConfig]()
		{
			SetSound(NewConfig->BreathingSound);
			SetVolumeMultiplier(NewConfig->Volume);
			SetPitchMultiplier(NewConfig->PitchMultiplier);
			FadeIn(NewConfig->TransitionTime * 0.5f, NewConfig->Volume);
			Play();
		}, TransitionTime * 0.5f, false);
	}
}

EBreathingState UBreathingAudioComponent::CalculateAutoState() const
{
	if (CurrentFearLevel >= FearThresholdPanicked)
	{
		return EBreathingState::Panicked;
	}

	if (CurrentStaminaLevel <= StaminaThresholdExhausted)
	{
		return EBreathingState::Exhausted;
	}

	if (CurrentStaminaLevel <= StaminaThresholdTired)
	{
		return EBreathingState::Tired;
	}

	if (CurrentFearLevel > 0.3f)
	{
		return EBreathingState::Normal;
	}

	return EBreathingState::Calm;
}
