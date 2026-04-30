// Copyright HorrorProject. All Rights Reserved.

#include "Player/Components/SanityComponent.h"
#include "Components/PostProcessComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Sound/SoundBase.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

USanityComponent::USanityComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
}

void USanityComponent::BeginPlay()
{
	Super::BeginPlay();

	SanityValue = MaxSanity;
	CurrentLevel = ESanityLevel::Stable;
	HallucinationCooldown = FMath::RandRange(HallucinationCooldownMin, HallucinationCooldownMax);
}

void USanityComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	UpdateSanityDrain(DeltaTime);
	UpdateNaturalRecovery(DeltaTime);
	UpdateHallucinationTimer(DeltaTime);
	UpdatePostProcessEffects(DeltaTime);
}

void USanityComponent::DrainSanity(float Amount, FName SourceId)
{
	if (Amount <= 0.0f)
	{
		return;
	}

	const float OldSanity = SanityValue;
	SanityValue = FMath::Max(0.0f, SanityValue - Amount);

	if (!FMath::IsNearlyEqual(SanityValue, OldSanity))
	{
		CheckLevelTransition();

		if (SanityValue <= HallucinationStartThreshold * MaxSanity && !bHallucinationActive)
		{
			TriggerRandomHallucination();
		}
	}
}

void USanityComponent::RecoverSanity(float Amount)
{
	if (Amount <= 0.0f)
	{
		return;
	}

	const float OldSanity = SanityValue;
	SanityValue = FMath::Min(MaxSanity, SanityValue + Amount);

	if (!FMath::IsNearlyEqual(SanityValue, OldSanity))
	{
		OnSanityRecovered.Broadcast(SanityValue - OldSanity);
		CheckLevelTransition();
	}
}

void USanityComponent::SetSanity(float NewValue)
{
	const float OldSanity = SanityValue;
	SanityValue = FMath::Clamp(NewValue, 0.0f, MaxSanity);

	if (!FMath::IsNearlyEqual(SanityValue, OldSanity))
	{
		CheckLevelTransition();
	}
}

float USanityComponent::GetSanityPercent() const
{
	return MaxSanity > 0.0f ? FMath::Clamp(SanityValue / MaxSanity, 0.0f, 1.0f) : 0.0f;
}

float USanityComponent::GetHallucinationIntensity() const
{
	if (!bHallucinationActive)
	{
		return 0.0f;
	}

	return CurrentIntensity;
}

void USanityComponent::TriggerHallucination(const FHallucinationEvent& Event)
{
	if (Event.Type == EHallucinationType::None)
	{
		return;
	}

	bHallucinationActive = true;
	ActiveHallucination = Event.Type;
	ActiveHallucinationTimer = Event.Duration;
	CurrentIntensity = Event.Intensity;

	if (Event.AssociatedSound)
	{
		UGameplayStatics::PlaySoundAtLocation(GetOwner(), Event.AssociatedSound, GetOwner()->GetActorLocation());
	}

	OnHallucinationTriggered.Broadcast(Event.Type, Event.Intensity);
}

void USanityComponent::SetIsolationFactor(float Factor)
{
	IsolationFactor = FMath::Clamp(Factor, 0.0f, 1.0f);
}

void USanityComponent::SetDarknessFactor(float Factor)
{
	DarknessFactor = FMath::Clamp(Factor, 0.0f, 1.0f);
}

void USanityComponent::SetThreatProximity(float DistanceCm)
{
	ThreatProximityCm = DistanceCm;
}

void USanityComponent::UpdateSanityDrain(float DeltaTime)
{
	const float DrainRate = CalculateDrainRate();
	if (DrainRate > 0.0f)
	{
		DrainSanity(DrainRate * DeltaTime, NAME_None);
	}
}

void USanityComponent::UpdateNaturalRecovery(float DeltaTime)
{
	if (SanityValue >= MaxSanity)
	{
		return;
	}

	const float RecoveryMultiplier = bInSafeZone ? SafeZoneRecoveryMultiplier : 1.0f;
	const float DrainRate = CalculateDrainRate();

	if (DrainRate <= 0.0f)
	{
		RecoverSanity(NaturalRecoveryRate * RecoveryMultiplier * DeltaTime);
	}
}

void USanityComponent::UpdateHallucinationTimer(float DeltaTime)
{
	if (bHallucinationActive)
	{
		ActiveHallucinationTimer -= DeltaTime;
		if (ActiveHallucinationTimer <= 0.0f)
		{
			bHallucinationActive = false;
			ActiveHallucination = EHallucinationType::None;
			CurrentIntensity = 0.0f;
			HallucinationCooldown = FMath::RandRange(HallucinationCooldownMin, HallucinationCooldownMax);
		}
	}
	else if (SanityValue <= HallucinationStartThreshold * MaxSanity)
	{
		HallucinationCooldown -= DeltaTime;
		if (HallucinationCooldown <= 0.0f)
		{
			TriggerRandomHallucination();
		}
	}
}

void USanityComponent::UpdatePostProcessEffects(float DeltaTime)
{
	const float SanityPercent = GetSanityPercent();
	const float TargetIntensity = (1.0f - SanityPercent) * MaxPostProcessIntensity;
	PostProcessIntensity = FMath::FInterpTo(PostProcessIntensity, TargetIntensity, DeltaTime, 2.0f);
}

void USanityComponent::CheckLevelTransition()
{
	const float Percent = GetSanityPercent();
	ESanityLevel NewLevel = ESanityLevel::Stable;

	if (Percent <= ShatteredThreshold)
	{
		NewLevel = ESanityLevel::Shattered;
	}
	else if (Percent <= BreakingThreshold)
	{
		NewLevel = ESanityLevel::Breaking;
	}
	else if (Percent <= DisturbedThreshold)
	{
		NewLevel = ESanityLevel::Disturbed;
	}
	else if (Percent <= UneasyThreshold)
	{
		NewLevel = ESanityLevel::Uneasy;
	}

	if (NewLevel != CurrentLevel)
	{
		CurrentLevel = NewLevel;
		OnSanityLevelChanged.Broadcast(CurrentLevel, Percent);
	}
}

void USanityComponent::TriggerRandomHallucination()
{
	if (HallucinationPool.Num() == 0)
	{
		return;
	}

	const int32 Index = FMath::RandRange(0, HallucinationPool.Num() - 1);
	TriggerHallucination(HallucinationPool[Index]);
}

float USanityComponent::CalculateDrainRate() const
{
	float TotalDrain = 0.0f;

	TotalDrain += DarknessFactor * DarknessDrainRate;
	TotalDrain += IsolationFactor * IsolationDrainRate;

	if (ThreatProximityCm < NearThreatDistanceCm)
	{
		const float ThreatFactor = 1.0f - (ThreatProximityCm / NearThreatDistanceCm);
		TotalDrain += ThreatFactor * ThreatDrainRate;
	}

	return TotalDrain;
}

float USanityComponent::GetLowestFactor() const
{
	return FMath::Min(DarknessFactor, IsolationFactor);
}
