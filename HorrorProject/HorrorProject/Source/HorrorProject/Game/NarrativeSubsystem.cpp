// Copyright HorrorProject. All Rights Reserved.

#include "Game/NarrativeSubsystem.h"
#include "Sound/SoundBase.h"
#include "Kismet/GameplayStatics.h"
#include "HorrorProject.h"

void UNarrativeSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	JournalEntryIndexMap.Empty();
	for (int32 i = 0; i < JournalEntries.Num(); ++i)
	{
		if (!JournalEntries[i].EntryId.IsNone())
		{
			JournalEntryIndexMap.Add(JournalEntries[i].EntryId, i);
		}
	}

	NarrativeBeatIndexMap.Empty();
	for (int32 i = 0; i < NarrativeBeats.Num(); ++i)
	{
		if (!NarrativeBeats[i].BeatId.IsNone())
		{
			NarrativeBeatIndexMap.Add(NarrativeBeats[i].BeatId, i);
		}
	}

	ChapterIndexMap.Empty();
	for (int32 i = 0; i < ChapterProgression.Num(); ++i)
	{
		if (ChapterProgression[i].ChapterTag.IsValid())
		{
			ChapterIndexMap.Add(ChapterProgression[i].ChapterTag, i);
		}
	}

	// Unlock beats with no prerequisites
	for (FHorrorNarrativeBeat& Beat : NarrativeBeats)
	{
		if (Beat.PrerequisiteBeatIds.Num() == 0 && Beat.State == ENarrativeBeatState::Locked)
		{
			Beat.State = ENarrativeBeatState::Available;
		}
	}
}

void UNarrativeSubsystem::CollectJournalEntry(FName EntryId)
{
	const int32* IndexPtr = JournalEntryIndexMap.Find(EntryId);
	if (!IndexPtr)
	{
		UE_LOG(LogHorrorProject, Warning, TEXT("CollectJournalEntry: EntryId '%s' not found"), *EntryId.ToString());
		return;
	}

	FHorrorJournalEntry& Entry = JournalEntries[*IndexPtr];
	if (Entry.bIsCollected)
	{
		return;
	}

	Entry.bIsCollected = true;
	Entry.CollectionWorldSeconds = 0.0f;

	if (UWorld* World = GetWorld())
	{
		Entry.CollectionWorldSeconds = World->GetTimeSeconds();
	}

	OnJournalEntryCollected.Broadcast(Entry);
	UpdateChapterProgress(Entry.ChapterTag);
}

void UNarrativeSubsystem::MarkJournalEntryRead(FName EntryId)
{
	const int32* IndexPtr = JournalEntryIndexMap.Find(EntryId);
	if (!IndexPtr)
	{
		return;
	}

	FHorrorJournalEntry& Entry = JournalEntries[*IndexPtr];
	if (!Entry.bIsCollected || Entry.bHasBeenRead)
	{
		return;
	}

	Entry.bHasBeenRead = true;
	OnJournalEntryRead.Broadcast(Entry);
}

bool UNarrativeSubsystem::TriggerNarrativeBeat(FName BeatId)
{
	const int32* IndexPtr = NarrativeBeatIndexMap.Find(BeatId);
	if (!IndexPtr)
	{
		UE_LOG(LogHorrorProject, Warning, TEXT("TriggerNarrativeBeat: BeatId '%s' not found"), *BeatId.ToString());
		return false;
	}

	FHorrorNarrativeBeat& Beat = NarrativeBeats[*IndexPtr];
	if (Beat.State != ENarrativeBeatState::Available)
	{
		return false;
	}

	if (!ArePrerequisitesMet(Beat))
	{
		return false;
	}

	Beat.State = ENarrativeBeatState::Playing;
	OnNarrativeBeatTriggered.Broadcast(Beat);

	if (Beat.VoiceOver)
	{
		APlayerController* PC = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr;
		if (PC)
		{
			UGameplayStatics::PlaySound2D(PC, Beat.VoiceOver);
		}
	}

	if (Beat.DisplayDuration > 0.0f)
	{
		FTimerHandle TimerHandle;
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this, BeatId]()
		{
			CompleteNarrativeBeat(BeatId);
		}, Beat.DisplayDuration, false);
	}

	return true;
}

void UNarrativeSubsystem::CompleteNarrativeBeat(FName BeatId)
{
	const int32* IndexPtr = NarrativeBeatIndexMap.Find(BeatId);
	if (!IndexPtr)
	{
		return;
	}

	FHorrorNarrativeBeat& Beat = NarrativeBeats[*IndexPtr];
	if (Beat.State != ENarrativeBeatState::Playing)
	{
		return;
	}

	Beat.State = ENarrativeBeatState::Completed;
	OnNarrativeBeatCompleted.Broadcast(Beat);
	UnlockDependentBeats(BeatId);
}

void UNarrativeSubsystem::OnEventReceived(FGameplayTag EventTag, FName SourceId)
{
	for (FHorrorNarrativeBeat& Beat : NarrativeBeats)
	{
		if (Beat.State == ENarrativeBeatState::Available
			&& Beat.TriggerEventTag == EventTag
			&& ArePrerequisitesMet(Beat))
		{
			TriggerNarrativeBeat(Beat.BeatId);
			break;
		}
	}
}

bool UNarrativeSubsystem::IsJournalEntryCollected(FName EntryId) const
{
	const int32* IndexPtr = JournalEntryIndexMap.Find(EntryId);
	if (!IndexPtr)
	{
		return false;
	}

	return JournalEntries[*IndexPtr].bIsCollected;
}

