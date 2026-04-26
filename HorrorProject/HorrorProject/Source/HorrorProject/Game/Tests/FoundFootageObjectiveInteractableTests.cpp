// Copyright Epic Games, Inc. All Rights Reserved.

#if WITH_DEV_AUTOMATION_TESTS

#include "Game/FoundFootageObjectiveInteractable.h"

#include "AI/HorrorThreatCharacter.h"
#include "Game/DeepWaterStationRouteKit.h"
#include "Game/HorrorEncounterDirector.h"
#include "Game/HorrorEventBusSubsystem.h"
#include "Game/HorrorGameModeBase.h"
#include "Engine/World.h"
#include "GameFramework/WorldSettings.h"
#include "Interaction/InteractableInterface.h"
#include "Misc/AutomationTest.h"
#include "Player/HorrorPlayerCharacter.h"
#include "Player/Components/InventoryComponent.h"
#include "Player/Components/NoteRecorderComponent.h"
#include "Player/Components/QuantumCameraComponent.h"
#include "Tests/AutomationCommon.h"

namespace
{
	AFoundFootageObjectiveInteractable* NewObjectiveInteractable(EFoundFootageInteractableObjective Objective, FName SourceId = NAME_None)
	{
		AFoundFootageObjectiveInteractable* Interactable = NewObject<AFoundFootageObjectiveInteractable>();
		Interactable->Objective = Objective;
		Interactable->SourceId = SourceId;
		return Interactable;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFoundFootageObjectiveInteractableFirstLoopTest,
	"HorrorProject.Game.FoundFootage.ObjectiveInteractable.CompletesFirstLoop",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FFoundFootageObjectiveInteractableFirstLoopTest::RunTest(const FString& Parameters)
{
	AHorrorGameModeBase* GameMode = NewObject<AHorrorGameModeBase>();
	TestNotNull(TEXT("GameMode should be constructible without loading a map."), GameMode);
	if (!GameMode)
	{
		return false;
	}

	AFoundFootageObjectiveInteractable* Bodycam = NewObjectiveInteractable(EFoundFootageInteractableObjective::Bodycam, TEXT("BodycamPickup"));
	TestTrue(TEXT("Objective actor should implement the interactable interface."), Bodycam->GetClass()->ImplementsInterface(UInteractableInterface::StaticClass()));

	const FHitResult EmptyHit;
	TestFalse(TEXT("CanInteract should fail safely when the actor has no world game mode."), IInteractableInterface::Execute_CanInteract(Bodycam, nullptr, EmptyHit));
	TestFalse(TEXT("Interact should fail safely when the actor has no world game mode."), IInteractableInterface::Execute_Interact(Bodycam, nullptr, EmptyHit));
	TestFalse(TEXT("Null game mode should no-op safely."), Bodycam->TryCompleteObjective(nullptr));

	AFoundFootageObjectiveInteractable* InvalidObjective = NewObjectiveInteractable(static_cast<EFoundFootageInteractableObjective>(255), TEXT("Invalid"));
	TestFalse(TEXT("Invalid objectives should not be interactable."), InvalidObjective->CanCompleteObjective(GameMode));
	TestFalse(TEXT("Invalid objectives should no-op safely."), InvalidObjective->TryCompleteObjective(GameMode));

	AFoundFootageObjectiveInteractable* FirstNoteBeforeBodycam = NewObjectiveInteractable(EFoundFootageInteractableObjective::FirstNote, TEXT("NoteBeforeBodycam"));
	TestFalse(TEXT("First note interactable should be locked before bodycam acquisition."), FirstNoteBeforeBodycam->CanCompleteObjective(GameMode));
	TestFalse(TEXT("First note interactable should not complete before bodycam acquisition."), FirstNoteBeforeBodycam->TryCompleteObjective(GameMode));
	AFoundFootageObjectiveInteractable* FirstAnomalyBeforeNote = NewObjectiveInteractable(EFoundFootageInteractableObjective::FirstAnomalyCandidate, TEXT("AnomalyBeforeNote"));
	TestFalse(TEXT("First anomaly candidate should be locked before the first note."), FirstAnomalyBeforeNote->CanCompleteObjective(GameMode));
	TestFalse(TEXT("First anomaly candidate should not complete before the first note."), FirstAnomalyBeforeNote->TryCompleteObjective(GameMode));
	AFoundFootageObjectiveInteractable* ArchiveBeforeAnomaly = NewObjectiveInteractable(EFoundFootageInteractableObjective::ArchiveReview, TEXT("ArchiveBeforeAnomaly"));
	TestFalse(TEXT("Archive review should be locked before the first anomaly recording."), ArchiveBeforeAnomaly->CanCompleteObjective(GameMode));
	TestFalse(TEXT("Archive review should not complete before the first anomaly recording."), ArchiveBeforeAnomaly->TryCompleteObjective(GameMode));

	AFoundFootageObjectiveInteractable* ExitGate = NewObjectiveInteractable(EFoundFootageInteractableObjective::ExitRouteGate, TEXT("ExitGate"));
	TestFalse(TEXT("Exit route gate should start locked."), ExitGate->CanCompleteObjective(GameMode));
	TestFalse(TEXT("Locked exit route gate should not complete."), ExitGate->TryCompleteObjective(GameMode));

	TestFalse(TEXT("Exit should start locked."), GameMode->IsExitUnlocked());
	TestTrue(TEXT("Bodycam objective should complete through the placeable interactable."), Bodycam->TryCompleteObjective(GameMode));
	TestTrue(TEXT("Bodycam completion should be reflected by the GameMode."), GameMode->HasBodycamAcquired());
	TestFalse(TEXT("Completed bodycam objective should stop being interactable."), Bodycam->CanCompleteObjective(GameMode));
	TestFalse(TEXT("Duplicate bodycam completion should no-op safely."), Bodycam->TryCompleteObjective(GameMode));

	AFoundFootageObjectiveInteractable* FirstNote = NewObjectiveInteractable(EFoundFootageInteractableObjective::FirstNote, TEXT("Note01"));
	TestTrue(TEXT("First note objective should complete through the placeable interactable."), FirstNote->TryCompleteObjective(GameMode));
	TestTrue(TEXT("First note completion should be reflected by the GameMode."), GameMode->HasCollectedFirstNote());
	TestFalse(TEXT("Duplicate first note completion should no-op safely."), FirstNote->TryCompleteObjective(GameMode));

	AFoundFootageObjectiveInteractable* FirstAnomalyRecord = NewObjectiveInteractable(EFoundFootageInteractableObjective::FirstAnomalyRecord, TEXT("Recorder"));
	TestFalse(TEXT("First anomaly record should not be completable before a candidate exists."), FirstAnomalyRecord->CanCompleteObjective(GameMode));
	TestFalse(TEXT("First anomaly record should no-op safely before a candidate exists."), FirstAnomalyRecord->TryCompleteObjective(GameMode));
	TestFalse(TEXT("Rejected first anomaly record should not complete the objective."), GameMode->HasRecordedFirstAnomaly());

	AFoundFootageObjectiveInteractable* FirstAnomalyCandidate = NewObjectiveInteractable(EFoundFootageInteractableObjective::FirstAnomalyCandidate, TEXT("Anomaly01"));
	TestTrue(TEXT("First anomaly candidate should be completable before a pending candidate exists."), FirstAnomalyCandidate->CanCompleteObjective(GameMode));
	TestTrue(TEXT("First anomaly candidate should register through the placeable interactable."), FirstAnomalyCandidate->TryCompleteObjective(GameMode));
	TestFalse(TEXT("Pending first anomaly candidate should stop being completable."), FirstAnomalyCandidate->CanCompleteObjective(GameMode));
	TestFalse(TEXT("Duplicate pending first anomaly candidate should no-op safely."), FirstAnomalyCandidate->TryCompleteObjective(GameMode));
	TestFalse(TEXT("First anomaly record should not be completable while the player is not recording."), FirstAnomalyRecord->CanCompleteObjective(GameMode));
	TestFalse(TEXT("First anomaly record should not complete while the player is not recording."), FirstAnomalyRecord->TryCompleteObjective(GameMode));
	TestFalse(TEXT("Non-recording first anomaly record should not complete the objective."), GameMode->HasRecordedFirstAnomaly());
	FirstAnomalyRecord->bIsRecordingForFirstAnomalyRecord = true;
	TestFalse(TEXT("First anomaly record requires the lead player's camera to be recording, not the legacy test flag."), FirstAnomalyRecord->CanCompleteObjective(GameMode));
	TestFalse(TEXT("First anomaly record should not complete until the lead player's camera is recording."), FirstAnomalyRecord->TryCompleteObjective(GameMode));
	TestFalse(TEXT("Legacy recording flag should not complete the objective without the player camera recording."), GameMode->HasRecordedFirstAnomaly());
	TestTrue(TEXT("Game mode recording validation should still allow direct recording when an active recorder is supplied."), GameMode->TryRecordFirstAnomaly(true));
	TestTrue(TEXT("First anomaly completion should be reflected by the GameMode."), GameMode->HasRecordedFirstAnomaly());
	TestFalse(TEXT("Recorded first anomaly record should stop being completable."), FirstAnomalyRecord->CanCompleteObjective(GameMode));
	TestFalse(TEXT("Duplicate first anomaly record should no-op safely."), FirstAnomalyRecord->TryCompleteObjective(GameMode));
	AFoundFootageObjectiveInteractable* AlreadyRecordedFirstAnomalyCandidate = NewObjectiveInteractable(EFoundFootageInteractableObjective::FirstAnomalyCandidate, TEXT("Anomaly02"));
	TestFalse(TEXT("Already recorded first anomaly should prevent new candidate completion."), AlreadyRecordedFirstAnomalyCandidate->CanCompleteObjective(GameMode));
	TestFalse(TEXT("Already recorded first anomaly should no-op new candidate completion."), AlreadyRecordedFirstAnomalyCandidate->TryCompleteObjective(GameMode));

	TestFalse(TEXT("Partially completed first loop should keep exit route gate locked."), ExitGate->CanCompleteObjective(GameMode));

	AFoundFootageObjectiveInteractable* ArchiveReview = NewObjectiveInteractable(EFoundFootageInteractableObjective::ArchiveReview, TEXT("ArchiveTerminal"));
	TestFalse(TEXT("Archive review should reject direct completion without an instigator."), ArchiveReview->CanCompleteObjective(GameMode));
	TestFalse(TEXT("Archive review should not complete directly without an instigator."), ArchiveReview->TryCompleteObjective(GameMode));
	TestFalse(TEXT("Direct archive review rejection should leave archive unreviewed."), GameMode->HasReviewedArchive());

	TestFalse(TEXT("Exit should remain locked until archive review completes through a player interaction."), GameMode->IsExitUnlocked());

	return true;
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFoundFootageObjectiveInteractableWorldCanInteractTest,
	"HorrorProject.Game.FoundFootage.ObjectiveInteractable.CanInteractUsesCompletionPreconditions",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FFoundFootageObjectiveInteractableWorldCanInteractTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for CanInteract coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	World->GetWorldSettings()->DefaultGameMode = AHorrorGameModeBase::StaticClass();
	TestTrue(TEXT("Transient world should create the FP-01 game mode."), World->SetGameMode(FURL()));
	AHorrorGameModeBase* GameMode = World->GetAuthGameMode<AHorrorGameModeBase>();
	TestNotNull(TEXT("Transient world should expose the FP-01 game mode."), GameMode);
	if (!GameMode)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	const FHitResult EmptyHit;
	AFoundFootageObjectiveInteractable* FirstAnomalyRecord = World->SpawnActor<AFoundFootageObjectiveInteractable>();
	FirstAnomalyRecord->Objective = EFoundFootageInteractableObjective::FirstAnomalyRecord;
	FirstAnomalyRecord->SourceId = TEXT("WorldRecorder");
	TestFalse(TEXT("CanInteract should reject first anomaly recording before a candidate exists."), FirstAnomalyRecord->CanInteract_Implementation(nullptr, EmptyHit));
	TestFalse(TEXT("Recording interaction should no-op before a candidate exists."), FirstAnomalyRecord->Interact_Implementation(nullptr, EmptyHit));
	FirstAnomalyRecord->bIsRecordingForFirstAnomalyRecord = true;
	TestFalse(TEXT("CanInteract should reject first anomaly recording before a candidate exists even when recording."), FirstAnomalyRecord->CanInteract_Implementation(nullptr, EmptyHit));
	TestFalse(TEXT("Recording interaction should no-op before a candidate exists even when recording."), FirstAnomalyRecord->Interact_Implementation(nullptr, EmptyHit));
	FirstAnomalyRecord->bIsRecordingForFirstAnomalyRecord = false;

	AFoundFootageObjectiveInteractable* FirstAnomalyCandidate = World->SpawnActor<AFoundFootageObjectiveInteractable>();
	FirstAnomalyCandidate->Objective = EFoundFootageInteractableObjective::FirstAnomalyCandidate;
	FirstAnomalyCandidate->SourceId = TEXT("WorldAnomaly01");
	TestFalse(TEXT("CanInteract should reject the first anomaly candidate before the first note."), FirstAnomalyCandidate->CanInteract_Implementation(nullptr, EmptyHit));
	TestFalse(TEXT("Candidate interaction should no-op before the first note."), FirstAnomalyCandidate->Interact_Implementation(nullptr, EmptyHit));
	TestTrue(TEXT("Bodycam acquisition should enable first note collection in the world interaction test."), GameMode->TryAcquireBodycam(TEXT("WorldBodycam"), true));
	TestTrue(TEXT("First note collection should enable first anomaly candidates in the world interaction test."), GameMode->TryCollectFirstNote(TEXT("WorldNote")));
	TestTrue(TEXT("CanInteract should allow the first anomaly candidate after the first note."), FirstAnomalyCandidate->CanInteract_Implementation(nullptr, EmptyHit));
	TestTrue(TEXT("Candidate interaction should register the pending anomaly."), FirstAnomalyCandidate->Interact_Implementation(nullptr, EmptyHit));
	TestFalse(TEXT("CanInteract should reject duplicate pending first anomaly candidates."), FirstAnomalyCandidate->CanInteract_Implementation(nullptr, EmptyHit));
	TestFalse(TEXT("Duplicate pending candidate interaction should no-op."), FirstAnomalyCandidate->Interact_Implementation(nullptr, EmptyHit));

	TestFalse(TEXT("CanInteract should reject first anomaly recording while not recording."), FirstAnomalyRecord->CanInteract_Implementation(nullptr, EmptyHit));
	TestFalse(TEXT("Non-recording first anomaly record interaction should no-op."), FirstAnomalyRecord->Interact_Implementation(nullptr, EmptyHit));
	FirstAnomalyRecord->bIsRecordingForFirstAnomalyRecord = true;
	TestFalse(TEXT("CanInteract should still reject first anomaly recording when only the legacy recording flag is set."), FirstAnomalyRecord->CanInteract_Implementation(nullptr, EmptyHit));
	TestFalse(TEXT("Recording interaction should still no-op when only the legacy recording flag is set."), FirstAnomalyRecord->Interact_Implementation(nullptr, EmptyHit));
	TestFalse(TEXT("Legacy recording flag should not record the first anomaly without a recording player camera."), GameMode->HasRecordedFirstAnomaly());
	TestFalse(TEXT("CanInteract should reject first anomaly recording after it has already been recorded."), FirstAnomalyRecord->CanInteract_Implementation(nullptr, EmptyHit));
	TestFalse(TEXT("Duplicate recording interaction should no-op after the first anomaly is recorded."), FirstAnomalyRecord->Interact_Implementation(nullptr, EmptyHit));
	TestFalse(TEXT("CanInteract should reject first anomaly candidates while the original candidate is still pending."), FirstAnomalyCandidate->CanInteract_Implementation(nullptr, EmptyHit));
	TestFalse(TEXT("Candidate interaction should no-op while the original candidate is still pending."), FirstAnomalyCandidate->Interact_Implementation(nullptr, EmptyHit));

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFoundFootageObjectiveInteractableWritesInstigatorProgressTest,
	"HorrorProject.Game.FoundFootage.ObjectiveInteractable.WritesInstigatorProgress",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FFoundFootageObjectiveInteractableWritesInstigatorProgressTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for progress bridge coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	World->GetWorldSettings()->DefaultGameMode = AHorrorGameModeBase::StaticClass();
	TestTrue(TEXT("Transient world should create the progress bridge game mode."), World->SetGameMode(FURL()));
	AHorrorGameModeBase* GameMode = World->GetAuthGameMode<AHorrorGameModeBase>();
	AHorrorPlayerCharacter* PlayerCharacter = World->SpawnActor<AHorrorPlayerCharacter>();
	TestNotNull(TEXT("Transient world should expose the progress bridge game mode."), GameMode);
	TestNotNull(TEXT("Transient world should spawn the horror player character."), PlayerCharacter);
	if (!GameMode || !PlayerCharacter)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	UInventoryComponent* Inventory = PlayerCharacter->GetInventoryComponent();
	UNoteRecorderComponent* NoteRecorder = PlayerCharacter->GetNoteRecorderComponent();
	TestNotNull(TEXT("Progress bridge player should expose inventory."), Inventory);
	TestNotNull(TEXT("Progress bridge player should expose note recorder."), NoteRecorder);
	if (!Inventory || !NoteRecorder)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	const FHitResult EmptyHit;
	const FName RejectedId(TEXT("Evidence.Rejected"));
	AFoundFootageObjectiveInteractable* RejectedRecord = World->SpawnActor<AFoundFootageObjectiveInteractable>();
	RejectedRecord->Objective = EFoundFootageInteractableObjective::FirstAnomalyRecord;
	RejectedRecord->SourceId = RejectedId;
	RejectedRecord->EvidenceMetadata.EvidenceId = RejectedId;
	RejectedRecord->EvidenceMetadata.DisplayName = FText::FromString(TEXT("Rejected Evidence"));
	RejectedRecord->bIsRecordingForFirstAnomalyRecord = true;
	RejectedRecord->TrailerBeatId = TEXT("Beat.RejectedRecord");
	RejectedRecord->ObjectiveHint = FText::FromString(TEXT("Rejected recording hint"));
	RejectedRecord->DebugLabel = FText::FromString(TEXT("Rejected Recording"));
	TestFalse(TEXT("Rejected anomaly recording should not complete without a candidate."), RejectedRecord->Interact_Implementation(PlayerCharacter, EmptyHit));
	TestFalse(TEXT("Rejected anomaly recording should not write instigator evidence."), Inventory->HasCollectedEvidenceId(RejectedId));
	FHorrorEvidenceMetadata RejectedMetadata;
	TestFalse(TEXT("Rejected anomaly recording should not register metadata."), Inventory->GetEvidenceMetadata(RejectedId, RejectedMetadata));
	FHorrorObjectiveMessageMetadata RejectedObjectiveMetadata;
	UHorrorEventBusSubsystem* EventBus = World->GetSubsystem<UHorrorEventBusSubsystem>();
	TestNotNull(TEXT("Progress bridge world should expose the EventBus subsystem."), EventBus);
	if (!EventBus)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}
	const int32 HistoryCountBeforeRejectedRecord = EventBus->GetHistory().Num();
	TestFalse(TEXT("Rejected anomaly recording should not register objective event metadata."), EventBus->GetObjectiveMetadataForTests(HorrorFoundFootageTags::FirstAnomalyRecordedEvent(), RejectedId, RejectedObjectiveMetadata));

	const FName BodycamId(TEXT("Evidence.Bodycam"));
	AFoundFootageObjectiveInteractable* Bodycam = World->SpawnActor<AFoundFootageObjectiveInteractable>();
	Bodycam->Objective = EFoundFootageInteractableObjective::Bodycam;
	Bodycam->SourceId = BodycamId;
	Bodycam->EvidenceMetadata.EvidenceId = BodycamId;
	Bodycam->EvidenceMetadata.DisplayName = FText::FromString(TEXT("Bodycam"));
	TestTrue(TEXT("Bodycam interaction should complete."), Bodycam->Interact_Implementation(PlayerCharacter, EmptyHit));
	TestTrue(TEXT("Bodycam interaction should record evidence on the instigator."), Inventory->HasCollectedEvidenceId(BodycamId));
	FHorrorEvidenceMetadata BodycamMetadata;
	TestTrue(TEXT("Bodycam interaction should register evidence metadata."), Inventory->GetEvidenceMetadata(BodycamId, BodycamMetadata));
	TestEqual(TEXT("Bodycam metadata should preserve display name."), BodycamMetadata.DisplayName.ToString(), FString(TEXT("Bodycam")));

	const FName NoteId(TEXT("Note.Intro"));
	AFoundFootageObjectiveInteractable* FirstNote = World->SpawnActor<AFoundFootageObjectiveInteractable>();
	FirstNote->Objective = EFoundFootageInteractableObjective::FirstNote;
	FirstNote->SourceId = NoteId;
	FirstNote->NoteMetadata.NoteId = NoteId;
	FirstNote->NoteMetadata.Title = FText::FromString(TEXT("Intro"));
	TestTrue(TEXT("First note interaction should complete."), FirstNote->Interact_Implementation(PlayerCharacter, EmptyHit));
	TestTrue(TEXT("First note interaction should record a note on the instigator."), NoteRecorder->HasRecordedNoteId(NoteId));
	FHorrorNoteMetadata NoteMetadata;
	TestTrue(TEXT("First note interaction should register note metadata."), NoteRecorder->GetNoteMetadata(NoteId, NoteMetadata));
	TestEqual(TEXT("First note metadata should preserve title."), NoteMetadata.Title.ToString(), FString(TEXT("Intro")));

	const FName AnomalyId(TEXT("Evidence.Anomaly01"));
	const FName RecorderId(TEXT("Evidence.Recorder"));
	AFoundFootageObjectiveInteractable* FirstAnomalyCandidate = World->SpawnActor<AFoundFootageObjectiveInteractable>();
	FirstAnomalyCandidate->Objective = EFoundFootageInteractableObjective::FirstAnomalyCandidate;
	FirstAnomalyCandidate->SourceId = AnomalyId;
	FirstAnomalyCandidate->EvidenceMetadata.EvidenceId = AnomalyId;
	FirstAnomalyCandidate->EvidenceMetadata.DisplayName = FText::FromString(TEXT("First Anomaly"));
	FirstAnomalyCandidate->TrailerBeatId = TEXT("Beat.FirstAnomalyCandidate");
	FirstAnomalyCandidate->ObjectiveHint = FText::FromString(TEXT("Investigate the visible anomaly."));
	FirstAnomalyCandidate->DebugLabel = FText::FromString(TEXT("First Anomaly Candidate"));
	const int32 HistoryCountBeforeCandidate = EventBus->GetHistory().Num();
	TestTrue(TEXT("Anomaly candidate interaction should complete."), FirstAnomalyCandidate->Interact_Implementation(PlayerCharacter, EmptyHit));
	FHorrorObjectiveMessageMetadata CandidateObjectiveMetadata;
	TestTrue(TEXT("Anomaly candidate interaction should register source objective metadata."), EventBus->GetObjectiveMetadataForTests(AnomalyId, CandidateObjectiveMetadata));
	TestEqual(TEXT("Anomaly candidate metadata should preserve trailer beat."), CandidateObjectiveMetadata.TrailerBeatId, FName(TEXT("Beat.FirstAnomalyCandidate")));
	TestEqual(TEXT("Anomaly candidate metadata should preserve objective hint."), CandidateObjectiveMetadata.ObjectiveHint.ToString(), FString(TEXT("Investigate the visible anomaly.")));
	TestEqual(TEXT("Anomaly candidate metadata should preserve debug label."), CandidateObjectiveMetadata.DebugLabel.ToString(), FString(TEXT("First Anomaly Candidate")));
	const int32 HistoryCountAfterCandidate = EventBus->GetHistory().Num();
	TestEqual(TEXT("Anomaly candidate interaction should not publish new objective history."), HistoryCountAfterCandidate, HistoryCountBeforeCandidate);
	TestFalse(TEXT("Duplicate anomaly candidate interaction should not complete."), FirstAnomalyCandidate->Interact_Implementation(PlayerCharacter, EmptyHit));
	TestEqual(TEXT("Duplicate anomaly candidate interaction should not publish objective history."), EventBus->GetHistory().Num(), HistoryCountAfterCandidate);

	AFoundFootageObjectiveInteractable* FirstAnomalyRecord = World->SpawnActor<AFoundFootageObjectiveInteractable>();
	FirstAnomalyRecord->Objective = EFoundFootageInteractableObjective::FirstAnomalyRecord;
	FirstAnomalyRecord->SourceId = RecorderId;
	FirstAnomalyRecord->EvidenceMetadata.EvidenceId = AnomalyId;
	FirstAnomalyRecord->EvidenceMetadata.DisplayName = FText::FromString(TEXT("First Anomaly"));
	FirstAnomalyRecord->bIsRecordingForFirstAnomalyRecord = true;
	TestFalse(TEXT("Anomaly record interaction should require the player's camera to be recording."), FirstAnomalyRecord->Interact_Implementation(PlayerCharacter, EmptyHit));
	TestFalse(TEXT("Rejected camera recording should not record candidate evidence."), Inventory->HasCollectedEvidenceId(AnomalyId));
	UQuantumCameraComponent* QuantumCamera = PlayerCharacter->GetQuantumCameraComponent();
	TestNotNull(TEXT("Progress bridge player should expose quantum camera."), QuantumCamera);
	if (!QuantumCamera)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}
	QuantumCamera->SetCameraAcquired(true);
	TestTrue(TEXT("Progress bridge test should acquire the player camera."), QuantumCamera->IsCameraAcquired());
	QuantumCamera->SetCameraEnabled(true);
	TestTrue(TEXT("Progress bridge test should enable the player camera."), QuantumCamera->IsCameraEnabled());
	TestTrue(TEXT("Progress bridge test should start recording through the player camera."), QuantumCamera->StartRecording());
	TestTrue(TEXT("Anomaly record interaction should complete while the player's camera is recording."), FirstAnomalyRecord->Interact_Implementation(PlayerCharacter, EmptyHit));
	TestTrue(TEXT("Anomaly record interaction should record the candidate evidence id."), Inventory->HasCollectedEvidenceId(AnomalyId));
	TestFalse(TEXT("Anomaly record interaction should not record the recorder id."), Inventory->HasCollectedEvidenceId(RecorderId));
	FHorrorEvidenceMetadata AnomalyMetadata;
	TestTrue(TEXT("Anomaly candidate interaction should register metadata for the later recording."), Inventory->GetEvidenceMetadata(AnomalyId, AnomalyMetadata));
	TestEqual(TEXT("Anomaly metadata should preserve display name."), AnomalyMetadata.DisplayName.ToString(), FString(TEXT("First Anomaly")));

