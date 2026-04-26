// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "VHSVisualEffectComponent.generated.h"

/**
 * VHS visual effect component for post-process effects
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class HORRORPROJECT_API UVHSVisualEffectComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UVHSVisualEffectComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, Category="VHS|Effects")
	void SetEffectIntensity(float Intensity);

	UFUNCTION(BlueprintCallable, Category="VHS|Effects")
	void EnableEffect(bool bEnable);

	UFUNCTION(BlueprintCallable, Category="VHS|Effects")
	void TriggerGlitch(float Duration = 0.5f, float Intensity = 1.0f);

	UFUNCTION(BlueprintPure, Category="VHS|Effects")
	float GetEffectIntensity() const { return CurrentIntensity; }

	UFUNCTION(BlueprintPure, Category="VHS|Effects")
	bool IsEffectEnabled() const { return bEffectEnabled; }

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, Category="VHS|Effects")
	float BaseIntensity = 0.3f;

	UPROPERTY(EditDefaultsOnly, Category="VHS|Effects")
	float NoiseFrequency = 10.0f;

	UPROPERTY(EditDefaultsOnly, Category="VHS|Effects")
	float ScanlineIntensity = 0.2f;

	UPROPERTY(EditDefaultsOnly, Category="VHS|Effects")
	float ChromaticAberration = 0.5f;

	UPROPERTY(EditDefaultsOnly, Category="VHS|Effects")
	float VignetteIntensity = 0.4f;

	UPROPERTY(BlueprintReadOnly, Category="VHS|Effects")
	float CurrentIntensity = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category="VHS|Effects")
	bool bEffectEnabled = false;

private:
	float GlitchTimer = 0.0f;
	float GlitchDuration = 0.0f;
	float GlitchIntensity = 0.0f;
	float TimeAccumulator = 0.0f;
};
