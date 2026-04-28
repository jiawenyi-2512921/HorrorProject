// Copyright Epic Games, Inc. All Rights Reserved.

#include "Game/DeepWaterStationRouteKit.h"

#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR

#include "AI/HorrorGolemBehaviorComponent.h"
#include "AI/HorrorThreatCharacter.h"
#include "Engine/World.h"
#include "Game/HorrorEncounterDirector.h"
#include "Game/HorrorEventBusSubsystem.h"
#include "Game/HorrorGameModeBase.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/WorldSettings.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/AutomationTest.h"
#include "Player/HorrorPlayerCharacter.h"
#include "Player/Components/InventoryComponent.h"
#include "Player/Components/NoteRecorderComponent.h"
#include "Player/Components/QuantumCameraComponent.h"
#include "Tests/AutomationCommon.h"

namespace
{
	const FString Day1RouteAutosaveSlotName(TEXT("SM13_Day1_Autosave"));
	constexpr int32 Day1RouteAutosaveUserIndex = 0;

	AHorrorPlayerCharacter* SpawnControlledRoutePlayer(FAutomationTestBase& Test, UWorld* World, const FVector& Location)
	{
		if (!World)
		{
			return nullptr;
		}

		FActorSpawnParameters SpawnParameters;
		SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		APlayerController* PlayerController = World->SpawnActor<APlayerController>(SpawnParameters);
		AHorrorPlayerCharacter* PlayerCharacter = World->SpawnActor<AHorrorPlayerCharacter>(Location, FRotator::ZeroRotator, SpawnParameters);
		Test.TestNotNull(TEXT("Route runtime test should spawn a player controller."), PlayerController);
		Test.TestNotNull(TEXT("Route runtime test should spawn a controlled horror player character."), PlayerCharacter);
		if (PlayerController)
		{
			PlayerController->PlayerState = World->SpawnActor<APlayerState>(SpawnParameters);
			World->AddController(PlayerController);
		}
		if (PlayerController && PlayerCharacter)
		{
			PlayerController->Possess(PlayerCharacter);
		}
		return PlayerCharacter;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDeepWaterStationRouteKitDefaultPasswordCluesTest,
	"HorrorProject.Game.DeepWaterStation.RouteKit.DefaultPasswordClues",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FDeepWaterStationRouteKitDefaultPasswordCluesTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for route kit password clue coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	ADeepWaterStationRouteKit* RouteKit = World->SpawnActor<ADeepWaterStationRouteKit>();
	TestNotNull(TEXT("Route kit should spawn for password clue coverage."), RouteKit);
	if (!RouteKit)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	RouteKit->ConfigureDefaultFirstLoopObjectiveNodes();
	TestEqual(TEXT("Default route kit should expose the first-loop nodes."), RouteKit->ObjectiveNodes.Num(), 6);
	if (RouteKit->ObjectiveNodes.Num() != 6)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	const FHorrorNoteMetadata& IntroNote = RouteKit->ObjectiveNodes[1].NoteMetadata;
	const FString IntroBody = IntroNote.Body.ToString();
	TestEqual(TEXT("Default first note should keep the expected note id."), IntroNote.NoteId, FName(TEXT("Note.Intro")));
	TestTrue(TEXT("Default first note should clue the tutorial door code."), IntroBody.Contains(TEXT("0417")));
	TestTrue(TEXT("Default first note should clue the maintenance hatch code."), IntroBody.Contains(TEXT("1939")));
	TestTrue(TEXT("Default first note should clue the anomaly room code."), IntroBody.Contains(TEXT("1831")));
	TestTrue(TEXT("Default first note should clue the monster route hatch code."), IntroBody.Contains(TEXT("1799")));
	TestTrue(TEXT("Default first note should clue the exit gate code."), IntroBody.Contains(TEXT("1697")));

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDeepWaterStationRouteKitArchiveRevealFailureRestoresCheckpointTest,
	"HorrorProject.Game.DeepWaterStation.RouteKit.ArchiveRevealFailureRestoresCheckpoint",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FDeepWaterStationRouteKitArchiveRevealFailureRestoresCheckpointTest::RunTest(const FString& Parameters)
{
	UGameplayStatics::DeleteGameInSlot(Day1RouteAutosaveSlotName, Day1RouteAutosaveUserIndex);

	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for Day1 route failure recovery coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		UGameplayStatics::DeleteGameInSlot(Day1RouteAutosaveSlotName, Day1RouteAutosaveUserIndex);
		return false;
	}

	World->GetWorldSettings()->DefaultGameMode = AHorrorGameModeBase::StaticClass();
	TestTrue(TEXT("Transient world should create the Day1 route failure recovery game mode."), World->SetGameMode(FURL()));
	AHorrorGameModeBase* GameMode = World->GetAuthGameMode<AHorrorGameModeBase>();
	AHorrorPlayerCharacter* PlayerCharacter = SpawnControlledRoutePlayer(*this, World, FVector(1000.0f, 300.0f, 80.0f));
	TestNotNull(TEXT("Day1 route failure recovery should expose the game mode."), GameMode);
	TestNotNull(TEXT("Day1 route failure recovery should spawn a controlled player."), PlayerCharacter);
	if (!GameMode || !PlayerCharacter)
	{
		TestWorld.DestroyTestWorld(false);
		UGameplayStatics::DeleteGameInSlot(Day1RouteAutosaveSlotName, Day1RouteAutosaveUserIndex);
		return false;
	}

	ADeepWaterStationRouteKit* RouteKit = World->SpawnActor<ADeepWaterStationRouteKit>();
	TestNotNull(TEXT("Day1 route failure recovery should spawn the route kit."), RouteKit);
	if (!RouteKit)
	{
		TestWorld.DestroyTestWorld(false);
		UGameplayStatics::DeleteGameInSlot(Day1RouteAutosaveSlotName, Day1RouteAutosaveUserIndex);
		return false;
	}

	RouteKit->ConfigureDefaultFirstLoopObjectiveNodes();
	TestEqual(TEXT("Day1 route failure recovery should spawn the full objective route."), RouteKit->SpawnObjectiveNodes(), 6);
	AHorrorEncounterDirector* EncounterDirector = RouteKit->SpawnEncounterDirector();
	const TArray<AFoundFootageObjectiveInteractable*>& SpawnedInteractables = RouteKit->GetSpawnedObjectiveInteractablesForTests();
	TestNotNull(TEXT("Day1 route failure recovery should spawn an encounter director."), EncounterDirector);
	TestEqual(TEXT("Day1 route failure recovery should retain six objective interactables."), SpawnedInteractables.Num(), 6);
	if (!EncounterDirector || SpawnedInteractables.Num() != 6)
	{
		TestWorld.DestroyTestWorld(false);
		UGameplayStatics::DeleteGameInSlot(Day1RouteAutosaveSlotName, Day1RouteAutosaveUserIndex);
		return false;
	}

	UHorrorEventBusSubsystem* EventBus = World->GetSubsystem<UHorrorEventBusSubsystem>();
	TestNotNull(TEXT("Day1 route failure recovery should expose the event bus."), EventBus);
	if (!EventBus)
	{
		TestWorld.DestroyTestWorld(false);
		UGameplayStatics::DeleteGameInSlot(Day1RouteAutosaveSlotName, Day1RouteAutosaveUserIndex);
		return false;
	}
	EventBus->ResetForTests();

	const FHitResult EmptyHit;
	TestTrue(TEXT("Bodycam interaction should complete before route failure checkpoint."), SpawnedInteractables[0]->Interact_Implementation(PlayerCharacter, EmptyHit));
	TestTrue(TEXT("First note interaction should complete before route failure checkpoint."), SpawnedInteractables[1]->Interact_Implementation(PlayerCharacter, EmptyHit));
	TestTrue(TEXT("Anomaly candidate interaction should complete before route failure checkpoint."), SpawnedInteractables[2]->Interact_Implementation(PlayerCharacter, EmptyHit));

	UQuantumCameraComponent* QuantumCamera = PlayerCharacter->GetQuantumCameraComponent();
	TestNotNull(TEXT("Day1 route failure player should expose quantum camera."), QuantumCamera);
	if (!QuantumCamera)
	{
		TestWorld.DestroyTestWorld(false);
		UGameplayStatics::DeleteGameInSlot(Day1RouteAutosaveSlotName, Day1RouteAutosaveUserIndex);
		return false;
	}
	QuantumCamera->SetCameraAcquired(true);
	QuantumCamera->SetCameraEnabled(true);
	TestTrue(TEXT("Day1 route failure player should start recording."), QuantumCamera->StartRecording());
	TestTrue(TEXT("Anomaly recording interaction should complete before route failure checkpoint."), SpawnedInteractables[3]->Interact_Implementation(PlayerCharacter, EmptyHit));
	TestTrue(TEXT("Anomaly objective should be recorded before route failure checkpoint."), GameMode->HasRecordedFirstAnomaly());
	TestEqual(TEXT("Encounter should still be primed before archive reveal."), EncounterDirector->GetEncounterPhase(), EHorrorEncounterPhase::Primed);

	const FTransform CheckpointTransform(FRotator(0.0f, 30.0f, 0.0f), FVector(1200.0f, 350.0f, 100.0f));
	PlayerCharacter->SetActorTransform(CheckpointTransform);
	TestTrue(TEXT("Route failure recovery should save a checkpoint before archive reveal."), GameMode->SaveDay1Checkpoint(TEXT("Checkpoint.Day1.RouteBeforeArchiveReveal")));

	TestTrue(TEXT("Archive review should complete and reveal the encounter."), SpawnedInteractables[4]->Interact_Implementation(PlayerCharacter, EmptyHit));
	TestTrue(TEXT("Archive review should unlock the exit before the failure."), GameMode->IsExitUnlocked());
	TestEqual(TEXT("Archive review should reveal the encounter before failure."), EncounterDirector->GetEncounterPhase(), EHorrorEncounterPhase::Revealed);
	TestTrue(TEXT("Archive reveal should gate the route before failure."), RouteKit->IsRouteGatedByEncounter());

	AHorrorThreatCharacter* Threat = EncounterDirector->GetThreatActor();
	UHorrorGolemBehaviorComponent* GolemBehavior = Threat ? Threat->GetGolemBehavior() : nullptr;
	TestNotNull(TEXT("Archive reveal should spawn the golem threat."), Threat);
	TestNotNull(TEXT("Archive reveal threat should expose golem behavior."), GolemBehavior);
	if (!Threat || !GolemBehavior)
	{
		TestWorld.DestroyTestWorld(false);
		UGameplayStatics::DeleteGameInSlot(Day1RouteAutosaveSlotName, Day1RouteAutosaveUserIndex);
		return false;
	}

	GolemBehavior->ForceStateTransition(EGolemEncounterState::FinalImpact);
	GolemBehavior->TickComponent(0.05f, LEVELTICK_All, nullptr);

	TestEqual(TEXT("Route golem failure should record the final impact death cause."), GameMode->GetLastPlayerFailureCause(), FName(TEXT("Death.Golem.FinalImpact")));
	TestTrue(TEXT("Route golem failure should recover from the saved checkpoint."), GameMode->DidLastPlayerFailureRecoverFromCheckpoint());
	TestTrue(TEXT("Route golem failure should restore player location to the checkpoint."), PlayerCharacter->GetActorLocation().Equals(CheckpointTransform.GetLocation(), KINDA_SMALL_NUMBER));
	TestTrue(TEXT("Route golem failure should preserve recorded anomaly progress."), GameMode->HasRecordedFirstAnomaly());
	TestFalse(TEXT("Route golem failure should roll back archive review progress to the checkpoint."), GameMode->HasReviewedArchive());
	TestFalse(TEXT("Route golem failure should roll back exit unlock to the checkpoint."), GameMode->IsExitUnlocked());
	TestEqual(TEXT("Route golem failure should restore the encounter to the primed checkpoint phase."), EncounterDirector->GetEncounterPhase(), EHorrorEncounterPhase::Primed);
	TestFalse(TEXT("Restored primed encounter should release the route gate."), RouteKit->IsRouteGatedByEncounter());
	TestFalse(TEXT("Golem behavior should be inactive after failure recovery."), GolemBehavior->IsBehaviorActive());
	TestTrue(TEXT("Route golem failure should publish a checkpoint-restored failure event."), EventBus->GetHistory().ContainsByPredicate(
		[](const FHorrorEventMessage& Message)
		{
			return Message.EventTag == HorrorDay1Tags::PlayerFailureEvent()
				&& Message.SourceId == FName(TEXT("Death.Golem.FinalImpact"))
				&& Message.StateTag == HorrorDay1Tags::CheckpointRestoredState();
		}));

	UGameplayStatics::DeleteGameInSlot(Day1RouteAutosaveSlotName, Day1RouteAutosaveUserIndex);
	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDeepWaterStationRouteKitDay1ObjectiveEncounterBridgeTest,
	"HorrorProject.Game.DeepWaterStation.RouteKit.Day1ObjectiveEncounterBridge",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FDeepWaterStationRouteKitDay1ObjectiveEncounterBridgeTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for active Day1 route bridge coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	World->GetWorldSettings()->DefaultGameMode = AHorrorGameModeBase::StaticClass();
	TestTrue(TEXT("Transient world should create the active Day1 route bridge game mode."), World->SetGameMode(FURL()));
	AHorrorGameModeBase* GameMode = World->GetAuthGameMode<AHorrorGameModeBase>();
	AHorrorPlayerCharacter* PlayerCharacter = World->SpawnActor<AHorrorPlayerCharacter>(FVector(1000.0f, 300.0f, 80.0f), FRotator::ZeroRotator);
	TestNotNull(TEXT("Active Day1 route bridge should expose the game mode."), GameMode);
	TestNotNull(TEXT("Active Day1 route bridge should spawn a player."), PlayerCharacter);
	if (!GameMode || !PlayerCharacter)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	ADeepWaterStationRouteKit* RouteKit = World->SpawnActor<ADeepWaterStationRouteKit>();
	TestNotNull(TEXT("Active Day1 route bridge should spawn the route kit."), RouteKit);
	if (!RouteKit)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	RouteKit->ConfigureDefaultFirstLoopObjectiveNodes();
	TestEqual(TEXT("Active Day1 route bridge should spawn the full objective route."), RouteKit->SpawnObjectiveNodes(), 6);
	AHorrorEncounterDirector* EncounterDirector = RouteKit->SpawnEncounterDirector();
	const TArray<AFoundFootageObjectiveInteractable*>& SpawnedInteractables = RouteKit->GetSpawnedObjectiveInteractablesForTests();
	TestNotNull(TEXT("Active Day1 route bridge should spawn an encounter director."), EncounterDirector);
	TestEqual(TEXT("Active Day1 route bridge should retain six objective interactables."), SpawnedInteractables.Num(), 6);
	if (!EncounterDirector || SpawnedInteractables.Num() != 6)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	UHorrorEventBusSubsystem* EventBus = World->GetSubsystem<UHorrorEventBusSubsystem>();
	TestNotNull(TEXT("Active Day1 route bridge should expose the event bus."), EventBus);
	if (!EventBus)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}
	EventBus->ResetForTests();

