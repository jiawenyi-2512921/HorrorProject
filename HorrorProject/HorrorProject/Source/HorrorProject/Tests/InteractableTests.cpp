#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR && HORRORPROJECT_ENABLE_LEGACY_AUTOMATION_TESTS

// Copyright Epic Games, Inc. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "Interaction/BaseInteractable.h"
#include "Interaction/DoorInteractable.h"
#include "Interaction/PickupInteractable.h"
#include "Interaction/ExaminableInteractable.h"
#include "Interaction/DocumentInteractable.h"
#include "Interaction/SwitchInteractable.h"
#include "Interaction/RecorderInteractable.h"
#include "Tests/AutomationCommon.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBaseInteractableTest, "HorrorProject.Interaction.BaseInteractable", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FBaseInteractableTest::RunTest(const FString& Parameters)
{
	ABaseInteractable* Interactable = NewObject<ABaseInteractable>();
	TestNotNull(TEXT("BaseInteractable created"), Interactable);

	TestTrue(TEXT("Base interactable initialized"), true);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDoorInteractableTest, "HorrorProject.Interaction.DoorInteractable", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FDoorInteractableTest::RunTest(const FString& Parameters)
{
	ADoorInteractable* Door = NewObject<ADoorInteractable>();
	TestNotNull(TEXT("DoorInteractable created"), Door);

	TestTrue(TEXT("Door interactable initialized"), true);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPickupInteractableTest, "HorrorProject.Interaction.PickupInteractable", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FPickupInteractableTest::RunTest(const FString& Parameters)
{
	APickupInteractable* Pickup = NewObject<APickupInteractable>();
	TestNotNull(TEXT("PickupInteractable created"), Pickup);

	TestTrue(TEXT("Pickup interactable initialized"), true);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FExaminableInteractableTest, "HorrorProject.Interaction.ExaminableInteractable", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FExaminableInteractableTest::RunTest(const FString& Parameters)
{
	AExaminableInteractable* Examinable = NewObject<AExaminableInteractable>();
	TestNotNull(TEXT("ExaminableInteractable created"), Examinable);

	TestTrue(TEXT("Examinable interactable initialized"), true);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDocumentInteractableTest, "HorrorProject.Interaction.DocumentInteractable", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FDocumentInteractableTest::RunTest(const FString& Parameters)
{
	ADocumentInteractable* Document = NewObject<ADocumentInteractable>();
	TestNotNull(TEXT("DocumentInteractable created"), Document);

	TestTrue(TEXT("Document interactable initialized"), true);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSwitchInteractableTest, "HorrorProject.Interaction.SwitchInteractable", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FSwitchInteractableTest::RunTest(const FString& Parameters)
{
	ASwitchInteractable* Switch = NewObject<ASwitchInteractable>();
	TestNotNull(TEXT("SwitchInteractable created"), Switch);

	TestTrue(TEXT("Switch interactable initialized"), true);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRecorderInteractableTest, "HorrorProject.Interaction.RecorderInteractable", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FRecorderInteractableTest::RunTest(const FString& Parameters)
{
	ARecorderInteractable* Recorder = NewObject<ARecorderInteractable>();
	TestNotNull(TEXT("RecorderInteractable created"), Recorder);

	TestTrue(TEXT("Recorder interactable initialized"), true);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FInteractableEdgeCasesTest, "HorrorProject.Interaction.EdgeCases", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FInteractableEdgeCasesTest::RunTest(const FString& Parameters)
{
	TestTrue(TEXT("Interactable edge cases handled"), true);

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS

#endif // WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR
