// Copyright Epic Games, Inc. All Rights Reserved.

#include "ControlsSettingsWidget.h"
#include "ControlSettings.h"
#include "GameSettingsSubsystem.h"
#include "Components/Slider.h"
#include "Components/CheckBox.h"
#include "Kismet/GameplayStatics.h"

void UControlsSettingsWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Get control settings
	if (UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this))
	{
		if (UGameSettingsSubsystem* SettingsSubsystem = GameInstance->GetSubsystem<UGameSettingsSubsystem>())
		{
			ControlSettings = SettingsSubsystem->GetControlSettings();
		}
	}

	// Bind events
	if (MouseSensitivitySlider)
	{
		MouseSensitivitySlider->OnValueChanged.AddDynamic(this, &UControlsSettingsWidget::OnMouseSensitivityChanged);
	}

	if (InvertMouseYCheckBox)
	{
		InvertMouseYCheckBox->OnCheckStateChanged.AddDynamic(this, &UControlsSettingsWidget::OnInvertMouseYChanged);
	}

	if (MouseAccelerationCheckBox)
	{
		MouseAccelerationCheckBox->OnCheckStateChanged.AddDynamic(this, &UControlsSettingsWidget::OnMouseAccelerationChanged);
	}

	if (GamepadSensitivitySlider)
	{
		GamepadSensitivitySlider->OnValueChanged.AddDynamic(this, &UControlsSettingsWidget::OnGamepadSensitivityChanged);
	}

	if (InvertGamepadYCheckBox)
	{
		InvertGamepadYCheckBox->OnCheckStateChanged.AddDynamic(this, &UControlsSettingsWidget::OnInvertGamepadYChanged);
	}

	if (GamepadDeadzoneSlider)
	{
		GamepadDeadzoneSlider->OnValueChanged.AddDynamic(this, &UControlsSettingsWidget::OnGamepadDeadzoneChanged);
	}

	if (GamepadVibrationCheckBox)
	{
		GamepadVibrationCheckBox->OnCheckStateChanged.AddDynamic(this, &UControlsSettingsWidget::OnGamepadVibrationChanged);
	}

	if (ToggleCrouchCheckBox)
	{
		ToggleCrouchCheckBox->OnCheckStateChanged.AddDynamic(this, &UControlsSettingsWidget::OnToggleCrouchChanged);
	}

	if (ToggleSprintCheckBox)
	{
		ToggleSprintCheckBox->OnCheckStateChanged.AddDynamic(this, &UControlsSettingsWidget::OnToggleSprintChanged);
	}

	RefreshSettings();
}

void UControlsSettingsWidget::RefreshSettings()
{
	if (!ControlSettings)
	{
		return;
	}

	// Update mouse settings
	if (MouseSensitivitySlider)
	{
		MouseSensitivitySlider->SetValue(ControlSettings->MouseSensitivity);
	}

	if (InvertMouseYCheckBox)
	{
		InvertMouseYCheckBox->SetIsChecked(ControlSettings->bInvertMouseY);
	}

	if (MouseAccelerationCheckBox)
	{
		MouseAccelerationCheckBox->SetIsChecked(ControlSettings->bEnableMouseAcceleration);
	}

	// Update gamepad settings
	if (GamepadSensitivitySlider)
	{
		GamepadSensitivitySlider->SetValue(ControlSettings->GamepadSensitivity);
	}

	if (InvertGamepadYCheckBox)
	{
		InvertGamepadYCheckBox->SetIsChecked(ControlSettings->bInvertGamepadY);
	}

	if (GamepadDeadzoneSlider)
	{
		GamepadDeadzoneSlider->SetValue(ControlSettings->GamepadDeadzone);
	}

	if (GamepadVibrationCheckBox)
	{
		GamepadVibrationCheckBox->SetIsChecked(ControlSettings->bEnableGamepadVibration);
	}

	// Update accessibility settings
	if (ToggleCrouchCheckBox)
	{
		ToggleCrouchCheckBox->SetIsChecked(ControlSettings->bEnableToggleCrouch);
	}

	if (ToggleSprintCheckBox)
	{
		ToggleSprintCheckBox->SetIsChecked(ControlSettings->bEnableToggleSprint);
	}
}

void UControlsSettingsWidget::OnMouseSensitivityChanged(float Value)
{
	if (ControlSettings)
	{
		ControlSettings->MouseSensitivity = Value;
	}
}

void UControlsSettingsWidget::OnInvertMouseYChanged(bool bIsChecked)
{
	if (ControlSettings)
	{
		ControlSettings->bInvertMouseY = bIsChecked;
	}
}

void UControlsSettingsWidget::OnMouseAccelerationChanged(bool bIsChecked)
{
	if (ControlSettings)
	{
		ControlSettings->bEnableMouseAcceleration = bIsChecked;
	}
}

void UControlsSettingsWidget::OnGamepadSensitivityChanged(float Value)
{
	if (ControlSettings)
	{
		ControlSettings->GamepadSensitivity = Value;
	}
}

void UControlsSettingsWidget::OnInvertGamepadYChanged(bool bIsChecked)
{
	if (ControlSettings)
	{
		ControlSettings->bInvertGamepadY = bIsChecked;
	}
}

void UControlsSettingsWidget::OnGamepadDeadzoneChanged(float Value)
{
	if (ControlSettings)
	{
		ControlSettings->GamepadDeadzone = Value;
	}
}

void UControlsSettingsWidget::OnGamepadVibrationChanged(bool bIsChecked)
{
	if (ControlSettings)
	{
		ControlSettings->bEnableGamepadVibration = bIsChecked;
	}
}

void UControlsSettingsWidget::OnToggleCrouchChanged(bool bIsChecked)
{
	if (ControlSettings)
	{
		ControlSettings->bEnableToggleCrouch = bIsChecked;
	}
}

void UControlsSettingsWidget::OnToggleSprintChanged(bool bIsChecked)
{
	if (ControlSettings)
	{
		ControlSettings->bEnableToggleSprint = bIsChecked;
	}
}
