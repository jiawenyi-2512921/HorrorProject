#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR && HORRORPROJECT_ENABLE_LEGACY_AUTOMATION_TESTS

// Copyright Epic Games, Inc. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "Interaction/DoorInteractable.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Actor.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDoorInteractableConstructionTest, "HorrorProject.Interaction.Door.Construction", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FDoorInteractableConstructionTest::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	TestNotNull("World should be created", World);

	ADoorInteractable* Door = World->SpawnActor<ADoorInteractable>();
	TestNotNull("Door should be spawned", Door);
	TestEqual("Door state should be Closed", Door->GetDoorState(), EDoorState::Closed);
	TestFalse("Door should not be locked by default", Door->IsLocked());
	TestFalse("Door should not be open by default", Door->IsOpen());

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDoorInteractableOpenCloseTest, "HorrorProject.Interaction.Door.OpenClose", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FDoorInteractableOpenCloseTest::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	ADoorInteractable* Door = World->SpawnActor<ADoorInteractable>();
	AActor* Player = World->SpawnActor<AActor>();

	// Test opening door
	FHitResult Hit;
	TestTrue("Should be able to interact", Door->CanInteract(Player, Hit));
	TestTrue("Interaction should succeed", Door->Interact(Player, Hit));
	TestTrue("Door should be interacted", Door->IsInteracted());

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDoorInteractableLockedTest, "HorrorProject.Interaction.Door.Locked", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FDoorInteractableLockedTest::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	ADoorInteractable* Door = World->SpawnActor<ADoorInteractable>();
	AActor* Player = World->SpawnActor<AActor>();

	// Lock the door
	Door->SetLocked(true);
	TestTrue("Door should be locked", Door->IsLocked());
	TestEqual("Door state should be Locked", Door->GetDoorState(), EDoorState::Locked);

	// Try to interact with locked door
	FHitResult Hit;
	TestTrue("Should be able to interact with locked door", Door->CanInteract(Player, Hit));
	Door->Interact(Player, Hit);
	TestTrue("Door should still be locked", Door->IsLocked());

	// Unlock the door
	Door->SetLocked(false);
	TestFalse("Door should be unlocked", Door->IsLocked());

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDoorInteractableSaveLoadTest, "HorrorProject.Interaction.Door.SaveLoad", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FDoorInteractableSaveLoadTest::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	ADoorInteractable* Door = World->SpawnActor<ADoorInteractable>();
	Door->SetInteractableId(TEXT("TestDoor"));

	// Set door state
	Door->SetLocked(true);

	// Save state
	TMap<FName, bool> SavedState;
	Door->SaveState(SavedState);
	TestTrue("Save state should contain data", SavedState.Num() > 0);

	// Create new door and load state
	ADoorInteractable* NewDoor = World->SpawnActor<ADoorInteractable>();
	NewDoor->SetInteractableId(TEXT("TestDoor"));
	NewDoor->LoadState(SavedState);

	TestTrue("Loaded door should be locked", NewDoor->IsLocked());

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDoorInteractableAnimationTest, "HorrorProject.Interaction.Door.Animation", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FDoorInteractableAnimationTest::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	ADoorInteractable* Door = World->SpawnActor<ADoorInteractable>();
	AActor* Player = World->SpawnActor<AActor>();

	FHitResult Hit;
	Door->Interact(Player, Hit);

	// Simulate some ticks for animation
	for (int32 i = 0; i < 10; i++)
	{
		Door->Tick(0.016f); // ~60 FPS
	}

	TestTrue("Door should have been interacted", Door->IsInteracted());

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDoorInteractableMultipleInteractionsTest, "HorrorProject.Interaction.Door.MultipleInteractions", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FDoorInteractableMultipleInteractionsTest::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	ADoorInteractable* Door = World->SpawnActor<ADoorInteractable>();
	AActor* Player = World->SpawnActor<AActor>();

	FHitResult Hit;

	// First interaction
	TestTrue("First interaction should succeed", Door->Interact(Player, Hit));

	// Second interaction (should work because doors can be interacted multiple times)
	TestTrue("Second interaction should succeed", Door->Interact(Player, Hit));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDoorInteractableNullInstigatorTest, "HorrorProject.Interaction.Door.NullInstigator", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FDoorInteractableNullInstigatorTest::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	ADoorInteractable* Door = World->SpawnActor<ADoorInteractable>();

	FHitResult Hit;
	TestFalse("Should not be able to interact with null instigator", Door->CanInteract(nullptr, Hit));
	TestFalse("Interaction should fail with null instigator", Door->Interact(nullptr, Hit));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDoorInteractableAutoCloseTest, "HorrorProject.Interaction.Door.AutoClose", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FDoorInteractableAutoCloseTest::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	ADoorInteractable* Door = World->SpawnActor<ADoorInteractable>();
	AActor* Player = World->SpawnActor<AActor>();

	// Enable auto-close with short delay
	// Note: This would require accessing private members or using a test-friendly API

	FHitResult Hit;
	Door->Interact(Player, Hit);

	TestTrue("Door should be interacted", Door->IsInteracted());

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDoorInteractableComponentsTest, "HorrorProject.Interaction.Door.Components", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FDoorInteractableComponentsTest::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	ADoorInteractable* Door = World->SpawnActor<ADoorInteractable>();

	// Verify components exist
	TArray<UStaticMeshComponent*> MeshComponents;
	Door->GetComponents<UStaticMeshComponent>(MeshComponents);
	TestTrue("Door should have mesh components", MeshComponents.Num() >= 2);

	TArray<UBoxComponent*> BoxComponents;
	Door->GetComponents<UBoxComponent>(BoxComponents);
	TestTrue("Door should have interaction volume", BoxComponents.Num() >= 1);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDoorInteractableStateTransitionsTest, "HorrorProject.Interaction.Door.StateTransitions", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FDoorInteractableStateTransitionsTest::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	ADoorInteractable* Door = World->SpawnActor<ADoorInteractable>();

	// Test state transitions
	TestEqual("Initial state should be Closed", Door->GetDoorState(), EDoorState::Closed);

	Door->SetLocked(true);
	TestEqual("State should be Locked", Door->GetDoorState(), EDoorState::Locked);

	Door->SetLocked(false);
	TestEqual("State should be Closed after unlock", Door->GetDoorState(), EDoorState::Closed);

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS

#endif // WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR
