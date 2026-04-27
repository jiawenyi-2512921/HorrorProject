// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Player/Components/QuantumCameraComponent.h"
#include "VHSEffectComponent.generated.h"

class UCameraComponent;
class UMaterialInterface;
class UMaterialInstanceDynamic;
class UVHSNoiseGenerator;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FBodycamAcquiredChangedSignature, bool, bIsAcquired);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FBodycamEnabledChangedSignature, bool, bIsEnabled);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FVHSFeedbackChangedSignature, bool, bIsActive, EQuantumCameraMode, CameraMode);

/**
 * Adds VHSEffect Component behavior to its owning actor in the Player module.
 */
UCLASS(ClassGroup=(Horror), BlueprintType, Blueprintable, meta=(BlueprintSpawnableComponent))
class HORRORPROJECT_API UVHSEffectComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UVHSEffectComponent();

	UFUNCTION(BlueprintCallable, Category="VHS|Bodycam")
	bool SetBodycamAcquired(bool bNewAcquired);

	UFUNCTION(BlueprintPure, Category="VHS|Bodycam")
	bool IsBodycamAcquired() const;

	UFUNCTION(BlueprintCallable, Category="VHS|Bodycam")
	bool SetBodycamEnabled(bool bNewEnabled);

	UFUNCTION(BlueprintPure, Category="VHS|Bodycam")
	bool IsBodycamEnabled() const;

	UFUNCTION(BlueprintCallable, Category="VHS|Feedback")
	bool SetFeedbackCameraMode(EQuantumCameraMode NewMode);

	UFUNCTION(BlueprintPure, Category="VHS|Feedback")
	bool IsFeedbackActive() const;

	UFUNCTION(BlueprintPure, Category="VHS|Feedback")
	EQuantumCameraMode GetFeedbackCameraMode() const;

	UFUNCTION(BlueprintCallable, Category="VHS|PostProcess")
	bool BindPostProcessCamera(UCameraComponent* CameraComponent);

	UFUNCTION(BlueprintCallable, Category="VHS|PostProcess")
	bool ResolveDefaultPostProcessMaterial();

	UFUNCTION(BlueprintCallable, Category="VHS|PostProcess")
	void RefreshPostProcessBinding();

	UFUNCTION(BlueprintPure, Category="VHS|PostProcess")
	UCameraComponent* GetBoundPostProcessCamera() const;

	UFUNCTION(BlueprintCallable, Category="VHS|Noise")
	void UpdateNoiseGenerator(float DeltaTime, float StressLevel, float BatteryLevel);

	UFUNCTION(BlueprintCallable, Category="VHS|Noise")
	UVHSNoiseGenerator* GetNoiseGenerator() const;

	UFUNCTION(BlueprintCallable, Category="VHS|Noise")
	void InitializeNoiseGenerator();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="VHS|PostProcess")
	TObjectPtr<UMaterialInterface> VHSPostProcessMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="VHS|PostProcess", meta=(ClampMin="0.0", ClampMax="1.0"))
	float VHSPostProcessBlendWeight = 1.0f;

	UPROPERTY(BlueprintAssignable, Category="VHS|Bodycam")
	FBodycamAcquiredChangedSignature OnBodycamAcquiredChanged;

	UPROPERTY(BlueprintAssignable, Category="VHS|Bodycam")
	FBodycamEnabledChangedSignature OnBodycamEnabledChanged;

	UPROPERTY(BlueprintAssignable, Category="VHS|Feedback")
	FVHSFeedbackChangedSignature OnVHSFeedbackChanged;

private:
	void UpdateFeedbackState(EQuantumCameraMode NewMode);
	void ApplyPostProcessBlendWeight();
	void CreateDynamicMaterialInstance();

	UPROPERTY(Transient)
	TObjectPtr<UCameraComponent> BoundPostProcessCamera;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> DynamicPostProcessMaterial;

	UPROPERTY(Transient)
	TObjectPtr<UVHSNoiseGenerator> NoiseGenerator;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="VHS|Bodycam", meta=(AllowPrivateAccess="true"))
	bool bBodycamAcquired = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="VHS|Bodycam", meta=(AllowPrivateAccess="true"))
	bool bBodycamEnabled = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="VHS|Feedback", meta=(AllowPrivateAccess="true"))
	bool bFeedbackActive = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="VHS|Feedback", meta=(AllowPrivateAccess="true"))
	EQuantumCameraMode FeedbackCameraMode = EQuantumCameraMode::Disabled;
};
