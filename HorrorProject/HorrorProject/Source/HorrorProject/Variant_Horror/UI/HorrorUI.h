// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Game/HorrorEventBusSubsystem.h"
#include "Game/HorrorFoundFootageContract.h"
#include "Player/Components/InventoryComponent.h"
#include "Player/Components/NoteRecorderComponent.h"
#include "Player/Components/QuantumCameraComponent.h"
#include "HorrorUI.generated.h"

class AHorrorCharacter;
class UInventoryComponent;
class UNoteRecorderComponent;
class UVHSEffectComponent;

USTRUCT(BlueprintType)
struct HORRORPROJECT_API FHorrorArchiveSnapshot
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category="Horror|Archive")
	TArray<FName> CollectedEvidenceIds;

	UPROPERTY(BlueprintReadOnly, Category="Horror|Archive")
	int32 TotalEvidenceCount = 0;

	UPROPERTY(BlueprintReadOnly, Category="Horror|Archive")
	TArray<FHorrorEvidenceMetadata> CollectedEvidenceMetadata;

	UPROPERTY(BlueprintReadOnly, Category="Horror|Archive")
	TArray<FName> RecordedNoteIds;

	UPROPERTY(BlueprintReadOnly, Category="Horror|Archive")
	int32 TotalRecordedNotes = 0;

	UPROPERTY(BlueprintReadOnly, Category="Horror|Archive")
	TArray<FHorrorNoteMetadata> RecordedNoteMetadata;
};

/**
 *  Simple UI for a first person horror game
 *  Manages character sprint meter display
 */
UCLASS(abstract)
class HORRORPROJECT_API UHorrorUI : public UUserWidget
{
	GENERATED_BODY()

public:

	/** Sets up delegate listeners for the passed character */
	void SetupCharacter(AHorrorCharacter* HorrorCharacter);

	/** Builds the current archive snapshot from explicit components for C++ and test callers. */
	static FHorrorArchiveSnapshot BuildArchiveSnapshot(const UInventoryComponent* InventoryComponent, const UNoteRecorderComponent* NoteRecorderComponent);

	/** Pushes the latest evidence and note state into Blueprint. */
	UFUNCTION(BlueprintCallable, Category="Horror|Archive")
	void RefreshArchiveSnapshot();

	/** Pushes the latest objective progress state into Blueprint. */
	UFUNCTION(BlueprintCallable, Category="Horror|Objectives")
	void RefreshObjectiveProgressSnapshot();

	/** Pushes checkpoint-restored state into state-derived UI without replaying objective events. */
	UFUNCTION(BlueprintCallable, Category="Horror|Save")
	void RestampCheckpointLoadedState();

	/** Called when the character's sprint meter is updated */
	UFUNCTION()
	void OnSprintMeterUpdated(float Percent);

	/** Called when the character's sprint state changes */
	UFUNCTION()
	void OnSprintStateChanged(bool bSprinting);

	/** Called when new evidence is collected on the observed player. */
	UFUNCTION()
	void OnEvidenceCollected(FName EvidenceId, int32 TotalEvidenceCount);

	/** Called when a new note is recorded on the observed player. */
	UFUNCTION()
	void OnNoteRecorded(FName NoteId, int32 TotalRecordedNotes);

	/** Pushes the latest VHS feedback state into Blueprint. */
	UFUNCTION(BlueprintCallable, Category="Horror|VHS")
	void RefreshVHSFeedbackState();

	/** Called when VHS feedback state changes on the observed player. */
	UFUNCTION()
	void OnVHSFeedbackChanged(bool bIsActive, EQuantumCameraMode CameraMode);

	/** Binds this widget to objective events from the current world. */
	UFUNCTION(BlueprintCallable, Category="Horror|Events")
	void BindObjectiveEvents();

	/** Called when the horror event bus publishes an objective event. */
	UFUNCTION()
	void OnObjectiveEventPublished(const FHorrorEventMessage& Message);

protected:

	virtual void NativeDestruct() override;

	/** Passes control to Blueprint to update the sprint meter widgets */
	UFUNCTION(BlueprintImplementableEvent, Category="Horror", meta = (DisplayName = "体力条已更新"))
	void BP_SprintMeterUpdated(float Percent);

