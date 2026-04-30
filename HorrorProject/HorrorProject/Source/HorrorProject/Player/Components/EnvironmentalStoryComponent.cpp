// Copyright HorrorProject. All Rights Reserved.

#include "Player/Components/EnvironmentalStoryComponent.h"
#include "Sound/SoundBase.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

UEnvironmentalStoryComponent::UEnvironmentalStoryComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
}

void UEnvironmentalStoryComponent::TriggerStoryEvent(FName EventId)
{
	for (int32 i = 0; i < StoryEvents.Num(); ++i)
	{
		FEnvironmentalStoryEvent& Event = StoryEvents[i];
		if (Event.EventId != EventId)
		{
			continue;
		}

		if (Event.bHasTriggered && !Event.bRepeatable)
		{
			return;
		}

		// Check if already active
		for (const FActiveEvent& Active : ActiveEvents)
		{
			if (Active.EventIndex == i)
			{
				return;
			}
		}

		Event.bHasTriggered = true;
		ApplyEventEffects(Event);

		FActiveEvent NewActive;
		NewActive.EventIndex = i;
		NewActive.RemainingTime = Event.Duration;
		ActiveEvents.Add(NewActive);

		OnStoryEventTriggered.Broadcast(Event.EventId, Event.Type);
		break;
	}
}

void UEnvironmentalStoryComponent::TriggerByTag(FGameplayTag Tag)
{
	if (!Tag.IsValid())
	{
		return;
	}

	for (int32 i = 0; i < StoryEvents.Num(); ++i)
	{
		FEnvironmentalStoryEvent& Event = StoryEvents[i];
		if (!Event.TriggerTag.MatchesTag(Tag))
		{
			continue;
		}

		if (Event.bHasTriggered && !Event.bRepeatable)
		{
			continue;
		}

		bool bAlreadyActive = false;
		for (const FActiveEvent& Active : ActiveEvents)
		{
			if (Active.EventIndex == i)
			{
				bAlreadyActive = true;
				break;
			}
		}

		if (bAlreadyActive)
		{
			continue;
		}

		Event.bHasTriggered = true;
		ApplyEventEffects(Event);

		FActiveEvent NewActive;
		NewActive.EventIndex = i;
		NewActive.RemainingTime = Event.Duration;
		ActiveEvents.Add(NewActive);

		OnStoryEventTriggered.Broadcast(Event.EventId, Event.Type);
	}
}

void UEnvironmentalStoryComponent::StopAllEvents()
{
	for (const FActiveEvent& Active : ActiveEvents)
	{
		if (Active.EventIndex >= 0 && Active.EventIndex < StoryEvents.Num())
		{
			RemoveEventEffects(StoryEvents[Active.EventIndex]);
		}
	}
	ActiveEvents.Empty();
}

bool UEnvironmentalStoryComponent::IsEventActive(FName EventId) const
{
	for (const FActiveEvent& Active : ActiveEvents)
	{
		if (Active.EventIndex >= 0 && Active.EventIndex < StoryEvents.Num())
		{
			if (StoryEvents[Active.EventIndex].EventId == EventId)
			{
				return true;
			}
		}
	}
	return false;
}

int32 UEnvironmentalStoryComponent::GetActiveEventCount() const
{
	return ActiveEvents.Num();
}

void UEnvironmentalStoryComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	UpdateActiveEvents(DeltaTime);
}

void UEnvironmentalStoryComponent::UpdateActiveEvents(float DeltaTime)
{
	for (int32 i = ActiveEvents.Num() - 1; i >= 0; --i)
	{
		FActiveEvent& Active = ActiveEvents[i];
		Active.RemainingTime -= DeltaTime;

		if (Active.RemainingTime <= 0.0f)
		{
			if (Active.EventIndex >= 0 && Active.EventIndex < StoryEvents.Num())
			{
				RemoveEventEffects(StoryEvents[Active.EventIndex]);
				OnStoryEventCompleted.Broadcast(
					StoryEvents[Active.EventIndex].EventId,
					StoryEvents[Active.EventIndex].Type);
			}
			ActiveEvents.RemoveAt(i);
		}
	}
}

void UEnvironmentalStoryComponent::ApplyEventEffects(const FEnvironmentalStoryEvent& Event)
{
	if (Event.AmbientSound)
	{
		UGameplayStatics::PlaySoundAtLocation(GetOwner(), Event.AmbientSound, GetOwner()->GetActorLocation());
	}

	if (Event.TriggerSound)
	{
		UGameplayStatics::PlaySoundAtLocation(GetOwner(), Event.TriggerSound, GetOwner()->GetActorLocation());
	}
}

void UEnvironmentalStoryComponent::RemoveEventEffects(const FEnvironmentalStoryEvent& Event)
{
	// Effects are time-based and auto-cleanup via sound completion
}
