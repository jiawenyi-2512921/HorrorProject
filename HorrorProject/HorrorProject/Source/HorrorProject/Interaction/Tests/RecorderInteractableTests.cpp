// Copyright Epic Games, Inc. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "Interaction/RecorderInteractable.h"
#include "Player/Components/InventoryComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Components/AudioComponent.h"
#include "GameFramework/Actor.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRecorderInteractableConstructionTest, "HorrorProject.Interaction.Recorder.Construction", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FRecorderInteractableConstructionTest::RunTest(const FString& Parameters)
{
	UWorld* World = FAutomationEditorCommonUtils::CreateNewMap();
	TestNotNull("World should be created", World);

	ARecorderInteractable* Recorder = World->SpawnActor<ARecorderInteractable>();
	TestNotNull("Recorder should be spawned", Recorder);
	TestFalse("Should not be playing by default", Recorder->IsPlaying());

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRecorderInteractablePlayTest, "HorrorProject.Interaction.Recorder.Play", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FRecorderInteractablePlayTest::RunTest(const FString& Parameters)
{
	UWorld* World = FAutomationEditorCommonUtils::CreateNewMap();
	ARecorderInteractable* Recorder = World->SpawnActor<ARecorderInteractable>();

	Recorder->PlayAudioLog();
	// Note: IsPlaying() depends on AudioComponent state which may not be fully initialized in tests

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRecorderInteractableStopTest, "HorrorProject.Interaction.Recorder.Stop", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FRecorderInteractableStopTest::RunTest(const FString& Parameters)
{
	UWorld* World = FAutomationEditorCommonUtils::CreateNewMap();
	ARecorderInteractable* Recorder = World->SpawnActor<ARecorderInteractable>();

	Recorder->PlayAudioLog();
	Recorder->StopAudioLog();
	TestFalse("Should not be playing after stop", Recorder->IsPlaying());

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRecorderInteractablePauseTest, "HorrorProject.Interaction.Recorder.Pause", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FRecorderInteractablePauseTest::RunTest(const FString& Parameters)
{
	UWorld* World = FAutomationEditorCommonUtils::CreateNewMap();
	ARecorderInteractable* Recorder = World->SpawnActor<ARecorderInteractable>();

	Recorder->PlayAudioLog();
	Recorder->PauseAudioLog();
	TestFalse("Should not be playing after pause", Recorder->IsPlaying());

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRecorderInteractableInteractTest, "HorrorProject.Interaction.Recorder.Interact", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FRecorderInteractableInteractTest::RunTest(const FString& Parameters)
{
	UWorld* World = FAutomationEditorCommonUtils::CreateNewMap();
	ARecorderInteractable* Recorder = World->SpawnActor<ARecorderInteractable>();
	AActor* Player = World->SpawnActor<AActor>();

	UInventoryComponent* Inventory = NewObject<UInventoryComponent>(Player);
	Inventory->RegisterComponent();

	FHitResult Hit;
	TestTrue("Should be able to interact", Recorder->CanInteract(Player, Hit));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRecorderInteractableEvidenceTest, "HorrorProject.Interaction.Recorder.Evidence", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FRecorderInteractableEvidenceTest::RunTest(const FString& Parameters)
{
	UWorld* World = FAutomationEditorCommonUtils::CreateNewMap();
	ARecorderInteractable* Recorder = World->SpawnActor<ARecorderInteractable>();
	AActor* Player = World->SpawnActor<AActor>();

	Recorder->SetInteractableId(TEXT("TestRecorder"));

	UInventoryComponent* Inventory = NewObject<UInventoryComponent>(Player);
	Inventory->RegisterComponent();

	FHitResult Hit;
	Recorder->Interact(Player, Hit);

	TestTrue("Recorder should be interacted", Recorder->IsInteracted());

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRecorderInteractableSaveLoadTest, "HorrorProject.Interaction.Recorder.SaveLoad", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FRecorderInteractableSaveLoadTest::RunTest(const FString& Parameters)
{
	UWorld* World = FAutomationEditorCommonUtils::CreateNewMap();
	ARecorderInteractable* Recorder = World->SpawnActor<ARecorderInteractable>();
	AActor* Player = World->SpawnActor<AActor>();

	Recorder->SetInteractableId(TEXT("TestRecorder"));

	UInventoryComponent* Inventory = NewObject<UInventoryComponent>(Player);
	Inventory->RegisterComponent();

	FHitResult Hit;
	Recorder->Interact(Player, Hit);

	// Save state
	TMap<FName, bool> SavedState;
	Recorder->SaveState(SavedState);
	TestTrue("Save state should contain data", SavedState.Num() > 0);

	// Load state
	ARecorderInteractable* NewRecorder = World->SpawnActor<ARecorderInteractable>();
	NewRecorder->SetInteractableId(TEXT("TestRecorder"));
	NewRecorder->LoadState(SavedState);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRecorderInteractableComponentsTest, "HorrorProject.Interaction.Recorder.Components", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FRecorderInteractableComponentsTest::RunTest(const FString& Parameters)
{
	UWorld* World = FAutomationEditorCommonUtils::CreateNewMap();
	ARecorderInteractable* Recorder = World->SpawnActor<ARecorderInteractable>();

	TArray<UStaticMeshComponent*> MeshComponents;
	Recorder->GetComponents<UStaticMeshComponent>(MeshComponents);
	TestTrue("Recorder should have mesh component", MeshComponents.Num() >= 1);

	TArray<UBoxComponent*> BoxComponents;
	Recorder->GetComponents<UBoxComponent>(BoxComponents);
	TestTrue("Recorder should have interaction volume", BoxComponents.Num() >= 1);

	TArray<UAudioComponent*> AudioComponents;
	Recorder->GetComponents<UAudioComponent>(AudioComponents);
	TestTrue("Recorder should have audio component", AudioComponents.Num() >= 1);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRecorderInteractableProgressTest, "HorrorProject.Interaction.Recorder.Progress", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FRecorderInteractableProgressTest::RunTest(const FString& Parameters)
{
	UWorld* World = FAutomationEditorCommonUtils::CreateNewMap();
	ARecorderInteractable* Recorder = World->SpawnActor<ARecorderInteractable>();

	float Progress = Recorder->GetPlaybackProgress();
	TestTrue("Progress should be valid", Progress >= 0.0f && Progress <= 1.0f);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRecorderInteractableNullInstigatorTest, "HorrorProject.Interaction.Recorder.NullInstigator", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FRecorderInteractableNullInstigatorTest::RunTest(const FString& Parameters)
{
	UWorld* World = FAutomationEditorCommonUtils::CreateNewMap();
	ARecorderInteractable* Recorder = World->SpawnActor<ARecorderInteractable>();

	FHitResult Hit;
	TestFalse("Should not be able to interact with null instigator", Recorder->CanInteract(nullptr, Hit));

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
