// Copyright Epic Games, Inc. All Rights Reserved.

#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR

#include "Engine/World.h"
#include "AI/HorrorThreatCharacter.h"
#include "Game/HorrorEncounterDirector.h"
#include "Game/HorrorFoundFootageContract.h"
#include "Game/HorrorGameModeBase.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/WorldSettings.h"
#include "Interaction/DoorInteractable.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/AutomationTest.h"
#include "Player/HorrorPlayerCharacter.h"
#include "Player/HorrorPlayerController.h"
#include "Player/Components/InventoryComponent.h"
#include "Player/Components/NoteRecorderComponent.h"
#include "Save/HorrorSaveGame.h"
#include "Save/HorrorSaveSubsystem.h"
#include "Tests/AutomationCommon.h"
#include "UI/Day1SliceHUD.h"
#include "UObject/UnrealType.h"

namespace
{
	const FString Day1AutosaveSlotName(TEXT("SM13_Day1_Autosave"));
	constexpr int32 Day1AutosaveUserIndex = 0;

	AHorrorGameModeBase* CreateDay1AutosaveTestGameMode(FAutomationTestBase& Test, FTestWorldWrapper& TestWorld, UWorld*& OutWorld)
	{
		OutWorld = nullptr;
		Test.TestTrue(TEXT("Transient game world should be created for Day 1 autosave coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
		OutWorld = TestWorld.GetTestWorld();
		if (!OutWorld)
		{
			return nullptr;
		}

		OutWorld->GetWorldSettings()->DefaultGameMode = AHorrorGameModeBase::StaticClass();
		Test.TestTrue(TEXT("Transient world should create the Day 1 autosave test game mode."), OutWorld->SetGameMode(FURL()));

		AHorrorGameModeBase* GameMode = OutWorld->GetAuthGameMode<AHorrorGameModeBase>();
		Test.TestNotNull(TEXT("Transient world should expose the Day 1 autosave test game mode."), GameMode);
		if (!GameMode)
		{
			TestWorld.DestroyTestWorld(false);
		}

		return GameMode;
	}

	AHorrorPlayerCharacter* SpawnDay1AutosaveTestPlayer(FAutomationTestBase& Test, UWorld* World)
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

	void SetInteractableIdForDay1AutosaveTest(AActor* Actor, FName InteractableId)
	{
		if (!Actor)
		{
			return;
		}

		if (FNameProperty* InteractableIdProperty = FindFProperty<FNameProperty>(Actor->GetClass(), TEXT("InteractableId")))
		{
			InteractableIdProperty->SetPropertyValue_InContainer(Actor, InteractableId);
		}
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorSaveSubsystemAutosavesBodycamMilestoneTest,
	"HorrorProject.Save.Subsystem.AutosavesBodycamMilestone",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorSaveSubsystemAutosavesBodycamMilestoneTest::RunTest(const FString& Parameters)
{
	UGameplayStatics::DeleteGameInSlot(Day1AutosaveSlotName, Day1AutosaveUserIndex);

	FTestWorldWrapper TestWorld;
	UWorld* World = nullptr;
	AHorrorGameModeBase* GameMode = CreateDay1AutosaveTestGameMode(*this, TestWorld, World);
	if (!GameMode || !World)
	{
		UGameplayStatics::DeleteGameInSlot(Day1AutosaveSlotName, Day1AutosaveUserIndex);
		return false;
	}

	AHorrorPlayerCharacter* PlayerCharacter = SpawnDay1AutosaveTestPlayer(*this, World);
	if (!PlayerCharacter)
	{
		TestWorld.DestroyTestWorld(false);
		UGameplayStatics::DeleteGameInSlot(Day1AutosaveSlotName, Day1AutosaveUserIndex);
		return false;
	}

	UHorrorSaveSubsystem* SaveSubsystem = World->GetGameInstance()
		? World->GetGameInstance()->GetSubsystem<UHorrorSaveSubsystem>()
		: nullptr;
	TestNotNull(TEXT("Bodycam autosave test should expose save subsystem."), SaveSubsystem);
	if (!SaveSubsystem)
	{
		TestWorld.DestroyTestWorld(false);
		UGameplayStatics::DeleteGameInSlot(Day1AutosaveSlotName, Day1AutosaveUserIndex);
		return false;
	}

	TestTrue(TEXT("Bodycam acquisition should record and trigger the Day 1 autosave checkpoint."), GameMode->TryAcquireBodycam(TEXT("Evidence.Bodycam"), true));
	GameMode->ImportFoundFootageSaveState(FHorrorFoundFootageSaveState());
	TestFalse(TEXT("Bodycam state should be cleared before loading the autosave."), GameMode->HasBodycamAcquired());

	SaveSubsystem->ClearCachedSaveOnly();
	TestTrue(TEXT("Bodycam autosave should load through the existing checkpoint API."), GameMode->LoadDay1Checkpoint());
	TestTrue(TEXT("Loaded bodycam autosave should restore bodycam objective progress."), GameMode->HasBodycamAcquired());

	UGameplayStatics::DeleteGameInSlot(Day1AutosaveSlotName, Day1AutosaveUserIndex);
	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorSaveSubsystemRestoresDay1CompletionStateTest,
	"HorrorProject.Save.Subsystem.RestoresDay1CompletionState",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorSaveSubsystemRestoresDay1CompletionStateTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	UWorld* World = nullptr;
	AHorrorGameModeBase* GameMode = CreateDay1AutosaveTestGameMode(*this, TestWorld, World);
	if (!GameMode || !World)
	{
		return false;
	}

	AHorrorPlayerCharacter* PlayerCharacter = SpawnDay1AutosaveTestPlayer(*this, World);
	if (!PlayerCharacter)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	UHorrorSaveSubsystem* SaveSubsystem = World->GetGameInstance()
		? World->GetGameInstance()->GetSubsystem<UHorrorSaveSubsystem>()
		: nullptr;
	TestNotNull(TEXT("Day1 completion restore test should expose save subsystem."), SaveSubsystem);
	if (!SaveSubsystem)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	TestTrue(TEXT("Bodycam acquisition should record before completion snapshot."), GameMode->TryAcquireBodycam(TEXT("BodycamPickup"), true));
	TestTrue(TEXT("First note should record before completion snapshot."), GameMode->TryCollectFirstNote(TEXT("NoteIntro")));
	TestTrue(TEXT("First anomaly candidate should register before completion snapshot."), GameMode->BeginFirstAnomalyCandidate(TEXT("Anomaly01")));
	TestTrue(TEXT("First anomaly should record before completion snapshot."), GameMode->TryRecordFirstAnomaly(true));
	TestTrue(TEXT("Archive review should unlock the exit before completion snapshot."), GameMode->TryReviewArchive(TEXT("ArchiveTerminal")));
	TestTrue(TEXT("Exit use should complete Day1 before completion snapshot."), GameMode->TryCompleteDay1(TEXT("Exit.ServiceDoor")));

	UHorrorSaveGame* Snapshot = SaveSubsystem->CreateCheckpointSnapshot(World, TEXT("Checkpoint.Day1.Complete"));
	TestNotNull(TEXT("Day1 completion restore test should create a checkpoint snapshot."), Snapshot);
	if (!Snapshot)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	GameMode->ImportDay1CompleteState(false);
	TestFalse(TEXT("Day1 completion state should be clear before checkpoint restore."), GameMode->IsDay1Complete());
	TestTrue(TEXT("Completion snapshot should apply."), SaveSubsystem->ApplyCheckpointSnapshot(World, Snapshot));
	TestTrue(TEXT("Completion snapshot should restore Day1 completion state."), GameMode->IsDay1Complete());

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorSaveSubsystemAutosavesDay1CompletionTest,
	"HorrorProject.Save.Subsystem.AutosavesDay1Completion",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorSaveSubsystemAutosavesDay1CompletionTest::RunTest(const FString& Parameters)
{
	UGameplayStatics::DeleteGameInSlot(Day1AutosaveSlotName, Day1AutosaveUserIndex);

	FTestWorldWrapper TestWorld;
	UWorld* World = nullptr;
	AHorrorGameModeBase* GameMode = CreateDay1AutosaveTestGameMode(*this, TestWorld, World);
	if (!GameMode || !World)
	{
		UGameplayStatics::DeleteGameInSlot(Day1AutosaveSlotName, Day1AutosaveUserIndex);
		return false;
	}

	AHorrorPlayerCharacter* PlayerCharacter = SpawnDay1AutosaveTestPlayer(*this, World);
	if (!PlayerCharacter)
	{
		TestWorld.DestroyTestWorld(false);
		UGameplayStatics::DeleteGameInSlot(Day1AutosaveSlotName, Day1AutosaveUserIndex);
		return false;
	}

	UHorrorSaveSubsystem* SaveSubsystem = World->GetGameInstance()
		? World->GetGameInstance()->GetSubsystem<UHorrorSaveSubsystem>()
		: nullptr;
	TestNotNull(TEXT("Day1 completion autosave test should expose save subsystem."), SaveSubsystem);
	if (!SaveSubsystem)
	{
		TestWorld.DestroyTestWorld(false);
		UGameplayStatics::DeleteGameInSlot(Day1AutosaveSlotName, Day1AutosaveUserIndex);
		return false;
	}

	TestTrue(TEXT("Bodycam acquisition should record before completion autosave."), GameMode->TryAcquireBodycam(TEXT("BodycamPickup"), true));
	TestTrue(TEXT("First note should record before completion autosave."), GameMode->TryCollectFirstNote(TEXT("NoteIntro")));
	TestTrue(TEXT("First anomaly candidate should register before completion autosave."), GameMode->BeginFirstAnomalyCandidate(TEXT("Anomaly01")));
	TestTrue(TEXT("First anomaly should record before completion autosave."), GameMode->TryRecordFirstAnomaly(true));
	TestTrue(TEXT("Archive review should unlock the exit before completion autosave."), GameMode->TryReviewArchive(TEXT("ArchiveTerminal")));
	AHorrorEncounterDirector* Director = GameMode->GetRuntimeEncounterDirector();
	TestNotNull(TEXT("Day1 completion autosave test should expose a runtime encounter director."), Director);
	if (!Director)
	{
		TestWorld.DestroyTestWorld(false);
		UGameplayStatics::DeleteGameInSlot(Day1AutosaveSlotName, Day1AutosaveUserIndex);
		return false;
	}

	Director->RevealRadius = 0.0f;
	TestTrue(TEXT("Completion autosave should handle an active revealed encounter."), Director->TriggerReveal(PlayerCharacter));
	TestEqual(TEXT("Encounter should be revealed before the final checkpoint is written."), Director->GetEncounterPhase(), EHorrorEncounterPhase::Revealed);
	TestTrue(TEXT("Exit use should complete Day1 and write the final checkpoint."), GameMode->TryCompleteDay1(TEXT("Exit.ServiceDoor")));

	GameMode->ImportDay1CompleteState(false);
	TestTrue(TEXT("Revealed encounter should resolve before loading the final autosave."), Director->ResolveEncounter());
	TestFalse(TEXT("Completion state should be clear before loading the final autosave."), GameMode->IsDay1Complete());

	SaveSubsystem->ClearCachedSaveOnly();
	TestTrue(TEXT("Day1 completion autosave should load through the existing checkpoint API."), GameMode->LoadDay1Checkpoint());
	TestTrue(TEXT("Loaded final autosave should restore Day1 completion."), GameMode->IsDay1Complete());
	TestEqual(TEXT("Loaded final autosave should not restart a completed Day1 chase."), Director->GetEncounterPhase(), EHorrorEncounterPhase::Resolved);

	UGameplayStatics::DeleteGameInSlot(Day1AutosaveSlotName, Day1AutosaveUserIndex);
	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorSaveSubsystemRestoresCompletedDay1InputLockTest,
	"HorrorProject.Save.Subsystem.RestoresCompletedDay1InputLock",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorSaveSubsystemRestoresCompletedDay1InputLockTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	UWorld* World = nullptr;
	AHorrorGameModeBase* GameMode = CreateDay1AutosaveTestGameMode(*this, TestWorld, World);
	if (!GameMode || !World)
	{
		return false;
	}

	AHorrorPlayerController* PlayerController = World->SpawnActor<AHorrorPlayerController>();
	ADay1SliceHUD* HUD = World->SpawnActor<ADay1SliceHUD>();
	AHorrorPlayerCharacter* PlayerCharacter = World->SpawnActor<AHorrorPlayerCharacter>();
	TestNotNull(TEXT("Completed restore test should spawn a horror player controller."), PlayerController);
	TestNotNull(TEXT("Completed restore test should spawn a Day1 HUD."), HUD);
	TestNotNull(TEXT("Completed restore test should spawn a horror player character."), PlayerCharacter);
	if (!PlayerController || !HUD || !PlayerCharacter)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	PlayerController->PlayerState = World->SpawnActor<APlayerState>();
	World->AddController(PlayerController);
	PlayerController->MyHUD = HUD;
	PlayerController->Possess(PlayerCharacter);

	UHorrorSaveSubsystem* SaveSubsystem = World->GetGameInstance()
		? World->GetGameInstance()->GetSubsystem<UHorrorSaveSubsystem>()
		: nullptr;
	TestNotNull(TEXT("Completed restore test should expose save subsystem."), SaveSubsystem);
	if (!SaveSubsystem)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	TestTrue(TEXT("Bodycam acquisition should record before completed restore snapshot."), GameMode->TryAcquireBodycam(TEXT("BodycamPickup"), true));
	TestTrue(TEXT("First note should record before completed restore snapshot."), GameMode->TryCollectFirstNote(TEXT("NoteIntro")));
	TestTrue(TEXT("First anomaly candidate should register before completed restore snapshot."), GameMode->BeginFirstAnomalyCandidate(TEXT("Anomaly01")));
	TestTrue(TEXT("First anomaly should record before completed restore snapshot."), GameMode->TryRecordFirstAnomaly(true));
	TestTrue(TEXT("Archive review should unlock exit before completed restore snapshot."), GameMode->TryReviewArchive(TEXT("ArchiveTerminal")));
	TestTrue(TEXT("Exit use should complete Day1 before completed restore snapshot."), GameMode->TryCompleteDay1(TEXT("Exit.ServiceDoor")));

	UHorrorSaveGame* Snapshot = SaveSubsystem->CreateCheckpointSnapshot(World, TEXT("Checkpoint.Day1.Complete"));
	TestNotNull(TEXT("Completed restore test should create a checkpoint snapshot."), Snapshot);
	if (!Snapshot)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	PlayerController->Destroy();
	HUD->Destroy();
	PlayerCharacter->Destroy();

	PlayerController = World->SpawnActor<AHorrorPlayerController>();
	HUD = World->SpawnActor<ADay1SliceHUD>();
	PlayerCharacter = World->SpawnActor<AHorrorPlayerCharacter>();
	TestNotNull(TEXT("Completed restore test should spawn a fresh restore controller."), PlayerController);
	TestNotNull(TEXT("Completed restore test should spawn a fresh restore HUD."), HUD);
	TestNotNull(TEXT("Completed restore test should spawn a fresh restore player character."), PlayerCharacter);
	if (!PlayerController || !HUD || !PlayerCharacter)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	PlayerController->PlayerState = World->SpawnActor<APlayerState>();
	World->AddController(PlayerController);
	PlayerController->MyHUD = HUD;
	PlayerController->Possess(PlayerCharacter);

	GameMode->ImportDay1CompleteState(false);
	HUD->ClearPasswordPrompt();
	TestFalse(TEXT("Controller completion input lock should start clear before checkpoint restore."), PlayerController->IsDay1CompletionInputLockedForTests());
	TestFalse(TEXT("HUD completion overlay should start hidden before checkpoint restore."), HUD->IsDay1CompletionOverlayVisibleForTests());

	TestTrue(TEXT("Completed checkpoint snapshot should apply."), SaveSubsystem->ApplyCheckpointSnapshot(World, Snapshot));
	TestTrue(TEXT("Completed checkpoint snapshot should restore Day1 completion state."), GameMode->IsDay1Complete());
	TestTrue(TEXT("Completed checkpoint restore should relock player movement/look input."), PlayerController->IsDay1CompletionInputLockedForTests());
	TestTrue(TEXT("Completed checkpoint restore should show the Day1 completion overlay."), HUD->IsDay1CompletionOverlayVisibleForTests());
	TestEqual(TEXT("Completed checkpoint restore should use the Day1 completion title."), HUD->GetDay1CompletionTitleForTests().ToString(), FString(TEXT("DAY 1 COMPLETE")));

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorSaveSubsystemRestoresPasswordDoorStateTest,
	"HorrorProject.Save.Subsystem.RestoresPasswordDoorState",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorSaveSubsystemRestoresPasswordDoorStateTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	UWorld* World = nullptr;
	AHorrorGameModeBase* GameMode = CreateDay1AutosaveTestGameMode(*this, TestWorld, World);
	if (!GameMode || !World)
	{
		return false;
	}

	AHorrorPlayerCharacter* PlayerCharacter = SpawnDay1AutosaveTestPlayer(*this, World);
	if (!PlayerCharacter)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	UHorrorSaveSubsystem* SaveSubsystem = World->GetGameInstance()
		? World->GetGameInstance()->GetSubsystem<UHorrorSaveSubsystem>()
		: nullptr;
	TestNotNull(TEXT("Door state restore test should expose save subsystem."), SaveSubsystem);
	if (!SaveSubsystem)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	ADoorInteractable* Door = World->SpawnActor<ADoorInteractable>();
	TestNotNull(TEXT("Door state restore test should spawn a password door."), Door);
	if (!Door)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	SetInteractableIdForDay1AutosaveTest(Door, TEXT("Door.Day1.TestPassword"));
	Door->ConfigurePassword(TEXT("0417"), FText::FromString(TEXT("Test password")));
	Door->DispatchBeginPlay();
	TestTrue(TEXT("Password door should unlock with the correct code before checkpointing."), Door->SubmitPassword(PlayerCharacter, TEXT("0417")));

	for (int32 TickIndex = 0; TickIndex < 60; ++TickIndex)
	{
		Door->Tick(0.05f);
	}

	TestTrue(TEXT("Password door should be open before checkpointing."), Door->IsOpen());
	TestTrue(TEXT("Password door should be unlocked before checkpointing."), Door->IsPasswordUnlocked());

	UHorrorSaveGame* Snapshot = SaveSubsystem->CreateCheckpointSnapshot(World, TEXT("Checkpoint.Day1.PasswordDoor"));
	TestNotNull(TEXT("Door state restore test should create a checkpoint snapshot."), Snapshot);
	if (!Snapshot)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	Door->ConfigurePassword(TEXT("0417"), FText::FromString(TEXT("Test password")));
	TestEqual(TEXT("Reset password door should be locked before checkpoint restore."), Door->GetDoorState(), EDoorState::Locked);
	TestFalse(TEXT("Reset password door should clear password unlock state."), Door->IsPasswordUnlocked());

	TestTrue(TEXT("Checkpoint restore should apply after a password door was saved."), SaveSubsystem->ApplyCheckpointSnapshot(World, Snapshot));
	TestTrue(TEXT("Checkpoint restore should keep the password door unlocked."), Door->IsPasswordUnlocked());
	TestTrue(TEXT("Checkpoint restore should reopen the saved password door."), Door->IsOpen());

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorSaveSubsystemRestoresRevealedEncounterCheckpointTest,
	"HorrorProject.Save.Subsystem.RestoresRevealedEncounterCheckpoint",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorSaveSubsystemRestoresRevealedEncounterCheckpointTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	UWorld* World = nullptr;
	AHorrorGameModeBase* GameMode = CreateDay1AutosaveTestGameMode(*this, TestWorld, World);
	if (!GameMode || !World)
	{
		return false;
	}

	AHorrorPlayerCharacter* PlayerCharacter = SpawnDay1AutosaveTestPlayer(*this, World);
	if (!PlayerCharacter)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	UHorrorSaveSubsystem* SaveSubsystem = World->GetGameInstance()
		? World->GetGameInstance()->GetSubsystem<UHorrorSaveSubsystem>()
		: nullptr;
	TestNotNull(TEXT("Revealed encounter restore test should expose save subsystem."), SaveSubsystem);
	if (!SaveSubsystem)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	TestTrue(TEXT("Bodycam should record before revealed encounter snapshot."), GameMode->TryAcquireBodycam(TEXT("Evidence.Bodycam"), true));
	TestTrue(TEXT("First note should prime the encounter before revealed encounter snapshot."), GameMode->TryCollectFirstNote(TEXT("Note.Intro")));
	AHorrorEncounterDirector* Director = GameMode->GetRuntimeEncounterDirector();
	TestNotNull(TEXT("Revealed encounter restore test should expose a runtime encounter director."), Director);
	if (!Director)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	Director->RevealRadius = 0.0f;
	TestTrue(TEXT("Anomaly candidate should register before revealed encounter snapshot."), GameMode->BeginFirstAnomalyCandidate(TEXT("Evidence.Anomaly01")));
	TestTrue(TEXT("Anomaly should record before revealed encounter snapshot."), GameMode->TryRecordFirstAnomaly(true));
	TestTrue(TEXT("Archive review should unlock the exit before revealed encounter snapshot."), GameMode->TryReviewArchive(TEXT("Archive.Terminal")));
	TestTrue(TEXT("Archive review should unlock the exit before the chase checkpoint."), GameMode->IsExitUnlocked());
	TestTrue(TEXT("Archive-owned reveal should start the chase before checkpointing."), Director->TriggerReveal(PlayerCharacter));
	TestEqual(TEXT("Encounter should be revealed before checkpointing."), Director->GetEncounterPhase(), EHorrorEncounterPhase::Revealed);
	TestTrue(TEXT("Revealed encounter should gate the route before checkpointing."), Director->IsRouteGated());

	UHorrorSaveGame* Snapshot = SaveSubsystem->CreateCheckpointSnapshot(World, TEXT("Checkpoint.Day1.RevealedEncounter"));
	TestNotNull(TEXT("Revealed encounter restore test should create a checkpoint snapshot."), Snapshot);
	if (!Snapshot)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	TestTrue(TEXT("Encounter should resolve after checkpoint so restore has a stale phase to replace."), Director->ResolveEncounter());
	TestEqual(TEXT("Encounter should be resolved before checkpoint restore."), Director->GetEncounterPhase(), EHorrorEncounterPhase::Resolved);

	TestTrue(TEXT("Revealed encounter checkpoint should apply."), SaveSubsystem->ApplyCheckpointSnapshot(World, Snapshot));
	TestTrue(TEXT("Revealed encounter checkpoint should keep the exit unlocked."), GameMode->IsExitUnlocked());
	TestFalse(TEXT("Revealed encounter checkpoint should not mark Day1 complete."), GameMode->IsDay1Complete());
	TestEqual(TEXT("Revealed encounter checkpoint should restore the active chase phase."), Director->GetEncounterPhase(), EHorrorEncounterPhase::Revealed);
	TestTrue(TEXT("Restored revealed encounter should gate the route again."), Director->IsRouteGated());
	TestNotNull(TEXT("Restored revealed encounter should keep a spawned threat."), Director->GetThreatActor());
	if (Director->GetThreatActor())
	{
		TestTrue(TEXT("Restored revealed encounter should reactivate the threat."), Director->GetThreatActor()->IsThreatActive());
	}

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorSaveSubsystemRestoresRecordedMetadataTest,
	"HorrorProject.Save.Subsystem.RestoresRecordedMetadata",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorSaveSubsystemRestoresRecordedMetadataTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	UWorld* World = nullptr;
	AHorrorGameModeBase* GameMode = CreateDay1AutosaveTestGameMode(*this, TestWorld, World);
	if (!GameMode || !World)
	{
		return false;
	}

	AHorrorPlayerCharacter* PlayerCharacter = SpawnDay1AutosaveTestPlayer(*this, World);
	if (!PlayerCharacter)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	UHorrorSaveSubsystem* SaveSubsystem = World->GetGameInstance()
		? World->GetGameInstance()->GetSubsystem<UHorrorSaveSubsystem>()
		: nullptr;
	TestNotNull(TEXT("Metadata restore test should expose save subsystem."), SaveSubsystem);
	if (!SaveSubsystem)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	UInventoryComponent* Inventory = PlayerCharacter->GetInventoryComponent();
	UNoteRecorderComponent* NoteRecorder = PlayerCharacter->GetNoteRecorderComponent();
	TestNotNull(TEXT("Metadata restore test should expose player inventory."), Inventory);
	TestNotNull(TEXT("Metadata restore test should expose player note recorder."), NoteRecorder);
	if (!Inventory || !NoteRecorder)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	FHorrorEvidenceMetadata BodycamMetadata;
	BodycamMetadata.EvidenceId = TEXT("Evidence.Bodycam");
	BodycamMetadata.DisplayName = FText::FromString(TEXT("Recovered Bodycam"));
	BodycamMetadata.Description = FText::FromString(TEXT("Recovered station bodycam with corrupted dive footage."));
	Inventory->RegisterEvidenceMetadata(BodycamMetadata);
	TestTrue(TEXT("Metadata restore test should collect evidence before checkpointing."), Inventory->AddCollectedEvidenceId(BodycamMetadata.EvidenceId));

	FHorrorNoteMetadata IntroNoteMetadata;
	IntroNoteMetadata.NoteId = TEXT("Note.Intro");
	IntroNoteMetadata.Title = FText::FromString(TEXT("Maintenance Codes"));
	IntroNoteMetadata.Body = FText::FromString(TEXT("Door sequence: 0417, 1939, 1831, 1799, 1697."));
	NoteRecorder->RegisterNoteMetadata(IntroNoteMetadata);
	TestTrue(TEXT("Metadata restore test should record note before checkpointing."), NoteRecorder->AddRecordedNoteId(IntroNoteMetadata.NoteId));

	UHorrorSaveGame* Snapshot = SaveSubsystem->CreateCheckpointSnapshot(World, TEXT("Checkpoint.Day1.Metadata"));
	TestNotNull(TEXT("Metadata restore test should create a checkpoint snapshot."), Snapshot);
	if (!Snapshot)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}
	TestEqual(TEXT("Checkpoint snapshot should persist collected evidence metadata."), Snapshot->CollectedEvidenceMetadata.Num(), 1);
	TestEqual(TEXT("Checkpoint snapshot should persist recorded note metadata."), Snapshot->RecordedNoteMetadata.Num(), 1);

	if (AController* OldController = PlayerCharacter->GetController())
	{
		OldController->Destroy();
	}
	PlayerCharacter->Destroy();

	PlayerCharacter = SpawnDay1AutosaveTestPlayer(*this, World);
	TestNotNull(TEXT("Metadata restore test should spawn a fresh player for restore."), PlayerCharacter);
	if (!PlayerCharacter)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	Inventory = PlayerCharacter->GetInventoryComponent();
	NoteRecorder = PlayerCharacter->GetNoteRecorderComponent();
	TestNotNull(TEXT("Fresh restore player should expose inventory."), Inventory);
	TestNotNull(TEXT("Fresh restore player should expose note recorder."), NoteRecorder);
	if (!Inventory || !NoteRecorder)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	FHorrorEvidenceMetadata RestoredEvidenceMetadata;
	FHorrorNoteMetadata RestoredNoteMetadata;
	TestFalse(TEXT("Fresh player should not already know saved evidence metadata."), Inventory->GetEvidenceMetadata(BodycamMetadata.EvidenceId, RestoredEvidenceMetadata));
	TestFalse(TEXT("Fresh player should not already know saved note metadata."), NoteRecorder->GetNoteMetadata(IntroNoteMetadata.NoteId, RestoredNoteMetadata));

	TestTrue(TEXT("Metadata checkpoint snapshot should apply."), SaveSubsystem->ApplyCheckpointSnapshot(World, Snapshot));
	TestTrue(TEXT("Restored checkpoint should keep collected evidence id."), Inventory->HasCollectedEvidenceId(BodycamMetadata.EvidenceId));
	TestTrue(TEXT("Restored checkpoint should keep recorded note id."), NoteRecorder->HasRecordedNoteId(IntroNoteMetadata.NoteId));
	TestTrue(TEXT("Restored checkpoint should restore evidence metadata."), Inventory->GetEvidenceMetadata(BodycamMetadata.EvidenceId, RestoredEvidenceMetadata));
	TestEqual(TEXT("Restored evidence metadata should keep display name."), RestoredEvidenceMetadata.DisplayName.ToString(), FString(TEXT("Recovered Bodycam")));
	TestTrue(TEXT("Restored checkpoint should restore note metadata."), NoteRecorder->GetNoteMetadata(IntroNoteMetadata.NoteId, RestoredNoteMetadata));
	TestEqual(TEXT("Restored note metadata should keep title."), RestoredNoteMetadata.Title.ToString(), FString(TEXT("Maintenance Codes")));
	TestEqual(TEXT("Restored note metadata should keep body."), RestoredNoteMetadata.Body.ToString(), FString(TEXT("Door sequence: 0417, 1939, 1831, 1799, 1697.")));

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

#endif
