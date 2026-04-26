// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "ConfigManager.generated.h"

class UGraphicsSettings;
class UHorrorAudioSettings;
class UControlSettings;
class UGameplaySettings;

/**
 * Config Manager
 * Handles loading, saving, and managing configuration files
 */
UCLASS()
class HORRORPROJECT_API UConfigManager : public UObject
{
	GENERATED_BODY()

public:
	UConfigManager();

	// Config operations
	UFUNCTION(BlueprintCallable, Category = "Config")
	bool LoadConfig(const FString& Category, UObject* SettingsObject);

	UFUNCTION(BlueprintCallable, Category = "Config")
	bool SaveConfig(const FString& Category, UObject* SettingsObject);

	UFUNCTION(BlueprintCallable, Category = "Config")
	bool DeleteConfig(const FString& Category);

	UFUNCTION(BlueprintCallable, Category = "Config")
	bool ConfigExists(const FString& Category) const;

	// Import/Export
	UFUNCTION(BlueprintCallable, Category = "Config")
	bool ExportSettings(const FString& FilePath, UGraphicsSettings* Graphics,
		UHorrorAudioSettings* Audio, UControlSettings* Controls, UGameplaySettings* Gameplay);

	UFUNCTION(BlueprintCallable, Category = "Config")
	bool ImportSettings(const FString& FilePath, UGraphicsSettings* Graphics,
		UHorrorAudioSettings* Audio, UControlSettings* Controls, UGameplaySettings* Gameplay);

	// Backup/Restore
	UFUNCTION(BlueprintCallable, Category = "Config")
	bool CreateBackup(const FString& BackupName);

	UFUNCTION(BlueprintCallable, Category = "Config")
	bool RestoreBackup(const FString& BackupName);

	UFUNCTION(BlueprintPure, Category = "Config")
	TArray<FString> GetAvailableBackups() const;

	// Version management
	UFUNCTION(BlueprintPure, Category = "Config")
	int32 GetConfigVersion() const { return ConfigVersion; }

	UFUNCTION(BlueprintCallable, Category = "Config")
	void SetConfigVersion(int32 Version) { ConfigVersion = Version; }

	// Paths
	UFUNCTION(BlueprintPure, Category = "Config")
	FString GetConfigDirectory() const;

	UFUNCTION(BlueprintPure, Category = "Config")
	FString GetBackupDirectory() const;

private:
	FString GetConfigFilePath(const FString& Category) const;
	FString GetBackupFilePath(const FString& BackupName) const;
	bool SerializeToJson(UObject* Object, TSharedPtr<FJsonObject>& OutJson);
	bool DeserializeFromJson(const TSharedPtr<FJsonObject>& Json, UObject* Object);

	UPROPERTY()
	int32 ConfigVersion;

	static constexpr int32 CurrentConfigVersion = 1;
};
