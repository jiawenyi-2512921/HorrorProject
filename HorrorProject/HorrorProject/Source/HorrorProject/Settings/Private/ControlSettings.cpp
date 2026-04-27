// Copyright Epic Games, Inc. All Rights Reserved.

#include "ControlSettings.h"
#include "GameFramework/PlayerInput.h"
#include "GameFramework/InputSettings.h"

namespace
{
	struct FDefaultKeyBindingSpec
	{
		FName ActionName;
		FKey PrimaryKey;
		FKey SecondaryKey = EKeys::Invalid;
	};

	const FDefaultKeyBindingSpec DefaultKeyBindings[] =
	{
		{ TEXT("MoveForward"), EKeys::W, EKeys::Up },
		{ TEXT("MoveBackward"), EKeys::S, EKeys::Down },
		{ TEXT("MoveLeft"), EKeys::A, EKeys::Left },
		{ TEXT("MoveRight"), EKeys::D, EKeys::Right },
		{ TEXT("Jump"), EKeys::SpaceBar },
		{ TEXT("Crouch"), EKeys::LeftControl, EKeys::C },
		{ TEXT("Sprint"), EKeys::LeftShift },
		{ TEXT("Interact"), EKeys::E },
		{ TEXT("Fire"), EKeys::LeftMouseButton },
		{ TEXT("Aim"), EKeys::RightMouseButton },
		{ TEXT("Reload"), EKeys::R },
		{ TEXT("Inventory"), EKeys::Tab, EKeys::I },
		{ TEXT("Pause"), EKeys::Escape },
	};
}

UControlSettings::UControlSettings()
{
	MouseSensitivity = 1.0f;
	bInvertMouseY = false;
	bInvertMouseX = false;
	MouseSmoothing = 0.0f;
	bEnableMouseAcceleration = false;

	GamepadSensitivity = 1.0f;
	bInvertGamepadY = false;
	bInvertGamepadX = false;
	GamepadDeadzone = 0.25f;
	bEnableGamepadVibration = true;
	VibrationIntensity = 1.0f;

	bEnableToggleCrouch = false;
	bEnableToggleSprint = false;
	bEnableAutoAim = false;
	HoldButtonDuration = 0.5f;

	InitializeDefaultBindings();
}

void UControlSettings::Apply()
{
	ApplyMouseSettings();
	ApplyGamepadSettings();
	ApplyKeyBindings();
}

void UControlSettings::ResetToDefaults()
{
	MouseSensitivity = 1.0f;
	bInvertMouseY = false;
	bInvertMouseX = false;
	GamepadSensitivity = 1.0f;
	bInvertGamepadY = false;
	bInvertGamepadX = false;

	InitializeDefaultBindings();
	Apply();
}

bool UControlSettings::SetKeyBinding(FName ActionName, FKey NewKey, bool bPrimary)
{
	if (IsKeyConflicting(NewKey, ActionName))
	{
		return false;
	}

	for (FKeyBinding& Binding : KeyBindings)
	{
		if (Binding.ActionName == ActionName)
		{
			if (bPrimary)
			{
				Binding.PrimaryKey = NewKey;
			}
			else
			{
				Binding.SecondaryKey = NewKey;
			}
			return true;
		}
	}

	FKeyBinding NewBinding;
	NewBinding.ActionName = ActionName;
	if (bPrimary)
	{
		NewBinding.PrimaryKey = NewKey;
	}
	else
	{
		NewBinding.SecondaryKey = NewKey;
	}
	KeyBindings.Add(NewBinding);

	return true;
}

FKeyBinding UControlSettings::GetKeyBinding(FName ActionName) const
{
	for (const FKeyBinding& Binding : KeyBindings)
	{
		if (Binding.ActionName == ActionName)
		{
			return Binding;
		}
	}

	return FKeyBinding();
}

bool UControlSettings::IsKeyConflicting(FKey Key, FName ExcludeAction) const
{
	for (const FKeyBinding& Binding : KeyBindings)
	{
		if (Binding.ActionName != ExcludeAction)
		{
			if (Binding.PrimaryKey == Key || Binding.SecondaryKey == Key)
			{
				return true;
			}
		}
	}

	return false;
}

void UControlSettings::ClearKeyBinding(FName ActionName, bool bPrimary)
{
	for (FKeyBinding& Binding : KeyBindings)
	{
		if (Binding.ActionName == ActionName)
		{
			if (bPrimary)
			{
				Binding.PrimaryKey = EKeys::Invalid;
			}
			else
			{
				Binding.SecondaryKey = EKeys::Invalid;
			}
			break;
		}
	}
}

void UControlSettings::InitializeDefaultBindings()
{
	KeyBindings.Empty();

	for (const FDefaultKeyBindingSpec& Spec : DefaultKeyBindings)
	{
		FKeyBinding Binding;
		Binding.ActionName = Spec.ActionName;
		Binding.PrimaryKey = Spec.PrimaryKey;
		Binding.SecondaryKey = Spec.SecondaryKey;
		KeyBindings.Add(Binding);
	}
}

void UControlSettings::ApplyMouseSettings()
{
	// Apply mouse settings to player controller
}

void UControlSettings::ApplyGamepadSettings()
{
	// Apply gamepad settings to player controller
}

void UControlSettings::ApplyKeyBindings()
{
	// Apply key bindings to input system
	if (UInputSettings* InputSettings = UInputSettings::GetInputSettings())
	{
		// Update action mappings
	}
}
