// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Game/HorrorAdvancedInteractionTypes.h"
#include "Game/HorrorEventBusSubsystem.h"
#include "Game/HorrorFoundFootageContract.h"
#include "Game/HorrorObjectiveNavigationTypes.h"
#include "GameFramework/HUD.h"
#include "Interaction/HorrorInteractionTypes.h"
#include "Player/Components/NoteRecorderComponent.h"
#include "Day1SliceHUD.generated.h"

UENUM(BlueprintType)
enum class EDay1PauseMenuSelection : uint8
{
	Resume,
	MouseSensitivity,
	MasterVolume,
	Brightness
};

/**
 * Native fallback HUD for the Day 1 vertical slice.
 * Keeps the slice readable even before final UMG widgets are wired.
 */
UCLASS()
class HORRORPROJECT_API ADay1SliceHUD : public AHUD
{
	GENERATED_BODY()

public:
	virtual void DrawHUD() override;

	void SetInteractionPrompt(const FText& PromptText);
	void SetInteractionContext(const FHorrorInteractionContext& Context);
	void ClearInteractionPrompt();

	void SetCurrentObjective(const FText& ObjectiveText);
	void SetObjectiveTracker(const FHorrorObjectiveTrackerSnapshot& Snapshot);
	void SetObjectiveNavigation(const FText& NavigationText);
	void SetObjectiveNavigationState(const FHorrorObjectiveNavigationState& NavigationState);
	void ClearObjectiveNavigation();
	void ShowObjectiveToast(
		const FText& TitleText,
		const FText& HintText,
		float DisplaySeconds = 5.0f,
		EHorrorObjectiveFeedbackSeverity Severity = EHorrorObjectiveFeedbackSeverity::Info);
	void SetAnomalyCaptureStatus(const FText& StatusText, float ProgressFraction, bool bLocked, bool bRequiresRecording);
	void ClearAnomalyCaptureStatus();
	void ShowTransientMessage(const FText& MessageText, const FLinearColor& MessageColor, float DisplaySeconds = 2.5f);
	void ShowAutosaveIndicator(const FText& IndicatorText, float DisplaySeconds = 2.0f);
	void ShowDay1CompletionOverlay(const FText& TitleText, const FText& HintText);
	void ClearDay1CompletionOverlay();
	void SetSurvivalStatus(bool bBodycamAcquired, bool bBodycamEnabled, bool bRecording, float FearPercent, float SprintPercent, bool bDanger);
	void SetBodycamBatteryStatus(float BatteryPercent, bool bLowBattery);
	void ClearBodycamBatteryStatus();
	void ShowNotesJournal(const TArray<FHorrorNoteMetadata>& RecordedNotes);
	void ClearNotesJournal();

	void ShowPasswordPrompt(const FText& DoorName, const FText& HintText, int32 EnteredDigits, int32 RequiredDigits);
	void ClearPasswordPrompt();

	void ShowPauseMenu(EDay1PauseMenuSelection Selection, float MouseSensitivity, float MasterVolume, float Brightness);
	void ClearPauseMenu();
	void SetAdvancedInteractionState(const FHorrorAdvancedInteractionHUDState& State);
	void ClearAdvancedInteractionState();

