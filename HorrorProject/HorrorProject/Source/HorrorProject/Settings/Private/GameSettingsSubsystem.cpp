// Copyright Epic Games, Inc. All Rights Reserved.

#include "GameSettingsSubsystem.h"
#include "GraphicsSettings.h"
#include "HorrorAudioSettings.h"
#include "ControlSettings.h"
#include "GameplaySettings.h"
#include "ConfigManager.h"
#include "ConfigValidator.h"
#include "ConfigMigration.h"
#include "DefaultConfigs.h"

void UGameSettingsSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	InitializeSettings();
	LoadSettings();
	MigrateSettings();
}

void UGameSettingsSubsystem::Deinitialize()
{
	if (bHasUnsavedChanges)
	{
		SaveSettings();
	}

	Super::Deinitialize();
}

void UGameSettingsSubsystem::InitializeSettings()
{
	GraphicsSettings = NewObject<UGraphicsSettings>(this);
	AudioSettings = NewObject<UHorrorAudioSettings>(this);
	ControlSettings = NewObject<UControlSettings>(this);
	GameplaySettings = NewObject<UGameplaySettings>(this);
	ConfigManager = NewObject<UConfigManager>(this);
}

void UGameSettingsSubsystem::LoadSettings()
{
	LoadDefaultSettings();
	bHasUnsavedChanges = false;
	DirtyCategories.Empty();

	if (!ConfigManager)
	{
		MarkDirty(NAME_None);
		return;
	}

	if (!ConfigManager->LoadConfig(TEXT("Graphics"), GraphicsSettings))
	{
		MarkDirty(TEXT("Graphics"));
	}

	if (!ConfigManager->LoadConfig(TEXT("Audio"), AudioSettings))
	{
		MarkDirty(TEXT("Audio"));
	}

	if (!ConfigManager->LoadConfig(TEXT("Controls"), ControlSettings))
	{
		MarkDirty(TEXT("Controls"));
	}

	if (!ConfigManager->LoadConfig(TEXT("Gameplay"), GameplaySettings))
	{
		MarkDirty(TEXT("Gameplay"));
	}
}

void UGameSettingsSubsystem::SaveSettings()
{
	if (!ConfigManager)
	{
		return;
	}

	if (DirtyCategories.Num() == 0)
	{
		// No specific categories dirty - save all
		ConfigManager->SaveConfig(TEXT("Graphics"), GraphicsSettings);
		ConfigManager->SaveConfig(TEXT("Audio"), AudioSettings);
		ConfigManager->SaveConfig(TEXT("Controls"), ControlSettings);
		ConfigManager->SaveConfig(TEXT("Gameplay"), GameplaySettings);
	}
	else
	{
		if (DirtyCategories.Contains(TEXT("Graphics")))
		{
			ConfigManager->SaveConfig(TEXT("Graphics"), GraphicsSettings);
		}

		if (DirtyCategories.Contains(TEXT("Audio")))
		{
			ConfigManager->SaveConfig(TEXT("Audio"), AudioSettings);
		}

		if (DirtyCategories.Contains(TEXT("Controls")))
		{
			ConfigManager->SaveConfig(TEXT("Controls"), ControlSettings);
		}

		if (DirtyCategories.Contains(TEXT("Gameplay")))
		{
			ConfigManager->SaveConfig(TEXT("Gameplay"), GameplaySettings);
		}
	}

	bHasUnsavedChanges = false;
	DirtyCategories.Empty();
}

void UGameSettingsSubsystem::ResetToDefaults(FName Category)
{
	if (Category == NAME_None || Category == TEXT("Graphics"))
	{
		UDefaultConfigs::ApplyDefaultGraphicsSettings(GraphicsSettings);
		MarkDirty(TEXT("Graphics"));
	}

	if (Category == NAME_None || Category == TEXT("Audio"))
	{
		UDefaultConfigs::ApplyDefaultAudioSettings(AudioSettings);
		MarkDirty(TEXT("Audio"));
	}

	if (Category == NAME_None || Category == TEXT("Controls"))
	{
		UDefaultConfigs::ApplyDefaultControlSettings(ControlSettings);
		MarkDirty(TEXT("Controls"));
	}

	if (Category == NAME_None || Category == TEXT("Gameplay"))
	{
		UDefaultConfigs::ApplyDefaultGameplaySettings(GameplaySettings);
		MarkDirty(TEXT("Gameplay"));
	}

	NotifySettingsChanged(Category);
}

