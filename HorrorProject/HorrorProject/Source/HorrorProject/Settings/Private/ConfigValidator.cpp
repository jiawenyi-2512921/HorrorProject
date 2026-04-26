// Copyright Epic Games, Inc. All Rights Reserved.

#include "ConfigValidator.h"
#include "GraphicsSettings.h"
#include "AudioSettings.h"
#include "ControlSettings.h"
#include "GameplaySettings.h"

bool UConfigValidator::ValidateGraphicsSettings(UGraphicsSettings* Settings)
{
	ClearResults();

	if (!Settings)
	{
		AddError(TEXT("Graphics settings object is null"));
		return false;
	}

	// Validate resolution
	if (!ValidateResolution(Settings->Resolution))
	{
		AddError(FString::Printf(TEXT("Invalid resolution: %dx%d"),
			Settings->Resolution.X, Settings->Resolution.Y));
	}

	// Validate frame rate limit
	if (Settings->FrameRateLimit < 0 || Settings->FrameRateLimit > 300)
	{
		AddWarning(TEXT("Frame rate limit should be between 0 and 300"));
	}

	// Validate quality settings
	if (Settings->ViewDistanceQuality < 0 || Settings->ViewDistanceQuality > 4)
	{
		AddError(TEXT("View distance quality out of range (0-4)"));
	}

	if (Settings->TextureQuality < 0 || Settings->TextureQuality > 4)
	{
		AddError(TEXT("Texture quality out of range (0-4)"));
	}

	// Validate brightness and gamma
	if (Settings->Brightness < 0.0f || Settings->Brightness > 2.0f)
	{
		AddWarning(TEXT("Brightness should be between 0.0 and 2.0"));
	}

	if (Settings->Gamma < 1.0f || Settings->Gamma > 3.0f)
	{
		AddWarning(TEXT("Gamma should be between 1.0 and 3.0"));
	}

	// Validate ray tracing
	if (Settings->bRayTracingEnabled && !Settings->IsRayTracingSupported())
	{
		AddWarning(TEXT("Ray tracing is not supported on this hardware"));
		Settings->bRayTracingEnabled = false;
	}

	return LastValidationResult.bIsValid;
}

bool UConfigValidator::ValidateAudioSettings(UAudioSettings* Settings)
{
	ClearResults();

	if (!Settings)
	{
		AddError(TEXT("Audio settings object is null"));
		return false;
	}

	// Validate volumes
	if (!ValidateVolumeRange(Settings->MasterVolume))
	{
		AddError(TEXT("Master volume out of range (0.0-1.0)"));
	}

	if (!ValidateVolumeRange(Settings->MusicVolume))
	{
		AddError(TEXT("Music volume out of range (0.0-1.0)"));
	}

	if (!ValidateVolumeRange(Settings->SFXVolume))
	{
		AddError(TEXT("SFX volume out of range (0.0-1.0)"));
	}

	if (!ValidateVolumeRange(Settings->DialogueVolume))
	{
		AddError(TEXT("Dialogue volume out of range (0.0-1.0)"));
	}

	// Validate quality settings
	if (Settings->AudioQuality < 0 || Settings->AudioQuality > 4)
	{
		AddError(TEXT("Audio quality out of range (0-4)"));
	}

	if (Settings->MaxChannels < 8 || Settings->MaxChannels > 128)
	{
		AddWarning(TEXT("Max channels should be between 8 and 128"));
	}

	// Validate spatial audio
	if (Settings->bEnableSpatialAudio && !Settings->IsSpatialAudioSupported())
	{
		AddWarning(TEXT("Spatial audio is not supported on this hardware"));
		Settings->bEnableSpatialAudio = false;
	}

	return LastValidationResult.bIsValid;
}

bool UConfigValidator::ValidateControlSettings(UControlSettings* Settings)
{
	ClearResults();

	if (!Settings)
	{
		AddError(TEXT("Control settings object is null"));
		return false;
	}

	// Validate sensitivity
	if (Settings->MouseSensitivity < 0.1f || Settings->MouseSensitivity > 10.0f)
	{
		AddWarning(TEXT("Mouse sensitivity should be between 0.1 and 10.0"));
	}

	if (Settings->GamepadSensitivity < 0.1f || Settings->GamepadSensitivity > 10.0f)
	{
		AddWarning(TEXT("Gamepad sensitivity should be between 0.1 and 10.0"));
	}

	// Validate deadzone
	if (Settings->GamepadDeadzone < 0.0f || Settings->GamepadDeadzone > 1.0f)
	{
		AddError(TEXT("Gamepad deadzone out of range (0.0-1.0)"));
	}

	// Validate vibration intensity
	if (Settings->VibrationIntensity < 0.0f || Settings->VibrationIntensity > 1.0f)
	{
		AddError(TEXT("Vibration intensity out of range (0.0-1.0)"));
	}

	// Validate key bindings
	if (!ValidateKeyBindings(Settings))
	{
		AddError(TEXT("Invalid key bindings detected"));
	}

	return LastValidationResult.bIsValid;
}

