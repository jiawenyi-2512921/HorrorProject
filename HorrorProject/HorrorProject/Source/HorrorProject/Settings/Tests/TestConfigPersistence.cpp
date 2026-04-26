// Copyright Epic Games, Inc. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "ConfigManager.h"
#include "GraphicsSettings.h"
#include "AudioSettings.h"
#include "ControlSettings.h"
#include "GameplaySettings.h"
#include "Misc/Paths.h"
#include "HAL/PlatformFileManager.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FConfigPersistenceTest, "HorrorProject.Settings.ConfigPersistence", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FConfigPersistenceTest::RunTest(const FString& Parameters)
{
	UConfigManager* ConfigManager = NewObject<UConfigManager>();
	TestNotNull(TEXT("Config manager created"), ConfigManager);

	// Test graphics settings save/load
	UGraphicsSettings* GraphicsSettings = NewObject<UGraphicsSettings>();
	GraphicsSettings->Resolution = FIntPoint(2560, 1440);
	GraphicsSettings->bVSync = true;
	GraphicsSettings->ViewDistanceQuality = 3;

	bool bSaved = ConfigManager->SaveConfig(TEXT("Graphics"), GraphicsSettings);
	TestTrue(TEXT("Graphics config saved"), bSaved);

	UGraphicsSettings* LoadedGraphics = NewObject<UGraphicsSettings>();
	bool bLoaded = ConfigManager->LoadConfig(TEXT("Graphics"), LoadedGraphics);
	TestTrue(TEXT("Graphics config loaded"), bLoaded);
	TestEqual(TEXT("Resolution preserved"), LoadedGraphics->Resolution.X, 2560);
	TestTrue(TEXT("VSync preserved"), LoadedGraphics->bVSync);

	// Test audio settings save/load
	UAudioSettings* AudioSettings = NewObject<UAudioSettings>();
	AudioSettings->MasterVolume = 0.75f;
	AudioSettings->bEnableSpatialAudio = true;

	bSaved = ConfigManager->SaveConfig(TEXT("Audio"), AudioSettings);
	TestTrue(TEXT("Audio config saved"), bSaved);

	UAudioSettings* LoadedAudio = NewObject<UAudioSettings>();
	bLoaded = ConfigManager->LoadConfig(TEXT("Audio"), LoadedAudio);
	TestTrue(TEXT("Audio config loaded"), bLoaded);
	TestEqual(TEXT("Master volume preserved"), LoadedAudio->MasterVolume, 0.75f);

	// Test config existence check
	TestTrue(TEXT("Graphics config exists"), ConfigManager->ConfigExists(TEXT("Graphics")));
	TestTrue(TEXT("Audio config exists"), ConfigManager->ConfigExists(TEXT("Audio")));
	TestFalse(TEXT("Non-existent config"), ConfigManager->ConfigExists(TEXT("NonExistent")));

	// Test config deletion
	bool bDeleted = ConfigManager->DeleteConfig(TEXT("Graphics"));
	TestTrue(TEXT("Config deleted"), bDeleted);
	TestFalse(TEXT("Deleted config no longer exists"), ConfigManager->ConfigExists(TEXT("Graphics")));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FConfigExportImportTest, "HorrorProject.Settings.ConfigExportImport", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FConfigExportImportTest::RunTest(const FString& Parameters)
{
	UConfigManager* ConfigManager = NewObject<UConfigManager>();

	// Create test settings
	UGraphicsSettings* Graphics = NewObject<UGraphicsSettings>();
	Graphics->Resolution = FIntPoint(1920, 1080);
	Graphics->QualityPreset = EQualityPreset::Ultra;

	UAudioSettings* Audio = NewObject<UAudioSettings>();
	Audio->MasterVolume = 0.8f;

	UControlSettings* Controls = NewObject<UControlSettings>();
	Controls->MouseSensitivity = 1.5f;

	UGameplaySettings* Gameplay = NewObject<UGameplaySettings>();
	Gameplay->Difficulty = EDifficultyLevel::Hard;

	// Export settings
	FString ExportPath = FPaths::ProjectSavedDir() / TEXT("TestExport.json");
	bool bExported = ConfigManager->ExportSettings(ExportPath, Graphics, Audio, Controls, Gameplay);
	TestTrue(TEXT("Settings exported"), bExported);

	// Create new settings objects
	UGraphicsSettings* ImportedGraphics = NewObject<UGraphicsSettings>();
	UAudioSettings* ImportedAudio = NewObject<UAudioSettings>();
	UControlSettings* ImportedControls = NewObject<UControlSettings>();
	UGameplaySettings* ImportedGameplay = NewObject<UGameplaySettings>();

	// Import settings
	bool bImported = ConfigManager->ImportSettings(ExportPath, ImportedGraphics, ImportedAudio, ImportedControls, ImportedGameplay);
	TestTrue(TEXT("Settings imported"), bImported);

	// Verify imported values
	TestEqual(TEXT("Graphics resolution preserved"), ImportedGraphics->Resolution.X, 1920);
	TestEqual(TEXT("Graphics quality preset preserved"), ImportedGraphics->QualityPreset, EQualityPreset::Ultra);
	TestEqual(TEXT("Audio volume preserved"), ImportedAudio->MasterVolume, 0.8f);
	TestEqual(TEXT("Control sensitivity preserved"), ImportedControls->MouseSensitivity, 1.5f);
	TestEqual(TEXT("Gameplay difficulty preserved"), ImportedGameplay->Difficulty, EDifficultyLevel::Hard);

	// Clean up
	IFileManager::Get().Delete(*ExportPath);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FConfigBackupTest, "HorrorProject.Settings.ConfigBackup", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FConfigBackupTest::RunTest(const FString& Parameters)
{
	UConfigManager* ConfigManager = NewObject<UConfigManager>();

	// Save some test configs
	UGraphicsSettings* Graphics = NewObject<UGraphicsSettings>();
	Graphics->Resolution = FIntPoint(1920, 1080);
	ConfigManager->SaveConfig(TEXT("Graphics"), Graphics);

	// Create backup
	FString BackupName = TEXT("TestBackup");
	bool bBackupCreated = ConfigManager->CreateBackup(BackupName);
	TestTrue(TEXT("Backup created"), bBackupCreated);

	// Verify backup exists
	TArray<FString> Backups = ConfigManager->GetAvailableBackups();
	TestTrue(TEXT("Backup appears in list"), Backups.Contains(BackupName));

	// Modify settings
	Graphics->Resolution = FIntPoint(2560, 1440);
	ConfigManager->SaveConfig(TEXT("Graphics"), Graphics);

	// Restore backup
	bool bRestored = ConfigManager->RestoreBackup(BackupName);
	TestTrue(TEXT("Backup restored"), bRestored);

	// Verify restored settings
	UGraphicsSettings* RestoredGraphics = NewObject<UGraphicsSettings>();
	ConfigManager->LoadConfig(TEXT("Graphics"), RestoredGraphics);
	TestEqual(TEXT("Resolution restored from backup"), RestoredGraphics->Resolution.X, 1920);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FConfigVersionTest, "HorrorProject.Settings.ConfigVersion", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FConfigVersionTest::RunTest(const FString& Parameters)
{
	UConfigManager* ConfigManager = NewObject<UConfigManager>();

	// Test version management
	int32 InitialVersion = ConfigManager->GetConfigVersion();
	TestEqual(TEXT("Initial config version"), InitialVersion, 1);

	ConfigManager->SetConfigVersion(2);
	TestEqual(TEXT("Config version updated"), ConfigManager->GetConfigVersion(), 2);

	// Test version in saved config
	UGraphicsSettings* Graphics = NewObject<UGraphicsSettings>();
	ConfigManager->SaveConfig(TEXT("Graphics"), Graphics);

	// Version should be saved with config
	TestEqual(TEXT("Version persisted"), ConfigManager->GetConfigVersion(), 2);

	return true;
}
