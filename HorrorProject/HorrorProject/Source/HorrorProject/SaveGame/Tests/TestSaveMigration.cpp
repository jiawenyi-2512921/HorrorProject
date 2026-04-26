#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR && HORRORPROJECT_ENABLE_LEGACY_AUTOMATION_TESTS

// Copyright Epic Games, Inc. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "SaveGame/SaveGameMigration.h"
#include "Save/HorrorSaveGame.h"
#include "Tests/AutomationCommon.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSaveGameMigrationBasicTest, "HorrorProject.SaveGame.Migration.Basic", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FSaveGameMigrationBasicTest::RunTest(const FString& Parameters)
{
	USaveGameMigration* Migration = NewObject<USaveGameMigration>();
	TestNotNull(TEXT("Migration should be created"), Migration);

	// Test current version
	TestEqual(TEXT("Current save version should be 1"), USaveGameMigration::GetCurrentSaveVersion(), 1);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSaveGameMigrationNeedsMigrationTest, "HorrorProject.SaveGame.Migration.NeedsMigration", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FSaveGameMigrationNeedsMigrationTest::RunTest(const FString& Parameters)
{
	USaveGameMigration* Migration = NewObject<USaveGameMigration>();
	TestNotNull(TEXT("Migration should be created"), Migration);

	// Create save with current version
	UHorrorSaveGame* CurrentSave = NewObject<UHorrorSaveGame>();
	CurrentSave->SaveVersion = USaveGameMigration::GetCurrentSaveVersion();
	TestFalse(TEXT("Current version save should not need migration"), Migration->NeedsMigration(CurrentSave));

	// Create save with old version
	UHorrorSaveGame* OldSave = NewObject<UHorrorSaveGame>();
	OldSave->SaveVersion = USaveGameMigration::GetCurrentSaveVersion() - 1;

	if (USaveGameMigration::GetCurrentSaveVersion() > 1)
	{
		TestTrue(TEXT("Old version save should need migration"), Migration->NeedsMigration(OldSave));
	}

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSaveGameMigrationExecuteTest, "HorrorProject.SaveGame.Migration.Execute", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FSaveGameMigrationExecuteTest::RunTest(const FString& Parameters)
{
	USaveGameMigration* Migration = NewObject<USaveGameMigration>();
	TestNotNull(TEXT("Migration should be created"), Migration);

	// Create save with current version
	UHorrorSaveGame* TestSave = NewObject<UHorrorSaveGame>();
	TestSave->SaveVersion = USaveGameMigration::GetCurrentSaveVersion();
	TestSave->CheckpointId = FName(TEXT("TestCheckpoint"));

	// Migrate (should succeed immediately as no migration needed)
	const bool bMigrationSuccess = Migration->MigrateSaveGame(TestSave);
	TestTrue(TEXT("Migration should succeed"), bMigrationSuccess);
	TestEqual(TEXT("Save version should remain current"), TestSave->SaveVersion, USaveGameMigration::GetCurrentSaveVersion());

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSaveGameMigrationNullSaveTest, "HorrorProject.SaveGame.Migration.NullSave", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FSaveGameMigrationNullSaveTest::RunTest(const FString& Parameters)
{
	USaveGameMigration* Migration = NewObject<USaveGameMigration>();
	TestNotNull(TEXT("Migration should be created"), Migration);

	// Test with null save
	TestFalse(TEXT("Migration with null save should fail"), Migration->MigrateSaveGame(nullptr));
	TestFalse(TEXT("NeedsMigration with null save should return false"), Migration->NeedsMigration(nullptr));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSaveGameMigrationVersionPreservationTest, "HorrorProject.SaveGame.Migration.VersionPreservation", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FSaveGameMigrationVersionPreservationTest::RunTest(const FString& Parameters)
{
	USaveGameMigration* Migration = NewObject<USaveGameMigration>();
	TestNotNull(TEXT("Migration should be created"), Migration);

	// Create save with current version
	UHorrorSaveGame* TestSave = NewObject<UHorrorSaveGame>();
	TestSave->SaveVersion = USaveGameMigration::GetCurrentSaveVersion();
	TestSave->CheckpointId = FName(TEXT("TestCheckpoint"));

	const int32 OriginalVersion = TestSave->SaveVersion;

	// Migrate
	Migration->MigrateSaveGame(TestSave);

	// Version should not change if already current
	TestEqual(TEXT("Version should be preserved"), TestSave->SaveVersion, OriginalVersion);

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR
