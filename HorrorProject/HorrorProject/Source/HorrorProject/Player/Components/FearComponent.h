// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FearComponent.generated.h"

UENUM(BlueprintType)
enum class EFearLevel : uint8
{
	Calm UMETA(DisplayName="平静"),
	Uneasy UMETA(DisplayName="不安"),
	Afraid UMETA(DisplayName="害怕"),
	Terrified UMETA(DisplayName="恐惧"),
	Panicked UMETA(DisplayName="恐慌")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FFearLevelChangedSignature, EFearLevel, NewLevel, float, FearPercent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FFearValueChangedSignature, float, FearValue, float, FearPercent);

/**
 * Adds Fear Component behavior to its owning actor in the Player module.
 */
UCLASS(ClassGroup=(Horror), BlueprintType, Blueprintable, meta=(BlueprintSpawnableComponent))
class HORRORPROJECT_API UFearComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UFearComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, Category="Fear")
	bool AddFear(float Amount, FName SourceId = NAME_None);

	UFUNCTION(BlueprintCallable, Category="Fear")
	bool RemoveFear(float Amount);

	UFUNCTION(BlueprintCallable, Category="Fear")
	bool SetFearValue(float NewValue);

	UFUNCTION(BlueprintPure, Category="Fear")
	float GetFearValue() const { return FearValue; }

	UFUNCTION(BlueprintPure, Category="Fear")
	float GetFearPercent() const;

	UFUNCTION(BlueprintPure, Category="Fear")
	EFearLevel GetFearLevel() const { return CurrentFearLevel; }

	UFUNCTION(BlueprintPure, Category="Fear")
	bool IsFearLevel(EFearLevel Level) const { return CurrentFearLevel == Level; }

	UFUNCTION(BlueprintPure, Category="Fear|Effects")
	float GetMovementSpeedMultiplier() const;

	UFUNCTION(BlueprintPure, Category="Fear|Effects")
	float GetCameraShakeIntensity() const;

	UFUNCTION(BlueprintPure, Category="Fear|Effects")
	float GetVignetteIntensity() const;

	UFUNCTION(BlueprintPure, Category="Fear|Effects")
	float GetHeartbeatVolume() const;

	UPROPERTY(BlueprintAssignable, Category="Fear")
	FFearLevelChangedSignature OnFearLevelChanged;

	UPROPERTY(BlueprintAssignable, Category="Fear")
	FFearValueChangedSignature OnFearValueChanged;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Fear", meta=(ClampMin="0.0", ClampMax="1000.0"))
	float MaxFearValue = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Fear", meta=(ClampMin="0.0"))
	float FearDecayRate = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Fear", meta=(ClampMin="0.0", ClampMax="100.0"))
	float FearDecayDelay = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Fear|Thresholds", meta=(ClampMin="0.0", ClampMax="1.0"))
	float UneasyThreshold = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Fear|Thresholds", meta=(ClampMin="0.0", ClampMax="1.0"))
	float AfraidThreshold = 0.4f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Fear|Thresholds", meta=(ClampMin="0.0", ClampMax="1.0"))
	float TerrifiedThreshold = 0.6f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Fear|Thresholds", meta=(ClampMin="0.0", ClampMax="1.0"))
	float PanickedThreshold = 0.8f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Fear|Effects", meta=(ClampMin="0.0", ClampMax="1.0"))
	float MinSpeedMultiplier = 0.7f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Fear|Effects", meta=(ClampMin="0.0", ClampMax="10.0"))
	float MaxCameraShakeIntensity = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Fear|Effects", meta=(ClampMin="0.0", ClampMax="1.0"))
	float MaxVignetteIntensity = 0.6f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Fear|Effects", meta=(ClampMin="0.0", ClampMax="1.0"))
	float MaxHeartbeatVolume = 1.0f;

private:
	void UpdateFearDecay(float DeltaTime);
	void UpdateFearLevel();
	EFearLevel CalculateFearLevel() const;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, SaveGame, Category="Fear", meta=(AllowPrivateAccess="true"))
	float FearValue = 0.0f;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Fear", meta=(AllowPrivateAccess="true"))
	EFearLevel CurrentFearLevel = EFearLevel::Calm;

	float TimeSinceLastFearIncrease = 0.0f;

	// Performance optimization: Timer handle for decay updates
	FTimerHandle FearDecayTimerHandle;
	void StartFearDecayTimer();
	void StopFearDecayTimer();
	void UpdateFearDecayTimer();
};
