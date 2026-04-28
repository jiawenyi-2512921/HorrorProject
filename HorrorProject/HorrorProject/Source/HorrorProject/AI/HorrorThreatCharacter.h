// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "HorrorThreatCharacter.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FHorrorThreatActiveChangedSignature, bool, bIsActive);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FHorrorThreatDetectedTargetChangedSignature, AActor*, DetectedTarget);

namespace HorrorThreatDefaults
{
	inline constexpr float DetectionRadiusCm = 800.0f;
}

UCLASS(MinimalAPI, Hidden, NotBlueprintable, NotPlaceable)
class UHorrorThreatDelegateProbe : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION()
	void HandleThreatActiveChanged(bool bIsActive);

	UFUNCTION()
	void HandleDetectedTargetChanged(AActor* DetectedTarget);

	TArray<bool> ActiveValues;
	TArray<TObjectPtr<AActor>> TargetValues;
};

/**
 * Implements actor-level Horror Threat Character behavior for the AI module.
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup=(Horror))
class HORRORPROJECT_API AHorrorThreatCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AHorrorThreatCharacter();

	UFUNCTION(BlueprintCallable, Category="Horror|Threat")
	bool ActivateThreat();

	UFUNCTION(BlueprintCallable, Category="Horror|Threat")
	bool DeactivateThreat();

	UFUNCTION(BlueprintPure, Category="Horror|Threat")
	bool IsThreatActive() const;

	UFUNCTION(BlueprintPure, Category="Horror|Threat")
	bool CanDetectActor(const AActor* TargetActor) const;

	UFUNCTION(BlueprintCallable, Category="Horror|Threat")
	bool UpdateDetectedTarget(AActor* TargetActor);

	UFUNCTION(BlueprintPure, Category="Horror|Threat")
	AActor* GetDetectedTarget() const;

	UFUNCTION(BlueprintCallable, Category="Horror|Threat")
	bool ClearDetectedTarget();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Threat")
	FName ThreatId = TEXT("Threat.Default");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Threat", meta=(ClampMin="0.0", Units="cm"))
	float DetectionRadius = HorrorThreatDefaults::DetectionRadiusCm;

	UFUNCTION(BlueprintPure, Category="Horror|Threat")
	class UHorrorGolemBehaviorComponent* GetGolemBehavior() const;

	UPROPERTY(BlueprintAssignable, Category="Horror|Threat")
	FHorrorThreatActiveChangedSignature OnThreatActiveChanged;

	UPROPERTY(BlueprintAssignable, Category="Horror|Threat")
	FHorrorThreatDetectedTargetChangedSignature OnDetectedTargetChanged;

private:
	void ApplyThreatAnimation(class UAnimationAsset* AnimationAsset);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Horror|Threat", meta=(AllowPrivateAccess="true"))
	TObjectPtr<class UHorrorGolemBehaviorComponent> GolemBehavior;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category="Horror|Threat|Animation", meta=(AllowPrivateAccess="true"))
	TObjectPtr<class UAnimationAsset> IdleAnimation;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category="Horror|Threat|Animation", meta=(AllowPrivateAccess="true"))
	TObjectPtr<class UAnimationAsset> RunAnimation;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Horror|Threat", meta=(AllowPrivateAccess="true"))
	bool bThreatActive = false;

	UPROPERTY(Transient, VisibleInstanceOnly, BlueprintReadOnly, Category="Horror|Threat", meta=(AllowPrivateAccess="true"))
	TWeakObjectPtr<AActor> DetectedTarget;
};