	FText GetCurrentObjectiveForTests() const { return CurrentObjective; }
	FText GetObjectiveTrackerTitleForTests() const { return ObjectiveTrackerTitle; }
	FText GetObjectiveTrackerDetailForTests() const { return ObjectiveTrackerDetail; }
	FText GetObjectiveTrackerProgressLabelForTests() const { return ObjectiveTrackerProgressLabel; }
	FText GetObjectiveTrackerInteractionLabelForTests() const { return ObjectiveTrackerInteractionLabel; }
	FText GetObjectiveTrackerMissionContextForTests() const { return ObjectiveTrackerMissionContext; }
	FText GetObjectiveTrackerFailureStakesForTests() const { return ObjectiveTrackerFailureStakes; }
	FName GetObjectiveTrackerActiveChapterIdForTests() const { return ObjectiveTrackerActiveChapterId; }
	FName GetObjectiveTrackerActiveObjectiveIdForTests() const { return ObjectiveTrackerActiveObjectiveId; }
	bool DoesObjectiveTrackerOpenInteractionPanelForTests() const { return bObjectiveTrackerOpensInteractionPanel; }
	float GetObjectiveTrackerProgressFractionForTests() const { return ObjectiveTrackerProgressFraction; }
	bool IsObjectiveTrackerUrgentForTests() const { return bObjectiveTrackerUrgent; }
	bool IsObjectiveTrackerRecordingRequiredForTests() const { return bObjectiveTrackerRequiresRecording; }
	int32 GetObjectiveChecklistItemCountForTests() const { return ObjectiveChecklistItems.Num(); }
	FText GetObjectiveChecklistItemLabelForTests(int32 Index) const { return ObjectiveChecklistItems.IsValidIndex(Index) ? ObjectiveChecklistItems[Index].Label : FText::GetEmpty(); }
	FText GetObjectiveChecklistItemDetailForTests(int32 Index) const { return ObjectiveChecklistItems.IsValidIndex(Index) ? ObjectiveChecklistItems[Index].Detail : FText::GetEmpty(); }
	FText GetObjectiveChecklistItemInteractionLabelForTests(int32 Index) const { return ObjectiveChecklistItems.IsValidIndex(Index) ? ObjectiveChecklistItems[Index].InteractionLabel : FText::GetEmpty(); }
	FText GetObjectiveChecklistItemLockReasonForTests(int32 Index) const { return ObjectiveChecklistItems.IsValidIndex(Index) ? ObjectiveChecklistItems[Index].LockReason : FText::GetEmpty(); }
	FText GetObjectiveChecklistItemStatusForTests(int32 Index) const { return ObjectiveChecklistItems.IsValidIndex(Index) ? ObjectiveChecklistItems[Index].StatusText : FText::GetEmpty(); }
	FText GetObjectiveChecklistItemTacticalLabelForTests(int32 Index) const { return ObjectiveChecklistItems.IsValidIndex(Index) ? ObjectiveChecklistItems[Index].TacticalLabel : FText::GetEmpty(); }
	FText GetObjectiveChecklistItemDeviceStatusForTests(int32 Index) const { return ObjectiveChecklistItems.IsValidIndex(Index) ? ObjectiveChecklistItems[Index].DeviceStatusLabel : FText::GetEmpty(); }
	FText GetObjectiveChecklistItemNextActionForTests(int32 Index) const { return ObjectiveChecklistItems.IsValidIndex(Index) ? ObjectiveChecklistItems[Index].NextActionLabel : FText::GetEmpty(); }
	FText GetObjectiveChecklistItemFailureRecoveryForTests(int32 Index) const { return ObjectiveChecklistItems.IsValidIndex(Index) ? ObjectiveChecklistItems[Index].FailureRecoveryLabel : FText::GetEmpty(); }
	FName GetObjectiveChecklistItemObjectiveIdForTests(int32 Index) const { return ObjectiveChecklistItems.IsValidIndex(Index) ? ObjectiveChecklistItems[Index].ObjectiveId : NAME_None; }
	FName GetObjectiveChecklistItemBeatIdForTests(int32 Index) const { return ObjectiveChecklistItems.IsValidIndex(Index) ? ObjectiveChecklistItems[Index].BeatId : NAME_None; }
	int32 GetObjectiveChecklistItemBeatIndexForTests(int32 Index) const { return ObjectiveChecklistItems.IsValidIndex(Index) ? ObjectiveChecklistItems[Index].BeatIndex : INDEX_NONE; }
	EHorrorCampaignObjectiveRuntimeStatus GetObjectiveChecklistItemRuntimeStatusForTests(int32 Index) const { return ObjectiveChecklistItems.IsValidIndex(Index) ? ObjectiveChecklistItems[Index].RuntimeStatus : EHorrorCampaignObjectiveRuntimeStatus::Hidden; }
	FName GetObjectiveChecklistItemFailureCauseForTests(int32 Index) const { return ObjectiveChecklistItems.IsValidIndex(Index) ? ObjectiveChecklistItems[Index].FailureCause : NAME_None; }
	FName GetObjectiveChecklistItemRecoveryActionForTests(int32 Index) const { return ObjectiveChecklistItems.IsValidIndex(Index) ? ObjectiveChecklistItems[Index].RecoveryAction : NAME_None; }
	EHorrorCampaignInteractionMode GetObjectiveChecklistItemInteractionModeForTests(int32 Index) const { return ObjectiveChecklistItems.IsValidIndex(Index) ? ObjectiveChecklistItems[Index].InteractionMode : EHorrorCampaignInteractionMode::Instant; }
	EHorrorCampaignObjectiveRiskLevel GetObjectiveChecklistItemRiskLevelForTests(int32 Index) const { return ObjectiveChecklistItems.IsValidIndex(Index) ? ObjectiveChecklistItems[Index].RiskLevel : EHorrorCampaignObjectiveRiskLevel::None; }
	EHorrorCampaignObjectiveBeatCompletionRule GetObjectiveChecklistItemBeatCompletionRuleForTests(int32 Index) const { return ObjectiveChecklistItems.IsValidIndex(Index) ? ObjectiveChecklistItems[Index].BeatCompletionRule : EHorrorCampaignObjectiveBeatCompletionRule::None; }
	EHorrorCampaignObjectiveBeatFailurePolicy GetObjectiveChecklistItemBeatFailurePolicyForTests(int32 Index) const { return ObjectiveChecklistItems.IsValidIndex(Index) ? ObjectiveChecklistItems[Index].BeatFailurePolicy : EHorrorCampaignObjectiveBeatFailurePolicy::None; }
	EHorrorCampaignObjectiveBeatNavigationRole GetObjectiveChecklistItemBeatNavigationRoleForTests(int32 Index) const { return ObjectiveChecklistItems.IsValidIndex(Index) ? ObjectiveChecklistItems[Index].BeatNavigationRole : EHorrorCampaignObjectiveBeatNavigationRole::None; }
	FText GetObjectiveChecklistItemRewardTextForTests(int32 Index) const { return ObjectiveChecklistItems.IsValidIndex(Index) ? ObjectiveChecklistItems[Index].RewardText : FText::GetEmpty(); }
	bool IsObjectiveChecklistItemCompleteForTests(int32 Index) const { return ObjectiveChecklistItems.IsValidIndex(Index) && ObjectiveChecklistItems[Index].bComplete; }
	bool IsObjectiveChecklistItemActiveForTests(int32 Index) const { return ObjectiveChecklistItems.IsValidIndex(Index) && ObjectiveChecklistItems[Index].bActive; }
	bool DoesObjectiveChecklistItemRequireRecordingForTests(int32 Index) const { return ObjectiveChecklistItems.IsValidIndex(Index) && ObjectiveChecklistItems[Index].bRequiresRecording; }
	bool IsObjectiveChecklistItemBlockedForTests(int32 Index) const { return ObjectiveChecklistItems.IsValidIndex(Index) && ObjectiveChecklistItems[Index].bBlocked; }
	bool DoesObjectiveChecklistItemOpenPanelForTests(int32 Index) const { return ObjectiveChecklistItems.IsValidIndex(Index) && ObjectiveChecklistItems[Index].bOpensInteractionPanel; }
	bool IsObjectiveChecklistItemRetryableForTests(int32 Index) const { return ObjectiveChecklistItems.IsValidIndex(Index) && ObjectiveChecklistItems[Index].bRetryable; }
	bool IsObjectiveChecklistItemMainlineForTests(int32 Index) const { return ObjectiveChecklistItems.IsValidIndex(Index) && ObjectiveChecklistItems[Index].bMainline; }
	float GetObjectiveChecklistItemRuntimeProgressForTests(int32 Index) const { return ObjectiveChecklistItems.IsValidIndex(Index) ? ObjectiveChecklistItems[Index].RuntimeProgressFraction : 0.0f; }
	float GetObjectiveChecklistItemPerformanceGradeForTests(int32 Index) const { return ObjectiveChecklistItems.IsValidIndex(Index) ? ObjectiveChecklistItems[Index].PerformanceGradeFraction : 0.0f; }
	float GetObjectiveChecklistItemInputPrecisionForTests(int32 Index) const { return ObjectiveChecklistItems.IsValidIndex(Index) ? ObjectiveChecklistItems[Index].InputPrecisionFraction : 0.0f; }
	float GetObjectiveChecklistItemDeviceLoadForTests(int32 Index) const { return ObjectiveChecklistItems.IsValidIndex(Index) ? ObjectiveChecklistItems[Index].DeviceLoadFraction : 0.0f; }
	float GetObjectiveChecklistItemRemainingSecondsForTests(int32 Index) const { return ObjectiveChecklistItems.IsValidIndex(Index) ? ObjectiveChecklistItems[Index].RemainingSeconds : 0.0f; }
	float GetObjectiveChecklistItemDistanceMetersForTests(int32 Index) const { return ObjectiveChecklistItems.IsValidIndex(Index) ? ObjectiveChecklistItems[Index].DistanceMeters : 0.0f; }
	FText GetObjectiveChecklistItemActionLineForTests(int32 Index) const { return ObjectiveChecklistItems.IsValidIndex(Index) ? BuildObjectiveChecklistActionLine(ObjectiveChecklistItems[Index]) : FText::GetEmpty(); }
	FText GetObjectiveChecklistItemInstrumentLineForTests(int32 Index) const { return ObjectiveChecklistItems.IsValidIndex(Index) ? BuildObjectiveChecklistInstrumentLine(ObjectiveChecklistItems[Index]) : FText::GetEmpty(); }
	float GetObjectiveChecklistItemTelemetryFractionForTests(int32 Index) const { return ObjectiveChecklistItems.IsValidIndex(Index) ? ComputeObjectiveChecklistTelemetryFraction(ObjectiveChecklistItems[Index]) : 0.0f; }
	int32 GetObjectiveGraphItemCountForTests() const { return ObjectiveGraphItems.Num(); }
	FText GetObjectiveGraphItemLabelForTests(int32 Index) const { return ObjectiveGraphItems.IsValidIndex(Index) ? ObjectiveGraphItems[Index].Label : FText::GetEmpty(); }
	FText GetObjectiveGraphItemStatusForTests(int32 Index) const { return ObjectiveGraphItems.IsValidIndex(Index) ? ObjectiveGraphItems[Index].StatusText : FText::GetEmpty(); }
	FText GetObjectiveGraphItemLockReasonForTests(int32 Index) const { return ObjectiveGraphItems.IsValidIndex(Index) ? ObjectiveGraphItems[Index].LockReason : FText::GetEmpty(); }
	FName GetObjectiveGraphItemObjectiveIdForTests(int32 Index) const { return ObjectiveGraphItems.IsValidIndex(Index) ? ObjectiveGraphItems[Index].ObjectiveId : NAME_None; }
	bool IsObjectiveGraphItemCompleteForTests(int32 Index) const { return ObjectiveGraphItems.IsValidIndex(Index) && ObjectiveGraphItems[Index].bComplete; }
	bool IsObjectiveGraphItemActiveForTests(int32 Index) const { return ObjectiveGraphItems.IsValidIndex(Index) && ObjectiveGraphItems[Index].bActive; }
	bool IsObjectiveGraphItemBlockedForTests(int32 Index) const { return ObjectiveGraphItems.IsValidIndex(Index) && ObjectiveGraphItems[Index].bBlocked; }
	bool IsObjectiveGraphItemOptionalForTests(int32 Index) const { return ObjectiveGraphItems.IsValidIndex(Index) && ObjectiveGraphItems[Index].bOptional; }
	bool IsObjectiveGraphItemNavigationFocusedForTests(int32 Index) const { return ObjectiveGraphItems.IsValidIndex(Index) && ObjectiveGraphItems[Index].bNavigationFocused; }
	int32 GetObjectiveGraphOverflowCountForTests() const { return ObjectiveGraphOverflowCount; }
	FText GetObjectiveGraphOverflowTextForTests() const { return ObjectiveGraphOverflowText; }
	float GetObjectiveTrackerPanelHeightForTests() const { return ComputeObjectiveTrackerPanelHeight(); }
	bool IsAnomalyCaptureStatusVisibleForTests() const { return bAnomalyCaptureStatusVisible; }
	FText GetAnomalyCaptureStatusForTests() const { return AnomalyCaptureStatus; }
	float GetAnomalyCaptureProgressForTests() const { return AnomalyCaptureProgressFraction; }
	bool IsAnomalyCaptureLockedForTests() const { return bAnomalyCaptureLocked; }
	bool DoesAnomalyCaptureRequireRecordingForTests() const { return bAnomalyCaptureRequiresRecording; }
	FText GetObjectiveNavigationForTests() const { return ObjectiveNavigation; }
	bool IsObjectiveNavigationVisibleForTests() const { return ObjectiveNavigationState.bVisible; }
	FText GetObjectiveNavigationLabelForTests() const { return ObjectiveNavigationState.Label; }
	FText GetObjectiveNavigationDirectionForTests() const { return ObjectiveNavigationState.DirectionText; }
	float GetObjectiveNavigationDistanceMetersForTests() const { return ObjectiveNavigationState.DistanceMeters; }
	bool IsObjectiveNavigationReachableForTests() const { return ObjectiveNavigationState.bReachable; }
	bool IsObjectiveNavigationRetryableForTests() const { return ObjectiveNavigationState.bRetryable; }
	EHorrorCampaignObjectiveRuntimeStatus GetObjectiveNavigationRuntimeStatusForTests() const { return ObjectiveNavigationState.RuntimeStatus; }
	FText GetObjectiveNavigationDeviceStatusForTests() const { return ObjectiveNavigationState.DeviceStatusLabel; }
	FText GetObjectiveNavigationNextActionForTests() const { return ObjectiveNavigationState.NextActionLabel; }
	FText GetObjectiveNavigationFailureRecoveryForTests() const { return ObjectiveNavigationState.FailureRecoveryLabel; }
	FText GetObjectiveNavigationDiagnosticTextForTests() const { return BuildObjectiveNavigationDiagnosticText(); }
	FText GetObjectiveNavigationRouteTextForTests() const { return BuildObjectiveNavigationRouteText(); }
	float GetObjectiveNavigationPerformanceGradeForTests() const { return ObjectiveNavigationState.PerformanceGradeFraction; }
	float GetObjectiveNavigationEscapeBudgetForTests() const { return ObjectiveNavigationState.EscapeTimeBudgetSeconds; }
	float GetObjectiveNavigationEstimatedEscapeArrivalForTests() const { return ObjectiveNavigationState.EstimatedEscapeArrivalSeconds; }
	FName GetObjectiveNavigationFailureCauseForTests() const { return ObjectiveNavigationState.FailureCause; }
	FName GetObjectiveNavigationRecoveryActionForTests() const { return ObjectiveNavigationState.RecoveryAction; }
	FText GetObjectiveToastTitleForTests() const { return ToastTitle; }
	FText GetObjectiveToastHintForTests() const { return ToastHint; }
	EHorrorObjectiveFeedbackSeverity GetObjectiveToastSeverityForTests() const { return ToastSeverity; }
	FText GetInteractionPromptForTests() const { return InteractionPrompt; }
	bool IsInteractionContextVisibleForTests() const { return InteractionContext.bVisible; }
	bool CanFocusedInteractionForTests() const { return InteractionContext.bCanInteract; }
	bool DoesFocusedInteractionRequireRecordingForTests() const { return InteractionContext.bRequiresRecording; }
	bool DoesFocusedInteractionOpenPanelForTests() const { return InteractionContext.bOpensPanel; }
	EHorrorInteractionVerb GetFocusedInteractionVerbForTests() const { return InteractionContext.Verb; }
	EHorrorInteractionInputStyle GetFocusedInteractionInputStyleForTests() const { return InteractionContext.InputStyle; }
	EHorrorInteractionRiskLevel GetFocusedInteractionRiskLevelForTests() const { return InteractionContext.RiskLevel; }
	FText GetFocusedInteractionActionForTests() const { return InteractionContext.ActionText; }
	FText GetFocusedInteractionDetailForTests() const { return InteractionContext.DetailText; }
	FText GetFocusedInteractionBlockedReasonForTests() const { return InteractionContext.BlockedReason; }
	FText GetFocusedInteractionInputTextForTests() const { return InteractionContext.InputText; }
	FName GetFocusedInteractionTargetIdForTests() const { return InteractionContext.TargetId; }
	bool IsPasswordPromptVisibleForTests() const { return bPasswordPromptVisible; }
	FText GetPasswordHintForTests() const { return PasswordHint; }
	bool IsPauseMenuVisibleForTests() const { return bPauseMenuVisible; }
	EDay1PauseMenuSelection GetPauseMenuSelectionForTests() const { return PauseMenuSelection; }
	bool IsAutosaveIndicatorVisibleForTests() const { return AutosaveIndicatorExpireWorldSeconds > GetWorldSeconds() && !AutosaveIndicatorText.IsEmpty(); }
	FText GetAutosaveIndicatorTextForTests() const { return AutosaveIndicatorText; }
	bool IsTransientMessageVisibleForTests() const { return TransientMessageExpireWorldSeconds > GetWorldSeconds() && !TransientMessage.IsEmpty(); }
	FText GetTransientMessageForTests() const { return TransientMessage; }
	FLinearColor GetTransientMessageColorForTests() const { return TransientMessageColor; }
	bool IsDay1CompletionOverlayVisibleForTests() const { return bDay1CompletionOverlayVisible; }
	FText GetDay1CompletionTitleForTests() const { return Day1CompletionTitle; }
	FText GetDay1CompletionHintForTests() const { return Day1CompletionHint; }
	bool IsDangerStatusVisibleForTests() const { return bStatusDanger; }
	bool IsBodycamBatteryVisibleForTests() const { return bBodycamBatteryVisible; }
	float GetBodycamBatteryPercentForTests() const { return BodycamBatteryPercent; }
	bool IsBodycamBatteryLowForTests() const { return bBodycamBatteryLow; }
	bool IsNotesJournalVisibleForTests() const { return bNotesJournalVisible; }
	bool IsNotesJournalEmptyForTests() const { return NotesJournalEntries.IsEmpty(); }
	FText GetNotesJournalEmptyTextForTests() const { return NotesJournalEmptyText; }
	int32 GetNotesJournalEntryCountForTests() const { return NotesJournalEntries.Num(); }
	FText GetNotesJournalTitleForTests(int32 Index) const { return NotesJournalEntries.IsValidIndex(Index) ? NotesJournalEntries[Index].Title : FText::GetEmpty(); }
	FText GetNotesJournalBodyForTests(int32 Index) const { return NotesJournalEntries.IsValidIndex(Index) ? NotesJournalEntries[Index].Body : FText::GetEmpty(); }
	bool IsAdvancedInteractionPanelVisibleForTests() const { return AdvancedInteractionState.bVisible; }
	EHorrorCampaignInteractionMode GetAdvancedInteractionModeForTests() const { return AdvancedInteractionState.Mode; }
	FText GetAdvancedInteractionTitleForTests() const { return AdvancedInteractionState.Title; }
	FName GetAdvancedInteractionExpectedInputForTests() const { return AdvancedInteractionState.ExpectedInputId; }
	FText GetAdvancedInteractionFeedbackForTests() const { return AdvancedInteractionState.FeedbackText; }
	EHorrorAdvancedInteractionFeedbackState GetAdvancedInteractionFeedbackStateForTests() const { return AdvancedInteractionState.FeedbackState; }
	FText GetAdvancedInteractionCurrentBeatLabelForTests() const { return AdvancedInteractionState.CurrentBeatLabel; }
	FText GetAdvancedInteractionCurrentBeatDetailForTests() const { return AdvancedInteractionState.CurrentBeatDetail; }
	FText GetAdvancedInteractionPhaseTextForTests() const { return AdvancedInteractionState.PhaseText; }
	FText GetAdvancedInteractionExpectedInputLabelForTests() const { return AdvancedInteractionState.ExpectedInputLabel; }
	FText GetAdvancedInteractionDeviceStatusForTests() const { return AdvancedInteractionState.DeviceStatusLabel; }
	FText GetAdvancedInteractionRiskLabelForTests() const { return AdvancedInteractionState.RiskLabel; }
	FText GetAdvancedInteractionRhythmLabelForTests() const { return AdvancedInteractionState.RhythmLabel; }
	FText GetAdvancedInteractionNextActionForTests() const { return AdvancedInteractionState.NextActionLabel; }
	FText GetAdvancedInteractionFailureRecoveryForTests() const { return AdvancedInteractionState.FailureRecoveryLabel; }
	int32 GetAdvancedInteractionInputOptionCountForTests() const { return AdvancedInteractionState.InputOptions.Num(); }
	FName GetAdvancedInteractionInputOptionIdForTests(int32 Index) const { return AdvancedInteractionState.InputOptions.IsValidIndex(Index) ? AdvancedInteractionState.InputOptions[Index].InputId : NAME_None; }
	FText GetAdvancedInteractionInputOptionLabelForTests(int32 Index) const { return AdvancedInteractionState.InputOptions.IsValidIndex(Index) ? AdvancedInteractionState.InputOptions[Index].DisplayLabel : FText::GetEmpty(); }
	FText GetAdvancedInteractionInputOptionKeyHintForTests(int32 Index) const { return AdvancedInteractionState.InputOptions.IsValidIndex(Index) ? AdvancedInteractionState.InputOptions[Index].KeyHint : FText::GetEmpty(); }
	int32 GetAdvancedInteractionInputOptionVisualSlotForTests(int32 Index) const { return AdvancedInteractionState.InputOptions.IsValidIndex(Index) ? AdvancedInteractionState.InputOptions[Index].VisualSlotIndex : INDEX_NONE; }
	FLinearColor GetAdvancedInteractionInputOptionVisualColorForTests(int32 Index) const { return AdvancedInteractionState.InputOptions.IsValidIndex(Index) ? AdvancedInteractionState.InputOptions[Index].VisualColor : FLinearColor::Transparent; }
	float GetAdvancedInteractionInputOptionLoadForTests(int32 Index) const { return AdvancedInteractionState.InputOptions.IsValidIndex(Index) ? AdvancedInteractionState.InputOptions[Index].LoadFraction : 0.0f; }
	float GetAdvancedInteractionInputOptionMotionForTests(int32 Index) const { return AdvancedInteractionState.InputOptions.IsValidIndex(Index) ? AdvancedInteractionState.InputOptions[Index].MotionFraction : 0.0f; }
	float GetAdvancedInteractionInputOptionResponseWindowForTests(int32 Index) const { return AdvancedInteractionState.InputOptions.IsValidIndex(Index) ? AdvancedInteractionState.InputOptions[Index].ResponseWindowFraction : 0.0f; }
	bool IsAdvancedInteractionInputOptionHazardousForTests(int32 Index) const { return AdvancedInteractionState.InputOptions.IsValidIndex(Index) && AdvancedInteractionState.InputOptions[Index].bHazardous; }
	bool IsAdvancedInteractionInputOptionStalledForTests(int32 Index) const { return AdvancedInteractionState.InputOptions.IsValidIndex(Index) && AdvancedInteractionState.InputOptions[Index].bStalled; }
	bool IsAdvancedInteractionInputOptionChainLinkedForTests(int32 Index) const { return AdvancedInteractionState.InputOptions.IsValidIndex(Index) && AdvancedInteractionState.InputOptions[Index].bChainLinked; }
	EHorrorAdvancedInteractionInputRole GetAdvancedInteractionInputOptionActionRoleForTests(int32 Index) const { return AdvancedInteractionState.InputOptions.IsValidIndex(Index) ? AdvancedInteractionState.InputOptions[Index].ActionRole : EHorrorAdvancedInteractionInputRole::SelectTarget; }
	FText GetAdvancedInteractionInputOptionActionVerbForTests(int32 Index) const { return AdvancedInteractionState.InputOptions.IsValidIndex(Index) ? AdvancedInteractionState.InputOptions[Index].ActionVerb : FText::GetEmpty(); }
	bool DoesAdvancedInteractionInputOptionRequireTimingWindowForTests(int32 Index) const { return AdvancedInteractionState.InputOptions.IsValidIndex(Index) && AdvancedInteractionState.InputOptions[Index].bRequiresTimingWindow; }
	bool DoesAdvancedInteractionInputOptionRequireStableSignalForTests(int32 Index) const { return AdvancedInteractionState.InputOptions.IsValidIndex(Index) && AdvancedInteractionState.InputOptions[Index].bRequiresStableSignal; }
	bool DoesAdvancedInteractionInputOptionAdvanceObjectiveForTests(int32 Index) const { return AdvancedInteractionState.InputOptions.IsValidIndex(Index) && AdvancedInteractionState.InputOptions[Index].bAdvancesObjective; }
	int32 GetAdvancedInteractionStepTrackCountForTests() const { return AdvancedInteractionState.StepTrack.Num(); }
	FName GetAdvancedInteractionStepTrackInputIdForTests(int32 Index) const { return AdvancedInteractionState.StepTrack.IsValidIndex(Index) ? AdvancedInteractionState.StepTrack[Index].InputId : NAME_None; }
	FText GetAdvancedInteractionStepTrackLabelForTests(int32 Index) const { return AdvancedInteractionState.StepTrack.IsValidIndex(Index) ? AdvancedInteractionState.StepTrack[Index].DisplayLabel : FText::GetEmpty(); }
	FText GetAdvancedInteractionStepTrackKeyHintForTests(int32 Index) const { return AdvancedInteractionState.StepTrack.IsValidIndex(Index) ? AdvancedInteractionState.StepTrack[Index].KeyHint : FText::GetEmpty(); }
	int32 GetAdvancedInteractionStepTrackVisualSlotForTests(int32 Index) const { return AdvancedInteractionState.StepTrack.IsValidIndex(Index) ? AdvancedInteractionState.StepTrack[Index].VisualSlotIndex : INDEX_NONE; }
	bool IsAdvancedInteractionStepTrackCompleteForTests(int32 Index) const { return AdvancedInteractionState.StepTrack.IsValidIndex(Index) && AdvancedInteractionState.StepTrack[Index].bComplete; }
	bool IsAdvancedInteractionStepTrackActiveForTests(int32 Index) const { return AdvancedInteractionState.StepTrack.IsValidIndex(Index) && AdvancedInteractionState.StepTrack[Index].bActive; }
	bool IsAdvancedInteractionStepTrackPreviewForTests(int32 Index) const { return AdvancedInteractionState.StepTrack.IsValidIndex(Index) && AdvancedInteractionState.StepTrack[Index].bPreview; }
	float GetAdvancedInteractionProgressForTests() const { return AdvancedInteractionState.ProgressFraction; }
	float GetAdvancedInteractionTimingForTests() const { return AdvancedInteractionState.TimingFraction; }
	float GetAdvancedInteractionTimingWindowStartForTests() const { return AdvancedInteractionState.TimingWindowStartFraction; }
	float GetAdvancedInteractionTimingWindowEndForTests() const { return AdvancedInteractionState.TimingWindowEndFraction; }
	float GetAdvancedInteractionStabilityForTests() const { return AdvancedInteractionState.StabilityFraction; }
	float GetAdvancedInteractionSignalBalanceForTests() const { return AdvancedInteractionState.SignalBalanceFraction; }
	float GetAdvancedInteractionSignalTargetBalanceForTests() const { return AdvancedInteractionState.SignalTargetBalanceFraction; }
	float GetAdvancedInteractionSpectralConfidenceForTests() const { return AdvancedInteractionState.SpectralConfidenceFraction; }
	float GetAdvancedInteractionSpectralNoiseForTests() const { return AdvancedInteractionState.SpectralNoiseFraction; }
	float GetAdvancedInteractionPerformanceGradeForTests() const { return AdvancedInteractionState.PerformanceGradeFraction; }
	float GetAdvancedInteractionInputPrecisionForTests() const { return AdvancedInteractionState.InputPrecisionFraction; }
	float GetAdvancedInteractionDeviceLoadForTests() const { return AdvancedInteractionState.DeviceLoadFraction; }
	float GetAdvancedInteractionRouteFlowForTests() const { return AdvancedInteractionState.RouteFlowFraction; }
	float GetAdvancedInteractionHazardPressureForTests() const { return AdvancedInteractionState.HazardPressureFraction; }
	float GetAdvancedInteractionTargetAlignmentForTests() const { return AdvancedInteractionState.TargetAlignmentFraction; }
	int32 GetAdvancedInteractionActiveInputSlotForTests() const { return AdvancedInteractionState.ActiveInputSlotIndex; }
	bool IsAdvancedInteractionTimingWindowOpenForTests() const { return AdvancedInteractionState.bTimingWindowOpen; }
	bool IsAdvancedInteractionPausedForTests() const { return AdvancedInteractionState.bPaused; }
	bool DidAdvancedInteractionRecentlySucceedForTests() const { return AdvancedInteractionState.bRecentSuccess; }
	bool DidAdvancedInteractionRecentlyFailForTests() const { return AdvancedInteractionState.bRecentFailure; }
	int32 GetAdvancedInteractionStepIndexForTests() const { return AdvancedInteractionState.StepIndex; }
	int32 GetAdvancedInteractionRequiredStepCountForTests() const { return AdvancedInteractionState.RequiredStepCount; }
	int32 GetAdvancedInteractionComboCountForTests() const { return AdvancedInteractionState.ComboCount; }
	int32 GetAdvancedInteractionMistakeCountForTests() const { return AdvancedInteractionState.MistakeCount; }
	FText GetAdvancedInteractionControlHintForTests() const { return BuildAdvancedInteractionControlHint(); }
	FText GetAdvancedInteractionTacticalSummaryForTests() const { return BuildAdvancedInteractionTacticalSummary(); }
	FText GetAdvancedInteractionOutcomeLineForTests() const { return BuildAdvancedInteractionOutcomeLine(); }
	EHorrorObjectiveFeedbackSeverity GetAdvancedInteractionOutcomeSeverityForTests() const { return ResolveAdvancedInteractionOutcomeSeverity(); }

private:
	void DrawCurrentObjective(float CanvasWidth);
	float ComputeObjectiveTrackerPanelHeight() const;
	FText BuildObjectiveNavigationDiagnosticText() const;
	FText BuildObjectiveNavigationRouteText() const;
	FText BuildObjectiveChecklistActionLine(const FHorrorObjectiveChecklistItem& Item) const;
	FText BuildObjectiveChecklistInstrumentLine(const FHorrorObjectiveChecklistItem& Item) const;
	float ComputeObjectiveChecklistTelemetryFraction(const FHorrorObjectiveChecklistItem& Item) const;
	void DrawAdvancedInteractionPanel(float CanvasWidth, float CanvasHeight);
	void DrawCircuitWiringPanel(float PanelX, float PanelY, float PanelWidth, float PanelHeight, UFont* Font, float WorldSeconds);
	void DrawGearCalibrationPanel(float PanelX, float PanelY, float PanelWidth, float PanelHeight, UFont* Font, float WorldSeconds);
	void DrawSpectralScanPanel(float PanelX, float PanelY, float PanelWidth, float PanelHeight, UFont* Font, float WorldSeconds);
	void DrawSignalTuningPanel(float PanelX, float PanelY, float PanelWidth, float PanelHeight, UFont* Font, float WorldSeconds);
	void DrawAdvancedInteractionHeader(float PanelX, float PanelY, float PanelWidth, UFont* Font, const FLinearColor& AccentColor);
	void DrawAdvancedInteractionStepTrack(float PanelX, float PanelY, float PanelWidth, UFont* Font, const FLinearColor& AccentColor);
	void DrawAdvancedInteractionProgress(float PanelX, float PanelY, float PanelWidth, UFont* Font, const FLinearColor& AccentColor);
	FText BuildAdvancedInteractionControlHint() const;
	FText BuildAdvancedInteractionTacticalSummary() const;
	FText BuildAdvancedInteractionOutcomeLine() const;
	EHorrorObjectiveFeedbackSeverity ResolveAdvancedInteractionOutcomeSeverity() const;
	void DrawLine(const FVector2D& Start, const FVector2D& End, const FLinearColor& Color, float Thickness = 2.0f);
	void DrawCircleLines(const FVector2D& Center, float Radius, const FLinearColor& Color, int32 Segments = 48, float Thickness = 2.0f);
	void DrawRotatedRect(const FVector2D& Center, const FVector2D& Size, float AngleRadians, const FLinearColor& Color);
	void DrawAnomalyCaptureStatus(float CanvasWidth, float CanvasHeight);
	void DrawObjectiveToast(float CanvasWidth);
	void DrawInteractionPrompt(float CanvasWidth, float CanvasHeight);
	void DrawTransientMessage(float CanvasWidth, float CanvasHeight);
	void DrawAutosaveIndicator(float CanvasWidth);
	void DrawSurvivalStatus(float CanvasWidth);
	void DrawPasswordPanel(float CanvasWidth, float CanvasHeight);
	void DrawPauseMenu(float CanvasWidth, float CanvasHeight);
	void DrawNotesJournal(float CanvasWidth, float CanvasHeight);
	void DrawDay1CompletionOverlay(float CanvasWidth, float CanvasHeight);
	void DrawPanel(float X, float Y, float Width, float Height, const FLinearColor& BackgroundColor, const FLinearColor& AccentColor);
	float GetWorldSeconds() const;

