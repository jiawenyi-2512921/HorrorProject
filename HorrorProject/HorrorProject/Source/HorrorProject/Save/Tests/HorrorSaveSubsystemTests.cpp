#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR && HORRORPROJECT_ENABLE_LEGACY_AUTOMATION_TESTS

// Copyright Epic Games, Inc. All Rights Reserved.

#if WITH_DEV_AUTOMATION_TESTS

#include "Save/HorrorSaveGame.h"
#include "Engine/World.h"
#include "Game/HorrorEventBusSubsystem.h"
#include "Game/HorrorFoundFootageContract.h"
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
#include "Player/Components/VHSEffectComponent.h"
#include "Save/HorrorSaveSubsystem.h"
#include "Tests/AutomationCommon.h"
namespace
{
	AHorrorGameModeBase* CreateSaveTestGameMode(FAutomationTestBase& Test, FTestWorldWrapper& TestWorld, UWorld*& OutWorld)
	{
		OutWorld = nullptr;
		Test.TestTrue(TEXT("Transient game world should be created for Save coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
		OutWorld = TestWorld.GetTestWorld();
		if (!OutWorld)
		{
			return nullptr;
		}

		OutWorld->GetWorldSettings()->DefaultGameMode = AHorrorGameModeBase::StaticClass();
		Test.TestTrue(TEXT("Transient world should create the save test game mode."), OutWorld->SetGameMode(FURL()));

		AHorrorGameModeBase* GameMode = OutWorld->GetAuthGameMode<AHorrorGameModeBase>();
		Test.TestNotNull(TEXT("Transient world should expose the save test game mode."), GameMode);
		if (!GameMode)
		{
			TestWorld.DestroyTestWorld(false);
		}

		return GameMode;
	}

	AHorrorPlayerCharacter* SpawnSaveTestPlayer(FAutomationTestBase& Test, UWorld* World)
	{
		if (!World)
		{
			return nullptr;
		}

		APlayerController* PlayerController = World->SpawnActor<APlayerController>();
		AHorrorPlayerCharacter* PlayerCharacter = World->SpawnActor<AHorrorPlayerCharacter>();
		Test.TestNotNull(TEXT("Transient world should spawn a player controller."), PlayerController);
		Test.TestNotNull(TEXT("Transient world should spawn the horror player character."), PlayerCharacter);
		if (PlayerController)
		{
			PlayerController->PlayerState = World->SpawnActor<APlayerState>();
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
	FHorrorFoundFootageSaveStateRoundTripTest,
	"HorrorProject.Save.FoundFootageContract.RoundTripsSaveState",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorFoundFootageSaveStateRoundTripTest::RunTest(const FString& Parameters)
{
	FHorrorFoundFootageContract SourceContract;
	SourceContract.RecordEvent(HorrorFoundFootageTags::BodycamAcquiredEvent(), TEXT("Bodycam"));
	SourceContract.RecordEvent(HorrorFoundFootageTags::FirstNoteCollectedEvent(), TEXT("FirstNote"));
	SourceContract.RecordEvent(HorrorFoundFootageTags::FirstAnomalyRecordedEvent(), TEXT("FirstAnomaly"));
	SourceContract.RecordEvent(HorrorFoundFootageTags::ArchiveReviewedEvent(), TEXT("ArchiveReviewed"));

	const FHorrorFoundFootageSaveState SaveState = SourceContract.ExportSaveState();

	FHorrorFoundFootageContract RestoredContract;
	RestoredContract.ImportSaveState(SaveState);

	TestTrue(TEXT("Bodycam event should round-trip."), RestoredContract.HasRecordedEvent(HorrorFoundFootageTags::BodycamAcquiredEvent()));
	TestTrue(TEXT("First note event should round-trip."), RestoredContract.HasRecordedEvent(HorrorFoundFootageTags::FirstNoteCollectedEvent()));
	TestTrue(TEXT("Archive reviewed event should round-trip."), RestoredContract.HasRecordedEvent(HorrorFoundFootageTags::ArchiveReviewedEvent()));
	TestTrue(TEXT("Exit unlocked event should round-trip."), RestoredContract.HasRecordedEvent(HorrorFoundFootageTags::ExitUnlockedEvent()));

	TestTrue(TEXT("Bodycam state should round-trip."), RestoredContract.HasCompletedState(HorrorFoundFootageTags::BodycamAcquiredState()));
	TestTrue(TEXT("First note state should round-trip."), RestoredContract.HasCompletedState(HorrorFoundFootageTags::FirstNoteCollectedState()));
	TestTrue(TEXT("Archive reviewed state should round-trip."), RestoredContract.HasCompletedState(HorrorFoundFootageTags::ArchiveReviewedState()));
	TestTrue(TEXT("Exit unlocked state should round-trip."), RestoredContract.HasCompletedState(HorrorFoundFootageTags::ExitUnlockedState()));
	TestTrue(TEXT("Exit should remain unlocked after restore."), RestoredContract.IsExitUnlocked());

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorSaveImportIdsSanitizesAndPreservesOrderTest,
	"HorrorProject.Save.ImportIds.SanitizesAndPreservesOrder",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorSaveImportIdsSanitizesAndPreservesOrderTest::RunTest(const FString& Parameters)
{
	UInventoryComponent* InventoryComponent = NewObject<UInventoryComponent>();
	const FName FirstEvidenceId(TEXT("Evidence.Bodycam"));
	const FName SecondEvidenceId(TEXT("Evidence.Note01"));
	InventoryComponent->ImportCollectedEvidenceIds({ NAME_None, FirstEvidenceId, FirstEvidenceId, SecondEvidenceId, NAME_None });

	const TArray<FName>& EvidenceIds = InventoryComponent->ExportCollectedEvidenceIds();
	TestEqual(TEXT("Imported evidence should reject NAME_None and duplicates."), EvidenceIds.Num(), 2);
	TestEqual(TEXT("First unique evidence ID should remain first."), EvidenceIds[0], FirstEvidenceId);
	TestEqual(TEXT("Second unique evidence ID should remain second."), EvidenceIds[1], SecondEvidenceId);
	TestFalse(TEXT("NAME_None evidence ID should not be queryable after import."), InventoryComponent->HasCollectedEvidenceId(NAME_None));

	UNoteRecorderComponent* NoteRecorderComponent = NewObject<UNoteRecorderComponent>();
	const FName FirstNoteId(TEXT("Note.Intro"));
	const FName SecondNoteId(TEXT("Note.Archive01"));
	NoteRecorderComponent->ImportRecordedNoteIds({ NAME_None, FirstNoteId, FirstNoteId, SecondNoteId, NAME_None });

	const TArray<FName>& NoteIds = NoteRecorderComponent->ExportRecordedNoteIds();
	TestEqual(TEXT("Imported notes should reject NAME_None and duplicates."), NoteIds.Num(), 2);
	TestEqual(TEXT("First unique note ID should remain first."), NoteIds[0], FirstNoteId);
	TestEqual(TEXT("Second unique note ID should remain second."), NoteIds[1], SecondNoteId);
	TestFalse(TEXT("NAME_None note ID should not be queryable after import."), NoteRecorderComponent->HasRecordedNoteId(NAME_None));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorSaveGameSnapshotContainsDay1StateTest,
	"HorrorProject.Save.SaveGame.ContainsDay1SnapshotState",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorSaveGameSnapshotContainsDay1StateTest::RunTest(const FString& Parameters)
{
	FHorrorFoundFootageContract Contract;
	Contract.RecordEvent(HorrorFoundFootageTags::BodycamAcquiredEvent(), TEXT("Bodycam"));
	Contract.RecordEvent(HorrorFoundFootageTags::FirstNoteCollectedEvent(), TEXT("FirstNote"));
	Contract.RecordEvent(HorrorFoundFootageTags::FirstAnomalyRecordedEvent(), TEXT("FirstAnomaly"));
	Contract.RecordEvent(HorrorFoundFootageTags::ArchiveReviewedEvent(), TEXT("ArchiveReviewed"));

	UInventoryComponent* InventoryComponent = NewObject<UInventoryComponent>();
	const FName FirstEvidenceId(TEXT("Evidence.Bodycam"));
	const FName SecondEvidenceId(TEXT("Evidence.Archive"));
	InventoryComponent->ImportCollectedEvidenceIds({ FirstEvidenceId, SecondEvidenceId });

	UNoteRecorderComponent* NoteRecorderComponent = NewObject<UNoteRecorderComponent>();
	const FName FirstNoteId(TEXT("Note.Intro"));
	const FName SecondNoteId(TEXT("Note.Archive01"));
	NoteRecorderComponent->ImportRecordedNoteIds({ FirstNoteId, SecondNoteId });

	UHorrorSaveGame* SaveGame = NewObject<UHorrorSaveGame>();
	const FName CheckpointId(TEXT("Checkpoint.Day1.Entry"));
	const FHorrorFoundFootageSaveState FoundFootageSaveState = Contract.ExportSaveState();
	const FName PendingAnomalyId(TEXT("Evidence.Anomaly01"));
	SaveGame->CheckpointId = CheckpointId;
	SaveGame->RecordedObjectiveEvents = FoundFootageSaveState.RecordedObjectiveEvents;
	SaveGame->CompletedObjectiveStates = FoundFootageSaveState.CompletedObjectiveStates;
	SaveGame->PendingFirstAnomalySourceId = PendingAnomalyId;
	SaveGame->CollectedEvidenceIds = InventoryComponent->ExportCollectedEvidenceIds();
	SaveGame->RecordedNoteIds = NoteRecorderComponent->ExportRecordedNoteIds();

	TestEqual(TEXT("Save version should be Day 1 version."), SaveGame->SaveVersion, 1);
	TestEqual(TEXT("Checkpoint ID should be captured."), SaveGame->CheckpointId, CheckpointId);
	TestTrue(TEXT("Bodycam objective event should be captured."), SaveGame->RecordedObjectiveEvents.Contains(HorrorFoundFootageTags::BodycamAcquiredEvent()));
	TestTrue(TEXT("Exit objective event should be captured."), SaveGame->RecordedObjectiveEvents.Contains(HorrorFoundFootageTags::ExitUnlockedEvent()));
	TestTrue(TEXT("Bodycam objective state should be captured."), SaveGame->CompletedObjectiveStates.Contains(HorrorFoundFootageTags::BodycamAcquiredState()));
	TestTrue(TEXT("Exit objective state should be captured."), SaveGame->CompletedObjectiveStates.Contains(HorrorFoundFootageTags::ExitUnlockedState()));
	TestEqual(TEXT("Pending anomaly source should be captured."), SaveGame->PendingFirstAnomalySourceId, PendingAnomalyId);
	TestEqual(TEXT("Evidence IDs should be captured in order."), SaveGame->CollectedEvidenceIds.Num(), 2);
	TestEqual(TEXT("First evidence ID should be captured first."), SaveGame->CollectedEvidenceIds[0], FirstEvidenceId);
	TestEqual(TEXT("Second evidence ID should be captured second."), SaveGame->CollectedEvidenceIds[1], SecondEvidenceId);
	TestEqual(TEXT("Note IDs should be captured in order."), SaveGame->RecordedNoteIds.Num(), 2);
	TestEqual(TEXT("First note ID should be captured first."), SaveGame->RecordedNoteIds[0], FirstNoteId);
	TestEqual(TEXT("Second note ID should be captured second."), SaveGame->RecordedNoteIds[1], SecondNoteId);

	return true;
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorSaveSubsystemSnapshotRequiresCompleteTargetsTest,
	"HorrorProject.Save.Subsystem.SnapshotRequiresCompleteTargets",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorSaveSubsystemSnapshotRequiresCompleteTargetsTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	UWorld* World = nullptr;
	AHorrorGameModeBase* GameMode = CreateSaveTestGameMode(*this, TestWorld, World);
	if (!GameMode || !World)
	{
		return false;
	}

	UHorrorSaveSubsystem* SaveSubsystem = World->GetGameInstance()
		? World->GetGameInstance()->GetSubsystem<UHorrorSaveSubsystem>()
		: nullptr;
	TestNotNull(TEXT("Save subsystem test object should be created."), SaveSubsystem);
	if (!SaveSubsystem)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	TestNull(TEXT("Snapshot creation should fail before a restorable player exists."), SaveSubsystem->CreateCheckpointSnapshot(World, TEXT("Checkpoint.NoPlayer")));

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorSaveSubsystemSnapshotRoundTripTest,
	"HorrorProject.Save.Subsystem.SnapshotRoundTripsDay1State",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorSaveSubsystemSnapshotRoundTripTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	UWorld* World = nullptr;
	AHorrorGameModeBase* GameMode = CreateSaveTestGameMode(*this, TestWorld, World);
	if (!GameMode || !World)
	{
		return false;
	}

	AHorrorPlayerCharacter* PlayerCharacter = SpawnSaveTestPlayer(*this, World);
	if (!PlayerCharacter)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	UInventoryComponent* Inventory = PlayerCharacter->GetInventoryComponent();
	UNoteRecorderComponent* NoteRecorder = PlayerCharacter->GetNoteRecorderComponent();
	UVHSEffectComponent* VHSEffect = PlayerCharacter->GetVHSEffectComponent();
	UQuantumCameraComponent* QuantumCamera = PlayerCharacter->GetQuantumCameraComponent();
	TestNotNull(TEXT("Save test player should expose inventory."), Inventory);
	TestNotNull(TEXT("Save test player should expose note recorder."), NoteRecorder);
	TestNotNull(TEXT("Save test player should expose VHS effect."), VHSEffect);
	TestNotNull(TEXT("Save test player should expose quantum camera."), QuantumCamera);
	if (!Inventory || !NoteRecorder || !VHSEffect || !QuantumCamera)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	const FName EvidenceId(TEXT("Evidence.Bodycam"));
	const FName NoteId(TEXT("Note.Intro"));
	TestTrue(TEXT("Bodycam objective should record before snapshot."), GameMode->TryAcquireBodycam(TEXT("BodycamPickup"), true));
	TestTrue(TEXT("VHS feedback should be active before snapshot."), VHSEffect->IsFeedbackActive());
	TestEqual(TEXT("Quantum camera should be in viewfinder before snapshot."), QuantumCamera->GetCameraMode(), EQuantumCameraMode::Viewfinder);
	TestTrue(TEXT("Evidence should record before snapshot."), Inventory->AddCollectedEvidenceId(EvidenceId));
	TestTrue(TEXT("Note should record before snapshot."), NoteRecorder->AddRecordedNoteId(NoteId));

	UHorrorSaveSubsystem* SaveSubsystem = World->GetGameInstance()
		? World->GetGameInstance()->GetSubsystem<UHorrorSaveSubsystem>()
		: nullptr;
	TestNotNull(TEXT("Save subsystem test object should be created."), SaveSubsystem);
	if (!SaveSubsystem)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	const FName CheckpointId(TEXT("Checkpoint.Day1.Test"));
	UHorrorSaveGame* SaveGame = SaveSubsystem->CreateCheckpointSnapshot(World, CheckpointId);
	TestNotNull(TEXT("Snapshot creation should succeed when all save targets exist."), SaveGame);
	if (!SaveGame)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	GameMode->ImportFoundFootageSaveState(FHorrorFoundFootageSaveState());
	Inventory->ImportCollectedEvidenceIds({});
	NoteRecorder->ImportRecordedNoteIds({});
	TestFalse(TEXT("Bodycam objective should be cleared before restore."), GameMode->HasBodycamAcquired());
	TestFalse(TEXT("VHS feedback should be inactive before restore."), VHSEffect->IsFeedbackActive());
	TestEqual(TEXT("Quantum camera should be disabled before restore."), QuantumCamera->GetCameraMode(), EQuantumCameraMode::Disabled);
	TestFalse(TEXT("Evidence should be cleared before restore."), Inventory->HasCollectedEvidenceId(EvidenceId));
	TestFalse(TEXT("Note should be cleared before restore."), NoteRecorder->HasRecordedNoteId(NoteId));

	SaveGame->SaveVersion = 2;
	TestFalse(TEXT("Snapshot application should reject unsupported save versions."), SaveSubsystem->ApplyCheckpointSnapshot(World, SaveGame));
	SaveGame->SaveVersion = 1;

	TestTrue(TEXT("Snapshot application should restore complete Day 1 state."), SaveSubsystem->ApplyCheckpointSnapshot(World, SaveGame));
	TestTrue(TEXT("Bodycam objective should restore from snapshot."), GameMode->HasBodycamAcquired());
	TestTrue(TEXT("VHS feedback should reactivate after restoring bodycam state."), VHSEffect->IsFeedbackActive());
	TestEqual(TEXT("Quantum camera should restore to viewfinder after bodycam state restore."), QuantumCamera->GetCameraMode(), EQuantumCameraMode::Viewfinder);
	TestTrue(TEXT("Evidence should restore from snapshot."), Inventory->HasCollectedEvidenceId(EvidenceId));
	TestTrue(TEXT("Note should restore from snapshot."), NoteRecorder->HasRecordedNoteId(NoteId));
	TestEqual(TEXT("Checkpoint id should be preserved in snapshot."), SaveGame->CheckpointId, CheckpointId);

	UHorrorSaveGame* DefaultCheckpointSaveGame = SaveSubsystem->CreateCheckpointSnapshot(World, NAME_None);
	TestNotNull(TEXT("Snapshot creation should allow empty checkpoint ids by applying the Day 1 default."), DefaultCheckpointSaveGame);
	if (DefaultCheckpointSaveGame)
	{
		TestEqual(TEXT("Empty checkpoint ids should normalize to the Day 1 autosave checkpoint."), DefaultCheckpointSaveGame->CheckpointId, FName(TEXT("Checkpoint.Day1.Auto")));
	}

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorSaveSubsystemSnapshotRoundTripsPendingFirstAnomalyCandidateTest,
	"HorrorProject.Save.Subsystem.SnapshotRoundTripsPendingFirstAnomalyCandidate",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorSaveSubsystemSnapshotRoundTripsPendingFirstAnomalyCandidateTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	UWorld* World = nullptr;
	AHorrorGameModeBase* GameMode = CreateSaveTestGameMode(*this, TestWorld, World);
	if (!GameMode || !World)
	{
		return false;
	}

	AHorrorPlayerCharacter* PlayerCharacter = SpawnSaveTestPlayer(*this, World);
	if (!PlayerCharacter)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	UQuantumCameraComponent* QuantumCamera = PlayerCharacter->GetQuantumCameraComponent();
	UInventoryComponent* Inventory = PlayerCharacter->GetInventoryComponent();
	UNoteRecorderComponent* NoteRecorder = PlayerCharacter->GetNoteRecorderComponent();
	UHorrorEventBusSubsystem* EventBus = World->GetSubsystem<UHorrorEventBusSubsystem>();
	UHorrorSaveSubsystem* SaveSubsystem = World->GetGameInstance()
		? World->GetGameInstance()->GetSubsystem<UHorrorSaveSubsystem>()
		: nullptr;
	TestNotNull(TEXT("Pending anomaly save test should expose quantum camera."), QuantumCamera);
	TestNotNull(TEXT("Pending anomaly save test should expose inventory."), Inventory);
	TestNotNull(TEXT("Pending anomaly save test should expose note recorder."), NoteRecorder);
	TestNotNull(TEXT("Pending anomaly save test should expose event bus."), EventBus);
	TestNotNull(TEXT("Pending anomaly save test should expose save subsystem."), SaveSubsystem);
	if (!QuantumCamera || !Inventory || !NoteRecorder || !EventBus || !SaveSubsystem)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	const FName PendingAnomalyId(TEXT("Evidence.Anomaly01"));
	TestTrue(TEXT("Bodycam prerequisite should record before pending anomaly snapshot."), GameMode->TryAcquireBodycam(TEXT("Evidence.Bodycam"), true));
	TestTrue(TEXT("Bodycam evidence should record before pending anomaly snapshot."), Inventory->AddCollectedEvidenceId(TEXT("Evidence.Bodycam")));
	TestTrue(TEXT("First note prerequisite should record before pending anomaly snapshot."), GameMode->TryCollectFirstNote(TEXT("Note.Intro")));
	TestTrue(TEXT("Intro note should record before pending anomaly snapshot."), NoteRecorder->AddRecordedNoteId(TEXT("Note.Intro")));
	TestTrue(TEXT("Pending anomaly candidate should register before snapshot."), GameMode->BeginFirstAnomalyCandidate(PendingAnomalyId));
	TestEqual(TEXT("Pending anomaly candidate source should be live before snapshot."), GameMode->GetPendingFirstAnomalySourceId(), PendingAnomalyId);

	UHorrorSaveGame* SaveGame = SaveSubsystem->CreateCheckpointSnapshot(World, TEXT("Checkpoint.PendingAnomaly"));
	TestNotNull(TEXT("Pending anomaly snapshot should be created."), SaveGame);
	if (!SaveGame)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}
	TestEqual(TEXT("Pending anomaly source should be written into the snapshot."), SaveGame->PendingFirstAnomalySourceId, PendingAnomalyId);

	GameMode->ImportFoundFootageSaveState(FHorrorFoundFootageSaveState());
	TestFalse(TEXT("Pending anomaly candidate should clear before restore."), GameMode->HasPendingFirstAnomalyCandidate());
	EventBus->ResetForTests();
	const int32 HistoryCountBeforeRestore = EventBus->GetHistory().Num();

	TestTrue(TEXT("Pending anomaly snapshot should apply."), SaveSubsystem->ApplyCheckpointSnapshot(World, SaveGame));
	TestTrue(TEXT("Pending anomaly candidate should restore from snapshot."), GameMode->HasPendingFirstAnomalyCandidate());
	TestEqual(TEXT("Pending anomaly candidate source should restore from snapshot."), GameMode->GetPendingFirstAnomalySourceId(), PendingAnomalyId);
	TestEqual(TEXT("Pending anomaly restore should not replay objective event history."), EventBus->GetHistory().Num(), HistoryCountBeforeRestore);
	TestTrue(TEXT("Recording should succeed after pending anomaly restore."), QuantumCamera->StartRecording());
	TestTrue(TEXT("Restored pending anomaly should complete when recording."), GameMode->TryRecordFirstAnomaly(GameMode->IsLeadPlayerRecording()));
	TestFalse(TEXT("Restored pending anomaly should clear after recording."), GameMode->HasPendingFirstAnomalyCandidate());
	TestTrue(TEXT("Restored pending anomaly should record objective progress."), GameMode->HasRecordedFirstAnomaly());

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorSaveSubsystemApplySnapshotDoesNotReplayEventsTest,
	"HorrorProject.Save.Subsystem.ApplySnapshotDoesNotReplayEvents",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorSaveSubsystemApplySnapshotDoesNotReplayEventsTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	UWorld* World = nullptr;
	AHorrorGameModeBase* GameMode = CreateSaveTestGameMode(*this, TestWorld, World);
	if (!GameMode || !World)
	{
		return false;
	}

	AHorrorPlayerCharacter* PlayerCharacter = SpawnSaveTestPlayer(*this, World);
	if (!PlayerCharacter)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	UInventoryComponent* Inventory = PlayerCharacter->GetInventoryComponent();
	UNoteRecorderComponent* NoteRecorder = PlayerCharacter->GetNoteRecorderComponent();
	UInventoryEvidenceDelegateProbe* EvidenceDelegateProbe = NewObject<UInventoryEvidenceDelegateProbe>();
	UNoteRecorderDelegateProbe* NoteDelegateProbe = NewObject<UNoteRecorderDelegateProbe>();
	UHorrorEventBusSubsystem* EventBus = World->GetSubsystem<UHorrorEventBusSubsystem>();
	UHorrorSaveSubsystem* SaveSubsystem = World->GetGameInstance()
		? World->GetGameInstance()->GetSubsystem<UHorrorSaveSubsystem>()
		: nullptr;
	TestNotNull(TEXT("No-replay save test should expose inventory."), Inventory);
	TestNotNull(TEXT("No-replay save test should expose note recorder."), NoteRecorder);
	TestNotNull(TEXT("No-replay save test should expose event bus."), EventBus);
	TestNotNull(TEXT("No-replay save test should expose save subsystem."), SaveSubsystem);
	if (!Inventory || !NoteRecorder || !EventBus || !SaveSubsystem)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	Inventory->OnEvidenceCollected.AddDynamic(EvidenceDelegateProbe, &UInventoryEvidenceDelegateProbe::HandleEvidenceCollected);
	NoteRecorder->OnNoteRecorded.AddDynamic(NoteDelegateProbe, &UNoteRecorderDelegateProbe::HandleNoteRecorded);
	EventBus->ResetForTests();
	int32 BroadcastCount = 0;
	const FDelegateHandle DelegateHandle = EventBus->GetOnEventPublishedNative().AddLambda(
		[&BroadcastCount](const FHorrorEventMessage&)
		{
			++BroadcastCount;
		});
	TestTrue(TEXT("Sentinel event should publish before checkpoint restore."), EventBus->Publish(HorrorFoundFootageTags::BodycamAcquiredEvent(), TEXT("Sentinel.Bodycam"), HorrorFoundFootageTags::BodycamAcquiredState(), nullptr));
	const int32 HistoryCountBeforeRestore = EventBus->GetHistory().Num();
	const int32 BroadcastCountBeforeRestore = BroadcastCount;

	FHorrorFoundFootageContract Contract;
	Contract.RecordEvent(HorrorFoundFootageTags::BodycamAcquiredEvent(), TEXT("Evidence.Bodycam"));
	Contract.RecordEvent(HorrorFoundFootageTags::FirstNoteCollectedEvent(), TEXT("Note.Intro"));
	Contract.RecordEvent(HorrorFoundFootageTags::FirstAnomalyRecordedEvent(), TEXT("Evidence.Anomaly01"));
	Contract.RecordEvent(HorrorFoundFootageTags::ArchiveReviewedEvent(), TEXT("Evidence.ArchiveReview"));
	const FHorrorFoundFootageSaveState FoundFootageSaveState = Contract.ExportSaveState();

	UHorrorSaveGame* SaveGame = NewObject<UHorrorSaveGame>();
	SaveGame->SaveVersion = 1;
	SaveGame->CheckpointId = TEXT("Checkpoint.NoReplay");
	SaveGame->RecordedObjectiveEvents = FoundFootageSaveState.RecordedObjectiveEvents;
	SaveGame->CompletedObjectiveStates = FoundFootageSaveState.CompletedObjectiveStates;
	SaveGame->CollectedEvidenceIds = { FName(TEXT("Evidence.Bodycam")), FName(TEXT("Evidence.Anomaly01")) };
	SaveGame->RecordedNoteIds = { FName(TEXT("Note.Intro")) };

	TestTrue(TEXT("Checkpoint restore should apply imported state."), SaveSubsystem->ApplyCheckpointSnapshot(World, SaveGame));
	TestEqual(TEXT("Checkpoint restore should not replay objective event history."), EventBus->GetHistory().Num(), HistoryCountBeforeRestore);
	TestEqual(TEXT("Checkpoint restore should not broadcast objective events."), BroadcastCount, BroadcastCountBeforeRestore);
	TestEqual(TEXT("Checkpoint restore should not broadcast evidence collection delegates."), EvidenceDelegateProbe->BroadcastCount, 0);
	TestEqual(TEXT("Checkpoint restore should not broadcast note recorder delegates."), NoteDelegateProbe->BroadcastCount, 0);
	TestTrue(TEXT("Checkpoint restore should import bodycam objective state."), GameMode->HasBodycamAcquired());
	TestTrue(TEXT("Checkpoint restore should import archive objective state."), GameMode->HasReviewedArchive());
	TestTrue(TEXT("Checkpoint restore should import bodycam evidence."), Inventory->HasCollectedEvidenceId(TEXT("Evidence.Bodycam")));
	TestTrue(TEXT("Checkpoint restore should import anomaly evidence."), Inventory->HasCollectedEvidenceId(TEXT("Evidence.Anomaly01")));
	TestTrue(TEXT("Checkpoint restore should import intro note."), NoteRecorder->HasRecordedNoteId(TEXT("Note.Intro")));

	EventBus->GetOnEventPublishedNative().Remove(DelegateHandle);
	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorSaveSubsystemSanitizesCrossStreamObjectiveStateTest,
	"HorrorProject.Save.Subsystem.SanitizesCrossStreamObjectiveState",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorSaveSubsystemSanitizesCrossStreamObjectiveStateTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	UWorld* World = nullptr;
	AHorrorGameModeBase* GameMode = CreateSaveTestGameMode(*this, TestWorld, World);
	if (!GameMode || !World)
	{
		return false;
	}

	AHorrorPlayerCharacter* PlayerCharacter = SpawnSaveTestPlayer(*this, World);
	if (!PlayerCharacter)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	UInventoryComponent* Inventory = PlayerCharacter->GetInventoryComponent();
	UNoteRecorderComponent* NoteRecorder = PlayerCharacter->GetNoteRecorderComponent();
	UHorrorSaveSubsystem* SaveSubsystem = World->GetGameInstance()
		? World->GetGameInstance()->GetSubsystem<UHorrorSaveSubsystem>()
		: nullptr;
	TestNotNull(TEXT("Cross-stream save test should expose inventory."), Inventory);
	TestNotNull(TEXT("Cross-stream save test should expose note recorder."), NoteRecorder);
	TestNotNull(TEXT("Cross-stream save test should expose save subsystem."), SaveSubsystem);
	if (!Inventory || !NoteRecorder || !SaveSubsystem)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	FHorrorFoundFootageContract Contract;
	Contract.RecordEvent(HorrorFoundFootageTags::BodycamAcquiredEvent(), TEXT("Evidence.Bodycam"));
	Contract.RecordEvent(HorrorFoundFootageTags::FirstNoteCollectedEvent(), TEXT("Note.Intro"));
	Contract.RecordEvent(HorrorFoundFootageTags::FirstAnomalyRecordedEvent(), TEXT("Evidence.Anomaly01"));
	Contract.RecordEvent(HorrorFoundFootageTags::ArchiveReviewedEvent(), TEXT("Evidence.ArchiveReview"));
	const FHorrorFoundFootageSaveState FoundFootageSaveState = Contract.ExportSaveState();

	UHorrorSaveGame* SaveGame = NewObject<UHorrorSaveGame>();
	SaveGame->SaveVersion = 1;
	SaveGame->CheckpointId = TEXT("Checkpoint.CrossStreamCorrupt");
	SaveGame->RecordedObjectiveEvents = FoundFootageSaveState.RecordedObjectiveEvents;
	SaveGame->CompletedObjectiveStates = FoundFootageSaveState.CompletedObjectiveStates;
	SaveGame->CollectedEvidenceIds = { FName(TEXT("Evidence.Bodycam")) };
	SaveGame->RecordedNoteIds = { FName(TEXT("Note.Intro")) };

	TestTrue(TEXT("Corrupt cross-stream snapshot should still apply sanitized state."), SaveSubsystem->ApplyCheckpointSnapshot(World, SaveGame));
	TestTrue(TEXT("Bodycam objective can restore when its evidence exists."), GameMode->HasBodycamAcquired());
	TestTrue(TEXT("First note objective can restore when its note exists."), GameMode->HasCollectedFirstNote());
	TestFalse(TEXT("Missing anomaly evidence should drop anomaly objective progress."), GameMode->HasRecordedFirstAnomaly());
	TestFalse(TEXT("Missing anomaly evidence should drop archive objective progress."), GameMode->HasReviewedArchive());
	TestFalse(TEXT("Missing anomaly evidence should keep exit locked."), GameMode->IsExitUnlocked());
	TestTrue(TEXT("Bodycam evidence should still import."), Inventory->HasCollectedEvidenceId(TEXT("Evidence.Bodycam")));
	TestFalse(TEXT("Missing anomaly evidence should remain absent."), Inventory->HasCollectedEvidenceId(TEXT("Evidence.Anomaly01")));
	TestTrue(TEXT("Intro note should still import."), NoteRecorder->HasRecordedNoteId(TEXT("Note.Intro")));

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

#endif

#endif // WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR
