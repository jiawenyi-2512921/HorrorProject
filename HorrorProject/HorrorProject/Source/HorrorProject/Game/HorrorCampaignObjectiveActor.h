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
	int32 GetRequiredInteractionCount() const { return RequiredInteractionCount; }

	UFUNCTION(BlueprintPure, Category="Horror|Campaign")
	int32 GetInteractionProgressCount() const { return InteractionProgressCount; }

	UFUNCTION(BlueprintPure, Category="Horror|Campaign")
	EHorrorCampaignInteractionMode GetInteractionMode() const { return ObjectiveDefinition.InteractionMode; }

	UFUNCTION(BlueprintPure, Category="Horror|Campaign|Advanced Interaction")
	bool IsAdvancedInteractionActive() const { return bAdvancedInteractionActive; }

	UFUNCTION(BlueprintPure, Category="Horror|Campaign|Advanced Interaction")
	float GetAdvancedInteractionProgressFraction() const { return AdvancedInteractionProgressFraction; }

	UFUNCTION(BlueprintPure, Category="Horror|Campaign|Advanced Interaction")
	FName GetExpectedAdvancedInputId() const { return ExpectedAdvancedInputId; }

	UFUNCTION(BlueprintPure, Category="Horror|Campaign|Advanced Interaction")
	float GetAdvancedInteractionTimingFraction() const { return AdvancedInteractionTimingFraction; }

	UFUNCTION(BlueprintPure, Category="Horror|Campaign|Advanced Interaction")
	bool IsAdvancedInteractionTimingWindowOpen() const;

	UFUNCTION(BlueprintPure, Category="Horror|Campaign|Advanced Interaction")
	FText GetAdvancedInteractionFeedbackText() const { return AdvancedInteractionFeedbackText; }

	UFUNCTION(BlueprintPure, Category="Horror|Campaign|Advanced Interaction")
	float GetAdvancedInteractionPauseRemainingSeconds() const { return AdvancedInteractionPauseRemainingSeconds; }

	UFUNCTION(BlueprintPure, Category="Horror|Campaign|Advanced Interaction")
	FHorrorAdvancedInteractionHUDState BuildAdvancedInteractionHUDState() const;

	UFUNCTION(BlueprintCallable, Category="Horror|Campaign|Advanced Interaction")
	bool SubmitAdvancedInteractionInput(FName InputId, AActor* InstigatorActor);

	UFUNCTION(BlueprintPure, Category="Horror|Campaign")
	bool IsTimedObjectiveActive() const { return bTimedObjectiveActive; }

	UFUNCTION(BlueprintPure, Category="Horror|Campaign")
	float GetTimedObjectiveDurationSeconds() const { return TimedObjectiveDurationSeconds; }

	UFUNCTION(BlueprintPure, Category="Horror|Campaign")
	float GetTimedObjectiveRemainingSeconds() const { return TimedObjectiveRemainingSeconds; }

	UFUNCTION(BlueprintPure, Category="Horror|Campaign")
	bool HasActiveEscapeDestination() const { return bTimedObjectiveActive && bTimedObjectiveUsesEscapeDestination; }

	UFUNCTION(BlueprintPure, Category="Horror|Campaign")
	FVector GetActiveEscapeDestinationWorldLocation() const { return TimedObjectiveEscapeDestinationLocation; }

	UFUNCTION(BlueprintPure, Category="Horror|Campaign")
	float GetEscapeCompletionRadius() const { return FMath::Max(50.0f, ObjectiveDefinition.EscapeCompletionRadius); }

	UFUNCTION(BlueprintCallable, Category="Horror|Campaign")
	void RefreshObjectiveState();

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
	bool HasConfiguredEscapeDestination() const;
	FVector ResolveEscapeDestinationWorldLocation() const;
	bool HasTimedObjectiveReachedEscapeDestination() const;
	void StartAdvancedInteraction();
	void ResetAdvancedInteractionState();
	void ResetAdvancedInteractionCue();
	void AdvanceExpectedAdvancedInput();
	void ApplyAdvancedInteractionTimingFailure();
	void ApplyAdvancedInteractionWrongInputFailure();
	bool CompleteAdvancedInteraction(AActor* InstigatorActor);
	FText BuildAdvancedInteractionPromptText() const;
	void StartTimedObjective(AActor* InstigatorActor);
	void UpdateTimedObjective(float DeltaTime);
	void UpdateAdvancedInteraction(float DeltaTime);
	bool CompleteObjective(AActor* InstigatorActor);
	void PublishAmbushStartedEvent(AActor* InstigatorActor) const;
	void ApplyObjectiveVisuals();
	void ApplyCompletedVisuals();
	FColor GetObjectiveColor() const;

	UPROPERTY(Transient)
	bool bCompleted = false;

	UPROPERTY(Transient)
	bool bAvailableForInteraction = true;

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
	float AdvancedInteractionPauseRemainingSeconds = 0.0f;

	UPROPERTY(Transient)
	bool bTimedObjectiveActive = false;

	UPROPERTY(Transient)
	float TimedObjectiveDurationSeconds = 0.0f;

	UPROPERTY(Transient)
	float TimedObjectiveRemainingSeconds = 0.0f;

	UPROPERTY(Transient)
	bool bTimedObjectiveUsesEscapeDestination = false;

	UPROPERTY(Transient)
	FVector TimedObjectiveEscapeDestinationLocation = FVector::ZeroVector;

	UPROPERTY(Transient)
	TWeakObjectPtr<AActor> TimedObjectiveInstigator;
};
