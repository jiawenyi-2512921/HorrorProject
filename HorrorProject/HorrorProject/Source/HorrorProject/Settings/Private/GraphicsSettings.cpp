// Copyright Epic Games, Inc. All Rights Reserved.

#include "GraphicsSettings.h"
#include "GameFramework/GameUserSettings.h"
#include "Engine/Engine.h"
#include "HAL/IConsoleManager.h"
#include "RHI.h"

namespace
{
	const FIntPoint GraphicsDefaultDisplayResolution(1920, 1080);
	const FIntPoint GraphicsDefaultQHDResolution(2560, 1440);
	const FIntPoint GraphicsDefaultUHDResolution(3840, 2160);
	constexpr float GraphicsDefaultShadowDistance = 5000.0f;

	void ApplyScalabilityPreset(
		UGraphicsSettings& Settings,
		int32 QualityLevel,
		EShadowQuality ShadowQuality,
		EHorrorAntiAliasingMethod AntiAliasingMethod,
		bool bEnableMotionBlur,
		bool bEnableAmbientOcclusion,
		bool bEnableRayTracing)
	{
		Settings.ViewDistanceQuality = QualityLevel;
		Settings.TextureQuality = QualityLevel;
		Settings.EffectsQuality = QualityLevel;
		Settings.PostProcessQuality = QualityLevel;
		Settings.FoliageQuality = QualityLevel;
		Settings.ShadingQuality = QualityLevel;
		Settings.ShadowQuality = ShadowQuality;
		Settings.AntiAliasingMethod = AntiAliasingMethod;
		Settings.bMotionBlur = bEnableMotionBlur;
		Settings.bAmbientOcclusion = bEnableAmbientOcclusion;
		Settings.bRayTracingEnabled = bEnableRayTracing;
	}
}

UGraphicsSettings::UGraphicsSettings()
{
	Resolution = GraphicsDefaultDisplayResolution;
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
	ShadowDistance = GraphicsDefaultShadowDistance;

	AntiAliasingMethod = EHorrorAntiAliasingMethod::TAA;
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
	const bool bRayTracingSupported = IsRayTracingSupported();

	switch (QualityPreset)
	{
	case EQualityPreset::Low:
		ApplyScalabilityPreset(*this, 0, EShadowQuality::Low, EHorrorAntiAliasingMethod::FXAA, false, false, false);
		break;

	case EQualityPreset::Medium:
		ApplyScalabilityPreset(*this, 1, EShadowQuality::Medium, EHorrorAntiAliasingMethod::TAA, true, true, false);
		break;

	case EQualityPreset::High:
		ApplyScalabilityPreset(*this, 2, EShadowQuality::High, EHorrorAntiAliasingMethod::TAA, true, true, false);
		break;

	case EQualityPreset::Ultra:
		ApplyScalabilityPreset(*this, 3, EShadowQuality::Ultra, EHorrorAntiAliasingMethod::TAA, true, true, bRayTracingSupported);
		break;

	case EQualityPreset::Cinematic:
		ApplyScalabilityPreset(*this, 4, EShadowQuality::Ultra, EHorrorAntiAliasingMethod::TAA, true, true, bRayTracingSupported);
		bRayTracedShadows = bRayTracingSupported;
		bRayTracedReflections = bRayTracingSupported;
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
			for (const FScreenResolutionRHI& AvailableResolution : ResolutionArray)
			{
				Resolutions.Add(FIntPoint(AvailableResolution.Width, AvailableResolution.Height));
			}
		}
	}

	if (Resolutions.Num() == 0)
	{
		Resolutions.Add(GraphicsDefaultDisplayResolution);
		Resolutions.Add(GraphicsDefaultQHDResolution);
		Resolutions.Add(GraphicsDefaultUHDResolution);
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
	if (IConsoleVariable* MotionBlurQualityCVar = IConsoleManager::Get().FindConsoleVariable(TEXT("r.MotionBlurQuality")))
	{
		MotionBlurQualityCVar->Set(bMotionBlur ? 4 : 0, ECVF_SetByGameSetting);
	}

	if (IConsoleVariable* MotionBlurAmountCVar = IConsoleManager::Get().FindConsoleVariable(TEXT("r.MotionBlur.Amount")))
	{
		MotionBlurAmountCVar->Set(bMotionBlur ? MotionBlurAmount : 0.0f, ECVF_SetByGameSetting);
	}

	if (IConsoleVariable* ColorMidCVar = IConsoleManager::Get().FindConsoleVariable(TEXT("r.Color.Mid")))
	{
		ColorMidCVar->Set(FMath::Clamp(Brightness, 0.2f, 2.0f) * 0.5f, ECVF_SetByGameSetting);
	}

	if (IConsoleVariable* TonemapperGammaCVar = IConsoleManager::Get().FindConsoleVariable(TEXT("r.TonemapperGamma")))
	{
		TonemapperGammaCVar->Set(FMath::Clamp(Gamma, 1.0f, 3.0f), ECVF_SetByGameSetting);
	}
}

void UGraphicsSettings::DetectHardwareCapabilities()
{
	if (UGameUserSettings* UserSettings = GEngine->GetGameUserSettings())
	{
		UserSettings->LoadSettings();
	}
}
