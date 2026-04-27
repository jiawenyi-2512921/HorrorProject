// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Camera/CameraShakeBase.h"
#include "ScreenEffectManager.generated.h"

class UCameraShakeBase;
class APlayerCameraManager;

UENUM(BlueprintType)
enum class ECameraShakeType : uint8
{
	Impact UMETA(DisplayName = "Impact Shake"),
	Explosion UMETA(DisplayName = "Explosion Shake"),
	Pressure UMETA(DisplayName = "Pressure Shake"),
	Fear UMETA(DisplayName = "Fear Shake"),
	Drowning UMETA(DisplayName = "Drowning Shake"),
	Death UMETA(DisplayName = "Death Shake")
};

USTRUCT(BlueprintType)
struct FCameraShakeSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Intensity = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Duration = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Frequency = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bOrientToCamera = true;
};

/**
 * Defines Screen Effect Manager behavior for the VFX module.
 */
UCLASS(ClassGroup=(VFX), meta=(BlueprintSpawnableComponent))
class HORRORPROJECT_API UScreenEffectManager : public UActorComponent
{
	GENERATED_BODY()

public:
	UScreenEffectManager();

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	// Apply camera shake
	UFUNCTION(BlueprintCallable, Category = "VFX|Screen")
	void ApplyCameraShake(ECameraShakeType ShakeType, float Intensity = 1.0f);

	// Apply custom camera shake
	UFUNCTION(BlueprintCallable, Category = "VFX|Screen")
	void ApplyCustomCameraShake(const FCameraShakeSettings& Settings);

	// Stop all camera shakes
	UFUNCTION(BlueprintCallable, Category = "VFX|Screen")
	void StopAllCameraShakes();

	// Apply impact feedback
	UFUNCTION(BlueprintCallable, Category = "VFX|Screen")
	void ApplyImpactFeedback(FVector ImpactLocation, float Strength);

	// Apply pressure feedback
	UFUNCTION(BlueprintCallable, Category = "VFX|Screen")
	void ApplyPressureFeedback(float PressureLevel);

	// Apply drowning feedback
	UFUNCTION(BlueprintCallable, Category = "VFX|Screen")
	void ApplyDrowningFeedback(float OxygenPercent);

	// Apply fear feedback
	UFUNCTION(BlueprintCallable, Category = "VFX|Screen")
	void ApplyFearFeedback(float FearLevel);

	// Apply death feedback
	UFUNCTION(BlueprintCallable, Category = "VFX|Screen")
	void ApplyDeathFeedback();

	// Set screen distortion
	UFUNCTION(BlueprintCallable, Category = "VFX|Screen")
	void SetScreenDistortion(float Amount);

	// Set screen blur
	UFUNCTION(BlueprintCallable, Category = "VFX|Screen")
	void SetScreenBlur(float Amount);

	// Clear all screen effects
	UFUNCTION(BlueprintCallable, Category = "VFX|Screen")
	void ClearAllScreenEffects();

protected:
	// Camera shake classes
	UPROPERTY(EditAnywhere, Category = "VFX|Screen")
	TMap<ECameraShakeType, TSubclassOf<UCameraShakeBase>> CameraShakeClasses;

	// Camera manager reference
	UPROPERTY()
	TObjectPtr<APlayerCameraManager> CameraManager;

	// Screen effect parameters
	UPROPERTY(EditAnywhere, Category = "VFX|Screen")
	float DistortionAmount = 0.0f;

	UPROPERTY(EditAnywhere, Category = "VFX|Screen")
	float BlurAmount = 0.0f;

	UPROPERTY(EditAnywhere, Category = "VFX|Screen")
	float MaxShakeIntensity = 2.0f;

	UPROPERTY(EditAnywhere, Category = "VFX|Screen")
	bool bEnableScreenEffects = true;

private:
	void UpdateScreenEffects(float DeltaTime);
	void ApplyShakeInternal(TSubclassOf<UCameraShakeBase> ShakeClass, float Scale);

	float CurrentPressure = 0.0f;
	float CurrentFear = 0.0f;
	float CurrentOxygen = 100.0f;
};
