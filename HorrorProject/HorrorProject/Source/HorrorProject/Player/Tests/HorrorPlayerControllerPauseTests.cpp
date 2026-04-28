// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/HorrorPlayerController.h"

#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR

#include "Game/HorrorEventBusSubsystem.h"
#include "Game/DeepWaterStationRouteKit.h"
#include "Game/HorrorFoundFootageContract.h"
#include "Game/HorrorGameModeBase.h"
#include "Game/HorrorEncounterDirector.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/WorldSettings.h"
#include "InputCoreTypes.h"
#include "Interaction/DoorInteractable.h"
#include "Misc/AutomationTest.h"
#include "Player/HorrorPlayerCharacter.h"
#include "Player/Components/NoteRecorderComponent.h"
#include "Tests/AutomationCommon.h"
#include "UI/Day1SliceHUD.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorPlayerControllerNativeInputContextsTest,
	"HorrorProject.Player.Controller.NativeInputContexts",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorPlayerControllerNativeInputContextsTest::RunTest(const FString& Parameters)
{
	const AHorrorPlayerController* NativeDefaults = GetDefault<AHorrorPlayerController>();
	TestNotNull(TEXT("Native player controller defaults should be available."), NativeDefaults);
	if (!NativeDefaults)
	{
		return false;
	}

	TestTrue(
		TEXT("Native Day1 controller should load the gameplay input mapping without relying on the old Blueprint controller."),
		NativeDefaults->GetDefaultMappingContextCountForTests() > 0);
	TestTrue(
		TEXT("Native Day1 controller should load mouse look input without relying on the old Blueprint controller."),
		NativeDefaults->GetMobileExcludedMappingContextCountForTests() > 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorPlayerControllerDay1PauseInputTest,
	"HorrorProject.Player.Controller.Day1PauseInput",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorPlayerControllerDay1PauseInputTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for Day1 pause input coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	AHorrorPlayerController* PlayerController = World->SpawnActor<AHorrorPlayerController>();
	TestNotNull(TEXT("Day1 pause input test should spawn a horror player controller."), PlayerController);
	ADay1SliceHUD* HUD = World->SpawnActor<ADay1SliceHUD>();
	TestNotNull(TEXT("Day1 pause input test should attach the native Day1 HUD."), HUD);
	ADoorInteractable* Door = World->SpawnActor<ADoorInteractable>();
	TestNotNull(TEXT("Day1 pause input test should spawn a password door."), Door);
	if (!PlayerController || !HUD || !Door)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}
	PlayerController->MyHUD = HUD;

	TestFalse(TEXT("Pause menu should start closed."), PlayerController->IsDay1PauseMenuOpenForTests());
	TestTrue(TEXT("Escape should open the Day1 pause menu."), PlayerController->HandleInputKeyForTests(EKeys::Escape));
	TestTrue(TEXT("Pause menu should be open after Escape."), PlayerController->IsDay1PauseMenuOpenForTests());
	TestEqual(TEXT("Pause menu should start on Resume."), PlayerController->GetDay1PauseMenuSelectionForTests(), EDay1PauseMenuSelection::Resume);

	TestTrue(TEXT("Down should move pause selection."), PlayerController->HandleInputKeyForTests(EKeys::Down));
	TestEqual(TEXT("Pause menu should move to mouse sensitivity."), PlayerController->GetDay1PauseMenuSelectionForTests(), EDay1PauseMenuSelection::MouseSensitivity);

	TestTrue(TEXT("Escape should close the Day1 pause menu."), PlayerController->HandleInputKeyForTests(EKeys::Escape));
	TestFalse(TEXT("Pause menu should close after Escape while paused."), PlayerController->IsDay1PauseMenuOpenForTests());

	UHorrorEventBusSubsystem* EventBus = World->GetSubsystem<UHorrorEventBusSubsystem>();
	TestNotNull(TEXT("Day1 pause input test should expose the event bus."), EventBus);
	if (EventBus)
	{
		TestTrue(TEXT("J should open the notes journal before completion."), PlayerController->HandleInputKeyForTests(EKeys::J));
		TestTrue(TEXT("Notes journal should be open before completion lock."), PlayerController->IsDay1NotesJournalOpenForTests());

		Door->ConfigurePassword(TEXT("2468"), FText::FromString(TEXT("完成锁定测试密码。")));
		PlayerController->BeginDoorPasswordEntry(Door);
		TestTrue(TEXT("Password entry should be active before completion lock."), PlayerController->IsAwaitingDoorPassword());
		TestTrue(TEXT("HUD should show password entry before completion lock."), HUD->IsPasswordPromptVisibleForTests());

		TestFalse(TEXT("Day1 completion input lock should start disabled."), PlayerController->IsDay1CompletionInputLockedForTests());
		PlayerController->BindObjectiveEventBusForTests();
		EventBus->Publish(HorrorDay1Tags::Day1CompletedEvent(), TEXT("Exit.Gate"), HorrorDay1Tags::Day1CompletedState(), PlayerController);
		TestTrue(TEXT("Day1 completion event should lock movement/look input."), PlayerController->IsDay1CompletionInputLockedForTests());
		TestFalse(TEXT("Day1 completion should close the notes journal."), PlayerController->IsDay1NotesJournalOpenForTests());
		TestFalse(TEXT("Day1 completion should cancel pending password entry."), PlayerController->IsAwaitingDoorPassword());
		TestFalse(TEXT("Day1 completion should clear the password HUD prompt."), HUD->IsPasswordPromptVisibleForTests());

		TestTrue(TEXT("Escape after Day1 completion should be consumed."), PlayerController->HandleInputKeyForTests(EKeys::Escape));
		TestFalse(TEXT("Escape after Day1 completion should not reopen pause."), PlayerController->IsDay1PauseMenuOpenForTests());
		TestTrue(TEXT("J after Day1 completion should be consumed."), PlayerController->HandleInputKeyForTests(EKeys::J));
		TestFalse(TEXT("J after Day1 completion should not reopen notes journal."), PlayerController->IsDay1NotesJournalOpenForTests());
		PlayerController->BeginDoorPasswordEntry(Door);
		TestFalse(TEXT("Completion lock should keep password entry inactive after new attempts."), PlayerController->IsAwaitingDoorPassword());
	}

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorPlayerControllerDay1ObjectiveNavigationTest,
	"HorrorProject.Player.Controller.Day1ObjectiveNavigation",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorPlayerControllerDay1ObjectiveNavigationTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for Day1 objective navigation coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	World->GetWorldSettings()->DefaultGameMode = AHorrorGameModeBase::StaticClass();
	TestTrue(TEXT("Transient world should create the Day1 navigation game mode."), World->SetGameMode(FURL()));
	AHorrorGameModeBase* GameMode = World->GetAuthGameMode<AHorrorGameModeBase>();
	TestNotNull(TEXT("Day1 navigation test should expose the game mode."), GameMode);
	if (!GameMode)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	AHorrorPlayerController* PlayerController = World->SpawnActor<AHorrorPlayerController>();
	ADay1SliceHUD* HUD = World->SpawnActor<ADay1SliceHUD>();
	AHorrorPlayerCharacter* PlayerCharacter = World->SpawnActor<AHorrorPlayerCharacter>(FVector(0.0f, 0.0f, 80.0f), FRotator::ZeroRotator);
	TestNotNull(TEXT("Day1 navigation test should spawn a horror player controller."), PlayerController);
	TestNotNull(TEXT("Day1 navigation test should attach the native Day1 HUD."), HUD);
	TestNotNull(TEXT("Day1 navigation test should spawn a horror player character."), PlayerCharacter);
	if (!PlayerController || !HUD || !PlayerCharacter)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	PlayerController->PlayerState = World->SpawnActor<APlayerState>();
	World->AddController(PlayerController);
	PlayerController->MyHUD = HUD;
	PlayerController->Possess(PlayerCharacter);
	GameMode->DispatchBeginPlay();
	PlayerController->RefreshDay1HUDStateForTests();

	TestFalse(TEXT("Objective navigation should be visible when the route kit has a next target."), HUD->GetObjectiveNavigationForTests().IsEmpty());
	TestTrue(
		TEXT("Objective navigation should include an approximate distance to the next target."),
		HUD->GetObjectiveNavigationForTests().ToString().Contains(TEXT("2 米")));
	TestTrue(
		TEXT("Objective navigation should include a facing-relative direction."),
		HUD->GetObjectiveNavigationForTests().ToString().Contains(TEXT("前方")));

	TestTrue(TEXT("Bodycam acquisition should advance the navigation target."), GameMode->TryAcquireBodycam(TEXT("Evidence.Bodycam"), true));
	PlayerController->RefreshDay1HUDStateForTests();
	TestTrue(
		TEXT("Objective navigation should update after the first objective completes."),
		HUD->GetObjectiveNavigationForTests().ToString().Contains(TEXT("6 米")));

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorPlayerControllerRestampClearsModalHUDStateTest,
	"HorrorProject.Player.Controller.RestampClearsModalHUDState",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorPlayerControllerRestampClearsModalHUDStateTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for checkpoint restamp UI coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	AHorrorPlayerController* PlayerController = World->SpawnActor<AHorrorPlayerController>();
	TestNotNull(TEXT("Checkpoint restamp test should spawn a horror player controller."), PlayerController);
	ADay1SliceHUD* HUD = World->SpawnActor<ADay1SliceHUD>();
	TestNotNull(TEXT("Checkpoint restamp test should attach a Day1 HUD."), HUD);
	ADoorInteractable* Door = World->SpawnActor<ADoorInteractable>();
	TestNotNull(TEXT("Checkpoint restamp test should spawn a password door."), Door);
	if (!PlayerController || !HUD || !Door)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}
	PlayerController->MyHUD = HUD;

	Door->ConfigurePassword(TEXT("1234"), FText::FromString(TEXT("备忘录密码。")));
	PlayerController->BeginDoorPasswordEntry(Door);
	TestTrue(TEXT("Password entry should be active before checkpoint restamp."), PlayerController->IsAwaitingDoorPassword());
	TestTrue(TEXT("Digit input should populate the password buffer before checkpoint restamp."), PlayerController->HandleInputKeyForTests(EKeys::One));
	TestEqual(TEXT("Password buffer should contain typed digits before checkpoint restamp."), PlayerController->GetDoorPasswordBufferForTests(), FString(TEXT("1")));

	PlayerController->RestampCheckpointLoadedUIState();
	TestFalse(TEXT("Checkpoint restamp should cancel pending password entry."), PlayerController->IsAwaitingDoorPassword());
	TestTrue(TEXT("Checkpoint restamp should clear pending password digits."), PlayerController->GetDoorPasswordBufferForTests().IsEmpty());

	TestTrue(TEXT("Escape should open the Day1 pause menu before checkpoint restamp."), PlayerController->HandleInputKeyForTests(EKeys::Escape));
	TestTrue(TEXT("Pause menu should be open before checkpoint restamp."), PlayerController->IsDay1PauseMenuOpenForTests());

	PlayerController->RestampCheckpointLoadedUIState();
	TestFalse(TEXT("Checkpoint restamp should close the Day1 pause menu."), PlayerController->IsDay1PauseMenuOpenForTests());

	if (UHorrorEventBusSubsystem* EventBus = World->GetSubsystem<UHorrorEventBusSubsystem>())
	{
		PlayerController->BindObjectiveEventBusForTests();
		EventBus->Publish(HorrorDay1Tags::Day1CompletedEvent(), TEXT("Exit.Gate"), HorrorDay1Tags::Day1CompletedState(), PlayerController);
		TestTrue(TEXT("Completion event should show the completion overlay before restamp."), HUD->IsDay1CompletionOverlayVisibleForTests());
		TestTrue(TEXT("Completion event should lock input before restamp."), PlayerController->IsDay1CompletionInputLockedForTests());

		PlayerController->RestampCheckpointLoadedUIState();
		TestFalse(TEXT("Non-complete checkpoint restamp should clear completion input lock."), PlayerController->IsDay1CompletionInputLockedForTests());
		TestFalse(TEXT("Non-complete checkpoint restamp should clear the completion overlay."), HUD->IsDay1CompletionOverlayVisibleForTests());
	}

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorPlayerControllerDay1NotesJournalInputTest,
	"HorrorProject.Player.Controller.Day1NotesJournalInput",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorPlayerControllerDay1NotesJournalInputTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for Day1 journal input coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	AHorrorPlayerController* PlayerController = World->SpawnActor<AHorrorPlayerController>();
	ADay1SliceHUD* HUD = World->SpawnActor<ADay1SliceHUD>();
	AHorrorPlayerCharacter* PlayerCharacter = World->SpawnActor<AHorrorPlayerCharacter>();
	TestNotNull(TEXT("Day1 journal input test should spawn a horror player controller."), PlayerController);
	TestNotNull(TEXT("Day1 journal input test should attach the native Day1 HUD."), HUD);
	TestNotNull(TEXT("Day1 journal input test should spawn a horror player character."), PlayerCharacter);
	if (!PlayerController || !HUD || !PlayerCharacter)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	PlayerController->MyHUD = HUD;
	PlayerController->Possess(PlayerCharacter);

	TestFalse(TEXT("Journal should start closed."), PlayerController->IsDay1NotesJournalOpenForTests());
	TestTrue(TEXT("J should open the Day1 notes journal."), PlayerController->HandleInputKeyForTests(EKeys::J));
	TestTrue(TEXT("Controller should track open notes journal state."), PlayerController->IsDay1NotesJournalOpenForTests());
	TestTrue(TEXT("HUD should show an empty journal when no notes are recorded."), HUD->IsNotesJournalVisibleForTests());
	TestTrue(TEXT("HUD should expose empty journal state with no recorded notes."), HUD->IsNotesJournalEmptyForTests());

	TestTrue(TEXT("J should close the Day1 notes journal."), PlayerController->HandleInputKeyForTests(EKeys::J));
	TestFalse(TEXT("Controller should track closed notes journal state."), PlayerController->IsDay1NotesJournalOpenForTests());
	TestFalse(TEXT("HUD should hide the journal after toggling closed."), HUD->IsNotesJournalVisibleForTests());

	UNoteRecorderComponent* NoteRecorder = PlayerCharacter->GetNoteRecorderComponent();
	TestNotNull(TEXT("Player character should expose a note recorder component."), NoteRecorder);
	if (!NoteRecorder)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	FHorrorNoteMetadata StationMemo;
	StationMemo.NoteId = TEXT("Note.StationMemo");
	StationMemo.Title = FText::FromString(TEXT("站内备忘录"));
	StationMemo.Body = FText::FromString(TEXT("上次下潜后门禁密码已更改。"));
	NoteRecorder->RegisterNoteMetadata(StationMemo);
	TestTrue(TEXT("Runtime note recorder should accept station memo id."), NoteRecorder->AddRecordedNoteId(StationMemo.NoteId));

	TestTrue(TEXT("Tab should open the Day1 notes journal."), PlayerController->HandleInputKeyForTests(EKeys::Tab));
	TestTrue(TEXT("Controller should track open notes journal after Tab."), PlayerController->IsDay1NotesJournalOpenForTests());
	TestEqual(TEXT("HUD should receive recorded note entries from the pawn note recorder."), HUD->GetNotesJournalEntryCountForTests(), 1);
	TestEqual(TEXT("HUD should receive recorded note title fallback from metadata."), HUD->GetNotesJournalTitleForTests(0).ToString(), FString(TEXT("站内备忘录")));
	TestEqual(TEXT("HUD should receive recorded note body fallback from metadata."), HUD->GetNotesJournalBodyForTests(0).ToString(), FString(TEXT("上次下潜后门禁密码已更改。")));

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorPlayerControllerWrongDoorPasswordShowsHUDReasonTest,
	"HorrorProject.Player.Controller.WrongDoorPasswordShowsHUDReason",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorPlayerControllerWrongDoorPasswordShowsHUDReasonTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for wrong password HUD feedback coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	AHorrorPlayerController* PlayerController = World->SpawnActor<AHorrorPlayerController>();
	ADay1SliceHUD* HUD = World->SpawnActor<ADay1SliceHUD>();
	ADoorInteractable* Door = World->SpawnActor<ADoorInteractable>();
	TestNotNull(TEXT("Wrong password test should spawn a horror player controller."), PlayerController);
	TestNotNull(TEXT("Wrong password test should attach the native Day1 HUD."), HUD);
	TestNotNull(TEXT("Wrong password test should spawn a password door."), Door);
	if (!PlayerController || !HUD || !Door)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	PlayerController->MyHUD = HUD;
	Door->ConfigurePassword(TEXT("9999"), FText::FromString(TEXT("最终档案密码。")));
	PlayerController->BeginDoorPasswordEntry(Door);
	TestTrue(TEXT("Password entry should be active before wrong code input."), PlayerController->IsAwaitingDoorPassword());

	TestTrue(TEXT("Journal key should be consumed while password entry is active."), PlayerController->HandleInputKeyForTests(EKeys::J));
	TestFalse(TEXT("Password entry should block the notes journal from opening underneath."), PlayerController->IsDay1NotesJournalOpenForTests());

	TestTrue(TEXT("Wrong password test should accept first digit input."), PlayerController->HandleInputKeyForTests(EKeys::One));
	TestTrue(TEXT("Wrong password test should accept second digit input."), PlayerController->HandleInputKeyForTests(EKeys::Two));
	TestTrue(TEXT("Wrong password test should accept third digit input."), PlayerController->HandleInputKeyForTests(EKeys::Three));
	TestTrue(TEXT("Wrong password test should accept fourth digit input."), PlayerController->HandleInputKeyForTests(EKeys::Four));
	TestTrue(TEXT("Enter should submit the wrong password."), PlayerController->HandleInputKeyForTests(EKeys::Enter));

	TestTrue(TEXT("Wrong password should keep password entry active."), PlayerController->IsAwaitingDoorPassword());
	TestTrue(TEXT("Wrong password should keep the password prompt visible."), HUD->IsPasswordPromptVisibleForTests());
	TestTrue(TEXT("Wrong password should show a transient HUD reason."), HUD->IsTransientMessageVisibleForTests());
	TestEqual(TEXT("Wrong password should explain the rejection."), HUD->GetTransientMessageForTests().ToString(), FString(TEXT("密码错误。")));

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorPlayerControllerDay1NoteRecordedShowsReviewPromptTest,
	"HorrorProject.Player.Controller.Day1NoteRecordedShowsReviewPrompt",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorPlayerControllerDay1NoteRecordedShowsReviewPromptTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for note-recorded HUD feedback coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	AHorrorPlayerController* PlayerController = World->SpawnActor<AHorrorPlayerController>();
	ADay1SliceHUD* HUD = World->SpawnActor<ADay1SliceHUD>();
	AHorrorPlayerCharacter* PlayerCharacter = World->SpawnActor<AHorrorPlayerCharacter>();
	TestNotNull(TEXT("Note-recorded HUD feedback test should spawn a horror player controller."), PlayerController);
	TestNotNull(TEXT("Note-recorded HUD feedback test should attach the native Day1 HUD."), HUD);
	TestNotNull(TEXT("Note-recorded HUD feedback test should spawn a horror player character."), PlayerCharacter);
	if (!PlayerController || !HUD || !PlayerCharacter)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	PlayerController->MyHUD = HUD;
	PlayerController->Possess(PlayerCharacter);

	UNoteRecorderComponent* NoteRecorder = PlayerCharacter->GetNoteRecorderComponent();
	TestNotNull(TEXT("Note-recorded HUD feedback test should expose the note recorder."), NoteRecorder);
	if (!NoteRecorder)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	TestTrue(TEXT("Possessing a horror player should bind the controller to note-recorded events."), PlayerController->IsBoundToNoteRecorderForTests(NoteRecorder));
	TestTrue(TEXT("Possessing a horror player should register the controller note-recorded delegate."), PlayerController->HasNoteRecordedDelegateForTests(NoteRecorder));
	TestTrue(TEXT("Adding a new recorded note should trigger controller HUD feedback."), NoteRecorder->AddRecordedNoteId(TEXT("Note.ObjectiveHint.Evidence.Anomaly01")));
	TestEqual(TEXT("Recorded note delegate should invoke controller feedback handler."), PlayerController->GetNoteRecordedFeedbackCountForTests(), 1);
	TestTrue(TEXT("Recorded note feedback should be visible on the native HUD."), HUD->IsTransientMessageVisibleForTests());
	TestTrue(TEXT("Recorded note feedback should tell the player how to review notes."), HUD->GetTransientMessageForTests().ToString().Contains(TEXT("J键/Tab键")));

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorPlayerControllerDoorPasswordHintMentionsNotesTest,
	"HorrorProject.Player.Controller.DoorPasswordHintMentionsNotes",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorPlayerControllerDoorPasswordHintMentionsNotesTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for password note hint coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	AHorrorPlayerController* PlayerController = World->SpawnActor<AHorrorPlayerController>();
	ADay1SliceHUD* HUD = World->SpawnActor<ADay1SliceHUD>();
	AHorrorPlayerCharacter* PlayerCharacter = World->SpawnActor<AHorrorPlayerCharacter>();
	ADoorInteractable* Door = World->SpawnActor<ADoorInteractable>();
	TestNotNull(TEXT("Password note hint test should spawn a horror player controller."), PlayerController);
	TestNotNull(TEXT("Password note hint test should attach the native Day1 HUD."), HUD);
	TestNotNull(TEXT("Password note hint test should spawn a horror player character."), PlayerCharacter);
	TestNotNull(TEXT("Password note hint test should spawn a password door."), Door);
	if (!PlayerController || !HUD || !PlayerCharacter || !Door)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	PlayerController->MyHUD = HUD;
	PlayerController->Possess(PlayerCharacter);

	UNoteRecorderComponent* NoteRecorder = PlayerCharacter->GetNoteRecorderComponent();
	TestNotNull(TEXT("Password note hint test should expose the note recorder."), NoteRecorder);
	if (!NoteRecorder)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}
	TestTrue(TEXT("Password note hint test should record a clue note."), NoteRecorder->AddRecordedNoteId(TEXT("Note.Intro")));

	Door->ConfigurePassword(TEXT("0417"), FText::FromString(TEXT("最终档案密码。")));
	PlayerController->BeginDoorPasswordEntry(Door);

	const FString Hint = HUD->GetPasswordHintForTests().ToString();
	TestTrue(TEXT("Password prompt should keep the door-specific hint."), Hint.Contains(TEXT("最终档案密码。")));
	TestTrue(TEXT("Password prompt should point players back to recorded notes."), Hint.Contains(TEXT("J键/Tab键")));

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorPlayerControllerDay1HUDFeedbackEventsTest,
	"HorrorProject.Player.Controller.Day1HUDFeedbackEvents",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorPlayerControllerDay1HUDFeedbackEventsTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for Day1 HUD feedback event coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	AHorrorPlayerController* PlayerController = World->SpawnActor<AHorrorPlayerController>();
	TestNotNull(TEXT("HUD feedback event test should spawn a horror player controller."), PlayerController);
	if (!PlayerController)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	ADay1SliceHUD* HUD = World->SpawnActor<ADay1SliceHUD>();
	PlayerController->MyHUD = HUD;
	TestNotNull(TEXT("HUD feedback event test should attach the native Day1 HUD."), HUD);
	if (!HUD)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	UHorrorEventBusSubsystem* EventBus = World->GetSubsystem<UHorrorEventBusSubsystem>();
	TestNotNull(TEXT("HUD feedback event test should expose the event bus."), EventBus);
	if (!EventBus)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	PlayerController->BindObjectiveEventBusForTests();
	EventBus->Publish(
		HorrorFoundFootageTags::BodycamAcquiredEvent(),
		TEXT("Bodycam"),
		HorrorFoundFootageTags::BodycamAcquiredState(),
		PlayerController);
	TestFalse(TEXT("Objective milestone events should not claim a checkpoint was saved before the save event."), HUD->IsAutosaveIndicatorVisibleForTests());

	EventBus->Publish(
		HorrorSaveTags::CheckpointSavedEvent(),
		TEXT("Checkpoint.Day1.BodycamAcquired"),
		HorrorSaveTags::CheckpointSavedState(),
		PlayerController);
	TestTrue(TEXT("Checkpoint saved events should show an autosave HUD indicator."), HUD->IsAutosaveIndicatorVisibleForTests());

	EventBus->Publish(
		HorrorDay1Tags::PlayerFailureEvent(),
		TEXT("Death.Test"),
		HorrorDay1Tags::CheckpointRestoredState(),
		PlayerController);
	TestTrue(TEXT("Checkpoint restored failure events should show transient HUD feedback."), HUD->IsTransientMessageVisibleForTests());
	TestEqual(TEXT("Checkpoint restored feedback should be explicit."), HUD->GetTransientMessageForTests().ToString(), FString(TEXT("已恢复到上一个检查点。")));

	EventBus->Publish(
		HorrorDay1Tags::PlayerFailureEvent(),
		TEXT("Death.Test"),
		HorrorDay1Tags::CheckpointMissingState(),
		PlayerController);
	TestTrue(TEXT("Missing-checkpoint failure events should show transient HUD feedback."), HUD->IsTransientMessageVisibleForTests());
	TestEqual(TEXT("Missing-checkpoint feedback should be explicit."), HUD->GetTransientMessageForTests().ToString(), FString(TEXT("没有可用检查点。")));

	EventBus->Publish(HorrorDay1Tags::Day1CompletedEvent(), TEXT("Exit.Gate"), HorrorDay1Tags::Day1CompletedState(), PlayerController);
	TestTrue(TEXT("Day1 completion should show the cut-to-black completion overlay."), HUD->IsDay1CompletionOverlayVisibleForTests());
	TestEqual(TEXT("Day1 completion should use an explicit completion title."), HUD->GetDay1CompletionTitleForTests().ToString(), FString(TEXT("第 1 天完成")));

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorPlayerControllerDay1HUDRouteKitDangerTest,
	"HorrorProject.Player.Controller.Day1HUDRouteKitDanger",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorPlayerControllerDay1HUDRouteKitDangerTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for RouteKit danger HUD coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	World->GetWorldSettings()->DefaultGameMode = AHorrorGameModeBase::StaticClass();
	TestTrue(TEXT("Transient world should create the Day1 RouteKit danger game mode."), World->SetGameMode(FURL()));
	AHorrorGameModeBase* GameMode = World->GetAuthGameMode<AHorrorGameModeBase>();
	TestNotNull(TEXT("RouteKit danger test should expose the game mode."), GameMode);
	if (!GameMode)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	AHorrorPlayerController* PlayerController = World->SpawnActor<AHorrorPlayerController>();
	ADay1SliceHUD* HUD = World->SpawnActor<ADay1SliceHUD>();
	AHorrorPlayerCharacter* PlayerCharacter = World->SpawnActor<AHorrorPlayerCharacter>(FVector(1000.0f, 300.0f, 80.0f), FRotator::ZeroRotator);
	TestNotNull(TEXT("RouteKit danger test should spawn a horror player controller."), PlayerController);
	TestNotNull(TEXT("RouteKit danger test should attach the native Day1 HUD."), HUD);
	TestNotNull(TEXT("RouteKit danger test should spawn a horror player character."), PlayerCharacter);
	if (!PlayerController || !HUD || !PlayerCharacter)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	PlayerController->PlayerState = World->SpawnActor<APlayerState>();
	World->AddController(PlayerController);
	PlayerController->MyHUD = HUD;
	PlayerController->Possess(PlayerCharacter);

	GameMode->DispatchBeginPlay();
	ADeepWaterStationRouteKit* RouteKit = GameMode->GetRuntimeRouteKit();
	TestNotNull(TEXT("RouteKit danger test should bootstrap the runtime route kit."), RouteKit);
	AHorrorEncounterDirector* EncounterDirector = RouteKit ? RouteKit->GetSpawnedEncounterDirector() : nullptr;
	TestNotNull(TEXT("RouteKit danger test should bootstrap the placed route encounter director."), EncounterDirector);
	if (!RouteKit || !EncounterDirector)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	TestFalse(TEXT("HUD danger should start hidden before the route encounter reveal."), HUD->IsDangerStatusVisibleForTests());
	TestTrue(TEXT("RouteKit should reveal its encounter when the player reaches the reveal point."), RouteKit->TriggerEncounterReveal(PlayerCharacter));
	TestTrue(TEXT("RouteKit should report the route gated while the encounter is revealed."), RouteKit->IsRouteGatedByEncounter());

	PlayerController->RefreshDay1HUDStateForTests();
	TestTrue(TEXT("HUD danger should reflect the RouteKit encounter gate, not only the GameMode fallback director."), HUD->IsDangerStatusVisibleForTests());

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

#endif