	const FHitResult EmptyHit;
	TestEqual(TEXT("Active Day1 route bridge encounter should start primed."), EncounterDirector->GetEncounterPhase(), EHorrorEncounterPhase::Primed);
	TestFalse(TEXT("Active Day1 route bridge should not gate the exit before archive review."), RouteKit->IsRouteGatedByEncounter());
	TestTrue(TEXT("Active Day1 route bridge bodycam interaction should complete."), SpawnedInteractables[0]->Interact_Implementation(PlayerCharacter, EmptyHit));
	TestTrue(TEXT("Active Day1 route bridge first note interaction should complete."), SpawnedInteractables[1]->Interact_Implementation(PlayerCharacter, EmptyHit));
	TestTrue(TEXT("Active Day1 route bridge anomaly candidate interaction should complete."), SpawnedInteractables[2]->Interact_Implementation(PlayerCharacter, EmptyHit));

	UQuantumCameraComponent* QuantumCamera = PlayerCharacter->GetQuantumCameraComponent();
	TestNotNull(TEXT("Active Day1 route bridge player should expose quantum camera."), QuantumCamera);
	if (!QuantumCamera)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}
	QuantumCamera->SetCameraAcquired(true);
	QuantumCamera->SetCameraEnabled(true);
	TestTrue(TEXT("Active Day1 route bridge player should start recording."), QuantumCamera->StartRecording());
	TestTrue(TEXT("Active Day1 route bridge anomaly recording interaction should complete."), SpawnedInteractables[3]->Interact_Implementation(PlayerCharacter, EmptyHit));
	TestTrue(TEXT("Active Day1 route bridge should record the first anomaly."), GameMode->HasRecordedFirstAnomaly());
	TestEqual(TEXT("Active Day1 route bridge should keep the encounter primed until archive review."), EncounterDirector->GetEncounterPhase(), EHorrorEncounterPhase::Primed);
	const UInventoryComponent* Inventory = PlayerCharacter->GetInventoryComponent();
	const UNoteRecorderComponent* NoteRecorder = PlayerCharacter->GetNoteRecorderComponent();
	TestNotNull(TEXT("Active Day1 route bridge player should expose inventory."), Inventory);
	TestNotNull(TEXT("Active Day1 route bridge player should expose note recorder."), NoteRecorder);
	if (!Inventory || !NoteRecorder)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}
	TestTrue(TEXT("Active Day1 route bridge should record bodycam evidence before archive review."), Inventory->HasCollectedEvidenceId(TEXT("Evidence.Bodycam")));
	TestTrue(TEXT("Active Day1 route bridge should record anomaly evidence before archive review."), Inventory->HasCollectedEvidenceId(TEXT("Evidence.Anomaly01")));
	TestTrue(TEXT("Active Day1 route bridge should record the intro note before archive review."), NoteRecorder->HasRecordedNoteId(TEXT("Note.Intro")));
	TestTrue(TEXT("Active Day1 route bridge archive should be interactable after first-loop evidence."), SpawnedInteractables[4]->CanInteract_Implementation(PlayerCharacter, EmptyHit));

	TestTrue(TEXT("Active Day1 route bridge archive interaction should complete."), SpawnedInteractables[4]->Interact_Implementation(PlayerCharacter, EmptyHit));
	TestTrue(TEXT("Active Day1 route bridge archive should unlock the exit."), GameMode->IsExitUnlocked());
	TestEqual(TEXT("Active Day1 route bridge archive review should reveal the encounter."), EncounterDirector->GetEncounterPhase(), EHorrorEncounterPhase::Revealed);
	AHorrorThreatCharacter* RevealedThreat = EncounterDirector->GetThreatActor();
	TestNotNull(TEXT("Active Day1 route bridge archive reveal should spawn the threat."), RevealedThreat);
	if (!RevealedThreat)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}
	TestTrue(TEXT("Active Day1 route bridge revealed threat should activate."), RevealedThreat->IsThreatActive());
	TestTrue(TEXT("Active Day1 route bridge should gate the exit during the reveal."), RouteKit->IsRouteGatedByEncounter());

	TestTrue(TEXT("Active Day1 route bridge exit gate should be interactable during the encounter."), SpawnedInteractables[5]->CanInteract_Implementation(PlayerCharacter, EmptyHit));
	TestTrue(TEXT("Active Day1 route bridge exit gate interaction should complete Day1."), SpawnedInteractables[5]->Interact_Implementation(PlayerCharacter, EmptyHit));
	TestEqual(TEXT("Active Day1 route bridge exit interaction should resolve the encounter."), EncounterDirector->GetEncounterPhase(), EHorrorEncounterPhase::Resolved);
	TestFalse(TEXT("Active Day1 route bridge should release the exit after encounter resolution."), RouteKit->IsRouteGatedByEncounter());
	TestFalse(TEXT("Active Day1 route bridge resolved threat should deactivate."), RevealedThreat->IsThreatActive());
	TestTrue(TEXT("Active Day1 route bridge should complete Day1 after the exit interaction."), GameMode->IsDay1Complete());
	TestTrue(TEXT("Active Day1 route bridge should publish the Day1 completion event."), EventBus->GetHistory().ContainsByPredicate(
		[](const FHorrorEventMessage& Message)
		{
			return Message.EventTag == HorrorDay1Tags::Day1CompletedEvent();
		}));

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

#endif
