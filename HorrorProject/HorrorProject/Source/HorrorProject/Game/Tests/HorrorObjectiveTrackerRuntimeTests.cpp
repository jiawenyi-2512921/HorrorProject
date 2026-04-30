// Copyright Epic Games, Inc. All Rights Reserved.

#include "Game/HorrorGameModeBase.h"

#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR

#include "Game/HorrorFoundFootageContract.h"
#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorObjectiveTrackerSnapshotProgressionTest,
	"HorrorProject.Game.Objectives.TrackerSnapshotProgression",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorObjectiveTrackerSnapshotProgressionTest::RunTest(const FString& Parameters)
{
	AHorrorGameModeBase* GameMode = NewObject<AHorrorGameModeBase>();
	TestNotNull(TEXT("Objective tracker test should create a GameMode instance."), GameMode);
	if (!GameMode)
	{
		return false;
	}

	FHorrorObjectiveTrackerSnapshot Tracker = GameMode->BuildObjectiveTrackerSnapshot();
	TestEqual(TEXT("Tracker should start at bodycam recovery."), Tracker.Stage, EHorrorObjectiveTrackerStage::RecoverBodycam);
	TestEqual(TEXT("Initial tracker should expose the first-loop milestone count."), Tracker.RequiredMilestoneCount, 5);
	TestEqual(TEXT("Initial tracker should expose a full Day1 checklist."), Tracker.ChecklistItems.Num(), 6);
	if (Tracker.ChecklistItems.Num() == 6)
	{
		TestTrue(TEXT("Initial checklist should make bodycam recovery active."), Tracker.ChecklistItems[0].bActive);
		TestFalse(TEXT("Initial checklist should not mark bodycam complete."), Tracker.ChecklistItems[0].bComplete);
		TestTrue(TEXT("Initial checklist labels should be Chinese."), Tracker.ChecklistItems[0].Label.ToString().Contains(TEXT("随身摄像机")));
	}
	TestEqual(TEXT("Initial tracker should have no completed milestones."), Tracker.CompletedMilestoneCount, 0);
	TestEqual(TEXT("Initial tracker should start at zero progress."), Tracker.ProgressFraction, 0.0f);
	TestFalse(TEXT("Bodycam recovery should not require active recording."), Tracker.bRequiresRecording);
	TestFalse(TEXT("Bodycam recovery should not be marked complete."), Tracker.bComplete);
	TestTrue(TEXT("Initial tracker should present a Chinese primary instruction."), Tracker.PrimaryInstruction.ToString().Contains(TEXT("随身摄像机")));

	TestTrue(TEXT("Bodycam acquisition should advance the tracker."), GameMode->TryAcquireBodycam(TEXT("Evidence.Bodycam"), true));
	Tracker = GameMode->BuildObjectiveTrackerSnapshot();
	TestEqual(TEXT("Tracker should advance to the first note."), Tracker.Stage, EHorrorObjectiveTrackerStage::ReadFirstNote);
	TestEqual(TEXT("Bodycam milestone should count as progress."), Tracker.CompletedMilestoneCount, 1);
	TestTrue(TEXT("Progress should increase after bodycam acquisition."), Tracker.ProgressFraction > 0.0f);
	TestTrue(TEXT("First note tracker should mention the station memo."), Tracker.PrimaryInstruction.ToString().Contains(TEXT("备忘录")));

	TestTrue(TEXT("First note should unlock anomaly framing."), GameMode->TryCollectFirstNote(TEXT("Note.Intro")));
	Tracker = GameMode->BuildObjectiveTrackerSnapshot();
	TestEqual(TEXT("Tracker should ask the player to frame the first anomaly."), Tracker.Stage, EHorrorObjectiveTrackerStage::FrameFirstAnomaly);
	TestFalse(TEXT("Framing the anomaly should not yet require recording."), Tracker.bRequiresRecording);
	TestTrue(TEXT("Framing tracker should mention anomaly."), Tracker.PrimaryInstruction.ToString().Contains(TEXT("异常")));

	TestTrue(TEXT("First anomaly candidate should be retained by the tracker."), GameMode->BeginFirstAnomalyCandidate(TEXT("Evidence.Anomaly01")));
	Tracker = GameMode->BuildObjectiveTrackerSnapshot();
	TestEqual(TEXT("Tracker should advance to anomaly recording once a candidate is pending."), Tracker.Stage, EHorrorObjectiveTrackerStage::RecordFirstAnomaly);
	if (Tracker.ChecklistItems.Num() == 6)
	{
		TestTrue(TEXT("Checklist should mark anomaly framing complete once a candidate is pending."), Tracker.ChecklistItems[2].bComplete);
		TestTrue(TEXT("Checklist should make anomaly recording active while pending."), Tracker.ChecklistItems[3].bActive);
		TestTrue(TEXT("Checklist should flag anomaly recording as requiring recording."), Tracker.ChecklistItems[3].bRequiresRecording);
	}
	TestTrue(TEXT("Pending anomaly recording should require active recording."), Tracker.bRequiresRecording);
	TestTrue(TEXT("Pending anomaly recording should be urgent."), Tracker.bUrgent);
	TestTrue(TEXT("Recording tracker should tell the player to keep recording."), Tracker.PrimaryInstruction.ToString().Contains(TEXT("录像")));

	TestTrue(TEXT("Recording the anomaly should advance to archive review."), GameMode->TryRecordFirstAnomaly(true));
	Tracker = GameMode->BuildObjectiveTrackerSnapshot();
	TestEqual(TEXT("Tracker should advance to archive review after anomaly recording."), Tracker.Stage, EHorrorObjectiveTrackerStage::ReviewArchive);
	TestFalse(TEXT("Archive review should no longer require recording."), Tracker.bRequiresRecording);
	TestTrue(TEXT("Archive tracker should mention the terminal."), Tracker.PrimaryInstruction.ToString().Contains(TEXT("档案")));

	TestTrue(TEXT("Archive review should unlock escape."), GameMode->TryReviewArchive(TEXT("Archive.Terminal")));
	Tracker = GameMode->BuildObjectiveTrackerSnapshot();
	TestEqual(TEXT("Tracker should advance to escape once the exit unlocks."), Tracker.Stage, EHorrorObjectiveTrackerStage::Escape);
	TestTrue(TEXT("Escape tracker should expose the unlocked exit."), Tracker.bExitUnlocked);
	TestTrue(TEXT("Escape tracker should be urgent."), Tracker.bUrgent);
	TestEqual(TEXT("All found-footage milestones should be complete before Day1 exit."), Tracker.CompletedMilestoneCount, 5);

	GameMode->ImportDay1CompleteState(true);
	Tracker = GameMode->BuildObjectiveTrackerSnapshot();
	TestEqual(TEXT("Tracker should expose the completed Day1 state."), Tracker.Stage, EHorrorObjectiveTrackerStage::Day1Complete);
	TestTrue(TEXT("Completed tracker should expose completion."), Tracker.bComplete);
	TestTrue(TEXT("Completed tracker should keep progress full."), FMath::IsNearlyEqual(Tracker.ProgressFraction, 1.0f));
	TestTrue(TEXT("Completed tracker should mention saved evidence."), Tracker.PrimaryInstruction.ToString().Contains(TEXT("证据")));

	return true;
}

#endif
