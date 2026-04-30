// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameFramework/PlayerController.h"
#include "UI/Day1SliceHUD.h"
#include "HorrorPlayerController.generated.h"

class UInputMappingContext;
class UHorrorUI;
class UUserWidget;
class ADoorInteractable;
class UHorrorEventBusSubsystem;
class ADay1SliceHUD;
class UGameSettingsSubsystem;
class UControlSettings;
class UHorrorAudioSettings;
class UGraphicsSettings;
class UNoteRecorderComponent;
class AHorrorGameModeBase;
class AHorrorCampaignObjectiveActor;
class AHorrorPlayerCharacter;
class AFoundFootageObjectiveInteractable;
struct FKey;
struct FInputKeyEventArgs;
struct FHorrorEventMessage;

/**
 *  Player Controller for a first person horror game
 *  Manages input mappings
 *  Manages UI
 */
UCLASS()
class HORRORPROJECT_API AHorrorPlayerController : public APlayerController
{
	GENERATED_BODY()

protected:

	/** Type of UI widget to spawn */
	UPROPERTY(EditAnywhere, Category="Horror|UI")
	TSubclassOf<UHorrorUI> HorrorUIClass;

	/** Temporary prototype HUD toggle. Keep disabled for playable smoke builds. */
	UPROPERTY(EditAnywhere, Category="Horror|UI")
	bool bAutoCreateHorrorUI = false;

	/** Pointer to the UI widget */
	TObjectPtr<UHorrorUI> HorrorUI;

public:

	/** Constructor */
	AHorrorPlayerController();

	/** Restamps state-derived UI after checkpoint restore without creating new widgets. */
	void RestampCheckpointLoadedUIState();

	/** Ensures the Day 1 slice uses the native Chinese HUD instead of older prototype widgets. */
	void EnsureDay1NativeHUD();

	/** Starts lightweight numeric password entry for a locked door. */
	void BeginDoorPasswordEntry(ADoorInteractable* Door);

	/** Clears transient Day1 modal/input state after failure or checkpoint restore. */
	void ResetDay1ModalInputState();

	void ShowPlayerMessage(const FText& MessageText, const FLinearColor& MessageColor = FLinearColor::White, float DisplaySeconds = 2.5f);
	bool TrySubmitActiveAdvancedInteractionExpectedInput();

	bool IsAwaitingDoorPassword() const { return PendingPasswordDoor.IsValid(); }
	FString GetDoorPasswordBufferForTests() const { return DoorPasswordBuffer; }
	bool IsDay1PauseMenuOpenForTests() const { return bDay1PauseMenuOpen; }
	bool IsDay1NotesJournalOpenForTests() const { return bDay1NotesJournalOpen; }
	bool IsDay1CompletionInputLockedForTests() const { return bDay1CompletionInputLocked; }
	EDay1PauseMenuSelection GetDay1PauseMenuSelectionForTests() const { return Day1PauseMenuSelection; }
#if WITH_DEV_AUTOMATION_TESTS
	bool HandleInputKeyForTests(const FKey& Key, EInputEvent Event = IE_Pressed);
	void BindObjectiveEventBusForTests();
	void RefreshDay1HUDStateForTests();
	bool UpdateDay1RuntimeStateForTests(float DeltaSeconds);
	bool IsBoundToNoteRecorderForTests(const UNoteRecorderComponent* NoteRecorder) const;
	bool HasNoteRecordedDelegateForTests(const UNoteRecorderComponent* NoteRecorder);
	void SetActiveAdvancedInteractionObjectiveForTests(AHorrorCampaignObjectiveActor* ObjectiveActor);
	AHorrorCampaignObjectiveActor* GetActiveAdvancedInteractionObjectiveForTests() const;
	int32 GetNoteRecordedFeedbackCountForTests() const { return NoteRecordedFeedbackCountForTests; }
	int32 GetDefaultMappingContextCountForTests() const { return DefaultMappingContexts.Num(); }
	int32 GetMobileExcludedMappingContextCountForTests() const { return MobileExcludedMappingContexts.Num(); }
#endif

protected:

	/** Input Mapping Contexts */
	UPROPERTY(EditAnywhere, Category ="Input|Input Mappings")
	TArray<UInputMappingContext*> DefaultMappingContexts;

	/** Input Mapping Contexts */
	UPROPERTY(EditAnywhere, Category="Input|Input Mappings")
	TArray<UInputMappingContext*> MobileExcludedMappingContexts;

	/** Mobile controls widget to spawn */
	UPROPERTY(EditAnywhere, Category="Input|Touch Controls")
	TSubclassOf<UUserWidget> MobileControlsWidgetClass;

	/** Pointer to the mobile controls widget */
	TObjectPtr<UUserWidget> MobileControlsWidget;

	/** Gameplay Initialization */
	virtual void BeginPlay() override;

