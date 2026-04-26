// Copyright Epic Games, Inc. All Rights Reserved.

#include "ArchiveViewerWidget.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"

void UArchiveViewerWidget::NativeConstruct()
{
	Super::NativeConstruct();
	BindToArchiveSubsystem();
	RefreshArchiveView();
}

void UArchiveViewerWidget::NativeDestruct()
{
	UnbindFromArchiveSubsystem();
	Super::NativeDestruct();
}

void UArchiveViewerWidget::RefreshArchiveView()
{
	UpdateCurrentEntries();
	BP_ArchiveEntriesUpdated(CurrentEntries);
}

void UArchiveViewerWidget::ApplyFilter(const FArchiveFilter& Filter)
{
	CurrentFilter = Filter;
	bFilterActive = true;
	RefreshArchiveView();
}

void UArchiveViewerWidget::ClearFilter()
{
	CurrentFilter = FArchiveFilter();
	bFilterActive = false;
	RefreshArchiveView();
}

void UArchiveViewerWidget::ViewEntry(FName EntryId)
{
	if (CachedArchiveSubsystem.IsValid())
	{
		CachedArchiveSubsystem->MarkEntryAsViewed(EntryId);
		RefreshArchiveView();
	}
}

int32 UArchiveViewerWidget::GetNewEntryCount() const
{
	if (CachedArchiveSubsystem.IsValid())
	{
		return CachedArchiveSubsystem->GetNewEntryCount();
	}
	return 0;
}

int32 UArchiveViewerWidget::GetTotalEntryCount() const
{
	if (CachedArchiveSubsystem.IsValid())
	{
		return CachedArchiveSubsystem->GetTotalEntryCount();
	}
	return 0;
}

void UArchiveViewerWidget::SortByDate(bool bNewestFirst)
{
	CurrentEntries.Sort([bNewestFirst](const FArchiveEntry& A, const FArchiveEntry& B)
	{
		return bNewestFirst ? (A.UnlockedTimestamp > B.UnlockedTimestamp) : (A.UnlockedTimestamp < B.UnlockedTimestamp);
	});
	BP_ArchiveEntriesUpdated(CurrentEntries);
}

void UArchiveViewerWidget::SortByImportance(bool bHighestFirst)
{
	CurrentEntries.Sort([bHighestFirst](const FArchiveEntry& A, const FArchiveEntry& B)
	{
		return bHighestFirst ? (A.Metadata.ImportanceLevel > B.Metadata.ImportanceLevel) : (A.Metadata.ImportanceLevel < B.Metadata.ImportanceLevel);
	});
	BP_ArchiveEntriesUpdated(CurrentEntries);
}

void UArchiveViewerWidget::SortByType()
{
	CurrentEntries.Sort([](const FArchiveEntry& A, const FArchiveEntry& B)
	{
		return A.Metadata.Type < B.Metadata.Type;
	});
	BP_ArchiveEntriesUpdated(CurrentEntries);
}

void UArchiveViewerWidget::OnArchiveEntryAdded(const FArchiveEntry& Entry)
{
	RefreshArchiveView();
	BP_NewEntryAdded(Entry);
}

void UArchiveViewerWidget::OnArchiveEntryViewed(FName EntryId)
{
	RefreshArchiveView();
	BP_EntryViewed(EntryId);
}

void UArchiveViewerWidget::BindToArchiveSubsystem()
{
	if (UWorld* World = GetWorld())
	{
		if (UGameInstance* GameInstance = World->GetGameInstance())
		{
			CachedArchiveSubsystem = GameInstance->GetSubsystem<UArchiveSubsystem>();
			if (CachedArchiveSubsystem.IsValid())
			{
				CachedArchiveSubsystem->OnArchiveEntryAdded.AddDynamic(this, &UArchiveViewerWidget::OnArchiveEntryAdded);
				CachedArchiveSubsystem->OnArchiveEntryViewed.AddDynamic(this, &UArchiveViewerWidget::OnArchiveEntryViewed);
			}
		}
	}
}

void UArchiveViewerWidget::UnbindFromArchiveSubsystem()
{
	if (CachedArchiveSubsystem.IsValid())
	{
		CachedArchiveSubsystem->OnArchiveEntryAdded.RemoveDynamic(this, &UArchiveViewerWidget::OnArchiveEntryAdded);
		CachedArchiveSubsystem->OnArchiveEntryViewed.RemoveDynamic(this, &UArchiveViewerWidget::OnArchiveEntryViewed);
		CachedArchiveSubsystem.Reset();
	}
}

void UArchiveViewerWidget::UpdateCurrentEntries()
{
	if (!CachedArchiveSubsystem.IsValid())
	{
		CurrentEntries.Empty();
		return;
	}

	if (bFilterActive)
	{
		CurrentEntries = CachedArchiveSubsystem->GetFilteredArchiveEntries(CurrentFilter);
	}
	else
	{
		CurrentEntries = CachedArchiveSubsystem->GetAllArchiveEntries();
	}
}
