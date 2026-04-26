// Copyright Epic Games, Inc. All Rights Reserved.

#include "ArchiveMenuWidget.h"
#include "Player/HorrorPlayerCharacter.h"
#include "Player/Components/InventoryComponent.h"
#include "Player/Components/NoteRecorderComponent.h"

void UArchiveMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

void UArchiveMenuWidget::OpenArchive()
{
	if (bIsOpen)
	{
		return;
	}

	bIsOpen = true;
	RefreshArchiveData();
	BP_ArchiveOpened();

	if (APlayerController* PC = GetOwningPlayer())
	{
		PC->SetInputMode(FInputModeUIOnly());
		PC->bShowMouseCursor = true;
	}
}

void UArchiveMenuWidget::CloseArchive()
{
	if (!bIsOpen)
	{
		return;
	}

	bIsOpen = false;
	SelectedEntryId = NAME_None;
	BP_ArchiveClosed();

	if (APlayerController* PC = GetOwningPlayer())
	{
		PC->SetInputMode(FInputModeGameOnly());
		PC->bShowMouseCursor = false;
	}
}

void UArchiveMenuWidget::RefreshArchiveData()
{
	TArray<FHorrorEvidenceMetadata> EvidenceList;
	TArray<FHorrorNoteMetadata> NotesList;

	if (APlayerController* PC = GetOwningPlayer())
	{
		if (AHorrorPlayerCharacter* PlayerChar = Cast<AHorrorPlayerCharacter>(PC->GetPawn()))
		{
			if (UInventoryComponent* Inventory = PlayerChar->GetInventoryComponent())
			{
				EvidenceList = Inventory->GetCollectedEvidenceMetadata();
			}

			if (UNoteRecorderComponent* NoteRecorder = PlayerChar->GetNoteRecorderComponent())
			{
				NotesList = NoteRecorder->GetRecordedNoteMetadata();
			}
		}
	}

	BP_ArchiveDataRefreshed(EvidenceList, NotesList);
}

void UArchiveMenuWidget::SelectEntry(FName EntryId)
{
	SelectedEntryId = EntryId;
	BP_EntrySelected(EntryId);
}

void UArchiveMenuWidget::FilterByType(EArchiveEntryType Type)
{
	CurrentFilter = Type;
	BP_FilterApplied(Type);
}

void UArchiveMenuWidget::ClearFilter()
{
	CurrentFilter = EArchiveEntryType::All;
	BP_FilterApplied(EArchiveEntryType::All);
}