	const FName ArchiveId(TEXT("Evidence.ArchiveReview"));
	AFoundFootageObjectiveInteractable* ArchiveReview = World->SpawnActor<AFoundFootageObjectiveInteractable>();
	ArchiveReview->Objective = EFoundFootageInteractableObjective::ArchiveReview;
	ArchiveReview->SourceId = ArchiveId;
	ArchiveReview->EvidenceMetadata.EvidenceId = ArchiveId;
	ArchiveReview->EvidenceMetadata.DisplayName = FText::FromString(TEXT("Archive Review"));
	const TArray<FName> RecordedNoteIds = NoteRecorder->GetRecordedNoteIds();
	const TArray<FName> CollectedEvidenceIds = Inventory->GetCollectedEvidenceIds();
	NoteRecorder->ImportRecordedNoteIds(TArray<FName>());
	TestFalse(TEXT("Archive review should reject players without recorded notes."), ArchiveReview->CanCompleteObjectiveForInstigator(GameMode, PlayerCharacter));
	TestFalse(TEXT("Archive review interaction should not complete without recorded notes."), ArchiveReview->Interact_Implementation(PlayerCharacter, EmptyHit));
	TestFalse(TEXT("Rejected archive review should not write archive evidence."), Inventory->HasCollectedEvidenceId(ArchiveId));
	NoteRecorder->ImportRecordedNoteIds(TArray<FName>{ FName(TEXT("Note.Unrelated")) });
	TestFalse(TEXT("Archive review should reject unrelated note progress."), ArchiveReview->CanCompleteObjectiveForInstigator(GameMode, PlayerCharacter));
	NoteRecorder->ImportRecordedNoteIds(RecordedNoteIds);
	Inventory->ImportCollectedEvidenceIds(TArray<FName>{ BodycamId, FName(TEXT("Evidence.Unrelated")) });
	TestFalse(TEXT("Archive review should reject unrelated anomaly evidence."), ArchiveReview->CanCompleteObjectiveForInstigator(GameMode, PlayerCharacter));
	Inventory->ImportCollectedEvidenceIds(TArray<FName>{ FName(TEXT("Evidence.Unrelated")), AnomalyId });
	TestFalse(TEXT("Archive review should reject missing bodycam evidence."), ArchiveReview->CanCompleteObjectiveForInstigator(GameMode, PlayerCharacter));
	Inventory->ImportCollectedEvidenceIds(CollectedEvidenceIds);
	TestTrue(TEXT("Archive review should allow players with authored first-loop evidence and note progress."), ArchiveReview->CanCompleteObjectiveForInstigator(GameMode, PlayerCharacter));
	TestTrue(TEXT("Archive review interaction should unlock the exit route."), ArchiveReview->Interact_Implementation(PlayerCharacter, EmptyHit));
	TestTrue(TEXT("Archive review should write archive evidence progress."), Inventory->HasCollectedEvidenceId(ArchiveId));
	FHorrorEvidenceMetadata ArchiveMetadata;
	TestTrue(TEXT("Archive review should register archive evidence metadata."), Inventory->GetEvidenceMetadata(ArchiveId, ArchiveMetadata));
	TestEqual(TEXT("Archive review metadata should preserve display name."), ArchiveMetadata.DisplayName.ToString(), FString(TEXT("Archive Review")));

