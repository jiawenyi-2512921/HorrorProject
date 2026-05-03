// Copyright Epic Games, Inc. All Rights Reserved.

#include "Game/HorrorGameModeBase.h"

#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR

#include "Misc/AutomationTest.h"
#include "Components/BoxComponent.h"
#include "Game/HorrorCampaignObjectiveActor.h"
#include "Game/HorrorFoundFootageContract.h"
#include "Tests/AutomationCommon.h"

namespace
{
bool ContainsLatinLetter(const FString& Text)
{
	for (const TCHAR Character : Text)
	{
		if ((Character >= TEXT('A') && Character <= TEXT('Z')) || (Character >= TEXT('a') && Character <= TEXT('z')))
		{
			return true;
		}
	}

	return false;
}
}

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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorObjectiveTrackerDay1ChecklistCarriesActionAndLockSummariesTest,
	"HorrorProject.Game.Objectives.Day1ChecklistCarriesActionAndLockSummaries",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorObjectiveTrackerDay1ChecklistCarriesActionAndLockSummariesTest::RunTest(const FString& Parameters)
{
	AHorrorGameModeBase* GameMode = NewObject<AHorrorGameModeBase>();
	TestNotNull(TEXT("Day1 checklist summary test should create a GameMode instance."), GameMode);
	if (!GameMode)
	{
		return false;
	}

	FHorrorObjectiveTrackerSnapshot Tracker = GameMode->BuildObjectiveTrackerSnapshot();
	TestEqual(TEXT("Day1 checklist should expose all six opening beats."), Tracker.ChecklistItems.Num(), 6);
	if (Tracker.ChecklistItems.Num() != 6)
	{
		return false;
	}

	TestTrue(TEXT("Bodycam beat should be active at the start."), Tracker.ChecklistItems[0].bActive);
	TestFalse(TEXT("Active bodycam beat should carry an action cue."), Tracker.ChecklistItems[0].NextActionLabel.IsEmpty());
	TestFalse(TEXT("Active bodycam beat should carry a status summary."), Tracker.ChecklistItems[0].StatusText.IsEmpty());
	TestTrue(TEXT("Future memo beat should be blocked until the bodycam is recovered."), Tracker.ChecklistItems[1].bBlocked);
	TestFalse(TEXT("Future memo beat should explain its prerequisite."), Tracker.ChecklistItems[1].LockReason.IsEmpty());
	TestTrue(TEXT("Archive beat should still be blocked at the start."), Tracker.ChecklistItems[4].bBlocked);
	TestTrue(TEXT("Escape beat should still be blocked at the start."), Tracker.ChecklistItems[5].bBlocked);

	for (const FHorrorObjectiveChecklistItem& Item : Tracker.ChecklistItems)
	{
		TestFalse(TEXT("Day1 checklist detail should be localized."), ContainsLatinLetter(Item.Detail.ToString()));
		TestFalse(TEXT("Day1 checklist status should be localized."), ContainsLatinLetter(Item.StatusText.ToString()));
		TestFalse(TEXT("Day1 checklist action/recovery/lock text should be localized."),
			ContainsLatinLetter(Item.NextActionLabel.ToString() + Item.FailureRecoveryLabel.ToString() + Item.LockReason.ToString()));
	}

	TestTrue(TEXT("Bodycam acquisition should advance the Day1 checklist."), GameMode->TryAcquireBodycam(TEXT("Evidence.Bodycam"), true));
	TestTrue(TEXT("First note should unlock anomaly framing."), GameMode->TryCollectFirstNote(TEXT("Note.Intro")));
	TestTrue(TEXT("First anomaly candidate should be retained by the tracker."), GameMode->BeginFirstAnomalyCandidate(TEXT("Evidence.Anomaly01")));
	Tracker = GameMode->BuildObjectiveTrackerSnapshot();
	TestEqual(TEXT("Pending anomaly should make recording the active checklist beat."), Tracker.Stage, EHorrorObjectiveTrackerStage::RecordFirstAnomaly);
	TestTrue(TEXT("Recording beat should be active while the anomaly is pending."), Tracker.ChecklistItems[3].bActive);
	TestTrue(TEXT("Recording beat should require the camera recording mode."), Tracker.ChecklistItems[3].bRequiresRecording);
	TestTrue(TEXT("Recording beat action cue should mention recording."), Tracker.ChecklistItems[3].NextActionLabel.ToString().Contains(TEXT("录像")));
	TestFalse(TEXT("Archive beat should remain blocked until the recording is captured."), Tracker.ChecklistItems[4].LockReason.IsEmpty());

	TestTrue(TEXT("Recording the anomaly should unlock archive review."), GameMode->TryRecordFirstAnomaly(true));
	Tracker = GameMode->BuildObjectiveTrackerSnapshot();
	TestEqual(TEXT("Archive review should become the active checklist beat."), Tracker.Stage, EHorrorObjectiveTrackerStage::ReviewArchive);
	TestTrue(TEXT("Archive beat should be active after recording."), Tracker.ChecklistItems[4].bActive);
	TestTrue(TEXT("Archive beat action cue should mention the archive terminal."), Tracker.ChecklistItems[4].NextActionLabel.ToString().Contains(TEXT("档案")));
	TestTrue(TEXT("Escape beat should explain that the archive review is still required."), Tracker.ChecklistItems[5].bBlocked && !Tracker.ChecklistItems[5].LockReason.IsEmpty());

	TestTrue(TEXT("Archive review should unlock escape."), GameMode->TryReviewArchive(TEXT("Archive.Terminal")));
	Tracker = GameMode->BuildObjectiveTrackerSnapshot();
	TestEqual(TEXT("Escape should become the active checklist beat."), Tracker.Stage, EHorrorObjectiveTrackerStage::Escape);
	TestTrue(TEXT("Escape beat should be active after the archive review."), Tracker.ChecklistItems[5].bActive);
	TestFalse(TEXT("Escape beat should provide an action cue."), Tracker.ChecklistItems[5].NextActionLabel.IsEmpty());

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorObjectiveTrackerPrefersDay1OnDay1MapBeforeCompletionTest,
	"HorrorProject.Game.Objectives.TrackerPrefersDay1OnDay1MapBeforeCompletion",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorObjectiveTrackerPrefersDay1OnDay1MapBeforeCompletionTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for Day1 priority tracker coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	TestTrue(TEXT("Transient world should create the Horror game mode."), World->SetGameMode(FURL()));
	AHorrorGameModeBase* GameMode = World->GetAuthGameMode<AHorrorGameModeBase>();
	TestNotNull(TEXT("Day1 priority tracker test should expose the game mode."), GameMode);
	if (!GameMode)
	{
		TestTrue(TEXT("Transient world should be destroyed cleanly after GameMode creation failure."), TestWorld.DestroyTestWorld(false));
		return false;
	}

	FString ErrorMessage;
	GameMode->InitGame(TEXT("/Game/DeepWaterStation/Maps/DemoMap_VerticalSlice_Day1"), TEXT(""), ErrorMessage);
	GameMode->ResetCampaignProgressForChapterForTests(TEXT("Chapter.DeepWaterStationFinale"));
	GameMode->ImportDay1CompleteState(false);

	const FHorrorObjectiveTrackerSnapshot Tracker = GameMode->BuildObjectiveTrackerSnapshot();
	TestEqual(TEXT("Day1 map should keep the found-footage opening tracker before Day1 completion."), Tracker.Stage, EHorrorObjectiveTrackerStage::RecoverBodycam);
	TestTrue(TEXT("Day1 tracker should not leak the preloaded campaign chapter before Day1 completion."), Tracker.ActiveChapterId.IsNone());
	TestTrue(TEXT("Day1 tracker should continue to mention the bodycam objective before campaign starts."), Tracker.PrimaryInstruction.ToString().Contains(TEXT("随身摄像机")));
	TestFalse(TEXT("Campaign HUD exposure should stay disabled until the Day1 found-footage flow is complete."), GameMode->ShouldExposeCampaignObjectivesToHUD());

	GameMode->ImportDay1CompleteState(true);
	TestTrue(TEXT("Campaign HUD exposure should remain available after the Day1 finale completes."), GameMode->ShouldExposeCampaignObjectivesToHUD());
	FHorrorObjectiveTrackerSnapshot CampaignTrackerAfterDay1 = GameMode->BuildObjectiveTrackerSnapshot();
	TestEqual(TEXT("Completed Day1 should not hide the active campaign objective tracker."), CampaignTrackerAfterDay1.Stage, EHorrorObjectiveTrackerStage::CampaignObjective);
	TestEqual(TEXT("Completed Day1 campaign tracker should keep the active chapter id."), CampaignTrackerAfterDay1.ActiveChapterId, FName(TEXT("Chapter.DeepWaterStationFinale")));

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorObjectiveTrackerHidesPreloadedCampaignActorsDuringDay1FlowTest,
	"HorrorProject.Game.Objectives.HidesPreloadedCampaignActorsDuringDay1Flow",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorObjectiveTrackerHidesPreloadedCampaignActorsDuringDay1FlowTest::RunTest(const FString& Parameters)
{
	const FHorrorCampaignChapterDefinition* DeepWaterChapter = FHorrorCampaign::FindChapterById(TEXT("Chapter.DeepWaterStationFinale"));
	TestNotNull(TEXT("Deep Water campaign chapter should exist for Day1 exposure gating coverage."), DeepWaterChapter);
	if (!DeepWaterChapter || DeepWaterChapter->Objectives.IsEmpty())
	{
		return false;
	}

	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for campaign actor exposure gating."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	TestTrue(TEXT("Transient world should create the Horror game mode."), World->SetGameMode(FURL()));
	AHorrorGameModeBase* GameMode = World->GetAuthGameMode<AHorrorGameModeBase>();
	AHorrorCampaignObjectiveActor* ObjectiveActor = World->SpawnActor<AHorrorCampaignObjectiveActor>();
	TestNotNull(TEXT("Campaign actor exposure test should expose the game mode."), GameMode);
	TestNotNull(TEXT("Campaign actor exposure test should spawn a preloaded campaign objective actor."), ObjectiveActor);
	if (!GameMode || !ObjectiveActor)
	{
		TestTrue(TEXT("Transient world should be destroyed cleanly after setup failure."), TestWorld.DestroyTestWorld(false));
		return false;
	}

	FString ErrorMessage;
	GameMode->InitGame(TEXT("/Game/DeepWaterStation/Maps/DemoMap_VerticalSlice_Day1"), TEXT(""), ErrorMessage);
	GameMode->ResetCampaignProgressForChapterForTests(DeepWaterChapter->ChapterId);
	GameMode->ImportDay1CompleteState(false);
	ObjectiveActor->ConfigureObjective(DeepWaterChapter->ChapterId, DeepWaterChapter->Objectives[0]);
	ObjectiveActor->RefreshObjectiveState();

	TestFalse(TEXT("Day1 found-footage flow should hide campaign HUD exposure."), GameMode->ShouldExposeCampaignObjectivesToHUD());
	TestFalse(TEXT("Preloaded campaign objective actor should not be available during the Day1 found-footage flow."), ObjectiveActor->IsAvailableForInteraction());
	TestFalse(TEXT("Preloaded campaign objective actor should not be presented in the world during the Day1 found-footage flow."), ObjectiveActor->IsPresentationVisible());
	TestEqual(
		TEXT("Hidden preloaded campaign objective should not remain traceable by interaction bounds."),
		ObjectiveActor->GetInteractionBoundsForTests()->GetCollisionEnabled(),
		ECollisionEnabled::NoCollision);

	GameMode->ImportDay1CompleteState(true);
	ObjectiveActor->RefreshObjectiveState();
	TestTrue(TEXT("Campaign HUD exposure should unlock after Day1 completion."), GameMode->ShouldExposeCampaignObjectivesToHUD());
	TestTrue(TEXT("Preloaded campaign objective actor should become available after Day1 completion."), ObjectiveActor->IsAvailableForInteraction());
	TestTrue(TEXT("Preloaded campaign objective actor should become visible after Day1 completion."), ObjectiveActor->IsPresentationVisible());

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorObjectiveTrackerExposesOnlyCurrentStoryCampaignActorTest,
	"HorrorProject.Game.Objectives.ExposesOnlyCurrentStoryCampaignActor",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorObjectiveTrackerExposesOnlyCurrentStoryCampaignActorTest::RunTest(const FString& Parameters)
{
	const FHorrorCampaignChapterDefinition* DeepWaterChapter = FHorrorCampaign::FindChapterById(TEXT("Chapter.DeepWaterStationFinale"));
	TestNotNull(TEXT("Deep Water campaign chapter should exist for current story actor exposure coverage."), DeepWaterChapter);
	if (!DeepWaterChapter)
	{
		return false;
	}

	const FHorrorCampaignObjectiveDefinition* CurrentObjective = FHorrorCampaign::FindObjectiveById(
		*DeepWaterChapter,
		TEXT("DeepWater.BootDryDock"));
	const FHorrorCampaignObjectiveDefinition* OptionalObjective = FHorrorCampaign::FindObjectiveById(
		*DeepWaterChapter,
		TEXT("DeepWater.ReviewFloodLog"));
	TestNotNull(TEXT("Current story objective should exist."), CurrentObjective);
	TestNotNull(TEXT("Optional flood-log objective should exist."), OptionalObjective);
	if (!CurrentObjective || !OptionalObjective)
	{
		return false;
	}

	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for current story actor exposure coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	TestTrue(TEXT("Transient world should create the Horror game mode."), World->SetGameMode(FURL()));
	AHorrorGameModeBase* GameMode = World->GetAuthGameMode<AHorrorGameModeBase>();
	AHorrorCampaignObjectiveActor* CurrentActor = World->SpawnActor<AHorrorCampaignObjectiveActor>();
	AHorrorCampaignObjectiveActor* OptionalActor = World->SpawnActor<AHorrorCampaignObjectiveActor>();
	TestNotNull(TEXT("Current story actor exposure test should expose the game mode."), GameMode);
	TestNotNull(TEXT("Current story actor exposure test should spawn the current objective actor."), CurrentActor);
	TestNotNull(TEXT("Current story actor exposure test should spawn the optional objective actor."), OptionalActor);
	if (!GameMode || !CurrentActor || !OptionalActor)
	{
		TestTrue(TEXT("Transient world should be destroyed cleanly after setup failure."), TestWorld.DestroyTestWorld(false));
		return false;
	}

	GameMode->ResetCampaignProgressForChapterForTests(DeepWaterChapter->ChapterId);
	GameMode->ImportDay1CompleteState(true);
	CurrentActor->ConfigureObjective(DeepWaterChapter->ChapterId, *CurrentObjective);
	OptionalActor->ConfigureObjective(DeepWaterChapter->ChapterId, *OptionalObjective);
	CurrentActor->RefreshObjectiveState();
	OptionalActor->RefreshObjectiveState();

	TestTrue(TEXT("Current story objective actor should be available."), CurrentActor->IsAvailableForInteraction());
	TestTrue(TEXT("Current story objective actor should be visible."), CurrentActor->IsPresentationVisible());
	TestFalse(TEXT("Out-of-story optional actor should not expose interaction."), OptionalActor->IsAvailableForInteraction());
	TestFalse(TEXT("Out-of-story optional actor should be hidden from the world presentation."), OptionalActor->IsPresentationVisible());
	TestEqual(
		TEXT("Out-of-story optional actor should not remain traceable by interaction bounds."),
		OptionalActor->GetInteractionBoundsForTests()->GetCollisionEnabled(),
		ECollisionEnabled::NoCollision);

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorObjectiveTrackerKeepsFoundFootageFlowWhenNoCampaignChapterTest,
	"HorrorProject.Game.Objectives.TrackerKeepsFoundFootageFlowWhenNoCampaignChapter",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorObjectiveTrackerKeepsFoundFootageFlowWhenNoCampaignChapterTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for Day1 fallback tracker coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	TestTrue(TEXT("Transient world should create the Horror game mode."), World->SetGameMode(FURL()));
	AHorrorGameModeBase* GameMode = World->GetAuthGameMode<AHorrorGameModeBase>();
	TestNotNull(TEXT("Day1 fallback tracker test should expose the game mode."), GameMode);
	if (!GameMode)
	{
		TestTrue(TEXT("Transient world should be destroyed cleanly after GameMode creation failure."), TestWorld.DestroyTestWorld(false));
		return false;
	}

	FString ErrorMessage;
	GameMode->InitGame(TEXT("/Game/DeepWaterStation/Maps/DemoMap_VerticalSlice_Day1"), TEXT(""), ErrorMessage);
	GameMode->ImportDay1CompleteState(false);

	const FHorrorObjectiveTrackerSnapshot Tracker = GameMode->BuildObjectiveTrackerSnapshot();
	TestEqual(TEXT("Day1 map without an active campaign chapter should keep the found-footage opening tracker."), Tracker.Stage, EHorrorObjectiveTrackerStage::RecoverBodycam);
	TestTrue(TEXT("Day1 fallback tracker should not expose a campaign chapter."), Tracker.ActiveChapterId.IsNone());
	TestTrue(TEXT("Day1 fallback tracker should continue to mention the bodycam objective."), Tracker.PrimaryInstruction.ToString().Contains(TEXT("随身摄像机")));
	TestFalse(TEXT("Campaign HUD exposure should stay disabled when no campaign chapter is active."), GameMode->ShouldExposeCampaignObjectivesToHUD());

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorObjectiveTrackerSpawnsCampaignRuntimeAfterDay1CompletionTest,
	"HorrorProject.Game.Objectives.TrackerSpawnsCampaignRuntimeAfterDay1Completion",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorObjectiveTrackerLocksFutureChaptersBeforeDay1CompletionTest,
	"HorrorProject.Game.Objectives.LocksFutureChaptersBeforeDay1Completion",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorObjectiveTrackerLocksFutureChaptersBeforeDay1CompletionTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for future chapter gating coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	TestTrue(TEXT("Transient world should create the Horror game mode."), World->SetGameMode(FURL()));
	AHorrorGameModeBase* GameMode = World->GetAuthGameMode<AHorrorGameModeBase>();
	TestNotNull(TEXT("Future chapter gating test should expose the game mode."), GameMode);
	if (!GameMode)
	{
		TestTrue(TEXT("Transient world should be destroyed cleanly after GameMode creation failure."), TestWorld.DestroyTestWorld(false));
		return false;
	}

	FString ErrorMessage;
	GameMode->InitGame(TEXT("/Game/ForestOfSpikes/Levels/Level_ForestOfSpikes_Demo_Night"), TEXT(""), ErrorMessage);
	GameMode->ImportDay1CompleteState(false);
	GameMode->DispatchBeginPlay();

	TestTrue(TEXT("Directly loading a future map before Day1 completion should leave campaign chapter inactive."), GameMode->GetCurrentCampaignChapterId().IsNone());
	TestFalse(TEXT("Directly loaded future map should not expose campaign objectives before Day1 completion."), GameMode->ShouldExposeCampaignObjectivesToHUD());
	TestEqual(TEXT("Directly loaded future map should not spawn runtime campaign objectives before Day1 completion."), GameMode->GetRuntimeCampaignObjectivesForTests().Num(), 0);

	const FHorrorObjectiveTrackerSnapshot Tracker = GameMode->BuildObjectiveTrackerSnapshot();
	TestFalse(TEXT("Future-map fallback tracker should not leak a campaign objective id before Day1 completion."), Tracker.Stage == EHorrorObjectiveTrackerStage::CampaignObjective);
	TestTrue(TEXT("Future-map fallback tracker should keep campaign chapter hidden before Day1 completion."), Tracker.ActiveChapterId.IsNone());

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

bool FHorrorObjectiveTrackerSpawnsCampaignRuntimeAfterDay1CompletionTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for Day1-to-campaign runtime coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	TestTrue(TEXT("Transient world should create the Horror game mode."), World->SetGameMode(FURL()));
	AHorrorGameModeBase* GameMode = World->GetAuthGameMode<AHorrorGameModeBase>();
	TestNotNull(TEXT("Day1-to-campaign runtime test should expose the game mode."), GameMode);
	if (!GameMode)
	{
		TestTrue(TEXT("Transient world should be destroyed cleanly after GameMode creation failure."), TestWorld.DestroyTestWorld(false));
		return false;
	}

	FString ErrorMessage;
	GameMode->InitGame(TEXT("/Game/DeepWaterStation/Maps/DemoMap_VerticalSlice_Day1"), TEXT(""), ErrorMessage);
	GameMode->ImportDay1CompleteState(false);

	TestTrue(TEXT("Bodycam acquisition should start the Day1 found-footage flow."), GameMode->TryAcquireBodycam(TEXT("Evidence.Bodycam"), true));
	TestTrue(TEXT("First note should unlock anomaly framing."), GameMode->TryCollectFirstNote(TEXT("Note.Intro")));
	TestTrue(TEXT("First anomaly candidate should register before recording."), GameMode->BeginFirstAnomalyCandidate(TEXT("Evidence.Anomaly01")));
	TestTrue(TEXT("First anomaly recording should unlock archive review."), GameMode->TryRecordFirstAnomaly(true));
	TestTrue(TEXT("Archive review should unlock the Day1 exit."), GameMode->TryReviewArchive(TEXT("Archive.Terminal")));

	TestTrue(TEXT("Completing the Day1 exit should succeed."), GameMode->TryCompleteDay1(TEXT("Exit.ServiceDoor")));
	TestTrue(TEXT("Day1 should be marked complete after the exit flow."), GameMode->IsDay1Complete());
	TestTrue(TEXT("Campaign objectives should be exposed after the found-footage flow completes."), GameMode->ShouldExposeCampaignObjectivesToHUD());
	TestEqual(TEXT("Day1 completion should activate the DeepWater campaign chapter."), GameMode->GetCurrentCampaignChapterId(), FName(TEXT("Chapter.DeepWaterStationFinale")));
	TestTrue(TEXT("Day1 completion should spawn runtime campaign objective actors."), GameMode->GetRuntimeCampaignObjectivesForTests().Num() > 0);

	const FHorrorObjectiveTrackerSnapshot Tracker = GameMode->BuildObjectiveTrackerSnapshot();
	TestEqual(TEXT("Tracker should switch to campaign objectives after Day1 runtime spawn."), Tracker.Stage, EHorrorObjectiveTrackerStage::CampaignObjective);
	TestEqual(TEXT("Tracker should report the active DeepWater campaign chapter."), Tracker.ActiveChapterId, FName(TEXT("Chapter.DeepWaterStationFinale")));
	TestFalse(TEXT("Tracker should expose a concrete campaign objective id."), Tracker.ActiveObjectiveId.IsNone());

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

#endif
