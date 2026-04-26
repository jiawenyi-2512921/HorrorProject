// Copyright Epic Games, Inc. All Rights Reserved.

#include "GraphicsSettings.h"
#include "GameFramework/GameUserSettings.h"
#include "Engine/Engine.h"
#include "RHI.h"

UGraphicsSettings::UGraphicsSettings()
{
	Resolution = FIntPoint(1920, 1080);
	bFullscreen = true;
	bVSync = false;
	FrameRateLimit = 0;

	QualityPreset = EQualityPreset::High;
	ViewDistanceQuality = 3;
	TextureQuality = 3;
	EffectsQuality = 3;
	PostProcessQuality = 3;
	FoliageQuality = 3;
	ShadingQuality = 3;

	ShadowQuality = EShadowQuality::High;
	bDynamicShadows = true;
	ShadowDistance = 5000.0f;

	AntiAliasingMethod = EAntiAliasingMethod::TAA;
	AntiAliasingQuality = 3;

	bMotionBlur = true;
	MotionBlurAmount = 0.5f;
	bAmbientOcclusion = true;
	bBloom = true;
	bLensFlares = true;
	Brightness = 1.0f;
	Gamma = 2.2f;

	bRayTracingEnabled = false;
	bRayTracedShadows = false;
	bRayTracedReflections = false;
	bRayTracedGlobalIllumination = false;
}

void UGraphicsSettings::Apply()
{
	ApplyResolution();
	ApplyQualitySettings();
	ApplyAdvancedSettings();

	if (UGameUserSettings* UserSettings = GEngine->GetGameUserSettings())
	{
		UserSettings->ApplySettings(false);
	}
}

void UGraphicsSettings::ApplyQualityPreset(int32 PresetLevel)
{
	QualityPreset = static_cast<EQualityPreset>(FMath::Clamp(PresetLevel, 0, 4));

	switch (QualityPreset)
	{
	case EQualityPreset::Low:
		ViewDistanceQuality = 0;
		TextureQuality = 0;
		EffectsQuality = 0;
		PostProcessQuality = 0;
		FoliageQuality = 0;
		ShadingQuality = 0;
		ShadowQuality = EShadowQuality::Low;
		AntiAliasingMethod = EAntiAliasingMethod::FXAA;
		bMotionBlur = false;
		bAmbientOcclusion = false;
		bRayTracingEnabled = false;
		break;

	case EQualityPreset::Medium:
		ViewDistanceQuality = 1;
		TextureQuality = 1;
		EffectsQuality = 1;
		PostProcessQuality = 1;
		FoliageQuality = 1;
		ShadingQuality = 1;
		ShadowQuality = EShadowQuality::Medium;
		AntiAliasingMethod = EAntiAliasingMethod::TAA;
		bMotionBlur = true;
		bAmbientOcclusion = true;
		bRayTracingEnabled = false;
		break;

	case EQualityPreset::High:
		ViewDistanceQuality = 2;
		TextureQuality = 2;
		EffectsQuality = 2;
		PostProcessQuality = 2;
		FoliageQuality = 2;
		ShadingQuality = 2;
		ShadowQuality = EShadowQuality::High;
		AntiAliasingMethod = EAntiAliasingMethod::TAA;
		bMotionBlur = true;
		bAmbientOcclusion = true;
		bRayTracingEnabled = false;
		break;

	case EQualityPreset::Ultra:
		ViewDistanceQuality = 3;
		TextureQuality = 3;
		EffectsQuality = 3;
		PostProcessQuality = 3;
		FoliageQuality = 3;
		ShadingQuality = 3;
		ShadowQuality = EShadowQuality::Ultra;
		AntiAliasingMethod = EAntiAliasingMethod::TAA;
		bMotionBlur = true;
		bAmbientOcclusion = true;
		bRayTracingEnabled = IsRayTracingSupported();
		break;

	case EQualityPreset::Cinematic:
		ViewDistanceQuality = 4;
		TextureQuality = 4;
		EffectsQuality = 4;
		PostProcessQuality = 4;
		FoliageQuality = 4;
		ShadingQuality = 4;
		ShadowQuality = EShadowQuality::Ultra;
		AntiAliasingMethod = EAntiAliasingMethod::TAA;
		bMotionBlur = true;
		bAmbientOcclusion = true;
		bRayTracingEnabled = IsRayTracingSupported();
		bRayTracedShadows = IsRayTracingSupported();
		bRayTracedReflections = IsRayTracingSupported();
		break;

	default:
		break;
	}
}

