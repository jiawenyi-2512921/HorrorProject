// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/AudioComponent.h"
#include "BreathingAudioComponent.generated.h"

UENUM(BlueprintType)
enum class EBreathingState : uint8
{
	Calm UMETA(DisplayName="Calm"),
	Normal UMETA(DisplayName="Normal"),
	Tired UMETA(DisplayName="Tired"),
	Exhausted UMETA(DisplayName="Exhausted"),
	Panicked UMETA(DisplayName="Panicked"),
	Holding UMETA(DisplayName="Holding Breath")
};

USTRUCT(BlueprintType)
struct HORRORPROJECT_API FBreathingStateConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Breathing")
	TObjectPtr<USoundBase> BreathingSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Breathing", meta=(ClampMin="0.0", ClampMax="1.0"))
	float Volume = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Breathing", meta=(ClampMin="0.1", ClampMax="3.0"))
	float PitchMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Breathing", meta=(ClampMin="0.1", ClampMax="10.0"))
	float BreathInterval = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Breathing")
	float TransitionTime = 1.0f;
};

/**
 * Adds Breathing Audio Component behavior to its owning actor in the Audio module.
 */
UCLASS(ClassGroup=(Audio), meta=(BlueprintSpawnableComponent))
class HORRORPROJECT_API UBreathingAudioComponent : public UAudioComponent
{
	GENERATED_BODY()

public:
	UBreathingAudioComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, Category="Breathing Audio")
	void SetBreathingState(EBreathingState NewState);

	UFUNCTION(BlueprintCallable, Category="Breathing Audio")
	void SetFearLevel(float FearLevel);

	UFUNCTION(BlueprintCallable, Category="Breathing Audio")
	void SetStaminaLevel(float StaminaLevel);

	UFUNCTION(BlueprintCallable, Category="Breathing Audio")
	void StartBreathing();

	UFUNCTION(BlueprintCallable, Category="Breathing Audio")
	void StopBreathing(float FadeOutTime = 1.0f);

	UFUNCTION(BlueprintCallable, Category="Breathing Audio")
	void HoldBreath();

	UFUNCTION(BlueprintCallable, Category="Breathing Audio")
	void ReleaseBreath();

	UFUNCTION(BlueprintPure, Category="Breathing Audio")
	EBreathingState GetCurrentState() const { return CurrentState; }

	UFUNCTION(BlueprintCallable, Category="Breathing Audio")
	void RegisterStateConfig(EBreathingState State, const FBreathingStateConfig& Config);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Breathing Audio")
	TMap<EBreathingState, FBreathingStateConfig> StateConfigs;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Breathing Audio")
	EBreathingState CurrentState = EBreathingState::Normal;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Breathing Audio")
	bool bAutoStart = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Breathing Audio")
	float FearThresholdPanicked = 0.8f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Breathing Audio")
	float StaminaThresholdExhausted = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Breathing Audio")
	float StaminaThresholdTired = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Breathing Audio")
	bool bUseAutoStateTransition = true;

private:
	float CurrentFearLevel = 0.0f;
	float CurrentStaminaLevel = 1.0f;
	float TimeSinceLastBreath = 0.0f;
	bool bIsBreathing = false;
	bool bIsHoldingBreath = false;

	void UpdateBreathingState();
	void PlayBreathSound();
	void TransitionToState(EBreathingState NewState);
	EBreathingState CalculateAutoState() const;
};
