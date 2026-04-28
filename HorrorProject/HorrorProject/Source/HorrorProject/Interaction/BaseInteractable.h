// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "InteractableInterface.h"
#include "GameplayTagContainer.h"
#include "BaseInteractable.generated.h"

class USoundBase;
class UHorrorEventBusSubsystem;

/**
 * Base class for all interactable objects in the game.
 * Provides common functionality like sound playback, event publishing, and state management.
 */
UCLASS(Abstract, BlueprintType, Blueprintable)
class HORRORPROJECT_API ABaseInteractable : public AActor, public IInteractableInterface
{
	GENERATED_BODY()

public:
	ABaseInteractable();

	// IInteractableInterface implementation
	virtual bool CanInteract_Implementation(AActor* InstigatorActor, const FHitResult& Hit) const override;
	virtual bool Interact_Implementation(AActor* InstigatorActor, const FHitResult& Hit) override;

	// State management
	UFUNCTION(BlueprintPure, Category="Interaction")
	bool IsInteracted() const { return bHasBeenInteracted; }

	UFUNCTION(BlueprintCallable, Category="Interaction")
	void SetInteracted(bool bInteracted) { bHasBeenInteracted = bInteracted; }

	// Save/Load support
	UFUNCTION(BlueprintCallable, Category="Interaction|Save")
	virtual void SaveState(TMap<FName, bool>& OutStateMap) const;

	UFUNCTION(BlueprintCallable, Category="Interaction|Save")
	virtual void LoadState(const TMap<FName, bool>& InStateMap);

	UFUNCTION(BlueprintPure, Category="Interaction")
	FName GetInteractableId() const { return InteractableId; }

	UFUNCTION(BlueprintPure, Category="Interaction")
	FText GetInteractionPromptText() const { return InteractionPrompt; }

protected:
	// Called when interaction occurs (override in subclasses)
	virtual void OnInteract(AActor* InstigatorActor, const FHitResult& Hit);

	// Helper functions
	void PlayInteractionSound() const;
	void PublishInteractionEvent(FGameplayTag EventTag, FGameplayTag StateTag = FGameplayTag::EmptyTag);
	UHorrorEventBusSubsystem* GetEventBus() const;

	// Configuration
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interaction")
	FName InteractableId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interaction")
	FText InteractionPrompt = FText::FromString(TEXT("互动"));

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interaction")
	bool bCanInteractMultipleTimes = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interaction")
	bool bRequireLineOfSight = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interaction|Audio")
	TObjectPtr<USoundBase> InteractionSound = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interaction|Events")
	FGameplayTag InteractionEventTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interaction|Events")
	FGameplayTag InteractionStateTag;

	// State
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Interaction|State")
	bool bHasBeenInteracted = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Interaction|State")
	float LastInteractionTime = 0.0f;
};
