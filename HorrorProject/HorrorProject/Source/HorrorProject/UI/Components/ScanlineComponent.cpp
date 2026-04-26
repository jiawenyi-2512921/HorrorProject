// Copyright Epic Games, Inc. All Rights Reserved.

#include "ScanlineComponent.h"

UScanlineComponent::UScanlineComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UScanlineComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UScanlineComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bScanlinesEnabled)
	{
		return;
	}

	TimeAccumulator += DeltaTime * ScanlineSpeed;
	CurrentOffset = FMath::Fmod(TimeAccumulator, 1.0f);
}

void UScanlineComponent::SetScanlineIntensity(float Intensity)
{
	ScanlineIntensity = FMath::Clamp(Intensity, 0.0f, 1.0f);
}

void UScanlineComponent::SetScanlineSpeed(float Speed)
{
	ScanlineSpeed = FMath::Max(Speed, 0.0f);
}

void UScanlineComponent::EnableScanlines(bool bEnable)
{
	bScanlinesEnabled = bEnable;
	if (!bEnable)
	{
		CurrentOffset = 0.0f;
	}
}
