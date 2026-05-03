// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Game/HorrorAdvancedInteractionTypes.h"
#include "Game/HorrorCampaign.h"
#include "GameFramework/Actor.h"
#include "Interaction/InteractableInterface.h"
#include "HorrorCampaignObjectiveActor.generated.h"

class UBoxComponent;
class UNiagaraComponent;
class UPointLightComponent;
class UStaticMeshComponent;
class UTextRenderComponent;

UCLASS(BlueprintType, Blueprintable, ClassGroup=(Horror))
class HORRORPROJECT_API AHorrorCampaignObjectiveActor : public AActor, public IInteractableInterface
{
	GENERATED_BODY()

public:
	AHorrorCampaignObjectiveActor();

	virtual void Tick(float DeltaTime) override;

	void ConfigureObjective(FName InChapterId, const FHorrorCampaignObjectiveDefinition& InObjectiveDefinition);

	virtual bool CanInteract_Implementation(AActor* InstigatorActor, const FHitResult& Hit) const override;
	virtual bool Interact_Implementation(AActor* InstigatorActor, const FHitResult& Hit) override;

	UFUNCTION(BlueprintPure, Category="Horror|Campaign")
	FText GetInteractionPromptText() const;

	UFUNCTION(BlueprintPure, Category="Horror|Campaign")
	FName GetChapterId() const { return ChapterId; }

	UFUNCTION(BlueprintPure, Category="Horror|Campaign")
	FName GetObjectiveId() const { return ObjectiveDefinition.ObjectiveId; }

	UFUNCTION(BlueprintPure, Category="Horror|Campaign")
	bool IsCompleted() const { return bCompleted; }

	UFUNCTION(BlueprintPure, Category="Horror|Campaign")
	bool IsAvailableForInteraction() const { return bAvailableForInteraction; }

	UFUNCTION(BlueprintPure, Category="Horror|Campaign")
	bool IsPresentationVisible() const { return bPresentationVisible; }

	UFUNCTION(BlueprintPure, Category="Horror|Campaign")
	int32 GetRequiredInteractionCount() const { return RequiredInteractionCount; }

	UFUNCTION(BlueprintPure, Category="Horror|Campaign")
	int32 GetInteractionProgressCount() const { return InteractionProgressCount; }

	UFUNCTION(BlueprintPure, Category="Horror|Campaign")
	EHorrorCampaignInteractionMode GetInteractionMode() const { return ObjectiveDefinition.InteractionMode; }

	const FHorrorCampaignObjectiveDefinition& GetObjectiveDefinitionForPresentation() const { return ObjectiveDefinition; }

	UFUNCTION(BlueprintPure, Category="Horror|Campaign")
	int32 GetObjectiveBeatCount() const { return ObjectiveDefinition.ObjectiveBeats.Num(); }

	UFUNCTION(BlueprintPure, Category="Horror|Campaign")
	int32 GetCurrentObjectiveBeatIndex() const;

	UFUNCTION(BlueprintPure, Category="Horror|Campaign")
	FText GetCurrentObjectiveBeatLabel() const;

	UFUNCTION(BlueprintPure, Category="Horror|Campaign")
	FText GetCurrentObjectiveBeatDetail() const;

	UFUNCTION(BlueprintPure, Category="Horror|Campaign")
	bool IsCurrentObjectiveBeatUrgent() const;

	UFUNCTION(BlueprintPure, Category="Horror|Campaign")
	bool DoesCurrentObjectiveBeatRequireRecording() const;

	UFUNCTION(BlueprintPure, Category="Horror|Campaign|Advanced Interaction")
	bool IsAdvancedInteractionActive() const { return bAdvancedInteractionActive; }

	UFUNCTION(BlueprintPure, Category="Horror|Campaign|Advanced Interaction")
	bool IsObjectiveFailedRetryable() const { return bObjectiveFailedRetryable; }

	UFUNCTION(BlueprintPure, Category="Horror|Campaign|Advanced Interaction")
	float GetAdvancedInteractionProgressFraction() const { return AdvancedInteractionProgressFraction; }

	UFUNCTION(BlueprintPure, Category="Horror|Campaign|Advanced Interaction")
	FName GetExpectedAdvancedInputId() const { return ExpectedAdvancedInputId; }

	UFUNCTION(BlueprintPure, Category="Horror|Campaign|Advanced Interaction")
	float GetAdvancedInteractionTimingFraction() const { return AdvancedInteractionTimingFraction; }

	UFUNCTION(BlueprintPure, Category="Horror|Campaign|Advanced Interaction")
	float GetAdvancedInteractionStabilityFraction() const { return AdvancedInteractionStabilityFraction; }

	UFUNCTION(BlueprintPure, Category="Horror|Campaign|Advanced Interaction")
	int32 GetAdvancedInteractionComboCount() const { return AdvancedInteractionComboCount; }

	UFUNCTION(BlueprintPure, Category="Horror|Campaign|Advanced Interaction")
	int32 GetAdvancedInteractionMistakeCount() const { return AdvancedInteractionMistakeCount; }

	UFUNCTION(BlueprintPure, Category="Horror|Campaign|Advanced Interaction")
	bool IsAdvancedInteractionTimingWindowOpen() const;

	UFUNCTION(BlueprintPure, Category="Horror|Campaign|Advanced Interaction")
	FText GetAdvancedInteractionFeedbackText() const { return AdvancedInteractionFeedbackText; }

	UFUNCTION(BlueprintPure, Category="Horror|Campaign|Advanced Interaction")
	float GetAdvancedInteractionPauseRemainingSeconds() const { return AdvancedInteractionPauseRemainingSeconds; }

	UFUNCTION(BlueprintPure, Category="Horror|Campaign|Advanced Interaction")
	FHorrorAdvancedInteractionHUDState BuildAdvancedInteractionHUDState() const;

	UFUNCTION(BlueprintPure, Category="Horror|Campaign|Advanced Interaction")
	FHorrorAdvancedInteractionOutcome GetLastAdvancedInteractionOutcome() const { return LastAdvancedInteractionOutcome; }

	UFUNCTION(BlueprintPure, Category="Horror|Campaign")
	FHorrorCampaignObjectiveRuntimeState BuildObjectiveRuntimeState() const;

	UFUNCTION(BlueprintPure, Category="Horror|Campaign|Save")
	bool HasPersistentObjectiveRuntimeState() const;

	UFUNCTION(BlueprintPure, Category="Horror|Campaign|Save")
	FHorrorCampaignObjectiveSaveState ExportObjectiveSaveState() const;

	UFUNCTION(BlueprintCallable, Category="Horror|Campaign|Save")
	void ImportObjectiveSaveState(const FHorrorCampaignObjectiveSaveState& SaveState, AActor* RestoredInstigator);

	UFUNCTION(BlueprintCallable, Category="Horror|Campaign|Advanced Interaction")
	bool SubmitAdvancedInteractionInput(FName InputId, AActor* InstigatorActor);

	UFUNCTION(BlueprintCallable, Category="Horror|Campaign|Advanced Interaction")
	bool SubmitAdvancedInteractionCommand(const FHorrorAdvancedInteractionInputCommand& Command, AActor* InstigatorActor);

	UFUNCTION(BlueprintCallable, Category="Horror|Campaign|Advanced Interaction")
	bool PromptForExplicitAdvancedInteractionSelection();

	UFUNCTION(BlueprintPure, Category="Horror|Campaign")
	bool IsTimedObjectiveActive() const { return bTimedObjectiveActive; }

	UFUNCTION(BlueprintPure, Category="Horror|Campaign")
	float GetTimedObjectiveDurationSeconds() const { return TimedObjectiveDurationSeconds; }

	UFUNCTION(BlueprintPure, Category="Horror|Campaign")
	float GetTimedObjectiveRemainingSeconds() const { return TimedObjectiveRemainingSeconds; }

	UFUNCTION(BlueprintPure, Category="Horror|Campaign")
	bool HasActiveEscapeDestination() const { return bTimedObjectiveActive && bTimedObjectiveUsesEscapeDestination; }

	UFUNCTION(BlueprintPure, Category="Horror|Campaign|Relic")
	bool IsRecoverRelicAwaitingDelivery() const { return bRecoverRelicAwaitingDelivery; }

	UFUNCTION(BlueprintPure, Category="Horror|Campaign")
	FVector GetActiveEscapeDestinationWorldLocation() const { return TimedObjectiveEscapeDestinationLocation; }

