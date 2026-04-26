// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ControlsSettingsWidget.generated.h"

class UControlSettings;
class USlider;
class UCheckBox;

/**
 * Controls Settings Widget
 * UI for control settings
 */
UCLASS()
class HORRORPROJECT_API UControlsSettingsWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	UFUNCTION(BlueprintCallable, Category = "Settings")
	void RefreshSettings();

protected:
	// Mouse settings
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USlider> MouseSensitivitySlider;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCheckBox> InvertMouseYCheckBox;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCheckBox> MouseAccelerationCheckBox;

	// Gamepad settings
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USlider> GamepadSensitivitySlider;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCheckBox> InvertGamepadYCheckBox;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USlider> GamepadDeadzoneSlider;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCheckBox> GamepadVibrationCheckBox;

	// Accessibility
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCheckBox> ToggleCrouchCheckBox;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCheckBox> ToggleSprintCheckBox;

	// Callbacks
	UFUNCTION()
	void OnMouseSensitivityChanged(float Value);

	UFUNCTION()
	void OnInvertMouseYChanged(bool bIsChecked);

	UFUNCTION()
	void OnMouseAccelerationChanged(bool bIsChecked);

	UFUNCTION()
	void OnGamepadSensitivityChanged(float Value);

	UFUNCTION()
	void OnInvertGamepadYChanged(bool bIsChecked);

	UFUNCTION()
	void OnGamepadDeadzoneChanged(float Value);

	UFUNCTION()
	void OnGamepadVibrationChanged(bool bIsChecked);

	UFUNCTION()
	void OnToggleCrouchChanged(bool bIsChecked);

	UFUNCTION()
	void OnToggleSprintChanged(bool bIsChecked);

private:
	UPROPERTY()
	TObjectPtr<UControlSettings> ControlSettings;
};
