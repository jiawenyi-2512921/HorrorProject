// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/Components/VHSNoiseGenerator.h"
#include "Materials/MaterialInstanceDynamic.h"

namespace HorrorVHSNoise
{
	constexpr float MinBatteryThreshold = 0.2f;
	constexpr float NoiseBlendSpeed = 2.0f;
	constexpr float ChromaticBlendSpeed = 3.0f;
	constexpr float Pi = 3.14159265359f;
	constexpr float Tau = Pi * 2.0f;
}

void UVHSNoiseGenerator::Initialize(UMaterialInstanceDynamic* MaterialInstance)
{
	DynamicMaterial = MaterialInstance;
	AccumulatedTime = 0.0f;
	NoisePhase = FMath::FRandRange(0.0f, HorrorVHSNoise::Tau);
	ScanlineOffset = 0.0f;
	CurrentNoiseIntensity = NoiseParams.BaseNoiseIntensity;
	CurrentChromaticAberration = NoiseParams.ChromaticAberration;

	if (DynamicMaterial)
	{
		UpdateMaterialParameters();
	}
}

void UVHSNoiseGenerator::UpdateNoise(float DeltaTime, float StressLevel, float BatteryLevel)
{
	if (!DynamicMaterial)
	{
		return;
	}

	AccumulatedTime += DeltaTime;
	NoisePhase += DeltaTime * NoiseParams.NoiseSpeed * HorrorVHSNoise::Tau;
	if (NoisePhase > HorrorVHSNoise::Tau)
	{
		NoisePhase -= HorrorVHSNoise::Tau;
	}

	ScanlineOffset += DeltaTime * NoiseParams.ScanlineSpeed;
	if (ScanlineOffset > 1.0f)
	{
		ScanlineOffset -= 1.0f;
	}

	const float TargetNoiseIntensity = CalculateNoiseIntensity(StressLevel, BatteryLevel);
	CurrentNoiseIntensity = FMath::FInterpTo(CurrentNoiseIntensity, TargetNoiseIntensity, DeltaTime, HorrorVHSNoise::NoiseBlendSpeed);

	const float TargetChromaticAberration = CalculateChromaticIntensity(StressLevel);
	CurrentChromaticAberration = FMath::FInterpTo(CurrentChromaticAberration, TargetChromaticAberration, DeltaTime, HorrorVHSNoise::ChromaticBlendSpeed);

	UpdateMaterialParameters();
}

void UVHSNoiseGenerator::SetNoiseParams(const FVHSNoiseParams& NewParams)
{
	NoiseParams = NewParams;
	if (DynamicMaterial)
	{
		UpdateMaterialParameters();
	}
}

void UVHSNoiseGenerator::SetBaseNoiseIntensity(float Intensity)
{
	NoiseParams.BaseNoiseIntensity = FMath::Clamp(Intensity, 0.0f, 1.0f);
	if (DynamicMaterial)
	{
		UpdateMaterialParameters();
	}
}

void UVHSNoiseGenerator::SetScanlineIntensity(float Intensity)
{
	NoiseParams.ScanlineIntensity = FMath::Clamp(Intensity, 0.0f, 1.0f);
	if (DynamicMaterial)
	{
		UpdateMaterialParameters();
	}
}

void UVHSNoiseGenerator::SetChromaticAberration(float Aberration)
{
	NoiseParams.ChromaticAberration = FMath::Clamp(Aberration, 0.0f, 1.0f);
	if (DynamicMaterial)
	{
		UpdateMaterialParameters();
	}
}

void UVHSNoiseGenerator::UpdateMaterialParameters()
{
	if (!DynamicMaterial)
	{
		return;
	}

	const float ProceduralNoise = GenerateProceduralNoise(AccumulatedTime, NoiseParams.NoiseFrequency);
	const float FinalNoiseIntensity = CurrentNoiseIntensity * (0.8f + ProceduralNoise * 0.2f);

	DynamicMaterial->SetScalarParameterValue(FName("NoiseIntensity"), FinalNoiseIntensity);
	DynamicMaterial->SetScalarParameterValue(FName("NoisePhase"), NoisePhase);
	DynamicMaterial->SetScalarParameterValue(FName("NoiseFrequency"), NoiseParams.NoiseFrequency);
	DynamicMaterial->SetScalarParameterValue(FName("ScanlineIntensity"), NoiseParams.ScanlineIntensity);
	DynamicMaterial->SetScalarParameterValue(FName("ScanlineCount"), NoiseParams.ScanlineCount);
	DynamicMaterial->SetScalarParameterValue(FName("ScanlineOffset"), ScanlineOffset);
	DynamicMaterial->SetScalarParameterValue(FName("ChromaticAberration"), CurrentChromaticAberration);
	DynamicMaterial->SetScalarParameterValue(FName("Time"), AccumulatedTime);
}

float UVHSNoiseGenerator::GenerateProceduralNoise(float Time, float Frequency) const
{
	const float X = FMath::Sin(Time * Frequency * 1.7f + NoisePhase) * 0.5f + 0.5f;
	const float Y = FMath::Sin(Time * Frequency * 2.3f + NoisePhase * 1.3f) * 0.5f + 0.5f;
	const float Z = FMath::Sin(Time * Frequency * 3.1f + NoisePhase * 0.7f) * 0.5f + 0.5f;
	return (X + Y + Z) / 3.0f;
}

float UVHSNoiseGenerator::CalculateNoiseIntensity(float StressLevel, float BatteryLevel) const
{
	float Intensity = NoiseParams.BaseNoiseIntensity;

	const float ClampedStress = FMath::Clamp(StressLevel, 0.0f, 1.0f);
	Intensity += ClampedStress * NoiseParams.StressNoiseMultiplier * NoiseParams.BaseNoiseIntensity;

	const float ClampedBattery = FMath::Clamp(BatteryLevel, 0.0f, 1.0f);
	if (ClampedBattery < HorrorVHSNoise::MinBatteryThreshold)
	{
		const float BatteryFactor = 1.0f - (ClampedBattery / HorrorVHSNoise::MinBatteryThreshold);
		Intensity += BatteryFactor * NoiseParams.LowBatteryNoiseMultiplier * NoiseParams.BaseNoiseIntensity;
	}

	return FMath::Clamp(Intensity, 0.0f, 1.0f);
}

float UVHSNoiseGenerator::CalculateChromaticIntensity(float StressLevel) const
{
	const float ClampedStress = FMath::Clamp(StressLevel, 0.0f, 1.0f);
	const float Intensity = NoiseParams.ChromaticAberration * (1.0f + ClampedStress * NoiseParams.ChromaticStressMultiplier);
	return FMath::Clamp(Intensity, 0.0f, 1.0f);
}
