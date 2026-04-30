#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR

// Copyright Epic Games, Inc. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "GameSettingsSubsystem.h"
#include "ConfigManager.h"
#include "GraphicsSettings.h"
#include "HorrorAudioSettings.h"
#include "ControlSettings.h"
#include "GameplaySettings.h"
#include "Engine/GameInstance.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"

namespace GameSettingsLoadFallbackTests
{
	class FTestSubsystemCollection : public FSubsystemCollectionBase
	{
	public:
		FTestSubsystemCollection()
			: FSubsystemCollectionBase()
		{
		}
	};

	const TArray<FString>& GetCategories()
	{
		static const TArray<FString> Categories{
			TEXT("Graphics"),
			TEXT("Audio"),
			TEXT("Controls"),
			TEXT("Gameplay")
		};
		return Categories;
	}

	FString GetConfigPath(const FString& Category)
	{
		return FPaths::ProjectSavedDir() / TEXT("Config") / (Category + TEXT(".json"));
	}

	void EnsureConfigDirectory()
	{
		IFileManager::Get().MakeDirectory(*(FPaths::ProjectSavedDir() / TEXT("Config")), true);
	}

	void DeleteConfig(const FString& Category)
	{
		IFileManager::Get().Delete(*GetConfigPath(Category), false, true);
	}

	void DeleteAllConfigs()
	{
		for (const FString& Category : GetCategories())
		{
			DeleteConfig(Category);
		}
	}

	struct FConfigSnapshot
	{
		FConfigSnapshot()
		{
			for (const FString& Category : GetCategories())
			{
				FString Contents;
				if (FFileHelper::LoadFileToString(Contents, *GetConfigPath(Category)))
				{
					OriginalContents.Add(Category, Contents);
				}
			}
		}

		~FConfigSnapshot()
		{
			for (const FString& Category : GetCategories())
			{
				const FString* Contents = OriginalContents.Find(Category);
				if (Contents)
				{
					FFileHelper::SaveStringToFile(*Contents, *GetConfigPath(Category));
				}
				else
				{
					DeleteConfig(Category);
				}
			}
		}

		TMap<FString, FString> OriginalContents;
	};
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMissingJsonFallsBackToDefaultsTest, "HorrorProject.Settings.MissingJsonFallsBackToDefaults", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FMissingJsonFallsBackToDefaultsTest::RunTest(const FString& Parameters)
{
	using namespace GameSettingsLoadFallbackTests;

	FConfigSnapshot ConfigSnapshot;
	EnsureConfigDirectory();
	DeleteAllConfigs();

	UConfigManager* ConfigManager = NewObject<UConfigManager>();
	UGraphicsSettings* SavedGraphics = NewObject<UGraphicsSettings>();
	SavedGraphics->bFullscreen = false;
	SavedGraphics->ViewDistanceQuality = 1;
	TestTrue(TEXT("Valid graphics config saved"), ConfigManager->SaveConfig(TEXT("Graphics"), SavedGraphics));

	UGameInstance* GameInstance = NewObject<UGameInstance>();
	UGameSettingsSubsystem* SettingsSubsystem = NewObject<UGameSettingsSubsystem>(GameInstance);
	FTestSubsystemCollection Collection;
	SettingsSubsystem->Initialize(Collection);

	TestFalse(TEXT("Existing graphics fullscreen is preserved"), SettingsSubsystem->GetGraphicsSettings()->bFullscreen);
	TestEqual(TEXT("Existing graphics quality is preserved"), SettingsSubsystem->GetGraphicsSettings()->ViewDistanceQuality, 1);
	TestEqual(TEXT("Missing audio falls back to default master volume"), SettingsSubsystem->GetAudioSettings()->MasterVolume, 1.0f);
	TestEqual(TEXT("Missing controls fall back to default sensitivity"), SettingsSubsystem->GetControlSettings()->MouseSensitivity, 1.0f);
	TestEqual(TEXT("Missing gameplay falls back to default FOV"), SettingsSubsystem->GetGameplaySettings()->FieldOfView, 90.0f);
	TestTrue(TEXT("Missing categories are marked for save"), SettingsSubsystem->HasUnsavedChanges());

	SettingsSubsystem->SaveSettings();
	TestTrue(TEXT("Missing audio config is written back"), ConfigManager->ConfigExists(TEXT("Audio")));
	TestTrue(TEXT("Missing controls config is written back"), ConfigManager->ConfigExists(TEXT("Controls")));
	TestTrue(TEXT("Missing gameplay config is written back"), ConfigManager->ConfigExists(TEXT("Gameplay")));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCorruptJsonRecoversAndPreservesValidCategoriesTest, "HorrorProject.Settings.CorruptJsonRecoversAndPreservesValidCategories", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FCorruptJsonRecoversAndPreservesValidCategoriesTest::RunTest(const FString& Parameters)
{
	using namespace GameSettingsLoadFallbackTests;

	FConfigSnapshot ConfigSnapshot;
	EnsureConfigDirectory();
	DeleteAllConfigs();

	UConfigManager* ConfigManager = NewObject<UConfigManager>();
	UGraphicsSettings* SavedGraphics = NewObject<UGraphicsSettings>();
	SavedGraphics->bVSync = true;
	SavedGraphics->TextureQuality = 1;
	TestTrue(TEXT("Valid graphics config saved"), ConfigManager->SaveConfig(TEXT("Graphics"), SavedGraphics));
	TestTrue(TEXT("Corrupt audio config written"), FFileHelper::SaveStringToFile(TEXT("{ \"MasterVolume\": "), *GetConfigPath(TEXT("Audio"))));

	UGameInstance* GameInstance = NewObject<UGameInstance>();
	UGameSettingsSubsystem* SettingsSubsystem = NewObject<UGameSettingsSubsystem>(GameInstance);
	FTestSubsystemCollection Collection;
	SettingsSubsystem->Initialize(Collection);

	TestTrue(TEXT("Valid graphics VSync is preserved"), SettingsSubsystem->GetGraphicsSettings()->bVSync);
	TestEqual(TEXT("Valid graphics texture quality is preserved"), SettingsSubsystem->GetGraphicsSettings()->TextureQuality, 1);
	TestEqual(TEXT("Corrupt audio falls back to default master volume"), SettingsSubsystem->GetAudioSettings()->MasterVolume, 1.0f);
	TestTrue(TEXT("Corrupt category is marked for save"), SettingsSubsystem->HasUnsavedChanges());

	SettingsSubsystem->SaveSettings();

	UHorrorAudioSettings* ReloadedAudio = NewObject<UHorrorAudioSettings>();
	TestTrue(TEXT("Corrupt audio config is repaired on save"), ConfigManager->LoadConfig(TEXT("Audio"), ReloadedAudio));
	TestEqual(TEXT("Repaired audio contains default master volume"), ReloadedAudio->MasterVolume, 1.0f);

	UGraphicsSettings* ReloadedGraphics = NewObject<UGraphicsSettings>();
	TestTrue(TEXT("Valid graphics config remains loadable"), ConfigManager->LoadConfig(TEXT("Graphics"), ReloadedGraphics));
	TestTrue(TEXT("Valid graphics config value remains intact"), ReloadedGraphics->bVSync);

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR
