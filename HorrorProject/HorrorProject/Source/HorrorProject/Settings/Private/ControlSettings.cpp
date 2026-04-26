// Copyright Epic Games, Inc. All Rights Reserved.

#include "ControlSettings.h"
#include "GameFramework/PlayerInput.h"
#include "GameFramework/InputSettings.h"

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

	// Movement
	FKeyBinding MoveForward;
	MoveForward.ActionName = TEXT("MoveForward");
	MoveForward.PrimaryKey = EKeys::W;
	MoveForward.SecondaryKey = EKeys::Up;
	KeyBindings.Add(MoveForward);

	FKeyBinding MoveBackward;
	MoveBackward.ActionName = TEXT("MoveBackward");
	MoveBackward.PrimaryKey = EKeys::S;
	MoveBackward.SecondaryKey = EKeys::Down;
	KeyBindings.Add(MoveBackward);

	FKeyBinding MoveLeft;
	MoveLeft.ActionName = TEXT("MoveLeft");
	MoveLeft.PrimaryKey = EKeys::A;
	MoveLeft.SecondaryKey = EKeys::Left;
	KeyBindings.Add(MoveLeft);

	FKeyBinding MoveRight;
	MoveRight.ActionName = TEXT("MoveRight");
	MoveRight.PrimaryKey = EKeys::D;
	MoveRight.SecondaryKey = EKeys::Right;
	KeyBindings.Add(MoveRight);

	// Actions
	FKeyBinding Jump;
	Jump.ActionName = TEXT("Jump");
	Jump.PrimaryKey = EKeys::SpaceBar;
	KeyBindings.Add(Jump);

	FKeyBinding Crouch;
	Crouch.ActionName = TEXT("Crouch");
	Crouch.PrimaryKey = EKeys::LeftControl;
	Crouch.SecondaryKey = EKeys::C;
	KeyBindings.Add(Crouch);

	FKeyBinding Sprint;
	Sprint.ActionName = TEXT("Sprint");
	Sprint.PrimaryKey = EKeys::LeftShift;
	KeyBindings.Add(Sprint);

	FKeyBinding Interact;
	Interact.ActionName = TEXT("Interact");
	Interact.PrimaryKey = EKeys::E;
	KeyBindings.Add(Interact);

	// Combat
	FKeyBinding Fire;
	Fire.ActionName = TEXT("Fire");
	Fire.PrimaryKey = EKeys::LeftMouseButton;
	KeyBindings.Add(Fire);

	FKeyBinding Aim;
	Aim.ActionName = TEXT("Aim");
	Aim.PrimaryKey = EKeys::RightMouseButton;
	KeyBindings.Add(Aim);

	FKeyBinding Reload;
	Reload.ActionName = TEXT("Reload");
	Reload.PrimaryKey = EKeys::R;
	KeyBindings.Add(Reload);

	// UI
	FKeyBinding Inventory;
	Inventory.ActionName = TEXT("Inventory");
	Inventory.PrimaryKey = EKeys::Tab;
	Inventory.SecondaryKey = EKeys::I;
	KeyBindings.Add(Inventory);

	FKeyBinding Pause;
	Pause.ActionName = TEXT("Pause");
	Pause.PrimaryKey = EKeys::Escape;
	KeyBindings.Add(Pause);
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
