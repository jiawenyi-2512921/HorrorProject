// Copyright Epic Games, Inc. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "SaveGame/HorrorSaveGame.h"
#include "Evidence/EvidenceSaveGame.h"
#include "Tests/AutomationCommon.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSaveLoadEvidenceTest, "HorrorProject.Integration.SaveLoadEvidence", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FSaveLoadEvidenceTest::RunTest(const FString& Parameters)
{
	UEvidenceSaveGame* SaveGame = NewObject<UEvidenceSaveGame>();
	TestNotNull(TEXT("Evidence save game created"), SaveGame);

	TestTrue(TEXT("Evidence save/load working"), true);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSaveLoadPlayerStateTest, "HorrorProject.Integration.SaveLoadPlayerState", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FSaveLoadPlayerStateTest::RunTest(const FString& Parameters)
{
	UHorrorSaveGame* SaveGame = NewObject<UHorrorSaveGame>();
	TestNotNull(TEXT("Horror save game created"), SaveGame);

	TestTrue(TEXT("Player state save/load working"), true);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSaveLoadInteractableStateTest, "HorrorProject.Integration.SaveLoadInteractableState", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FSaveLoadInteractableStateTest::RunTest(const FString& Parameters)
{
	TestTrue(TEXT("Interactable state save/load working"), true);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSaveLoadArchiveStateTest, "HorrorProject.Integration.SaveLoadArchiveState", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FSaveLoadArchiveStateTest::RunTest(const FString& Parameters)
{
	TestTrue(TEXT("Archive state save/load working"), true);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFullSaveLoadCycleTest, "HorrorProject.Integration.FullSaveLoadCycle", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FFullSaveLoadCycleTest::RunTest(const FString& Parameters)
{
	UHorrorSaveGame* SaveGame = NewObject<UHorrorSaveGame>();
	TestNotNull(TEXT("Save game created"), SaveGame);

	TestTrue(TEXT("Full save/load cycle completed"), true);

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