	UFUNCTION(BlueprintPure, Category="Horror|Campaign|Relic")
	FVector GetRecoverRelicDeliveryWorldLocation() const { return RecoverRelicDeliveryLocation; }

	UFUNCTION(BlueprintPure, Category="Horror|Campaign")
	float GetEscapeCompletionRadius() const { return FMath::Max(50.0f, ObjectiveDefinition.EscapeCompletionRadius); }

	UFUNCTION(BlueprintPure, Category="Horror|Campaign|Relic")
	float GetRecoverRelicDeliveryCompletionRadius() const { return FMath::Max(50.0f, ObjectiveDefinition.RelicDeliveryCompletionRadius); }

	UFUNCTION(BlueprintCallable, Category="Horror|Campaign")
	bool AbortTimedObjectiveForRecovery(AActor* InstigatorActor, FName FailureCause, FText FailureTitle, FText RetryHint);

	UFUNCTION(BlueprintCallable, Category="Horror|Campaign")
	void RefreshObjectiveState();

	bool IsRecoverRelicAwaitingDeliveryForTests() const { return bRecoverRelicAwaitingDelivery; }

	UBoxComponent* GetInteractionBoundsForTests() const { return InteractionBounds.Get(); }

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Horror|Campaign")
	TObjectPtr<UBoxComponent> InteractionBounds;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Horror|Campaign")
	TObjectPtr<UStaticMeshComponent> VisualMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Horror|Campaign")
	TObjectPtr<UTextRenderComponent> LabelText;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Horror|Campaign")
	TObjectPtr<UPointLightComponent> MarkerLight;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Horror|Campaign")
	TObjectPtr<UNiagaraComponent> ObjectiveVFX;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Horror|Campaign")
	FName ChapterId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Horror|Campaign")
	FHorrorCampaignObjectiveDefinition ObjectiveDefinition;

private:
	bool IsTimedSurvivalObjective() const;
	bool UsesAdvancedInteraction() const;
	bool IsAuthoredCampaignObjective() const;
	const FHorrorCampaignObjectiveBeat* GetCurrentObjectiveBeat() const;
	FText BuildObjectiveLockReasonText() const;
	bool HasConfiguredEscapeDestination() const;
	bool IsCarryReturnRelicObjective() const;
	bool HasConfiguredRelicDeliveryAnchor() const;
	FVector ResolveEscapeDestinationWorldLocation(AActor* InstigatorActor = nullptr) const;
	FVector ResolveRecoverRelicDeliveryWorldLocation(AActor* InstigatorActor = nullptr) const;
	float ResolveRecoverRelicDeliveryDistanceMeters(AActor* InstigatorActor = nullptr) const;
	float ResolveRecoverRelicRouteDistanceMeters() const;
	bool BeginRecoverRelicDelivery(AActor* InstigatorActor);
	FTransform ResolveAmbushThreatAnchorTransform(AActor* InstigatorActor) const;
	bool HasTimedObjectiveReachedEscapeDestination() const;
	float GetAdvancedInteractionSuccessProgress() const;
	float GetAdvancedInteractionCueCycleSeconds() const;
	float GetAdvancedInteractionTimingWindowStart() const;
	float GetAdvancedInteractionTimingWindowEnd() const;
	float GetAdvancedInteractionFailureProgressPenalty() const;
	float GetAdvancedInteractionFailureStabilityDamage() const;
	float GetAdvancedInteractionSuccessStabilityRecovery() const;
	float GetAdvancedInteractionFailurePauseSeconds() const;
	float GetAdvancedInteractionRetryAssistFraction() const;
	bool IsSignalTuningMode() const;
	bool IsSpectralScanMode() const;
	bool IsGearCalibrationMode() const;
	bool IsCircuitHazardInput(FName InputId) const;
	bool IsSignalTuningConfirmInput(FName InputId) const;
	bool IsSignalTuningAdjustInput(FName InputId) const;
	bool IsSpectralScanFilterAligned() const;
	float ResolveSpectralScanTargetFocus() const;
	float ResolveInitialSignalTuningBalance() const;
	void AdjustSpectralScanFilter(FName InputId);
	void AdjustSpectralScanFilterByAxis(float AxisValue, float HoldSeconds);
	void AdjustSignalTuningBalance(FName InputId);
	void AdjustSignalTuningBalanceByAxis(float AxisValue, float HoldSeconds);
	bool IsSignalTuningBalanceAligned() const;
	bool IsSpectralScanConfidenceReady() const;
	void StartAdvancedInteraction();
	void ResetAdvancedInteractionState();
	void ResetAdvancedInteractionCue();
	void AdvanceExpectedAdvancedInput();
	FName ResolveGearCalibrationInputForStep(int32 StepIndex) const;
	void MarkAdvancedInteractionSuccess();
	void MarkAdvancedInteractionFailure(float StabilityDamage);
	void SetAdvancedInteractionFault(FName FaultId);
	void ClearAdvancedInteractionFault();
	void ResetLastAdvancedInteractionOutcome();
	void RecordAdvancedInteractionOutcome(
		EHorrorAdvancedInteractionOutcomeKind Kind,
		FName InputId,
		EHorrorAdvancedInteractionFeedbackState FeedbackState,
		bool bConsumesInput,
		bool bAdvancesProgress,
		bool bRetryable = false,
		FName FailureCause = NAME_None,
		FName RecoveryAction = NAME_None,
		FName FaultId = NAME_None);
	bool RecordAdvancedInteractionFailureOutcomeAfterFeedback(
		EHorrorAdvancedInteractionOutcomeKind FailureKind,
		FName InputId,
		FName FailureFaultId,
		AActor* InstigatorActor);
	TArray<FHorrorAdvancedInteractionInputOption> BuildAdvancedInteractionInputOptions() const;
	TArray<FHorrorAdvancedInteractionStepTrackItem> BuildAdvancedInteractionStepTrack() const;
	FText BuildAdvancedInteractionPhaseText() const;
	FText BuildExpectedAdvancedInputLabel() const;
	FText BuildAdvancedInteractionDeviceStatusLabel() const;
	FText BuildAdvancedInteractionRiskLabel() const;
	FText BuildAdvancedInteractionRhythmLabel() const;
	FText BuildAdvancedInteractionNextActionLabel() const;
	FText BuildAdvancedInteractionFailureRecoveryLabel() const;
	FText BuildGearCalibrationChainLabel() const;
	float BuildAdvancedInteractionPerformanceGrade() const;
	float BuildAdvancedInteractionInputPrecision() const;
	float BuildAdvancedInteractionDeviceLoad() const;
	float BuildAdvancedInteractionRouteFlow() const;
	float BuildAdvancedInteractionHazardPressure() const;
	float BuildAdvancedInteractionTargetAlignment() const;
	int32 BuildAdvancedInteractionActiveInputSlot() const;
	EHorrorAdvancedInteractionFeedbackState GetAdvancedInteractionFeedbackState() const;
	FText BuildMultiStepPhaseText() const;
	void ApplyAdvancedInteractionTimingFailure();
	void ApplyAdvancedInteractionWrongInputFailure();
	FText BuildAdvancedInteractionPausedText(bool bExplicitSelection) const;
	FText BuildStandardInteractionDeviceStatusLabel() const;
	FText BuildStandardInteractionNextActionLabel() const;
	FText BuildStandardInteractionFailureRecoveryLabel() const;
	FName GetAdvancedInteractionFailureCause() const;
	bool TryHandleAdvancedInteractionFailureExhausted(AActor* InstigatorActor);
	bool CompleteAdvancedInteraction(AActor* InstigatorActor);
	FText BuildAdvancedInteractionPromptText() const;
	EHorrorAdvancedInteractionOutcomeKind ResolveAdvancedInteractionEventOutcomeKind(bool bSuccess) const;
	void PublishAdvancedInteractionFeedbackEvent(bool bSuccess, AActor* InstigatorActor) const;
	void PublishObjectiveFailedEvent(AActor* InstigatorActor, const FText& FailureTitle, const FText& RetryHint, FName FailureCause, FName RecoveryAction) const;
	void StartTimedObjective(AActor* InstigatorActor);
	void FailTimedObjective(AActor* InstigatorActor);
	void UpdateTimedObjective(float DeltaTime);
	void UpdateAdvancedInteraction(float DeltaTime);
	void RefreshRuntimeTickState();
	bool CompleteObjective(AActor* InstigatorActor);
	void PublishAmbushStartedEvent(AActor* InstigatorActor) const;
	void ApplyObjectiveVisuals();
	void ApplyCompletedVisuals();
	void ApplyHiddenLockedVisuals();
	FColor GetObjectiveColor() const;