	UPROPERTY(Transient)
	FText InteractionPrompt;

	UPROPERTY(Transient)
	FText CurrentObjective;

	UPROPERTY(Transient)
	FText ObjectiveTrackerTitle;

	UPROPERTY(Transient)
	FText ObjectiveTrackerDetail;

	UPROPERTY(Transient)
	FText ObjectiveTrackerProgressLabel;

	UPROPERTY(Transient)
	FText ObjectiveTrackerInteractionLabel;

	UPROPERTY(Transient)
	FText ObjectiveTrackerMissionContext;

	UPROPERTY(Transient)
	FText ObjectiveTrackerFailureStakes;

	UPROPERTY(Transient)
	FText ObjectiveNavigation;

	UPROPERTY(Transient)
	FHorrorObjectiveNavigationState ObjectiveNavigationState;

	UPROPERTY(Transient)
	FText AnomalyCaptureStatus;

	UPROPERTY(Transient)
	FText ToastTitle;

	UPROPERTY(Transient)
	FText ToastHint;

	UPROPERTY(Transient)
	EHorrorObjectiveFeedbackSeverity ToastSeverity = EHorrorObjectiveFeedbackSeverity::Info;

	UPROPERTY(Transient)
	FText TransientMessage;

	UPROPERTY(Transient)
	FText AutosaveIndicatorText;

