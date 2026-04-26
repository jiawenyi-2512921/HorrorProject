// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "ConfigMigration.generated.h"

class UConfigManager;

/**
 * Config Migration
 * Handles migration of configuration files between versions
 */
UCLASS()
class HORRORPROJECT_API UConfigMigration : public UObject
{
	GENERATED_BODY()

public:
	// Migration operations
	UFUNCTION(BlueprintCallable, Category = "Migration")
	bool MigrateIfNeeded(UConfigManager* ConfigManager);

	UFUNCTION(BlueprintCallable, Category = "Migration")
	bool MigrateFromVersion(int32 FromVersion, int32 ToVersion, UConfigManager* ConfigManager);

	UFUNCTION(BlueprintPure, Category = "Migration")
	bool NeedsMigration(int32 CurrentVersion) const;

	UFUNCTION(BlueprintPure, Category = "Migration")
	int32 GetLatestVersion() const { return LatestVersion; }

private:
	bool MigrateV0ToV1(UConfigManager* ConfigManager);
	bool BackupBeforeMigration(UConfigManager* ConfigManager);

	static constexpr int32 LatestVersion = 1;
};
