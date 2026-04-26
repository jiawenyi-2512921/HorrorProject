// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

class FHorrorFoundFootageContract;
struct FHorrorFoundFootageRecordedEvent;
struct FHorrorFoundFootageStateChange;

class HORRORPROJECT_API FHorrorAnomalyDirector
{
public:
	bool BeginFirstAnomalyCandidate(FName SourceId, const FHorrorFoundFootageContract& Contract);
	bool TryRecordFirstAnomaly(bool bIsRecording, FHorrorFoundFootageContract& Contract, TArray<FHorrorFoundFootageRecordedEvent>* OutRecordedEvents = nullptr, TArray<FHorrorFoundFootageStateChange>* OutStateChanges = nullptr);
	bool HasPendingFirstAnomalyCandidate() const;
	FName GetPendingFirstAnomalySourceId() const;
	void ImportPendingFirstAnomalyCandidate(FName SourceId, const FHorrorFoundFootageContract& Contract);

private:
	FName PendingFirstAnomalySourceId;
};
