// Copyright Epic Games, Inc. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "SaveGame/SaveSlotManager.h"
#include "Save/HorrorSaveGame.h"
#include "Tests/AutomationCommon.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSaveSlotManagerBasicTest, "HorrorProject.SaveGame.SaveSlotManager.Basic", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FSaveSlotManagerBasicTest::RunTest(const FString& Parameters)
{
	USaveSlotManager* SlotManager = NewObject<USaveSlotManager>();
	TestNotNull(TEXT("SlotManager should be created"), SlotManager);

	// Test initial state - all slots should be empty
	for (int32 i = 0; i < USaveSlotManager::MaxSaveSlots; ++i)
	{
		TestTrue(FString::Printf(TEXT("Slot %d should be empty initially"), i), SlotManager->IsSlotEmpty(i));
	}

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSaveSlotManagerSaveLoadTest, "HorrorProject.SaveGame.SaveSlotManager.SaveLoad", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FSaveSlotManagerSaveLoadTest::RunTest(const FString& Parameters)
{
	USaveSlotManager* SlotManager = NewObject<USaveSlotManager>();
	TestNotNull(TEXT("SlotManager should be created"), SlotManager);

	// Create a test save game
	UHorrorSaveGame* TestSave = NewObject<UHorrorSaveGame>();
	TestSave->CheckpointId = FName(TEXT("TestCheckpoint"));
	TestSave->SaveVersion = 1;

	// Save to slot 0
	const bool bSaveSuccess = SlotManager->SaveToSlot(0, TestSave);
	TestTrue(TEXT("Save to slot 0 should succeed"), bSaveSuccess);

	// Verify slot is no longer empty
	TestFalse(TEXT("Slot 0 should not be empty after save"), SlotManager->IsSlotEmpty(0));

	// Load from slot 0
	UHorrorSaveGame* LoadedSave = SlotManager->LoadFromSlot(0);
	TestNotNull(TEXT("Loaded save should not be null"), LoadedSave);

	if (LoadedSave)
	{
		TestEqual(TEXT("Checkpoint ID should match"), LoadedSave->CheckpointId, TestSave->CheckpointId);
		TestEqual(TEXT("Save version should match"), LoadedSave->SaveVersion, TestSave->SaveVersion);
	}

	// Cleanup
	SlotManager->DeleteSlot(0);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSaveSlotManagerMultipleSlotTest, "HorrorProject.SaveGame.SaveSlotManager.MultipleSlots", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FSaveSlotManagerMultipleSlotTest::RunTest(const FString& Parameters)
{
	USaveSlotManager* SlotManager = NewObject<USaveSlotManager>();
	TestNotNull(TEXT("SlotManager should be created"), SlotManager);

	// Create and save to multiple slots
	for (int32 i = 0; i < USaveSlotManager::MaxSaveSlots; ++i)
	{
		UHorrorSaveGame* TestSave = NewObject<UHorrorSaveGame>();
		TestSave->CheckpointId = FName(*FString::Printf(TEXT("Checkpoint_%d"), i));
		TestSave->SaveVersion = 1;

		const bool bSaveSuccess = SlotManager->SaveToSlot(i, TestSave);
		TestTrue(FString::Printf(TEXT("Save to slot %d should succeed"), i), bSaveSuccess);
	}

	// Verify all slots are filled
	for (int32 i = 0; i < USaveSlotManager::MaxSaveSlots; ++i)
	{
		TestFalse(FString::Printf(TEXT("Slot %d should not be empty"), i), SlotManager->IsSlotEmpty(i));
	}

	// Load and verify each slot
	for (int32 i = 0; i < USaveSlotManager::MaxSaveSlots; ++i)
	{
		UHorrorSaveGame* LoadedSave = SlotManager->LoadFromSlot(i);
		TestNotNull(FString::Printf(TEXT("Loaded save from slot %d should not be null"), i), LoadedSave);

		if (LoadedSave)
		{
			const FName ExpectedCheckpoint = FName(*FString::Printf(TEXT("Checkpoint_%d"), i));
			TestEqual(FString::Printf(TEXT("Checkpoint ID for slot %d should match"), i), LoadedSave->CheckpointId, ExpectedCheckpoint);
		}
	}

	// Cleanup
	for (int32 i = 0; i < USaveSlotManager::MaxSaveSlots; ++i)
	{
		SlotManager->DeleteSlot(i);
	}

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSaveSlotManagerDeleteTest, "HorrorProject.SaveGame.SaveSlotManager.Delete", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FSaveSlotManagerDeleteTest::RunTest(const FString& Parameters)
{
	USaveSlotManager* SlotManager = NewObject<USaveSlotManager>();
	TestNotNull(TEXT("SlotManager should be created"), SlotManager);

	// Create and save
	UHorrorSaveGame* TestSave = NewObject<UHorrorSaveGame>();
	TestSave->CheckpointId = FName(TEXT("TestCheckpoint"));
	SlotManager->SaveToSlot(0, TestSave);

	// Verify slot is filled
	TestFalse(TEXT("Slot 0 should not be empty"), SlotManager->IsSlotEmpty(0));

	// Delete slot
	const bool bDeleteSuccess = SlotManager->DeleteSlot(0);
	TestTrue(TEXT("Delete slot 0 should succeed"), bDeleteSuccess);

	// Verify slot is empty
	TestTrue(TEXT("Slot 0 should be empty after delete"), SlotManager->IsSlotEmpty(0));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSaveSlotManagerMetadataTest, "HorrorProject.SaveGame.SaveSlotManager.Metadata", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FSaveSlotManagerMetadataTest::RunTest(const FString& Parameters)
{
	USaveSlotManager* SlotManager = NewObject<USaveSlotManager>();
	TestNotNull(TEXT("SlotManager should be created"), SlotManager);

	// Create and save
	UHorrorSaveGame* TestSave = NewObject<UHorrorSaveGame>();
	TestSave->CheckpointId = FName(TEXT("TestCheckpoint"));
	TestSave->SaveVersion = 1;
	SlotManager->SaveToSlot(0, TestSave);

	// Get metadata
	FSaveSlotMetadata Metadata = SlotManager->GetSlotMetadata(0);
	TestEqual(TEXT("Metadata slot index should be 0"), Metadata.SlotIndex, 0);
	TestEqual(TEXT("Metadata checkpoint should match"), Metadata.CheckpointId, TestSave->CheckpointId);
	TestEqual(TEXT("Metadata version should match"), Metadata.SaveVersion, TestSave->SaveVersion);

	// Cleanup
	SlotManager->DeleteSlot(0);

	return true;
}
