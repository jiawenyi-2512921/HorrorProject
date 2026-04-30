// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/Components/FlashlightComponent.h"
#include "Components/SpotLightComponent.h"
#include "GameFramework/Character.h"

UFlashlightComponent::UFlashlightComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false; // Enable only when flashlight is on
}

void UFlashlightComponent::BeginPlay()
{
	Super::BeginPlay();

	BatterySeconds = StartingBatterySeconds;
	bBatteryDepletedBroadcast = false;

	if (!BoundSpotLight)
	{
		TryAutoBindSpotLight();
	}

	if (BoundSpotLight)
	{
		BaseIntensity = BoundSpotLight->Intensity;
	}

	UpdateLightState();
}

void UFlashlightComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bFlashlightOn && bAutoDepleteBattery)
	{
		UpdateBatteryDrain(DeltaTime);
	}

	if (bFlashlightOn && BoundSpotLight)
	{
		UpdateFlickerEffect(DeltaTime);
	}
}

bool UFlashlightComponent::ToggleFlashlight()
{
	return SetFlashlightEnabled(!bFlashlightOn);
}

bool UFlashlightComponent::SetFlashlightEnabled(bool bEnabled)
{
	if (bEnabled && IsBatteryDepleted())
	{
		return false;
	}

	if (bFlashlightOn == bEnabled)
	{
		return false;
	}

	bFlashlightOn = bEnabled;
	SetComponentTickEnabled(bFlashlightOn); // Only tick when flashlight is active
	UpdateLightState();
	OnFlashlightToggled.Broadcast(bFlashlightOn);

	return true;
}

bool UFlashlightComponent::AddBatteryCharge(float Seconds)
{
	if (Seconds <= 0.0f)
	{
		return false;
	}

	const float OldBattery = BatterySeconds;
	BatterySeconds = FMath::Clamp(BatterySeconds + Seconds, 0.0f, MaxBatterySeconds);

	if (!FMath::IsNearlyEqual(BatterySeconds, OldBattery))
	{
		bBatteryDepletedBroadcast = false;
		OnBatteryChanged.Broadcast(GetBatteryPercent(), BatterySeconds);
		return true;
	}

	return false;
}

float UFlashlightComponent::GetBatteryPercent() const
{
	if (MaxBatterySeconds <= 0.0f)
	{
		return 0.0f;
	}
	return FMath::Clamp(BatterySeconds / MaxBatterySeconds, 0.0f, 1.0f);
}

bool UFlashlightComponent::BindSpotLight(USpotLightComponent* SpotLight)
{
	if (!SpotLight)
	{
		return false;
	}

	BoundSpotLight = SpotLight;
	BaseIntensity = SpotLight->Intensity;
	UpdateLightState();

	return true;
}

void UFlashlightComponent::UpdateLightState()
{
	if (!BoundSpotLight)
	{
		return;
	}

	BoundSpotLight->SetVisibility(bFlashlightOn && !IsBatteryDepleted());
}

void UFlashlightComponent::UpdateBatteryDrain(float DeltaTime)
{
	const float OldBattery = BatterySeconds;
	BatterySeconds = FMath::Max(0.0f, BatterySeconds - (DeltaTime * BatteryDrainRate));

	if (!FMath::IsNearlyEqual(BatterySeconds, OldBattery))
	{
		OnBatteryChanged.Broadcast(GetBatteryPercent(), BatterySeconds);
	}

	if (IsBatteryDepleted())
	{
		if (bAutoDisableOnDepletion && bFlashlightOn)
		{
			SetFlashlightEnabled(false);
		}

		if (!bBatteryDepletedBroadcast)
		{
			bBatteryDepletedBroadcast = true;
			OnBatteryDepleted.Broadcast();
		}
	}
}

void UFlashlightComponent::UpdateFlickerEffect(float DeltaTime)
{
	if (!BoundSpotLight || BaseIntensity <= 0.0f)
	{
		return;
	}

	const float BatteryPercent = GetBatteryPercent();
	if (BatteryPercent > FlickerThresholdPercent)
	{
		BoundSpotLight->SetIntensity(BaseIntensity);
		return;
	}

	FlickerTime = FMath::Fmod(FlickerTime + DeltaTime * FlickerFrequency, 1000.0f);
	const float FlickerNoise = FMath::PerlinNoise1D(FlickerTime);
	const float FlickerAmount = FMath::GetMappedRangeValueClamped(
		FVector2D(-1.0f, 1.0f),
		FVector2D(FlickerIntensityMin, 1.0f),
		FlickerNoise
	);

	const float FlickerIntensity = BaseIntensity * FlickerAmount * (BatteryPercent / FlickerThresholdPercent);
	BoundSpotLight->SetIntensity(FlickerIntensity);
}

bool UFlashlightComponent::TryAutoBindSpotLight()
{
	ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
	if (!OwnerCharacter)
	{
		return false;
	}

	USpotLightComponent* SpotLight = OwnerCharacter->FindComponentByClass<USpotLightComponent>();
	if (!SpotLight)
	{
		return false;
	}

	return BindSpotLight(SpotLight);
}
