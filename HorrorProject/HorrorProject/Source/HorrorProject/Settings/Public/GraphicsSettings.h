// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GraphicsSettings.generated.h"

UENUM(BlueprintType)
enum class EQualityPreset : uint8
{
	Low = 0 UMETA(DisplayName = "低"),
	Medium = 1 UMETA(DisplayName = "中"),
	High = 2 UMETA(DisplayName = "高"),
	Ultra = 3 UMETA(DisplayName = "超高"),
	Cinematic = 4 UMETA(DisplayName = "电影级"),
	Custom = 0xff UMETA(DisplayName = "自定义")
};

UENUM(BlueprintType)
enum class EHorrorAntiAliasingMethod : uint8
{
	None UMETA(DisplayName = "无"),
	FXAA UMETA(DisplayName = "快速近似抗锯齿"),
	TAA UMETA(DisplayName = "时间抗锯齿"),
	MSAA_2x UMETA(DisplayName = "多重采样 2 倍"),
	MSAA_4x UMETA(DisplayName = "多重采样 4 倍"),
	MSAA_8x UMETA(DisplayName = "多重采样 8 倍")
};

UENUM(BlueprintType)
enum class EShadowQuality : uint8
{
	Low UMETA(DisplayName = "低"),
	Medium UMETA(DisplayName = "中"),
	High UMETA(DisplayName = "高"),
	Ultra UMETA(DisplayName = "超高")
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
