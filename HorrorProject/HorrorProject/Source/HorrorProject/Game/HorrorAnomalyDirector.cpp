// Copyright Epic Games, Inc. All Rights Reserved.

#include "Game/HorrorAnomalyDirector.h"

#include "Game/HorrorFoundFootageContract.h"

bool FHorrorAnomalyDirector::BeginFirstAnomalyCandidate(FName SourceId, const FHorrorFoundFootageContract& Contract)
{
	if (SourceId.IsNone()
		|| !PendingFirstAnomalySourceId.IsNone()
		|| !Contract.HasCompletedState(HorrorFoundFootageTags::FirstNoteCollectedState())
		|| Contract.HasRecordedEvent(HorrorFoundFootageTags::FirstAnomalyRecordedEvent()))
	{
		return false;
	}

	PendingFirstAnomalySourceId = SourceId;
	return true;
}

bool FHorrorAnomalyDirector::TryRecordFirstAnomaly(bool bIsRecording, FHorrorFoundFootageContract& Contract, TArray<FHorrorFoundFootageRecordedEvent>* OutRecordedEvents, TArray<FHorrorFoundFootageStateChange>* OutStateChanges)
{
	if (!bIsRecording || PendingFirstAnomalySourceId.IsNone())
	{
		return false;
	}

	const FName SourceId = PendingFirstAnomalySourceId;
	const bool bRecorded = Contract.RecordEvent(HorrorFoundFootageTags::FirstAnomalyRecordedEvent(), SourceId, OutRecordedEvents, OutStateChanges);
	if (bRecorded)
	{
		PendingFirstAnomalySourceId = NAME_None;
	}

	return bRecorded;
}

bool FHorrorAnomalyDirector::HasPendingFirstAnomalyCandidate() const
{
	return !PendingFirstAnomalySourceId.IsNone();
}

FName FHorrorAnomalyDirector::GetPendingFirstAnomalySourceId() const
{
	return PendingFirstAnomalySourceId;
}

void FHorrorAnomalyDirector::ImportPendingFirstAnomalyCandidate(FName SourceId, const FHorrorFoundFootageContract& Contract)
{
	PendingFirstAnomalySourceId = NAME_None;
	if (!SourceId.IsNone()
		&& Contract.HasCompletedState(HorrorFoundFootageTags::FirstNoteCollectedState())
		&& !Contract.HasRecordedEvent(HorrorFoundFootageTags::FirstAnomalyRecordedEvent()))
	{
		PendingFirstAnomalySourceId = SourceId;
	}
}
