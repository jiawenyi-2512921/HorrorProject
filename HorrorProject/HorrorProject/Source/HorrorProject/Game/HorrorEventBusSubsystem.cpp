// Copyright Epic Games, Inc. All Rights Reserved.

#include "Game/HorrorEventBusSubsystem.h"

#include "Engine/World.h"

namespace
{
	bool IsObjectiveMetadataEmpty(const FHorrorObjectiveMessageMetadata& Metadata)
	{
		return Metadata.TrailerBeatId.IsNone()
			&& Metadata.ObjectiveHint.IsEmpty()
			&& Metadata.DebugLabel.IsEmpty();
	}

	// Performance optimization: Use hash-based key instead of string concatenation
	FName MakeEventSourceMetadataKey(FGameplayTag EventTag, FName SourceId)
	{
		const uint32 Hash = HashCombine(GetTypeHash(EventTag), GetTypeHash(SourceId));
		return FName(*FString::Printf(TEXT("EventKey_%u"), Hash));
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
	const FName EventSourceMetadataKey = MakeEventSourceMetadataKey(EventTag, SourceId);
	if (const FHorrorObjectiveMessageMetadata* EventMetadata = ObjectiveMetadataByEventAndSourceId.Find(EventSourceMetadataKey))
	{
		Message.TrailerBeatId = EventMetadata->TrailerBeatId;
		Message.ObjectiveHint = EventMetadata->ObjectiveHint;
		Message.DebugLabel = EventMetadata->DebugLabel;
	}
	else if (const FHorrorObjectiveMessageMetadata* SourceMetadata = ObjectiveMetadataBySourceId.Find(SourceId))
	{
		Message.TrailerBeatId = SourceMetadata->TrailerBeatId;
		Message.ObjectiveHint = SourceMetadata->ObjectiveHint;
		Message.DebugLabel = SourceMetadata->DebugLabel;
	}

	// Performance optimization: Reserve capacity and use circular buffer approach
	if (History.Num() == 0)
	{
		History.Reserve(HistoryCapacity);
	}

	FHorrorEventMessage HistoryMessage = Message;
	HistoryMessage.SourceObject = nullptr;

	if (History.Num() >= HistoryCapacity)
	{
		History.RemoveAt(0, 1, EAllowShrinking::No);
	}
	History.Add(HistoryMessage);
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
	return History;
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

	const FName EventSourceMetadataKey = MakeEventSourceMetadataKey(EventTag, SourceId);
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

	const FName EventSourceMetadataKey = MakeEventSourceMetadataKey(EventTag, SourceId);
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

	const FName EventSourceMetadataKey = MakeEventSourceMetadataKey(EventTag, SourceId);
	if (const FHorrorObjectiveMessageMetadata* Metadata = ObjectiveMetadataByEventAndSourceId.Find(EventSourceMetadataKey))
	{
		OutMetadata = *Metadata;
		return true;
	}

	return false;
}

void UHorrorEventBusSubsystem::ResetForTests()
{
	History.Reset();
	ObjectiveMetadataBySourceId.Reset();
	ObjectiveMetadataByEventAndSourceId.Reset();
	OnEventPublishedNative.Clear();
	OnEventPublished.Clear();
}

#if WITH_DEV_AUTOMATION_TESTS
void UHorrorEventBusSubsystem::SetHistoryCapacityForTests(int32 NewHistoryCapacity)
{
	HistoryCapacity = FMath::Max(1, NewHistoryCapacity);
	if (History.Num() > HistoryCapacity)
	{
		History.RemoveAt(0, History.Num() - HistoryCapacity, EAllowShrinking::No);
	}
}
#endif
