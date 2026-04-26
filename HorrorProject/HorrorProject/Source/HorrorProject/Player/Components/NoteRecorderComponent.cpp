// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/Components/NoteRecorderComponent.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"

void UNoteRecorderDelegateProbe::HandleNoteRecorded(FName NoteId, int32 TotalRecordedNotes)
{
	++BroadcastCount;
	LastNoteId = NoteId;
	LastTotalRecordedNotes = TotalRecordedNotes;
	NoteIds.Add(NoteId);
	TotalRecordedNoteCounts.Add(TotalRecordedNotes);
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FNoteRecorderComponentStoresUniqueNoteIdsTest,
	"HorrorProject.Player.NoteRecorder.StoresUniqueNoteIds",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FNoteRecorderComponentStoresUniqueNoteIdsTest::RunTest(const FString& Parameters)
{
	UNoteRecorderComponent* NoteRecorderComponent = NewObject<UNoteRecorderComponent>();
	UNoteRecorderDelegateProbe* DelegateProbe = NewObject<UNoteRecorderDelegateProbe>();
	NoteRecorderComponent->OnNoteRecorded.AddDynamic(DelegateProbe, &UNoteRecorderDelegateProbe::HandleNoteRecorded);
	const FName FirstNoteId(TEXT("Note.Intro"));
	const FName SecondNoteId(TEXT("Note.Archive01"));

	TestFalse(TEXT("Empty note IDs should be rejected."), NoteRecorderComponent->AddRecordedNoteId(NAME_None));
	TestTrue(TEXT("First note ID should be accepted."), NoteRecorderComponent->AddRecordedNoteId(FirstNoteId));
	TestFalse(TEXT("Duplicate note IDs should be ignored."), NoteRecorderComponent->AddRecordedNoteId(FirstNoteId));
	TestTrue(TEXT("Second note ID should be accepted."), NoteRecorderComponent->AddRecordedNoteId(SecondNoteId));
	TestEqual(TEXT("Runtime note recording should broadcast unique additions only."), DelegateProbe->BroadcastCount, 2);
	TestEqual(TEXT("Note delegate should publish last recorded id."), DelegateProbe->LastNoteId, SecondNoteId);
	TestEqual(TEXT("Note delegate should publish last total count."), DelegateProbe->LastTotalRecordedNotes, 2);
	TestEqual(TEXT("Note delegate should preserve id payload order."), DelegateProbe->NoteIds, TArray<FName>({ FirstNoteId, SecondNoteId }));
	TestEqual(TEXT("Note delegate should preserve total payload order."), DelegateProbe->TotalRecordedNoteCounts, TArray<int32>({ 1, 2 }));
	TestTrue(TEXT("Known note IDs should be queryable."), NoteRecorderComponent->HasRecordedNoteId(FirstNoteId));
	TestFalse(TEXT("Unknown note IDs should not be queryable."), NoteRecorderComponent->HasRecordedNoteId(TEXT("Note.Unknown")));
	TestEqual(TEXT("Recorded note count should match unique IDs."), NoteRecorderComponent->GetRecordedNoteCount(), 2);

	const TArray<FName>& RecordedNotes = NoteRecorderComponent->GetRecordedNoteIds();
	TestEqual(TEXT("Recorded note IDs should preserve acquisition order."), RecordedNotes.Num(), 2);
	TestEqual(TEXT("First recorded note should remain first."), RecordedNotes[0], FirstNoteId);
	TestEqual(TEXT("Second recorded note should remain second."), RecordedNotes[1], SecondNoteId);

	FHorrorNoteMetadata IntroMetadata;
	IntroMetadata.NoteId = FirstNoteId;
	IntroMetadata.Title = FText::FromString(TEXT("Intro"));
	IntroMetadata.Body = FText::FromString(TEXT("The first archive note."));
	NoteRecorderComponent->RegisterNoteMetadata(IntroMetadata);

	FHorrorNoteMetadata InvalidMetadata;
	InvalidMetadata.NoteId = NAME_None;
	InvalidMetadata.Title = FText::FromString(TEXT("Invalid"));
	NoteRecorderComponent->RegisterNoteMetadata(InvalidMetadata);

	FHorrorNoteMetadata ResolvedMetadata;
	TestFalse(TEXT("NAME_None note metadata should be ignored."), NoteRecorderComponent->GetNoteMetadata(NAME_None, ResolvedMetadata));
	TestTrue(TEXT("Registered note metadata should be queryable."), NoteRecorderComponent->GetNoteMetadata(FirstNoteId, ResolvedMetadata));
	TestEqual(TEXT("Note metadata should preserve title."), ResolvedMetadata.Title.ToString(), FString(TEXT("Intro")));

	FHorrorNoteMetadata UpdatedIntroMetadata;
	UpdatedIntroMetadata.NoteId = FirstNoteId;
	UpdatedIntroMetadata.Title = FText::FromString(TEXT("Recovered Intro"));
	UpdatedIntroMetadata.Body = FText::FromString(TEXT("Updated archive note."));
	NoteRecorderComponent->RegisterNoteMetadata(UpdatedIntroMetadata);
	TestTrue(TEXT("Updated note metadata should remain queryable."), NoteRecorderComponent->GetNoteMetadata(FirstNoteId, ResolvedMetadata));
	TestEqual(TEXT("Note metadata should overwrite same-id entries deterministically."), ResolvedMetadata.Title.ToString(), FString(TEXT("Recovered Intro")));

	FHorrorNoteMetadata ArchiveMetadata;
	ArchiveMetadata.NoteId = SecondNoteId;
	ArchiveMetadata.Title = FText::FromString(TEXT("Archive"));
	NoteRecorderComponent->RegisterNoteMetadata(ArchiveMetadata);

	const TArray<FHorrorNoteMetadata> RecordedMetadata = NoteRecorderComponent->GetRecordedNoteMetadata();
	TestEqual(TEXT("Recorded note metadata should include registered recorded notes only."), RecordedMetadata.Num(), 2);
	if (RecordedMetadata.Num() == 2)
	{
		TestEqual(TEXT("Recorded note metadata should preserve recorded order for first id."), RecordedMetadata[0].NoteId, FirstNoteId);
		TestEqual(TEXT("Recorded note metadata should preserve recorded order for second id."), RecordedMetadata[1].NoteId, SecondNoteId);
		TestEqual(TEXT("Recorded note metadata should include overwritten first metadata."), RecordedMetadata[0].Title.ToString(), FString(TEXT("Recovered Intro")));
	}

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FNoteRecorderComponentImportDoesNotBroadcastTest,
	"HorrorProject.Player.NoteRecorder.ImportDoesNotBroadcast",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FNoteRecorderComponentImportDoesNotBroadcastTest::RunTest(const FString& Parameters)
{
	UNoteRecorderComponent* NoteRecorderComponent = NewObject<UNoteRecorderComponent>();
	UNoteRecorderDelegateProbe* DelegateProbe = NewObject<UNoteRecorderDelegateProbe>();
	NoteRecorderComponent->OnNoteRecorded.AddDynamic(DelegateProbe, &UNoteRecorderDelegateProbe::HandleNoteRecorded);
	TestNotNull(TEXT("Note import test should create a note recorder component."), NoteRecorderComponent);
	if (!NoteRecorderComponent)
	{
		return false;
	}

	NoteRecorderComponent->ImportRecordedNoteIds({ NAME_None, TEXT("Note.Intro"), TEXT("Note.Intro"), TEXT("Note.Archive01") });

	TestEqual(TEXT("Note import should sanitize and de-dupe ids."), NoteRecorderComponent->ExportRecordedNoteIds().Num(), 2);
	TestTrue(TEXT("Note import should restore the intro note."), NoteRecorderComponent->HasRecordedNoteId(TEXT("Note.Intro")));
	TestTrue(TEXT("Note import should restore the archive note."), NoteRecorderComponent->HasRecordedNoteId(TEXT("Note.Archive01")));
	TestEqual(TEXT("Note import should not broadcast recording delegates."), DelegateProbe->BroadcastCount, 0);

	TestTrue(TEXT("Runtime note recording should still broadcast after import."), NoteRecorderComponent->AddRecordedNoteId(TEXT("Note.RestoredRuntime")));
	TestEqual(TEXT("Runtime note recording should broadcast once."), DelegateProbe->BroadcastCount, 1);
	TestEqual(TEXT("Runtime note recording should broadcast restored runtime id."), DelegateProbe->LastNoteId, FName(TEXT("Note.RestoredRuntime")));
	TestEqual(TEXT("Runtime note recording should broadcast total after imported ids."), DelegateProbe->LastTotalRecordedNotes, 3);

	return true;
}

#endif

UNoteRecorderComponent::UNoteRecorderComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

bool UNoteRecorderComponent::AddRecordedNoteId(FName NoteId)
{
	if (NoteId.IsNone() || RecordedNoteIds.Contains(NoteId))
	{
		return false;
	}

	RecordedNoteIds.Add(NoteId);
	OnNoteRecorded.Broadcast(NoteId, RecordedNoteIds.Num());
	return true;
}

bool UNoteRecorderComponent::HasRecordedNoteId(FName NoteId) const
{
	return !NoteId.IsNone() && RecordedNoteIds.Contains(NoteId);
}

const TArray<FName>& UNoteRecorderComponent::GetRecordedNoteIds() const
{
	return RecordedNoteIds;
}

const TArray<FName>& UNoteRecorderComponent::ExportRecordedNoteIds() const
{
	return RecordedNoteIds;
}

void UNoteRecorderComponent::ImportRecordedNoteIds(const TArray<FName>& NoteIds)
{
	RecordedNoteIds.Reset();

	for (const FName NoteId : NoteIds)
	{
		if (NoteId.IsNone() || RecordedNoteIds.Contains(NoteId))
		{
			continue;
		}

		RecordedNoteIds.Add(NoteId);
	}
}

void UNoteRecorderComponent::RegisterNoteMetadata(const FHorrorNoteMetadata& Metadata)
{
	if (Metadata.NoteId.IsNone())
	{
		return;
	}

	NoteMetadataById.Add(Metadata.NoteId, Metadata);
}

bool UNoteRecorderComponent::GetNoteMetadata(FName NoteId, FHorrorNoteMetadata& OutMetadata) const
{
	if (const FHorrorNoteMetadata* Metadata = NoteMetadataById.Find(NoteId))
	{
		OutMetadata = *Metadata;
		return true;
	}

	return false;
}

TArray<FHorrorNoteMetadata> UNoteRecorderComponent::GetRecordedNoteMetadata() const
{
	TArray<FHorrorNoteMetadata> RecordedMetadata;
	for (const FName NoteId : RecordedNoteIds)
	{
		if (const FHorrorNoteMetadata* Metadata = NoteMetadataById.Find(NoteId))
		{
			RecordedMetadata.Add(*Metadata);
		}
	}
	return RecordedMetadata;
}

int32 UNoteRecorderComponent::GetRecordedNoteCount() const
{
	return RecordedNoteIds.Num();
}