	virtual void Tick(float DeltaSeconds) override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/** Possessed pawn initialization */
	virtual void OnPossess(APawn* aPawn) override;
	virtual void OnUnPossess() override;

	/** Input mapping context setup */
	virtual void SetupInputComponent() override;

	/** Captures numeric password input while a locked door is awaiting a code. */
	virtual bool InputKey(const FInputKeyEventArgs& Params) override;

	/** Adds the configured mapping contexts to the local enhanced input subsystem once. */
	void ApplyInputMappingContexts();

private:

	ADay1SliceHUD* GetDay1SliceHUD() const;
	bool ShouldUseDay1NativeHUD() const;
	void BindObjectiveEventBus();
	void UnbindObjectiveEventBus();
	void HandleObjectiveEventPublished(const FHorrorEventMessage& Message);
	void BindPawnNoteRecorder(APawn* PawnOverride = nullptr);
	void UnbindPawnNoteRecorder();
	UFUNCTION()
	void HandlePawnNoteRecorded(FName NoteId, int32 TotalRecordedNotes);
	bool UpdateDay1RuntimeState(float DeltaSeconds);
	void RefreshDay1HUDState();
	void RefreshAdvancedInteractionHUD(ADay1SliceHUD& Day1HUD, const AHorrorPlayerCharacter* HorrorPlayerCharacter);
	void RefreshInteractionPrompt();
	void RefreshAnomalyCaptureHUD(ADay1SliceHUD& Day1HUD, const AHorrorPlayerCharacter* HorrorPlayerCharacter, const AHorrorGameModeBase* HorrorGameMode);
	AHorrorCampaignObjectiveActor* ResolveActiveAdvancedInteractionObjective(const AHorrorPlayerCharacter* HorrorPlayerCharacter = nullptr);
	AFoundFootageObjectiveInteractable* ResolveFocusedFirstAnomalyTarget(const AHorrorPlayerCharacter& HorrorPlayerCharacter) const;
	bool IsFocusedFirstAnomalyTarget(const AFoundFootageObjectiveInteractable* ObjectiveActor) const;
	bool TryAutoCaptureFocusedAnomaly(float DeltaSeconds);
	FText BuildObjectivePrompt() const;
	bool TryHandleAdvancedInteractionKey(const FKey& Key);
	void ClearDoorPasswordEntry();
	void CancelDoorPasswordEntry();
	void ShowDoorPasswordPrompt() const;
	bool HandleDoorPasswordKey(const FKey& Key);
	void ApplyDay1CompletionInputLock();
	void OpenDay1PauseMenu();
	void CloseDay1PauseMenu();
	void OpenDay1NotesJournal();
	void CloseDay1NotesJournal();
	bool ToggleDay1NotesJournal();
	void RefreshDay1NotesJournalHUD();
	UNoteRecorderComponent* GetPawnNoteRecorder() const;
	bool HandleDay1PauseMenuKey(const FKey& Key);
	void MoveDay1PauseMenuSelection(int32 Direction);
	void AdjustDay1PauseMenuValue(float Direction);
	void RefreshDay1PauseMenuHUD();
	UGameSettingsSubsystem* GetGameSettingsSubsystem() const;
	UControlSettings* GetControlSettings() const;
	UHorrorAudioSettings* GetAudioSettings() const;
	UGraphicsSettings* GetGraphicsSettings() const;
	static bool IsDay1AutosaveMilestoneState(FGameplayTag StateTag);
	static bool TryGetDigitForKey(const FKey& Key, TCHAR& OutDigit);

	/** Prevents duplicate IMC registration across BeginPlay / SetupInputComponent. */
	bool bInputContextsApplied = false;

	TWeakObjectPtr<ADoorInteractable> PendingPasswordDoor;
	FString DoorPasswordBuffer;
	bool bDay1PauseMenuOpen = false;
	bool bDay1NotesJournalOpen = false;
	bool bDay1CompletionInputLocked = false;
	EDay1PauseMenuSelection Day1PauseMenuSelection = EDay1PauseMenuSelection::Resume;
	TWeakObjectPtr<UHorrorEventBusSubsystem> BoundObjectiveEventBus;
	FDelegateHandle ObjectiveEventHandle;
	TWeakObjectPtr<UNoteRecorderComponent> BoundNoteRecorder;
	FDelegateHandle NoteRecordedHandle;
	TWeakObjectPtr<AFoundFootageObjectiveInteractable> LockedAnomalyTarget;
	TWeakObjectPtr<AHorrorCampaignObjectiveActor> ActiveAdvancedInteractionObjective;
	float FocusedAnomalyLockSeconds = 0.0f;
#if WITH_DEV_AUTOMATION_TESTS
	int32 NoteRecordedFeedbackCountForTests = 0;
#endif

};