	AFoundFootageObjectiveInteractable* ExitGate = World->SpawnActor<AFoundFootageObjectiveInteractable>();
	ExitGate->Objective = EFoundFootageInteractableObjective::ExitRouteGate;
	const int32 EvidenceCountBeforeExit = Inventory->GetCollectedEvidenceCount();
	const int32 NoteCountBeforeExit = NoteRecorder->GetRecordedNoteCount();
	TestTrue(TEXT("Unlocked exit route gate interaction should complete."), ExitGate->Interact_Implementation(PlayerCharacter, EmptyHit));
	TestEqual(TEXT("Exit route gate should not write evidence progress."), Inventory->GetCollectedEvidenceCount(), EvidenceCountBeforeExit);
	TestEqual(TEXT("Exit route gate should not write note progress."), NoteRecorder->GetRecordedNoteCount(), NoteCountBeforeExit);

	ADeepWaterStationRouteKit* RouteKit = World->SpawnActor<ADeepWaterStationRouteKit>();
	TestNotNull(TEXT("Objective progress bridge should spawn a route kit for owner isolation coverage."), RouteKit);
	if (!RouteKit)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}
	RouteKit->SetActorLocation(FVector(10000.0f, 0.0f, 0.0f));
	AHorrorEncounterDirector* EncounterDirector = RouteKit->SpawnEncounterDirector();
	TestNotNull(TEXT("Objective progress bridge should spawn a route kit encounter director."), EncounterDirector);
	if (!EncounterDirector)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}
	AFoundFootageObjectiveInteractable* RouteOwnedArchiveReview = World->SpawnActor<AFoundFootageObjectiveInteractable>();
	RouteOwnedArchiveReview->SetOwner(RouteKit);
	RouteOwnedArchiveReview->Objective = EFoundFootageInteractableObjective::ArchiveReview;
	RouteOwnedArchiveReview->SourceId = TEXT("Evidence.RouteOwnedArchiveReview");
	TestFalse(TEXT("Route-owned archive review should reject players outside the encounter reveal radius."), RouteOwnedArchiveReview->CanCompleteObjectiveForInstigator(GameMode, PlayerCharacter));
	TestFalse(TEXT("Route-owned archive review should not complete when encounter reveal would fail."), RouteOwnedArchiveReview->Interact_Implementation(PlayerCharacter, EmptyHit));
	TestEqual(TEXT("Rejected route-owned archive review should leave the encounter primed."), EncounterDirector->GetEncounterPhase(), EHorrorEncounterPhase::Primed);

	AFoundFootageObjectiveInteractable* StandaloneArchiveReview = World->SpawnActor<AFoundFootageObjectiveInteractable>();
	StandaloneArchiveReview->Objective = EFoundFootageInteractableObjective::ArchiveReview;
	StandaloneArchiveReview->SourceId = TEXT("Evidence.StandaloneArchiveReview");
	TestFalse(TEXT("Standalone duplicate archive review should not reveal a route kit encounter."), StandaloneArchiveReview->Interact_Implementation(PlayerCharacter, EmptyHit));
	TestEqual(TEXT("Unowned archive review interactions should leave route kit encounters primed."), EncounterDirector->GetEncounterPhase(), EHorrorEncounterPhase::Primed);
	TestFalse(TEXT("Unowned archive review interactions should not spawn a route kit threat."), EncounterDirector->GetThreatActor() != nullptr);

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

#endif

