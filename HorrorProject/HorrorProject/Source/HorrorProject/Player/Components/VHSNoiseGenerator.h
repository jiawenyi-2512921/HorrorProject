// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "VHSNoiseGenerator.generated.h"

class UMaterialInstanceDynamic;

USTRUCT(BlueprintType)
struct FVHSNoiseParams
{
	GENERATED_BODY()

	static constexpr float DefaultScanlineCount = 480.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Noise", meta=(ClampMin="0.0", ClampMax="1.0"))
	float BaseNoiseIntensity = 0.15f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Noise", meta=(ClampMin="0.0", ClampMax="1.0"))
	float StressNoiseMultiplier = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Noise", meta=(ClampMin="0.0", ClampMax="1.0"))
	float LowBatteryNoiseMultiplier = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Scanlines", meta=(ClampMin="0.0", ClampMax="1.0"))
	float ScanlineIntensity = 0.3f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Scanlines", meta=(ClampMin="100.0", ClampMax="1000.0"))
	float ScanlineCount = DefaultScanlineCount;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Scanlines", meta=(ClampMin="0.0", ClampMax="10.0"))
	float ScanlineSpeed = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Chromatic", meta=(ClampMin="0.0", ClampMax="1.0"))
	float ChromaticAberration = 0.02f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Chromatic", meta=(ClampMin="0.0", ClampMax="1.0"))
	float ChromaticStressMultiplier = 2.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Noise", meta=(ClampMin="0.1", ClampMax="10.0"))
	float NoiseFrequency = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Noise", meta=(ClampMin="0.0", ClampMax="1.0"))
	float NoiseSpeed = 0.5f;
};

/**
 * Defines VHSNoise Generator behavior for the Player module.
 */
UCLASS(BlueprintType)
class HORRORPROJECT_API UVHSNoiseGenerator : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category="VHS|Noise")
	void Initialize(UMaterialInstanceDynamic* MaterialInstance);

	UFUNCTION(BlueprintCallable, Category="VHS|Noise")
	void UpdateNoise(float DeltaTime, float StressLevel, float BatteryLevel);

	UFUNCTION(BlueprintCallable, Category="VHS|Noise")
	void SetNoiseParams(const FVHSNoiseParams& NewParams);

	UFUNCTION(BlueprintPure, Category="VHS|Noise")
	FVHSNoiseParams GetNoiseParams() const { return NoiseParams; }

	UFUNCTION(BlueprintCallable, Category="VHS|Noise")
	void SetBaseNoiseIntensity(float Intensity);

	UFUNCTION(BlueprintCallable, Category="VHS|Noise")
	void SetScanlineIntensity(float Intensity);

	UFUNCTION(BlueprintCallable, Category="VHS|Noise")
	void SetChromaticAberration(float Aberration);

	UFUNCTION(BlueprintPure, Category="VHS|Noise")
	float GetCurrentNoiseIntensity() const { return CurrentNoiseIntensity; }

	UFUNCTION(BlueprintPure, Category="VHS|Noise")
	float GetCurrentChromaticAberration() const { return CurrentChromaticAberration; }

private:
	void UpdateMaterialParameters();
	float GenerateProceduralNoise(float Time, float Frequency) const;
	float CalculateNoiseIntensity(float StressLevel, float BatteryLevel) const;
	float CalculateChromaticIntensity(float StressLevel) const;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> DynamicMaterial;

	UPROPERTY(EditAnywhere, Category="VHS|Noise")
	FVHSNoiseParams NoiseParams;

	float AccumulatedTime = 0.0f;
	float CurrentNoiseIntensity = 0.0f;
	float CurrentChromaticAberration = 0.0f;
	float NoisePhase = 0.0f;
	float ScanlineOffset = 0.0f;
};
