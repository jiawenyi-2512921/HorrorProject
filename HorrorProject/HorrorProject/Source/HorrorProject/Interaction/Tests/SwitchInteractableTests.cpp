#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR && HORRORPROJECT_ENABLE_LEGACY_AUTOMATION_TESTS

// Copyright Epic Games, Inc. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "Interaction/SwitchInteractable.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Actor.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSwitchInteractableConstructionTest, "HorrorProject.Interaction.Switch.Construction", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FSwitchInteractableConstructionTest::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	TestNotNull("World should be created", World);

	ASwitchInteractable* Switch = World->SpawnActor<ASwitchInteractable>();
	TestNotNull("Switch should be spawned", Switch);
	TestFalse("Switch should be off by default", Switch->IsSwitchOn());
	TestEqual("Switch type should be Toggle", Switch->GetSwitchType(), ESwitchType::Toggle);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSwitchInteractableToggleTest, "HorrorProject.Interaction.Switch.Toggle", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FSwitchInteractableToggleTest::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	ASwitchInteractable* Switch = World->SpawnActor<ASwitchInteractable>();
	AActor* Player = World->SpawnActor<AActor>();

	FHitResult Hit;
	TestFalse("Switch should be off initially", Switch->IsSwitchOn());

	Switch->Interact(Player, Hit);
	TestTrue("Switch should be on after interaction", Switch->IsSwitchOn());

	Switch->Interact(Player, Hit);
	TestFalse("Switch should be off after second interaction", Switch->IsSwitchOn());

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSwitchInteractableSetStateTest, "HorrorProject.Interaction.Switch.SetState", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FSwitchInteractableSetStateTest::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	ASwitchInteractable* Switch = World->SpawnActor<ASwitchInteractable>();

	Switch->SetSwitchState(true);
	TestTrue("Switch should be on", Switch->IsSwitchOn());

	Switch->SetSwitchState(false);
	TestFalse("Switch should be off", Switch->IsSwitchOn());

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSwitchInteractableOneTimeTest, "HorrorProject.Interaction.Switch.OneTime", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FSwitchInteractableOneTimeTest::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	ASwitchInteractable* Switch = World->SpawnActor<ASwitchInteractable>();
	AActor* Player = World->SpawnActor<AActor>();

	// Note: Would need to set SwitchType to OneTime via reflection or test API

	FHitResult Hit;
	TestTrue("First interaction should succeed", Switch->Interact(Player, Hit));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSwitchInteractableSaveLoadTest, "HorrorProject.Interaction.Switch.SaveLoad", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FSwitchInteractableSaveLoadTest::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	ASwitchInteractable* Switch = World->SpawnActor<ASwitchInteractable>();
	Switch->SetInteractableId(TEXT("TestSwitch"));

	Switch->SetSwitchState(true);

	// Save state
	TMap<FName, bool> SavedState;
	Switch->SaveState(SavedState);
	TestTrue("Save state should contain data", SavedState.Num() > 0);

	// Load state
	ASwitchInteractable* NewSwitch = World->SpawnActor<ASwitchInteractable>();
	NewSwitch->SetInteractableId(TEXT("TestSwitch"));
	NewSwitch->LoadState(SavedState);

	TestTrue("Loaded switch should be on", NewSwitch->IsSwitchOn());

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSwitchInteractableComponentsTest, "HorrorProject.Interaction.Switch.Components", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FSwitchInteractableComponentsTest::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	ASwitchInteractable* Switch = World->SpawnActor<ASwitchInteractable>();

	TArray<UStaticMeshComponent*> MeshComponents;
	Switch->GetComponents<UStaticMeshComponent>(MeshComponents);
	TestTrue("Switch should have mesh components", MeshComponents.Num() >= 2);

	TArray<UBoxComponent*> BoxComponents;
	Switch->GetComponents<UBoxComponent>(BoxComponents);
	TestTrue("Switch should have interaction volume", BoxComponents.Num() >= 1);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSwitchInteractableEventTest, "HorrorProject.Interaction.Switch.Event", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FSwitchInteractableEventTest::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	ASwitchInteractable* Switch = World->SpawnActor<ASwitchInteractable>();

	bool bEventFired = false;
	bool bEventState = false;

	Switch->OnSwitchActivated.AddLambda([&](ABaseInteractable* InSwitch, bool bIsOn) {
		bEventFired = true;
		bEventState = bIsOn;
	});

	Switch->SetSwitchState(true);

	TestTrue("Event should have fired", bEventFired);
	TestTrue("Event state should be on", bEventState);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSwitchInteractableConnectedActorsTest, "HorrorProject.Interaction.Switch.ConnectedActors", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FSwitchInteractableConnectedActorsTest::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	ASwitchInteractable* Switch = World->SpawnActor<ASwitchInteractable>();
	AActor* ConnectedActor = World->SpawnActor<AActor>();

	// Note: Would need to add ConnectedActor to Switch's ConnectedActors array

	Switch->SetSwitchState(true);

	TestTrue("Switch should be on", Switch->IsSwitchOn());

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSwitchInteractableNullInstigatorTest, "HorrorProject.Interaction.Switch.NullInstigator", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FSwitchInteractableNullInstigatorTest::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	ASwitchInteractable* Switch = World->SpawnActor<ASwitchInteractable>();

	FHitResult Hit;
	TestFalse("Should not be able to interact with null instigator", Switch->CanInteract(nullptr, Hit));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSwitchInteractableMultipleInteractionsTest, "HorrorProject.Interaction.Switch.MultipleInteractions", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FSwitchInteractableMultipleInteractionsTest::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	ASwitchInteractable* Switch = World->SpawnActor<ASwitchInteractable>();
	AActor* Player = World->SpawnActor<AActor>();

	FHitResult Hit;

	for (int32 i = 0; i < 5; i++)
	{
		TestTrue("Interaction should succeed", Switch->Interact(Player, Hit));
	}

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS

#endif // WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR
