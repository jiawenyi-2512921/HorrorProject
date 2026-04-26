#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR && HORRORPROJECT_ENABLE_LEGACY_AUTOMATION_TESTS

// Copyright Epic Games, Inc. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "SaveGame/AutoSaveComponent.h"
#include "Tests/AutomationCommon.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAutoSaveComponentBasicTest, "HorrorProject.SaveGame.AutoSave.Basic", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FAutoSaveComponentBasicTest::RunTest(const FString& Parameters)
{
	UAutoSaveComponent* AutoSave = NewObject<UAutoSaveComponent>();
	TestNotNull(TEXT("AutoSaveComponent should be created"), AutoSave);

	// Test default values
	TestTrue(TEXT("Auto save should be enabled by default"), AutoSave->bAutoSaveEnabled);
	TestEqual(TEXT("Default interval should be 300 seconds"), AutoSave->AutoSaveIntervalSeconds, 300.0f);
	TestEqual(TEXT("Default slot should be 0"), AutoSave->AutoSaveSlotIndex, 0);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAutoSaveComponentEnableDisableTest, "HorrorProject.SaveGame.AutoSave.EnableDisable", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FAutoSaveComponentEnableDisableTest::RunTest(const FString& Parameters)
{
	UAutoSaveComponent* AutoSave = NewObject<UAutoSaveComponent>();
	TestNotNull(TEXT("AutoSaveComponent should be created"), AutoSave);

	// Test enable/disable
	AutoSave->EnableAutoSave(false);
	TestFalse(TEXT("Auto save should be disabled"), AutoSave->bAutoSaveEnabled);

	AutoSave->EnableAutoSave(true);
	TestTrue(TEXT("Auto save should be enabled"), AutoSave->bAutoSaveEnabled);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAutoSaveComponentIntervalTest, "HorrorProject.SaveGame.AutoSave.Interval", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FAutoSaveComponentIntervalTest::RunTest(const FString& Parameters)
{
	UAutoSaveComponent* AutoSave = NewObject<UAutoSaveComponent>();
	TestNotNull(TEXT("AutoSaveComponent should be created"), AutoSave);

	// Test setting interval
	AutoSave->SetAutoSaveInterval(120.0f);
	TestEqual(TEXT("Interval should be set to 120 seconds"), AutoSave->AutoSaveIntervalSeconds, 120.0f);

	// Test clamping - too low
	AutoSave->SetAutoSaveInterval(10.0f);
	TestEqual(TEXT("Interval should be clamped to minimum 30 seconds"), AutoSave->AutoSaveIntervalSeconds, 30.0f);

	// Test clamping - too high
	AutoSave->SetAutoSaveInterval(1000.0f);
	TestEqual(TEXT("Interval should be clamped to maximum 600 seconds"), AutoSave->AutoSaveIntervalSeconds, 600.0f);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAutoSaveComponentSlotTest, "HorrorProject.SaveGame.AutoSave.Slot", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FAutoSaveComponentSlotTest::RunTest(const FString& Parameters)
{
	UAutoSaveComponent* AutoSave = NewObject<UAutoSaveComponent>();
	TestNotNull(TEXT("AutoSaveComponent should be created"), AutoSave);

	// Test slot assignment
	AutoSave->AutoSaveSlotIndex = 1;
	TestEqual(TEXT("Auto save slot should be 1"), AutoSave->AutoSaveSlotIndex, 1);

	AutoSave->AutoSaveSlotIndex = 2;
	TestEqual(TEXT("Auto save slot should be 2"), AutoSave->AutoSaveSlotIndex, 2);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAutoSaveComponentNotificationTest, "HorrorProject.SaveGame.AutoSave.Notification", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FAutoSaveComponentNotificationTest::RunTest(const FString& Parameters)
{
	UAutoSaveComponent* AutoSave = NewObject<UAutoSaveComponent>();
	TestNotNull(TEXT("AutoSaveComponent should be created"), AutoSave);

	// Test notification flag
	AutoSave->bShowAutoSaveNotification = false;
	TestFalse(TEXT("Notification should be disabled"), AutoSave->bShowAutoSaveNotification);

	AutoSave->bShowAutoSaveNotification = true;
	TestTrue(TEXT("Notification should be enabled"), AutoSave->bShowAutoSaveNotification);

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR
