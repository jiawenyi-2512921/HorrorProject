// Copyright Epic Games, Inc. All Rights Reserved.

#include "ArchiveSubsystem.h"

void UArchiveSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	ArchiveEntries.Empty();
}

void UArchiveSubsystem::Deinitialize()
{
	ArchiveEntries.Empty();
	Super::Deinitialize();
}

bool UArchiveSubsystem::AddArchiveEntry(const FExtendedEvidenceMetadata& Metadata)
{
	if (Metadata.EvidenceId.IsNone())
	{
		return false;
	}

	if (ArchiveEntries.Contains(Metadata.EvidenceId))
	{
		return false;
	}

	FArchiveEntry NewEntry;
	NewEntry.EntryId = Metadata.EvidenceId;
	NewEntry.Metadata = Metadata;
	NewEntry.UnlockedTimestamp = FDateTime::Now();
	NewEntry.bIsNew = true;
	NewEntry.bIsViewed = false;

	ArchiveEntries.Add(Metadata.EvidenceId, NewEntry);
	OnArchiveEntryAdded.Broadcast(NewEntry);

	return true;
}

bool UArchiveSubsystem::MarkEntryAsViewed(FName EntryId)
{
	if (FArchiveEntry* Entry = ArchiveEntries.Find(EntryId))
	{
		if (!Entry->bIsViewed)
		{
			Entry->bIsViewed = true;
			Entry->bIsNew = false;
			OnArchiveEntryViewed.Broadcast(EntryId);
			return true;
		}
	}
	return false;
}

bool UArchiveSubsystem::MarkEntryAsOld(FName EntryId)
{
	if (FArchiveEntry* Entry = ArchiveEntries.Find(EntryId))
	{
		Entry->bIsNew = false;
		return true;
	}
	return false;
}

bool UArchiveSubsystem::GetArchiveEntry(FName EntryId, FArchiveEntry& OutEntry) const
{
	if (const FArchiveEntry* Entry = ArchiveEntries.Find(EntryId))
	{
		OutEntry = *Entry;
		return true;
	}
	return false;
}

TArray<FArchiveEntry> UArchiveSubsystem::GetAllArchiveEntries() const
{
	TArray<FArchiveEntry> Result;
	ArchiveEntries.GenerateValueArray(Result);

	Result.Sort([](const FArchiveEntry& A, const FArchiveEntry& B)
	{
		return A.UnlockedTimestamp > B.UnlockedTimestamp;
	});

	return Result;
}

TArray<FArchiveEntry> UArchiveSubsystem::GetFilteredArchiveEntries(const FArchiveFilter& Filter) const
{
	TArray<FArchiveEntry> Result;

	for (const auto& Pair : ArchiveEntries)
	{
		if (PassesFilter(Pair.Value, Filter))
		{
			Result.Add(Pair.Value);
		}
	}

	Result.Sort([](const FArchiveEntry& A, const FArchiveEntry& B)
	{
		return A.UnlockedTimestamp > B.UnlockedTimestamp;
	});

	return Result;
}

TArray<FArchiveEntry> UArchiveSubsystem::GetNewEntries() const
{
	TArray<FArchiveEntry> Result;

	for (const auto& Pair : ArchiveEntries)
	{
		if (Pair.Value.bIsNew)
		{
			Result.Add(Pair.Value);
		}
	}

	return Result;
}

TArray<FArchiveEntry> UArchiveSubsystem::GetUnviewedEntries() const
{
	TArray<FArchiveEntry> Result;

	for (const auto& Pair : ArchiveEntries)
	{
		if (!Pair.Value.bIsViewed)
		{
			Result.Add(Pair.Value);
		}
	}

	return Result;
}

int32 UArchiveSubsystem::GetNewEntryCount() const
{
	int32 Count = 0;
	for (const auto& Pair : ArchiveEntries)
	{
		if (Pair.Value.bIsNew)
		{
			Count++;
		}
	}
	return Count;
}

int32 UArchiveSubsystem::GetTotalEntryCount() const
{
	return ArchiveEntries.Num();
}

void UArchiveSubsystem::ClearAllEntries()
{
	ArchiveEntries.Empty();
}

TArray<FName> UArchiveSubsystem::ExportArchiveState() const
{
	TArray<FName> Result;
	ArchiveEntries.GenerateKeyArray(Result);
	return Result;
}

void UArchiveSubsystem::ImportArchiveState(const TArray<FName>& EntryIds)
{
	for (const FName& EntryId : EntryIds)
	{
		if (EntryId.IsNone() || ArchiveEntries.Contains(EntryId))
		{
			continue;
		}

		FArchiveEntry Entry;
		Entry.EntryId = EntryId;
		Entry.Metadata.EvidenceId = EntryId;
		Entry.Metadata.DisplayName = FText::FromName(EntryId);
		Entry.UnlockedTimestamp = FDateTime::Now();
		Entry.bIsNew = false;
		Entry.bIsViewed = true;
		ArchiveEntries.Add(EntryId, Entry);
	}
}

TArray<FArchiveEntry> UArchiveSubsystem::ExportArchiveEntries() const
{
	return GetAllArchiveEntries();
}

void UArchiveSubsystem::ImportArchiveEntries(const TArray<FArchiveEntry>& Entries)
{
	ArchiveEntries.Empty();

	for (FArchiveEntry Entry : Entries)
	{
		if (Entry.EntryId.IsNone())
		{
			Entry.EntryId = Entry.Metadata.EvidenceId;
		}

		if (Entry.EntryId.IsNone())
		{
			continue;
		}

		if (Entry.Metadata.EvidenceId.IsNone())
		{
			Entry.Metadata.EvidenceId = Entry.EntryId;
		}

		if (Entry.UnlockedTimestamp == FDateTime())
		{
			Entry.UnlockedTimestamp = FDateTime::Now();
		}

		ArchiveEntries.Add(Entry.EntryId, Entry);
	}
}

bool UArchiveSubsystem::PassesFilter(const FArchiveEntry& Entry, const FArchiveFilter& Filter) const
{
	if (Filter.AllowedTypes.Num() > 0)
	{
		if (!Filter.AllowedTypes.Contains(Entry.Metadata.Type))
		{
			return false;
		}
	}

	if (Filter.AllowedCategories.Num() > 0)
	{
		if (!Filter.AllowedCategories.Contains(Entry.Metadata.Category))
		{
			return false;
		}
	}

	if (Filter.RequiredTags.Num() > 0)
	{
		if (!Entry.Metadata.Tags.HasAny(Filter.RequiredTags))
		{
			return false;
		}
	}

	if (Filter.bKeyEvidenceOnly && !Entry.Metadata.bIsKeyEvidence)
	{
		return false;
	}

	if (Entry.Metadata.ImportanceLevel < Filter.MinImportanceLevel)
	{
		return false;
	}

	return true;
}
