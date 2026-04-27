// Copyright Epic Games, Inc. All Rights Reserved.

#include "PostProcessController.h"
#include "Engine/PostProcessVolume.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/CameraComponent.h"

namespace
{
	constexpr float PostProcessScanlineCount = 200.0f;
}

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
		UWorld* World = GetWorld();
		if (!World)
		{
			UE_LOG(LogTemp, Warning, TEXT("PostProcessController: World is null in BeginPlay"));
			return;
		}

		TArray<AActor*> FoundVolumes;
		UGameplayStatics::GetAllActorsOfClass(World, APostProcessVolume::StaticClass(), FoundVolumes);

		for (AActor* FoundVolume : FoundVolumes)
		{
			PostProcessVolume = Cast<APostProcessVolume>(FoundVolume);
			if (PostProcessVolume)
			{
				break;
			}
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
		if (!CreateDynamicMaterial(EffectType))
		{
			UE_LOG(LogTemp, Warning, TEXT("PostProcessController: No material available for effect %d"), static_cast<int32>(EffectType));
			return;
		}
	}

	FPostProcessEffectSettings Settings;
	Settings.Intensity = FMath::Clamp(Intensity, 0.0f, 1.0f);
	Settings.Duration = Duration;
	Settings.bAutoFade = Duration > 0.0f;

	ActiveEffects.Add(EffectType, Settings);
	if (!UpdateMaterialParameters(EffectType, Settings.Intensity))
	{
		ActiveEffects.Remove(EffectType);
	}
}

void UPostProcessController::RemoveEffect(EPostProcessEffectType EffectType, bool bFadeOut)
{
	if (ActiveEffects.Contains(EffectType))
	{
		if (bFadeOut)
		{
			if (FPostProcessEffectSettings* Settings = ActiveEffects.Find(EffectType))
			{
				Settings->bAutoFade = true;
				Settings->Duration = 0.5f; // Fade out over 0.5 seconds
			}
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
	if (FPostProcessEffectSettings* Settings = ActiveEffects.Find(EffectType))
	{
		Settings->Intensity = FMath::Clamp(Intensity, 0.0f, 1.0f);
		if (!UpdateMaterialParameters(EffectType, Settings->Intensity))
		{
			ActiveEffects.Remove(EffectType);
		}
	}
}

float UPostProcessController::GetEffectIntensity(EPostProcessEffectType EffectType) const
{
	if (const FPostProcessEffectSettings* Settings = ActiveEffects.Find(EffectType))
	{
		return Settings->Intensity;
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

bool UPostProcessController::CreateDynamicMaterial(EPostProcessEffectType EffectType)
{
	if (TObjectPtr<UMaterialInterface>* MaterialPtr = EffectMaterials.Find(EffectType); MaterialPtr && *MaterialPtr)
	{
		UMaterialInstanceDynamic* DynMat = UMaterialInstanceDynamic::Create(MaterialPtr->Get(), this);
		if (DynMat)
		{
			DynamicMaterials.Add(EffectType, DynMat);
			return true;
		}
	}

	return false;
}

bool UPostProcessController::UpdateMaterialParameters(EPostProcessEffectType EffectType, float Intensity)
{
	UMaterialInstanceDynamic* DynMat = FindDynamicMaterial(EffectType);
	if (!ApplyCommonMaterialParameters(DynMat, Intensity))
	{
		return false;
	}

	ApplyEffectSpecificMaterialParameters(EffectType, DynMat, Intensity);
	return true;
}

UMaterialInstanceDynamic* UPostProcessController::FindDynamicMaterial(EPostProcessEffectType EffectType) const
{
	const TObjectPtr<UMaterialInstanceDynamic>* DynamicMaterialPtr = DynamicMaterials.Find(EffectType);
	return DynamicMaterialPtr && IsValid(DynamicMaterialPtr->Get()) ? DynamicMaterialPtr->Get() : nullptr;
}

bool UPostProcessController::ApplyCommonMaterialParameters(UMaterialInstanceDynamic* DynamicMaterial, float Intensity)
{
	if (!IsValid(DynamicMaterial))
	{
		return false;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Warning, TEXT("PostProcessController: World is null in UpdateMaterialParameters"));
		return false;
	}

	DynamicMaterial->SetScalarParameterValue(FName("Intensity"), Intensity);
	DynamicMaterial->SetScalarParameterValue(FName("Time"), World->GetTimeSeconds());
	return true;
}

void UPostProcessController::ApplyEffectSpecificMaterialParameters(EPostProcessEffectType EffectType, UMaterialInstanceDynamic* DynamicMaterial, float Intensity)
{
	if (!DynamicMaterial)
	{
		return;
	}

	switch (EffectType)
	{
	case EPostProcessEffectType::PressureVignette:
		ApplyPressureMaterialParameters(DynamicMaterial, Intensity);
		break;

	case EPostProcessEffectType::ChromaticAberration:
		DynamicMaterial->SetScalarParameterValue(FName("AberrationAmount"), Intensity * 5.0f);
		break;

	case EPostProcessEffectType::NoiseOverlay:
		ApplyNoiseMaterialParameters(DynamicMaterial, Intensity);
		break;

	case EPostProcessEffectType::Scanlines:
		ApplyScanlineMaterialParameters(DynamicMaterial);
		break;

	case EPostProcessEffectType::LensDroplets:
		DynamicMaterial->SetScalarParameterValue(FName("DropletAmount"), Intensity);
		break;

	case EPostProcessEffectType::FearEffect:
		ApplyFearMaterialParameters(DynamicMaterial, Intensity);
		break;

	case EPostProcessEffectType::DeathEffect:
		ApplyDeathMaterialParameters(DynamicMaterial, Intensity);
		break;

	default:
		break;
	}
}

void UPostProcessController::ApplyPressureMaterialParameters(UMaterialInstanceDynamic* DynamicMaterial, float Intensity) const
{
	DynamicMaterial->SetScalarParameterValue(FName("VignetteIntensity"), Intensity);
	DynamicMaterial->SetScalarParameterValue(FName("VignettePower"), 2.0f + Intensity * 2.0f);
}

void UPostProcessController::ApplyNoiseMaterialParameters(UMaterialInstanceDynamic* DynamicMaterial, float Intensity) const
{
	DynamicMaterial->SetScalarParameterValue(FName("NoiseAmount"), Intensity);
	DynamicMaterial->SetScalarParameterValue(FName("NoiseScale"), 100.0f);
}

void UPostProcessController::ApplyScanlineMaterialParameters(UMaterialInstanceDynamic* DynamicMaterial) const
{
	DynamicMaterial->SetScalarParameterValue(FName("ScanlineSpeed"), ScanlineSpeed);
	DynamicMaterial->SetScalarParameterValue(FName("ScanlineCount"), PostProcessScanlineCount);
}

void UPostProcessController::ApplyFearMaterialParameters(UMaterialInstanceDynamic* DynamicMaterial, float Intensity) const
{
	DynamicMaterial->SetScalarParameterValue(FName("DistortionAmount"), Intensity * 0.1f);
	DynamicMaterial->SetScalarParameterValue(FName("ColorShift"), Intensity);
}

void UPostProcessController::ApplyDeathMaterialParameters(UMaterialInstanceDynamic* DynamicMaterial, float Intensity) const
{
	DynamicMaterial->SetScalarParameterValue(FName("Desaturation"), Intensity);
	DynamicMaterial->SetScalarParameterValue(FName("Darkness"), Intensity);
}
