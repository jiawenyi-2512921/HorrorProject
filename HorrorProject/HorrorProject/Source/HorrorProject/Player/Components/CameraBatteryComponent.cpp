// Copyright Epic Games, Inc. All Rights Reserved.

#include "CameraBatteryComponent.h"
#include "Game/HorrorEventBusSubsystem.h"
#include "Audio/HorrorAudioSubsystem.h"
#include "Engine/World.h"

UCameraBatteryComponent::UCameraBatteryComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;

	BatteryDepletedEventTag = FGameplayTag::RequestGameplayTag(FName("Horror.Camera.Battery.Depleted"));
	BatteryLowEventTag = FGameplayTag::RequestGameplayTag(FName("Horror.Camera.Battery.Low"));
	BatteryChargingEventTag = FGameplayTag::RequestGameplayTag(FName("Horror.Camera.Battery.Charging"));
}

void UCameraBatteryComponent::BeginPlay()
{
	Super::BeginPlay();

	BatteryPercentage = FMath::Clamp(InitialBatteryPercentage, 0.0f, 100.0f);
	bHasTriggeredLowWarning = BatteryPercentage <= LowBatteryThreshold;
	bHasTriggeredDepletion = BatteryPercentage <= 0.0f;
}

void UCameraBatteryComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	UpdateBattery(DeltaTime);
}

void UCameraBatteryComponent::StartRecordingDrain()
{
	if (!bIsRecordingDraining && !IsBatteryDepleted())
	{
		bIsRecordingDraining = true;
	}
}

void UCameraBatteryComponent::StopRecordingDrain()
{
	bIsRecordingDraining = false;
}

void UCameraBatteryComponent::StartFlashlightDrain()
{
	if (!bIsFlashlightDraining && !IsBatteryDepleted())
	{
		bIsFlashlightDraining = true;
	}
}

void UCameraBatteryComponent::StopFlashlightDrain()
{
	bIsFlashlightDraining = false;
}

void UCameraBatteryComponent::StartCharging()
{
	if (!bIsCharging)
	{
		bIsCharging = true;
		OnBatteryChargingChanged.Broadcast();
		PublishEventBusEvent(BatteryChargingEventTag);

		bHasTriggeredLowWarning = false;
		bHasTriggeredDepletion = false;
	}
}

void UCameraBatteryComponent::StopCharging()
{
	if (bIsCharging)
	{
		bIsCharging = false;
		OnBatteryChargingChanged.Broadcast();
	}
}

void UCameraBatteryComponent::SetBatteryPercentage(float NewPercentage)
{
	float ClampedPercentage = FMath::Clamp(NewPercentage, 0.0f, 100.0f);

	if (!FMath::IsNearlyEqual(BatteryPercentage, ClampedPercentage, 0.01f))
	{
		BatteryPercentage = ClampedPercentage;
		BroadcastBatteryChange();
	}
}

void UCameraBatteryComponent::UpdateBattery(float DeltaTime)
{
	if (DeltaTime <= 0.0f)
	{
		return;
	}

	float OldPercentage = BatteryPercentage;

	if (bIsCharging)
	{
		BatteryPercentage = FMath::Min(BatteryPercentage + ChargeRate * DeltaTime, 100.0f);

		if (BatteryPercentage >= 100.0f)
		{
			StopCharging();
		}
	}
	else
	{
		float TotalDrain = 0.0f;

		if (bIsRecordingDraining)
		{
			TotalDrain += RecordingDrainRate;
		}

		if (bIsFlashlightDraining)
		{
			TotalDrain += FlashlightDrainRate;
		}

		if (TotalDrain > 0.0f)
		{
			BatteryPercentage = FMath::Max(BatteryPercentage - TotalDrain * DeltaTime, 0.0f);
		}
	}

	if (!FMath::IsNearlyEqual(OldPercentage, BatteryPercentage, 0.01f))
	{
		BroadcastBatteryChange();
	}

	if (!bHasTriggeredLowWarning && IsBatteryLow() && !bIsCharging)
	{
		bHasTriggeredLowWarning = true;
		OnBatteryLowWarning.Broadcast();
		PublishEventBusEvent(BatteryLowEventTag);
	}

	if (!bHasTriggeredDepletion && IsBatteryDepleted())
	{
		bHasTriggeredDepletion = true;
		bIsRecordingDraining = false;
		bIsFlashlightDraining = false;

		OnBatteryDepleted.Broadcast();
		PublishEventBusEvent(BatteryDepletedEventTag);
	}
}

void UCameraBatteryComponent::BroadcastBatteryChange()
{
	OnBatteryPercentageChanged.Broadcast(BatteryPercentage);
}

void UCameraBatteryComponent::PublishEventBusEvent(FGameplayTag EventTag)
{
	if (!EventTag.IsValid())
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	UHorrorEventBusSubsystem* EventBus = World->GetSubsystem<UHorrorEventBusSubsystem>();
	if (EventBus)
	{
		EventBus->Publish(EventTag, FName("CameraBattery"), FGameplayTag::EmptyTag, this);
	}
}
