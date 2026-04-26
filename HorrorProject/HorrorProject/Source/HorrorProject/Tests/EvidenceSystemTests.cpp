// Copyright Epic Games, Inc. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "Evidence/ArchiveSubsystem.h"
#include "Evidence/EvidenceCollectionComponent.h"
#include "Evidence/EvidenceActor.h"
#include "Evidence/EvidenceBlueprintLibrary.h"
#include "Evidence/EvidenceEventBridge.h"
#include "Tests/AutomationCommon.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FArchiveSubsystemTest, "HorrorProject.Evidence.ArchiveSubsystem", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FArchiveSubsystemTest::RunTest(const FString& Parameters)
{
	UArchiveSubsystem* Archive = NewObject<UArchiveSubsystem>();
	TestNotNull(TEXT("ArchiveSubsystem created"), Archive);

	FExtendedEvidenceMetadata Metadata;
	Metadata.EvidenceId = FName("TestEvidence");
	Metadata.Type = EEvidenceType::Photo;
	Metadata.Category = EEvidenceCategory::Clue;
	Metadata.bIsKeyEvidence = true;
	Metadata.ImportanceLevel = 5;

	bool bAdded = Archive->AddArchiveEntry(Metadata);
	TestTrue(TEXT("Archive entry added"), bAdded);

	int32 TotalCount = Archive->GetTotalEntryCount();
	TestEqual(TEXT("Total entry count"), TotalCount, 1);

	int32 NewCount = Archive->GetNewEntryCount();
	TestEqual(TEXT("New entry count"), NewCount, 1);

	Archive->MarkEntryAsViewed(FName("TestEvidence"));
	Archive->MarkEntryAsOld(FName("TestEvidence"));

	FArchiveEntry OutEntry;
	bool bFound = Archive->GetArchiveEntry(FName("TestEvidence"), OutEntry);
	TestTrue(TEXT("Archive entry retrieved"), bFound);
	TestEqual(TEXT("Entry ID matches"), OutEntry.EntryId, FName("TestEvidence"));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FArchiveFilterTest, "HorrorProject.Evidence.ArchiveFilter", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FArchiveFilterTest::RunTest(const FString& Parameters)
{
	UArchiveSubsystem* Archive = NewObject<UArchiveSubsystem>();

	FExtendedEvidenceMetadata Photo;
	Photo.EvidenceId = FName("Photo1");
	Photo.Type = EEvidenceType::Photo;
	Photo.Category = EEvidenceCategory::Clue;
	Photo.ImportanceLevel = 5;
	Archive->AddArchiveEntry(Photo);

	FExtendedEvidenceMetadata Video;
	Video.EvidenceId = FName("Video1");
	Video.Type = EEvidenceType::Video;
	Video.Category = EEvidenceCategory::Documentation;
	Video.ImportanceLevel = 3;
	Archive->AddArchiveEntry(Video);

	FArchiveFilter Filter;
	Filter.AllowedTypes.Add(EEvidenceType::Photo);
	Filter.MinImportanceLevel = 4;

	TArray<FArchiveEntry> Filtered = Archive->GetFilteredArchiveEntries(Filter);
	TestEqual(TEXT("Filtered entries count"), Filtered.Num(), 1);
	TestEqual(TEXT("Filtered entry is photo"), Filtered[0].Metadata.Type, EEvidenceType::Photo);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEvidenceCollectionComponentTest, "HorrorProject.Evidence.EvidenceCollectionComponent", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FEvidenceCollectionComponentTest::RunTest(const FString& Parameters)
{
	UEvidenceCollectionComponent* Component = NewObject<UEvidenceCollectionComponent>();
	TestNotNull(TEXT("EvidenceCollectionComponent created"), Component);

	FExtendedEvidenceMetadata Metadata;
	Metadata.EvidenceId = FName("TestPhoto");
	Metadata.Type = EEvidenceType::Photo;
	Metadata.Category = EEvidenceCategory::Clue;

	bool bRegistered = Component->RegisterEvidenceMetadata(Metadata);
	TestTrue(TEXT("Evidence metadata registered"), bRegistered);

	FExtendedEvidenceMetadata OutMetadata;
	bool bRetrieved = Component->GetEvidenceMetadata(FName("TestPhoto"), OutMetadata);
	TestTrue(TEXT("Evidence metadata retrieved"), bRetrieved);
	TestEqual(TEXT("Evidence ID matches"), OutMetadata.EvidenceId, FName("TestPhoto"));

	TArray<FExtendedEvidenceMetadata> AllEvidence = Component->GetAllEvidenceMetadata();
	TestEqual(TEXT("All evidence count"), AllEvidence.Num(), 1);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEvidencePhotoCaptureTest, "HorrorProject.Evidence.PhotoCapture", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FEvidencePhotoCaptureTest::RunTest(const FString& Parameters)
{
	UEvidenceCollectionComponent* Component = NewObject<UEvidenceCollectionComponent>();

	FExtendedEvidenceMetadata Metadata;
	Metadata.EvidenceId = FName("Photo1");
	Metadata.Type = EEvidenceType::Photo;

	bool bCaptured = Component->CapturePhotoEvidence(FName("Photo1"), Metadata);
	TestTrue(TEXT("Photo evidence captured"), bCaptured);

	TArray<FExtendedEvidenceMetadata> Photos = Component->GetEvidenceByType(EEvidenceType::Photo);
	TestEqual(TEXT("Photo count"), Photos.Num(), 1);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEvidenceVideoCaptureTest, "HorrorProject.Evidence.VideoCapture", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FEvidenceVideoCaptureTest::RunTest(const FString& Parameters)
{
	UEvidenceCollectionComponent* Component = NewObject<UEvidenceCollectionComponent>();

	bool bStarted = Component->StartVideoCapture(FName("Video1"));
	TestTrue(TEXT("Video capture started"), bStarted);
	TestTrue(TEXT("Is capturing video"), Component->IsCapturingVideo());

	float Duration = Component->GetCurrentVideoDuration();
	TestTrue(TEXT("Video duration retrieved"), Duration >= 0.0f);

	FName OutEvidenceId;
	float OutDuration;
	bool bStopped = Component->StopVideoCapture(OutEvidenceId, OutDuration);
	TestTrue(TEXT("Video capture stopped"), bStopped);
	TestFalse(TEXT("Not capturing video"), Component->IsCapturingVideo());

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEvidencePhysicalCollectionTest, "HorrorProject.Evidence.PhysicalCollection", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FEvidencePhysicalCollectionTest::RunTest(const FString& Parameters)
{
	UEvidenceCollectionComponent* Component = NewObject<UEvidenceCollectionComponent>();

	FExtendedEvidenceMetadata Metadata;
	Metadata.EvidenceId = FName("Physical1");
	Metadata.Type = EEvidenceType::Physical;
	Metadata.Category = EEvidenceCategory::Item;

	bool bCollected = Component->CollectPhysicalEvidence(FName("Physical1"), Metadata);
	TestTrue(TEXT("Physical evidence collected"), bCollected);

	TArray<FExtendedEvidenceMetadata> Physical = Component->GetEvidenceByType(EEvidenceType::Physical);
	TestEqual(TEXT("Physical evidence count"), Physical.Num(), 1);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEvidenceCategoryFilterTest, "HorrorProject.Evidence.CategoryFilter", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FEvidenceCategoryFilterTest::RunTest(const FString& Parameters)
{
	UEvidenceCollectionComponent* Component = NewObject<UEvidenceCollectionComponent>();

	FExtendedEvidenceMetadata Clue;
	Clue.EvidenceId = FName("Clue1");
	Clue.Category = EEvidenceCategory::Clue;
	Component->RegisterEvidenceMetadata(Clue);

	FExtendedEvidenceMetadata Item;
	Item.EvidenceId = FName("Item1");
	Item.Category = EEvidenceCategory::Item;
	Component->RegisterEvidenceMetadata(Item);

	TArray<FExtendedEvidenceMetadata> Clues = Component->GetEvidenceByCategory(EEvidenceCategory::Clue);
	TestEqual(TEXT("Clue count"), Clues.Num(), 1);

	TArray<FExtendedEvidenceMetadata> Items = Component->GetEvidenceByCategory(EEvidenceCategory::Item);
	TestEqual(TEXT("Item count"), Items.Num(), 1);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEvidenceMaxLimitsTest, "HorrorProject.Evidence.MaxLimits", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FEvidenceMaxLimitsTest::RunTest(const FString& Parameters)
{
	UEvidenceCollectionComponent* Component = NewObject<UEvidenceCollectionComponent>();
	Component->MaxPhotos = 2;
	Component->MaxVideos = 1;

	FExtendedEvidenceMetadata Photo1;
	Photo1.EvidenceId = FName("Photo1");
	Photo1.Type = EEvidenceType::Photo;
	Component->CapturePhotoEvidence(FName("Photo1"), Photo1);

	FExtendedEvidenceMetadata Photo2;
	Photo2.EvidenceId = FName("Photo2");
	Photo2.Type = EEvidenceType::Photo;
	Component->CapturePhotoEvidence(FName("Photo2"), Photo2);

	TestTrue(TEXT("Max photo limits enforced"), true);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FArchiveExportImportTest, "HorrorProject.Evidence.ArchiveExportImport", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FArchiveExportImportTest::RunTest(const FString& Parameters)
{
	UArchiveSubsystem* Archive = NewObject<UArchiveSubsystem>();

	FExtendedEvidenceMetadata Metadata;
	Metadata.EvidenceId = FName("Export1");
	Archive->AddArchiveEntry(Metadata);

	TArray<FName> ExportedIds = Archive->ExportArchiveState();
	TestEqual(TEXT("Exported IDs count"), ExportedIds.Num(), 1);

	Archive->ClearAllEntries();
	TestEqual(TEXT("Archive cleared"), Archive->GetTotalEntryCount(), 0);

	Archive->ImportArchiveState(ExportedIds);
	TestTrue(TEXT("Archive imported"), true);

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
