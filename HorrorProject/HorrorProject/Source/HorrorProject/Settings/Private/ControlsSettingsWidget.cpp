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

	ResolveControlSettings();
	BindMouseControls();
	BindGamepadControls();
	BindAccessibilityControls();
	RefreshSettings();
}

void UControlsSettingsWidget::ResolveControlSettings()
{
	if (UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this))
	{
		if (UGameSettingsSubsystem* SettingsSubsystem = GameInstance->GetSubsystem<UGameSettingsSubsystem>())
		{
			ControlSettings = SettingsSubsystem->GetControlSettings();
		}
	}
}

void UControlsSettingsWidget::BindMouseControls()
{
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
}

void UControlsSettingsWidget::BindGamepadControls()
{
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
}

void UControlsSettingsWidget::BindAccessibilityControls()
{
	if (ToggleCrouchCheckBox)
	{
		ToggleCrouchCheckBox->OnCheckStateChanged.AddDynamic(this, &UControlsSettingsWidget::OnToggleCrouchChanged);
	}

	if (ToggleSprintCheckBox)
	{
		ToggleSprintCheckBox->OnCheckStateChanged.AddDynamic(this, &UControlsSettingsWidget::OnToggleSprintChanged);
	}
}

void UControlsSettingsWidget::RefreshSettings()
{
	if (!ControlSettings)
	{
		return;
	}

	RefreshMouseControls();
	RefreshGamepadControls();
	RefreshAccessibilityControls();
}

void UControlsSettingsWidget::RefreshMouseControls()
{
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
}

void UControlsSettingsWidget::RefreshGamepadControls()
{
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
}

void UControlsSettingsWidget::RefreshAccessibilityControls()
{
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
