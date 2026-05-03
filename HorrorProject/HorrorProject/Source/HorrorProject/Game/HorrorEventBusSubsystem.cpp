// Copyright Epic Games, Inc. All Rights Reserved.

#include "Game/HorrorEventBusSubsystem.h"

#include "Engine/World.h"

namespace
{
	bool IsObjectiveMetadataEmpty(const FHorrorObjectiveMessageMetadata& Metadata)
	{
		return Metadata.TrailerBeatId.IsNone()
			&& Metadata.ObjectiveHint.IsEmpty()
			&& Metadata.DebugLabel.IsEmpty()
			&& Metadata.FeedbackSeverity == EHorrorObjectiveFeedbackSeverity::Info
			&& !Metadata.bRetryable
			&& FMath::IsNearlyEqual(Metadata.DisplaySeconds, 5.0f)
			&& Metadata.FailureCause.IsNone()
			&& Metadata.RecoveryAction.IsNone()
			&& Metadata.AdvancedOutcomeKind == EHorrorAdvancedInteractionOutcomeKind::Ignored
			&& Metadata.AdvancedFaultId.IsNone()
			&& Metadata.AttemptIndex <= 0;
	}
}

bool UHorrorEventBusSubsystem::Publish(FGameplayTag EventTag, FName SourceId, FGameplayTag StateTag, UObject* SourceObject)
{
	const UWorld* World = GetWorld();
	if (!World || !EventTag.IsValid())
	{
		return false;
	}

	FHorrorEventMessage Message;
	Message.EventTag = EventTag;
	Message.StateTag = StateTag;
	Message.SourceId = SourceId;
	Message.SourceObject = SourceObject;
	Message.WorldSeconds = World->GetTimeSeconds();
	const FHorrorEventSourceMetadataKey EventSourceMetadataKey(EventTag, SourceId);
	if (const FHorrorObjectiveMessageMetadata* EventMetadata = ObjectiveMetadataByEventAndSourceId.Find(EventSourceMetadataKey))
	{
		Message.TrailerBeatId = EventMetadata->TrailerBeatId;
		Message.ObjectiveHint = EventMetadata->ObjectiveHint;
		Message.DebugLabel = EventMetadata->DebugLabel;
		Message.FeedbackSeverity = EventMetadata->FeedbackSeverity;
		Message.bRetryable = EventMetadata->bRetryable;
		Message.DisplaySeconds = FMath::Max(0.1f, EventMetadata->DisplaySeconds);
		Message.FailureCause = EventMetadata->FailureCause;
		Message.RecoveryAction = EventMetadata->RecoveryAction;
		Message.AdvancedOutcomeKind = EventMetadata->AdvancedOutcomeKind;
		Message.AdvancedFaultId = EventMetadata->AdvancedFaultId;
		Message.AttemptIndex = FMath::Max(0, EventMetadata->AttemptIndex);
	}
	else if (const FHorrorObjectiveMessageMetadata* SourceMetadata = ObjectiveMetadataBySourceId.Find(SourceId))
	{
		Message.TrailerBeatId = SourceMetadata->TrailerBeatId;
		Message.ObjectiveHint = SourceMetadata->ObjectiveHint;
		Message.DebugLabel = SourceMetadata->DebugLabel;
		Message.FeedbackSeverity = SourceMetadata->FeedbackSeverity;
		Message.bRetryable = SourceMetadata->bRetryable;
		Message.DisplaySeconds = FMath::Max(0.1f, SourceMetadata->DisplaySeconds);
		Message.FailureCause = SourceMetadata->FailureCause;
		Message.RecoveryAction = SourceMetadata->RecoveryAction;
		Message.AdvancedOutcomeKind = SourceMetadata->AdvancedOutcomeKind;
		Message.AdvancedFaultId = SourceMetadata->AdvancedFaultId;
		Message.AttemptIndex = FMath::Max(0, SourceMetadata->AttemptIndex);
	}

	FHorrorEventMessage HistoryMessage = Message;
	HistoryMessage.SourceObject = nullptr; // Prevent GC issues from holding UObject refs in history
	AppendHistoryMessage(MoveTemp(HistoryMessage));
	OnEventPublished.Broadcast(Message);
	OnEventPublishedNative.Broadcast(Message);
	return true;
}

FHorrorEventPublishedNativeDelegate& UHorrorEventBusSubsystem::GetOnEventPublishedNative()
{
	return OnEventPublishedNative;
}

const TArray<FHorrorEventMessage>& UHorrorEventBusSubsystem::GetHistory() const
{
	RebuildOrderedHistoryView();
	return OrderedHistoryView;
}

void UHorrorEventBusSubsystem::RegisterObjectiveMetadata(FName SourceId, const FHorrorObjectiveMessageMetadata& Metadata)
{
	if (SourceId.IsNone())
	{
		return;
	}

	if (IsObjectiveMetadataEmpty(Metadata))
	{
		UnregisterObjectiveMetadata(SourceId);
		return;
	}

	ObjectiveMetadataBySourceId.Add(SourceId, Metadata);
}

