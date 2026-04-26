// Copyright Epic Games, Inc. All Rights Reserved.

#include "VHSVisualEffectComponent.h"

UVHSVisualEffectComponent::UVHSVisualEffectComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UVHSVisualEffectComponent::BeginPlay()
{
	Super::BeginPlay();
	CurrentIntensity = bEffectEnabled ? BaseIntensity : 0.0f;
}

void UVHSVisualEffectComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
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

void UVHSVisualEffectComponent::SetEffectIntensity(float Intensity)
{
	BaseIntensity = FMath::Clamp(Intensity, 0.0f, 1.0f);
	if (GlitchTimer <= 0.0f)
	{
		CurrentIntensity = bEffectEnabled ? BaseIntensity : 0.0f;
	}
}

void UVHSVisualEffectComponent::EnableEffect(bool bEnable)
{
	if (bEffectEnabled != bEnable)
	{
		bEffectEnabled = bEnable;
		SetComponentTickEnabled(bEnable);
		CurrentIntensity = bEnable ? BaseIntensity : 0.0f;
	}
}

void UVHSVisualEffectComponent::TriggerGlitch(float Duration, float Intensity)
{
	if (!bEffectEnabled)
	{
		return;
	}

	GlitchDuration = FMath::Max(Duration, 0.1f);
	GlitchTimer = GlitchDuration;
	GlitchIntensity = FMath::Clamp(Intensity, 0.0f, 1.0f);
}