	UPROPERTY(Transient)
	FText Day1CompletionTitle;

	UPROPERTY(Transient)
	FText Day1CompletionHint;

	UPROPERTY(Transient)
	FText PasswordDoorName;

	UPROPERTY(Transient)
	FText PasswordHint;

	UPROPERTY(Transient)
	FText NotesJournalEmptyText;

	UPROPERTY(Transient)
	TArray<FHorrorNoteMetadata> NotesJournalEntries;

	UPROPERTY(Transient)
	TArray<FHorrorObjectiveChecklistItem> ObjectiveChecklistItems;

	UPROPERTY(Transient)
	TArray<FHorrorObjectiveChecklistItem> ObjectiveGraphItems;

	UPROPERTY(Transient)
	FText ObjectiveGraphOverflowText;

	UPROPERTY(Transient)
	FHorrorAdvancedInteractionHUDState AdvancedInteractionState;

	UPROPERTY(Transient)
	FHorrorInteractionContext InteractionContext;

	FLinearColor TransientMessageColor = FLinearColor::White;
	float ToastExpireWorldSeconds = -1.0f;
	float TransientMessageExpireWorldSeconds = -1.0f;
	float AutosaveIndicatorExpireWorldSeconds = -1.0f;
	int32 PasswordEnteredDigits = 0;
	int32 PasswordRequiredDigits = 0;
	int32 ObjectiveGraphOverflowCount = 0;
	float PauseMouseSensitivity = 1.0f;
	float PauseMasterVolume = 1.0f;
	float PauseBrightness = 1.0f;
	float StatusFearPercent = 0.0f;
	float StatusSprintPercent = 1.0f;
	float BodycamBatteryPercent = 100.0f;
	float ObjectiveTrackerProgressFraction = 0.0f;
	float AnomalyCaptureProgressFraction = 0.0f;
	FName ObjectiveTrackerActiveChapterId = NAME_None;
	FName ObjectiveTrackerActiveObjectiveId = NAME_None;
	EDay1PauseMenuSelection PauseMenuSelection = EDay1PauseMenuSelection::Resume;
	bool bObjectiveTrackerUrgent = false;
	bool bObjectiveTrackerRequiresRecording = false;
	bool bObjectiveTrackerOpensInteractionPanel = false;
	bool bAnomalyCaptureStatusVisible = false;
	bool bAnomalyCaptureLocked = false;
	bool bAnomalyCaptureRequiresRecording = false;
	bool bStatusBodycamAcquired = false;
	bool bStatusBodycamEnabled = false;
	bool bStatusRecording = false;
	bool bStatusDanger = false;
	bool bBodycamBatteryVisible = false;
	bool bBodycamBatteryLow = false;
	bool bPasswordPromptVisible = false;
	bool bPauseMenuVisible = false;
	bool bNotesJournalVisible = false;
	bool bDay1CompletionOverlayVisible = false;
};
