// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GameplaySettings.generated.h"

UENUM(BlueprintType)
enum class EDifficultyLevel : uint8
{
	Easy,
	Normal,
	Hard,
	Nightmare
};

/**
 * Gameplay Settings
 * Manages all gameplay-related settings
 */
UCLASS(BlueprintType, Config=GameUserSettings)
class HORRORPROJECT_API UGameplaySettings : public UObject
{
	GENERATED_BODY()

public:
	UGameplaySettings();

	// Difficulty
	UPROPERTY(Config, BlueprintReadWrite, Category = "Difficulty")
	EDifficultyLevel Difficulty;

	UPROPERTY(Config, BlueprintReadWrite, Category = "Difficulty")
	bool bPermadeath;

	UPROPERTY(Config, BlueprintReadWrite, Category = "Difficulty")
	float DamageMultiplier;

	UPROPERTY(Config, BlueprintReadWrite, Category = "Difficulty")
	float EnemyHealthMultiplier;

	// HUD
	UPROPERTY(Config, BlueprintReadWrite, Category = "HUD")
	bool bShowCrosshair;

	UPROPERTY(Config, BlueprintReadWrite, Category = "HUD")
	bool bShowHealthBar;

	UPROPERTY(Config, BlueprintReadWrite, Category = "HUD")
	bool bShowMinimap;

	UPROPERTY(Config, BlueprintReadWrite, Category = "HUD")
	bool bShowObjectiveMarkers;

	UPROPERTY(Config, BlueprintReadWrite, Category = "HUD")
	float HUDScale;

	UPROPERTY(Config, BlueprintReadWrite, Category = "HUD")
	float HUDOpacity;

	// Gameplay Assists
	UPROPERTY(Config, BlueprintReadWrite, Category = "Assists")
	bool bEnableAutoSave;

	UPROPERTY(Config, BlueprintReadWrite, Category = "Assists")
	float AutoSaveInterval;

	UPROPERTY(Config, BlueprintReadWrite, Category = "Assists")
	bool bEnableQuickTimeEvents;

	UPROPERTY(Config, BlueprintReadWrite, Category = "Assists")
	bool bEnableTutorialHints;

	UPROPERTY(Config, BlueprintReadWrite, Category = "Assists")
	bool bEnableObjectiveHints;

	// Camera
	UPROPERTY(Config, BlueprintReadWrite, Category = "Camera")
	float FieldOfView;

	UPROPERTY(Config, BlueprintReadWrite, Category = "Camera")
	bool bEnableCameraShake;

	UPROPERTY(Config, BlueprintReadWrite, Category = "Camera")
	float CameraShakeIntensity;

	UPROPERTY(Config, BlueprintReadWrite, Category = "Camera")
	bool bEnableHeadBob;

	// Language
	UPROPERTY(Config, BlueprintReadWrite, Category = "Language")
	FString Language;

	UPROPERTY(Config, BlueprintReadWrite, Category = "Language")
	FString VoiceLanguage;

	// Accessibility
	UPROPERTY(Config, BlueprintReadWrite, Category = "Accessibility")
	bool bColorBlindMode;

	UPROPERTY(Config, BlueprintReadWrite, Category = "Accessibility")
	int32 ColorBlindType;

	UPROPERTY(Config, BlueprintReadWrite, Category = "Accessibility")
	bool bReduceFlashing;

	UPROPERTY(Config, BlueprintReadWrite, Category = "Accessibility")
	bool bReduceMotion;

	UPROPERTY(Config, BlueprintReadWrite, Category = "Accessibility")
	float TextSize;

	// Operations
	UFUNCTION(BlueprintCallable, Category = "Gameplay")
	void Apply();

	UFUNCTION(BlueprintCallable, Category = "Gameplay")
	void SetDifficulty(EDifficultyLevel NewDifficulty);

	UFUNCTION(BlueprintPure, Category = "Gameplay")
	TArray<FString> GetAvailableLanguages() const;

	UFUNCTION(BlueprintCallable, Category = "Gameplay")
	void SetLanguage(const FString& NewLanguage);

private:
	void ApplyDifficultySettings();
	void ApplyHUDSettings();
	void ApplyCameraSettings();
	void ApplyLanguageSettings();
};
