// Copyright Epic Games, Inc. All Rights Reserved.

#include "Interaction/BaseInteractable.h"
#include "Game/HorrorEventBusSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"

ABaseInteractable::ABaseInteractable()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = false;
}

bool ABaseInteractable::CanInteract_Implementation(AActor* InstigatorActor, const FHitResult& Hit) const
{
	if (!InstigatorActor)
	{
		return false;
	}

	// Check if already interacted and can't interact multiple times
	if (bHasBeenInteracted && !bCanInteractMultipleTimes)
	{
		return false;
	}

	return true;
}

bool ABaseInteractable::Interact_Implementation(AActor* InstigatorActor, const FHitResult& Hit)
{
	if (!CanInteract_Implementation(InstigatorActor, Hit))
	{
		return false;
	}

	// Update state
	bHasBeenInteracted = true;
	LastInteractionTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;

	// Play sound
	PlayInteractionSound();

	// Publish event
	if (InteractionEventTag.IsValid())
	{
		PublishInteractionEvent(InteractionEventTag, InteractionStateTag);
	}

	// Call subclass implementation
	OnInteract(InstigatorActor, Hit);

	return true;
}

void ABaseInteractable::SaveState(TMap<FName, bool>& OutStateMap) const
{
	if (InteractableId != NAME_None)
	{
		OutStateMap.Add(InteractableId, bHasBeenInteracted);
	}
}

void ABaseInteractable::LoadState(const TMap<FName, bool>& InStateMap)
{
	if (InteractableId != NAME_None)
	{
		if (const bool* State = InStateMap.Find(InteractableId))
		{
			bHasBeenInteracted = *State;
		}
	}
}

void ABaseInteractable::OnInteract(AActor* InstigatorActor, const FHitResult& Hit)
{
	// Override in subclasses
}

void ABaseInteractable::PlayInteractionSound() const
{
	if (InteractionSound && GetWorld())
	{
		UGameplayStatics::PlaySoundAtLocation(
			GetWorld(),
			InteractionSound,
			GetActorLocation()
		);
	}
}

void ABaseInteractable::PublishInteractionEvent(FGameplayTag EventTag, FGameplayTag StateTag)
{
	UHorrorEventBusSubsystem* EventBus = GetEventBus();
	if (EventBus && EventTag.IsValid())
	{
		EventBus->Publish(EventTag, InteractableId, StateTag, this);
	}
}

UHorrorEventBusSubsystem* ABaseInteractable::GetEventBus() const
{
	if (UWorld* World = GetWorld())
	{
		return World->GetSubsystem<UHorrorEventBusSubsystem>();
	}
	return nullptr;
}