void UGraphicsSettings::AutoDetect()
{
	DetectHardwareCapabilities();

	if (UGameUserSettings* UserSettings = GEngine->GetGameUserSettings())
	{
		UserSettings->RunHardwareBenchmark();

		int32 RecommendedQuality = UserSettings->GetRecommendedResolutionScale();
		ApplyQualityPreset(FMath::Clamp(RecommendedQuality, 0, 4));
	}
}

bool UGraphicsSettings::IsRayTracingSupported() const
{
	return GRHISupportsRayTracing && GRHISupportsRayTracingShaders;
}

TArray<FIntPoint> UGraphicsSettings::GetSupportedResolutions() const
{
	TArray<FIntPoint> Resolutions;

	if (UGameUserSettings* UserSettings = GEngine->GetGameUserSettings())
	{
		FScreenResolutionArray ResolutionArray;
		if (RHIGetAvailableResolutions(ResolutionArray, true))
		{
			for (const FScreenResolutionRHI& Resolution : ResolutionArray)
			{
				Resolutions.Add(FIntPoint(Resolution.Width, Resolution.Height));
			}
		}
	}

	if (Resolutions.Num() == 0)
	{
		Resolutions.Add(FIntPoint(1920, 1080));
		Resolutions.Add(FIntPoint(2560, 1440));
		Resolutions.Add(FIntPoint(3840, 2160));
	}

	return Resolutions;
}

void UGraphicsSettings::SetResolution(FIntPoint NewResolution, bool bApplyImmediately)
{
	Resolution = NewResolution;

	if (bApplyImmediately)
	{
		ApplyResolution();
	}
}

void UGraphicsSettings::SetFullscreen(bool bNewFullscreen, bool bApplyImmediately)
{
	bFullscreen = bNewFullscreen;

	if (bApplyImmediately)
	{
		ApplyResolution();
	}
}

void UGraphicsSettings::ApplyResolution()
{
	if (UGameUserSettings* UserSettings = GEngine->GetGameUserSettings())
	{
		UserSettings->SetScreenResolution(Resolution);
		UserSettings->SetFullscreenMode(bFullscreen ? EWindowMode::Fullscreen : EWindowMode::Windowed);
		UserSettings->SetVSyncEnabled(bVSync);
		UserSettings->SetFrameRateLimit(FrameRateLimit);
	}
}

void UGraphicsSettings::ApplyQualitySettings()
{
	if (UGameUserSettings* UserSettings = GEngine->GetGameUserSettings())
	{
		UserSettings->SetViewDistanceQuality(ViewDistanceQuality);
		UserSettings->SetTextureQuality(TextureQuality);
		UserSettings->SetVisualEffectQuality(EffectsQuality);
		UserSettings->SetPostProcessingQuality(PostProcessQuality);
		UserSettings->SetFoliageQuality(FoliageQuality);
		UserSettings->SetShadingQuality(ShadingQuality);
		UserSettings->SetShadowQuality(static_cast<int32>(ShadowQuality));
		UserSettings->SetAntiAliasingQuality(AntiAliasingQuality);
	}
}

void UGraphicsSettings::ApplyAdvancedSettings()
{
	if (GEngine)
	{
		GEngine->SetMotionBlurEnabled(bMotionBlur);
	}
}

void UGraphicsSettings::DetectHardwareCapabilities()
{
	if (UGameUserSettings* UserSettings = GEngine->GetGameUserSettings())
	{
		UserSettings->LoadSettings();
	}
}
