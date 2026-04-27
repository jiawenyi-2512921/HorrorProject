// Copyright Epic Games, Inc. All Rights Reserved.

#include "GameplaySettings.h"
#include "Internationalization/Culture.h"
#include "Internationalization/Internationalization.h"

namespace
{
	constexpr float GameplayDefaultAutoSaveIntervalSeconds = 300.0f;
	constexpr float GameplayDefaultFieldOfView = 90.0f;
}

UGameplaySettings::UGameplaySettings()
{
	Difficulty = EDifficultyLevel::Normal;
	bPermadeath = false;
	DamageMultiplier = 1.0f;
	EnemyHealthMultiplier = 1.0f;

	bShowCrosshair = true;
	bShowHealthBar = true;
	bShowMinimap = true;
	bShowObjectiveMarkers = true;
	HUDScale = 1.0f;
	HUDOpacity = 1.0f;

	bEnableAutoSave = true;
	AutoSaveInterval = GameplayDefaultAutoSaveIntervalSeconds;
	bEnableQuickTimeEvents = true;
	bEnableTutorialHints = true;
	bEnableObjectiveHints = true;

	FieldOfView = GameplayDefaultFieldOfView;
	bEnableCameraShake = true;
	CameraShakeIntensity = 1.0f;
	bEnableHeadBob = true;

	Language = TEXT("en");
	VoiceLanguage = TEXT("en");

	bColorBlindMode = false;
	ColorBlindType = 0;
	bReduceFlashing = false;
	bReduceMotion = false;
	TextSize = 1.0f;
}

void UGameplaySettings::Apply()
{
	ApplyDifficultySettings();
	ApplyHUDSettings();
	ApplyCameraSettings();
	ApplyLanguageSettings();
}

void UGameplaySettings::SetDifficulty(EDifficultyLevel NewDifficulty)
{
	Difficulty = NewDifficulty;

	switch (Difficulty)
	{
	case EDifficultyLevel::Easy:
		DamageMultiplier = 0.5f;
		EnemyHealthMultiplier = 0.75f;
		break;

	case EDifficultyLevel::Normal:
		DamageMultiplier = 1.0f;
		EnemyHealthMultiplier = 1.0f;
		break;

	case EDifficultyLevel::Hard:
		DamageMultiplier = 1.5f;
		EnemyHealthMultiplier = 1.5f;
		break;

	case EDifficultyLevel::Nightmare:
		DamageMultiplier = 2.0f;
		EnemyHealthMultiplier = 2.0f;
		break;
	}

	ApplyDifficultySettings();
}

TArray<FString> UGameplaySettings::GetAvailableLanguages() const
{
	TArray<FString> Languages;
	Languages.Add(TEXT("en"));
	Languages.Add(TEXT("zh"));
	Languages.Add(TEXT("ja"));
	Languages.Add(TEXT("ko"));
	Languages.Add(TEXT("es"));
	Languages.Add(TEXT("fr"));
	Languages.Add(TEXT("de"));
	Languages.Add(TEXT("ru"));
	return Languages;
}

void UGameplaySettings::SetLanguage(const FString& NewLanguage)
{
	Language = NewLanguage;
	ApplyLanguageSettings();
}

void UGameplaySettings::ApplyDifficultySettings()
{
	// Apply difficulty settings to game state
}

void UGameplaySettings::ApplyHUDSettings()
{
	// Apply HUD settings to UI
}

void UGameplaySettings::ApplyCameraSettings()
{
	// Apply camera settings to player camera
}

void UGameplaySettings::ApplyLanguageSettings()
{
	FInternationalization::Get().SetCurrentCulture(Language);
}