	UPROPERTY(Transient)
	bool bCompleted = false;

	UPROPERTY(Transient)
	bool bAvailableForInteraction = true;

	UPROPERTY(Transient)
	bool bPresentationVisible = true;

	UPROPERTY(Transient)
	int32 RequiredInteractionCount = 1;

	UPROPERTY(Transient)
	int32 InteractionProgressCount = 0;

	UPROPERTY(Transient)
	bool bAdvancedInteractionActive = false;

	UPROPERTY(Transient)
	float AdvancedInteractionProgressFraction = 0.0f;

	UPROPERTY(Transient)
	int32 AdvancedInteractionStepIndex = 0;

	UPROPERTY(Transient)
	FName ExpectedAdvancedInputId = NAME_None;

	UPROPERTY(Transient)
	float AdvancedInteractionCueElapsedSeconds = 0.0f;

	UPROPERTY(Transient)
	float AdvancedInteractionTimingFraction = 0.0f;

	UPROPERTY(Transient)
	bool bAdvancedInteractionCueResolved = false;

	UPROPERTY(Transient)
	FText AdvancedInteractionFeedbackText;

	UPROPERTY(Transient)
	float AdvancedInteractionStabilityFraction = 1.0f;

	UPROPERTY(Transient)
	float SignalTuningBalanceFraction = 0.5f;

