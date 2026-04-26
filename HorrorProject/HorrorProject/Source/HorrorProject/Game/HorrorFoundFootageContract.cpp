// Copyright Epic Games, Inc. All Rights Reserved.

#include "Game/HorrorFoundFootageContract.h"

namespace HorrorFoundFootageTags
{
	UE_DEFINE_GAMEPLAY_TAG(BodycamAcquiredEventTag, "Event.Bodycam.Acquired");
	UE_DEFINE_GAMEPLAY_TAG(FirstNoteCollectedEventTag, "Event.Note.FirstCollected");
	UE_DEFINE_GAMEPLAY_TAG(FirstAnomalyRecordedEventTag, "Event.Anomaly.FirstRecorded");
	UE_DEFINE_GAMEPLAY_TAG(ArchiveReviewedEventTag, "Event.Archive.Reviewed");
	UE_DEFINE_GAMEPLAY_TAG(ExitUnlockedEventTag, "Event.Exit.Unlocked");
	UE_DEFINE_GAMEPLAY_TAG(BodycamAcquiredStateTag, "State.Objective.BodycamAcquired");
	UE_DEFINE_GAMEPLAY_TAG(FirstNoteCollectedStateTag, "State.Objective.FirstNoteCollected");
	UE_DEFINE_GAMEPLAY_TAG(FirstAnomalyRecordedStateTag, "State.Objective.FirstAnomalyRecorded");
	UE_DEFINE_GAMEPLAY_TAG(ArchiveReviewedStateTag, "State.Objective.ArchiveReviewed");
	UE_DEFINE_GAMEPLAY_TAG(ExitUnlockedStateTag, "State.Objective.ExitUnlocked");

	FGameplayTag BodycamAcquiredEvent() { return BodycamAcquiredEventTag; }
	FGameplayTag FirstNoteCollectedEvent() { return FirstNoteCollectedEventTag; }
	FGameplayTag FirstAnomalyRecordedEvent() { return FirstAnomalyRecordedEventTag; }
	FGameplayTag ArchiveReviewedEvent() { return ArchiveReviewedEventTag; }
	FGameplayTag ExitUnlockedEvent() { return ExitUnlockedEventTag; }

	FGameplayTag BodycamAcquiredState() { return BodycamAcquiredStateTag; }
	FGameplayTag FirstNoteCollectedState() { return FirstNoteCollectedStateTag; }
	FGameplayTag FirstAnomalyRecordedState() { return FirstAnomalyRecordedStateTag; }
	FGameplayTag ArchiveReviewedState() { return ArchiveReviewedStateTag; }
	FGameplayTag ExitUnlockedState() { return ExitUnlockedStateTag; }
}

bool FHorrorFoundFootageContract::RecordEvent(FGameplayTag EventTag, FName SourceId, TArray<FHorrorFoundFootageRecordedEvent>* OutRecordedEvents, TArray<FHorrorFoundFootageStateChange>* OutStateChanges)
{
	if (!EventTag.IsValid() || EventTag == HorrorFoundFootageTags::ExitUnlockedEvent() || RecordedEvents.Contains(EventTag))
	{
		return false;
	}

	if (EventTag == HorrorFoundFootageTags::FirstNoteCollectedEvent()
		&& !HasCompletedState(HorrorFoundFootageTags::BodycamAcquiredState()))
	{
		return false;
	}

	if (EventTag == HorrorFoundFootageTags::FirstAnomalyRecordedEvent()
		&& !HasCompletedState(HorrorFoundFootageTags::FirstNoteCollectedState()))
	{
		return false;
	}

	if (EventTag == HorrorFoundFootageTags::ArchiveReviewedEvent()
		&& !HasCompletedState(HorrorFoundFootageTags::FirstAnomalyRecordedState()))
	{
		return false;
	}

	const FGameplayTag StateTag = StateForEvent(EventTag);
	if (!StateTag.IsValid())
	{
		return false;
	}

	RecordedEvents.Add(EventTag);
	if (OutRecordedEvents)
	{
		OutRecordedEvents->Add({ EventTag, SourceId });
	}

	MarkStateCompleted(StateTag, OutStateChanges);
	TryUnlockExit(OutRecordedEvents, OutStateChanges);
	return true;
}

