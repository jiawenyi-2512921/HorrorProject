// Copyright HorrorProject. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SanityComponent.generated.h"

class UPostProcessComponent;
class UMaterialInstanceDynamic;
class USoundBase;

UENUM(BlueprintType)
enum class ESanityLevel : uint8
{
	Stable UMETA(DisplayName="稳定"),
	Uneasy UMETA(DisplayName="不安"),
	Disturbed UMETA(DisplayName="受扰"),
	Breaking UMETA(DisplayName="崩溃边缘"),
	Shattered UMETA(DisplayName="破碎")
};

UENUM(BlueprintType)
enum class EHallucinationType : uint8
{
	None UMETA(DisplayName="无"),
	VisualDistortion UMETA(DisplayName="视觉扭曲"),
	ShadowMovement UMETA(DisplayName="影子移动"),
	PeripheralFigure UMETA(DisplayName="余光人影"),
	AuditoryWhisper UMETA(DisplayName="耳语幻听"),
	EnvironmentalShift UMETA(DisplayName="环境错位")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSanityLevelChangedSignature, ESanityLevel, NewLevel, float, SanityPercent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FHallucinationTriggeredSignature, EHallucinationType, Type, float, Intensity);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSanityRecoverySignature, float, RecoveredAmount);

USTRUCT(BlueprintType)
struct FHallucinationEvent
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Sanity")
	EHallucinationType Type = EHallucinationType::None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Sanity", meta=(ClampMin="0.0", ClampMax="1.0"))
	float Intensity = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Sanity", meta=(ClampMin="0.0", Units="s"))
	float Duration = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Sanity")
	TObjectPtr<USoundBase> AssociatedSound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Sanity")
	bool bRequiresLineOfSight = false;
};

UCLASS(ClassGroup=(Horror), BlueprintType, Blueprintable, meta=(BlueprintSpawnableComponent))
class HORRORPROJECT_API USanityComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USanityComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, Category="Sanity")
	void DrainSanity(float Amount, FName SourceId = NAME_None);

	UFUNCTION(BlueprintCallable, Category="Sanity")
	void RecoverSanity(float Amount);

	UFUNCTION(BlueprintCallable, Category="Sanity")
	void SetSanity(float NewValue);

	UFUNCTION(BlueprintPure, Category="Sanity")
	float GetSanity() const { return SanityValue; }

	UFUNCTION(BlueprintPure, Category="Sanity")
	float GetSanityPercent() const;

	UFUNCTION(BlueprintPure, Category="Sanity")
	ESanityLevel GetSanityLevel() const { return CurrentLevel; }

	UFUNCTION(BlueprintPure, Category="Sanity")
	bool IsHallucinating() const { return bHallucinationActive; }

	UFUNCTION(BlueprintPure, Category="Sanity")
	EHallucinationType GetActiveHallucination() const { return ActiveHallucination; }

	UFUNCTION(BlueprintPure, Category="Sanity")
	float GetHallucinationIntensity() const;

	UFUNCTION(BlueprintCallable, Category="Sanity")
	void TriggerHallucination(const FHallucinationEvent& Event);

	UFUNCTION(BlueprintCallable, Category="Sanity")
	void SetIsolationFactor(float Factor);

	UFUNCTION(BlueprintCallable, Category="Sanity")
	void SetDarknessFactor(float Factor);

	UFUNCTION(BlueprintCallable, Category="Sanity")
	void SetThreatProximity(float DistanceCm);

	UPROPERTY(BlueprintAssignable, Category="Sanity")
	FSanityLevelChangedSignature OnSanityLevelChanged;

	UPROPERTY(BlueprintAssignable, Category="Sanity")
	FHallucinationTriggeredSignature OnHallucinationTriggered;

	UPROPERTY(BlueprintAssignable, Category="Sanity")
	FSanityRecoverySignature OnSanityRecovered;

	// Sanity drain rates
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Sanity|Drain", meta=(ClampMin="0.0"))
	float DarknessDrainRate = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Sanity|Drain", meta=(ClampMin="0.0"))
	float IsolationDrainRate = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Sanity|Drain", meta=(ClampMin="0.0"))
	float ThreatDrainRate = 8.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Sanity|Drain", meta=(ClampMin="0.0"))
	float NearThreatDistanceCm = 1500.0f;

	// Thresholds
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Sanity|Thresholds", meta=(ClampMin="0.0", ClampMax="1.0"))
	float UneasyThreshold = 0.75f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Sanity|Thresholds", meta=(ClampMin="0.0", ClampMax="1.0"))
	float DisturbedThreshold = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Sanity|Thresholds", meta=(ClampMin="0.0", ClampMax="1.0"))
	float BreakingThreshold = 0.25f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Sanity|Thresholds", meta=(ClampMin="0.0", ClampMax="1.0"))
	float ShatteredThreshold = 0.1f;

	// Hallucination settings
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Sanity|Hallucination", meta=(ClampMin="0.0", ClampMax="1.0"))
	float HallucinationStartThreshold = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Sanity|Hallucination", meta=(ClampMin="0.0", Units="s"))
	float HallucinationCooldownMin = 8.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Sanity|Hallucination", meta=(ClampMin="0.0", Units="s"))
	float HallucinationCooldownMax = 25.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Sanity|Hallucination")
	TArray<FHallucinationEvent> HallucinationPool;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Sanity|Recovery", meta=(ClampMin="0.0"))
	float NaturalRecoveryRate = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Sanity|Recovery", meta=(ClampMin="0.0", ClampMax="1.0"))
	float SafeZoneRecoveryMultiplier = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Sanity|Recovery")
	bool bInSafeZone = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Sanity|Effects", meta=(ClampMin="0.0", ClampMax="1.0"))
	float MaxPostProcessIntensity = 0.8f;

protected:
	virtual void BeginPlay() override;

private:
	void UpdateSanityDrain(float DeltaTime);
	void UpdateNaturalRecovery(float DeltaTime);
	void UpdateHallucinationTimer(float DeltaTime);
	void UpdatePostProcessEffects(float DeltaTime);
	void CheckLevelTransition();
	void TriggerRandomHallucination();

	float CalculateDrainRate() const;
	float GetLowestFactor() const;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, SaveGame, Category="Sanity", meta=(AllowPrivateAccess="true"))
	float SanityValue = 100.0f;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Sanity", meta=(AllowPrivateAccess="true"))
	ESanityLevel CurrentLevel = ESanityLevel::Stable;

	float MaxSanity = 100.0f;
	float DarknessFactor = 0.0f;
	float IsolationFactor = 0.0f;
	float ThreatProximityCm = TNumericLimits<float>::Max();
	float HallucinationCooldown = 0.0f;
	bool bHallucinationActive = false;
	EHallucinationType ActiveHallucination = EHallucinationType::None;
	float CurrentIntensity = 0.0f;
	float ActiveHallucinationTimer = 0.0f;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> PostProcessMaterialInstance;

	float PostProcessIntensity = 0.0f;
};
