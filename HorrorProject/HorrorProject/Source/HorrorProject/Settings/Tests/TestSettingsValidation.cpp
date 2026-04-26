// Copyright Epic Games, Inc. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "ConfigValidator.h"
#include "GraphicsSettings.h"
#include "AudioSettings.h"
#include "ControlSettings.h"
#include "GameplaySettings.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSettingsValidationTest, "HorrorProject.Settings.Validation", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FSettingsValidationTest::RunTest(const FString& Parameters)
{
	UConfigValidator* Validator = NewObject<UConfigValidator>();
	TestNotNull(TEXT("Validator created"), Validator);

	// Test graphics settings validation
	UGraphicsSettings* Graphics = NewObject<UGraphicsSettings>();
	Graphics->Resolution = FIntPoint(1920, 1080);
	Graphics->ViewDistanceQuality = 3;
	Graphics->Brightness = 1.0f;

	bool bValid = Validator->ValidateGraphicsSettings(Graphics);
	TestTrue(TEXT("Valid graphics settings"), bValid);

	// Test invalid resolution
	Graphics->Resolution = FIntPoint(100, 100);
	bValid = Validator->ValidateGraphicsSettings(Graphics);
	TestFalse(TEXT("Invalid resolution detected"), bValid);

	// Test invalid quality setting
	Graphics->Resolution = FIntPoint(1920, 1080);
	Graphics->ViewDistanceQuality = 10;
	bValid = Validator->ValidateGraphicsSettings(Graphics);
	TestFalse(TEXT("Invalid quality setting detected"), bValid);

	// Test audio settings validation
	UAudioSettings* Audio = NewObject<UAudioSettings>();
	Audio->MasterVolume = 0.8f;
	Audio->MusicVolume = 0.7f;
	Audio->AudioQuality = 3;

	bValid = Validator->ValidateAudioSettings(Audio);
	TestTrue(TEXT("Valid audio settings"), bValid);

	// Test invalid volume
	Audio->MasterVolume = 1.5f;
	bValid = Validator->ValidateAudioSettings(Audio);
	TestFalse(TEXT("Invalid volume detected"), bValid);

	// Test control settings validation
	UControlSettings* Controls = NewObject<UControlSettings>();
	Controls->MouseSensitivity = 1.0f;
	Controls->GamepadDeadzone = 0.25f;

	bValid = Validator->ValidateControlSettings(Controls);
	TestTrue(TEXT("Valid control settings"), bValid);

	// Test invalid deadzone
	Controls->GamepadDeadzone = 1.5f;
	bValid = Validator->ValidateControlSettings(Controls);
	TestFalse(TEXT("Invalid deadzone detected"), bValid);

	// Test gameplay settings validation
	UGameplaySettings* Gameplay = NewObject<UGameplaySettings>();
	Gameplay->FieldOfView = 90.0f;
	Gameplay->HUDOpacity = 1.0f;

	bValid = Validator->ValidateGameplaySettings(Gameplay);
	TestTrue(TEXT("Valid gameplay settings"), bValid);

	// Test invalid FOV
	Gameplay->FieldOfView = 30.0f;
	bValid = Validator->ValidateGameplaySettings(Gameplay);
	TestFalse(TEXT("Invalid FOV detected"), bValid);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FValidationResultTest, "HorrorProject.Settings.ValidationResult", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FValidationResultTest::RunTest(const FString& Parameters)
{
	UConfigValidator* Validator = NewObject<UConfigValidator>();

	// Create settings with multiple errors
	UGraphicsSettings* Graphics = NewObject<UGraphicsSettings>();
	Graphics->Resolution = FIntPoint(100, 100);
	Graphics->ViewDistanceQuality = 10;
	Graphics->Brightness = 5.0f;

	Validator->ValidateGraphicsSettings(Graphics);
	FValidationResult Result = Validator->GetLastValidationResult();

	TestFalse(TEXT("Validation failed"), Result.bIsValid);
	TestTrue(TEXT("Has errors"), Result.Errors.Num() > 0);

	// Test validation with warnings
	Graphics->Resolution = FIntPoint(1920, 1080);
	Graphics->ViewDistanceQuality = 3;
	Graphics->Brightness = 1.8f;
	Graphics->FrameRateLimit = 500;

	Validator->ValidateGraphicsSettings(Graphics);
	Result = Validator->GetLastValidationResult();

	TestTrue(TEXT("Has warnings"), Result.Warnings.Num() > 0);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FKeyBindingValidationTest, "HorrorProject.Settings.KeyBindingValidation", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FKeyBindingValidationTest::RunTest(const FString& Parameters)
{
	UControlSettings* Controls = NewObject<UControlSettings>();
	UConfigValidator* Validator = NewObject<UConfigValidator>();

	// Add valid key bindings
	FKeyBinding Binding1;
	Binding1.ActionName = TEXT("Action1");
	Binding1.PrimaryKey = EKeys::W;
	Controls->KeyBindings.Add(Binding1);

	FKeyBinding Binding2;
	Binding2.ActionName = TEXT("Action2");
	Binding2.PrimaryKey = EKeys::S;
	Controls->KeyBindings.Add(Binding2);

	bool bValid = Validator->ValidateControlSettings(Controls);
	TestTrue(TEXT("Valid key bindings"), bValid);

	// Add conflicting key binding
	FKeyBinding Binding3;
	Binding3.ActionName = TEXT("Action3");
	Binding3.PrimaryKey = EKeys::W;
	Controls->KeyBindings.Add(Binding3);

	Validator->ValidateControlSettings(Controls);
	FValidationResult Result = Validator->GetLastValidationResult();
	TestTrue(TEXT("Duplicate key warning"), Result.Warnings.Num() > 0);

	// Test key conflict detection
	TestTrue(TEXT("Key conflict detected"), Controls->IsKeyConflicting(EKeys::W));
	TestFalse(TEXT("No conflict for unused key"), Controls->IsKeyConflicting(EKeys::A));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FValidateAllSettingsTest, "HorrorProject.Settings.ValidateAll", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FValidateAllSettingsTest::RunTest(const FString& Parameters)
{
	UConfigValidator* Validator = NewObject<UConfigValidator>();

	// Create valid settings
	UGraphicsSettings* Graphics = NewObject<UGraphicsSettings>();
	Graphics->Resolution = FIntPoint(1920, 1080);
	Graphics->ViewDistanceQuality = 3;

	UAudioSettings* Audio = NewObject<UAudioSettings>();
	Audio->MasterVolume = 0.8f;

	UControlSettings* Controls = NewObject<UControlSettings>();
	Controls->MouseSensitivity = 1.0f;

	UGameplaySettings* Gameplay = NewObject<UGameplaySettings>();
	Gameplay->FieldOfView = 90.0f;

	// Validate all settings
	FValidationResult Result = Validator->ValidateAllSettings(Graphics, Audio, Controls, Gameplay);
	TestTrue(TEXT("All settings valid"), Result.bIsValid);

	// Introduce errors
	Graphics->ViewDistanceQuality = 10;
	Audio->MasterVolume = 2.0f;

	Result = Validator->ValidateAllSettings(Graphics, Audio, Controls, Gameplay);
	TestFalse(TEXT("Invalid settings detected"), Result.bIsValid);
	TestTrue(TEXT("Multiple errors reported"), Result.Errors.Num() >= 2);

	return true;
}