void UGameSettingsSubsystem::ApplySettings(FName Category)
{
	if (Category == NAME_None || Category == TEXT("Graphics"))
	{
		GraphicsSettings->Apply();
	}

	if (Category == NAME_None || Category == TEXT("Audio"))
	{
		AudioSettings->Apply();
	}

	if (Category == NAME_None || Category == TEXT("Controls"))
	{
		ControlSettings->Apply();
	}

	if (Category == NAME_None || Category == TEXT("Gameplay"))
	{
		GameplaySettings->Apply();
	}

	NotifySettingsChanged(Category);
}

bool UGameSettingsSubsystem::ExportSettings(const FString& FilePath)
{
	if (!ConfigManager)
	{
		return false;
	}

	return ConfigManager->ExportSettings(FilePath, GraphicsSettings, AudioSettings,
		ControlSettings, GameplaySettings);
}

bool UGameSettingsSubsystem::ImportSettings(const FString& FilePath)
{
	if (!ConfigManager)
	{
		return false;
	}

	bool bSuccess = ConfigManager->ImportSettings(FilePath, GraphicsSettings, AudioSettings,
		ControlSettings, GameplaySettings);

	if (bSuccess)
	{
		MarkDirty(NAME_None);
		ApplySettings();
	}

	return bSuccess;
}

bool UGameSettingsSubsystem::ValidateSettings(FName Category)
{
	UConfigValidator* Validator = NewObject<UConfigValidator>();

	bool bValid = true;

	if (Category == NAME_None || Category == TEXT("Graphics"))
	{
		bValid &= Validator->ValidateGraphicsSettings(GraphicsSettings);
	}

	if (Category == NAME_None || Category == TEXT("Audio"))
	{
		bValid &= Validator->ValidateAudioSettings(AudioSettings);
	}

	if (Category == NAME_None || Category == TEXT("Controls"))
	{
		bValid &= Validator->ValidateControlSettings(ControlSettings);
	}

	if (Category == NAME_None || Category == TEXT("Gameplay"))
	{
		bValid &= Validator->ValidateGameplaySettings(GameplaySettings);
	}

	return bValid;
}

void UGameSettingsSubsystem::AutoDetectSettings()
{
	if (GraphicsSettings)
	{
		GraphicsSettings->AutoDetect();
		MarkDirty(TEXT("Graphics"));
	}

	if (AudioSettings)
	{
		AudioSettings->AutoDetect();
		MarkDirty(TEXT("Audio"));
	}

	NotifySettingsChanged(NAME_None);
}

void UGameSettingsSubsystem::ApplyQualityPreset(int32 PresetLevel)
{
	if (GraphicsSettings)
	{
		GraphicsSettings->ApplyQualityPreset(PresetLevel);
		MarkDirty(TEXT("Graphics"));
		NotifySettingsChanged(TEXT("Graphics"));
	}
}

TArray<FString> UGameSettingsSubsystem::GetAvailableQualityPresets() const
{
	return TArray<FString>{
		TEXT("低"),
		TEXT("中"),
		TEXT("高"),
		TEXT("超高"),
		TEXT("电影级")
	};
}

void UGameSettingsSubsystem::MarkDirty(FName Category)
{
	bHasUnsavedChanges = true;

	if (Category != NAME_None)
	{
		DirtyCategories.Add(Category);
	}
	else
	{
		DirtyCategories.Add(TEXT("Graphics"));
		DirtyCategories.Add(TEXT("Audio"));
		DirtyCategories.Add(TEXT("Controls"));
		DirtyCategories.Add(TEXT("Gameplay"));
	}
}

void UGameSettingsSubsystem::LoadDefaultSettings()
{
	UDefaultConfigs::ApplyDefaultGraphicsSettings(GraphicsSettings);
	UDefaultConfigs::ApplyDefaultAudioSettings(AudioSettings);
	UDefaultConfigs::ApplyDefaultControlSettings(ControlSettings);
	UDefaultConfigs::ApplyDefaultGameplaySettings(GameplaySettings);
}

void UGameSettingsSubsystem::MigrateSettings()
{
	UConfigMigration* Migration = NewObject<UConfigMigration>();
	Migration->MigrateIfNeeded(ConfigManager);
}

void UGameSettingsSubsystem::NotifySettingsChanged(FName Category)
{
	OnSettingsChanged.Broadcast(Category);
}
