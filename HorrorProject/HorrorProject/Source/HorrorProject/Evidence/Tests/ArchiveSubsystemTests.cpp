#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR && HORRORPROJECT_ENABLE_LEGACY_AUTOMATION_TESTS

// Copyright Epic Games, Inc. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "Evidence/ArchiveSubsystem.h"
#include "Evidence/EvidenceTypes.h"
#include "Engine/GameInstance.h"

#if WITH_DEV_AUTOMATION_TESTS

BEGIN_DEFINE_SPEC(FArchiveSubsystemSpec, "HorrorProject.Evidence.Archive",
	EAutomationTestFlags::ProductFilter | EAutomationTestFlags::EditorContext)
	TObjectPtr<UWorld> TestWorld;
	TObjectPtr<UArchiveSubsystem> ArchiveSubsystem;
END_DEFINE_SPEC(FArchiveSubsystemSpec)

void FArchiveSubsystemSpec::Define()
{
	BeforeEach([this]()
	{
		TestWorld = FAutomationEditorCommonUtils::CreateNewMap();
		UGameInstance* GameInstance = TestWorld->GetGameInstance();
		ArchiveSubsystem = GameInstance->GetSubsystem<UArchiveSubsystem>();
		ArchiveSubsystem->ClearAllEntries();
	});

	AfterEach([this]()
	{
		if (TestWorld)
		{
			TestWorld->DestroyWorld(false);
			TestWorld = nullptr;
		}
	});

	Describe("Archive Entry Management", [this]()
	{
		It("should add archive entry", [this]()
		{
			FExtendedEvidenceMetadata Metadata;
			Metadata.EvidenceId = FName("Evidence001");
			Metadata.DisplayName = FText::FromString("Test Evidence");

			bool bAdded = ArchiveSubsystem->AddArchiveEntry(Metadata);

			TestTrue("Entry added", bAdded);
			TestEqual("Total count is 1", ArchiveSubsystem->GetTotalEntryCount(), 1);
		});

		It("should not add duplicate entries", [this]()
		{
			FExtendedEvidenceMetadata Metadata;
			Metadata.EvidenceId = FName("Evidence001");

			ArchiveSubsystem->AddArchiveEntry(Metadata);
			bool bSecondAdd = ArchiveSubsystem->AddArchiveEntry(Metadata);

			TestFalse("Duplicate rejected", bSecondAdd);
			TestEqual("Still only 1 entry", ArchiveSubsystem->GetTotalEntryCount(), 1);
		});

		It("should retrieve archive entry", [this]()
		{
			FExtendedEvidenceMetadata Metadata;
			Metadata.EvidenceId = FName("Evidence001");
			Metadata.ImportanceLevel = 3;

			ArchiveSubsystem->AddArchiveEntry(Metadata);

			FArchiveEntry Retrieved;
			bool bFound = ArchiveSubsystem->GetArchiveEntry(FName("Evidence001"), Retrieved);

			TestTrue("Entry found", bFound);
			TestEqual("ID matches", Retrieved.EntryId, FName("Evidence001"));
			TestEqual("Importance matches", Retrieved.Metadata.ImportanceLevel, 3);
		});
	});

	Describe("Entry State Management", [this]()
	{
		It("should mark entry as viewed", [this]()
		{
			FExtendedEvidenceMetadata Metadata;
			Metadata.EvidenceId = FName("Evidence001");
			ArchiveSubsystem->AddArchiveEntry(Metadata);

			bool bMarked = ArchiveSubsystem->MarkEntryAsViewed(FName("Evidence001"));

			TestTrue("Marked as viewed", bMarked);

			FArchiveEntry Entry;
			ArchiveSubsystem->GetArchiveEntry(FName("Evidence001"), Entry);
			TestTrue("Entry is viewed", Entry.bIsViewed);
			TestFalse("Entry is not new", Entry.bIsNew);
		});

		It("should track new entries", [this]()
		{
			FExtendedEvidenceMetadata Metadata1;
			Metadata1.EvidenceId = FName("Evidence001");
			ArchiveSubsystem->AddArchiveEntry(Metadata1);

			FExtendedEvidenceMetadata Metadata2;
			Metadata2.EvidenceId = FName("Evidence002");
			ArchiveSubsystem->AddArchiveEntry(Metadata2);

			ArchiveSubsystem->MarkEntryAsViewed(FName("Evidence001"));

			TestEqual("One new entry", ArchiveSubsystem->GetNewEntryCount(), 1);
			TArray<FArchiveEntry> NewEntries = ArchiveSubsystem->GetNewEntries();
			TestEqual("New entries array size", NewEntries.Num(), 1);
			TestEqual("Correct new entry", NewEntries[0].EntryId, FName("Evidence002"));
		});

		It("should track unviewed entries", [this]()
		{
			FExtendedEvidenceMetadata Metadata1;
			Metadata1.EvidenceId = FName("Evidence001");
			ArchiveSubsystem->AddArchiveEntry(Metadata1);

			FExtendedEvidenceMetadata Metadata2;
			Metadata2.EvidenceId = FName("Evidence002");
			ArchiveSubsystem->AddArchiveEntry(Metadata2);

			ArchiveSubsystem->MarkEntryAsViewed(FName("Evidence001"));

			TArray<FArchiveEntry> Unviewed = ArchiveSubsystem->GetUnviewedEntries();
			TestEqual("One unviewed entry", Unviewed.Num(), 1);
			TestEqual("Correct unviewed entry", Unviewed[0].EntryId, FName("Evidence002"));
		});
	});

	Describe("Archive Filtering", [this]()
	{
		It("should filter by evidence type", [this]()
		{
			FExtendedEvidenceMetadata Photo;
			Photo.EvidenceId = FName("Photo001");
			Photo.Type = EEvidenceType::Photo;
			ArchiveSubsystem->AddArchiveEntry(Photo);

			FExtendedEvidenceMetadata Physical;
			Physical.EvidenceId = FName("Physical001");
			Physical.Type = EEvidenceType::Physical;
			ArchiveSubsystem->AddArchiveEntry(Physical);

			FArchiveFilter Filter;
			Filter.AllowedTypes.Add(EEvidenceType::Photo);

			TArray<FArchiveEntry> Filtered = ArchiveSubsystem->GetFilteredArchiveEntries(Filter);

			TestEqual("One photo entry", Filtered.Num(), 1);
			TestEqual("Correct entry", Filtered[0].EntryId, FName("Photo001"));
		});

		It("should filter by category", [this]()
		{
			FExtendedEvidenceMetadata Clue;
			Clue.EvidenceId = FName("Clue001");
			Clue.Category = EEvidenceCategory::Clue;
			ArchiveSubsystem->AddArchiveEntry(Clue);

			FExtendedEvidenceMetadata Witness;
			Witness.EvidenceId = FName("Witness001");
			Witness.Category = EEvidenceCategory::Witness;
			ArchiveSubsystem->AddArchiveEntry(Witness);

			FArchiveFilter Filter;
			Filter.AllowedCategories.Add(EEvidenceCategory::Clue);

			TArray<FArchiveEntry> Filtered = ArchiveSubsystem->GetFilteredArchiveEntries(Filter);

			TestEqual("One clue entry", Filtered.Num(), 1);
			TestEqual("Correct entry", Filtered[0].EntryId, FName("Clue001"));
		});

		It("should filter by key evidence flag", [this]()
		{
			FExtendedEvidenceMetadata Key;
			Key.EvidenceId = FName("Key001");
			Key.bIsKeyEvidence = true;
			ArchiveSubsystem->AddArchiveEntry(Key);

			FExtendedEvidenceMetadata Normal;
			Normal.EvidenceId = FName("Normal001");
			Normal.bIsKeyEvidence = false;
			ArchiveSubsystem->AddArchiveEntry(Normal);

			FArchiveFilter Filter;
			Filter.bKeyEvidenceOnly = true;

			TArray<FArchiveEntry> Filtered = ArchiveSubsystem->GetFilteredArchiveEntries(Filter);

			TestEqual("One key evidence", Filtered.Num(), 1);
			TestEqual("Correct entry", Filtered[0].EntryId, FName("Key001"));
		});

		It("should filter by importance level", [this]()
		{
			FExtendedEvidenceMetadata Low;
			Low.EvidenceId = FName("Low001");
			Low.ImportanceLevel = 1;
			ArchiveSubsystem->AddArchiveEntry(Low);

			FExtendedEvidenceMetadata High;
			High.EvidenceId = FName("High001");
			High.ImportanceLevel = 5;
			ArchiveSubsystem->AddArchiveEntry(High);

			FArchiveFilter Filter;
			Filter.MinImportanceLevel = 3;

			TArray<FArchiveEntry> Filtered = ArchiveSubsystem->GetFilteredArchiveEntries(Filter);

			TestEqual("One high importance", Filtered.Num(), 1);
			TestEqual("Correct entry", Filtered[0].EntryId, FName("High001"));
		});
	});

	Describe("Archive State Export/Import", [this]()
	{
		It("should export archive state", [this]()
		{
			FExtendedEvidenceMetadata Metadata1;
			Metadata1.EvidenceId = FName("Evidence001");
			ArchiveSubsystem->AddArchiveEntry(Metadata1);

			FExtendedEvidenceMetadata Metadata2;
			Metadata2.EvidenceId = FName("Evidence002");
			ArchiveSubsystem->AddArchiveEntry(Metadata2);

			TArray<FName> Exported = ArchiveSubsystem->ExportArchiveState();

			TestEqual("Two entries exported", Exported.Num(), 2);
			TestTrue("Contains Evidence001", Exported.Contains(FName("Evidence001")));
			TestTrue("Contains Evidence002", Exported.Contains(FName("Evidence002")));
		});

		It("should import archive state", [this]()
		{
			TArray<FName> ToImport;
			ToImport.Add(FName("Evidence001"));
			ToImport.Add(FName("Evidence002"));

			ArchiveSubsystem->ImportArchiveState(ToImport);

			TestEqual("Two entries imported", ArchiveSubsystem->GetTotalEntryCount(), 2);
		});
	});

	Describe("Archive Events", [this]()
	{
		It("should broadcast entry added event", [this]()
		{
			bool bEventFired = false;
			FName AddedId = NAME_None;

			ArchiveSubsystem->OnArchiveEntryAdded.AddLambda([&](const FArchiveEntry& Entry)
			{
				bEventFired = true;
				AddedId = Entry.EntryId;
			});

			FExtendedEvidenceMetadata Metadata;
			Metadata.EvidenceId = FName("Evidence001");
			ArchiveSubsystem->AddArchiveEntry(Metadata);

			TestTrue("Event fired", bEventFired);
			TestEqual("Correct ID in event", AddedId, FName("Evidence001"));
		});

		It("should broadcast entry viewed event", [this]()
		{
			bool bEventFired = false;
			FName ViewedId = NAME_None;

			FExtendedEvidenceMetadata Metadata;
			Metadata.EvidenceId = FName("Evidence001");
			ArchiveSubsystem->AddArchiveEntry(Metadata);

			ArchiveSubsystem->OnArchiveEntryViewed.AddLambda([&](FName EntryId)
			{
				bEventFired = true;
				ViewedId = EntryId;
			});

			ArchiveSubsystem->MarkEntryAsViewed(FName("Evidence001"));

			TestTrue("Event fired", bEventFired);
			TestEqual("Correct ID in event", ViewedId, FName("Evidence001"));
		});
	});
}

#endif

#endif // WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR
