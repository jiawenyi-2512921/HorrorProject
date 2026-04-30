// Copyright HorrorProject. All Rights Reserved.

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Game/NarrativeSubsystem.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FNarrativeCollectEntryTest,
	"HorrorProject.Game.Narrative.CollectJournalEntry",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FNarrativeCollectEntryTest::RunTest(const FString& Parameters)
{
	UNarrativeSubsystem* Narrative = NewObject<UNarrativeSubsystem>();

	FHorrorJournalEntry Entry;
	Entry.EntryId = FName("TestEntry");
	Entry.Title = FText::FromString(TEXT("Test Title"));
	Entry.Type = EJournalEntryType::ResearchNote;
	Narrative->JournalEntries.Add(Entry);

	// Initialize index maps
	UNarrativeSubsystem* TestNarrative = NewObject<UNarrativeSubsystem>();
	TestNarrative->JournalEntries = Narrative->JournalEntries;

	TestFalse(TEXT("Entry should not be collected initially"), TestNarrative->IsJournalEntryCollected(FName("TestEntry")));

	TestNarrative->CollectJournalEntry(FName("TestEntry"));
	TestTrue(TEXT("Entry should be collected after CollectJournalEntry"), TestNarrative->IsJournalEntryCollected(FName("TestEntry")));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FNarrativeCollectNonexistentTest,
	"HorrorProject.Game.Narrative.CollectNonexistentEntryNoop",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FNarrativeCollectNonexistentTest::RunTest(const FString& Parameters)
{
	UNarrativeSubsystem* Narrative = NewObject<UNarrativeSubsystem>();

	// Should not crash
	Narrative->CollectJournalEntry(FName("Nonexistent"));
	TestFalse(TEXT("Nonexistent entry should not be collected"), Narrative->IsJournalEntryCollected(FName("Nonexistent")));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FNarrativeGetCollectedEntriesTest,
	"HorrorProject.Game.Narrative.GetCollectedEntriesReturnsOnlyCollected",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FNarrativeGetCollectedEntriesTest::RunTest(const FString& Parameters)
{
	UNarrativeSubsystem* Narrative = NewObject<UNarrativeSubsystem>();

	FHorrorJournalEntry Entry1;
	Entry1.EntryId = FName("Entry1");
	Entry1.Title = FText::FromString(TEXT("Entry 1"));
	Narrative->JournalEntries.Add(Entry1);

	FHorrorJournalEntry Entry2;
	Entry2.EntryId = FName("Entry2");
	Entry2.Title = FText::FromString(TEXT("Entry 2"));
	Narrative->JournalEntries.Add(Entry2);

	TArray<FHorrorJournalEntry> Collected = Narrative->GetCollectedEntries();
	TestEqual(TEXT("Should have 0 collected entries initially"), Collected.Num(), 0);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FNarrativeCompletionPercentTest,
	"HorrorProject.Game.Narrative.OverallCompletionPercent",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FNarrativeCompletionPercentTest::RunTest(const FString& Parameters)
{
	UNarrativeSubsystem* Narrative = NewObject<UNarrativeSubsystem>();

	FHorrorJournalEntry Entry1;
	Entry1.EntryId = FName("Entry1");
	Narrative->JournalEntries.Add(Entry1);

	FHorrorJournalEntry Entry2;
	Entry2.EntryId = FName("Entry2");
	Narrative->JournalEntries.Add(Entry2);

	TestTrue(TEXT("Initial completion should be 0"), FMath::IsNearlyEqual(Narrative->GetOverallCompletionPercent(), 0.0f));

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
