// Copyright Epic Games, Inc. All Rights Reserved.

#include "SaveGameMigration.h"
#include "HorrorProject/Save/HorrorSaveGame.h"

bool USaveGameMigration::MigrateSaveGame(UHorrorSaveGame* SaveGame)
{
	if (!SaveGame)
	{
		return false;
	}

	if (!bMigrationStepsRegistered)
	{
		RegisterMigrationSteps();
	}

	if (!NeedsMigration(SaveGame))
	{
		return true;
	}

	const int32 StartVersion = SaveGame->SaveVersion;
	UE_LOG(LogTemp, Log, TEXT("Migrating save game from version %d to %d"), StartVersion, CurrentSaveVersion);

	for (const FSaveGameMigrationStep& Step : MigrationSteps)
	{
		if (SaveGame->SaveVersion == Step.FromVersion)
		{
			if (Step.MigrationFunction && Step.MigrationFunction(SaveGame))
			{
				SaveGame->SaveVersion = Step.ToVersion;
				UE_LOG(LogTemp, Log, TEXT("Successfully migrated from version %d to %d"), Step.FromVersion, Step.ToVersion);
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("Failed to migrate from version %d to %d"), Step.FromVersion, Step.ToVersion);
				return false;
			}
		}
	}

	return SaveGame->SaveVersion == CurrentSaveVersion;
}

bool USaveGameMigration::NeedsMigration(const UHorrorSaveGame* SaveGame) const
{
	return SaveGame && SaveGame->SaveVersion < CurrentSaveVersion;
}

void USaveGameMigration::RegisterMigrationSteps()
{
	MigrationSteps.Empty();

	// Example migration from V1 to V2
	FSaveGameMigrationStep V1ToV2;
	V1ToV2.FromVersion = 1;
	V1ToV2.ToVersion = 2;
	V1ToV2.MigrationFunction = [this](UHorrorSaveGame* SaveGame) -> bool
	{
		return MigrateFromV1ToV2(SaveGame);
	};
	MigrationSteps.Add(V1ToV2);

	bMigrationStepsRegistered = true;
}

bool USaveGameMigration::MigrateFromV1ToV2(UHorrorSaveGame* SaveGame)
{
	if (!SaveGame)
	{
		return false;
	}

	// Example migration logic
	// Add any new fields or transform existing data
	UE_LOG(LogTemp, Log, TEXT("Performing V1 to V2 migration"));

	// Migration successful
	return true;
}
