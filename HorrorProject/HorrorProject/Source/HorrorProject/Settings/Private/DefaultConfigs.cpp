// Copyright Epic Games, Inc. All Rights Reserved.

#include "DefaultConfigs.h"
#include "GraphicsSettings.h"
#include "HorrorAudioSettings.h"
#include "ControlSettings.h"
#include "GameplaySettings.h"
#include "GameFramework/GameUserSettings.h"
#include "Engine/Engine.h"

namespace
{
	const FIntPoint DefaultConfigDisplayResolution(1920, 1080);
	constexpr float DefaultConfigShadowDistance = 5000.0f;
	constexpr int32 DefaultConfigMaxAudioChannels = 32;
	constexpr float DefaultConfigAutoSaveIntervalSeconds = 300.0f;
	constexpr float DefaultConfigFieldOfView = 90.0f;
}

void UDefaultConfigs::ApplyDefaultGraphicsSettings(UGraphicsSettings* Settings)
{
	if (!Settings)
	{
		return;
	}

	Settings->Resolution = DefaultConfigDisplayResolution;
	Settings->bFullscreen = true;
	Settings->bVSync = false;
	Settings->FrameRateLimit = 0;

	Settings->QualityPreset = EQualityPreset::High;
	Settings->ViewDistanceQuality = 3;
	Settings->TextureQuality = 3;
	Settings->EffectsQuality = 3;
	Settings->PostProcessQuality = 3;
	Settings->FoliageQuality = 3;
	Settings->ShadingQuality = 3;

	Settings->ShadowQuality = EShadowQuality::High;
	Settings->bDynamicShadows = true;
	Settings->ShadowDistance = DefaultConfigShadowDistance;

	Settings->AntiAliasingMethod = EHorrorAntiAliasingMethod::TAA;
	Settings->AntiAliasingQuality = 3;

	Settings->bMotionBlur = true;
	Settings->MotionBlurAmount = 0.5f;
	Settings->bAmbientOcclusion = true;
	Settings->bBloom = true;
	Settings->bLensFlares = true;
	Settings->Brightness = 1.0f;
	Settings->Gamma = 2.2f;

	Settings->bRayTracingEnabled = false;
	Settings->bRayTracedShadows = false;
	Settings->bRayTracedReflections = false;
	Settings->bRayTracedGlobalIllumination = false;
}

void UDefaultConfigs::ApplyDefaultAudioSettings(UHorrorAudioSettings* Settings)
{
	if (!Settings)
	{
		return;
	}

	Settings->MasterVolume = 1.0f;
	Settings->MusicVolume = 0.8f;
	Settings->SFXVolume = 1.0f;
	Settings->DialogueVolume = 1.0f;
	Settings->AmbienceVolume = 0.7f;
	Settings->UIVolume = 0.9f;

	Settings->AudioQuality = 3;
	Settings->bEnableReverb = true;
	Settings->bEnableOcclusion = true;
	Settings->MaxChannels = DefaultConfigMaxAudioChannels;

	Settings->bEnableSpatialAudio = true;
	Settings->bEnableHRTF = false;
	Settings->bEnableAttenuation = true;

	Settings->OutputDevice = TEXT("Default");
	Settings->bEnableSurround = false;
	Settings->SpeakerConfiguration = 2;

	Settings->bEnableSubtitles = true;
	Settings->SubtitleSize = 1.0f;
	Settings->bEnableClosedCaptions = false;
	Settings->bEnableAudioDescription = false;
}

void UDefaultConfigs::ApplyDefaultControlSettings(UControlSettings* Settings)
{
	if (!Settings)
	{
		return;
	}

	Settings->MouseSensitivity = 1.0f;
	Settings->bInvertMouseY = false;
	Settings->bInvertMouseX = false;
	Settings->MouseSmoothing = 0.0f;
	Settings->bEnableMouseAcceleration = false;

	Settings->GamepadSensitivity = 1.0f;
	Settings->bInvertGamepadY = false;
	Settings->bInvertGamepadX = false;
	Settings->GamepadDeadzone = 0.25f;
	Settings->bEnableGamepadVibration = true;
	Settings->VibrationIntensity = 1.0f;

	Settings->bEnableToggleCrouch = false;
	Settings->bEnableToggleSprint = false;
	Settings->bEnableAutoAim = false;
	Settings->HoldButtonDuration = 0.5f;
}

void UDefaultConfigs::ApplyDefaultGameplaySettings(UGameplaySettings* Settings)
{
	if (!Settings)
	{
		return;
	}

	Settings->Difficulty = EDifficultyLevel::Normal;
	Settings->bPermadeath = false;
	Settings->DamageMultiplier = 1.0f;
	Settings->EnemyHealthMultiplier = 1.0f;

	Settings->bShowCrosshair = true;
	Settings->bShowHealthBar = true;
	Settings->bShowMinimap = true;
	Settings->bShowObjectiveMarkers = true;
	Settings->HUDScale = 1.0f;
	Settings->HUDOpacity = 1.0f;

	Settings->bEnableAutoSave = true;
	Settings->AutoSaveInterval = DefaultConfigAutoSaveIntervalSeconds;
	Settings->bEnableQuickTimeEvents = true;
	Settings->bEnableTutorialHints = true;
	Settings->bEnableObjectiveHints = true;

	Settings->FieldOfView = DefaultConfigFieldOfView;
	Settings->bEnableCameraShake = true;
	Settings->CameraShakeIntensity = 1.0f;
	Settings->bEnableHeadBob = true;

	Settings->Language = TEXT("zh-Hans");
	Settings->VoiceLanguage = TEXT("zh-Hans");

	Settings->bColorBlindMode = false;
	Settings->ColorBlindType = 0;
	Settings->bReduceFlashing = false;
	Settings->bReduceMotion = false;
	Settings->TextSize = 1.0f;
}

void UDefaultConfigs::ApplyRecommendedGraphicsSettings(UGraphicsSettings* Settings)
{
	if (!Settings)
	{
		return;
	}

	int32 QualityLevel = DetectRecommendedQualityLevel();
	Settings->ApplyQualityPreset(QualityLevel);
}

void UDefaultConfigs::ApplyRecommendedAudioSettings(UHorrorAudioSettings* Settings)
{
	if (!Settings)
	{
		return;
	}

	ApplyDefaultAudioSettings(Settings);
	Settings->AutoDetect();
}

int32 UDefaultConfigs::DetectRecommendedQualityLevel()
{
	if (UGameUserSettings* UserSettings = GEngine->GetGameUserSettings())
	{
		UserSettings->RunHardwareBenchmark();
		return FMath::Clamp(UserSettings->GetRecommendedResolutionScale(), 0, 4);
	}

	return 2; // Default to Medium
}