bool FHorrorFoundFootageContract::HasRecordedEvent(FGameplayTag EventTag) const
{
	return EventTag.IsValid() && RecordedEvents.Contains(EventTag);
}

bool FHorrorFoundFootageContract::HasCompletedState(FGameplayTag StateTag) const
{
	return StateTag.IsValid() && CompletedStates.Contains(StateTag);
}

bool FHorrorFoundFootageContract::IsExitUnlocked() const
{
	return HasCompletedState(HorrorFoundFootageTags::ExitUnlockedState());
}

FGameplayTag FHorrorFoundFootageContract::GetStateForEvent(FGameplayTag EventTag) const
{
	return StateForEvent(EventTag);
}

FHorrorFoundFootageProgressSnapshot FHorrorFoundFootageContract::BuildSnapshot() const
{
	FHorrorFoundFootageProgressSnapshot Snapshot;
	const TPair<FGameplayTag, FGameplayTag> MilestonePairs[] = {
		{ HorrorFoundFootageTags::BodycamAcquiredEvent(), HorrorFoundFootageTags::BodycamAcquiredState() },
		{ HorrorFoundFootageTags::FirstNoteCollectedEvent(), HorrorFoundFootageTags::FirstNoteCollectedState() },
		{ HorrorFoundFootageTags::FirstAnomalyRecordedEvent(), HorrorFoundFootageTags::FirstAnomalyRecordedState() },
		{ HorrorFoundFootageTags::ArchiveReviewedEvent(), HorrorFoundFootageTags::ArchiveReviewedState() },
		{ HorrorFoundFootageTags::ExitUnlockedEvent(), HorrorFoundFootageTags::ExitUnlockedState() }
	};

	for (const TPair<FGameplayTag, FGameplayTag>& Pair : MilestonePairs)
	{
		const bool bCompleted = HasCompletedState(Pair.Value);
		Snapshot.Milestones.Add({ Pair.Key, Pair.Value, bCompleted });
		if (bCompleted)
		{
			++Snapshot.CompletedMilestoneCount;
		}
	}

	Snapshot.bExitUnlocked = IsExitUnlocked();
	return Snapshot;
}

FHorrorFoundFootageSaveState FHorrorFoundFootageContract::ExportSaveState() const
{
	FHorrorFoundFootageSaveState SaveState;

	const FGameplayTag ObjectiveEvents[] = {
		HorrorFoundFootageTags::BodycamAcquiredEvent(),
		HorrorFoundFootageTags::FirstNoteCollectedEvent(),
		HorrorFoundFootageTags::FirstAnomalyRecordedEvent(),
		HorrorFoundFootageTags::ArchiveReviewedEvent(),
		HorrorFoundFootageTags::ExitUnlockedEvent()
	};

	for (const FGameplayTag& ObjectiveEvent : ObjectiveEvents)
	{
		if (HasRecordedEvent(ObjectiveEvent))
		{
			SaveState.RecordedObjectiveEvents.Add(ObjectiveEvent);
		}
	}

	const FGameplayTag ObjectiveStates[] = {
		HorrorFoundFootageTags::BodycamAcquiredState(),
		HorrorFoundFootageTags::FirstNoteCollectedState(),
		HorrorFoundFootageTags::FirstAnomalyRecordedState(),
		HorrorFoundFootageTags::ArchiveReviewedState(),
		HorrorFoundFootageTags::ExitUnlockedState()
	};

	for (const FGameplayTag& ObjectiveState : ObjectiveStates)
	{
		if (HasCompletedState(ObjectiveState))
		{
			SaveState.CompletedObjectiveStates.Add(ObjectiveState);
		}
	}

	return SaveState;
}