bool UNarrativeSubsystem::GetJournalEntry(FName EntryId, FHorrorJournalEntry& OutEntry) const
{
	const int32* IndexPtr = JournalEntryIndexMap.Find(EntryId);
	if (!IndexPtr)
	{
		return false;
	}

	OutEntry = JournalEntries[*IndexPtr];
	return true;
}

TArray<FHorrorJournalEntry> UNarrativeSubsystem::GetCollectedEntries() const
{
	TArray<FHorrorJournalEntry> Result;
	for (const FHorrorJournalEntry& Entry : JournalEntries)
	{
		if (Entry.bIsCollected)
		{
			Result.Add(Entry);
		}
	}
	return Result;
}

TArray<FHorrorJournalEntry> UNarrativeSubsystem::GetEntriesByChapter(FGameplayTag ChapterTag) const
{
	TArray<FHorrorJournalEntry> Result;
	for (const FHorrorJournalEntry& Entry : JournalEntries)
	{
		if (Entry.ChapterTag == ChapterTag && Entry.bIsCollected)
		{
			Result.Add(Entry);
		}
	}
	return Result;
}

TArray<FHorrorJournalEntry> UNarrativeSubsystem::GetEntriesByType(EJournalEntryType Type) const
{
	TArray<FHorrorJournalEntry> Result;
	for (const FHorrorJournalEntry& Entry : JournalEntries)
	{
		if (Entry.Type == Type && Entry.bIsCollected)
		{
			Result.Add(Entry);
		}
	}
	return Result;
}

bool UNarrativeSubsystem::GetNarrativeBeat(FName BeatId, FHorrorNarrativeBeat& OutBeat) const
{
	const int32* IndexPtr = NarrativeBeatIndexMap.Find(BeatId);
	if (!IndexPtr)
	{
		return false;
	}

	OutBeat = NarrativeBeats[*IndexPtr];
	return true;
}

TArray<FHorrorNarrativeBeat> UNarrativeSubsystem::GetAvailableBeats() const
{
	TArray<FHorrorNarrativeBeat> Result;
	for (const FHorrorNarrativeBeat& Beat : NarrativeBeats)
	{
		if (Beat.State == ENarrativeBeatState::Available)
		{
			Result.Add(Beat);
		}
	}
	return Result;
}

bool UNarrativeSubsystem::GetChapterProgress(FGameplayTag ChapterTag, FHorrorChapterProgress& OutProgress) const
{
	const int32* IndexPtr = ChapterIndexMap.Find(ChapterTag);
	if (!IndexPtr)
	{
		return false;
	}

	OutProgress = ChapterProgression[*IndexPtr];
	return true;
}

TArray<FHorrorChapterProgress> UNarrativeSubsystem::GetAllChapterProgress() const
{
	return ChapterProgression;
}

float UNarrativeSubsystem::GetOverallCompletionPercent() const
{
	int32 TotalItems = 0;
	int32 CompletedItems = 0;

	for (const FHorrorJournalEntry& Entry : JournalEntries)
	{
		TotalItems++;
		if (Entry.bIsCollected)
		{
			CompletedItems++;
		}
	}

	for (const FHorrorNarrativeBeat& Beat : NarrativeBeats)
	{
		TotalItems++;
		if (Beat.State == ENarrativeBeatState::Completed)
		{
			CompletedItems++;
		}
	}

	return TotalItems > 0 ? static_cast<float>(CompletedItems) / static_cast<float>(TotalItems) : 0.0f;
}

void UNarrativeSubsystem::UpdateChapterProgress(FGameplayTag ChapterTag)
{
	const int32* IndexPtr = ChapterIndexMap.Find(ChapterTag);
	if (!IndexPtr)
	{
		return;
	}

	FHorrorChapterProgress& Progress = ChapterProgression[*IndexPtr];
	Progress.NotesCollected = 0;
	Progress.NotesTotal = 0;

	for (const FHorrorJournalEntry& Entry : JournalEntries)
	{
		if (Entry.ChapterTag == ChapterTag)
		{
			Progress.NotesTotal++;
			if (Entry.bIsCollected)
			{
				Progress.NotesCollected++;
			}
		}
	}

	Progress.BeatsCompleted = 0;
	Progress.BeatsTotal = 0;
	for (const FHorrorNarrativeBeat& Beat : NarrativeBeats)
	{
		if (Beat.TriggerEventTag.MatchesTag(ChapterTag))
		{
			Progress.BeatsTotal++;
			if (Beat.State == ENarrativeBeatState::Completed)
			{
				Progress.BeatsCompleted++;
			}
		}
	}

	Progress.bChapterComplete = (Progress.NotesCollected == Progress.NotesTotal && Progress.NotesTotal > 0)
		&& (Progress.BeatsCompleted == Progress.BeatsTotal && Progress.BeatsTotal > 0);

	OnChapterProgressChanged.Broadcast(Progress);
}

void UNarrativeSubsystem::UnlockDependentBeats(FName CompletedBeatId)
{
	for (FHorrorNarrativeBeat& Beat : NarrativeBeats)
	{
		if (Beat.State != ENarrativeBeatState::Locked)
		{
			continue;
		}

		if (Beat.PrerequisiteBeatIds.Contains(CompletedBeatId) && ArePrerequisitesMet(Beat))
		{
			Beat.State = ENarrativeBeatState::Available;
		}
	}
}

bool UNarrativeSubsystem::ArePrerequisitesMet(const FHorrorNarrativeBeat& Beat) const
{
	for (const FName& PrereqId : Beat.PrerequisiteBeatIds)
	{
		const int32* IndexPtr = NarrativeBeatIndexMap.Find(PrereqId);
		if (!IndexPtr)
		{
			return false;
		}

		if (NarrativeBeats[*IndexPtr].State != ENarrativeBeatState::Completed)
		{
			return false;
		}
	}
	return true;
}
