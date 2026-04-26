// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ScanlineComponent.generated.h"

/**
 * Scanline effect component for CRT/VHS display simulation
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class HORRORPROJECT_API UScanlineComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UScanlineComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, Category="Scanline|Effects")
	void SetScanlineIntensity(float Intensity);

	UFUNCTION(BlueprintCallable, Category="Scanline|Effects")
	void SetScanlineSpeed(float Speed);

	UFUNCTION(BlueprintCallable, Category="Scanline|Effects")
	void EnableScanlines(bool bEnable);

	UFUNCTION(BlueprintPure, Category="Scanline|Effects")
	float GetScanlineIntensity() const { return ScanlineIntensity; }

	UFUNCTION(BlueprintPure, Category="Scanline|Effects")
	float GetScanlineOffset() const { return CurrentOffset; }

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, Category="Scanline|Effects")
	float ScanlineIntensity = 0.2f;

	UPROPERTY(EditDefaultsOnly, Category="Scanline|Effects")
	float ScanlineSpeed = 0.5f;

	UPROPERTY(EditDefaultsOnly, Category="Scanline|Effects")
	float ScanlineCount = 480.0f;

	UPROPERTY(EditDefaultsOnly, Category="Scanline|Effects")
	float ScanlineThickness = 0.5f;

	UPROPERTY(BlueprintReadOnly, Category="Scanline|Effects")
	float CurrentOffset = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category="Scanline|Effects")
	bool bScanlinesEnabled = true;

private:
	float TimeAccumulator = 0.0f;
};