	/** Passes control to Blueprint to update the sprint meter status */
	UFUNCTION(BlueprintImplementableEvent, Category="Horror", meta = (DisplayName = "奔跑状态已变化"))
	void BP_SprintStateChanged(bool bSprinting);

	/** Passes control to Blueprint to refresh archive/evidence widgets from the latest component state. */
	UFUNCTION(BlueprintImplementableEvent, Category="Horror|Archive", meta = (DisplayName = "档案快照已更新"))
	void BP_ArchiveSnapshotUpdated(
		const TArray<FName>& CollectedEvidenceIds,
		int32 TotalEvidenceCount,
		const TArray<FName>& RecordedNoteIds,
		int32 TotalRecordedNotes);

	/** Passes control to Blueprint to refresh archive/evidence widgets with metadata-rich rows. */
	UFUNCTION(BlueprintImplementableEvent, Category="Horror|Archive", meta = (DisplayName = "档案元数据快照已更新"))
	void BP_ArchiveMetadataSnapshotUpdated(const FHorrorArchiveSnapshot& Snapshot);

	/** Passes control to Blueprint when new evidence is collected. */
	UFUNCTION(BlueprintImplementableEvent, Category="Horror|Archive", meta = (DisplayName = "证据已收集"))
	void BP_EvidenceCollected(FName EvidenceId, int32 TotalEvidenceCount);

	/** Passes control to Blueprint when a new note is recorded. */
	UFUNCTION(BlueprintImplementableEvent, Category="Horror|Archive", meta = (DisplayName = "笔记已记录"))
	void BP_NoteRecorded(FName NoteId, int32 TotalRecordedNotes);

	/** Passes control to Blueprint when bodycam feedback changes. */
	UFUNCTION(BlueprintImplementableEvent, Category="Horror|VHS", meta = (DisplayName = "录像带反馈已变化"))
	void BP_VHSFeedbackChanged(bool bIsActive, EQuantumCameraMode CameraMode);

	/** Passes objective event bus messages into Blueprint. */
	UFUNCTION(BlueprintImplementableEvent, Category="Horror|Events", meta = (DisplayName = "目标事件已发布"))
	void BP_ObjectiveEventPublished(const FHorrorEventMessage& Message);

	/** Passes current objective progress into Blueprint. */
	UFUNCTION(BlueprintImplementableEvent, Category="Horror|Objectives", meta = (DisplayName = "目标进度快照已更新"))
	void BP_ObjectiveProgressSnapshotUpdated(const FHorrorFoundFootageProgressSnapshot& Snapshot);

	/** Requests a concise objective/event toast from Blueprint. */
	UFUNCTION(BlueprintImplementableEvent, Category="Horror|Events", meta = (DisplayName = "请求目标提示"))
	void BP_ObjectiveToastRequested(FGameplayTag EventTag, FName SourceId, FGameplayTag StateTag);

	/** Requests a metadata-rich objective/event toast from Blueprint. */
	UFUNCTION(BlueprintImplementableEvent, Category="Horror|Events", meta = (DisplayName = "请求带元数据的目标提示"))
	void BP_ObjectiveToastRequestedWithMetadata(FGameplayTag EventTag, FName SourceId, FGameplayTag StateTag, FName TrailerBeatId, const FText& ObjectiveHint, const FText& DebugLabel);

private:

	/** Removes any active delegate bindings owned by this widget. */
	void ReleaseObservedDelegates();

	/** Tracks the currently observed character so delegate bindings stay clean across possessions. */
	TWeakObjectPtr<AHorrorCharacter> ObservedCharacter;

	/** Tracks the currently observed inventory component for archive/evidence updates. */
	TWeakObjectPtr<UInventoryComponent> ObservedInventoryComponent;

	/** Tracks the currently observed note recorder for archive/note updates. */
	TWeakObjectPtr<UNoteRecorderComponent> ObservedNoteRecorderComponent;

	/** Tracks the currently observed VHS effect component for feedback UI updates. */
	TWeakObjectPtr<UVHSEffectComponent> ObservedVHSEffectComponent;

	/** Tracks the event bus currently feeding objective messages into this widget. */
	TWeakObjectPtr<UHorrorEventBusSubsystem> ObservedEventBus;
};
