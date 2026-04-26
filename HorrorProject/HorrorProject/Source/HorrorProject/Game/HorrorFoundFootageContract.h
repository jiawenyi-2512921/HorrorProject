// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "NativeGameplayTags.h"
#include "HorrorFoundFootageContract.generated.h"

struct HORRORPROJECT_API FHorrorFoundFootageRecordedEvent
{
	FGameplayTag EventTag;
	FName SourceId;
};

struct HORRORPROJECT_API FHorrorFoundFootageStateChange
{
	FGameplayTag StateTag;
	bool bCompleted = false;
};

USTRUCT(BlueprintType)
struct HORRORPROJECT_API FHorrorFoundFootageMilestone
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category="Horror|Objectives")
	FGameplayTag EventTag;

	UPROPERTY(BlueprintReadOnly, Category="Horror|Objectives")
	FGameplayTag StateTag;

	UPROPERTY(BlueprintReadOnly, Category="Horror|Objectives")
	bool bCompleted = false;
};

USTRUCT(BlueprintType)
struct HORRORPROJECT_API FHorrorFoundFootageProgressSnapshot
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category="Horror|Objectives")
	TArray<FHorrorFoundFootageMilestone> Milestones;

	UPROPERTY(BlueprintReadOnly, Category="Horror|Objectives")
	int32 CompletedMilestoneCount = 0;

	UPROPERTY(BlueprintReadOnly, Category="Horror|Objectives")
	bool bExitUnlocked = false;
};

struct HORRORPROJECT_API FHorrorFoundFootageSaveState
{
	TArray<FGameplayTag> RecordedObjectiveEvents;
	TArray<FGameplayTag> CompletedObjectiveStates;
};

namespace HorrorFoundFootageTags
{
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(BodycamAcquiredEventTag);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(FirstNoteCollectedEventTag);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(FirstAnomalyRecordedEventTag);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(ArchiveReviewedEventTag);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(ExitUnlockedEventTag);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(BodycamAcquiredStateTag);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(FirstNoteCollectedStateTag);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(FirstAnomalyRecordedStateTag);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(ArchiveReviewedStateTag);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(ExitUnlockedStateTag);

	HORRORPROJECT_API FGameplayTag BodycamAcquiredEvent();
	HORRORPROJECT_API FGameplayTag FirstNoteCollectedEvent();
	HORRORPROJECT_API FGameplayTag FirstAnomalyRecordedEvent();
	HORRORPROJECT_API FGameplayTag ArchiveReviewedEvent();
	HORRORPROJECT_API FGameplayTag ExitUnlockedEvent();

	HORRORPROJECT_API FGameplayTag BodycamAcquiredState();
	HORRORPROJECT_API FGameplayTag FirstNoteCollectedState();
	HORRORPROJECT_API FGameplayTag FirstAnomalyRecordedState();
	HORRORPROJECT_API FGameplayTag ArchiveReviewedState();
	HORRORPROJECT_API FGameplayTag ExitUnlockedState();
}

class HORRORPROJECT_API FHorrorFoundFootageContract
{
public:
	bool RecordEvent(FGameplayTag EventTag, FName SourceId, TArray<FHorrorFoundFootageRecordedEvent>* OutRecordedEvents = nullptr, TArray<FHorrorFoundFootageStateChange>* OutStateChanges = nullptr);

	bool HasRecordedEvent(FGameplayTag EventTag) const;
	bool HasCompletedState(FGameplayTag StateTag) const;
	bool IsExitUnlocked() const;
	FGameplayTag GetStateForEvent(FGameplayTag EventTag) const;
	FHorrorFoundFootageProgressSnapshot BuildSnapshot() const;
	FHorrorFoundFootageSaveState ExportSaveState() const;
	void ImportSaveState(const FHorrorFoundFootageSaveState& SaveState);

private:
	void MarkStateCompleted(FGameplayTag StateTag, TArray<FHorrorFoundFootageStateChange>* OutStateChanges);
	void TryUnlockExit(TArray<FHorrorFoundFootageRecordedEvent>* OutRecordedEvents, TArray<FHorrorFoundFootageStateChange>* OutStateChanges);
	FGameplayTag StateForEvent(FGameplayTag EventTag) const;

	TSet<FGameplayTag> RecordedEvents;
	TSet<FGameplayTag> CompletedStates;
};
