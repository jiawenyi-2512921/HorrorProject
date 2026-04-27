// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/Components/NoteRecorderComponent.h"

void UNoteRecorderDelegateProbe::HandleNoteRecorded(FName NoteId, int32 TotalRecordedNotes)
{
	++BroadcastCount;
	LastNoteId = NoteId;
	LastTotalRecordedNotes = TotalRecordedNotes;
	NoteIds.Add(NoteId);
	TotalRecordedNoteCounts.Add(TotalRecordedNotes);
}

UNoteRecorderComponent::UNoteRecorderComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

bool UNoteRecorderComponent::AddRecordedNoteId(FName NoteId)
{
	if (NoteId.IsNone() || RecordedNoteIds.Contains(NoteId))
	{
		return false;
	}

	RecordedNoteIds.Add(NoteId);
	OnNoteRecorded.Broadcast(NoteId, RecordedNoteIds.Num());
	return true;
}

bool UNoteRecorderComponent::HasRecordedNoteId(FName NoteId) const
{
	return !NoteId.IsNone() && RecordedNoteIds.Contains(NoteId);
}

const TArray<FName>& UNoteRecorderComponent::GetRecordedNoteIds() const
{
	return RecordedNoteIds;
}

const TArray<FName>& UNoteRecorderComponent::ExportRecordedNoteIds() const
{
	return RecordedNoteIds;
}

void UNoteRecorderComponent::ImportRecordedNoteIds(const TArray<FName>& NoteIds)
{
	RecordedNoteIds.Reset();

	for (const FName NoteId : NoteIds)
	{
		if (NoteId.IsNone() || RecordedNoteIds.Contains(NoteId))
		{
			continue;
		}

		RecordedNoteIds.Add(NoteId);
	}
}

void UNoteRecorderComponent::RegisterNoteMetadata(const FHorrorNoteMetadata& Metadata)
{
	if (Metadata.NoteId.IsNone())
	{
		return;
	}

	NoteMetadataById.Add(Metadata.NoteId, Metadata);
}

bool UNoteRecorderComponent::GetNoteMetadata(FName NoteId, FHorrorNoteMetadata& OutMetadata) const
{
	if (const FHorrorNoteMetadata* Metadata = NoteMetadataById.Find(NoteId))
	{
		OutMetadata = *Metadata;
		return true;
	}

	return false;
}

TArray<FHorrorNoteMetadata> UNoteRecorderComponent::GetRecordedNoteMetadata() const
{
	TArray<FHorrorNoteMetadata> RecordedMetadata;
	for (const FName NoteId : RecordedNoteIds)
	{
		if (const FHorrorNoteMetadata* Metadata = NoteMetadataById.Find(NoteId))
		{
			RecordedMetadata.Add(*Metadata);
		}
	}
	return RecordedMetadata;
}

int32 UNoteRecorderComponent::GetRecordedNoteCount() const
{
	return RecordedNoteIds.Num();
}
