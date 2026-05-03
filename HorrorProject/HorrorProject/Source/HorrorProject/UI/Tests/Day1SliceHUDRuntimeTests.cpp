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

	FHorrorInteractionContext InteractionContext;
	InteractionContext.bVisible = true;
	InteractionContext.bCanInteract = false;
	InteractionContext.bRequiresRecording = true;
	InteractionContext.bOpensPanel = false;
	InteractionContext.Verb = EHorrorInteractionVerb::Record;
	InteractionContext.InputStyle = EHorrorInteractionInputStyle::RecordAim;
	InteractionContext.RiskLevel = EHorrorInteractionRiskLevel::High;
	InteractionContext.ActionText = FText::FromString(TEXT("记录异常"));
	InteractionContext.BlockedReason = FText::FromString(TEXT("先开启录像，再按互动键记录异常。"));
	InteractionContext.InputText = FText::FromString(TEXT("开启录像并保持对准"));
	InteractionContext.TargetId = TEXT("Evidence.Anomaly01");
	HUD->SetInteractionContext(InteractionContext);
	TestTrue(TEXT("Day1 HUD should expose structured interaction context visibility."), HUD->IsInteractionContextVisibleForTests());
	TestFalse(TEXT("Day1 HUD should expose blocked interaction state."), HUD->CanFocusedInteractionForTests());
	TestTrue(TEXT("Day1 HUD should expose recording requirements for focused interactions."), HUD->DoesFocusedInteractionRequireRecordingForTests());
	TestEqual(TEXT("Day1 HUD should store the structured interaction verb."), HUD->GetFocusedInteractionVerbForTests(), EHorrorInteractionVerb::Record);
	TestEqual(TEXT("Day1 HUD should store the structured input style."), HUD->GetFocusedInteractionInputStyleForTests(), EHorrorInteractionInputStyle::RecordAim);
	TestEqual(TEXT("Day1 HUD should store the structured risk level."), HUD->GetFocusedInteractionRiskLevelForTests(), EHorrorInteractionRiskLevel::High);
	TestEqual(TEXT("Day1 HUD should store structured interaction action text."), HUD->GetFocusedInteractionActionForTests().ToString(), FString(TEXT("记录异常")));
	TestEqual(TEXT("Day1 HUD should promote blocked reason into the legacy prompt field."), HUD->GetInteractionPromptForTests().ToString(), FString(TEXT("先开启录像，再按互动键记录异常。")));
	TestEqual(TEXT("Day1 HUD should store structured input text."), HUD->GetFocusedInteractionInputTextForTests().ToString(), FString(TEXT("开启录像并保持对准")));
	TestEqual(TEXT("Day1 HUD should store structured target id."), HUD->GetFocusedInteractionTargetIdForTests(), FName(TEXT("Evidence.Anomaly01")));
	HUD->ClearInteractionPrompt();
	TestFalse(TEXT("Day1 HUD should clear structured interaction context visibility."), HUD->IsInteractionContextVisibleForTests());

	HUD->ShowObjectiveToast(FText::FromString(TEXT("取得随身摄像机")), FText::FromString(TEXT("找到并阅读第一份站内笔记。")));
	TestEqual(TEXT("Default objective toast severity should remain informational."), HUD->GetObjectiveToastSeverityForTests(), EHorrorObjectiveFeedbackSeverity::Info);
	TestEqual(
		TEXT("Objective toasts should update the persistent objective fallback."),
		HUD->GetCurrentObjectiveForTests().ToString(),
		FString(TEXT("找到并阅读第一份站内笔记。")));
	HUD->ShowObjectiveToast(
		FText::FromString(TEXT("追逐失败")),
		FText::FromString(TEXT("重新互动可再次尝试。")),
		6.5f,
		EHorrorObjectiveFeedbackSeverity::Failure);
	TestEqual(TEXT("Objective toast should store failure severity for visual treatment."), HUD->GetObjectiveToastSeverityForTests(), EHorrorObjectiveFeedbackSeverity::Failure);

	HUD->SetObjectiveNavigation(FText::FromString(TEXT("2 m ahead")));
	TestEqual(
		TEXT("Day1 HUD should store objective navigation feedback."),
		HUD->GetObjectiveNavigationForTests().ToString(),
		FString(TEXT("2 m ahead")));
	FHorrorObjectiveNavigationState NavigationState;
	NavigationState.bVisible = true;
	NavigationState.Label = FText::FromString(TEXT("档案终端"));
	NavigationState.DirectionText = FText::FromString(TEXT("前方"));
	NavigationState.StatusText = FText::FromString(TEXT("前方 18 米"));
	NavigationState.WorldLocation = FVector(1800.0f, 0.0f, 95.0f);
	NavigationState.DistanceMeters = 18.0f;
	NavigationState.AngleDegrees = 8.0f;
	NavigationState.bArrived = false;
	NavigationState.bReachable = true;
	NavigationState.bRetryable = true;
	NavigationState.RuntimeStatus = EHorrorCampaignObjectiveRuntimeStatus::FailedRetryable;
	NavigationState.DeviceStatusLabel = FText::FromString(TEXT("逃生窗口：紧张"));
	NavigationState.NextActionLabel = FText::FromString(TEXT("冲刺 18 米，预计 4 秒到达，余量 3 秒"));
	NavigationState.FailureRecoveryLabel = FText::FromString(TEXT("返回追逐起点后重新尝试"));
	NavigationState.PerformanceGradeFraction = 1.4f;
	NavigationState.EscapeTimeBudgetSeconds = 3.2f;
	NavigationState.EstimatedEscapeArrivalSeconds = -1.0f;
	NavigationState.FailureCause = TEXT("Failure.Campaign.PursuitTimeout");
	NavigationState.RecoveryAction = TEXT("Recovery.Campaign.ReturnToEscapeStart");
	HUD->SetObjectiveNavigationState(NavigationState);
	TestTrue(TEXT("Day1 HUD should expose structured objective navigation visibility."), HUD->IsObjectiveNavigationVisibleForTests());
	TestEqual(TEXT("Day1 HUD should store structured navigation label."), HUD->GetObjectiveNavigationLabelForTests().ToString(), FString(TEXT("档案终端")));
	TestEqual(TEXT("Day1 HUD should store structured navigation direction."), HUD->GetObjectiveNavigationDirectionForTests().ToString(), FString(TEXT("前方")));
	TestTrue(TEXT("Day1 HUD should store structured navigation distance."), FMath::IsNearlyEqual(HUD->GetObjectiveNavigationDistanceMetersForTests(), 18.0f));
	TestTrue(TEXT("Day1 HUD should expose structured navigation reachability."), HUD->IsObjectiveNavigationReachableForTests());
	TestTrue(TEXT("Day1 HUD should expose retryable navigation state."), HUD->IsObjectiveNavigationRetryableForTests());
	TestEqual(TEXT("Day1 HUD should store runtime navigation status."), HUD->GetObjectiveNavigationRuntimeStatusForTests(), EHorrorCampaignObjectiveRuntimeStatus::FailedRetryable);
	TestEqual(TEXT("Day1 HUD should store navigation device diagnostics."), HUD->GetObjectiveNavigationDeviceStatusForTests().ToString(), FString(TEXT("逃生窗口：紧张")));
	TestEqual(TEXT("Day1 HUD should store navigation next-action cue."), HUD->GetObjectiveNavigationNextActionForTests().ToString(), FString(TEXT("冲刺 18 米，预计 4 秒到达，余量 3 秒")));
	TestEqual(TEXT("Day1 HUD should store navigation recovery cue."), HUD->GetObjectiveNavigationFailureRecoveryForTests().ToString(), FString(TEXT("返回追逐起点后重新尝试")));
	TestTrue(TEXT("Day1 HUD should build visible navigation diagnostic text."), HUD->GetObjectiveNavigationDiagnosticTextForTests().ToString().Contains(TEXT("逃生窗口：紧张")));
	TestTrue(TEXT("Day1 HUD navigation diagnostics should include the sprint cue."), HUD->GetObjectiveNavigationDiagnosticTextForTests().ToString().Contains(TEXT("冲刺 18 米")));
	TestTrue(TEXT("Day1 HUD retry navigation diagnostics should include recovery cue."), HUD->GetObjectiveNavigationDiagnosticTextForTests().ToString().Contains(TEXT("返回追逐起点")));
	const FString NavigationRouteText = HUD->GetObjectiveNavigationRouteTextForTests().ToString();
	TestTrue(TEXT("Day1 HUD route card should surface the destination label."), NavigationRouteText.Contains(TEXT("档案终端")));
	TestTrue(TEXT("Day1 HUD route card should surface the direction label."), NavigationRouteText.Contains(TEXT("前方")));
	TestTrue(TEXT("Day1 HUD route card should surface the signed heading angle."), NavigationRouteText.Contains(TEXT("8°")));
	TestTrue(TEXT("Day1 HUD route card should surface the meter distance."), NavigationRouteText.Contains(TEXT("18 米")));
	TestTrue(TEXT("Day1 HUD route card should surface retryability."), NavigationRouteText.Contains(TEXT("可重试")));
	TestTrue(TEXT("Day1 HUD route card should surface the next action cue."), NavigationRouteText.Contains(TEXT("冲刺 18 米")));
	TestEqual(TEXT("Day1 HUD should clamp navigation performance grade."), HUD->GetObjectiveNavigationPerformanceGradeForTests(), 1.0f);
	TestTrue(TEXT("Day1 HUD should preserve remaining escape budget telemetry."), FMath::IsNearlyEqual(HUD->GetObjectiveNavigationEscapeBudgetForTests(), 3.2f));
	TestEqual(TEXT("Day1 HUD should clamp negative estimated escape arrival telemetry."), HUD->GetObjectiveNavigationEstimatedEscapeArrivalForTests(), 0.0f);
	TestEqual(TEXT("Day1 HUD should store navigation failure cause."), HUD->GetObjectiveNavigationFailureCauseForTests(), FName(TEXT("Failure.Campaign.PursuitTimeout")));
	TestEqual(TEXT("Day1 HUD should store navigation recovery action."), HUD->GetObjectiveNavigationRecoveryActionForTests(), FName(TEXT("Recovery.Campaign.ReturnToEscapeStart")));
	FHorrorObjectiveNavigationState BlockedNavigationState = NavigationState;
	BlockedNavigationState.bReachable = false;
	BlockedNavigationState.bRetryable = false;
	BlockedNavigationState.RuntimeStatus = EHorrorCampaignObjectiveRuntimeStatus::Locked;
	HUD->SetObjectiveNavigationState(BlockedNavigationState);
	TestTrue(
		TEXT("Day1 HUD route card should warn when the active destination is not reachable."),
		HUD->GetObjectiveNavigationRouteTextForTests().ToString().Contains(TEXT("不可达")));
	HUD->ClearObjectiveNavigation();
	TestTrue(TEXT("Day1 HUD should clear objective navigation feedback."), HUD->GetObjectiveNavigationForTests().IsEmpty());
	TestFalse(TEXT("Day1 HUD should clear structured objective navigation feedback."), HUD->IsObjectiveNavigationVisibleForTests());
	TestTrue(TEXT("Day1 HUD should clear the route-card text with navigation state."), HUD->GetObjectiveNavigationRouteTextForTests().IsEmpty());

	FHorrorObjectiveTrackerSnapshot ObjectiveTracker;
	ObjectiveTracker.Stage = EHorrorObjectiveTrackerStage::RecordFirstAnomaly;
	ObjectiveTracker.Title = FText::FromString(TEXT("记录异常"));
	ObjectiveTracker.PrimaryInstruction = FText::FromString(TEXT("保持录像，对准异常。"));
	ObjectiveTracker.SecondaryInstruction = FText::FromString(TEXT("稳定取景直到信号锁定。"));
	ObjectiveTracker.ProgressLabel = FText::FromString(TEXT("进度 2/5"));
	ObjectiveTracker.InteractionLabel = FText::FromString(TEXT("录像瞄准"));
	ObjectiveTracker.MissionContext = FText::FromString(TEXT("异常需要被镜头稳定捕捉。"));
	ObjectiveTracker.FailureStakes = FText::FromString(TEXT("未录像会导致证据链中断。"));
	ObjectiveTracker.ActiveChapterId = TEXT("DeepWater");
	ObjectiveTracker.ActiveObjectiveId = TEXT("RecordAnomaly");
	FHorrorObjectiveChecklistItem ChecklistItem;
	ChecklistItem.Stage = EHorrorObjectiveTrackerStage::RecordFirstAnomaly;
	ChecklistItem.Label = FText::FromString(TEXT("录像锁定第一个异常"));
	ChecklistItem.Detail = FText::FromString(TEXT("保持画面中心稳定。"));
	ChecklistItem.InteractionLabel = FText::FromString(TEXT("录像瞄准"));
	ChecklistItem.StatusText = FText::FromString(TEXT("等待录像锁定"));
	ChecklistItem.TacticalLabel = FText::FromString(TEXT("玩法：接线  |  判定：交互窗口  |  失败：重试阶段"));
	ChecklistItem.DeviceStatusLabel = FText::FromString(TEXT("端子同步中"));
	ChecklistItem.NextActionLabel = FText::FromString(TEXT("按 A 接入蓝色端子"));
	ChecklistItem.FailureRecoveryLabel = FText::FromString(TEXT("松手等待残压释放，再重新接入"));
	ChecklistItem.ObjectiveId = TEXT("RecordAnomaly");
	ChecklistItem.BeatId = TEXT("Beat.RecordFirstAnomaly");
	ChecklistItem.BeatIndex = 1;
	ChecklistItem.RuntimeStatus = EHorrorCampaignObjectiveRuntimeStatus::AdvancedInteractionActive;
	ChecklistItem.FailureCause = TEXT("Failure.Campaign.Test");
	ChecklistItem.RecoveryAction = TEXT("Recovery.Objective.Retry");
	ChecklistItem.InteractionMode = EHorrorCampaignInteractionMode::CircuitWiring;
	ChecklistItem.RiskLevel = EHorrorCampaignObjectiveRiskLevel::High;
	ChecklistItem.BeatCompletionRule = EHorrorCampaignObjectiveBeatCompletionRule::AdvancedWindow;
	ChecklistItem.BeatFailurePolicy = EHorrorCampaignObjectiveBeatFailurePolicy::RetryBeat;
	ChecklistItem.BeatNavigationRole = EHorrorCampaignObjectiveBeatNavigationRole::ObjectiveActor;
	ChecklistItem.RewardText = FText::FromString(TEXT("新增证据，恐惧下降"));
	ChecklistItem.bActive = true;
	ChecklistItem.bRequiresRecording = true;
	ChecklistItem.bRetryable = true;
	ChecklistItem.bMainline = true;
	ChecklistItem.RuntimeProgressFraction = 0.5f;
	ChecklistItem.PerformanceGradeFraction = 0.82f;
	ChecklistItem.InputPrecisionFraction = 0.74f;
	ChecklistItem.DeviceLoadFraction = 0.31f;
	ChecklistItem.RemainingSeconds = 12.0f;
	ChecklistItem.DistanceMeters = 18.0f;
	ObjectiveTracker.ChecklistItems.Add(ChecklistItem);
	FHorrorObjectiveChecklistItem GraphItem;
	GraphItem.Label = FText::FromString(TEXT("校准备用信标"));
	GraphItem.StatusText = FText::FromString(TEXT("可执行目标"));
	GraphItem.LockReason = FText::FromString(TEXT("锁定：先完成主供电"));
	GraphItem.ObjectiveId = TEXT("Graph.Beacon");
	GraphItem.bActive = true;
	GraphItem.bBlocked = false;
	GraphItem.bOptional = true;
	GraphItem.bNavigationFocused = true;
	ObjectiveTracker.ObjectiveGraphItems.Add(GraphItem);
	ObjectiveTracker.CompletedMilestoneCount = 2;
	ObjectiveTracker.RequiredMilestoneCount = 5;
	ObjectiveTracker.ProgressFraction = 0.4f;
	ObjectiveTracker.bRequiresRecording = true;
	ObjectiveTracker.bUrgent = true;
	ObjectiveTracker.bUsesFocusedInteraction = true;
	ObjectiveTracker.bOpensInteractionPanel = false;
	HUD->SetObjectiveTracker(ObjectiveTracker);
	TestEqual(TEXT("Day1 HUD should store the tracker title."), HUD->GetObjectiveTrackerTitleForTests().ToString(), FString(TEXT("记录异常")));
	TestEqual(TEXT("Day1 HUD should promote the tracker primary instruction."), HUD->GetCurrentObjectiveForTests().ToString(), FString(TEXT("保持录像，对准异常。")));
	TestEqual(TEXT("Day1 HUD should store the tracker secondary instruction."), HUD->GetObjectiveTrackerDetailForTests().ToString(), FString(TEXT("稳定取景直到信号锁定。")));
	TestEqual(TEXT("Day1 HUD should store the tracker progress label."), HUD->GetObjectiveTrackerProgressLabelForTests().ToString(), FString(TEXT("进度 2/5")));
	TestEqual(TEXT("Day1 HUD should store the tracker interaction label."), HUD->GetObjectiveTrackerInteractionLabelForTests().ToString(), FString(TEXT("录像瞄准")));
	TestEqual(TEXT("Day1 HUD should store mission context text."), HUD->GetObjectiveTrackerMissionContextForTests().ToString(), FString(TEXT("异常需要被镜头稳定捕捉。")));
	TestEqual(TEXT("Day1 HUD should store failure stakes text."), HUD->GetObjectiveTrackerFailureStakesForTests().ToString(), FString(TEXT("未录像会导致证据链中断。")));
	TestEqual(TEXT("Day1 HUD should store active chapter id."), HUD->GetObjectiveTrackerActiveChapterIdForTests(), FName(TEXT("DeepWater")));
	TestEqual(TEXT("Day1 HUD should store active objective id."), HUD->GetObjectiveTrackerActiveObjectiveIdForTests(), FName(TEXT("RecordAnomaly")));
	TestFalse(TEXT("Day1 HUD should expose whether the tracker opens an interaction panel."), HUD->DoesObjectiveTrackerOpenInteractionPanelForTests());
	TestTrue(TEXT("Day1 HUD should expose urgent objective state."), HUD->IsObjectiveTrackerUrgentForTests());
	TestTrue(TEXT("Day1 HUD should expose recording requirement state."), HUD->IsObjectiveTrackerRecordingRequiredForTests());
	TestTrue(TEXT("Day1 HUD should store objective progress fraction."), FMath::IsNearlyEqual(HUD->GetObjectiveTrackerProgressFractionForTests(), 0.4f));
	TestEqual(TEXT("Day1 HUD should store objective checklist items."), HUD->GetObjectiveChecklistItemCountForTests(), 1);
	TestEqual(TEXT("Day1 HUD should expose objective checklist labels."), HUD->GetObjectiveChecklistItemLabelForTests(0).ToString(), FString(TEXT("录像锁定第一个异常")));
	TestEqual(TEXT("Day1 HUD should expose objective checklist detail."), HUD->GetObjectiveChecklistItemDetailForTests(0).ToString(), FString(TEXT("保持画面中心稳定。")));
	TestEqual(TEXT("Day1 HUD should expose objective checklist interaction labels."), HUD->GetObjectiveChecklistItemInteractionLabelForTests(0).ToString(), FString(TEXT("录像瞄准")));
	TestEqual(TEXT("Day1 HUD should expose objective checklist runtime status text."), HUD->GetObjectiveChecklistItemStatusForTests(0).ToString(), FString(TEXT("等待录像锁定")));
	TestEqual(TEXT("Day1 HUD should expose objective checklist tactical labels."), HUD->GetObjectiveChecklistItemTacticalLabelForTests(0).ToString(), FString(TEXT("玩法：接线  |  判定：交互窗口  |  失败：重试阶段")));
	TestEqual(TEXT("Day1 HUD should expose objective checklist device diagnostics."), HUD->GetObjectiveChecklistItemDeviceStatusForTests(0).ToString(), FString(TEXT("端子同步中")));
	TestEqual(TEXT("Day1 HUD should expose objective checklist next action."), HUD->GetObjectiveChecklistItemNextActionForTests(0).ToString(), FString(TEXT("按 A 接入蓝色端子")));
	TestEqual(TEXT("Day1 HUD should expose objective checklist recovery guidance."), HUD->GetObjectiveChecklistItemFailureRecoveryForTests(0).ToString(), FString(TEXT("松手等待残压释放，再重新接入")));
	TestEqual(TEXT("Day1 HUD should expose objective checklist ids."), HUD->GetObjectiveChecklistItemObjectiveIdForTests(0), FName(TEXT("RecordAnomaly")));
	TestEqual(TEXT("Day1 HUD should expose objective checklist beat ids."), HUD->GetObjectiveChecklistItemBeatIdForTests(0), FName(TEXT("Beat.RecordFirstAnomaly")));
	TestEqual(TEXT("Day1 HUD should expose objective checklist beat indices."), HUD->GetObjectiveChecklistItemBeatIndexForTests(0), 1);
	TestEqual(TEXT("Day1 HUD should expose objective checklist runtime status."), HUD->GetObjectiveChecklistItemRuntimeStatusForTests(0), EHorrorCampaignObjectiveRuntimeStatus::AdvancedInteractionActive);
	TestEqual(TEXT("Day1 HUD should expose objective checklist failure causes."), HUD->GetObjectiveChecklistItemFailureCauseForTests(0), FName(TEXT("Failure.Campaign.Test")));
	TestEqual(TEXT("Day1 HUD should expose objective checklist recovery actions."), HUD->GetObjectiveChecklistItemRecoveryActionForTests(0), FName(TEXT("Recovery.Objective.Retry")));
	TestEqual(TEXT("Day1 HUD should expose objective checklist interaction mode."), HUD->GetObjectiveChecklistItemInteractionModeForTests(0), EHorrorCampaignInteractionMode::CircuitWiring);
	TestEqual(TEXT("Day1 HUD should expose objective checklist risk level."), HUD->GetObjectiveChecklistItemRiskLevelForTests(0), EHorrorCampaignObjectiveRiskLevel::High);
	TestEqual(TEXT("Day1 HUD should expose objective checklist beat completion rule."), HUD->GetObjectiveChecklistItemBeatCompletionRuleForTests(0), EHorrorCampaignObjectiveBeatCompletionRule::AdvancedWindow);
	TestEqual(TEXT("Day1 HUD should expose objective checklist beat failure policy."), HUD->GetObjectiveChecklistItemBeatFailurePolicyForTests(0), EHorrorCampaignObjectiveBeatFailurePolicy::RetryBeat);
	TestEqual(TEXT("Day1 HUD should expose objective checklist beat navigation role."), HUD->GetObjectiveChecklistItemBeatNavigationRoleForTests(0), EHorrorCampaignObjectiveBeatNavigationRole::ObjectiveActor);
	TestEqual(TEXT("Day1 HUD should expose objective checklist reward text."), HUD->GetObjectiveChecklistItemRewardTextForTests(0).ToString(), FString(TEXT("新增证据，恐惧下降")));
	TestTrue(TEXT("Day1 HUD should expose the active checklist row."), HUD->IsObjectiveChecklistItemActiveForTests(0));
	TestTrue(TEXT("Day1 HUD should expose checklist recording requirements."), HUD->DoesObjectiveChecklistItemRequireRecordingForTests(0));
	TestFalse(TEXT("Day1 HUD should expose blocked checklist rows."), HUD->IsObjectiveChecklistItemBlockedForTests(0));
	TestTrue(TEXT("Day1 HUD should expose retryable checklist rows."), HUD->IsObjectiveChecklistItemRetryableForTests(0));
	TestTrue(TEXT("Day1 HUD should expose mainline checklist rows."), HUD->IsObjectiveChecklistItemMainlineForTests(0));
	TestTrue(TEXT("Day1 HUD should expose checklist runtime progress."), FMath::IsNearlyEqual(HUD->GetObjectiveChecklistItemRuntimeProgressForTests(0), 0.5f));
	TestTrue(TEXT("Day1 HUD should expose checklist performance grade."), FMath::IsNearlyEqual(HUD->GetObjectiveChecklistItemPerformanceGradeForTests(0), 0.82f));
	TestTrue(TEXT("Day1 HUD should expose checklist input precision telemetry."), FMath::IsNearlyEqual(HUD->GetObjectiveChecklistItemInputPrecisionForTests(0), 0.74f));
	TestTrue(TEXT("Day1 HUD should expose checklist device load telemetry."), FMath::IsNearlyEqual(HUD->GetObjectiveChecklistItemDeviceLoadForTests(0), 0.31f));
	TestTrue(TEXT("Day1 HUD should expose checklist remaining seconds."), FMath::IsNearlyEqual(HUD->GetObjectiveChecklistItemRemainingSecondsForTests(0), 12.0f));
	TestTrue(TEXT("Day1 HUD should expose checklist distance meters."), FMath::IsNearlyEqual(HUD->GetObjectiveChecklistItemDistanceMetersForTests(0), 18.0f));
	const FString ChecklistActionLine = HUD->GetObjectiveChecklistItemActionLineForTests(0).ToString();
	TestTrue(TEXT("Day1 HUD checklist action line should include the live status."), ChecklistActionLine.Contains(TEXT("等待录像锁定")));
	TestTrue(TEXT("Day1 HUD checklist action line should include the next action cue."), ChecklistActionLine.Contains(TEXT("按 A 接入蓝色端子")));
	TestTrue(TEXT("Day1 HUD checklist action line should include retry recovery guidance."), ChecklistActionLine.Contains(TEXT("松手等待残压释放")));
	const FString ChecklistInstrumentLine = HUD->GetObjectiveChecklistItemInstrumentLineForTests(0).ToString();
	TestTrue(TEXT("Day1 HUD checklist instrument line should name the mechanic."), ChecklistInstrumentLine.Contains(TEXT("接线")));
	TestTrue(TEXT("Day1 HUD checklist instrument line should expose active advanced status."), ChecklistInstrumentLine.Contains(TEXT("交互中")));
	TestTrue(TEXT("Day1 HUD checklist instrument line should expose progress telemetry."), ChecklistInstrumentLine.Contains(TEXT("进度 50%")));
	TestTrue(TEXT("Day1 HUD checklist instrument line should expose precision telemetry."), ChecklistInstrumentLine.Contains(TEXT("精度 74%")));
	TestTrue(TEXT("Day1 HUD checklist instrument line should expose load telemetry."), ChecklistInstrumentLine.Contains(TEXT("负载 31%")));
	TestTrue(TEXT("Day1 HUD checklist instrument line should expose route distance."), ChecklistInstrumentLine.Contains(TEXT("18 米")));
	TestTrue(TEXT("Day1 HUD checklist instrument line should expose countdown telemetry."), ChecklistInstrumentLine.Contains(TEXT("12秒")));
	TestTrue(
		TEXT("Day1 HUD checklist telemetry rail should combine active progress and skill telemetry."),
		FMath::IsNearlyEqual(HUD->GetObjectiveChecklistItemTelemetryFractionForTests(0), 0.82f));
	TestEqual(TEXT("Day1 HUD should store objective graph rows separately from active checklist beats."), HUD->GetObjectiveGraphItemCountForTests(), 1);
	TestEqual(TEXT("Day1 HUD should expose objective graph labels."), HUD->GetObjectiveGraphItemLabelForTests(0).ToString(), FString(TEXT("校准备用信标")));
	TestEqual(TEXT("Day1 HUD should expose objective graph status text."), HUD->GetObjectiveGraphItemStatusForTests(0).ToString(), FString(TEXT("可执行目标")));
	TestEqual(TEXT("Day1 HUD should expose objective graph ids."), HUD->GetObjectiveGraphItemObjectiveIdForTests(0), FName(TEXT("Graph.Beacon")));
	TestTrue(TEXT("Day1 HUD should expose active objective graph rows."), HUD->IsObjectiveGraphItemActiveForTests(0));
	TestTrue(TEXT("Day1 HUD should expose manually focused navigation graph rows."), HUD->IsObjectiveGraphItemNavigationFocusedForTests(0));
	TestFalse(TEXT("Day1 HUD should expose blocked objective graph rows."), HUD->IsObjectiveGraphItemBlockedForTests(0));
	TestTrue(TEXT("Day1 HUD should expose optional objective graph rows."), HUD->IsObjectiveGraphItemOptionalForTests(0));
	const float SingleGraphPanelHeight = HUD->GetObjectiveTrackerPanelHeightForTests();
	for (int32 ExtraGraphIndex = 1; ExtraGraphIndex < 4; ++ExtraGraphIndex)
	{
		FHorrorObjectiveChecklistItem ExtraGraphItem = GraphItem;
		ExtraGraphItem.Label = FText::Format(
			NSLOCTEXT("Day1SliceHUDTest", "ExtraGraphItem", "备用任务节点 {0}"),
			FText::AsNumber(ExtraGraphIndex + 1));
		ExtraGraphItem.ObjectiveId = FName(*FString::Printf(TEXT("Graph.Extra%d"), ExtraGraphIndex));
		ExtraGraphItem.bActive = ExtraGraphIndex == 1;
		ExtraGraphItem.bBlocked = ExtraGraphIndex == 2;
		ExtraGraphItem.bOptional = ExtraGraphIndex == 3;
		ObjectiveTracker.ObjectiveGraphItems.Add(ExtraGraphItem);
	}
	HUD->SetObjectiveTracker(ObjectiveTracker);
	TestEqual(TEXT("Day1 HUD should retain the visible chapter graph rows for dense task planning."), HUD->GetObjectiveGraphItemCountForTests(), 4);
	TestTrue(
		TEXT("Objective tracker panel height should budget visible chapter graph rows instead of drawing them outside the panel."),
		HUD->GetObjectiveTrackerPanelHeightForTests() >= SingleGraphPanelHeight + 100.0f);
	FHorrorObjectiveChecklistItem HiddenFocusedGraphItem = GraphItem;
	HiddenFocusedGraphItem.Label = FText::FromString(TEXT("远端锁定导航节点"));
	HiddenFocusedGraphItem.ObjectiveId = TEXT("Graph.HiddenFocus");
	HiddenFocusedGraphItem.bActive = false;
	HiddenFocusedGraphItem.bOptional = false;
	HiddenFocusedGraphItem.bBlocked = false;
	HiddenFocusedGraphItem.bNavigationFocused = true;
	ObjectiveTracker.ObjectiveGraphItems.Add(HiddenFocusedGraphItem);
	HUD->SetObjectiveTracker(ObjectiveTracker);
	TestTrue(TEXT("Day1 HUD should keep the manually focused graph node visible even when it would otherwise be beyond the first four rows."), HUD->IsObjectiveGraphItemNavigationFocusedForTests(3));
	TestEqual(TEXT("The focused overflow graph node should replace the last visible row instead of being hidden."), HUD->GetObjectiveGraphItemObjectiveIdForTests(3), FName(TEXT("Graph.HiddenFocus")));
	TestEqual(TEXT("Day1 HUD should remember how many graph nodes were hidden by the compact mission map."), HUD->GetObjectiveGraphOverflowCountForTests(), 1);
	TestTrue(TEXT("Day1 HUD should explain hidden mission graph nodes in Chinese."), HUD->GetObjectiveGraphOverflowTextForTests().ToString().Contains(TEXT("还有 1 个任务节点")));

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
	CircuitPanel.FeedbackState = EHorrorAdvancedInteractionFeedbackState::Success;
	CircuitPanel.CurrentBeatLabel = FText::FromString(TEXT("接线同步"));
	CircuitPanel.CurrentBeatDetail = FText::FromString(TEXT("在发光窗口内接入正确线路。"));
	CircuitPanel.PhaseText = FText::FromString(TEXT("等待端子同步"));
	CircuitPanel.ExpectedInputLabel = FText::FromString(TEXT("蓝色端子"));
	CircuitPanel.DeviceStatusLabel = FText::FromString(TEXT("端子同步中"));
	CircuitPanel.RiskLabel = FText::FromString(TEXT("低风险"));
	CircuitPanel.RhythmLabel = FText::FromString(TEXT("同步窗口开启"));
	CircuitPanel.NextActionLabel = FText::FromString(TEXT("按 A 接入蓝色端子"));
	CircuitPanel.FailureRecoveryLabel = FText::FromString(TEXT("松手等待残压释放，再重新接入"));
	CircuitPanel.PerformanceGradeFraction = 1.4f;
	CircuitPanel.InputPrecisionFraction = 1.3f;
	CircuitPanel.DeviceLoadFraction = -0.25f;
	CircuitPanel.RouteFlowFraction = 1.6f;
	CircuitPanel.HazardPressureFraction = 1.2f;
	CircuitPanel.TargetAlignmentFraction = -0.5f;
	CircuitPanel.ActiveInputSlotIndex = 9;
	FHorrorAdvancedInteractionInputOption BlueOption;
	BlueOption.InputId = TEXT("蓝色端子");
	BlueOption.DisplayLabel = FText::FromString(TEXT("蓝色端子"));
	BlueOption.KeyHint = FText::FromString(TEXT("A"));
	BlueOption.ActionRole = EHorrorAdvancedInteractionInputRole::ConnectCircuit;
	BlueOption.ActionVerb = FText::FromString(TEXT("接入"));
	BlueOption.bRequiresTimingWindow = true;
	BlueOption.bAdvancesObjective = true;
	BlueOption.VisualSlotIndex = 0;
	BlueOption.VisualColor = FLinearColor(0.24f, 0.68f, 1.0f, 0.96f);
	CircuitPanel.InputOptions.Add(BlueOption);
	FHorrorAdvancedInteractionInputOption RedOption;
	RedOption.InputId = TEXT("红色端子");
	RedOption.DisplayLabel = FText::FromString(TEXT("红色端子"));
	RedOption.KeyHint = FText::FromString(TEXT("S"));
	RedOption.ActionRole = EHorrorAdvancedInteractionInputRole::AvoidHazard;
	RedOption.ActionVerb = FText::FromString(TEXT("避开"));
	RedOption.bAdvancesObjective = false;
	RedOption.VisualSlotIndex = 1;
	RedOption.VisualColor = FLinearColor(1.0f, 0.18f, 0.10f, 0.96f);
	RedOption.LoadFraction = 0.92f;
	RedOption.bHazardous = true;
	CircuitPanel.InputOptions.Add(RedOption);
	CircuitPanel.ProgressFraction = 0.48f;
	CircuitPanel.TimingFraction = 0.58f;
	CircuitPanel.TimingWindowStartFraction = 0.36f;
	CircuitPanel.TimingWindowEndFraction = 0.72f;
	CircuitPanel.StabilityFraction = 0.76f;
	CircuitPanel.PauseRemainingSeconds = 0.0f;
	CircuitPanel.bTimingWindowOpen = true;
	CircuitPanel.bPaused = false;
	CircuitPanel.bRecentSuccess = true;
	CircuitPanel.bRecentFailure = false;
	CircuitPanel.StepIndex = 2;
	CircuitPanel.RequiredStepCount = 3;
	CircuitPanel.ComboCount = 2;
	CircuitPanel.MistakeCount = 1;
	for (int32 StepIndex = 0; StepIndex < CircuitPanel.RequiredStepCount; ++StepIndex)
	{
		FHorrorAdvancedInteractionStepTrackItem Step;
		Step.InputId = StepIndex == 1 ? FName(TEXT("红色端子")) : FName(TEXT("蓝色端子"));
		Step.DisplayLabel = FText::FromName(Step.InputId);
		Step.KeyHint = FText::FromString(StepIndex == 1 ? TEXT("S") : TEXT("A"));
		Step.StepIndex = StepIndex;
		Step.VisualSlotIndex = StepIndex == 1 ? 1 : 0;
		Step.bComplete = StepIndex < CircuitPanel.StepIndex;
		Step.bActive = StepIndex == CircuitPanel.StepIndex;
		CircuitPanel.StepTrack.Add(Step);
	}
	HUD->SetAdvancedInteractionState(CircuitPanel);
	TestTrue(TEXT("Day1 HUD should expose advanced interaction panel visibility."), HUD->IsAdvancedInteractionPanelVisibleForTests());
	TestEqual(TEXT("Day1 HUD should store the advanced interaction mode."), HUD->GetAdvancedInteractionModeForTests(), EHorrorCampaignInteractionMode::CircuitWiring);
	TestEqual(TEXT("Day1 HUD should store the advanced panel title."), HUD->GetAdvancedInteractionTitleForTests().ToString(), FString(TEXT("修复深水站主供电")));
	TestEqual(TEXT("Day1 HUD should store the expected animated input id."), HUD->GetAdvancedInteractionExpectedInputForTests(), FName(TEXT("蓝色端子")));
	TestEqual(TEXT("Day1 HUD should store advanced interaction feedback."), HUD->GetAdvancedInteractionFeedbackForTests().ToString(), FString(TEXT("蓝色电弧：线路接入成功。")));
	TestEqual(TEXT("Day1 HUD should store advanced interaction feedback state."), HUD->GetAdvancedInteractionFeedbackStateForTests(), EHorrorAdvancedInteractionFeedbackState::Success);
	TestEqual(TEXT("Day1 HUD should store the advanced objective beat label."), HUD->GetAdvancedInteractionCurrentBeatLabelForTests().ToString(), FString(TEXT("接线同步")));
	TestEqual(TEXT("Day1 HUD should store the advanced objective beat detail."), HUD->GetAdvancedInteractionCurrentBeatDetailForTests().ToString(), FString(TEXT("在发光窗口内接入正确线路。")));
	TestEqual(TEXT("Day1 HUD should store the advanced interaction phase text."), HUD->GetAdvancedInteractionPhaseTextForTests().ToString(), FString(TEXT("等待端子同步")));
	TestEqual(TEXT("Day1 HUD should store the localized expected input label."), HUD->GetAdvancedInteractionExpectedInputLabelForTests().ToString(), FString(TEXT("蓝色端子")));
	TestEqual(TEXT("Day1 HUD should store the advanced device diagnostic label."), HUD->GetAdvancedInteractionDeviceStatusForTests().ToString(), FString(TEXT("端子同步中")));
	TestEqual(TEXT("Day1 HUD should store the advanced interaction risk label."), HUD->GetAdvancedInteractionRiskLabelForTests().ToString(), FString(TEXT("低风险")));
	TestEqual(TEXT("Day1 HUD should store the advanced interaction rhythm label."), HUD->GetAdvancedInteractionRhythmLabelForTests().ToString(), FString(TEXT("同步窗口开启")));
	TestEqual(TEXT("Day1 HUD should store the advanced next-action cue."), HUD->GetAdvancedInteractionNextActionForTests().ToString(), FString(TEXT("按 A 接入蓝色端子")));
	TestEqual(TEXT("Day1 HUD should store the advanced failure recovery cue."), HUD->GetAdvancedInteractionFailureRecoveryForTests().ToString(), FString(TEXT("松手等待残压释放，再重新接入")));
	TestEqual(TEXT("Day1 HUD should clamp the performance grade."), HUD->GetAdvancedInteractionPerformanceGradeForTests(), 1.0f);
	TestEqual(TEXT("Day1 HUD should clamp advanced interaction input precision."), HUD->GetAdvancedInteractionInputPrecisionForTests(), 1.0f);
	TestEqual(TEXT("Day1 HUD should clamp advanced interaction device load."), HUD->GetAdvancedInteractionDeviceLoadForTests(), 0.0f);
	TestEqual(TEXT("Day1 HUD should clamp circuit route flow for animated board traces."), HUD->GetAdvancedInteractionRouteFlowForTests(), 1.0f);
	TestEqual(TEXT("Day1 HUD should clamp circuit hazard pressure for spark effects."), HUD->GetAdvancedInteractionHazardPressureForTests(), 1.0f);
	TestEqual(TEXT("Day1 HUD should clamp circuit target alignment for the terminal lens."), HUD->GetAdvancedInteractionTargetAlignmentForTests(), 0.0f);
	TestEqual(TEXT("Day1 HUD should repair invalid active input slots to the expected option for board focus rendering."), HUD->GetAdvancedInteractionActiveInputSlotForTests(), 0);
	TestTrue(TEXT("Day1 HUD should store advanced interaction progress."), FMath::IsNearlyEqual(HUD->GetAdvancedInteractionProgressForTests(), 0.48f));
	TestTrue(TEXT("Day1 HUD should store advanced interaction timing cursor."), FMath::IsNearlyEqual(HUD->GetAdvancedInteractionTimingForTests(), 0.58f));
	TestTrue(TEXT("Day1 HUD should store the advanced interaction timing window start."), FMath::IsNearlyEqual(HUD->GetAdvancedInteractionTimingWindowStartForTests(), 0.36f));
	TestTrue(TEXT("Day1 HUD should store the advanced interaction timing window end."), FMath::IsNearlyEqual(HUD->GetAdvancedInteractionTimingWindowEndForTests(), 0.72f));
	TestTrue(TEXT("Day1 HUD should store advanced interaction stability."), FMath::IsNearlyEqual(HUD->GetAdvancedInteractionStabilityForTests(), 0.76f));
	TestTrue(TEXT("Day1 HUD should expose whether the timing window is open."), HUD->IsAdvancedInteractionTimingWindowOpenForTests());
	TestFalse(TEXT("Day1 HUD should expose paused state for animated devices."), HUD->IsAdvancedInteractionPausedForTests());
	TestTrue(TEXT("Day1 HUD should expose recent success feedback."), HUD->DidAdvancedInteractionRecentlySucceedForTests());
	TestFalse(TEXT("Day1 HUD should expose recent failure feedback."), HUD->DidAdvancedInteractionRecentlyFailForTests());
	TestEqual(TEXT("Day1 HUD should store the advanced interaction step index."), HUD->GetAdvancedInteractionStepIndexForTests(), 2);
	TestEqual(TEXT("Day1 HUD should store the required animated step count."), HUD->GetAdvancedInteractionRequiredStepCountForTests(), 3);
	TestEqual(TEXT("Day1 HUD should store advanced interaction combo count."), HUD->GetAdvancedInteractionComboCountForTests(), 2);
	TestEqual(TEXT("Day1 HUD should store advanced interaction mistake count."), HUD->GetAdvancedInteractionMistakeCountForTests(), 1);
	TestEqual(TEXT("Day1 HUD should store advanced input option visual slots."), HUD->GetAdvancedInteractionInputOptionVisualSlotForTests(0), 0);
	TestTrue(TEXT("Day1 HUD should store advanced input option visual color."), HUD->GetAdvancedInteractionInputOptionVisualColorForTests(0).B > HUD->GetAdvancedInteractionInputOptionVisualColorForTests(0).R);
	TestTrue(TEXT("Day1 HUD should store advanced input option load for circuit hazard readouts."), FMath::IsNearlyEqual(HUD->GetAdvancedInteractionInputOptionLoadForTests(1), 0.92f));
	TestTrue(TEXT("Day1 HUD should expose hazardous circuit terminals for the animated panel."), HUD->IsAdvancedInteractionInputOptionHazardousForTests(1));
	TestEqual(TEXT("Day1 HUD should store the advanced input action role."), HUD->GetAdvancedInteractionInputOptionActionRoleForTests(0), EHorrorAdvancedInteractionInputRole::ConnectCircuit);
	TestTrue(TEXT("Day1 HUD should store the advanced input action verb."), HUD->GetAdvancedInteractionInputOptionActionVerbForTests(0).ToString().Contains(TEXT("接入")));
	TestTrue(TEXT("Day1 HUD should expose timing-window inputs for instruments."), HUD->DoesAdvancedInteractionInputOptionRequireTimingWindowForTests(0));
	TestTrue(TEXT("Day1 HUD should expose progress-making instrument actions."), HUD->DoesAdvancedInteractionInputOptionAdvanceObjectiveForTests(0));
	TestEqual(TEXT("Day1 HUD should store the hazard avoidance role."), HUD->GetAdvancedInteractionInputOptionActionRoleForTests(1), EHorrorAdvancedInteractionInputRole::AvoidHazard);
	TestFalse(TEXT("Day1 HUD should expose hazard avoidance as non-progress action."), HUD->DoesAdvancedInteractionInputOptionAdvanceObjectiveForTests(1));
	TestTrue(
		TEXT("Circuit advanced panel should tell the player to use explicit device keys."),
		HUD->GetAdvancedInteractionControlHintForTests().ToString().Contains(TEXT("A/S/D")));
	TestTrue(
		TEXT("Circuit advanced panel should surface semantic action verbs in option hints."),
		HUD->GetAdvancedInteractionControlHintForTests().ToString().Contains(TEXT("A：接入蓝色端子")));
	TestTrue(
		TEXT("Circuit advanced panel should mark hazardous options in option hints."),
		HUD->GetAdvancedInteractionControlHintForTests().ToString().Contains(TEXT("S：避开红色端子 / 危险")));
	TestTrue(
		TEXT("Circuit advanced panel should warn players to avoid the high-load red terminal."),
		HUD->GetAdvancedInteractionControlHintForTests().ToString().Contains(TEXT("避开红色高负载")));
	TestFalse(
		TEXT("Circuit advanced panel should not claim the interaction key auto-connects the highlighted terminal."),
		HUD->GetAdvancedInteractionControlHintForTests().ToString().Contains(TEXT("互动键：接入")));
	const FString TacticalSummary = HUD->GetAdvancedInteractionTacticalSummaryForTests().ToString();
	TestTrue(TEXT("Advanced interaction panel should summarize the active gameplay mode."), TacticalSummary.Contains(TEXT("电路接线")));
	TestTrue(TEXT("Advanced interaction panel should summarize the current target input."), TacticalSummary.Contains(TEXT("目标 蓝色端子")));
	TestTrue(TEXT("Advanced interaction panel should summarize timing window telemetry."), TacticalSummary.Contains(TEXT("窗口 36-72%")));
	TestTrue(TEXT("Advanced interaction panel should summarize stability telemetry."), TacticalSummary.Contains(TEXT("稳定 76%")));
	TestTrue(TEXT("Advanced interaction panel should summarize device load telemetry."), TacticalSummary.Contains(TEXT("负载 0%")));
	TestTrue(TEXT("Advanced interaction panel should summarize player performance grade."), TacticalSummary.Contains(TEXT("评分 100%")));
	TestTrue(TEXT("Circuit advanced panel should summarize animated route flow telemetry."), TacticalSummary.Contains(TEXT("线路 100%")));
	TestTrue(TEXT("Circuit advanced panel should summarize leaking terminal pressure telemetry."), TacticalSummary.Contains(TEXT("漏电 100%")));

	FHorrorAdvancedInteractionHUDState HazardOutcomePanel = CircuitPanel;
	HazardOutcomePanel.FeedbackText = FText::GetEmpty();
	HazardOutcomePanel.FeedbackState = EHorrorAdvancedInteractionFeedbackState::Neutral;
	HazardOutcomePanel.bRecentSuccess = false;
	HazardOutcomePanel.bRecentFailure = false;
	HazardOutcomePanel.LastOutcome.Kind = EHorrorAdvancedInteractionOutcomeKind::Hazard;
	HazardOutcomePanel.LastOutcome.FeedbackState = EHorrorAdvancedInteractionFeedbackState::Failure;
	HazardOutcomePanel.LastOutcome.InputId = TEXT("红色端子");
	HazardOutcomePanel.LastOutcome.FaultId = TEXT("Fault.Advanced.CircuitHazard");
	HazardOutcomePanel.LastOutcome.bConsumesInput = true;
	HazardOutcomePanel.LastOutcome.bRetryable = true;
	HUD->SetAdvancedInteractionState(HazardOutcomePanel);
	TestEqual(TEXT("Advanced HUD should derive critical visual severity directly from LastOutcome."), HUD->GetAdvancedInteractionOutcomeSeverityForTests(), EHorrorObjectiveFeedbackSeverity::Critical);
	TestTrue(TEXT("Advanced HUD should surface the outcome kind even when feedback text is empty."), HUD->GetAdvancedInteractionOutcomeLineForTests().ToString().Contains(TEXT("危险输入")));
	TestTrue(TEXT("Advanced HUD should surface the outcome input id for the animated task window."), HUD->GetAdvancedInteractionOutcomeLineForTests().ToString().Contains(TEXT("红色端子")));
	TestTrue(TEXT("Advanced HUD should surface retry recovery guidance from the outcome."), HUD->GetAdvancedInteractionOutcomeLineForTests().ToString().Contains(TEXT("松手等待残压释放")));

	FHorrorAdvancedInteractionHUDState SuccessOutcomePanel = CircuitPanel;
	SuccessOutcomePanel.FeedbackText = FText::GetEmpty();
	SuccessOutcomePanel.FeedbackState = EHorrorAdvancedInteractionFeedbackState::Neutral;
	SuccessOutcomePanel.LastOutcome.Kind = EHorrorAdvancedInteractionOutcomeKind::Completed;
	SuccessOutcomePanel.LastOutcome.InputId = TEXT("蓝色端子");
	SuccessOutcomePanel.LastOutcome.bAdvancesProgress = true;
	HUD->SetAdvancedInteractionState(SuccessOutcomePanel);
	TestEqual(TEXT("Advanced HUD should derive success visual severity directly from LastOutcome."), HUD->GetAdvancedInteractionOutcomeSeverityForTests(), EHorrorObjectiveFeedbackSeverity::Success);
	TestTrue(TEXT("Advanced HUD should surface objective completion from LastOutcome."), HUD->GetAdvancedInteractionOutcomeLineForTests().ToString().Contains(TEXT("目标完成")));
	TestTrue(TEXT("Advanced HUD should tell the player progress advanced after a completed outcome."), HUD->GetAdvancedInteractionOutcomeLineForTests().ToString().Contains(TEXT("进度已推进")));

	FHorrorAdvancedInteractionHUDState GearPanel = CircuitPanel;
	GearPanel.Mode = EHorrorCampaignInteractionMode::GearCalibration;
	GearPanel.ExpectedInputId = TEXT("齿轮2");
	GearPanel.ExpectedInputLabel = FText::FromString(TEXT("齿轮2"));
	GearPanel.RouteFlowFraction = 0.46f;
	GearPanel.HazardPressureFraction = 0.61f;
	GearPanel.TargetAlignmentFraction = 0.73f;
	GearPanel.ActiveInputSlotIndex = 1;
	GearPanel.InputOptions.Reset();
	for (int32 OptionIndex = 0; OptionIndex < 3; ++OptionIndex)
	{
		FHorrorAdvancedInteractionInputOption Option;
		Option.InputId = OptionIndex == 0 ? FName(TEXT("齿轮1")) : (OptionIndex == 1 ? FName(TEXT("齿轮2")) : FName(TEXT("齿轮3")));
		Option.DisplayLabel = FText::FromName(Option.InputId);
		Option.KeyHint = FText::FromString(OptionIndex == 0 ? TEXT("A") : (OptionIndex == 1 ? TEXT("S") : TEXT("D")));
		Option.ActionRole = EHorrorAdvancedInteractionInputRole::CalibrateGear;
		Option.ActionVerb = FText::FromString(TEXT("拨动"));
		Option.VisualSlotIndex = OptionIndex;
		Option.VisualColor = FLinearColor(0.98f, 0.62f, 0.25f, 0.96f);
		GearPanel.InputOptions.Add(Option);
	}
	GearPanel.StepTrack.Reset();
	for (int32 StepIndex = 0; StepIndex < GearPanel.RequiredStepCount; ++StepIndex)
	{
		FHorrorAdvancedInteractionStepTrackItem Step;
		Step.InputId = StepIndex == 0 ? FName(TEXT("齿轮2")) : FName(TEXT("齿轮1"));
		Step.DisplayLabel = FText::FromName(Step.InputId);
		Step.KeyHint = FText::FromString(StepIndex == 0 ? TEXT("S") : TEXT("A"));
		Step.StepIndex = StepIndex;
		Step.VisualSlotIndex = StepIndex == 0 ? 1 : 0;
		GearPanel.StepTrack.Add(Step);
	}
	HUD->SetAdvancedInteractionState(GearPanel);
	TestTrue(
		TEXT("Gear advanced panel should tell the player to use explicit device keys."),
		HUD->GetAdvancedInteractionControlHintForTests().ToString().Contains(TEXT("A/S/D")));
	TestFalse(
		TEXT("Gear advanced panel should not claim the interaction key auto-turns the stopped gear."),
		HUD->GetAdvancedInteractionControlHintForTests().ToString().Contains(TEXT("互动键：拨动")));
	TestTrue(TEXT("Gear advanced panel should store chain engagement telemetry."), FMath::IsNearlyEqual(HUD->GetAdvancedInteractionRouteFlowForTests(), 0.46f));
	TestTrue(TEXT("Gear advanced panel should store jam pressure telemetry."), FMath::IsNearlyEqual(HUD->GetAdvancedInteractionHazardPressureForTests(), 0.61f));
	TestTrue(TEXT("Gear advanced panel should store tooth alignment telemetry."), FMath::IsNearlyEqual(HUD->GetAdvancedInteractionTargetAlignmentForTests(), 0.73f));
	TestEqual(TEXT("Gear advanced panel should expose the active gear slot for focus rendering."), HUD->GetAdvancedInteractionActiveInputSlotForTests(), 1);
	const FString GearTacticalSummary = HUD->GetAdvancedInteractionTacticalSummaryForTests().ToString();
	TestTrue(TEXT("Gear tactical summary should include chain engagement telemetry."), GearTacticalSummary.Contains(TEXT("链路 46%")));
	TestTrue(TEXT("Gear tactical summary should include jam pressure telemetry."), GearTacticalSummary.Contains(TEXT("卡滞 61%")));
	TestTrue(TEXT("Gear tactical summary should include tooth alignment telemetry."), GearTacticalSummary.Contains(TEXT("咬合 73%")));

	FHorrorAdvancedInteractionHUDState ScanPanel = CircuitPanel;
	ScanPanel.Mode = EHorrorCampaignInteractionMode::SpectralScan;
	ScanPanel.ExpectedInputId = TEXT("低频波段");
	ScanPanel.ExpectedInputLabel = FText::FromString(TEXT("低频波段"));
	ScanPanel.SpectralConfidenceFraction = 0.68f;
	ScanPanel.SpectralNoiseFraction = 0.31f;
	ScanPanel.InputOptions.Reset();
	FHorrorAdvancedInteractionInputOption LowBandOption;
	LowBandOption.InputId = TEXT("低频波段");
	LowBandOption.DisplayLabel = FText::FromString(TEXT("低频波段"));
	LowBandOption.KeyHint = FText::FromString(TEXT("A"));
	LowBandOption.VisualSlotIndex = 0;
	LowBandOption.VisualColor = FLinearColor(0.36f, 0.78f, 1.0f, 0.96f);
	ScanPanel.InputOptions.Add(LowBandOption);
	HUD->SetAdvancedInteractionState(ScanPanel);
	TestEqual(TEXT("Day1 HUD should store the spectral scan interaction mode."), HUD->GetAdvancedInteractionModeForTests(), EHorrorCampaignInteractionMode::SpectralScan);
	TestTrue(
		TEXT("Spectral scan panel should tell the player to sweep before locking."),
		HUD->GetAdvancedInteractionControlHintForTests().ToString().Contains(TEXT("扫频"))
			&& HUD->GetAdvancedInteractionControlHintForTests().ToString().Contains(TEXT("锁定")));
	TestTrue(
		TEXT("Spectral scan panel should keep explicit device-key controls."),
		HUD->GetAdvancedInteractionControlHintForTests().ToString().Contains(TEXT("A/D"))
			&& HUD->GetAdvancedInteractionControlHintForTests().ToString().Contains(TEXT("S")));
	const FString ScanTacticalSummary = HUD->GetAdvancedInteractionTacticalSummaryForTests().ToString();
	TestTrue(TEXT("Spectral scan tactical summary should use a dedicated anomaly spectrum readout."), ScanTacticalSummary.Contains(TEXT("异常频谱仪")));
	TestTrue(TEXT("Spectral scan tactical summary should include target band."), ScanTacticalSummary.Contains(TEXT("波段 低频波段")));
	TestTrue(TEXT("Spectral scan tactical summary should include confidence telemetry."), ScanTacticalSummary.Contains(TEXT("置信 68%")));
	TestTrue(TEXT("Spectral scan tactical summary should include noise telemetry."), ScanTacticalSummary.Contains(TEXT("噪点 31%")));
	TestTrue(TEXT("Spectral scan tactical summary should include lock readiness."), ScanTacticalSummary.Contains(TEXT("锁定 可锁定")));
	TestFalse(TEXT("Spectral scan tactical summary should not fall back to generic timing-window text."), ScanTacticalSummary.Contains(TEXT("窗口")));

	FHorrorAdvancedInteractionHUDState TuningPanel = CircuitPanel;
	TuningPanel.Mode = EHorrorCampaignInteractionMode::SignalTuning;
	TuningPanel.ExpectedInputId = TEXT("中心频率");
	TuningPanel.ExpectedInputLabel = FText::FromString(TEXT("中心频率"));
	TuningPanel.SignalBalanceFraction = 0.28f;
	TuningPanel.SignalTargetBalanceFraction = 0.50f;
	TuningPanel.InputOptions.Reset();
	FHorrorAdvancedInteractionInputOption LeftChannelOption;
	LeftChannelOption.InputId = TEXT("左声道");
	LeftChannelOption.DisplayLabel = FText::FromString(TEXT("左声道"));
	LeftChannelOption.KeyHint = FText::FromString(TEXT("A"));
	LeftChannelOption.VisualSlotIndex = 0;
	LeftChannelOption.VisualColor = FLinearColor(0.28f, 0.72f, 1.0f, 0.96f);
	LeftChannelOption.MotionFraction = -0.2f;
	LeftChannelOption.ResponseWindowFraction = 0.16f;
	TuningPanel.InputOptions.Add(LeftChannelOption);
	FHorrorAdvancedInteractionInputOption CenterFrequencyOption;
	CenterFrequencyOption.InputId = TEXT("中心频率");
	CenterFrequencyOption.DisplayLabel = FText::FromString(TEXT("中心频率"));
	CenterFrequencyOption.KeyHint = FText::FromString(TEXT("S"));
	CenterFrequencyOption.VisualSlotIndex = 1;
	CenterFrequencyOption.VisualColor = FLinearColor(1.0f, 0.88f, 0.34f, 0.96f);
	CenterFrequencyOption.LoadFraction = 1.25f;
	CenterFrequencyOption.ResponseWindowFraction = 0.12f;
	CenterFrequencyOption.bHazardous = true;
	TuningPanel.InputOptions.Add(CenterFrequencyOption);
	FHorrorAdvancedInteractionInputOption RightChannelOption;
	RightChannelOption.InputId = TEXT("右声道");
	RightChannelOption.DisplayLabel = FText::FromString(TEXT("右声道"));
	RightChannelOption.KeyHint = FText::FromString(TEXT("D"));
	RightChannelOption.VisualSlotIndex = 2;
	RightChannelOption.VisualColor = FLinearColor(0.42f, 1.0f, 0.76f, 0.96f);
	RightChannelOption.MotionFraction = 1.35f;
	RightChannelOption.ResponseWindowFraction = 0.78f;
	TuningPanel.InputOptions.Add(RightChannelOption);
	HUD->SetAdvancedInteractionState(TuningPanel);
	TestEqual(TEXT("Day1 HUD should store the signal tuning interaction mode."), HUD->GetAdvancedInteractionModeForTests(), EHorrorCampaignInteractionMode::SignalTuning);
	TestTrue(TEXT("Signal tuning panel should store left-shifted playback balance for the animated meter."), FMath::IsNearlyEqual(HUD->GetAdvancedInteractionSignalBalanceForTests(), 0.28f));
	TestTrue(TEXT("Signal tuning panel should store target playback balance for the animated meter."), FMath::IsNearlyEqual(HUD->GetAdvancedInteractionSignalTargetBalanceForTests(), 0.50f));
	TestEqual(TEXT("Signal tuning panel should expose all three channel controls."), HUD->GetAdvancedInteractionInputOptionCountForTests(), 3);
	TestEqual(TEXT("Signal tuning panel should preserve the center lock control slot."), HUD->GetAdvancedInteractionInputOptionIdForTests(1), FName(TEXT("中心频率")));
	TestTrue(TEXT("Signal tuning panel should clamp low channel motion telemetry for stable animation."), FMath::IsNearlyEqual(HUD->GetAdvancedInteractionInputOptionMotionForTests(0), 0.0f));
	TestTrue(TEXT("Signal tuning panel should clamp high channel motion telemetry for stable animation."), FMath::IsNearlyEqual(HUD->GetAdvancedInteractionInputOptionMotionForTests(2), 1.0f));
	TestTrue(TEXT("Signal tuning panel should preserve corrective-channel response strength."), FMath::IsNearlyEqual(HUD->GetAdvancedInteractionInputOptionResponseWindowForTests(2), 0.78f));
	TestTrue(TEXT("Signal tuning panel should clamp unsafe center-lock load telemetry."), FMath::IsNearlyEqual(HUD->GetAdvancedInteractionInputOptionLoadForTests(1), 1.0f));
	TestTrue(TEXT("Signal tuning panel should expose unsafe center lock state for animated warnings."), HUD->IsAdvancedInteractionInputOptionHazardousForTests(1));
	TestTrue(
		TEXT("Signal tuning panel should tell the player to align stereo playback before locking."),
		HUD->GetAdvancedInteractionControlHintForTests().ToString().Contains(TEXT("左右声道"))
			&& HUD->GetAdvancedInteractionControlHintForTests().ToString().Contains(TEXT("中心频率")));
	TestTrue(
		TEXT("Signal tuning panel should keep explicit device-key controls."),
		HUD->GetAdvancedInteractionControlHintForTests().ToString().Contains(TEXT("A/S/D")));
	TestFalse(
		TEXT("Signal tuning panel should not mention terminals."),
		HUD->GetAdvancedInteractionControlHintForTests().ToString().Contains(TEXT("端子")));
	TestFalse(
		TEXT("Signal tuning panel should not mention gears."),
		HUD->GetAdvancedInteractionControlHintForTests().ToString().Contains(TEXT("齿轮")));
	const FString SignalTacticalSummary = HUD->GetAdvancedInteractionTacticalSummaryForTests().ToString();
	TestTrue(TEXT("Signal tuning tactical summary should use a dedicated stereo diagnostic readout."), SignalTacticalSummary.Contains(TEXT("声像")));
	TestTrue(TEXT("Signal tuning tactical summary should keep the current tuning target in Chinese."), SignalTacticalSummary.Contains(TEXT("目标 中心频率")));
	TestTrue(TEXT("Signal tuning tactical summary should diagnose whether center lock is safe."), SignalTacticalSummary.Contains(TEXT("锁定")));
	TestTrue(TEXT("Signal tuning tactical summary should include distortion telemetry."), SignalTacticalSummary.Contains(TEXT("失真")));
	TestFalse(TEXT("Signal tuning tactical summary should not fall back to generic timing-window text."), SignalTacticalSummary.Contains(TEXT("窗口")));
	for (int32 CharacterIndex = 0; CharacterIndex < SignalTacticalSummary.Len(); ++CharacterIndex)
	{
		const TCHAR Character = SignalTacticalSummary[CharacterIndex];
		TestFalse(
			TEXT("Signal tuning tactical summary should not contain non-key English labels."),
			(Character >= TEXT('A') && Character <= TEXT('Z')) || (Character >= TEXT('a') && Character <= TEXT('z')));
	}

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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDay1SliceHUDRejectsIncompleteAdvancedInteractionStateTest,
	"HorrorProject.UI.Day1SliceHUD.RejectsIncompleteAdvancedInteractionState",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FDay1SliceHUDRejectsIncompleteAdvancedInteractionStateTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for advanced HUD contract coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	ADay1SliceHUD* HUD = World->SpawnActor<ADay1SliceHUD>();
	TestNotNull(TEXT("Advanced HUD contract test should spawn the native fallback HUD."), HUD);
	if (!HUD)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	FHorrorAdvancedInteractionHUDState IncompleteState;
	IncompleteState.bVisible = true;
	IncompleteState.Mode = EHorrorCampaignInteractionMode::GearCalibration;
	IncompleteState.Title = FText::FromString(TEXT("损坏的齿轮面板"));
	IncompleteState.ExpectedInputId = TEXT("齿轮2");
	IncompleteState.RequiredStepCount = 3;
	IncompleteState.ActiveInputSlotIndex = 7;

	HUD->SetAdvancedInteractionState(IncompleteState);
	TestFalse(TEXT("Incomplete advanced state should not expose the interactive panel."), HUD->IsAdvancedInteractionPanelVisibleForTests());
	TestTrue(TEXT("Incomplete advanced state should surface a localized recovery message."), HUD->IsTransientMessageVisibleForTests());
	TestTrue(TEXT("Incomplete advanced state recovery message should name the interaction panel."), HUD->GetTransientMessageForTests().ToString().Contains(TEXT("交互面板")));
	TestFalse(TEXT("Incomplete advanced state recovery message should not expose English text."), HUD->GetTransientMessageForTests().ToString().Contains(TEXT("Advanced")));
	TestEqual(TEXT("Invalid active slot should be cleared with the rejected panel."), HUD->GetAdvancedInteractionActiveInputSlotForTests(), INDEX_NONE);

	FHorrorAdvancedInteractionHUDState RecoveredState = IncompleteState;
	RecoveredState.InputOptions.Reset();
	for (int32 OptionIndex = 0; OptionIndex < 3; ++OptionIndex)
	{
		FHorrorAdvancedInteractionInputOption Option;
		Option.InputId = OptionIndex == 0 ? FName(TEXT("齿轮1")) : (OptionIndex == 1 ? FName(TEXT("齿轮2")) : FName(TEXT("齿轮3")));
		Option.DisplayLabel = FText::FromName(Option.InputId);
		Option.KeyHint = FText::FromString(OptionIndex == 0 ? TEXT("A") : (OptionIndex == 1 ? TEXT("S") : TEXT("D")));
		Option.VisualSlotIndex = OptionIndex;
		RecoveredState.InputOptions.Add(Option);
	}
	RecoveredState.StepTrack.Reset();
	for (int32 StepIndex = 0; StepIndex < RecoveredState.RequiredStepCount; ++StepIndex)
	{
		FHorrorAdvancedInteractionStepTrackItem Step;
		Step.InputId = StepIndex == 0 ? FName(TEXT("齿轮2")) : FName(TEXT("齿轮1"));
		Step.DisplayLabel = FText::FromName(Step.InputId);
		Step.KeyHint = FText::FromString(TEXT("S"));
		Step.StepIndex = StepIndex;
		Step.VisualSlotIndex = 1;
		RecoveredState.StepTrack.Add(Step);
	}
	RecoveredState.ActiveInputSlotIndex = 1;

	HUD->SetAdvancedInteractionState(RecoveredState);
	TestTrue(TEXT("Recovered advanced state should expose the interactive panel."), HUD->IsAdvancedInteractionPanelVisibleForTests());
	TestEqual(TEXT("Recovered advanced state should preserve the valid active slot."), HUD->GetAdvancedInteractionActiveInputSlotForTests(), 1);
	TestEqual(TEXT("Recovered advanced state should preserve option count."), HUD->GetAdvancedInteractionInputOptionCountForTests(), 3);
	TestEqual(TEXT("Recovered advanced state should preserve step track count."), HUD->GetAdvancedInteractionStepTrackCountForTests(), 3);

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDay1SliceHUDPrioritizesMissionGraphRowsTest,
	"HorrorProject.UI.Day1SliceHUD.PrioritizesMissionGraphRows",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FDay1SliceHUDPrioritizesMissionGraphRowsTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for mission graph priority coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	ADay1SliceHUD* HUD = World->SpawnActor<ADay1SliceHUD>();
	TestNotNull(TEXT("Mission graph priority test should spawn the native fallback HUD."), HUD);
	if (!HUD)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	FHorrorObjectiveTrackerSnapshot Tracker;
	Tracker.PrimaryInstruction = FText::FromString(TEXT("校准任务图谱优先级。"));
	auto MakeGraphItem = [](FName ObjectiveId, const TCHAR* Label)
	{
		FHorrorObjectiveChecklistItem Item;
		Item.ObjectiveId = ObjectiveId;
		Item.Label = FText::FromString(Label);
		Item.StatusText = FText::FromString(TEXT("图谱节点"));
		return Item;
	};

	FHorrorObjectiveChecklistItem CompletedItem = MakeGraphItem(TEXT("Graph.Completed"), TEXT("已完成节点"));
	CompletedItem.bComplete = true;
	FHorrorObjectiveChecklistItem OptionalItem = MakeGraphItem(TEXT("Graph.Optional"), TEXT("普通可选节点"));
	OptionalItem.bOptional = true;
	OptionalItem.bMainline = false;
	FHorrorObjectiveChecklistItem CurrentItem = MakeGraphItem(TEXT("Graph.Current"), TEXT("当前主线节点"));
	CurrentItem.bActive = true;
	CurrentItem.bMainline = true;
	FHorrorObjectiveChecklistItem ParallelMainlineItem = MakeGraphItem(TEXT("Graph.ParallelMainline"), TEXT("并行主线节点"));
	ParallelMainlineItem.bMainline = true;
	FHorrorObjectiveChecklistItem LockedPrerequisiteItem = MakeGraphItem(TEXT("Graph.LockedPrerequisite"), TEXT("锁定前置节点"));
	LockedPrerequisiteItem.bBlocked = true;
	LockedPrerequisiteItem.LockReason = FText::FromString(TEXT("锁定：先完成并行主线节点"));
	FHorrorObjectiveChecklistItem FocusedItem = MakeGraphItem(TEXT("Graph.NavigationFocus"), TEXT("导航锁定节点"));
	FocusedItem.bNavigationFocused = true;
	FocusedItem.bMainline = true;

	Tracker.ObjectiveGraphItems = {
		CompletedItem,
		OptionalItem,
		CurrentItem,
		ParallelMainlineItem,
		LockedPrerequisiteItem,
		FocusedItem
	};

	HUD->SetObjectiveTracker(Tracker);
	TestEqual(TEXT("Mission graph should still draw a compact four-row map."), HUD->GetObjectiveGraphItemCountForTests(), 4);
	TestEqual(TEXT("Current story objective should remain the first visible graph row."), HUD->GetObjectiveGraphItemObjectiveIdForTests(0), FName(TEXT("Graph.Current")));
	TestEqual(TEXT("Manual navigation focus should remain visible near the top of the graph."), HUD->GetObjectiveGraphItemObjectiveIdForTests(1), FName(TEXT("Graph.NavigationFocus")));
	TestEqual(TEXT("Available parallel mainline should outrank completed and optional rows."), HUD->GetObjectiveGraphItemObjectiveIdForTests(2), FName(TEXT("Graph.ParallelMainline")));
	TestEqual(TEXT("Locked prerequisite context should stay visible when it explains progression."), HUD->GetObjectiveGraphItemObjectiveIdForTests(3), FName(TEXT("Graph.LockedPrerequisite")));
	TestEqual(TEXT("Two lower-priority graph rows should be folded behind the overflow hint."), HUD->GetObjectiveGraphOverflowCountForTests(), 2);
	TestTrue(TEXT("Overflow hint should remain localized."), HUD->GetObjectiveGraphOverflowTextForTests().ToString().Contains(TEXT("还有 2 个任务节点")));

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

#endif
