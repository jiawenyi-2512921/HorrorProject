// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PostProcessController.generated.h"

class UMaterialInstanceDynamic;
class APostProcessVolume;

UENUM(BlueprintType)
enum class EPostProcessEffectType : uint8
{
	VHS UMETA(DisplayName = "VHS Effect"),
	Underwater UMETA(DisplayName = "Underwater"),
	PressureVignette UMETA(DisplayName = "Pressure Vignette"),
	LensDroplets UMETA(DisplayName = "Lens Droplets"),
	ChromaticAberration UMETA(DisplayName = "Chromatic Aberration"),
	Scanlines UMETA(DisplayName = "Scanlines"),
	NoiseOverlay UMETA(DisplayName = "Noise Overlay"),
	DeathEffect UMETA(DisplayName = "Death Effect"),
	FearEffect UMETA(DisplayName = "Fear Effect")
};

USTRUCT(BlueprintType)
struct FPostProcessEffectSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Intensity = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FadeSpeed = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAutoFade = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Duration = 0.0f;
};

UCLASS(ClassGroup=(VFX), meta=(BlueprintSpawnableComponent))
class HORRORPROJECT_API UPostProcessController : public UActorComponent
{
	GENERATED_BODY()

public:
	UPostProcessController();

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	// Apply post process effect
	UFUNCTION(BlueprintCallable, Category = "VFX|PostProcess")
	void ApplyEffect(EPostProcessEffectType EffectType, float Intensity = 1.0f, float Duration = 0.0f);

	// Remove post process effect
	UFUNCTION(BlueprintCallable, Category = "VFX|PostProcess")
	void RemoveEffect(EPostProcessEffectType EffectType, bool bFadeOut = true);

	// Set effect intensity
	UFUNCTION(BlueprintCallable, Category = "VFX|PostProcess")
	void SetEffectIntensity(EPostProcessEffectType EffectType, float Intensity);

	// Get effect intensity
	UFUNCTION(BlueprintPure, Category = "VFX|PostProcess")
	float GetEffectIntensity(EPostProcessEffectType EffectType) const;

	// Pressure effect (increases with depth)
	UFUNCTION(BlueprintCallable, Category = "VFX|PostProcess")
	void UpdatePressureEffect(float Depth, float MaxDepth);

	// Drowning effect
	UFUNCTION(BlueprintCallable, Category = "VFX|PostProcess")
	void ApplyDrowningEffect(float OxygenPercent);

	// Impact shake effect
	UFUNCTION(BlueprintCallable, Category = "VFX|PostProcess")
	void ApplyImpactEffect(FVector ImpactLocation, float Strength);

	// Fear effect (increases with fear level)
	UFUNCTION(BlueprintCallable, Category = "VFX|PostProcess")
	void UpdateFearEffect(float FearLevel);

	// Death effect
	UFUNCTION(BlueprintCallable, Category = "VFX|PostProcess")
	void ApplyDeathEffect();

	// Clear all effects
	UFUNCTION(BlueprintCallable, Category = "VFX|PostProcess")
	void ClearAllEffects();

protected:
	// Post process volume reference
	UPROPERTY(EditAnywhere, Category = "VFX|PostProcess")
	TObjectPtr<APostProcessVolume> PostProcessVolume;

	// Material references
	UPROPERTY(EditAnywhere, Category = "VFX|PostProcess")
	TMap<EPostProcessEffectType, TObjectPtr<UMaterialInterface>> EffectMaterials;

	// Dynamic material instances
	UPROPERTY()
	TMap<EPostProcessEffectType, TObjectPtr<UMaterialInstanceDynamic>> DynamicMaterials;

	// Active effects
	UPROPERTY()
	TMap<EPostProcessEffectType, FPostProcessEffectSettings> ActiveEffects;

	// Effect parameters
	UPROPERTY(EditAnywhere, Category = "VFX|PostProcess")
	float VignetteIntensity = 0.0f;

	UPROPERTY(EditAnywhere, Category = "VFX|PostProcess")
	float ChromaticAberrationAmount = 0.0f;

	UPROPERTY(EditAnywhere, Category = "VFX|PostProcess")
	float NoiseAmount = 0.0f;

	UPROPERTY(EditAnywhere, Category = "VFX|PostProcess")
	float ScanlineSpeed = 1.0f;

private:
	void UpdateEffectIntensities(float DeltaTime);
	void CreateDynamicMaterial(EPostProcessEffectType EffectType);
	void UpdateMaterialParameters(EPostProcessEffectType EffectType, float Intensity);

	float CurrentPressure = 0.0f;
	float CurrentFear = 0.0f;
	float DeathEffectTime = 0.0f;
};
