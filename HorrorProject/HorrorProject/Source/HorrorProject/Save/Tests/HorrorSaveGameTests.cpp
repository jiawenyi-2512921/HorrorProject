#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR && HORRORPROJECT_ENABLE_LEGACY_AUTOMATION_TESTS

// Copyright Epic Games, Inc. All Rights Reserved.

#include "Save/HorrorSaveGame.h"
#include "Misc/AutomationTest.h"
#include "GameplayTagsManager.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorSaveGameInitializationTest,
	"HorrorProject.Save.SaveGame.Initialization",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorSaveGameInitializationTest::RunTest(const FString& Parameters)
{
	UHorrorSaveGame* SaveGame = NewObject<UHorrorSaveGame>();

	TestNotNull(TEXT("Save game should be created"), SaveGame);
	TestEqual(TEXT("Save version should be 1"), SaveGame->SaveVersion, 1);
	TestEqual(TEXT("Checkpoint ID should be None"), SaveGame->CheckpointId, NAME_None);
	TestEqual(TEXT("Recorded objective events should be empty"), SaveGame->RecordedObjectiveEvents.Num(), 0);
	TestEqual(TEXT("Completed objective states should be empty"), SaveGame->CompletedObjectiveStates.Num(), 0);
	TestEqual(TEXT("Collected evidence IDs should be empty"), SaveGame->CollectedEvidenceIds.Num(), 0);
	TestEqual(TEXT("Recorded note IDs should be empty"), SaveGame->RecordedNoteIds.Num(), 0);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorSaveGameCheckpointTest,
	"HorrorProject.Save.SaveGame.Checkpoint",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorSaveGameCheckpointTest::RunTest(const FString& Parameters)
{
	UHorrorSaveGame* SaveGame = NewObject<UHorrorSaveGame>();

	SaveGame->CheckpointId = FName("Checkpoint_01");
	TestEqual(TEXT("Checkpoint ID should be set"), SaveGame->CheckpointId, FName("Checkpoint_01"));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorSaveGameObjectiveEventsTest,
	"HorrorProject.Save.SaveGame.ObjectiveEvents",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorSaveGameObjectiveEventsTest::RunTest(const FString& Parameters)
{
	UHorrorSaveGame* SaveGame = NewObject<UHorrorSaveGame>();

	FGameplayTag Event1 = FGameplayTag::RequestGameplayTag(FName("Horror.Objective.Event1"));
	FGameplayTag Event2 = FGameplayTag::RequestGameplayTag(FName("Horror.Objective.Event2"));

	SaveGame->RecordedObjectiveEvents.Add(Event1);
	SaveGame->RecordedObjectiveEvents.Add(Event2);

	TestEqual(TEXT("Should have 2 recorded events"), SaveGame->RecordedObjectiveEvents.Num(), 2);
	TestTrue(TEXT("Should contain Event1"), SaveGame->RecordedObjectiveEvents.Contains(Event1));
	TestTrue(TEXT("Should contain Event2"), SaveGame->RecordedObjectiveEvents.Contains(Event2));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorSaveGameCompletedObjectivesTest,
	"HorrorProject.Save.SaveGame.CompletedObjectives",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorSaveGameCompletedObjectivesTest::RunTest(const FString& Parameters)
{
	UHorrorSaveGame* SaveGame = NewObject<UHorrorSaveGame>();

	FGameplayTag State1 = FGameplayTag::RequestGameplayTag(FName("Horror.Objective.State1"));
	FGameplayTag State2 = FGameplayTag::RequestGameplayTag(FName("Horror.Objective.State2"));

	SaveGame->CompletedObjectiveStates.Add(State1);
	SaveGame->CompletedObjectiveStates.Add(State2);

	TestEqual(TEXT("Should have 2 completed states"), SaveGame->CompletedObjectiveStates.Num(), 2);
	TestTrue(TEXT("Should contain State1"), SaveGame->CompletedObjectiveStates.Contains(State1));
	TestTrue(TEXT("Should contain State2"), SaveGame->CompletedObjectiveStates.Contains(State2));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorSaveGameEvidenceCollectionTest,
	"HorrorProject.Save.SaveGame.EvidenceCollection",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorSaveGameEvidenceCollectionTest::RunTest(const FString& Parameters)
{
	UHorrorSaveGame* SaveGame = NewObject<UHorrorSaveGame>();

	SaveGame->CollectedEvidenceIds.Add(FName("Evidence_01"));
	SaveGame->CollectedEvidenceIds.Add(FName("Evidence_02"));
	SaveGame->CollectedEvidenceIds.Add(FName("Evidence_03"));

	TestEqual(TEXT("Should have 3 collected evidence items"), SaveGame->CollectedEvidenceIds.Num(), 3);
	TestTrue(TEXT("Should contain Evidence_01"), SaveGame->CollectedEvidenceIds.Contains(FName("Evidence_01")));
	TestTrue(TEXT("Should contain Evidence_02"), SaveGame->CollectedEvidenceIds.Contains(FName("Evidence_02")));
	TestTrue(TEXT("Should contain Evidence_03"), SaveGame->CollectedEvidenceIds.Contains(FName("Evidence_03")));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorSaveGameNoteRecordingTest,
	"HorrorProject.Save.SaveGame.NoteRecording",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorSaveGameNoteRecordingTest::RunTest(const FString& Parameters)
{
	UHorrorSaveGame* SaveGame = NewObject<UHorrorSaveGame>();

	SaveGame->RecordedNoteIds.Add(FName("Note_01"));
	SaveGame->RecordedNoteIds.Add(FName("Note_02"));

	TestEqual(TEXT("Should have 2 recorded notes"), SaveGame->RecordedNoteIds.Num(), 2);
	TestTrue(TEXT("Should contain Note_01"), SaveGame->RecordedNoteIds.Contains(FName("Note_01")));
	TestTrue(TEXT("Should contain Note_02"), SaveGame->RecordedNoteIds.Contains(FName("Note_02")));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorSaveGamePlayerTransformTest,
	"HorrorProject.Save.SaveGame.PlayerTransform",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorSaveGamePlayerTransformTest::RunTest(const FString& Parameters)
{
	UHorrorSaveGame* SaveGame = NewObject<UHorrorSaveGame>();

	FVector Location(100.0f, 200.0f, 300.0f);
	FRotator Rotation(10.0f, 20.0f, 30.0f);
	FVector Scale(1.0f, 1.0f, 1.0f);

	SaveGame->PlayerTransform = FTransform(Rotation, Location, Scale);
	SaveGame->PlayerControlRotation = FRotator(15.0f, 25.0f, 0.0f);

	TestEqual(TEXT("Player location should be saved"), SaveGame->PlayerTransform.GetLocation(), Location);
	TestEqual(TEXT("Player rotation should be saved"), SaveGame->PlayerTransform.Rotator(), Rotation);
	TestEqual(TEXT("Player control rotation should be saved"), SaveGame->PlayerControlRotation, FRotator(15.0f, 25.0f, 0.0f));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorSaveGameAnomalySourceTest,
	"HorrorProject.Save.SaveGame.AnomalySource",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorSaveGameAnomalySourceTest::RunTest(const FString& Parameters)
{
	UHorrorSaveGame* SaveGame = NewObject<UHorrorSaveGame>();

	SaveGame->PendingFirstAnomalySourceId = FName("AnomalySource_01");
	TestEqual(TEXT("Anomaly source ID should be set"), SaveGame->PendingFirstAnomalySourceId, FName("AnomalySource_01"));

	return true;
}

#endif

#endif // WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR
