// Copyright Epic Games, Inc. All Rights Reserved.

#include "ConfigMigration.h"
#include "ConfigManager.h"

bool UConfigMigration::MigrateIfNeeded(UConfigManager* ConfigManager)
{
	if (!ConfigManager)
	{
		return false;
	}

	int32 CurrentVersion = ConfigManager->GetConfigVersion();

	if (!NeedsMigration(CurrentVersion))
	{
		return true;
	}

	// Create backup before migration
	if (!BackupBeforeMigration(ConfigManager))
	{
		return false;
	}

	return MigrateFromVersion(CurrentVersion, LatestVersion, ConfigManager);
}

bool UConfigMigration::MigrateFromVersion(int32 FromVersion, int32 ToVersion, UConfigManager* ConfigManager)
{
	if (!ConfigManager)
	{
		return false;
	}

	bool bSuccess = true;

	// Migrate through each version
	for (int32 Version = FromVersion; Version < ToVersion; ++Version)
	{
		switch (Version)
		{
		case 0:
			bSuccess &= MigrateV0ToV1(ConfigManager);
			break;

		default:
			break;
		}

		if (!bSuccess)
		{
			return false;
		}
	}

	// Update config version
	ConfigManager->SetConfigVersion(ToVersion);

	return true;
}

bool UConfigMigration::NeedsMigration(int32 CurrentVersion) const
{
	return CurrentVersion < LatestVersion;
}

bool UConfigMigration::MigrateV0ToV1(UConfigManager* ConfigManager)
{
	if (!ConfigManager)
	{
		return false;
	}

	// Migration logic from version 0 to version 1
	// This is where you would handle any breaking changes or data transformations

	return true;
}

bool UConfigMigration::BackupBeforeMigration(UConfigManager* ConfigManager)
{
	if (!ConfigManager)
	{
		return false;
	}

	FString BackupName = FString::Printf(TEXT("PreMigration_v%d_%s"),
		ConfigManager->GetConfigVersion(),
		*FDateTime::Now().ToString());

	return ConfigManager->CreateBackup(BackupName);
}
