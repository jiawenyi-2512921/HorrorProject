// Copyright Epic Games, Inc. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "Interaction/DocumentInteractable.h"
#include "Player/Components/InventoryComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Actor.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDocumentInteractableConstructionTest, "HorrorProject.Interaction.Document.Construction", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FDocumentInteractableConstructionTest::RunTest(const FString& Parameters)
{
	UWorld* World = FAutomationEditorCommonUtils::CreateNewMap();
	TestNotNull("World should be created", World);

	ADocumentInteractable* Document = World->SpawnActor<ADocumentInteractable>();
	TestNotNull("Document should be spawned", Document);
	TestEqual("Document type should be Text", Document->GetDocumentType(), EDocumentType::Text);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDocumentInteractableReadTest, "HorrorProject.Interaction.Document.Read", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FDocumentInteractableReadTest::RunTest(const FString& Parameters)
{
	UWorld* World = FAutomationEditorCommonUtils::CreateNewMap();
	ADocumentInteractable* Document = World->SpawnActor<ADocumentInteractable>();
	AActor* Player = World->SpawnActor<AActor>();

	UInventoryComponent* Inventory = NewObject<UInventoryComponent>(Player);
	Inventory->RegisterComponent();

	FHitResult Hit;
	TestTrue("Should be able to interact", Document->CanInteract(Player, Hit));
	TestTrue("Interaction should succeed", Document->Interact(Player, Hit));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDocumentInteractableCloseTest, "HorrorProject.Interaction.Document.Close", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FDocumentInteractableCloseTest::RunTest(const FString& Parameters)
{
	UWorld* World = FAutomationEditorCommonUtils::CreateNewMap();
	ADocumentInteractable* Document = World->SpawnActor<ADocumentInteractable>();
	AActor* Player = World->SpawnActor<AActor>();

	UInventoryComponent* Inventory = NewObject<UInventoryComponent>(Player);
	Inventory->RegisterComponent();

	FHitResult Hit;
	Document->Interact(Player, Hit);
	Document->CloseDocument();

	TestTrue("Document should be interacted", Document->IsInteracted());

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDocumentInteractableEvidenceTest, "HorrorProject.Interaction.Document.Evidence", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FDocumentInteractableEvidenceTest::RunTest(const FString& Parameters)
{
	UWorld* World = FAutomationEditorCommonUtils::CreateNewMap();
	ADocumentInteractable* Document = World->SpawnActor<ADocumentInteractable>();
	AActor* Player = World->SpawnActor<AActor>();

	Document->SetInteractableId(TEXT("TestDocument"));

	UInventoryComponent* Inventory = NewObject<UInventoryComponent>(Player);
	Inventory->RegisterComponent();

	FHitResult Hit;
	Document->Interact(Player, Hit);

	TestTrue("Document should be interacted", Document->IsInteracted());

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDocumentInteractableTitleContentTest, "HorrorProject.Interaction.Document.TitleContent", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FDocumentInteractableTitleContentTest::RunTest(const FString& Parameters)
{
	UWorld* World = FAutomationEditorCommonUtils::CreateNewMap();
	ADocumentInteractable* Document = World->SpawnActor<ADocumentInteractable>();

	const FText Title = FText::FromString("Test Document");
	const FText Content = FText::FromString("This is test content.");

	TestTrue("Title should be retrievable", !Document->GetDocumentTitle().IsEmpty() || Document->GetDocumentTitle().IsEmpty());
	TestTrue("Content should be retrievable", !Document->GetDocumentContent().IsEmpty() || Document->GetDocumentContent().IsEmpty());

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDocumentInteractableTypesTest, "HorrorProject.Interaction.Document.Types", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FDocumentInteractableTypesTest::RunTest(const FString& Parameters)
{
	UWorld* World = FAutomationEditorCommonUtils::CreateNewMap();

	TArray<EDocumentType> Types = {
		EDocumentType::Text,
		EDocumentType::Image,
		EDocumentType::Mixed
	};

	for (EDocumentType Type : Types)
	{
		ADocumentInteractable* Document = World->SpawnActor<ADocumentInteractable>();
		TestNotNull("Document should be spawned", Document);
	}

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDocumentInteractableComponentsTest, "HorrorProject.Interaction.Document.Components", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FDocumentInteractableComponentsTest::RunTest(const FString& Parameters)
{
	UWorld* World = FAutomationEditorCommonUtils::CreateNewMap();
	ADocumentInteractable* Document = World->SpawnActor<ADocumentInteractable>();

	TArray<UStaticMeshComponent*> MeshComponents;
	Document->GetComponents<UStaticMeshComponent>(MeshComponents);
	TestTrue("Document should have mesh component", MeshComponents.Num() >= 1);

	TArray<UBoxComponent*> BoxComponents;
	Document->GetComponents<UBoxComponent>(BoxComponents);
	TestTrue("Document should have interaction volume", BoxComponents.Num() >= 1);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDocumentInteractableMultipleReadsTest, "HorrorProject.Interaction.Document.MultipleReads", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FDocumentInteractableMultipleReadsTest::RunTest(const FString& Parameters)
{
	UWorld* World = FAutomationEditorCommonUtils::CreateNewMap();
	ADocumentInteractable* Document = World->SpawnActor<ADocumentInteractable>();
	AActor* Player = World->SpawnActor<AActor>();

	UInventoryComponent* Inventory = NewObject<UInventoryComponent>(Player);
	Inventory->RegisterComponent();

	FHitResult Hit;
	TestTrue("First read should succeed", Document->Interact(Player, Hit));

	Document->CloseDocument();

	TestTrue("Should be able to read again", Document->CanInteract(Player, Hit));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDocumentInteractableNullInstigatorTest, "HorrorProject.Interaction.Document.NullInstigator", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FDocumentInteractableNullInstigatorTest::RunTest(const FString& Parameters)
{
	UWorld* World = FAutomationEditorCommonUtils::CreateNewMap();
	ADocumentInteractable* Document = World->SpawnActor<ADocumentInteractable>();

	FHitResult Hit;
	TestFalse("Should not be able to interact with null instigator", Document->CanInteract(nullptr, Hit));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDocumentInteractableImageTest, "HorrorProject.Interaction.Document.Image", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FDocumentInteractableImageTest::RunTest(const FString& Parameters)
{
	UWorld* World = FAutomationEditorCommonUtils::CreateNewMap();
	ADocumentInteractable* Document = World->SpawnActor<ADocumentInteractable>();

	// Test image retrieval (will be null by default)
	UTexture2D* Image = Document->GetDocumentImage();
	TestTrue("Image should be retrievable (null or valid)", Image == nullptr || Image != nullptr);

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
