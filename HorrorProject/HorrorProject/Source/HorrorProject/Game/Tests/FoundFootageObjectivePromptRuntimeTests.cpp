// Copyright Epic Games, Inc. All Rights Reserved.

#include "Game/FoundFootageObjectiveInteractable.h"

#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR

#include "Engine/World.h"
#include "Game/HorrorGameModeBase.h"
#include "GameFramework/WorldSettings.h"
#include "Misc/AutomationTest.h"
#include "Player/HorrorPlayerCharacter.h"
#include "Player/Components/InventoryComponent.h"
#include "Player/Components/NoteRecorderComponent.h"
#include "Tests/AutomationCommon.h"

namespace
{
	AHorrorGameModeBase* CreateObjectivePromptGameMode(FAutomationTestBase& Test, FTestWorldWrapper& TestWorld, UWorld*& OutWorld)
	{
		OutWorld = nullptr;
		Test.TestTrue(TEXT("Transient game world should be created for objective prompt coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
		OutWorld = TestWorld.GetTestWorld();
		if (!OutWorld)
		{
			return nullptr;
		}

		OutWorld->GetWorldSettings()->DefaultGameMode = AHorrorGameModeBase::StaticClass();
		Test.TestTrue(TEXT("Transient world should create the Day 1 game mode."), OutWorld->SetGameMode(FURL()));

		AHorrorGameModeBase* GameMode = OutWorld->GetAuthGameMode<AHorrorGameModeBase>();
		Test.TestNotNull(TEXT("Transient world should expose the Day 1 game mode."), GameMode);
		if (!GameMode)
		{
			TestWorld.DestroyTestWorld(false);
		}

		return GameMode;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFoundFootageObjectivePromptExplainsBlockedStatesTest,
	"HorrorProject.Game.FoundFootageObjective.PromptExplainsBlockedStates",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FFoundFootageObjectivePromptExplainsBlockedStatesTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	UWorld* World = nullptr;
	AHorrorGameModeBase* GameMode = CreateObjectivePromptGameMode(*this, TestWorld, World);
	if (!GameMode || !World)
	{
		return false;
	}

	AFoundFootageObjectiveInteractable* Bodycam = World->SpawnActor<AFoundFootageObjectiveInteractable>();
	AFoundFootageObjectiveInteractable* FirstNote = World->SpawnActor<AFoundFootageObjectiveInteractable>();
	AFoundFootageObjectiveInteractable* AnomalyRecorder = World->SpawnActor<AFoundFootageObjectiveInteractable>();
	AFoundFootageObjectiveInteractable* Archive = World->SpawnActor<AFoundFootageObjectiveInteractable>();
	AFoundFootageObjectiveInteractable* ExitGate = World->SpawnActor<AFoundFootageObjectiveInteractable>();
	TestNotNull(TEXT("Bodycam objective should spawn."), Bodycam);
	TestNotNull(TEXT("First note objective should spawn."), FirstNote);
	TestNotNull(TEXT("Anomaly recorder objective should spawn."), AnomalyRecorder);
	TestNotNull(TEXT("Archive objective should spawn."), Archive);
	TestNotNull(TEXT("Exit objective should spawn."), ExitGate);
	if (!Bodycam || !FirstNote || !AnomalyRecorder || !Archive || !ExitGate)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	Bodycam->Objective = EFoundFootageInteractableObjective::Bodycam;
	Bodycam->DebugLabel = FText::FromString(TEXT("取得随身摄像机"));
	FirstNote->Objective = EFoundFootageInteractableObjective::FirstNote;
	FirstNote->DebugLabel = FText::FromString(TEXT("站内笔记"));
	AnomalyRecorder->Objective = EFoundFootageInteractableObjective::FirstAnomalyRecord;
	AnomalyRecorder->DebugLabel = FText::FromString(TEXT("异常录制窗口"));
	Archive->Objective = EFoundFootageInteractableObjective::ArchiveReview;
	Archive->DebugLabel = FText::FromString(TEXT("档案终端"));
	ExitGate->Objective = EFoundFootageInteractableObjective::ExitRouteGate;
	ExitGate->DebugLabel = FText::FromString(TEXT("出口闸门"));

	TestEqual(
		TEXT("Available bodycam objective should present an action prompt."),
		Bodycam->GetInteractionPromptText(nullptr).ToString(),
		FString(TEXT("E键  取得随身摄像机")));

	TestEqual(
		TEXT("Blocked first note objective should explain that the bodycam is required."),
		FirstNote->GetInteractionPromptText(nullptr).ToString(),
		FString(TEXT("先找回随身摄像机。")));

	TestEqual(
		TEXT("Blocked anomaly recording should explain the missing framed anomaly."),
		AnomalyRecorder->GetInteractionPromptText(nullptr).ToString(),
		FString(TEXT("先对准异常点。")));

	TestEqual(
		TEXT("Blocked archive should explain that anomaly recording is required."),
		Archive->GetInteractionPromptText(nullptr).ToString(),
		FString(TEXT("先记录异常点，再使用档案终端。")));

	TestEqual(
		TEXT("Blocked exit should explain that archive review unlocks it."),
		ExitGate->GetInteractionPromptText(nullptr).ToString(),
		FString(TEXT("出口已锁定。先审查档案。")));

	TestTrue(TEXT("Bodycam objective should complete for follow-up prompt coverage."), Bodycam->TryCompleteObjective(GameMode));
	TestEqual(
		TEXT("Completed bodycam objective should explain its completed state."),
		Bodycam->GetInteractionPromptText(nullptr).ToString(),
		FString(TEXT("随身摄像机已经找回。")));

	TestTrue(TEXT("First note objective should complete so the anomaly can be framed."), FirstNote->TryCompleteObjective(GameMode));
	TestTrue(TEXT("First anomaly candidate should be framed for recording prompt coverage."), GameMode->BeginFirstAnomalyCandidate(TEXT("Evidence.Anomaly01")));
	TestFalse(TEXT("Transient prompt test should cover the non-recording camera state."), GameMode->IsLeadPlayerRecording());
	TestEqual(
		TEXT("Blocked anomaly recording with a framed candidate should tell the player to start recording and interact."),
		AnomalyRecorder->GetInteractionPromptText(nullptr).ToString(),
			FString(TEXT("先开始录制，再按 E键记录异常点。")));

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFoundFootageObjectiveArchiveReviewRecordsSummaryNoteTest,
	"HorrorProject.Game.FoundFootageObjective.ArchiveReviewRecordsSummaryNote",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FFoundFootageObjectiveArchiveReviewRecordsSummaryNoteTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	UWorld* World = nullptr;
	AHorrorGameModeBase* GameMode = CreateObjectivePromptGameMode(*this, TestWorld, World);
	if (!GameMode || !World)
	{
		return false;
	}

	AHorrorPlayerCharacter* PlayerCharacter = World->SpawnActor<AHorrorPlayerCharacter>();
	AFoundFootageObjectiveInteractable* Archive = World->SpawnActor<AFoundFootageObjectiveInteractable>();
	TestNotNull(TEXT("Archive summary test should spawn a player character."), PlayerCharacter);
	TestNotNull(TEXT("Archive summary test should spawn an archive objective."), Archive);
	if (!PlayerCharacter || !Archive)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	UInventoryComponent* Inventory = PlayerCharacter->GetInventoryComponent();
	UNoteRecorderComponent* NoteRecorder = PlayerCharacter->GetNoteRecorderComponent();
	TestNotNull(TEXT("Player should expose inventory for archive prerequisites."), Inventory);
	TestNotNull(TEXT("Player should expose note recorder for archive summary."), NoteRecorder);
	if (!Inventory || !NoteRecorder)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	TestTrue(TEXT("Bodycam state should record before archive review."), GameMode->TryAcquireBodycam(TEXT("Evidence.Bodycam"), true));
	TestTrue(TEXT("First note state should record before archive review."), GameMode->TryCollectFirstNote(TEXT("Note.Intro")));
	TestTrue(TEXT("First anomaly candidate should be framed before archive review."), GameMode->BeginFirstAnomalyCandidate(TEXT("Evidence.Anomaly01")));
	TestTrue(TEXT("First anomaly should be recorded before archive review."), GameMode->TryRecordFirstAnomaly(true));

	TestTrue(TEXT("Player inventory should contain the bodycam evidence prerequisite."), Inventory->AddCollectedEvidenceId(TEXT("Evidence.Bodycam")));
	TestTrue(TEXT("Player inventory should contain the anomaly evidence prerequisite."), Inventory->AddCollectedEvidenceId(TEXT("Evidence.Anomaly01")));
	TestTrue(TEXT("Player note recorder should contain the intro note prerequisite."), NoteRecorder->AddRecordedNoteId(TEXT("Note.Intro")));

	Archive->Objective = EFoundFootageInteractableObjective::ArchiveReview;
	Archive->SourceId = TEXT("Evidence.ArchiveTerminal");
	Archive->EvidenceMetadata.EvidenceId = TEXT("Evidence.ArchiveTerminal");
	Archive->EvidenceMetadata.DisplayName = FText::FromString(TEXT("档案终端"));
	Archive->EvidenceMetadata.Description = FText::FromString(TEXT("已恢复路线档案与闸门指令。"));

	const FHitResult EmptyHit;
	TestTrue(TEXT("Archive terminal should complete once the player has all evidence prerequisites."), Archive->Interact_Implementation(PlayerCharacter, EmptyHit));
	TestTrue(TEXT("Archive review should unlock the exit."), GameMode->IsExitUnlocked());
	TestTrue(TEXT("Archive terminal evidence should be collected."), Inventory->HasCollectedEvidenceId(TEXT("Evidence.ArchiveTerminal")));
	TestTrue(TEXT("Archive review should record a readable journal summary."), NoteRecorder->HasRecordedNoteId(TEXT("Note.ArchiveSummary")));

	FHorrorNoteMetadata SummaryMetadata;
	TestTrue(TEXT("Archive summary should register metadata for the journal UI."), NoteRecorder->GetNoteMetadata(TEXT("Note.ArchiveSummary"), SummaryMetadata));
	TestEqual(TEXT("Archive summary should keep the expected id."), SummaryMetadata.NoteId, FName(TEXT("Note.ArchiveSummary")));
	TestTrue(TEXT("Archive summary title should identify the archive terminal."), SummaryMetadata.Title.ToString().Contains(TEXT("档案")));
	TestTrue(TEXT("Archive summary should mention the recorded anomaly evidence."), SummaryMetadata.Body.ToString().Contains(TEXT("一号异常录像")));
	TestTrue(TEXT("Archive summary should preserve the exit gate code clue."), SummaryMetadata.Body.ToString().Contains(TEXT("1697")));

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFoundFootageObjectiveAnomalyCandidateRecordsHintNoteTest,
	"HorrorProject.Game.FoundFootageObjective.AnomalyCandidateRecordsHintNote",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FFoundFootageObjectiveAnomalyCandidateRecordsHintNoteTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	UWorld* World = nullptr;
	AHorrorGameModeBase* GameMode = CreateObjectivePromptGameMode(*this, TestWorld, World);
	if (!GameMode || !World)
	{
		return false;
	}

	AHorrorPlayerCharacter* PlayerCharacter = World->SpawnActor<AHorrorPlayerCharacter>();
	AFoundFootageObjectiveInteractable* FirstAnomalyCandidate = World->SpawnActor<AFoundFootageObjectiveInteractable>();
	TestNotNull(TEXT("Anomaly hint note test should spawn a player character."), PlayerCharacter);
	TestNotNull(TEXT("Anomaly hint note test should spawn an anomaly candidate objective."), FirstAnomalyCandidate);
	if (!PlayerCharacter || !FirstAnomalyCandidate)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	UNoteRecorderComponent* NoteRecorder = PlayerCharacter->GetNoteRecorderComponent();
	TestNotNull(TEXT("Player should expose note recorder for anomaly hint notes."), NoteRecorder);
	if (!NoteRecorder)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	TestTrue(TEXT("Bodycam state should record before anomaly candidate."), GameMode->TryAcquireBodycam(TEXT("Evidence.Bodycam"), true));
	TestTrue(TEXT("First note state should record before anomaly candidate."), GameMode->TryCollectFirstNote(TEXT("Note.Intro")));

	FirstAnomalyCandidate->Objective = EFoundFootageInteractableObjective::FirstAnomalyCandidate;
	FirstAnomalyCandidate->SourceId = TEXT("Evidence.Anomaly01");
	FirstAnomalyCandidate->ObjectiveHint = FText::FromString(TEXT("调查可见异常。"));
	FirstAnomalyCandidate->DebugLabel = FText::FromString(TEXT("第一个异常目标"));

	const FName HintNoteId(TEXT("Note.ObjectiveHint.Evidence.Anomaly01"));
	const int32 NoteCountBeforeCandidate = NoteRecorder->GetRecordedNoteCount();
	const FHitResult EmptyHit;
	TestTrue(TEXT("Anomaly candidate should complete after bodycam and first note."), FirstAnomalyCandidate->Interact_Implementation(PlayerCharacter, EmptyHit));
	TestTrue(TEXT("Anomaly candidate should record its objective hint into the journal."), NoteRecorder->HasRecordedNoteId(HintNoteId));
	TestEqual(TEXT("Anomaly candidate hint should add one journal entry."), NoteRecorder->GetRecordedNoteCount(), NoteCountBeforeCandidate + 1);

	FHorrorNoteMetadata HintMetadata;
	TestTrue(TEXT("Anomaly candidate hint should register journal metadata."), NoteRecorder->GetNoteMetadata(HintNoteId, HintMetadata));
	TestEqual(TEXT("Anomaly candidate hint title should come from the debug label."), HintMetadata.Title.ToString(), FString(TEXT("第一个异常目标")));
	TestEqual(TEXT("Anomaly candidate hint body should come from the objective hint."), HintMetadata.Body.ToString(), FString(TEXT("调查可见异常。")));

	TestFalse(TEXT("Duplicate anomaly candidate interaction should no-op."), FirstAnomalyCandidate->Interact_Implementation(PlayerCharacter, EmptyHit));
	TestEqual(TEXT("Duplicate anomaly candidate interaction should not add another hint note."), NoteRecorder->GetRecordedNoteCount(), NoteCountBeforeCandidate + 1);

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

#endif
