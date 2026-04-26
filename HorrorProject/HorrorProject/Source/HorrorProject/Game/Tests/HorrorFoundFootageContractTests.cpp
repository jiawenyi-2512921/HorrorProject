// Copyright Epic Games, Inc. All Rights Reserved.

#if WITH_DEV_AUTOMATION_TESTS

#include "Game/HorrorFoundFootageContract.h"

#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorFoundFootageContractTracksMilestonesTest,
	"HorrorProject.Game.FoundFootage.ContractTracksMilestones",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FHorrorFoundFootageContractTracksMilestonesTest::RunTest(const FString& Parameters)
{
	FHorrorFoundFootageContract Contract;

	TestFalse(TEXT("Exit starts locked."), Contract.IsExitUnlocked());
	TestFalse(
		TEXT("Direct exit unlock events should be rejected."),
		Contract.RecordEvent(HorrorFoundFootageTags::ExitUnlockedEvent(), TEXT("Bypass"), nullptr, nullptr));
	TestFalse(TEXT("Exit remains locked after direct terminal event."), Contract.IsExitUnlocked());

	TArray<FHorrorFoundFootageRecordedEvent> RecordedEvents;
	TArray<FHorrorFoundFootageStateChange> StateChanges;

	TestFalse(
		TEXT("State tags should be rejected as objective events."),
		Contract.RecordEvent(HorrorFoundFootageTags::BodycamAcquiredState(), TEXT("InvalidStateEvent"), &RecordedEvents, &StateChanges));
	TestFalse(
		TEXT("Rejected state tags should not be recorded as objective events."),
		Contract.HasRecordedEvent(HorrorFoundFootageTags::BodycamAcquiredState()));
	TestEqual(TEXT("Rejected state tags should not append recorded event output."), RecordedEvents.Num(), 0);
	TestEqual(TEXT("Rejected state tags should not append state changes."), StateChanges.Num(), 0);

	TestFalse(
		TEXT("First note should be rejected before bodycam acquisition."),
		Contract.RecordEvent(HorrorFoundFootageTags::FirstNoteCollectedEvent(), TEXT("Note01"), &RecordedEvents, &StateChanges));
	TestFalse(
		TEXT("First anomaly should be rejected before the first note."),
		Contract.RecordEvent(HorrorFoundFootageTags::FirstAnomalyRecordedEvent(), TEXT("Anomaly01"), &RecordedEvents, &StateChanges));
	TestFalse(
		TEXT("Archive review should be rejected before the first anomaly recording."),
		Contract.RecordEvent(HorrorFoundFootageTags::ArchiveReviewedEvent(), TEXT("ArchiveTerminal"), &RecordedEvents, &StateChanges));

	TestTrue(
		TEXT("Bodycam acquisition should be accepted once."),
		Contract.RecordEvent(HorrorFoundFootageTags::BodycamAcquiredEvent(), TEXT("BodycamPickup"), &RecordedEvents, &StateChanges));
	TestTrue(
		TEXT("Bodycam state should be marked complete."),
		Contract.HasCompletedState(HorrorFoundFootageTags::BodycamAcquiredState()));
	TestFalse(
		TEXT("Duplicate bodycam events should be ignored."),
		Contract.RecordEvent(HorrorFoundFootageTags::BodycamAcquiredEvent(), TEXT("BodycamPickup"), &RecordedEvents, &StateChanges));

	TestTrue(
		TEXT("First note should be accepted."),
		Contract.RecordEvent(HorrorFoundFootageTags::FirstNoteCollectedEvent(), TEXT("Note01"), &RecordedEvents, &StateChanges));
	TestTrue(
		TEXT("First anomaly should be accepted."),
		Contract.RecordEvent(HorrorFoundFootageTags::FirstAnomalyRecordedEvent(), TEXT("Anomaly01"), &RecordedEvents, &StateChanges));
	TestTrue(
		TEXT("Archive review should be accepted."),
		Contract.RecordEvent(HorrorFoundFootageTags::ArchiveReviewedEvent(), TEXT("ArchiveTerminal"), &RecordedEvents, &StateChanges));

	TestTrue(TEXT("Exit should unlock after all prerequisite milestones complete."), Contract.IsExitUnlocked());
	TestTrue(
		TEXT("Exit unlocked state should be marked complete."),
		Contract.HasCompletedState(HorrorFoundFootageTags::ExitUnlockedState()));
	TestTrue(
		TEXT("Exit unlocked event should be recorded automatically."),
		Contract.HasRecordedEvent(HorrorFoundFootageTags::ExitUnlockedEvent()));

	const FHorrorFoundFootageProgressSnapshot Snapshot = Contract.BuildSnapshot();
	TestEqual(TEXT("Snapshot should expose all five milestones."), Snapshot.Milestones.Num(), 5);
	TestEqual(TEXT("Snapshot should count all completed milestones."), Snapshot.CompletedMilestoneCount, 5);
	TestTrue(TEXT("Snapshot should report an unlocked exit."), Snapshot.bExitUnlocked);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorFoundFootageImportRejectsOutOfOrderMilestonesTest,
	"HorrorProject.Game.FoundFootage.ImportSaveStateRejectsOutOfOrderMilestones",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FHorrorFoundFootageImportRejectsOutOfOrderMilestonesTest::RunTest(const FString& Parameters)
{
	FHorrorFoundFootageSaveState SaveState;
	SaveState.RecordedObjectiveEvents = { HorrorFoundFootageTags::FirstNoteCollectedEvent() };
	SaveState.CompletedObjectiveStates = { HorrorFoundFootageTags::FirstNoteCollectedState() };

	FHorrorFoundFootageContract Contract;
	Contract.ImportSaveState(SaveState);

	TestFalse(TEXT("Out-of-order imported first note event should be rejected."), Contract.HasRecordedEvent(HorrorFoundFootageTags::FirstNoteCollectedEvent()));
	TestFalse(TEXT("Out-of-order imported first note state should be rejected."), Contract.HasCompletedState(HorrorFoundFootageTags::FirstNoteCollectedState()));
	TestFalse(TEXT("Out-of-order import should not unlock exit."), Contract.IsExitUnlocked());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorFoundFootageImportRejectsTerminalBypassTest,
	"HorrorProject.Game.FoundFootage.ImportSaveStateRejectsTerminalBypass",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FHorrorFoundFootageImportRejectsTerminalBypassTest::RunTest(const FString& Parameters)
{
	FHorrorFoundFootageSaveState SaveState;
	SaveState.RecordedObjectiveEvents = {
		HorrorFoundFootageTags::BodycamAcquiredEvent(),
		HorrorFoundFootageTags::ExitUnlockedEvent()
	};
	SaveState.CompletedObjectiveStates = {
		HorrorFoundFootageTags::BodycamAcquiredState(),
		HorrorFoundFootageTags::ExitUnlockedState()
	};

	FHorrorFoundFootageContract Contract;
	Contract.ImportSaveState(SaveState);

	TestTrue(TEXT("Valid imported bodycam event should be accepted."), Contract.HasRecordedEvent(HorrorFoundFootageTags::BodycamAcquiredEvent()));
	TestTrue(TEXT("Valid imported bodycam state should be accepted."), Contract.HasCompletedState(HorrorFoundFootageTags::BodycamAcquiredState()));
	TestFalse(TEXT("Terminal imported exit event should be rejected without all prerequisites."), Contract.HasRecordedEvent(HorrorFoundFootageTags::ExitUnlockedEvent()));
	TestFalse(TEXT("Terminal imported exit state should be rejected without all prerequisites."), Contract.HasCompletedState(HorrorFoundFootageTags::ExitUnlockedState()));
	TestFalse(TEXT("Terminal bypass import should not unlock exit."), Contract.IsExitUnlocked());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorFoundFootageImportIgnoresUnknownAndDuplicateTagsTest,
	"HorrorProject.Game.FoundFootage.ImportSaveStateIgnoresUnknownAndDuplicateTags",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FHorrorFoundFootageImportIgnoresUnknownAndDuplicateTagsTest::RunTest(const FString& Parameters)
{
	FHorrorFoundFootageSaveState SaveState;
	SaveState.RecordedObjectiveEvents = {
		FGameplayTag(),
		HorrorFoundFootageTags::BodycamAcquiredEvent(),
		HorrorFoundFootageTags::BodycamAcquiredEvent(),
		HorrorFoundFootageTags::ArchiveReviewedState()
	};
	SaveState.CompletedObjectiveStates = {
		FGameplayTag(),
		HorrorFoundFootageTags::BodycamAcquiredState(),
		HorrorFoundFootageTags::BodycamAcquiredState(),
		HorrorFoundFootageTags::ArchiveReviewedEvent()
	};

	FHorrorFoundFootageContract Contract;
	Contract.ImportSaveState(SaveState);
	const FHorrorFoundFootageSaveState ExportedState = Contract.ExportSaveState();

	TestEqual(TEXT("Imported duplicate/unknown events should export one canonical bodycam event."), ExportedState.RecordedObjectiveEvents.Num(), 1);
	TestEqual(TEXT("Imported duplicate/unknown states should export one canonical bodycam state."), ExportedState.CompletedObjectiveStates.Num(), 1);
	if (ExportedState.RecordedObjectiveEvents.Num() == 1)
	{
		TestEqual(TEXT("Imported event export should be canonical bodycam event."), ExportedState.RecordedObjectiveEvents[0], HorrorFoundFootageTags::BodycamAcquiredEvent());
	}
	if (ExportedState.CompletedObjectiveStates.Num() == 1)
	{
		TestEqual(TEXT("Imported state export should be canonical bodycam state."), ExportedState.CompletedObjectiveStates[0], HorrorFoundFootageTags::BodycamAcquiredState());
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorFoundFootageImportRoundTripsValidProgressTest,
	"HorrorProject.Game.FoundFootage.ImportSaveStateRoundTripsValidProgress",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FHorrorFoundFootageImportRoundTripsValidProgressTest::RunTest(const FString& Parameters)
{
	FHorrorFoundFootageContract SourceContract;
	TestTrue(TEXT("Bodycam should record before export."), SourceContract.RecordEvent(HorrorFoundFootageTags::BodycamAcquiredEvent(), TEXT("Bodycam")));
	TestTrue(TEXT("First note should record before export."), SourceContract.RecordEvent(HorrorFoundFootageTags::FirstNoteCollectedEvent(), TEXT("Note")));
	TestTrue(TEXT("First anomaly should record before export."), SourceContract.RecordEvent(HorrorFoundFootageTags::FirstAnomalyRecordedEvent(), TEXT("Anomaly")));
	TestFalse(TEXT("Partial exported progress should not unlock exit."), SourceContract.IsExitUnlocked());

	FHorrorFoundFootageContract ImportedContract;
	ImportedContract.ImportSaveState(SourceContract.ExportSaveState());
	const FHorrorFoundFootageSaveState ImportedState = ImportedContract.ExportSaveState();

	TestEqual(TEXT("Imported partial progress should preserve three canonical events."), ImportedState.RecordedObjectiveEvents.Num(), 3);
	TestEqual(TEXT("Imported partial progress should preserve three canonical states."), ImportedState.CompletedObjectiveStates.Num(), 3);
	TestTrue(TEXT("Imported bodycam event should be present."), ImportedContract.HasRecordedEvent(HorrorFoundFootageTags::BodycamAcquiredEvent()));
	TestTrue(TEXT("Imported first note event should be present."), ImportedContract.HasRecordedEvent(HorrorFoundFootageTags::FirstNoteCollectedEvent()));
	TestTrue(TEXT("Imported first anomaly event should be present."), ImportedContract.HasRecordedEvent(HorrorFoundFootageTags::FirstAnomalyRecordedEvent()));
	TestFalse(TEXT("Imported partial progress should not auto-unlock exit."), ImportedContract.IsExitUnlocked());
	const FHorrorFoundFootageProgressSnapshot Snapshot = ImportedContract.BuildSnapshot();
	TestEqual(TEXT("Imported partial progress snapshot should count three completed milestones."), Snapshot.CompletedMilestoneCount, 3);
	TestFalse(TEXT("Imported partial progress snapshot should report locked exit."), Snapshot.bExitUnlocked);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorFoundFootageImportDerivesExitUnlockFromCompletePrerequisitesTest,
	"HorrorProject.Game.FoundFootage.ImportSaveStateDerivesExitUnlockFromCompletePrerequisites",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FHorrorFoundFootageImportDerivesExitUnlockFromCompletePrerequisitesTest::RunTest(const FString& Parameters)
{
	FHorrorFoundFootageSaveState SaveState;
	SaveState.RecordedObjectiveEvents = {
		HorrorFoundFootageTags::BodycamAcquiredEvent(),
		HorrorFoundFootageTags::FirstNoteCollectedEvent(),
		HorrorFoundFootageTags::FirstAnomalyRecordedEvent(),
		HorrorFoundFootageTags::ArchiveReviewedEvent()
	};
	SaveState.CompletedObjectiveStates = {
		HorrorFoundFootageTags::BodycamAcquiredState(),
		HorrorFoundFootageTags::FirstNoteCollectedState(),
		HorrorFoundFootageTags::FirstAnomalyRecordedState(),
		HorrorFoundFootageTags::ArchiveReviewedState()
	};

	FHorrorFoundFootageContract Contract;
	Contract.ImportSaveState(SaveState);
	const FHorrorFoundFootageSaveState ExportedState = Contract.ExportSaveState();

	TestTrue(TEXT("Complete imported prerequisites should unlock exit."), Contract.IsExitUnlocked());
	TestTrue(TEXT("Complete imported prerequisites should derive exit event."), Contract.HasRecordedEvent(HorrorFoundFootageTags::ExitUnlockedEvent()));
	TestTrue(TEXT("Complete imported prerequisites should derive exit state."), Contract.HasCompletedState(HorrorFoundFootageTags::ExitUnlockedState()));
	TestEqual(TEXT("Complete imported prerequisites should export five canonical events."), ExportedState.RecordedObjectiveEvents.Num(), 5);
	TestEqual(TEXT("Complete imported prerequisites should export five canonical states."), ExportedState.CompletedObjectiveStates.Num(), 5);
	return true;
}

#endif

