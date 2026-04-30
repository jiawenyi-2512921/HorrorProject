// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Game/HorrorAdvancedInteractionTypes.h"
#include "Game/HorrorFoundFootageContract.h"
#include "GameFramework/HUD.h"
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
	void ClearInteractionPrompt();

	void SetCurrentObjective(const FText& ObjectiveText);
	void SetObjectiveTracker(const FHorrorObjectiveTrackerSnapshot& Snapshot);
	void SetObjectiveNavigation(const FText& NavigationText);
	void ClearObjectiveNavigation();
	void ShowObjectiveToast(const FText& TitleText, const FText& HintText, float DisplaySeconds = 5.0f);
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
	float GetObjectiveTrackerProgressFractionForTests() const { return ObjectiveTrackerProgressFraction; }
	bool IsObjectiveTrackerUrgentForTests() const { return bObjectiveTrackerUrgent; }
	bool IsObjectiveTrackerRecordingRequiredForTests() const { return bObjectiveTrackerRequiresRecording; }
	int32 GetObjectiveChecklistItemCountForTests() const { return ObjectiveChecklistItems.Num(); }
	FText GetObjectiveChecklistItemLabelForTests(int32 Index) const { return ObjectiveChecklistItems.IsValidIndex(Index) ? ObjectiveChecklistItems[Index].Label : FText::GetEmpty(); }
	bool IsObjectiveChecklistItemCompleteForTests(int32 Index) const { return ObjectiveChecklistItems.IsValidIndex(Index) && ObjectiveChecklistItems[Index].bComplete; }
	bool IsObjectiveChecklistItemActiveForTests(int32 Index) const { return ObjectiveChecklistItems.IsValidIndex(Index) && ObjectiveChecklistItems[Index].bActive; }
	bool DoesObjectiveChecklistItemRequireRecordingForTests(int32 Index) const { return ObjectiveChecklistItems.IsValidIndex(Index) && ObjectiveChecklistItems[Index].bRequiresRecording; }
	bool IsAnomalyCaptureStatusVisibleForTests() const { return bAnomalyCaptureStatusVisible; }
	FText GetAnomalyCaptureStatusForTests() const { return AnomalyCaptureStatus; }
	float GetAnomalyCaptureProgressForTests() const { return AnomalyCaptureProgressFraction; }
	bool IsAnomalyCaptureLockedForTests() const { return bAnomalyCaptureLocked; }
	bool DoesAnomalyCaptureRequireRecordingForTests() const { return bAnomalyCaptureRequiresRecording; }
	FText GetObjectiveNavigationForTests() const { return ObjectiveNavigation; }
	FText GetInteractionPromptForTests() const { return InteractionPrompt; }
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
	float GetAdvancedInteractionProgressForTests() const { return AdvancedInteractionState.ProgressFraction; }
	float GetAdvancedInteractionTimingForTests() const { return AdvancedInteractionState.TimingFraction; }
	bool IsAdvancedInteractionTimingWindowOpenForTests() const { return AdvancedInteractionState.bTimingWindowOpen; }
	bool IsAdvancedInteractionPausedForTests() const { return AdvancedInteractionState.bPaused; }
	int32 GetAdvancedInteractionStepIndexForTests() const { return AdvancedInteractionState.StepIndex; }
	int32 GetAdvancedInteractionRequiredStepCountForTests() const { return AdvancedInteractionState.RequiredStepCount; }

private:
	void DrawCurrentObjective(float CanvasWidth);
	void DrawAdvancedInteractionPanel(float CanvasWidth, float CanvasHeight);
	void DrawCircuitWiringPanel(float PanelX, float PanelY, float PanelWidth, float PanelHeight, UFont* Font, float WorldSeconds);
	void DrawGearCalibrationPanel(float PanelX, float PanelY, float PanelWidth, float PanelHeight, UFont* Font, float WorldSeconds);
	void DrawAdvancedInteractionHeader(float PanelX, float PanelY, float PanelWidth, UFont* Font, const FLinearColor& AccentColor);
	void DrawAdvancedInteractionProgress(float PanelX, float PanelY, float PanelWidth, UFont* Font, const FLinearColor& AccentColor);
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
	FText ObjectiveNavigation;

	UPROPERTY(Transient)
	FText AnomalyCaptureStatus;

	UPROPERTY(Transient)
	FText ToastTitle;

	UPROPERTY(Transient)
	FText ToastHint;

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
	FHorrorAdvancedInteractionHUDState AdvancedInteractionState;

	FLinearColor TransientMessageColor = FLinearColor::White;
	float ToastExpireWorldSeconds = -1.0f;
	float TransientMessageExpireWorldSeconds = -1.0f;
	float AutosaveIndicatorExpireWorldSeconds = -1.0f;
	int32 PasswordEnteredDigits = 0;
	int32 PasswordRequiredDigits = 0;
	float PauseMouseSensitivity = 1.0f;
	float PauseMasterVolume = 1.0f;
	float PauseBrightness = 1.0f;
	float StatusFearPercent = 0.0f;
	float StatusSprintPercent = 1.0f;
	float BodycamBatteryPercent = 100.0f;
	float ObjectiveTrackerProgressFraction = 0.0f;
	float AnomalyCaptureProgressFraction = 0.0f;
	EDay1PauseMenuSelection PauseMenuSelection = EDay1PauseMenuSelection::Resume;
	bool bObjectiveTrackerUrgent = false;
	bool bObjectiveTrackerRequiresRecording = false;
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
