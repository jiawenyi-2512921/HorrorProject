// Copyright Epic Games, Inc. All Rights Reserved.

#include "PostProcessController.h"
#include "Engine/PostProcessVolume.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/CameraComponent.h"

UPostProcessController::UPostProcessController()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PostUpdateWork;
}

void UPostProcessController::BeginPlay()
{
	Super::BeginPlay();

	// Find or create post process volume
	if (!PostProcessVolume)
	{
		TArray<AActor*> FoundVolumes;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), APostProcessVolume::StaticClass(), FoundVolumes);

		if (FoundVolumes.Num() > 0)
		{
			PostProcessVolume = Cast<APostProcessVolume>(FoundVolumes[0]);
		}
	}

	// Create dynamic materials for all effect types
	for (const auto& Pair : EffectMaterials)
	{
		CreateDynamicMaterial(Pair.Key);
	}
}

void UPostProcessController::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	UpdateEffectIntensities(DeltaTime);
}

void UPostProcessController::ApplyEffect(EPostProcessEffectType EffectType, float Intensity, float Duration)
{
	if (!DynamicMaterials.Contains(EffectType))
	{
		CreateDynamicMaterial(EffectType);
	}

	FPostProcessEffectSettings Settings;
	Settings.Intensity = FMath::Clamp(Intensity, 0.0f, 1.0f);
	Settings.Duration = Duration;
	Settings.bAutoFade = Duration > 0.0f;

	ActiveEffects.Add(EffectType, Settings);
	UpdateMaterialParameters(EffectType, Settings.Intensity);
}

void UPostProcessController::RemoveEffect(EPostProcessEffectType EffectType, bool bFadeOut)
{
	if (ActiveEffects.Contains(EffectType))
	{
		if (bFadeOut)
		{
			FPostProcessEffectSettings& Settings = ActiveEffects[EffectType];
			Settings.bAutoFade = true;
			Settings.Duration = 0.5f; // Fade out over 0.5 seconds
		}
		else
		{
			ActiveEffects.Remove(EffectType);
			UpdateMaterialParameters(EffectType, 0.0f);
		}
	}
}

void UPostProcessController::SetEffectIntensity(EPostProcessEffectType EffectType, float Intensity)
{
	if (ActiveEffects.Contains(EffectType))
	{
		ActiveEffects[EffectType].Intensity = FMath::Clamp(Intensity, 0.0f, 1.0f);
		UpdateMaterialParameters(EffectType, ActiveEffects[EffectType].Intensity);
	}
}

float UPostProcessController::GetEffectIntensity(EPostProcessEffectType EffectType) const
{
	if (ActiveEffects.Contains(EffectType))
	{
		return ActiveEffects[EffectType].Intensity;
	}
	return 0.0f;
}

void UPostProcessController::UpdatePressureEffect(float Depth, float MaxDepth)
{
	CurrentPressure = FMath::Clamp(Depth / MaxDepth, 0.0f, 1.0f);

	// Apply vignette based on pressure
	VignetteIntensity = CurrentPressure * 0.8f;
	ApplyEffect(EPostProcessEffectType::PressureVignette, VignetteIntensity);

	// Apply chromatic aberration at high pressure
	if (CurrentPressure > 0.5f)
	{
		ChromaticAberrationAmount = (CurrentPressure - 0.5f) * 2.0f;
		ApplyEffect(EPostProcessEffectType::ChromaticAberration, ChromaticAberrationAmount);
	}
}

void UPostProcessController::ApplyDrowningEffect(float OxygenPercent)
{
	float DrowningIntensity = 1.0f - FMath::Clamp(OxygenPercent / 100.0f, 0.0f, 1.0f);

	// Increase vignette
	ApplyEffect(EPostProcessEffectType::PressureVignette, DrowningIntensity * 0.9f);

	// Add noise overlay
	ApplyEffect(EPostProcessEffectType::NoiseOverlay, DrowningIntensity * 0.6f);

	// Add chromatic aberration
	ApplyEffect(EPostProcessEffectType::ChromaticAberration, DrowningIntensity * 0.5f);
}

void UPostProcessController::ApplyImpactEffect(FVector ImpactLocation, float Strength)
{
	// Apply chromatic aberration burst
	ApplyEffect(EPostProcessEffectType::ChromaticAberration, Strength, 0.3f);

	// Apply noise overlay
	ApplyEffect(EPostProcessEffectType::NoiseOverlay, Strength * 0.5f, 0.2f);
}

