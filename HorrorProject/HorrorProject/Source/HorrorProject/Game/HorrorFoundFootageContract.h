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

UENUM(BlueprintType)
enum class EHorrorObjectiveTrackerStage : uint8
{
	RecoverBodycam UMETA(DisplayName="取回随身摄像机"),
	ReadFirstNote UMETA(DisplayName="阅读第一份备忘录"),
	FrameFirstAnomaly UMETA(DisplayName="对准第一个异常"),
	RecordFirstAnomaly UMETA(DisplayName="记录第一个异常"),
	ReviewArchive UMETA(DisplayName="复查档案"),
	Escape UMETA(DisplayName="撤离"),
	Day1Complete UMETA(DisplayName="第一天完成"),
	CampaignObjective UMETA(DisplayName="战役目标")
};

USTRUCT(BlueprintType)
struct HORRORPROJECT_API FHorrorObjectiveChecklistItem
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category="Horror|Objectives")
	EHorrorObjectiveTrackerStage Stage = EHorrorObjectiveTrackerStage::RecoverBodycam;

	UPROPERTY(BlueprintReadOnly, Category="Horror|Objectives")
	FText Label;

	UPROPERTY(BlueprintReadOnly, Category="Horror|Objectives")
	bool bComplete = false;

	UPROPERTY(BlueprintReadOnly, Category="Horror|Objectives")
	bool bActive = false;

	UPROPERTY(BlueprintReadOnly, Category="Horror|Objectives")
	bool bRequiresRecording = false;
};

USTRUCT(BlueprintType)
struct HORRORPROJECT_API FHorrorObjectiveTrackerSnapshot
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category="Horror|Objectives")
	EHorrorObjectiveTrackerStage Stage = EHorrorObjectiveTrackerStage::RecoverBodycam;

	UPROPERTY(BlueprintReadOnly, Category="Horror|Objectives")
	FText Title;

	UPROPERTY(BlueprintReadOnly, Category="Horror|Objectives")
	FText PrimaryInstruction;

	UPROPERTY(BlueprintReadOnly, Category="Horror|Objectives")
	FText SecondaryInstruction;

	UPROPERTY(BlueprintReadOnly, Category="Horror|Objectives")
	FText ProgressLabel;

	UPROPERTY(BlueprintReadOnly, Category="Horror|Objectives")
	TArray<FHorrorObjectiveChecklistItem> ChecklistItems;

	UPROPERTY(BlueprintReadOnly, Category="Horror|Objectives")
	int32 CompletedMilestoneCount = 0;

	UPROPERTY(BlueprintReadOnly, Category="Horror|Objectives")
	int32 RequiredMilestoneCount = 0;

	UPROPERTY(BlueprintReadOnly, Category="Horror|Objectives")
	float ProgressFraction = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category="Horror|Objectives")
	bool bRequiresRecording = false;

	UPROPERTY(BlueprintReadOnly, Category="Horror|Objectives")
	bool bUrgent = false;

	UPROPERTY(BlueprintReadOnly, Category="Horror|Objectives")
	bool bExitUnlocked = false;

	UPROPERTY(BlueprintReadOnly, Category="Horror|Objectives")
	bool bComplete = false;
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

namespace HorrorDay1Tags
{
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Day1CompletedEventTag);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(PlayerFailureEventTag);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Day1CompletedStateTag);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(CheckpointRestoredStateTag);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(CheckpointMissingStateTag);

	HORRORPROJECT_API FGameplayTag Day1CompletedEvent();
	HORRORPROJECT_API FGameplayTag PlayerFailureEvent();
	HORRORPROJECT_API FGameplayTag Day1CompletedState();
	HORRORPROJECT_API FGameplayTag CheckpointRestoredState();
	HORRORPROJECT_API FGameplayTag CheckpointMissingState();
}

namespace HorrorSaveTags
{
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(CheckpointSavedEventTag);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(CheckpointSaveFailedEventTag);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(CheckpointSavedStateTag);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(CheckpointSaveFailedStateTag);

	HORRORPROJECT_API FGameplayTag CheckpointSavedEvent();
	HORRORPROJECT_API FGameplayTag CheckpointSaveFailedEvent();
	HORRORPROJECT_API FGameplayTag CheckpointSavedState();
	HORRORPROJECT_API FGameplayTag CheckpointSaveFailedState();
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
