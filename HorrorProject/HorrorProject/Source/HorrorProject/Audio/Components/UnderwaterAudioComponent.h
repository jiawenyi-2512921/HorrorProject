// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/AudioComponent.h"
#include "UnderwaterAudioComponent.generated.h"

USTRUCT(BlueprintType)
struct HORRORPROJECT_API FUnderwaterAudioConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Underwater Audio")
	TObjectPtr<USoundBase> UnderwaterAmbient;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Underwater Audio")
	TObjectPtr<USoundBase> BubbleSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Underwater Audio")
	TObjectPtr<USoundBase> SurfaceBreachSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Underwater Audio")
	TObjectPtr<USoundBase> DrowningSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Underwater Audio", meta=(ClampMin="0.0", ClampMax="1.0"))
	float UnderwaterVolume = 0.7f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Underwater Audio", meta=(ClampMin="0.0", ClampMax="1.0"))
	float LowPassFilterFrequency = 0.3f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Underwater Audio")
	float TransitionTime = 1.5f;
};

/**
 * Adds Underwater Audio Component behavior to its owning actor in the Audio module.
 */
UCLASS(ClassGroup=(Audio), meta=(BlueprintSpawnableComponent))
class HORRORPROJECT_API UUnderwaterAudioComponent : public UAudioComponent
{
	GENERATED_BODY()

public:
	UUnderwaterAudioComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, Category="Underwater Audio")
	void EnterWater(float Depth);

	UFUNCTION(BlueprintCallable, Category="Underwater Audio")
	void ExitWater();

	UFUNCTION(BlueprintCallable, Category="Underwater Audio")
	void UpdateDepth(float Depth);

	UFUNCTION(BlueprintCallable, Category="Underwater Audio")
	void PlayBubbleSound();

	UFUNCTION(BlueprintCallable, Category="Underwater Audio")
	void SetOxygenLevel(float OxygenLevel);

	UFUNCTION(BlueprintPure, Category="Underwater Audio")
	bool IsUnderwater() const { return bIsUnderwater; }

	UFUNCTION(BlueprintPure, Category="Underwater Audio")
	float GetCurrentDepth() const { return CurrentDepth; }

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Underwater Audio")
	FUnderwaterAudioConfig AudioConfig;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Underwater Audio")
	float MaxDepth = 1000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Underwater Audio")
	float BubbleInterval = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Underwater Audio")
	float BubbleIntervalVariation = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Underwater Audio")
	float OxygenWarningThreshold = 0.3f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Underwater Audio")
	float DepthPressureMultiplier = 0.5f;

private:
	bool bIsUnderwater = false;
	float CurrentDepth = 0.0f;
	float CurrentOxygenLevel = 1.0f;
	float TimeSinceLastBubble = 0.0f;
	float NextBubbleInterval = 2.0f;

	UPROPERTY(Transient)
	TObjectPtr<UAudioComponent> AmbientComponent;

	UPROPERTY(Transient)
	TObjectPtr<UAudioComponent> DrowningComponent;

	void UpdateUnderwaterEffects();
	void UpdateBubbles(float DeltaTime);
	void UpdateDrowningSound();
	float CalculateDepthMultiplier() const;
};
