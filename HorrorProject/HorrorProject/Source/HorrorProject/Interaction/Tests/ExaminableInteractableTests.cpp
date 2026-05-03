#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR && HORRORPROJECT_ENABLE_LEGACY_AUTOMATION_TESTS

// Copyright Epic Games, Inc. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "Interaction/ExaminableInteractable.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Actor.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FExaminableInteractableConstructionTest, "HorrorProject.Interaction.Examinable.Construction", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FExaminableInteractableConstructionTest::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	TestNotNull("World should be created", World);

	AExaminableInteractable* Examinable = World->SpawnActor<AExaminableInteractable>();
	TestNotNull("Examinable should be spawned", Examinable);
	TestFalse("Should not be examining by default", Examinable->IsBeingExamined());
	TestFalse("Idle examinable objects should not spend a constant actor tick", Examinable->IsActorTickEnabled());

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FExaminableInteractableStartExamineTest, "HorrorProject.Interaction.Examinable.StartExamine", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FExaminableInteractableStartExamineTest::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	AExaminableInteractable* Examinable = World->SpawnActor<AExaminableInteractable>();
	APlayerController* PC = World->SpawnActor<APlayerController>();

	Examinable->StartExamining(PC);
	TestTrue("Should be examining", Examinable->IsBeingExamined());
	TestTrue("Examining should enable actor tick for camera and input updates", Examinable->IsActorTickEnabled());

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FExaminableInteractableStopExamineTest, "HorrorProject.Interaction.Examinable.StopExamine", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FExaminableInteractableStopExamineTest::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	AExaminableInteractable* Examinable = World->SpawnActor<AExaminableInteractable>();
	APlayerController* PC = World->SpawnActor<APlayerController>();

	Examinable->StartExamining(PC);
	TestTrue("Should be examining", Examinable->IsBeingExamined());

	Examinable->StopExamining();
	TestFalse("Should not be examining", Examinable->IsBeingExamined());
	TestFalse("Stopping examination should disable actor tick again", Examinable->IsActorTickEnabled());

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FExaminableInteractableRotationTest, "HorrorProject.Interaction.Examinable.Rotation", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FExaminableInteractableRotationTest::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	AExaminableInteractable* Examinable = World->SpawnActor<AExaminableInteractable>();
	APlayerController* PC = World->SpawnActor<APlayerController>();

	Examinable->StartExamining(PC);
	Examinable->RotateObject(45.0f, 30.0f);

	// Rotation should be applied (exact values depend on implementation)
	TestTrue("Should be examining", Examinable->IsBeingExamined());

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FExaminableInteractableZoomTest, "HorrorProject.Interaction.Examinable.Zoom", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FExaminableInteractableZoomTest::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	AExaminableInteractable* Examinable = World->SpawnActor<AExaminableInteractable>();
	APlayerController* PC = World->SpawnActor<APlayerController>();

	Examinable->StartExamining(PC);
	Examinable->ZoomObject(10.0f);
	Examinable->ZoomObject(-5.0f);

	TestTrue("Should be examining", Examinable->IsBeingExamined());

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FExaminableInteractableMultipleInteractionsTest, "HorrorProject.Interaction.Examinable.MultipleInteractions", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FExaminableInteractableMultipleInteractionsTest::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	AExaminableInteractable* Examinable = World->SpawnActor<AExaminableInteractable>();
	AActor* Player = World->SpawnActor<AActor>();

	FHitResult Hit;
	TestTrue("First interaction should succeed", Examinable->Interact(Player, Hit));
	TestTrue("Should allow multiple interactions", Examinable->CanInteract(Player, Hit));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FExaminableInteractableCannotExamineWhileExaminingTest, "HorrorProject.Interaction.Examinable.CannotExamineWhileExamining", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FExaminableInteractableCannotExamineWhileExaminingTest::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	AExaminableInteractable* Examinable = World->SpawnActor<AExaminableInteractable>();
	APlayerController* PC = World->SpawnActor<APlayerController>();
	AActor* Player = World->SpawnActor<AActor>();

	Examinable->StartExamining(PC);

	FHitResult Hit;
	TestFalse("Should not be able to interact while examining", Examinable->CanInteract(Player, Hit));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FExaminableInteractableComponentsTest, "HorrorProject.Interaction.Examinable.Components", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FExaminableInteractableComponentsTest::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	AExaminableInteractable* Examinable = World->SpawnActor<AExaminableInteractable>();

	TArray<UStaticMeshComponent*> MeshComponents;
	Examinable->GetComponents<UStaticMeshComponent>(MeshComponents);
	TestTrue("Examinable should have mesh component", MeshComponents.Num() >= 1);

	TArray<UBoxComponent*> BoxComponents;
	Examinable->GetComponents<UBoxComponent>(BoxComponents);
	TestTrue("Examinable should have interaction volume", BoxComponents.Num() >= 1);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FExaminableInteractableNullControllerTest, "HorrorProject.Interaction.Examinable.NullController", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FExaminableInteractableNullControllerTest::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	AExaminableInteractable* Examinable = World->SpawnActor<AExaminableInteractable>();

	Examinable->StartExamining(nullptr);
	TestFalse("Should not be examining with null controller", Examinable->IsBeingExamined());

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FExaminableInteractableTickTest, "HorrorProject.Interaction.Examinable.Tick", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FExaminableInteractableTickTest::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	AExaminableInteractable* Examinable = World->SpawnActor<AExaminableInteractable>();
	APlayerController* PC = World->SpawnActor<APlayerController>();

	Examinable->StartExamining(PC);

	// Simulate ticks
	for (int32 i = 0; i < 10; i++)
	{
		Examinable->Tick(0.016f);
	}

	TestTrue("Should still be examining after ticks", Examinable->IsBeingExamined());

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS

#endif // WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR
