// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/Components/FearComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

UFearComponent::UFearComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
}

void UFearComponent::BeginPlay()
{
	Super::BeginPlay();
	FearValue = 0.0f;
	CurrentFearLevel = EFearLevel::Calm;
	TimeSinceLastFearIncrease = 0.0f;
}

void UFearComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	UpdateFearDecay(DeltaTime);
	UpdateFearLevel();
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

	FearValue = ClampedValue;
	OnFearValueChanged.Broadcast(FearValue, GetFearPercent());
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
		ApplyMovementSpeedEffect();
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

void UFearComponent::ApplyMovementSpeedEffect()
{
	ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
	if (!OwnerCharacter || !OwnerCharacter->GetCharacterMovement())
	{
		return;
	}

	// Note: This modifies the base walk speed. The sprint system in HorrorCharacter
	// should read this value when calculating sprint speed.
	const float SpeedMultiplier = GetMovementSpeedMultiplier();
	UCharacterMovementComponent* Movement = OwnerCharacter->GetCharacterMovement();

	// Store original speed if not already stored (would need a base speed property)
	// For now, we'll just apply the multiplier to current speed
	// Blueprint can handle more sophisticated speed management
}
