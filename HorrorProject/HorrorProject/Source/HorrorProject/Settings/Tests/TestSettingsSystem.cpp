#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR && HORRORPROJECT_ENABLE_LEGACY_AUTOMATION_TESTS

// Copyright Epic Games, Inc. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "GameSettingsSubsystem.h"
#include "GraphicsSettings.h"
#include "HorrorAudioSettings.h"
#include "ControlSettings.h"
#include "GameplaySettings.h"
#include "Tests/AutomationCommon.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSettingsSystemTest, "HorrorProject.Settings.SettingsSystem", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FSettingsSystemTest::RunTest(const FString& Parameters)
{
	// Test settings subsystem initialization
	UGameSettingsSubsystem* SettingsSubsystem = NewObject<UGameSettingsSubsystem>();
	TestNotNull(TEXT("Settings subsystem created"), SettingsSubsystem);

	// Test graphics settings
	UGraphicsSettings* GraphicsSettings = NewObject<UGraphicsSettings>();
	TestNotNull(TEXT("Graphics settings created"), GraphicsSettings);
	TestEqual(TEXT("Default resolution width"), GraphicsSettings->Resolution.X, 1920);
	TestEqual(TEXT("Default resolution height"), GraphicsSettings->Resolution.Y, 1080);
	TestTrue(TEXT("Default fullscreen"), GraphicsSettings->bFullscreen);

	// Test quality preset application
	GraphicsSettings->ApplyQualityPreset(0); // Low
	TestEqual(TEXT("Low preset view distance"), GraphicsSettings->ViewDistanceQuality, 0);

	GraphicsSettings->ApplyQualityPreset(3); // Ultra
	TestEqual(TEXT("Ultra preset view distance"), GraphicsSettings->ViewDistanceQuality, 3);

	// Test audio settings
	UHorrorAudioSettings* AudioSettings = NewObject<UHorrorAudioSettings>();
	TestNotNull(TEXT("Audio settings created"), AudioSettings);
	TestEqual(TEXT("Default master volume"), AudioSettings->MasterVolume, 1.0f);
	TestEqual(TEXT("Default music volume"), AudioSettings->MusicVolume, 0.8f);

	// Test volume clamping
	AudioSettings->SetMasterVolume(1.5f, false);
	TestEqual(TEXT("Volume clamped to max"), AudioSettings->MasterVolume, 1.0f);

	AudioSettings->SetMasterVolume(-0.5f, false);
	TestEqual(TEXT("Volume clamped to min"), AudioSettings->MasterVolume, 0.0f);

	// Test control settings
	UControlSettings* ControlSettings = NewObject<UControlSettings>();
	TestNotNull(TEXT("Control settings created"), ControlSettings);
	TestEqual(TEXT("Default mouse sensitivity"), ControlSettings->MouseSensitivity, 1.0f);
	TestFalse(TEXT("Default invert mouse Y"), ControlSettings->bInvertMouseY);

	// Test key bindings
	FKeyBinding TestBinding;
	TestBinding.ActionName = TEXT("TestAction");
	TestBinding.PrimaryKey = EKeys::W;
	ControlSettings->KeyBindings.Add(TestBinding);

	FKeyBinding RetrievedBinding = ControlSettings->GetKeyBinding(TEXT("TestAction"));
	TestEqual(TEXT("Key binding retrieved"), RetrievedBinding.PrimaryKey, EKeys::W);

	// Test gameplay settings
	UGameplaySettings* GameplaySettings = NewObject<UGameplaySettings>();
	TestNotNull(TEXT("Gameplay settings created"), GameplaySettings);
	TestEqual(TEXT("Default difficulty"), GameplaySettings->Difficulty, EDifficultyLevel::Normal);
	TestEqual(TEXT("Default FOV"), GameplaySettings->FieldOfView, 90.0f);

	// Test difficulty application
	GameplaySettings->SetDifficulty(EDifficultyLevel::Hard);
	TestEqual(TEXT("Hard difficulty damage multiplier"), GameplaySettings->DamageMultiplier, 1.5f);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FQualityPresetTest, "HorrorProject.Settings.QualityPresets", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FQualityPresetTest::RunTest(const FString& Parameters)
{
	UGraphicsSettings* Settings = NewObject<UGraphicsSettings>();

	// Test Low preset
	Settings->ApplyQualityPreset(0);
	TestEqual(TEXT("Low preset quality"), Settings->ViewDistanceQuality, 0);
	TestEqual(TEXT("Low preset AA"), Settings->AntiAliasingMethod, EHorrorAntiAliasingMethod::FXAA);
	TestFalse(TEXT("Low preset motion blur"), Settings->bMotionBlur);

	// Test Medium preset
	Settings->ApplyQualityPreset(1);
	TestEqual(TEXT("Medium preset quality"), Settings->ViewDistanceQuality, 1);
	TestEqual(TEXT("Medium preset AA"), Settings->AntiAliasingMethod, EHorrorAntiAliasingMethod::TAA);
	TestTrue(TEXT("Medium preset motion blur"), Settings->bMotionBlur);

	// Test High preset
	Settings->ApplyQualityPreset(2);
	TestEqual(TEXT("High preset quality"), Settings->ViewDistanceQuality, 2);
	TestEqual(TEXT("High preset shadow quality"), Settings->ShadowQuality, EShadowQuality::High);

	// Test Ultra preset
	Settings->ApplyQualityPreset(3);
	TestEqual(TEXT("Ultra preset quality"), Settings->ViewDistanceQuality, 3);
	TestEqual(TEXT("Ultra preset shadow quality"), Settings->ShadowQuality, EShadowQuality::Ultra);

	// Test Cinematic preset
	Settings->ApplyQualityPreset(4);
	TestEqual(TEXT("Cinematic preset quality"), Settings->ViewDistanceQuality, 4);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FResolutionTest, "HorrorProject.Settings.Resolution", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FResolutionTest::RunTest(const FString& Parameters)
{
	UGraphicsSettings* Settings = NewObject<UGraphicsSettings>();

	// Test resolution setting
	FIntPoint NewResolution(2560, 1440);
	Settings->SetResolution(NewResolution, false);
	TestEqual(TEXT("Resolution width set"), Settings->Resolution.X, 2560);
	TestEqual(TEXT("Resolution height set"), Settings->Resolution.Y, 1440);

	// Test fullscreen toggle
	Settings->SetFullscreen(false, false);
	TestFalse(TEXT("Fullscreen disabled"), Settings->bFullscreen);

	Settings->SetFullscreen(true, false);
	TestTrue(TEXT("Fullscreen enabled"), Settings->bFullscreen);

	// Test supported resolutions
	TArray<FIntPoint> SupportedResolutions = Settings->GetSupportedResolutions();
	TestTrue(TEXT("Has supported resolutions"), SupportedResolutions.Num() > 0);

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR
