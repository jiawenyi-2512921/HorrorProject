// Copyright Epic Games, Inc. All Rights Reserved.

#include "NoiseOverlayComponent.h"

UNoiseOverlayComponent::UNoiseOverlayComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UNoiseOverlayComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UNoiseOverlayComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bNoiseEnabled)
	{
		return;
	}

	TimeAccumulator += DeltaTime * NoiseSpeed;

	const float Noise1 = FMath::PerlinNoise1D(TimeAccumulator);
	const float Noise2 = FMath::PerlinNoise1D(TimeAccumulator * 2.3f + 100.0f);
	CurrentNoiseValue = (Noise1 + Noise2 * 0.5f) * NoiseIntensity;
}

void UNoiseOverlayComponent::SetNoiseIntensity(float Intensity)
{
	NoiseIntensity = FMath::Clamp(Intensity, 0.0f, 1.0f);
}

void UNoiseOverlayComponent::SetNoiseSpeed(float Speed)
{
	NoiseSpeed = FMath::Max(Speed, 0.1f);
}

void UNoiseOverlayComponent::EnableNoise(bool bEnable)
{
	bNoiseEnabled = bEnable;
	if (!bEnable)
	{
		CurrentNoiseValue = 0.0f;
	}
}
