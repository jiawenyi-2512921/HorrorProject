// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/Components/FearComponent.h"

UFearComponent::UFearComponent()
{
	// Performance optimization: Use timer-based updates instead of tick
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UFearComponent::BeginPlay()
{
	Super::BeginPlay();
	FearValue = 0.0f;
	CurrentFearLevel = EFearLevel::Calm;
	TimeSinceLastFearIncrease = 0.0f;
}

void UFearComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	StopFearDecayTimer();
	Super::EndPlay(EndPlayReason);
}

void UFearComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	// Performance optimization: Tick disabled, using timer-based updates
}

void UFearComponent::UpdateFearDecayTimer()
{
	if (FearValue <= 0.0f)
	{
		StopFearDecayTimer();
		return;
	}

	const float DeltaTime = 0.1f;
	UpdateFearDecay(DeltaTime);
}

bool UFearComponent::AddFear(float Amount, FName SourceId)
{
	if (Amount <= 0.0f)
	{
		return false;
	}

	const float OldValue = FearValue;
	FearValue = FMath::Clamp(FearValue + Amount, 0.0f, MaxFearValue);
	TimeSinceLastFearIncrease = 0.0f;

	if (FearValue != OldValue)
	{
		OnFearValueChanged.Broadcast(FearValue, GetFearPercent());
		UpdateFearLevel();
		StartFearDecayTimer();
		return true;
	}

	return false;
}

bool UFearComponent::RemoveFear(float Amount)
{
	if (Amount <= 0.0f)
	{
		return false;
	}

	const float OldValue = FearValue;
	FearValue = FMath::Max(0.0f, FearValue - Amount);

	if (FearValue != OldValue)
	{
		OnFearValueChanged.Broadcast(FearValue, GetFearPercent());
		UpdateFearLevel();
		if (FearValue <= 0.0f)
		{
			StopFearDecayTimer();
		}
		return true;
	}

	return false;
}

bool UFearComponent::SetFearValue(float NewValue)
{
	const float ClampedValue = FMath::Clamp(NewValue, 0.0f, MaxFearValue);
	if (FearValue == ClampedValue)
	{
		return false;
	}

	const float OldValue = FearValue;
	FearValue = ClampedValue;
	if (FearValue > OldValue)
	{
		TimeSinceLastFearIncrease = 0.0f;
	}
	OnFearValueChanged.Broadcast(FearValue, GetFearPercent());
	UpdateFearLevel();
	if (FearValue > 0.0f)
	{
		StartFearDecayTimer();
	}
	else
	{
		StopFearDecayTimer();
	}
	return true;
}

float UFearComponent::GetFearPercent() const
{
	if (MaxFearValue <= 0.0f)
	{
		return 0.0f;
	}
	return FMath::Clamp(FearValue / MaxFearValue, 0.0f, 1.0f);
}

float UFearComponent::GetMovementSpeedMultiplier() const
{
	const float FearPercent = GetFearPercent();
	return FMath::Lerp(1.0f, MinSpeedMultiplier, FearPercent);
}

float UFearComponent::GetCameraShakeIntensity() const
{
	const float FearPercent = GetFearPercent();
	return FearPercent * MaxCameraShakeIntensity;
}

float UFearComponent::GetVignetteIntensity() const
{
	const float FearPercent = GetFearPercent();
	return FearPercent * MaxVignetteIntensity;
}

float UFearComponent::GetHeartbeatVolume() const
{
	const float FearPercent = GetFearPercent();
	return FMath::Pow(FearPercent, 2.0f) * MaxHeartbeatVolume;
}

void UFearComponent::UpdateFearDecay(float DeltaTime)
{
	if (FearValue <= 0.0f)
	{
		StopFearDecayTimer();
		return;
	}

	TimeSinceLastFearIncrease += DeltaTime;

	if (TimeSinceLastFearIncrease >= FearDecayDelay)
	{
		const float DecayAmount = FearDecayRate * DeltaTime;
		RemoveFear(DecayAmount);
	}
}

void UFearComponent::UpdateFearLevel()
{
	const EFearLevel NewLevel = CalculateFearLevel();
	if (NewLevel != CurrentFearLevel)
	{
		CurrentFearLevel = NewLevel;
		OnFearLevelChanged.Broadcast(CurrentFearLevel, GetFearPercent());
	}
}

EFearLevel UFearComponent::CalculateFearLevel() const
{
	const float FearPercent = GetFearPercent();

	if (FearPercent >= PanickedThreshold)
	{
		return EFearLevel::Panicked;
	}
	if (FearPercent >= TerrifiedThreshold)
	{
		return EFearLevel::Terrified;
	}
	if (FearPercent >= AfraidThreshold)
	{
		return EFearLevel::Afraid;
	}
	if (FearPercent >= UneasyThreshold)
	{
		return EFearLevel::Uneasy;
	}

	return EFearLevel::Calm;
}

void UFearComponent::StartFearDecayTimer()
{
	UWorld* World = GetWorld();
	if (!World || World->GetTimerManager().IsTimerActive(FearDecayTimerHandle))
	{
		return;
	}

	World->GetTimerManager().SetTimer(FearDecayTimerHandle, this, &UFearComponent::UpdateFearDecayTimer, 0.1f, true);
}

void UFearComponent::StopFearDecayTimer()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(FearDecayTimerHandle);
	}
}
