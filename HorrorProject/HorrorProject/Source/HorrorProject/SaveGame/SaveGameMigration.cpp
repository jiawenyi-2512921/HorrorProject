// Copyright HorrorProject. All Rights Reserved.

#include "SaveGameMigration.h"
#include "HorrorProject/Save/HorrorSaveGame.h"

DEFINE_LOG_CATEGORY(LogSaveMigration);

bool USaveGameMigration::MigrateSaveGame(UHorrorSaveGame* SaveGame)
{
	if (!SaveGame)
	{
		UE_LOG(LogSaveMigration, Error, TEXT("MigrateSaveGame: null SaveGame"));
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
	UE_LOG(LogSaveMigration, Log, TEXT("Migrating save game from version %d to %d"), StartVersion, CurrentSaveVersion);

	constexpr int32 MaxIterations = 16;
	int32 Iterations = 0;

	while (SaveGame->SaveVersion < CurrentSaveVersion && Iterations < MaxIterations)
	{
		bool bFoundStep = false;

		for (const FSaveGameMigrationStep& Step : MigrationSteps)
		{
			if (SaveGame->SaveVersion == Step.FromVersion)
			{
				if (Step.MigrationFunction && Step.MigrationFunction(SaveGame))
				{
					SaveGame->SaveVersion = Step.ToVersion;
					UE_LOG(LogSaveMigration, Log, TEXT("Successfully migrated from version %d to %d"), Step.FromVersion, Step.ToVersion);
					bFoundStep = true;
				}
				else
				{
					UE_LOG(LogSaveMigration, Error, TEXT("Failed to migrate from version %d to %d"), Step.FromVersion, Step.ToVersion);
					return false;
				}
				break;
			}
		}

		if (!bFoundStep)
		{
			UE_LOG(LogSaveMigration, Error, TEXT("No migration step found for version %d"), SaveGame->SaveVersion);
			return false;
		}

		++Iterations;
	}

	if (Iterations >= MaxIterations)
	{
		UE_LOG(LogSaveMigration, Error, TEXT("Migration exceeded maximum iterations (%d)"), MaxIterations);
		return false;
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

	UE_LOG(LogSaveMigration, Log, TEXT("Performing V1 to V2 migration"));

	// V2 changes: SaveVersion bumped, any structural changes applied here
	return true;
}