bool UConfigValidator::ValidateGameplaySettings(UGameplaySettings* Settings)
{
	ClearResults();

	if (!Settings)
	{
		AddError(TEXT("Gameplay settings object is null"));
		return false;
	}

	// Validate FOV
	if (!ValidateFOV(Settings->FieldOfView))
	{
		AddError(TEXT("Field of view out of range (60-120)"));
	}

	// Validate HUD settings
	if (Settings->HUDScale < 0.5f || Settings->HUDScale > 2.0f)
	{
		AddWarning(TEXT("HUD scale should be between 0.5 and 2.0"));
	}

	if (Settings->HUDOpacity < 0.0f || Settings->HUDOpacity > 1.0f)
	{
		AddError(TEXT("HUD opacity out of range (0.0-1.0)"));
	}

	// Validate auto-save interval
	if (Settings->bEnableAutoSave && Settings->AutoSaveInterval < 60.0f)
	{
		AddWarning(TEXT("Auto-save interval should be at least 60 seconds"));
	}

	// Validate camera shake
	if (Settings->CameraShakeIntensity < 0.0f || Settings->CameraShakeIntensity > 1.0f)
	{
		AddError(TEXT("Camera shake intensity out of range (0.0-1.0)"));
	}

	// Validate text size
	if (Settings->TextSize < 0.5f || Settings->TextSize > 2.0f)
	{
		AddWarning(TEXT("Text size should be between 0.5 and 2.0"));
	}

	return LastValidationResult.bIsValid;
}

FValidationResult UConfigValidator::ValidateAllSettings(UGraphicsSettings* Graphics,
	UAudioSettings* Audio, UControlSettings* Controls, UGameplaySettings* Gameplay)
{
	ClearResults();

	ValidateGraphicsSettings(Graphics);
	ValidateAudioSettings(Audio);
	ValidateControlSettings(Controls);
	ValidateGameplaySettings(Gameplay);

	return LastValidationResult;
}

bool UConfigValidator::ValidateResolution(const FIntPoint& Resolution)
{
	return Resolution.X >= 640 && Resolution.X <= 7680 &&
		   Resolution.Y >= 480 && Resolution.Y <= 4320;
}

bool UConfigValidator::ValidateVolumeRange(float Volume)
{
	return Volume >= 0.0f && Volume <= 1.0f;
}

bool UConfigValidator::ValidateKeyBindings(UControlSettings* Settings)
{
	if (!Settings)
	{
		return false;
	}

	TSet<FKey> UsedKeys;

	for (const FKeyBinding& Binding : Settings->GetAllKeyBindings())
	{
		if (Binding.PrimaryKey.IsValid())
		{
			if (UsedKeys.Contains(Binding.PrimaryKey))
			{
				AddWarning(FString::Printf(TEXT("Duplicate key binding: %s"),
					*Binding.PrimaryKey.ToString()));
			}
			UsedKeys.Add(Binding.PrimaryKey);
		}

		if (Binding.SecondaryKey.IsValid())
		{
			if (UsedKeys.Contains(Binding.SecondaryKey))
			{
				AddWarning(FString::Printf(TEXT("Duplicate key binding: %s"),
					*Binding.SecondaryKey.ToString()));
			}
			UsedKeys.Add(Binding.SecondaryKey);
		}
	}

	return true;
}

bool UConfigValidator::ValidateFOV(float FOV)
{
	return FOV >= 60.0f && FOV <= 120.0f;
}

void UConfigValidator::AddError(const FString& Error)
{
	LastValidationResult.Errors.Add(Error);
	LastValidationResult.bIsValid = false;
}

void UConfigValidator::AddWarning(const FString& Warning)
{
	LastValidationResult.Warnings.Add(Warning);
}

void UConfigValidator::ClearResults()
{
	LastValidationResult = FValidationResult();
}
