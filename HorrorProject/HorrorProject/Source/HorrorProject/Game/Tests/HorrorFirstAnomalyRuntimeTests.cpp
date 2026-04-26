#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR && HORRORPROJECT_ENABLE_LEGACY_AUTOMATION_TESTS

// Copyright Epic Games, Inc. All Rights Reserved.

#if WITH_DEV_AUTOMATION_TESTS

#include "Game/HorrorAnomalyDirector.h"
#include "Game/HorrorFoundFootageContract.h"

#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorAnomalyDirectorRequiresRecordingTest,
	"HorrorProject.Game.FoundFootage.FirstAnomaly.RequiresRecording",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorAnomalyDirectorRequiresRecordingTest::RunTest(const FString& Parameters)
{
	FHorrorFoundFootageContract Contract;
	FHorrorAnomalyDirector AnomalyDirector;
	TArray<FHorrorFoundFootageRecordedEvent> RecordedEvents;
	TArray<FHorrorFoundFootageStateChange> StateChanges;

	TestFalse(TEXT("Recording attempts should fail before a candidate is registered."), AnomalyDirector.TryRecordFirstAnomaly(true, Contract, &RecordedEvents, &StateChanges));
	TestFalse(TEXT("Starting a first-anomaly candidate should fail before first note is collected."), AnomalyDirector.BeginFirstAnomalyCandidate(TEXT("AnomalyBeforeNote"), Contract));
	TestTrue(TEXT("Bodycam acquisition should unlock first note collection for the anomaly precondition."), Contract.RecordEvent(HorrorFoundFootageTags::BodycamAcquiredEvent(), TEXT("Bodycam")));
	TestTrue(TEXT("First note collection should unlock first anomaly candidates."), Contract.RecordEvent(HorrorFoundFootageTags::FirstNoteCollectedEvent(), TEXT("FirstNote")));
	TestTrue(TEXT("Starting a first-anomaly candidate should succeed with a valid source id."), AnomalyDirector.BeginFirstAnomalyCandidate(TEXT("Anomaly01"), Contract));
	TestTrue(TEXT("The anomaly director should retain a pending candidate until it records successfully."), AnomalyDirector.HasPendingFirstAnomalyCandidate());
	TestEqual(TEXT("The anomaly director should preserve the first pending candidate source id."), AnomalyDirector.GetPendingFirstAnomalySourceId(), FName(TEXT("Anomaly01")));
	TestFalse(TEXT("A second candidate should not overwrite an existing pending candidate."), AnomalyDirector.BeginFirstAnomalyCandidate(TEXT("Anomaly02"), Contract));
	TestEqual(TEXT("Rejected pending candidates should leave the original source id intact."), AnomalyDirector.GetPendingFirstAnomalySourceId(), FName(TEXT("Anomaly01")));

	TestFalse(TEXT("A non-recording attempt should not record the first anomaly."), AnomalyDirector.TryRecordFirstAnomaly(false, Contract, &RecordedEvents, &StateChanges));
	TestFalse(TEXT("The contract should stay unchanged until recording is active."), Contract.HasRecordedEvent(HorrorFoundFootageTags::FirstAnomalyRecordedEvent()));
	TestTrue(TEXT("Rejected attempts should leave the candidate pending."), AnomalyDirector.HasPendingFirstAnomalyCandidate());

	TestTrue(TEXT("Recording=true should allow the first anomaly to be recorded."), AnomalyDirector.TryRecordFirstAnomaly(true, Contract, &RecordedEvents, &StateChanges));
	TestTrue(TEXT("The first anomaly event should be recorded by the contract."), Contract.HasRecordedEvent(HorrorFoundFootageTags::FirstAnomalyRecordedEvent()));
	TestTrue(TEXT("Recording the event should also complete the first anomaly objective state."), Contract.HasCompletedState(HorrorFoundFootageTags::FirstAnomalyRecordedState()));
	TestFalse(TEXT("Successful recording should clear the pending candidate."), AnomalyDirector.HasPendingFirstAnomalyCandidate());
	TestEqual(TEXT("Exactly one event should be recorded through the helper."), RecordedEvents.Num(), 1);
	if (RecordedEvents.Num() > 0)
	{
		TestEqual(TEXT("The helper should preserve the candidate source id when it records."), RecordedEvents[0].SourceId, FName(TEXT("Anomaly01")));
	}
	TestEqual(TEXT("Recording the first anomaly should complete exactly one state."), StateChanges.Num(), 1);
	TestFalse(TEXT("Once recorded, a duplicate first-anomaly candidate should be ignored."), AnomalyDirector.BeginFirstAnomalyCandidate(TEXT("Anomaly02"), Contract));
	TestFalse(TEXT("Without a new candidate, duplicate recording attempts should be ignored."), AnomalyDirector.TryRecordFirstAnomaly(true, Contract, &RecordedEvents, &StateChanges));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorAnomalyDirectorImportPendingCandidateSanitizesRestoredStateTest,
	"HorrorProject.Game.FoundFootage.FirstAnomaly.ImportPendingCandidateSanitizesRestoredState",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorAnomalyDirectorImportPendingCandidateSanitizesRestoredStateTest::RunTest(const FString& Parameters)
{
	FHorrorFoundFootageContract Contract;
	FHorrorAnomalyDirector AnomalyDirector;

	AnomalyDirector.ImportPendingFirstAnomalyCandidate(NAME_None, Contract);
	TestFalse(TEXT("NAME_None import should leave no pending candidate."), AnomalyDirector.HasPendingFirstAnomalyCandidate());

	AnomalyDirector.ImportPendingFirstAnomalyCandidate(TEXT("AnomalyBeforeNote"), Contract);
	TestFalse(TEXT("Pending candidate import should fail before first note state exists."), AnomalyDirector.HasPendingFirstAnomalyCandidate());

	TestTrue(TEXT("Bodycam acquisition should unlock first note import coverage."), Contract.RecordEvent(HorrorFoundFootageTags::BodycamAcquiredEvent(), TEXT("Bodycam")));
	TestTrue(TEXT("First note collection should allow pending anomaly imports."), Contract.RecordEvent(HorrorFoundFootageTags::FirstNoteCollectedEvent(), TEXT("FirstNote")));
	AnomalyDirector.ImportPendingFirstAnomalyCandidate(TEXT("Anomaly01"), Contract);
	TestTrue(TEXT("Valid restored pending candidate should import after first note."), AnomalyDirector.HasPendingFirstAnomalyCandidate());
	TestEqual(TEXT("Valid restored pending candidate should preserve source id."), AnomalyDirector.GetPendingFirstAnomalySourceId(), FName(TEXT("Anomaly01")));

	TestTrue(TEXT("Restored pending candidate should record while recording."), AnomalyDirector.TryRecordFirstAnomaly(true, Contract));
	TestTrue(TEXT("Recording restored candidate should write the anomaly event."), Contract.HasRecordedEvent(HorrorFoundFootageTags::FirstAnomalyRecordedEvent()));
	TestFalse(TEXT("Recording restored candidate should clear pending state."), AnomalyDirector.HasPendingFirstAnomalyCandidate());

	AnomalyDirector.ImportPendingFirstAnomalyCandidate(TEXT("StaleAnomaly"), Contract);
	TestFalse(TEXT("Import should not resurrect pending candidates after anomaly completion."), AnomalyDirector.HasPendingFirstAnomalyCandidate());
	TestEqual(TEXT("Rejected stale import should leave pending source empty."), AnomalyDirector.GetPendingFirstAnomalySourceId(), NAME_None);

	return true;
}

#endif

#endif // WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR
