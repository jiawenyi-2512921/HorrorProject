// Copyright HorrorProject. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GameplayTagContainer.h"
#include "NarrativeSubsystem.generated.h"

class USoundBase;

UENUM(BlueprintType)
enum class EJournalEntryType : uint8
{
	ResearchNote UMETA(DisplayName="研究笔记"),
	PersonalLog UMETA(DisplayName="个人日志"),
	IncidentReport UMETA(DisplayName="事故报告"),
	AudioTranscript UMETA(DisplayName="音频转录"),
	MysteriousMessage UMETA(DisplayName="神秘讯息"),
	Blueprint UMETA(DisplayName="蓝图/示意图")
};

UENUM(BlueprintType)
enum class ENarrativeBeatState : uint8
{
	Locked UMETA(DisplayName="已锁定"),
	Available UMETA(DisplayName="可播放"),
	Playing UMETA(DisplayName="播放中"),
	Completed UMETA(DisplayName="已完成"),
	Skipped UMETA(DisplayName="已跳过")
};

USTRUCT(BlueprintType)
struct FHorrorJournalEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Narrative")
	FName EntryId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Narrative")
	EJournalEntryType Type = EJournalEntryType::ResearchNote;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Narrative")
	FText Title;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Narrative")
	FText Content;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Narrative")
	FText Author;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Narrative")
	FGameplayTag ChapterTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Narrative")
	int32 SortOrder = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Narrative")
	bool bIsEncrypted = false;

	UPROPERTY(Transient, BlueprintReadOnly, Category="Narrative")
	bool bIsCollected = false;

	UPROPERTY(Transient, BlueprintReadOnly, Category="Narrative")
	bool bHasBeenRead = false;

	UPROPERTY(Transient, BlueprintReadOnly, Category="Narrative")
	float CollectionWorldSeconds = 0.0f;
};

USTRUCT(BlueprintType)
struct FHorrorNarrativeBeat
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Narrative")
	FName BeatId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Narrative")
	FGameplayTag TriggerEventTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Narrative")
	FText DialogueText;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Narrative")
	FText SpeakerName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Narrative")
	TObjectPtr<USoundBase> VoiceOver;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Narrative", meta=(ClampMin="0.0", Units="s"))
	float DisplayDuration = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Narrative")
	TArray<FName> PrerequisiteBeatIds;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Narrative")
	int32 Priority = 0;

	UPROPERTY(Transient, BlueprintReadOnly, Category="Narrative")
	ENarrativeBeatState State = ENarrativeBeatState::Locked;
};

USTRUCT(BlueprintType)
struct FHorrorChapterProgress
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Narrative")
	FGameplayTag ChapterTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Narrative")
	FText ChapterTitle;

	UPROPERTY(Transient, BlueprintReadOnly, Category="Narrative")
	int32 NotesCollected = 0;

	UPROPERTY(Transient, BlueprintReadOnly, Category="Narrative")
	int32 NotesTotal = 0;

	UPROPERTY(Transient, BlueprintReadOnly, Category="Narrative")
	int32 BeatsCompleted = 0;

	UPROPERTY(Transient, BlueprintReadOnly, Category="Narrative")
	int32 BeatsTotal = 0;

	UPROPERTY(Transient, BlueprintReadOnly, Category="Narrative")
	bool bChapterComplete = false;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FJournalEntryCollectedSignature, const FHorrorJournalEntry&, Entry);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FJournalEntryReadSignature, const FHorrorJournalEntry&, Entry);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FNarrativeBeatTriggeredSignature, const FHorrorNarrativeBeat&, Beat);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FNarrativeBeatCompletedSignature, const FHorrorNarrativeBeat&, Beat);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FChapterProgressChangedSignature, const FHorrorChapterProgress&, Progress);

UCLASS()
class HORRORPROJECT_API UNarrativeSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	UFUNCTION(BlueprintCallable, Category="Narrative")
	void CollectJournalEntry(FName EntryId);

	UFUNCTION(BlueprintCallable, Category="Narrative")
	void MarkJournalEntryRead(FName EntryId);

	UFUNCTION(BlueprintCallable, Category="Narrative")
	bool TriggerNarrativeBeat(FName BeatId);

	UFUNCTION(BlueprintCallable, Category="Narrative")
	void CompleteNarrativeBeat(FName BeatId);

	UFUNCTION(BlueprintCallable, Category="Narrative")
	void OnEventReceived(FGameplayTag EventTag, FName SourceId);

	UFUNCTION(BlueprintPure, Category="Narrative")
	bool IsJournalEntryCollected(FName EntryId) const;

	UFUNCTION(BlueprintPure, Category="Narrative")
	bool GetJournalEntry(FName EntryId, FHorrorJournalEntry& OutEntry) const;

	UFUNCTION(BlueprintPure, Category="Narrative")
	TArray<FHorrorJournalEntry> GetCollectedEntries() const;

	UFUNCTION(BlueprintPure, Category="Narrative")
	TArray<FHorrorJournalEntry> GetEntriesByChapter(FGameplayTag ChapterTag) const;

	UFUNCTION(BlueprintPure, Category="Narrative")
	TArray<FHorrorJournalEntry> GetEntriesByType(EJournalEntryType Type) const;

	UFUNCTION(BlueprintPure, Category="Narrative")
	bool GetNarrativeBeat(FName BeatId, FHorrorNarrativeBeat& OutBeat) const;

	UFUNCTION(BlueprintPure, Category="Narrative")
	TArray<FHorrorNarrativeBeat> GetAvailableBeats() const;

	UFUNCTION(BlueprintPure, Category="Narrative")
	bool GetChapterProgress(FGameplayTag ChapterTag, FHorrorChapterProgress& OutProgress) const;

	UFUNCTION(BlueprintPure, Category="Narrative")
	TArray<FHorrorChapterProgress> GetAllChapterProgress() const;

	UFUNCTION(BlueprintPure, Category="Narrative")
	float GetOverallCompletionPercent() const;

	UPROPERTY(BlueprintAssignable, Category="Narrative")
	FJournalEntryCollectedSignature OnJournalEntryCollected;

	UPROPERTY(BlueprintAssignable, Category="Narrative")
	FJournalEntryReadSignature OnJournalEntryRead;

	UPROPERTY(BlueprintAssignable, Category="Narrative")
	FNarrativeBeatTriggeredSignature OnNarrativeBeatTriggered;

	UPROPERTY(BlueprintAssignable, Category="Narrative")
	FNarrativeBeatCompletedSignature OnNarrativeBeatCompleted;

	UPROPERTY(BlueprintAssignable, Category="Narrative")
	FChapterProgressChangedSignature OnChapterProgressChanged;

	// Content registration
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Narrative")
	TArray<FHorrorJournalEntry> JournalEntries;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Narrative")
	TArray<FHorrorNarrativeBeat> NarrativeBeats;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Narrative")
	TArray<FHorrorChapterProgress> ChapterProgression;

private:
	void UpdateChapterProgress(FGameplayTag ChapterTag);
	void UnlockDependentBeats(FName CompletedBeatId);
	bool ArePrerequisitesMet(const FHorrorNarrativeBeat& Beat) const;

	TMap<FName, int32> JournalEntryIndexMap;
	TMap<FName, int32> NarrativeBeatIndexMap;
	TMap<FGameplayTag, int32> ChapterIndexMap;
};
