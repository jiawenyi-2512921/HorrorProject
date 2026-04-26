// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "InputAction.h"
#include "ControlSettings.generated.h"

USTRUCT(BlueprintType)
struct FKeyBinding
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Input")
	FName ActionName;

	UPROPERTY(BlueprintReadWrite, Category = "Input")
	FKey PrimaryKey;

	UPROPERTY(BlueprintReadWrite, Category = "Input")
	FKey SecondaryKey;

	UPROPERTY(BlueprintReadWrite, Category = "Input")
	bool bShift;

	UPROPERTY(BlueprintReadWrite, Category = "Input")
	bool bCtrl;

	UPROPERTY(BlueprintReadWrite, Category = "Input")
	bool bAlt;

	FKeyBinding()
		: ActionName(NAME_None)
		, PrimaryKey(EKeys::Invalid)
		, SecondaryKey(EKeys::Invalid)
		, bShift(false)
		, bCtrl(false)
		, bAlt(false)
	{}
};

/**
 * Control Settings
 * Manages all input and control-related settings
 */
UCLASS(BlueprintType, Config=GameUserSettings)
class HORRORPROJECT_API UControlSettings : public UObject
{
	GENERATED_BODY()

public:
	UControlSettings();

	// Mouse Settings
	UPROPERTY(Config, BlueprintReadWrite, Category = "Mouse")
	float MouseSensitivity;

	UPROPERTY(Config, BlueprintReadWrite, Category = "Mouse")
	bool bInvertMouseY;

	UPROPERTY(Config, BlueprintReadWrite, Category = "Mouse")
	bool bInvertMouseX;

	UPROPERTY(Config, BlueprintReadWrite, Category = "Mouse")
	float MouseSmoothing;

	UPROPERTY(Config, BlueprintReadWrite, Category = "Mouse")
	bool bEnableMouseAcceleration;

	// Gamepad Settings
	UPROPERTY(Config, BlueprintReadWrite, Category = "Gamepad")
	float GamepadSensitivity;

	UPROPERTY(Config, BlueprintReadWrite, Category = "Gamepad")
	bool bInvertGamepadY;

	UPROPERTY(Config, BlueprintReadWrite, Category = "Gamepad")
	bool bInvertGamepadX;

	UPROPERTY(Config, BlueprintReadWrite, Category = "Gamepad")
	float GamepadDeadzone;

	UPROPERTY(Config, BlueprintReadWrite, Category = "Gamepad")
	bool bEnableGamepadVibration;

	UPROPERTY(Config, BlueprintReadWrite, Category = "Gamepad")
	float VibrationIntensity;

	// Key Bindings
	UPROPERTY(Config, BlueprintReadWrite, Category = "Bindings")
	TArray<FKeyBinding> KeyBindings;

	// Accessibility
	UPROPERTY(Config, BlueprintReadWrite, Category = "Accessibility")
	bool bEnableToggleCrouch;

	UPROPERTY(Config, BlueprintReadWrite, Category = "Accessibility")
	bool bEnableToggleSprint;

	UPROPERTY(Config, BlueprintReadWrite, Category = "Accessibility")
	bool bEnableAutoAim;

	UPROPERTY(Config, BlueprintReadWrite, Category = "Accessibility")
	float HoldButtonDuration;

	// Operations
	UFUNCTION(BlueprintCallable, Category = "Controls")
	void Apply();

	UFUNCTION(BlueprintCallable, Category = "Controls")
	void ResetToDefaults();

	UFUNCTION(BlueprintCallable, Category = "Controls")
	bool SetKeyBinding(FName ActionName, FKey NewKey, bool bPrimary = true);

	UFUNCTION(BlueprintPure, Category = "Controls")
	FKeyBinding GetKeyBinding(FName ActionName) const;

	UFUNCTION(BlueprintPure, Category = "Controls")
	bool IsKeyConflicting(FKey Key, FName ExcludeAction = NAME_None) const;

	UFUNCTION(BlueprintCallable, Category = "Controls")
	void ClearKeyBinding(FName ActionName, bool bPrimary = true);

	UFUNCTION(BlueprintPure, Category = "Controls")
	TArray<FKeyBinding> GetAllKeyBindings() const { return KeyBindings; }

private:
	void InitializeDefaultBindings();
	void ApplyMouseSettings();
	void ApplyGamepadSettings();
	void ApplyKeyBindings();
};