void FHorrorFoundFootageContract::ImportSaveState(const FHorrorFoundFootageSaveState& SaveState)
{
	RecordedEvents.Reset();
	CompletedStates.Reset();

	TSet<FGameplayTag> ImportedEvents;
	for (const FGameplayTag& EventTag : SaveState.RecordedObjectiveEvents)
	{
		if (EventTag.IsValid())
		{
			ImportedEvents.Add(EventTag);
		}
	}

	TSet<FGameplayTag> ImportedStates;
	for (const FGameplayTag& StateTag : SaveState.CompletedObjectiveStates)
	{
		if (StateTag.IsValid())
		{
			ImportedStates.Add(StateTag);
		}
	}

	const TPair<FGameplayTag, FGameplayTag> PrerequisiteMilestones[] = {
		{ HorrorFoundFootageTags::BodycamAcquiredEvent(), HorrorFoundFootageTags::BodycamAcquiredState() },
		{ HorrorFoundFootageTags::FirstNoteCollectedEvent(), HorrorFoundFootageTags::FirstNoteCollectedState() },
		{ HorrorFoundFootageTags::FirstAnomalyRecordedEvent(), HorrorFoundFootageTags::FirstAnomalyRecordedState() },
		{ HorrorFoundFootageTags::ArchiveReviewedEvent(), HorrorFoundFootageTags::ArchiveReviewedState() }
	};

	bool bImportedAllExitPrerequisites = true;
	for (const TPair<FGameplayTag, FGameplayTag>& Milestone : PrerequisiteMilestones)
	{
		if (!ImportedEvents.Contains(Milestone.Key) || !ImportedStates.Contains(Milestone.Value))
		{
			bImportedAllExitPrerequisites = false;
			break;
		}

		RecordedEvents.Add(Milestone.Key);
		CompletedStates.Add(Milestone.Value);
	}

	if (bImportedAllExitPrerequisites)
	{
		RecordedEvents.Add(HorrorFoundFootageTags::ExitUnlockedEvent());
		CompletedStates.Add(HorrorFoundFootageTags::ExitUnlockedState());
	}
}

void FHorrorFoundFootageContract::MarkStateCompleted(FGameplayTag StateTag, TArray<FHorrorFoundFootageStateChange>* OutStateChanges)
{
	if (!StateTag.IsValid() || CompletedStates.Contains(StateTag))
	{
		return;
	}

	CompletedStates.Add(StateTag);
	if (OutStateChanges)
	{
		OutStateChanges->Add({ StateTag, true });
	}
}

void FHorrorFoundFootageContract::TryUnlockExit(TArray<FHorrorFoundFootageRecordedEvent>* OutRecordedEvents, TArray<FHorrorFoundFootageStateChange>* OutStateChanges)
{
	if (IsExitUnlocked())
	{
		return;
	}

	if (!HasCompletedState(HorrorFoundFootageTags::BodycamAcquiredState())
		|| !HasCompletedState(HorrorFoundFootageTags::FirstNoteCollectedState())
		|| !HasCompletedState(HorrorFoundFootageTags::FirstAnomalyRecordedState())
		|| !HasCompletedState(HorrorFoundFootageTags::ArchiveReviewedState()))
	{
		return;
	}

	const FGameplayTag ExitEvent = HorrorFoundFootageTags::ExitUnlockedEvent();
	RecordedEvents.Add(ExitEvent);
	if (OutRecordedEvents)
	{
		OutRecordedEvents->Add({ ExitEvent, TEXT("FoundFootageContract") });
	}
	MarkStateCompleted(HorrorFoundFootageTags::ExitUnlockedState(), OutStateChanges);
}

FGameplayTag FHorrorFoundFootageContract::StateForEvent(FGameplayTag EventTag) const
{
	if (EventTag == HorrorFoundFootageTags::BodycamAcquiredEvent())
	{
		return HorrorFoundFootageTags::BodycamAcquiredState();
	}
	if (EventTag == HorrorFoundFootageTags::FirstNoteCollectedEvent())
	{
		return HorrorFoundFootageTags::FirstNoteCollectedState();
	}
	if (EventTag == HorrorFoundFootageTags::FirstAnomalyRecordedEvent())
	{
		return HorrorFoundFootageTags::FirstAnomalyRecordedState();
	}
	if (EventTag == HorrorFoundFootageTags::ArchiveReviewedEvent())
	{
		return HorrorFoundFootageTags::ArchiveReviewedState();
	}
	if (EventTag == HorrorFoundFootageTags::ExitUnlockedEvent())
	{
		return HorrorFoundFootageTags::ExitUnlockedState();
	}
	return FGameplayTag();
}
