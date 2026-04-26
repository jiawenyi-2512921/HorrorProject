// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NoiseOverlayComponent.generated.h"

/**
 * Noise overlay component for film grain and static effects
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class HORRORPROJECT_API UNoiseOverlayComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UNoiseOverlayComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, Category="Noise|Effects")
	void SetNoiseIntensity(float Intensity);

	UFUNCTION(BlueprintCallable, Category="Noise|Effects")
	void SetNoiseSpeed(float Speed);

	UFUNCTION(BlueprintCallable, Category="Noise|Effects")
	void EnableNoise(bool bEnable);

	UFUNCTION(BlueprintPure, Category="Noise|Effects")
	float GetNoiseIntensity() const { return NoiseIntensity; }

	UFUNCTION(BlueprintPure, Category="Noise|Effects")
	float GetNoiseValue() const { return CurrentNoiseValue; }

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, Category="Noise|Effects")
	float NoiseIntensity = 0.15f;

	UPROPERTY(EditDefaultsOnly, Category="Noise|Effects")
	float NoiseSpeed = 20.0f;

	UPROPERTY(EditDefaultsOnly, Category="Noise|Effects")
	float NoiseScale = 1.0f;

	UPROPERTY(BlueprintReadOnly, Category="Noise|Effects")
	float CurrentNoiseValue = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category="Noise|Effects")
	bool bNoiseEnabled = true;

private:
	float TimeAccumulator = 0.0f;
};
