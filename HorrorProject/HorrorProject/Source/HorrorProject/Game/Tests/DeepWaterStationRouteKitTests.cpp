#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR && HORRORPROJECT_ENABLE_LEGACY_AUTOMATION_TESTS

// Copyright Epic Games, Inc. All Rights Reserved.

#if WITH_DEV_AUTOMATION_TESTS

#include "Game/DeepWaterStationRouteKit.h"

#include "AI/HorrorThreatCharacter.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "Game/HorrorEncounterDirector.h"
#include "Game/HorrorEventBusSubsystem.h"
#include "Game/HorrorGameModeBase.h"
#include "GameFramework/WorldSettings.h"
#include "Player/HorrorPlayerCharacter.h"
#include "Player/Components/QuantumCameraComponent.h"
#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDeepWaterStationRouteKitSpawnsObjectiveNodesTest,
	"HorrorProject.Game.DeepWaterStation.RouteKit.SpawnsObjectiveNodes",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FDeepWaterStationRouteKitSpawnsObjectiveNodesTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for route kit coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	World->GetWorldSettings()->DefaultGameMode = AHorrorGameModeBase::StaticClass();
	TestTrue(TEXT("Transient world should create the route kit game mode."), World->SetGameMode(FURL()));
	AHorrorGameModeBase* GameMode = World->GetAuthGameMode<AHorrorGameModeBase>();
	TestNotNull(TEXT("Transient world should expose the route kit game mode."), GameMode);
	if (!GameMode)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	ADeepWaterStationRouteKit* RouteKit = World->SpawnActor<ADeepWaterStationRouteKit>();
	TestNotNull(TEXT("Route kit should spawn in a transient world."), RouteKit);
	if (!RouteKit)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	RouteKit->ConfigureDefaultFirstLoopObjectiveNodes();

	TestEqual(TEXT("Default route kit preset should configure the Day 1 objective sequence."), RouteKit->ObjectiveNodes.Num(), 6);
	if (RouteKit->ObjectiveNodes.Num() != 6)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	TestEqual(TEXT("Default bodycam node should preserve its source id."), RouteKit->ObjectiveNodes[0].SourceId, FName(TEXT("Evidence.Bodycam")));
	TestEqual(TEXT("Default bodycam node should have the first corridor placement."), RouteKit->ObjectiveNodes[0].RelativeTransform.GetLocation(), FVector(200.0f, 0.0f, 80.0f));
	TestEqual(TEXT("Default first note node should preserve note metadata."), RouteKit->ObjectiveNodes[1].NoteMetadata.Title.ToString(), FString(TEXT("维护记录")));
	TestEqual(TEXT("Default first note node should expose a trailer beat id."), RouteKit->ObjectiveNodes[1].TrailerBeatId, FName(TEXT("Beat.FirstNote")));
	TestEqual(TEXT("Route kit should resolve trailer beats by source id."), RouteKit->GetTrailerBeatIdForSourceId(TEXT("Note.Intro")), FName(TEXT("Beat.FirstNote")));
	TestEqual(TEXT("Route kit should return no trailer beat for unknown source ids."), RouteKit->GetTrailerBeatIdForSourceId(TEXT("Missing.Source")), NAME_None);
	const TArray<FName> TrailerBeatIds = RouteKit->GetTrailerBeatIds();
	TestEqual(TEXT("Route kit should expose one trailer beat per default route node."), TrailerBeatIds.Num(), 6);
	if (TrailerBeatIds.Num() != 6)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}
	TestEqual(TEXT("Route kit should preserve trailer beat order."), TrailerBeatIds[0], FName(TEXT("Beat.BodycamAcquire")));
	TestEqual(TEXT("Route kit should expose an exit gate trailer beat."), TrailerBeatIds[5], FName(TEXT("Beat.ExitGate")));
	RouteKit->ObjectiveNodes[5].TrailerBeatId = RouteKit->ObjectiveNodes[4].TrailerBeatId;
	const TArray<FName> UniqueTrailerBeatIds = RouteKit->GetTrailerBeatIds();
	TestEqual(TEXT("Route kit should collapse duplicate trailer beat ids for cue scheduling."), UniqueTrailerBeatIds.Num(), 5);
	TestEqual(TEXT("Route kit should preserve the first occurrence when removing duplicate trailer beats."), UniqueTrailerBeatIds.Last(), FName(TEXT("Beat.ArchiveReview")));
	RouteKit->ObjectiveNodes[5].TrailerBeatId = FName(TEXT("Beat.ExitGate"));
	TestEqual(TEXT("Default first note node should expose an objective hint."), RouteKit->ObjectiveNodes[1].ObjectiveHint.ToString(), FString(TEXT("阅读第一份站内笔记。")));
	TestEqual(TEXT("Default first note node should advance down the corridor."), RouteKit->ObjectiveNodes[1].RelativeTransform.GetLocation(), FVector(600.0f, 0.0f, 80.0f));
	TestTrue(TEXT("Default first anomaly record node should require recording."), RouteKit->ObjectiveNodes[3].bIsRecordingForFirstAnomalyRecord);
	TestEqual(TEXT("Default first anomaly record node should carry anomaly metadata."), RouteKit->ObjectiveNodes[3].EvidenceMetadata.EvidenceId, FName(TEXT("Evidence.Anomaly01")));
	TestEqual(TEXT("Default first anomaly record metadata should preserve display name."), RouteKit->ObjectiveNodes[3].EvidenceMetadata.DisplayName.ToString(), FString(TEXT("第一个异常")));
	TestEqual(TEXT("Default exit gate node should be the final corridor placement."), RouteKit->ObjectiveNodes[5].RelativeTransform.GetLocation(), FVector(2200.0f, 0.0f, 80.0f));
	TArray<FText> ValidationErrors;
	TestTrue(TEXT("Default route kit preset should validate cleanly."), RouteKit->ValidateObjectiveNodes(ValidationErrors));
	TestEqual(TEXT("Default route kit preset should not produce validation errors."), ValidationErrors.Num(), 0);
	Swap(RouteKit->ObjectiveNodes[1], RouteKit->ObjectiveNodes[2]);
	TestFalse(TEXT("Route kit validation should reject out-of-order first-loop objectives."), RouteKit->ValidateObjectiveNodes(ValidationErrors));
	TestTrue(TEXT("Route kit validation should report an order error."), ValidationErrors.Num() > 0);
	Swap(RouteKit->ObjectiveNodes[1], RouteKit->ObjectiveNodes[2]);
	ValidationErrors.Reset();

	RouteKit->ObjectiveNodes[4].TrailerBeatId = NAME_None;
	TestFalse(TEXT("Route kit validation should reject archive nodes without trailer beats."), RouteKit->ValidateObjectiveNodes(ValidationErrors));
	TestTrue(TEXT("Route kit validation should report missing archive trailer beat."), ValidationErrors.Num() > 0);
	RouteKit->ObjectiveNodes[4].TrailerBeatId = TEXT("Beat.ArchiveReview");
	ValidationErrors.Reset();

	RouteKit->ObjectiveNodes[5].TrailerBeatId = TEXT("Beat.ArchiveReview");
	TestFalse(TEXT("Route kit validation should reject duplicate trailer beats."), RouteKit->ValidateObjectiveNodes(ValidationErrors));
	TestTrue(TEXT("Route kit validation should report duplicate trailer beats."), ValidationErrors.Num() > 0);
	RouteKit->ObjectiveNodes[5].TrailerBeatId = TEXT("Beat.ExitGate");
	ValidationErrors.Reset();

	RouteKit->ObjectiveNodes[4].ObjectiveHint = FText();
	TestFalse(TEXT("Route kit validation should reject archive nodes without objective hints."), RouteKit->ValidateObjectiveNodes(ValidationErrors));
	TestTrue(TEXT("Route kit validation should report missing archive objective hint."), ValidationErrors.Num() > 0);
	RouteKit->ObjectiveNodes[4].ObjectiveHint = FText::FromString(TEXT("在档案终端查看录像。"));
	ValidationErrors.Reset();

	RouteKit->ObjectiveNodes[5].DebugLabel = FText();
	TestFalse(TEXT("Route kit validation should reject exit nodes without debug labels."), RouteKit->ValidateObjectiveNodes(ValidationErrors));
	TestTrue(TEXT("Route kit validation should report missing exit debug label."), ValidationErrors.Num() > 0);
	RouteKit->ObjectiveNodes[5].DebugLabel = FText::FromString(TEXT("出口闸门"));
	ValidationErrors.Reset();

	FDeepWaterStationObjectiveNode ExtraNode = RouteKit->ObjectiveNodes[5];
	ExtraNode.SourceId = TEXT("Exit.Extra");
	ExtraNode.TrailerBeatId = TEXT("Beat.ExitExtra");
	RouteKit->ObjectiveNodes.Add(ExtraNode);
	TestFalse(TEXT("Route kit validation should reject extra first-loop objective nodes."), RouteKit->ValidateObjectiveNodes(ValidationErrors));
	TestTrue(TEXT("Route kit validation should report extra objective nodes."), ValidationErrors.Num() > 0);
	RouteKit->ObjectiveNodes.RemoveAt(RouteKit->ObjectiveNodes.Num() - 1);
	ValidationErrors.Reset();

	RouteKit->ObjectiveNodes[3].bIsRecordingForFirstAnomalyRecord = false;
	TestFalse(TEXT("Route kit validation should require the first anomaly record node to require recording."), RouteKit->ValidateObjectiveNodes(ValidationErrors));
	TestTrue(TEXT("Route kit validation should report missing anomaly recording flag."), ValidationErrors.Num() > 0);
	RouteKit->ObjectiveNodes[3].bIsRecordingForFirstAnomalyRecord = true;
	ValidationErrors.Reset();

	RouteKit->ObjectiveNodes[2].bIsRecordingForFirstAnomalyRecord = true;
	TestFalse(TEXT("Route kit validation should reject recording flags on non-record objectives."), RouteKit->ValidateObjectiveNodes(ValidationErrors));
	TestTrue(TEXT("Route kit validation should report misplaced anomaly recording flag."), ValidationErrors.Num() > 0);
	RouteKit->ObjectiveNodes[2].bIsRecordingForFirstAnomalyRecord = false;
	ValidationErrors.Reset();

	TestEqual(TEXT("Route kit should spawn one interactable per objective node."), RouteKit->SpawnObjectiveNodes(), 6);
	TestEqual(TEXT("Repeated route kit spawning should no-op after nodes already exist."), RouteKit->SpawnObjectiveNodes(), 0);
	const TArray<AFoundFootageObjectiveInteractable*>& SpawnedInteractables = RouteKit->GetSpawnedObjectiveInteractablesForTests();
	TestEqual(TEXT("Route kit should retain only the first spawned objective interactables."), SpawnedInteractables.Num(), 6);
	if (SpawnedInteractables.Num() != 6)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	TestEqual(TEXT("Bodycam node should preserve its objective."), SpawnedInteractables[0]->Objective, EFoundFootageInteractableObjective::Bodycam);
	TestEqual(TEXT("Bodycam node should preserve its source id."), SpawnedInteractables[0]->SourceId, FName(TEXT("Evidence.Bodycam")));
	TestEqual(TEXT("Bodycam node should spawn at its corridor placement."), SpawnedInteractables[0]->GetActorLocation(), FVector(200.0f, 0.0f, 80.0f));
	TestEqual(TEXT("Bodycam node should preserve evidence metadata."), SpawnedInteractables[0]->EvidenceMetadata.DisplayName.ToString(), FString(TEXT("随身摄像机")));
	TestEqual(TEXT("Bodycam node should preserve its trailer beat id."), SpawnedInteractables[0]->TrailerBeatId, FName(TEXT("Beat.BodycamAcquire")));
	TestNotNull(TEXT("Bodycam node should expose a visible pickup mesh."), SpawnedInteractables[0]->GetVisualMeshComponentForTests());
	if (SpawnedInteractables[0]->GetVisualMeshComponentForTests())
	{
		TestNotNull(TEXT("Bodycam pickup mesh should resolve to a static mesh asset."), SpawnedInteractables[0]->GetVisualMeshComponentForTests()->GetStaticMesh());
	}
	TestEqual(TEXT("First note node should preserve note metadata."), SpawnedInteractables[1]->NoteMetadata.Title.ToString(), FString(TEXT("维护记录")));
	TestEqual(TEXT("First note node should preserve its objective hint."), SpawnedInteractables[1]->ObjectiveHint.ToString(), FString(TEXT("阅读第一份站内笔记。")));
	TestEqual(TEXT("First note node should preserve its debug label."), SpawnedInteractables[1]->DebugLabel.ToString(), FString(TEXT("站内笔记")));
	TestEqual(TEXT("Exit gate should spawn at the final corridor placement."), SpawnedInteractables[5]->GetActorLocation(), FVector(2200.0f, 0.0f, 80.0f));
	TestEqual(TEXT("Exit gate should preserve its trailer beat id."), SpawnedInteractables[5]->TrailerBeatId, FName(TEXT("Beat.ExitGate")));
	TestTrue(TEXT("First anomaly record should preserve recording precondition."), SpawnedInteractables[3]->bIsRecordingForFirstAnomalyRecord);
	TestEqual(TEXT("First anomaly record should preserve anomaly metadata."), SpawnedInteractables[3]->EvidenceMetadata.EvidenceId, FName(TEXT("Evidence.Anomaly01")));

	AHorrorEncounterDirector* EncounterDirector = RouteKit->SpawnEncounterDirector();
	TestNotNull(TEXT("Route kit should spawn a golem encounter director."), EncounterDirector);
	if (!EncounterDirector)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}
	TestEqual(TEXT("Route kit should prime the default encounter id."), EncounterDirector->GetEncounterId(), FName(TEXT("Encounter.GolemReveal01")));
	TestTrue(TEXT("Route kit should pass the default encounter threat class."), EncounterDirector->ThreatClass == AHorrorThreatCharacter::StaticClass());
	TestEqual(TEXT("Route kit should pass the authored encounter threat transform."), EncounterDirector->ThreatRelativeTransform.GetLocation(), FVector(100.0f, 0.0f, 0.0f));
	TestEqual(TEXT("Route kit should prime the encounter director."), EncounterDirector->GetEncounterPhase(), EHorrorEncounterPhase::Primed);
	TestEqual(TEXT("Repeated encounter director spawning should return the same director."), RouteKit->SpawnEncounterDirector(), EncounterDirector);
	TestFalse(TEXT("Route kit should not gate the route before encounter reveal."), RouteKit->IsRouteGatedByEncounter());
	AHorrorThreatCharacter* EncounterTarget = World->SpawnActor<AHorrorThreatCharacter>(FVector(1000.0f, 300.0f, 80.0f), FRotator::ZeroRotator);
	TestNotNull(TEXT("Route kit should spawn an encounter reveal target."), EncounterTarget);
	if (!EncounterTarget)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}
	TestTrue(TEXT("Route kit should trigger the encounter reveal for an in-radius target."), RouteKit->TriggerEncounterReveal(EncounterTarget));
	AHorrorThreatCharacter* RouteThreat = EncounterDirector->GetThreatActor();
	TestNotNull(TEXT("Route kit reveal should spawn the authored encounter threat."), RouteThreat);
	if (!RouteThreat)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}
	TestEqual(TEXT("Route kit encounter threat should spawn at its authored world position."), RouteThreat->GetActorLocation(), FVector(1100.0f, 300.0f, 80.0f));
	TestTrue(TEXT("Route kit encounter threat should activate during reveal."), RouteThreat->IsThreatActive());
	TestTrue(TEXT("Route kit encounter threat should detect the reveal target."), RouteThreat->GetDetectedTarget() == EncounterTarget);
	TestTrue(TEXT("Route kit should gate the route during encounter reveal."), RouteKit->IsRouteGatedByEncounter());
	TestFalse(TEXT("Route exit gate should reject interaction while encounter is gated but exit remains locked."), SpawnedInteractables[5]->CanInteract_Implementation(EncounterTarget, FHitResult()));
	TestFalse(TEXT("Route exit gate interaction should fail while encounter is gated but exit remains locked."), SpawnedInteractables[5]->Interact_Implementation(EncounterTarget, FHitResult()));
	TestEqual(TEXT("Rejected locked exit interaction should keep encounter revealed."), EncounterDirector->GetEncounterPhase(), EHorrorEncounterPhase::Revealed);
	TestTrue(TEXT("Rejected locked exit interaction should keep route gated."), RouteKit->IsRouteGatedByEncounter());
	TestFalse(TEXT("Rejected locked exit interaction should not unlock first-loop exit."), GameMode->IsExitUnlocked());
	TestTrue(TEXT("Rejected locked exit interaction should keep threat active."), RouteThreat->IsThreatActive());

	TestTrue(TEXT("Route bodycam should complete."), SpawnedInteractables[0]->TryCompleteObjective(GameMode));
	TestTrue(TEXT("Route first note should complete."), SpawnedInteractables[1]->TryCompleteObjective(GameMode));
	TestTrue(TEXT("Route anomaly candidate should register."), SpawnedInteractables[2]->TryCompleteObjective(GameMode));
	TestFalse(TEXT("Route anomaly record should require the lead player's camera to be recording."), SpawnedInteractables[3]->TryCompleteObjective(GameMode));

	AHorrorPlayerCharacter* PlayerCharacter = World->SpawnActor<AHorrorPlayerCharacter>();
	TestNotNull(TEXT("Route kit transient world should spawn a recording player."), PlayerCharacter);
	if (!PlayerCharacter)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}
	UQuantumCameraComponent* QuantumCamera = PlayerCharacter->GetQuantumCameraComponent();
	TestNotNull(TEXT("Route kit recording player should expose quantum camera."), QuantumCamera);
	if (!QuantumCamera)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}
	QuantumCamera->SetCameraAcquired(true);
	TestTrue(TEXT("Route kit recording player should acquire the camera."), QuantumCamera->IsCameraAcquired());
	QuantumCamera->SetCameraEnabled(true);
	TestTrue(TEXT("Route kit recording player should enable the camera."), QuantumCamera->IsCameraEnabled());
	TestTrue(TEXT("Route kit recording player should start recording."), QuantumCamera->StartRecording());
	TestTrue(TEXT("Route anomaly record should complete while the lead player's camera is recording."), SpawnedInteractables[3]->TryCompleteObjective(GameMode));
	TestFalse(TEXT("Route archive review should reject direct completion without an instigator."), SpawnedInteractables[4]->CanCompleteObjective(GameMode));
	TestFalse(TEXT("Route archive review should not complete directly without an instigator."), SpawnedInteractables[4]->TryCompleteObjective(GameMode));
	TestFalse(TEXT("Route exit gate should remain locked until archive review completes through player interaction."), SpawnedInteractables[5]->CanCompleteObjective(GameMode));
	TestFalse(TEXT("Route exit gate should not complete before archive review."), SpawnedInteractables[5]->TryCompleteObjective(GameMode));
	TestFalse(TEXT("Rejected direct archive review should leave archive unreviewed."), GameMode->HasReviewedArchive());
	TestTrue(TEXT("Route kit low-level encounter resolution remains available for direct sequencing."), RouteKit->ResolveEncounter());
	TestFalse(TEXT("Route kit should release the route after direct encounter resolution."), RouteKit->IsRouteGatedByEncounter());
	TestFalse(TEXT("Route exit gate should remain locked after rejected direct archive review."), SpawnedInteractables[5]->CanCompleteObjective(GameMode));
	TestFalse(TEXT("Route exit gate should not complete after rejected direct archive review."), SpawnedInteractables[5]->TryCompleteObjective(GameMode));
	TestFalse(TEXT("Route should leave exit locked after rejected direct archive review."), GameMode->IsExitUnlocked());

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDeepWaterStationRouteKitObjectiveEncounterBridgeTest,
	"HorrorProject.Game.DeepWaterStation.RouteKit.ObjectiveEncounterBridge",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FDeepWaterStationRouteKitObjectiveEncounterBridgeTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for route objective encounter bridge coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	World->GetWorldSettings()->DefaultGameMode = AHorrorGameModeBase::StaticClass();
	TestTrue(TEXT("Transient world should create the route bridge game mode."), World->SetGameMode(FURL()));
	AHorrorGameModeBase* GameMode = World->GetAuthGameMode<AHorrorGameModeBase>();
	AHorrorPlayerCharacter* PlayerCharacter = World->SpawnActor<AHorrorPlayerCharacter>(FVector(1000.0f, 300.0f, 80.0f), FRotator::ZeroRotator);
	TestNotNull(TEXT("Transient world should expose the route bridge game mode."), GameMode);
	TestNotNull(TEXT("Transient world should spawn a route bridge player."), PlayerCharacter);
	if (!GameMode || !PlayerCharacter)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	ADeepWaterStationRouteKit* RouteKit = World->SpawnActor<ADeepWaterStationRouteKit>();
	TestNotNull(TEXT("Route bridge test should spawn the route kit."), RouteKit);
	if (!RouteKit)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	RouteKit->ConfigureDefaultFirstLoopObjectiveNodes();
	TestEqual(TEXT("Route bridge test should spawn the full objective route."), RouteKit->SpawnObjectiveNodes(), 6);
	AHorrorEncounterDirector* EncounterDirector = RouteKit->SpawnEncounterDirector();
	TestNotNull(TEXT("Route bridge test should spawn an encounter director."), EncounterDirector);
	const TArray<AFoundFootageObjectiveInteractable*>& SpawnedInteractables = RouteKit->GetSpawnedObjectiveInteractablesForTests();
	TestEqual(TEXT("Route bridge test should retain six spawned objective interactables."), SpawnedInteractables.Num(), 6);
	if (!EncounterDirector || SpawnedInteractables.Num() != 6)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	const FHitResult EmptyHit;
	UHorrorEventBusSubsystem* EventBus = World->GetSubsystem<UHorrorEventBusSubsystem>();
	TestNotNull(TEXT("Route bridge test should expose the event bus."), EventBus);
	if (!EventBus)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}
	EventBus->ResetForTests();

	TestEqual(TEXT("Route bridge encounter should start primed."), EncounterDirector->GetEncounterPhase(), EHorrorEncounterPhase::Primed);
	TestFalse(TEXT("Route bridge should not gate the exit before archive review."), RouteKit->IsRouteGatedByEncounter());
	TestTrue(TEXT("Route bridge bodycam interaction should complete."), SpawnedInteractables[0]->Interact_Implementation(PlayerCharacter, EmptyHit));
	const TArray<FHorrorEventMessage>& BodycamHistory = EventBus->GetHistory();
	TestEqual(TEXT("Route bridge bodycam interaction should publish one event."), BodycamHistory.Num(), 1);
	if (BodycamHistory.Num() >= 1)
	{
		TestEqual(TEXT("Route bridge bodycam event should carry the bodycam source id."), BodycamHistory[0].SourceId, FName(TEXT("Evidence.Bodycam")));
		TestEqual(TEXT("Route bridge bodycam event should carry its trailer beat id."), BodycamHistory[0].TrailerBeatId, FName(TEXT("Beat.BodycamAcquire")));
		TestEqual(TEXT("Route bridge bodycam event should carry its objective hint."), BodycamHistory[0].ObjectiveHint.ToString(), FString(TEXT("找回随身摄像机。")));
		TestEqual(TEXT("Route bridge bodycam event should carry its debug label."), BodycamHistory[0].DebugLabel.ToString(), FString(TEXT("取得随身摄像机")));
	}
	TestTrue(TEXT("Route bridge first note interaction should complete."), SpawnedInteractables[1]->Interact_Implementation(PlayerCharacter, EmptyHit));
	TestTrue(TEXT("Route bridge anomaly candidate interaction should complete."), SpawnedInteractables[2]->Interact_Implementation(PlayerCharacter, EmptyHit));

	UQuantumCameraComponent* QuantumCamera = PlayerCharacter->GetQuantumCameraComponent();
	TestNotNull(TEXT("Route bridge player should expose quantum camera."), QuantumCamera);
	if (!QuantumCamera)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}
	QuantumCamera->SetCameraAcquired(true);
	QuantumCamera->SetCameraEnabled(true);
	TestTrue(TEXT("Route bridge player should start recording."), QuantumCamera->StartRecording());
	TestTrue(TEXT("Route bridge anomaly recording interaction should complete."), SpawnedInteractables[3]->Interact_Implementation(PlayerCharacter, EmptyHit));
	const TArray<FHorrorEventMessage>& AnomalyRecordHistory = EventBus->GetHistory();
	TestTrue(TEXT("Route bridge anomaly recording should publish an event."), AnomalyRecordHistory.Num() >= 3);
	if (AnomalyRecordHistory.Num() >= 3)
	{
		TestEqual(TEXT("Route bridge anomaly record event should carry the pending anomaly source id."), AnomalyRecordHistory[2].SourceId, FName(TEXT("Evidence.Anomaly01")));
		TestEqual(TEXT("Route bridge anomaly record event should carry record trailer beat id."), AnomalyRecordHistory[2].TrailerBeatId, FName(TEXT("Beat.FirstAnomalyRecord")));
		TestEqual(TEXT("Route bridge anomaly record event should carry record objective hint."), AnomalyRecordHistory[2].ObjectiveHint.ToString(), FString(TEXT("异常点可见时开始录制。")));
	}

	TestTrue(TEXT("Route bridge archive interaction should complete."), SpawnedInteractables[4]->Interact_Implementation(PlayerCharacter, EmptyHit));
	TestEqual(TEXT("Route bridge archive review should reveal the encounter."), EncounterDirector->GetEncounterPhase(), EHorrorEncounterPhase::Revealed);
	AHorrorThreatCharacter* RevealedThreat = EncounterDirector->GetThreatActor();
	TestNotNull(TEXT("Route bridge archive reveal should spawn the authored threat."), RevealedThreat);
	if (!RevealedThreat)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}
	TestTrue(TEXT("Route bridge revealed threat should activate."), RevealedThreat->IsThreatActive());
	TestTrue(TEXT("Route bridge revealed threat should detect the player."), RevealedThreat->GetDetectedTarget() == PlayerCharacter);
	TestTrue(TEXT("Route bridge should gate the exit during the reveal."), RouteKit->IsRouteGatedByEncounter());
	TestTrue(TEXT("Route bridge should report that the exit can resolve the encounter."), RouteKit->CanResolveEncounterAtExit());
	TestTrue(TEXT("Route bridge exit gate should be interactable while the encounter is active."), SpawnedInteractables[5]->CanInteract_Implementation(PlayerCharacter, EmptyHit));
	TestTrue(TEXT("Route bridge exit gate interaction should complete escape resolution."), SpawnedInteractables[5]->Interact_Implementation(PlayerCharacter, EmptyHit));
	TestEqual(TEXT("Route bridge exit interaction should resolve the encounter."), EncounterDirector->GetEncounterPhase(), EHorrorEncounterPhase::Resolved);
	TestFalse(TEXT("Route bridge should release the exit after encounter resolution."), RouteKit->IsRouteGatedByEncounter());
	TestFalse(TEXT("Route bridge resolved threat should deactivate."), RevealedThreat->IsThreatActive());
	TestTrue(TEXT("Route bridge should leave first-loop exit state unlocked."), GameMode->IsExitUnlocked());

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

#endif

#endif // WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR
