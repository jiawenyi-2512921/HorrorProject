// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GraphicsSettings.generated.h"

UENUM(BlueprintType)
enum class EQualityPreset : uint8
{
	Low = 0,
	Medium = 1,
	High = 2,
	Ultra = 3,
	Cinematic = 4,
	Custom = 0xff
};

UENUM(BlueprintType)
enum class EHorrorAntiAliasingMethod : uint8
{
	None,
	FXAA,
	TAA,
	MSAA_2x,
	MSAA_4x,
	MSAA_8x
};

UENUM(BlueprintType)
enum class EShadowQuality : uint8
{
	Low,
	Medium,
	High,
	Ultra
};

/**
 * Graphics Settings
 * Manages all graphics-related settings
 */
UCLASS(BlueprintType, Config=GameUserSettings)
class HORRORPROJECT_API UGraphicsSettings : public UObject
{
	GENERATED_BODY()

public:
	UGraphicsSettings();

	// Resolution
	UPROPERTY(Config, BlueprintReadWrite, Category = "Display")
	FIntPoint Resolution;

	UPROPERTY(Config, BlueprintReadWrite, Category = "Display")
	bool bFullscreen;

	UPROPERTY(Config, BlueprintReadWrite, Category = "Display")
	bool bVSync;

	UPROPERTY(Config, BlueprintReadWrite, Category = "Display")
	int32 FrameRateLimit;

	// Quality
	UPROPERTY(Config, BlueprintReadWrite, Category = "Quality")
	EQualityPreset QualityPreset;

	UPROPERTY(Config, BlueprintReadWrite, Category = "Quality")
	int32 ViewDistanceQuality;

	UPROPERTY(Config, BlueprintReadWrite, Category = "Quality")
	int32 TextureQuality;

	UPROPERTY(Config, BlueprintReadWrite, Category = "Quality")
	int32 EffectsQuality;

	UPROPERTY(Config, BlueprintReadWrite, Category = "Quality")
	int32 PostProcessQuality;

	UPROPERTY(Config, BlueprintReadWrite, Category = "Quality")
	int32 FoliageQuality;

	UPROPERTY(Config, BlueprintReadWrite, Category = "Quality")
	int32 ShadingQuality;

	// Shadows
	UPROPERTY(Config, BlueprintReadWrite, Category = "Shadows")
	EShadowQuality ShadowQuality;

	UPROPERTY(Config, BlueprintReadWrite, Category = "Shadows")
	bool bDynamicShadows;

	UPROPERTY(Config, BlueprintReadWrite, Category = "Shadows")
	float ShadowDistance;

	// Anti-Aliasing
	UPROPERTY(Config, BlueprintReadWrite, Category = "Anti-Aliasing")
	EHorrorAntiAliasingMethod AntiAliasingMethod;

	UPROPERTY(Config, BlueprintReadWrite, Category = "Anti-Aliasing")
	int32 AntiAliasingQuality;

	// Advanced
	UPROPERTY(Config, BlueprintReadWrite, Category = "Advanced")
	bool bMotionBlur;

	UPROPERTY(Config, BlueprintReadWrite, Category = "Advanced")
	float MotionBlurAmount;

	UPROPERTY(Config, BlueprintReadWrite, Category = "Advanced")
	bool bAmbientOcclusion;

	UPROPERTY(Config, BlueprintReadWrite, Category = "Advanced")
	bool bBloom;

	UPROPERTY(Config, BlueprintReadWrite, Category = "Advanced")
	bool bLensFlares;

	UPROPERTY(Config, BlueprintReadWrite, Category = "Advanced")
	float Brightness;

	UPROPERTY(Config, BlueprintReadWrite, Category = "Advanced")
	float Gamma;

	// Ray Tracing
	UPROPERTY(Config, BlueprintReadWrite, Category = "RayTracing")
	bool bRayTracingEnabled;

	UPROPERTY(Config, BlueprintReadWrite, Category = "RayTracing")
	bool bRayTracedShadows;

	UPROPERTY(Config, BlueprintReadWrite, Category = "RayTracing")
	bool bRayTracedReflections;

	UPROPERTY(Config, BlueprintReadWrite, Category = "RayTracing")
	bool bRayTracedGlobalIllumination;

	// Operations
	UFUNCTION(BlueprintCallable, Category = "Graphics")
	void Apply();

	UFUNCTION(BlueprintCallable, Category = "Graphics")
	void ApplyQualityPreset(int32 PresetLevel);

	UFUNCTION(BlueprintCallable, Category = "Graphics")
	void AutoDetect();

	UFUNCTION(BlueprintPure, Category = "Graphics")
	bool IsRayTracingSupported() const;

	UFUNCTION(BlueprintPure, Category = "Graphics")
	TArray<FIntPoint> GetSupportedResolutions() const;

	UFUNCTION(BlueprintCallable, Category = "Graphics")
	void SetResolution(FIntPoint NewResolution, bool bApplyImmediately = true);

	UFUNCTION(BlueprintCallable, Category = "Graphics")
	void SetFullscreen(bool bNewFullscreen, bool bApplyImmediately = true);

private:
	void ApplyResolution();
	void ApplyQualitySettings();
	void ApplyAdvancedSettings();
	void DetectHardwareCapabilities();
};
