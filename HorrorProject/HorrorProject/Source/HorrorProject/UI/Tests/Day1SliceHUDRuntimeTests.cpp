// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/Day1SliceHUD.h"

#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR

#include "Game/HorrorAdvancedInteractionTypes.h"
#include "Game/HorrorFoundFootageContract.h"
#include "Misc/AutomationTest.h"
#include "Player/Components/NoteRecorderComponent.h"
#include "Tests/AutomationCommon.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDay1SliceHUDStoresNativeFallbackStateTest,
	"HorrorProject.UI.Day1SliceHUD.StoresNativeFallbackState",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FDay1SliceHUDStoresNativeFallbackStateTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for Day1 HUD coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	ADay1SliceHUD* HUD = World->SpawnActor<ADay1SliceHUD>();
	TestNotNull(TEXT("Day1 HUD test should spawn the native fallback HUD."), HUD);
	if (!HUD)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	const FText Objective = FText::FromString(TEXT("找回随身摄像机。"));
	HUD->SetCurrentObjective(Objective);
	TestEqual(TEXT("Day1 HUD should store the current objective."), HUD->GetCurrentObjectiveForTests().ToString(), Objective.ToString());

	const FText Prompt = FText::FromString(TEXT("E  打开门"));
	HUD->SetInteractionPrompt(Prompt);
	TestEqual(TEXT("Day1 HUD should store the interaction prompt."), HUD->GetInteractionPromptForTests().ToString(), Prompt.ToString());

	HUD->ClearInteractionPrompt();
	TestTrue(TEXT("Day1 HUD should clear the interaction prompt."), HUD->GetInteractionPromptForTests().IsEmpty());

	HUD->ShowObjectiveToast(FText::FromString(TEXT("取得随身摄像机")), FText::FromString(TEXT("找到并阅读第一份站内笔记。")));
	TestEqual(
		TEXT("Objective toasts should update the persistent objective fallback."),
		HUD->GetCurrentObjectiveForTests().ToString(),
		FString(TEXT("找到并阅读第一份站内笔记。")));

	HUD->SetObjectiveNavigation(FText::FromString(TEXT("2 m ahead")));
	TestEqual(
		TEXT("Day1 HUD should store objective navigation feedback."),
		HUD->GetObjectiveNavigationForTests().ToString(),
		FString(TEXT("2 m ahead")));
	HUD->ClearObjectiveNavigation();
	TestTrue(TEXT("Day1 HUD should clear objective navigation feedback."), HUD->GetObjectiveNavigationForTests().IsEmpty());

	FHorrorObjectiveTrackerSnapshot ObjectiveTracker;
	ObjectiveTracker.Stage = EHorrorObjectiveTrackerStage::RecordFirstAnomaly;
	ObjectiveTracker.Title = FText::FromString(TEXT("记录异常"));
	ObjectiveTracker.PrimaryInstruction = FText::FromString(TEXT("保持录像，对准异常。"));
	ObjectiveTracker.SecondaryInstruction = FText::FromString(TEXT("稳定取景直到信号锁定。"));
	ObjectiveTracker.ProgressLabel = FText::FromString(TEXT("进度 2/5"));
	FHorrorObjectiveChecklistItem ChecklistItem;
	ChecklistItem.Stage = EHorrorObjectiveTrackerStage::RecordFirstAnomaly;
	ChecklistItem.Label = FText::FromString(TEXT("录像锁定第一个异常"));
	ChecklistItem.bActive = true;
	ChecklistItem.bRequiresRecording = true;
	ObjectiveTracker.ChecklistItems.Add(ChecklistItem);
	ObjectiveTracker.CompletedMilestoneCount = 2;
	ObjectiveTracker.RequiredMilestoneCount = 5;
	ObjectiveTracker.ProgressFraction = 0.4f;
	ObjectiveTracker.bRequiresRecording = true;
	ObjectiveTracker.bUrgent = true;
	HUD->SetObjectiveTracker(ObjectiveTracker);
	TestEqual(TEXT("Day1 HUD should store the tracker title."), HUD->GetObjectiveTrackerTitleForTests().ToString(), FString(TEXT("记录异常")));
	TestEqual(TEXT("Day1 HUD should promote the tracker primary instruction."), HUD->GetCurrentObjectiveForTests().ToString(), FString(TEXT("保持录像，对准异常。")));
	TestEqual(TEXT("Day1 HUD should store the tracker secondary instruction."), HUD->GetObjectiveTrackerDetailForTests().ToString(), FString(TEXT("稳定取景直到信号锁定。")));
	TestEqual(TEXT("Day1 HUD should store the tracker progress label."), HUD->GetObjectiveTrackerProgressLabelForTests().ToString(), FString(TEXT("进度 2/5")));
	TestTrue(TEXT("Day1 HUD should expose urgent objective state."), HUD->IsObjectiveTrackerUrgentForTests());
	TestTrue(TEXT("Day1 HUD should expose recording requirement state."), HUD->IsObjectiveTrackerRecordingRequiredForTests());
	TestTrue(TEXT("Day1 HUD should store objective progress fraction."), FMath::IsNearlyEqual(HUD->GetObjectiveTrackerProgressFractionForTests(), 0.4f));
	TestEqual(TEXT("Day1 HUD should store objective checklist items."), HUD->GetObjectiveChecklistItemCountForTests(), 1);
	TestEqual(TEXT("Day1 HUD should expose objective checklist labels."), HUD->GetObjectiveChecklistItemLabelForTests(0).ToString(), FString(TEXT("录像锁定第一个异常")));
	TestTrue(TEXT("Day1 HUD should expose the active checklist row."), HUD->IsObjectiveChecklistItemActiveForTests(0));
	TestTrue(TEXT("Day1 HUD should expose checklist recording requirements."), HUD->DoesObjectiveChecklistItemRequireRecordingForTests(0));

	HUD->SetAnomalyCaptureStatus(FText::FromString(TEXT("异常已对准，开启录像锁定。")), 0.35f, false, true);
	TestTrue(TEXT("Day1 HUD should expose anomaly capture status visibility."), HUD->IsAnomalyCaptureStatusVisibleForTests());
	TestEqual(TEXT("Day1 HUD should store anomaly capture status text."), HUD->GetAnomalyCaptureStatusForTests().ToString(), FString(TEXT("异常已对准，开启录像锁定。")));
	TestTrue(TEXT("Day1 HUD should store anomaly capture progress fraction."), FMath::IsNearlyEqual(HUD->GetAnomalyCaptureProgressForTests(), 0.35f));
	TestFalse(TEXT("Day1 HUD should expose capture locked state."), HUD->IsAnomalyCaptureLockedForTests());
	TestTrue(TEXT("Day1 HUD should expose capture recording requirement state."), HUD->DoesAnomalyCaptureRequireRecordingForTests());
	HUD->ClearAnomalyCaptureStatus();
	TestFalse(TEXT("Day1 HUD should clear anomaly capture status visibility."), HUD->IsAnomalyCaptureStatusVisibleForTests());

	FHorrorAdvancedInteractionHUDState CircuitPanel;
	CircuitPanel.bVisible = true;
	CircuitPanel.Mode = EHorrorCampaignInteractionMode::CircuitWiring;
	CircuitPanel.Title = FText::FromString(TEXT("修复深水站主供电"));
	CircuitPanel.ExpectedInputId = TEXT("蓝色端子");
	CircuitPanel.FeedbackText = FText::FromString(TEXT("蓝色电弧：线路接入成功。"));
	CircuitPanel.ProgressFraction = 0.48f;
	CircuitPanel.TimingFraction = 0.58f;
	CircuitPanel.PauseRemainingSeconds = 0.0f;
	CircuitPanel.bTimingWindowOpen = true;
	CircuitPanel.bPaused = false;
	CircuitPanel.StepIndex = 2;
	CircuitPanel.RequiredStepCount = 3;
	HUD->SetAdvancedInteractionState(CircuitPanel);
	TestTrue(TEXT("Day1 HUD should expose advanced interaction panel visibility."), HUD->IsAdvancedInteractionPanelVisibleForTests());
	TestEqual(TEXT("Day1 HUD should store the advanced interaction mode."), HUD->GetAdvancedInteractionModeForTests(), EHorrorCampaignInteractionMode::CircuitWiring);
	TestEqual(TEXT("Day1 HUD should store the advanced panel title."), HUD->GetAdvancedInteractionTitleForTests().ToString(), FString(TEXT("修复深水站主供电")));
	TestEqual(TEXT("Day1 HUD should store the expected animated input id."), HUD->GetAdvancedInteractionExpectedInputForTests(), FName(TEXT("蓝色端子")));
	TestEqual(TEXT("Day1 HUD should store advanced interaction feedback."), HUD->GetAdvancedInteractionFeedbackForTests().ToString(), FString(TEXT("蓝色电弧：线路接入成功。")));
	TestTrue(TEXT("Day1 HUD should store advanced interaction progress."), FMath::IsNearlyEqual(HUD->GetAdvancedInteractionProgressForTests(), 0.48f));
	TestTrue(TEXT("Day1 HUD should store advanced interaction timing cursor."), FMath::IsNearlyEqual(HUD->GetAdvancedInteractionTimingForTests(), 0.58f));
	TestTrue(TEXT("Day1 HUD should expose whether the timing window is open."), HUD->IsAdvancedInteractionTimingWindowOpenForTests());
	TestFalse(TEXT("Day1 HUD should expose paused state for animated devices."), HUD->IsAdvancedInteractionPausedForTests());
	TestEqual(TEXT("Day1 HUD should store the advanced interaction step index."), HUD->GetAdvancedInteractionStepIndexForTests(), 2);
	TestEqual(TEXT("Day1 HUD should store the required animated step count."), HUD->GetAdvancedInteractionRequiredStepCountForTests(), 3);

	HUD->ClearAdvancedInteractionState();
	TestFalse(TEXT("Day1 HUD should clear the advanced interaction panel."), HUD->IsAdvancedInteractionPanelVisibleForTests());

	HUD->ShowPasswordPrompt(
		FText::FromString(TEXT("Door_0417")),
		FText::FromString(TEXT("备忘录上的前四个数字。")),
		2,
		4);
	TestTrue(TEXT("Day1 HUD should expose password prompt visibility for runtime tests."), HUD->IsPasswordPromptVisibleForTests());

	HUD->ClearPasswordPrompt();
	TestFalse(TEXT("Day1 HUD should clear password prompt visibility."), HUD->IsPasswordPromptVisibleForTests());

	HUD->ShowPauseMenu(EDay1PauseMenuSelection::MasterVolume, 1.2f, 0.75f, 1.1f);
	TestTrue(TEXT("Day1 HUD should expose pause menu visibility for runtime tests."), HUD->IsPauseMenuVisibleForTests());
	TestEqual(TEXT("Day1 HUD should store pause menu selection."), HUD->GetPauseMenuSelectionForTests(), EDay1PauseMenuSelection::MasterVolume);

	HUD->ClearPauseMenu();
	TestFalse(TEXT("Day1 HUD should clear pause menu visibility."), HUD->IsPauseMenuVisibleForTests());

	HUD->ShowAutosaveIndicator(FText::FromString(TEXT("检查点已保存。")));
	TestTrue(TEXT("Day1 HUD should expose autosave indicator visibility for runtime tests."), HUD->IsAutosaveIndicatorVisibleForTests());
	TestEqual(TEXT("Day1 HUD should store the autosave indicator text."), HUD->GetAutosaveIndicatorTextForTests().ToString(), FString(TEXT("检查点已保存。")));

	HUD->SetBodycamBatteryStatus(18.5f, true);
	TestTrue(TEXT("Day1 HUD should expose bodycam battery visibility for runtime tests."), HUD->IsBodycamBatteryVisibleForTests());
	TestEqual(TEXT("Day1 HUD should clamp and store bodycam battery percentage."), HUD->GetBodycamBatteryPercentForTests(), 18.5f);
	TestTrue(TEXT("Day1 HUD should expose low battery state."), HUD->IsBodycamBatteryLowForTests());

	HUD->SetBodycamBatteryStatus(140.0f, false);
	TestEqual(TEXT("Day1 HUD should clamp battery overcharge to 100 percent."), HUD->GetBodycamBatteryPercentForTests(), 100.0f);
	TestFalse(TEXT("Day1 HUD should clear low battery state when the source is healthy."), HUD->IsBodycamBatteryLowForTests());

	HUD->ClearBodycamBatteryStatus();
	TestFalse(TEXT("Day1 HUD should clear bodycam battery visibility."), HUD->IsBodycamBatteryVisibleForTests());

	HUD->ShowDay1CompletionOverlay(
		FText::FromString(TEXT("第 1 天完成")),
		FText::FromString(TEXT("证据已保全。画面切黑。")));
	TestTrue(TEXT("Day1 HUD should expose completion overlay visibility for runtime tests."), HUD->IsDay1CompletionOverlayVisibleForTests());
	TestEqual(TEXT("Day1 HUD should store the completion overlay title."), HUD->GetDay1CompletionTitleForTests().ToString(), FString(TEXT("第 1 天完成")));
	HUD->ClearDay1CompletionOverlay();
	TestFalse(TEXT("Day1 HUD should clear completion overlay visibility."), HUD->IsDay1CompletionOverlayVisibleForTests());

	TestFalse(TEXT("Day1 journal should start hidden."), HUD->IsNotesJournalVisibleForTests());
	HUD->ShowNotesJournal({});
	TestTrue(TEXT("Day1 journal should become visible for empty native fallback state."), HUD->IsNotesJournalVisibleForTests());
	TestTrue(TEXT("Day1 journal should expose empty note state."), HUD->IsNotesJournalEmptyForTests());
	TestEqual(TEXT("Day1 journal should use an empty fallback title."), HUD->GetNotesJournalEmptyTextForTests().ToString(), FString(TEXT("还没有记录任何笔记。")));

	FHorrorNoteMetadata StationMemo;
	StationMemo.NoteId = TEXT("Note.StationMemo");
	StationMemo.Title = FText::FromString(TEXT("站内备忘录"));
	StationMemo.Body = FText::FromString(TEXT("上次下潜后门禁密码已更改。"));
	HUD->ShowNotesJournal({ StationMemo });
	TestTrue(TEXT("Day1 journal should remain visible after note entries are supplied."), HUD->IsNotesJournalVisibleForTests());
	TestFalse(TEXT("Day1 journal should no longer report empty state when note entries exist."), HUD->IsNotesJournalEmptyForTests());
	TestEqual(TEXT("Day1 journal should expose recorded note count."), HUD->GetNotesJournalEntryCountForTests(), 1);
	TestEqual(TEXT("Day1 journal should expose the first note title."), HUD->GetNotesJournalTitleForTests(0).ToString(), FString(TEXT("站内备忘录")));
	TestEqual(TEXT("Day1 journal should expose the first note body."), HUD->GetNotesJournalBodyForTests(0).ToString(), FString(TEXT("上次下潜后门禁密码已更改。")));

	HUD->ClearNotesJournal();
	TestFalse(TEXT("Day1 journal should clear visibility."), HUD->IsNotesJournalVisibleForTests());

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

#endif
