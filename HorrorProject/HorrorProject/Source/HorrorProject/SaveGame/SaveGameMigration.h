// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "SaveGameMigration.generated.h"

class UHorrorSaveGame;

USTRUCT()
struct FSaveGameMigrationStep
{
	GENERATED_BODY()

	int32 FromVersion = 0;
	int32 ToVersion = 0;
	TFunction<bool(UHorrorSaveGame*)> MigrationFunction;
};

UCLASS()
class HORRORPROJECT_API USaveGameMigration : public UObject
{
	GENERATED_BODY()

public:
	static constexpr int32 CurrentSaveVersion = 1;

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
