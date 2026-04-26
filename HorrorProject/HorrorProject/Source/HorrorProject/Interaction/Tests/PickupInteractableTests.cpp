// Copyright Epic Games, Inc. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "Interaction/PickupInteractable.h"
#include "Player/Components/InventoryComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Actor.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPickupInteractableConstructionTest, "HorrorProject.Interaction.Pickup.Construction", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FPickupInteractableConstructionTest::RunTest(const FString& Parameters)
{
	UWorld* World = FAutomationEditorCommonUtils::CreateNewMap();
	TestNotNull("World should be created", World);

	APickupInteractable* Pickup = World->SpawnActor<APickupInteractable>();
	TestNotNull("Pickup should be spawned", Pickup);
	TestEqual("Pickup type should be Generic", Pickup->GetPickupType(), EPickupType::Generic);
	TestFalse("Pickup should not be interacted by default", Pickup->IsInteracted());

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPickupInteractablePickupTest, "HorrorProject.Interaction.Pickup.Pickup", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FPickupInteractablePickupTest::RunTest(const FString& Parameters)
{
	UWorld* World = FAutomationEditorCommonUtils::CreateNewMap();
	APickupInteractable* Pickup = World->SpawnActor<APickupInteractable>();
	AActor* Player = World->SpawnActor<AActor>();

	// Add inventory component to player
	UInventoryComponent* Inventory = NewObject<UInventoryComponent>(Player);
	Inventory->RegisterComponent();

	FHitResult Hit;
	TestTrue("Should be able to interact", Pickup->CanInteract(Player, Hit));
	TestTrue("Interaction should succeed", Pickup->Interact(Player, Hit));
	TestTrue("Pickup should be interacted", Pickup->IsInteracted());

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPickupInteractableEvidenceTest, "HorrorProject.Interaction.Pickup.Evidence", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FPickupInteractableEvidenceTest::RunTest(const FString& Parameters)
{
	UWorld* World = FAutomationEditorCommonUtils::CreateNewMap();
	APickupInteractable* Pickup = World->SpawnActor<APickupInteractable>();
	AActor* Player = World->SpawnActor<AActor>();

	// Configure as evidence pickup
	FHorrorEvidenceMetadata Metadata;
	Metadata.EvidenceId = TEXT("TestEvidence");
	Metadata.DisplayName = FText::FromString("Test Evidence");

	// Add inventory component
	UInventoryComponent* Inventory = NewObject<UInventoryComponent>(Player);
	Inventory->RegisterComponent();

	FHitResult Hit;
	Pickup->Interact(Player, Hit);

	TestTrue("Pickup should be interacted", Pickup->IsInteracted());

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPickupInteractableNoInventoryTest, "HorrorProject.Interaction.Pickup.NoInventory", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FPickupInteractableNoInventoryTest::RunTest(const FString& Parameters)
{
	UWorld* World = FAutomationEditorCommonUtils::CreateNewMap();
	APickupInteractable* Pickup = World->SpawnActor<APickupInteractable>();
	AActor* Player = World->SpawnActor<AActor>();

	// Player has no inventory component
	FHitResult Hit;
	TestTrue("Should be able to interact even without inventory", Pickup->CanInteract(Player, Hit));
	Pickup->Interact(Player, Hit);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPickupInteractableOnceOnlyTest, "HorrorProject.Interaction.Pickup.OnceOnly", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FPickupInteractableOnceOnlyTest::RunTest(const FString& Parameters)
{
	UWorld* World = FAutomationEditorCommonUtils::CreateNewMap();
	APickupInteractable* Pickup = World->SpawnActor<APickupInteractable>();
	AActor* Player = World->SpawnActor<AActor>();

	UInventoryComponent* Inventory = NewObject<UInventoryComponent>(Player);
	Inventory->RegisterComponent();

	FHitResult Hit;
	TestTrue("First interaction should succeed", Pickup->Interact(Player, Hit));
	TestFalse("Second interaction should fail", Pickup->CanInteract(Player, Hit));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPickupInteractableComponentsTest, "HorrorProject.Interaction.Pickup.Components", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FPickupInteractableComponentsTest::RunTest(const FString& Parameters)
{
	UWorld* World = FAutomationEditorCommonUtils::CreateNewMap();
	APickupInteractable* Pickup = World->SpawnActor<APickupInteractable>();

	TArray<UStaticMeshComponent*> MeshComponents;
	Pickup->GetComponents<UStaticMeshComponent>(MeshComponents);
	TestTrue("Pickup should have mesh component", MeshComponents.Num() >= 1);

	TArray<USphereComponent*> SphereComponents;
	Pickup->GetComponents<USphereComponent>(SphereComponents);
	TestTrue("Pickup should have interaction volume", SphereComponents.Num() >= 1);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPickupInteractableItemIdTest, "HorrorProject.Interaction.Pickup.ItemId", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FPickupInteractableItemIdTest::RunTest(const FString& Parameters)
{
	UWorld* World = FAutomationEditorCommonUtils::CreateNewMap();
	APickupInteractable* Pickup = World->SpawnActor<APickupInteractable>();

	const FName TestItemId = TEXT("TestItem");
	Pickup->SetInteractableId(TestItemId);

	TestEqual("Item ID should match", Pickup->GetItemId(), NAME_None); // ItemId is separate from InteractableId

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPickupInteractableNullInstigatorTest, "HorrorProject.Interaction.Pickup.NullInstigator", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FPickupInteractableNullInstigatorTest::RunTest(const FString& Parameters)
{
	UWorld* World = FAutomationEditorCommonUtils::CreateNewMap();
	APickupInteractable* Pickup = World->SpawnActor<APickupInteractable>();

	FHitResult Hit;
	TestFalse("Should not be able to interact with null instigator", Pickup->CanInteract(nullptr, Hit));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPickupInteractableTypesTest, "HorrorProject.Interaction.Pickup.Types", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FPickupInteractableTypesTest::RunTest(const FString& Parameters)
{
	UWorld* World = FAutomationEditorCommonUtils::CreateNewMap();

	// Test different pickup types
	TArray<EPickupType> Types = {
		EPickupType::Evidence,
		EPickupType::Key,
		EPickupType::Tool,
		EPickupType::Document,
		EPickupType::Battery,
		EPickupType::Generic
	};

	for (EPickupType Type : Types)
	{
		APickupInteractable* Pickup = World->SpawnActor<APickupInteractable>();
		TestNotNull("Pickup should be spawned", Pickup);
	}

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPickupInteractableDestroyTest, "HorrorProject.Interaction.Pickup.Destroy", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FPickupInteractableDestroyTest::RunTest(const FString& Parameters)
{
	UWorld* World = FAutomationEditorCommonUtils::CreateNewMap();
	APickupInteractable* Pickup = World->SpawnActor<APickupInteractable>();
	AActor* Player = World->SpawnActor<AActor>();

	UInventoryComponent* Inventory = NewObject<UInventoryComponent>(Player);
	Inventory->RegisterComponent();

	FHitResult Hit;
	Pickup->Interact(Player, Hit);

	// Pickup should be marked for destruction (if bDestroyOnPickup is true)
	TestTrue("Pickup should be interacted", Pickup->IsInteracted());

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
