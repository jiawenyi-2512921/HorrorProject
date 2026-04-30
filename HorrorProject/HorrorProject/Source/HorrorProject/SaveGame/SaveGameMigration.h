// Copyright HorrorProject. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "SaveGameMigration.generated.h"

class UHorrorSaveGame;

struct FSaveGameMigrationStep
{
	int32 FromVersion = 0;
	int32 ToVersion = 0;
	TFunction<bool(UHorrorSaveGame*)> MigrationFunction;
};

DECLARE_LOG_CATEGORY_EXTERN(LogSaveMigration, Log, All);

/**
 * Save game version migration system.
 * Manages sequential migration steps from older save formats to the current version.
 */
UCLASS()
class HORRORPROJECT_API USaveGameMigration : public UObject
{
	GENERATED_BODY()

public:
	static constexpr int32 CurrentSaveVersion = 2;

	UFUNCTION(BlueprintCallable, Category="SaveGame")
	bool MigrateSaveGame(UHorrorSaveGame* SaveGame);

	UFUNCTION(BlueprintPure, Category="SaveGame")
	bool NeedsMigration(const UHorrorSaveGame* SaveGame) const;

	UFUNCTION(BlueprintPure, Category="SaveGame")
	static int32 GetCurrentSaveVersion() { return CurrentSaveVersion; }

private:
	void RegisterMigrationSteps();
	bool MigrateFromV1ToV2(UHorrorSaveGame* SaveGame);

	TArray<FSaveGameMigrationStep> MigrationSteps;
	bool bMigrationStepsRegistered = false;
};