	UPROPERTY(Transient)
	float SignalTuningTargetBalanceFraction = 0.5f;

	UPROPERTY(Transient)
	float SpectralScanConfidenceFraction = 0.0f;

	UPROPERTY(Transient)
	float SpectralScanNoiseFraction = 0.0f;

	UPROPERTY(Transient)
	float SpectralScanFilterFocusFraction = 0.5f;

	UPROPERTY(Transient)
	float SpectralScanTargetFocusFraction = 0.5f;

	UPROPERTY(Transient)
	int32 AdvancedInteractionComboCount = 0;

	UPROPERTY(Transient)
	int32 AdvancedInteractionMistakeCount = 0;

	UPROPERTY(Transient)
	bool bAdvancedInteractionRecentSuccess = false;

	UPROPERTY(Transient)
	bool bAdvancedInteractionRecentFailure = false;

	UPROPERTY(Transient)
	FName LastAdvancedInteractionFaultId = NAME_None;

	UPROPERTY(Transient)
	FHorrorAdvancedInteractionOutcome LastAdvancedInteractionOutcome;

	UPROPERTY(Transient)
	float AdvancedInteractionPauseRemainingSeconds = 0.0f;

	UPROPERTY(Transient)
	int32 ObjectiveFailureAttemptCount = 0;

	UPROPERTY(Transient)
	bool bObjectiveFailedRetryable = false;

	UPROPERTY(Transient)
	FName LastObjectiveFailureCause = NAME_None;

	UPROPERTY(Transient)
	FName LastObjectiveRecoveryAction = NAME_None;

	UPROPERTY(Transient)
	bool bTimedObjectiveActive = false;

	UPROPERTY(Transient)
	float TimedObjectiveDurationSeconds = 0.0f;

	UPROPERTY(Transient)
	float TimedObjectiveRemainingSeconds = 0.0f;

	UPROPERTY(Transient)
	bool bTimedObjectiveUsesEscapeDestination = false;

	UPROPERTY(Transient)
	bool bRecoverRelicAwaitingDelivery = false;

	UPROPERTY(Transient)
	FVector TimedObjectiveEscapeDestinationLocation = FVector::ZeroVector;

	UPROPERTY(Transient)
	FVector RecoverRelicDeliveryLocation = FVector::ZeroVector;

	UPROPERTY(Transient)
	bool bHasTimedObjectiveOriginTransform = false;

	UPROPERTY(Transient)
	FTransform TimedObjectiveOriginTransform = FTransform::Identity;

	UPROPERTY(Transient)
	TWeakObjectPtr<AActor> TimedObjectiveInstigator;
};
