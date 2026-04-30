// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/Components/InteractionComponent.h"

#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR

#include "Components/BoxComponent.h"
#include "Engine/World.h"
#include "Game/FoundFootageObjectiveInteractable.h"
#include "Game/HorrorGameModeBase.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/WorldSettings.h"
#include "Misc/AutomationTest.h"
#include "Player/HorrorPlayerCharacter.h"
#include "Player/HorrorPlayerController.h"
#include "Player/Components/InventoryComponent.h"
#include "Player/Components/QuantumCameraComponent.h"
#include "Tests/AutomationCommon.h"
#include "UI/Day1SliceHUD.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FInteractionComponentBlockedObjectiveShowsHUDReasonTest,
	"HorrorProject.Player.Interaction.BlockedObjectiveCanInteractFalseShowsHUDReason",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FInteractionComponentBlockedObjectiveShowsHUDReasonTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for blocked interaction feedback coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	World->GetWorldSettings()->DefaultGameMode = AHorrorGameModeBase::StaticClass();
	TestTrue(TEXT("Transient world should create the Day1 game mode for interaction feedback."), World->SetGameMode(FURL()));
	AHorrorGameModeBase* GameMode = World->GetAuthGameMode<AHorrorGameModeBase>();

	AHorrorPlayerController* PlayerController = World->SpawnActor<AHorrorPlayerController>();
	ADay1SliceHUD* HUD = World->SpawnActor<ADay1SliceHUD>();
	AHorrorPlayerCharacter* PlayerCharacter = World->SpawnActor<AHorrorPlayerCharacter>();
	AFoundFootageObjectiveInteractable* Recorder = World->SpawnActor<AFoundFootageObjectiveInteractable>();
	TestNotNull(TEXT("Blocked interaction test should expose the game mode."), GameMode);
	TestNotNull(TEXT("Blocked interaction test should spawn a player controller."), PlayerController);
	TestNotNull(TEXT("Blocked interaction test should attach the native Day1 HUD."), HUD);
	TestNotNull(TEXT("Blocked interaction test should spawn a player character."), PlayerCharacter);
	TestNotNull(TEXT("Blocked interaction test should spawn the blocked objective."), Recorder);
	if (!GameMode || !PlayerController || !HUD || !PlayerCharacter || !Recorder)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	PlayerController->PlayerState = World->SpawnActor<APlayerState>();
	World->AddController(PlayerController);
	PlayerController->MyHUD = HUD;
	PlayerController->Possess(PlayerCharacter);

	Recorder->Objective = EFoundFootageInteractableObjective::FirstAnomalyRecord;
	Recorder->SourceId = TEXT("Evidence.Recorder");
	Recorder->bIsRecordingForFirstAnomalyRecord = true;

	FHitResult ObjectiveHit;
	ObjectiveHit.HitObjectHandle = FActorInstanceHandle(Recorder);
	ObjectiveHit.Component = Recorder->FindComponentByClass<UBoxComponent>();

	UInteractionComponent* Interaction = PlayerCharacter->GetInteractionComponent();
	TestNotNull(TEXT("Player should expose an interaction component."), Interaction);
	if (!Interaction)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	TestFalse(TEXT("Blocked anomaly recording should reject interaction before a candidate exists."), Interaction->TryInteractWithHit(ObjectiveHit));
	TestFalse(TEXT("Rejected interaction should not record the first anomaly."), GameMode->HasRecordedFirstAnomaly());
	TestTrue(TEXT("Rejected interaction should show a transient HUD reason."), HUD->IsTransientMessageVisibleForTests());
	TestEqual(
		TEXT("Rejected interaction should explain the missing anomaly candidate."),
		HUD->GetTransientMessageForTests().ToString(),
		FString(TEXT("先对准异常。")));

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FInteractionComponentRecordsAnomalyFromFocusedCandidateTest,
	"HorrorProject.Player.Interaction.RecordsAnomalyFromFocusedCandidate",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FInteractionComponentRecordsAnomalyFromFocusedCandidateTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for focused anomaly recording coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	World->GetWorldSettings()->DefaultGameMode = AHorrorGameModeBase::StaticClass();
	TestTrue(TEXT("Transient world should create the Day1 game mode for focused anomaly recording."), World->SetGameMode(FURL()));
	AHorrorGameModeBase* GameMode = World->GetAuthGameMode<AHorrorGameModeBase>();
	AHorrorPlayerController* PlayerController = World->SpawnActor<AHorrorPlayerController>();
	AHorrorPlayerCharacter* PlayerCharacter = World->SpawnActor<AHorrorPlayerCharacter>();
	AFoundFootageObjectiveInteractable* AnomalyCandidate = World->SpawnActor<AFoundFootageObjectiveInteractable>();
	TestNotNull(TEXT("Focused anomaly test should expose the game mode."), GameMode);
	TestNotNull(TEXT("Focused anomaly test should spawn a player controller."), PlayerController);
	TestNotNull(TEXT("Focused anomaly test should spawn a player character."), PlayerCharacter);
	TestNotNull(TEXT("Focused anomaly test should spawn an anomaly candidate."), AnomalyCandidate);
	if (!GameMode || !PlayerController || !PlayerCharacter || !AnomalyCandidate)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	PlayerController->PlayerState = World->SpawnActor<APlayerState>();
	World->AddController(PlayerController);
	PlayerController->Possess(PlayerCharacter);

	const FName AnomalyId(TEXT("Evidence.Anomaly01"));
	AnomalyCandidate->Objective = EFoundFootageInteractableObjective::FirstAnomalyCandidate;
	AnomalyCandidate->SourceId = AnomalyId;
	AnomalyCandidate->EvidenceMetadata.EvidenceId = AnomalyId;
	AnomalyCandidate->EvidenceMetadata.DisplayName = FText::FromString(TEXT("First Anomaly"));

	FHitResult ObjectiveHit;
	ObjectiveHit.HitObjectHandle = FActorInstanceHandle(AnomalyCandidate);
	ObjectiveHit.Component = AnomalyCandidate->FindComponentByClass<UBoxComponent>();

	UInteractionComponent* Interaction = PlayerCharacter->GetInteractionComponent();
	UQuantumCameraComponent* QuantumCamera = PlayerCharacter->GetQuantumCameraComponent();
	UInventoryComponent* Inventory = PlayerCharacter->GetInventoryComponent();
	TestNotNull(TEXT("Focused anomaly test player should expose interaction."), Interaction);
	TestNotNull(TEXT("Focused anomaly test player should expose quantum camera."), QuantumCamera);
	TestNotNull(TEXT("Focused anomaly test player should expose inventory."), Inventory);
	if (!Interaction || !QuantumCamera || !Inventory)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	TestTrue(TEXT("Bodycam acquisition should unlock the first note."), GameMode->TryAcquireBodycam(TEXT("Evidence.Bodycam"), true));
	TestTrue(TEXT("First note collection should unlock anomaly alignment."), GameMode->TryCollectFirstNote(TEXT("Note.Intro")));
	TestTrue(TEXT("First interaction with the focused anomaly should register the candidate."), Interaction->TryInteractWithHit(ObjectiveHit));
	TestTrue(TEXT("Focused anomaly should now be pending."), GameMode->HasPendingFirstAnomalyCandidate());
	TestFalse(TEXT("Focused anomaly should not be recorded before camera recording."), GameMode->HasRecordedFirstAnomaly());

	QuantumCamera->SetCameraAcquired(true);
	TestTrue(TEXT("Focused anomaly test should have an acquired camera."), QuantumCamera->IsCameraAcquired());
	QuantumCamera->SetCameraEnabled(true);
	TestTrue(TEXT("Focused anomaly test should have an enabled camera."), QuantumCamera->IsCameraEnabled());
	TestTrue(TEXT("Focused anomaly test should start recording."), QuantumCamera->StartRecording());
	TestTrue(TEXT("Second interaction with the same focused anomaly should record it while filming."), Interaction->TryInteractWithHit(ObjectiveHit));
	TestTrue(TEXT("Focused anomaly recording should complete the first anomaly objective."), GameMode->HasRecordedFirstAnomaly());
	TestFalse(TEXT("Focused anomaly recording should clear the pending candidate."), GameMode->HasPendingFirstAnomalyCandidate());
	TestTrue(TEXT("Focused anomaly recording should write anomaly evidence to inventory."), Inventory->HasCollectedEvidenceId(AnomalyId));

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

#endif