void UHorrorEventBusSubsystem::RegisterObjectiveMetadata(FGameplayTag EventTag, FName SourceId, const FHorrorObjectiveMessageMetadata& Metadata)
{
	if (!EventTag.IsValid() || SourceId.IsNone())
	{
		return;
	}

	if (IsObjectiveMetadataEmpty(Metadata))
	{
		UnregisterObjectiveMetadata(EventTag, SourceId);
		return;
	}

	const FHorrorEventSourceMetadataKey EventSourceMetadataKey(EventTag, SourceId);
	ObjectiveMetadataByEventAndSourceId.Add(EventSourceMetadataKey, Metadata);
}

void UHorrorEventBusSubsystem::UnregisterObjectiveMetadata(FName SourceId)
{
	if (SourceId.IsNone())
	{
		return;
	}

	ObjectiveMetadataBySourceId.Remove(SourceId);
}

void UHorrorEventBusSubsystem::UnregisterObjectiveMetadata(FGameplayTag EventTag, FName SourceId)
{
	if (!EventTag.IsValid() || SourceId.IsNone())
	{
		return;
	}

	const FHorrorEventSourceMetadataKey EventSourceMetadataKey(EventTag, SourceId);
	ObjectiveMetadataByEventAndSourceId.Remove(EventSourceMetadataKey);
}

bool UHorrorEventBusSubsystem::GetObjectiveMetadataForTests(FName SourceId, FHorrorObjectiveMessageMetadata& OutMetadata) const
{
	if (const FHorrorObjectiveMessageMetadata* Metadata = ObjectiveMetadataBySourceId.Find(SourceId))
	{
		OutMetadata = *Metadata;
		return true;
	}

	return false;
}

bool UHorrorEventBusSubsystem::GetObjectiveMetadataForTests(FGameplayTag EventTag, FName SourceId, FHorrorObjectiveMessageMetadata& OutMetadata) const
{
	if (!EventTag.IsValid() || SourceId.IsNone())
	{
		return false;
	}

	const FHorrorEventSourceMetadataKey EventSourceMetadataKey(EventTag, SourceId);
	if (const FHorrorObjectiveMessageMetadata* Metadata = ObjectiveMetadataByEventAndSourceId.Find(EventSourceMetadataKey))
	{
		OutMetadata = *Metadata;
		return true;
	}

	return false;
}

void UHorrorEventBusSubsystem::ResetForTests()
{
	HistoryRing.Reset();
	OrderedHistoryView.Reset();
	bOrderedHistoryViewDirty = true;
	HistoryStartIndex = 0;
	HistoryCount = 0;
	ObjectiveMetadataBySourceId.Reset();
	ObjectiveMetadataByEventAndSourceId.Reset();
	OnEventPublishedNative.Clear();
	OnEventPublished.Clear();
}

#if WITH_DEV_AUTOMATION_TESTS
void UHorrorEventBusSubsystem::SetHistoryCapacityForTests(int32 NewHistoryCapacity)
{
	const int32 ClampedCapacity = FMath::Max(1, NewHistoryCapacity);
	if (ClampedCapacity == HistoryCapacity)
	{
		return;
	}

	RebuildOrderedHistoryView();
	HistoryCapacity = ClampedCapacity;
	if (OrderedHistoryView.Num() > HistoryCapacity)
	{
		OrderedHistoryView.RemoveAt(0, OrderedHistoryView.Num() - HistoryCapacity, EAllowShrinking::No);
	}

	HistoryRing = OrderedHistoryView;
	HistoryRing.Reserve(HistoryCapacity);
	HistoryStartIndex = 0;
	HistoryCount = HistoryRing.Num();
	bOrderedHistoryViewDirty = true;
}
#endif

void UHorrorEventBusSubsystem::AppendHistoryMessage(FHorrorEventMessage&& Message)
{
	EnsureHistoryRingCapacity();
	if (HistoryRing.Num() < HistoryCapacity)
	{
		HistoryRing.Add(MoveTemp(Message));
		HistoryCount = HistoryRing.Num();
		bOrderedHistoryViewDirty = true;
		return;
	}

	const int32 WriteIndex = HistoryStartIndex;
	HistoryRing[WriteIndex] = MoveTemp(Message);
	HistoryStartIndex = (HistoryStartIndex + 1) % HistoryCapacity;
	HistoryCount = HistoryCapacity;
	bOrderedHistoryViewDirty = true;
}

void UHorrorEventBusSubsystem::EnsureHistoryRingCapacity()
{
	HistoryCapacity = FMath::Max(1, HistoryCapacity);
	if (HistoryRing.Max() < HistoryCapacity)
	{
		HistoryRing.Reserve(HistoryCapacity);
	}
}

void UHorrorEventBusSubsystem::RebuildOrderedHistoryView() const
{
	if (!bOrderedHistoryViewDirty)
	{
		return;
	}

	OrderedHistoryView.Reset(HistoryCount);
	if (HistoryCount <= 0 || HistoryRing.Num() <= 0)
	{
		bOrderedHistoryViewDirty = false;
		return;
	}

	for (int32 Offset = 0; Offset < HistoryCount; ++Offset)
	{
		const int32 SourceIndex = (HistoryStartIndex + Offset) % HistoryRing.Num();
		OrderedHistoryView.Add(HistoryRing[SourceIndex]);
	}
	bOrderedHistoryViewDirty = false;
}