void UPostProcessController::UpdateFearEffect(float FearLevel)
{
	CurrentFear = FMath::Clamp(FearLevel, 0.0f, 1.0f);

	// Apply fear effects
	ApplyEffect(EPostProcessEffectType::FearEffect, CurrentFear);
	ApplyEffect(EPostProcessEffectType::NoiseOverlay, CurrentFear * 0.3f);
	ApplyEffect(EPostProcessEffectType::Scanlines, CurrentFear * 0.4f);
}

void UPostProcessController::ApplyDeathEffect()
{
	DeathEffectTime = 0.0f;
	ApplyEffect(EPostProcessEffectType::DeathEffect, 1.0f, 3.0f);
	ApplyEffect(EPostProcessEffectType::NoiseOverlay, 1.0f, 3.0f);
}

void UPostProcessController::ClearAllEffects()
{
	for (auto& Pair : ActiveEffects)
	{
		UpdateMaterialParameters(Pair.Key, 0.0f);
	}
	ActiveEffects.Empty();

	CurrentPressure = 0.0f;
	CurrentFear = 0.0f;
	DeathEffectTime = 0.0f;
}

void UPostProcessController::UpdateEffectIntensities(float DeltaTime)
{
	TArray<EPostProcessEffectType> EffectsToRemove;

	for (auto& Pair : ActiveEffects)
	{
		FPostProcessEffectSettings& Settings = Pair.Value;

		if (Settings.bAutoFade && Settings.Duration > 0.0f)
		{
			Settings.Duration -= DeltaTime;

			if (Settings.Duration <= 0.0f)
			{
				EffectsToRemove.Add(Pair.Key);
			}
			else
			{
				// Fade out
				float FadeAmount = DeltaTime * Settings.FadeSpeed;
				Settings.Intensity = FMath::Max(0.0f, Settings.Intensity - FadeAmount);
				UpdateMaterialParameters(Pair.Key, Settings.Intensity);
			}
		}
	}

	// Remove completed effects
	for (EPostProcessEffectType EffectType : EffectsToRemove)
	{
		UpdateMaterialParameters(EffectType, 0.0f);
		ActiveEffects.Remove(EffectType);
	}
}

void UPostProcessController::CreateDynamicMaterial(EPostProcessEffectType EffectType)
{
	if (EffectMaterials.Contains(EffectType) && EffectMaterials[EffectType])
	{
		UMaterialInstanceDynamic* DynMat = UMaterialInstanceDynamic::Create(EffectMaterials[EffectType], this);
		if (DynMat)
		{
			DynamicMaterials.Add(EffectType, DynMat);
		}
	}
}

void UPostProcessController::UpdateMaterialParameters(EPostProcessEffectType EffectType, float Intensity)
{
	if (!DynamicMaterials.Contains(EffectType))
	{
		return;
	}

	UMaterialInstanceDynamic* DynMat = DynamicMaterials[EffectType];
	if (!DynMat)
	{
		return;
	}

	// Update common parameters
	DynMat->SetScalarParameterValue(FName("Intensity"), Intensity);
	DynMat->SetScalarParameterValue(FName("Time"), GetWorld()->GetTimeSeconds());

	// Update specific parameters based on effect type
	switch (EffectType)
	{
	case EPostProcessEffectType::PressureVignette:
		DynMat->SetScalarParameterValue(FName("VignetteIntensity"), Intensity);
		DynMat->SetScalarParameterValue(FName("VignettePower"), 2.0f + Intensity * 2.0f);
		break;

	case EPostProcessEffectType::ChromaticAberration:
		DynMat->SetScalarParameterValue(FName("AberrationAmount"), Intensity * 5.0f);
		break;

	case EPostProcessEffectType::NoiseOverlay:
		DynMat->SetScalarParameterValue(FName("NoiseAmount"), Intensity);
		DynMat->SetScalarParameterValue(FName("NoiseScale"), 100.0f);
		break;

	case EPostProcessEffectType::Scanlines:
		DynMat->SetScalarParameterValue(FName("ScanlineSpeed"), ScanlineSpeed);
		DynMat->SetScalarParameterValue(FName("ScanlineCount"), 200.0f);
		break;

	case EPostProcessEffectType::LensDroplets:
		DynMat->SetScalarParameterValue(FName("DropletAmount"), Intensity);
		break;

	case EPostProcessEffectType::FearEffect:
		DynMat->SetScalarParameterValue(FName("DistortionAmount"), Intensity * 0.1f);
		DynMat->SetScalarParameterValue(FName("ColorShift"), Intensity);
		break;

	case EPostProcessEffectType::DeathEffect:
		DynMat->SetScalarParameterValue(FName("Desaturation"), Intensity);
		DynMat->SetScalarParameterValue(FName("Darkness"), Intensity);
		break;

	default:
		break;
	}
}
