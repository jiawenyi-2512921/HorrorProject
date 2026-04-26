// Copyright Epic Games, Inc. All Rights Reserved.

#include "VHSEffectComponent.h"

UVHSEffectComponent::UVHSEffectComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UVHSEffectComponent::BeginPlay()
{
	Super::BeginPlay();
	CurrentIntensity = bEffectEnabled ? BaseIntensity : 0.0f;
}

void UVHSEffectComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	TimeAccumulator += DeltaTime;

	if (GlitchTimer > 0.0f)
	{
		GlitchTimer -= DeltaTime;
		const float GlitchProgress = 1.0f - (GlitchTimer / GlitchDuration);
		const float GlitchCurve = FMath::Sin(GlitchProgress * PI);
		CurrentIntensity = BaseIntensity + (GlitchIntensity * GlitchCurve);

		if (GlitchTimer <= 0.0f)
		{
			GlitchTimer = 0.0f;
			CurrentIntensity = BaseIntensity;
		}
	}
}

void UVHSEffectComponent::SetEffectIntensity(float Intensity)
{
	BaseIntensity = FMath::Clamp(Intensity, 0.0f, 1.0f);
	if (GlitchTimer <= 0.0f)
	{
		CurrentIntensity = bEffectEnabled ? BaseIntensity : 0.0f;
	}
}

void UVHSEffectComponent::EnableEffect(bool bEnable)
{
	if (bEffectEnabled != bEnable)
	{
		bEffectEnabled = bEnable;
		SetComponentTickEnabled(bEnable);
		CurrentIntensity = bEnable ? BaseIntensity : 0.0f;
	}
}

void UVHSEffectComponent::TriggerGlitch(float Duration, float Intensity)
{
	if (!bEffectEnabled)
	{
		return;
	}

	GlitchDuration = FMath::Max(Duration, 0.1f);
	GlitchTimer = GlitchDuration;
	GlitchIntensity = FMath::Clamp(Intensity, 0.0f, 1.0f);
}
