// Copyright Epic Games, Inc. All Rights Reserved.

#include "Audio/HorrorAudioSubsystem.h"

#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR

#include "Game/HorrorFoundFootageContract.h"
#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorAudioDay1DefaultMappingsTest,
	"HorrorProject.Audio.Day1.DefaultMappings",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorAudioDay1DefaultMappingsTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for Day1 audio coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	UHorrorAudioSubsystem* AudioSubsystem = World->GetSubsystem<UHorrorAudioSubsystem>();
	TestNotNull(TEXT("Day1 audio test should create the horror audio subsystem."), AudioSubsystem);
	if (!AudioSubsystem)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	TestTrue(TEXT("Bodycam acquisition should have a default sound mapping."), AudioSubsystem->HasEventMappingForTests(HorrorFoundFootageTags::BodycamAcquiredEvent()));
	TestTrue(TEXT("First note should have a default sound mapping."), AudioSubsystem->HasEventMappingForTests(HorrorFoundFootageTags::FirstNoteCollectedEvent()));
	TestTrue(TEXT("First anomaly should have a default sound mapping."), AudioSubsystem->HasEventMappingForTests(HorrorFoundFootageTags::FirstAnomalyRecordedEvent()));
	TestTrue(TEXT("Archive review should have a default sound mapping."), AudioSubsystem->HasEventMappingForTests(HorrorFoundFootageTags::ArchiveReviewedEvent()));
	TestTrue(TEXT("Exit unlock should have a default sound mapping."), AudioSubsystem->HasEventMappingForTests(HorrorFoundFootageTags::ExitUnlockedEvent()));
	TestTrue(TEXT("Door opened should have a default Day1 interaction feedback mapping."), AudioSubsystem->HasEventMappingForTests(FGameplayTag::RequestGameplayTag(FName(TEXT("Event.Interaction.Door.Opened")), false)));
	TestTrue(TEXT("Day1 completion should have a default escape-resolution feedback mapping."), AudioSubsystem->HasEventMappingForTests(HorrorDay1Tags::Day1CompletedEvent()));
	TestTrue(TEXT("Player failure should have a default impact feedback mapping."), AudioSubsystem->HasEventMappingForTests(HorrorDay1Tags::PlayerFailureEvent()));
	TestTrue(TEXT("Encounter priming should have a default tension feedback mapping."), AudioSubsystem->HasEventMappingForTests(FGameplayTag::RequestGameplayTag(FName(TEXT("Encounter.Primed")), false)));
	TestTrue(TEXT("Encounter reveal should have a default sound mapping."), AudioSubsystem->HasEventMappingForTests(FGameplayTag::RequestGameplayTag(FName(TEXT("Encounter.Revealed")), false)));
	TestTrue(TEXT("Golem full chase should have a default pressure feedback mapping."), AudioSubsystem->HasEventMappingForTests(FGameplayTag::RequestGameplayTag(FName(TEXT("Encounter.Golem.FullChase")), false)));
	TestTrue(TEXT("Encounter resolution should have a default pressure-release feedback mapping."), AudioSubsystem->HasEventMappingForTests(FGameplayTag::RequestGameplayTag(FName(TEXT("Encounter.Resolved")), false)));
	TestEqual(TEXT("Bodycam acquisition should resolve as interaction audio."), AudioSubsystem->GetEventMappingCategoryForTests(HorrorFoundFootageTags::BodycamAcquiredEvent()), EHorrorAudioCategory::Interaction);
	TestEqual(TEXT("First anomaly should resolve as anomaly audio."), AudioSubsystem->GetEventMappingCategoryForTests(HorrorFoundFootageTags::FirstAnomalyRecordedEvent()), EHorrorAudioCategory::Anomaly);
	TestEqual(TEXT("Encounter reveal should resolve as escape audio."), AudioSubsystem->GetEventMappingCategoryForTests(FGameplayTag::RequestGameplayTag(FName(TEXT("Encounter.Revealed")), false)), EHorrorAudioCategory::Escape);
	TestEqual(TEXT("Golem full chase should resolve as escape audio."), AudioSubsystem->GetEventMappingCategoryForTests(FGameplayTag::RequestGameplayTag(FName(TEXT("Encounter.Golem.FullChase")), false)), EHorrorAudioCategory::Escape);
	TestEqual(TEXT("Door opened should resolve as site audio."), AudioSubsystem->GetEventMappingCategoryForTests(FGameplayTag::RequestGameplayTag(FName(TEXT("Event.Interaction.Door.Opened")), false)), EHorrorAudioCategory::Site);
	TestEqual(TEXT("Day1 completion should resolve as escape audio."), AudioSubsystem->GetEventMappingCategoryForTests(HorrorDay1Tags::Day1CompletedEvent()), EHorrorAudioCategory::Escape);
	TestEqual(TEXT("Player failure should resolve as anomaly audio."), AudioSubsystem->GetEventMappingCategoryForTests(HorrorDay1Tags::PlayerFailureEvent()), EHorrorAudioCategory::Anomaly);
	TestTrue(TEXT("Default Day1 audio should register all critical gameplay feedback mappings."), AudioSubsystem->GetEventMappingCountForTests() >= 12);

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorAudioDay1StageTransitionsTest,
	"HorrorProject.Audio.Day1.StageTransitions",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorAudioDay1StageTransitionsTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for Day1 audio stage coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	UHorrorAudioSubsystem* AudioSubsystem = World->GetSubsystem<UHorrorAudioSubsystem>();
	TestNotNull(TEXT("Day1 audio stage test should create the horror audio subsystem."), AudioSubsystem);
	if (!AudioSubsystem)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	TestEqual(TEXT("Day1 audio should default to exploration."), AudioSubsystem->GetDay1AudioStage(), EHorrorDay1AudioStage::Exploration);
	TestEqual(TEXT("Default stage should expose an ambient category for Blueprint wiring."), AudioSubsystem->GetDay1AudioStageCategory(), EHorrorAudioCategory::Ambient);
	TestEqual(TEXT("Default stage should expose a stable volume multiplier."), AudioSubsystem->GetDay1AudioStageVolumeMultiplier(), 0.6f);
	TestEqual(TEXT("Default stage should not have a source id yet."), AudioSubsystem->GetLastDay1AudioSourceId(), NAME_None);

	TestTrue(TEXT("First note event should be accepted."), AudioSubsystem->HandleDay1Event(HorrorFoundFootageTags::FirstNoteCollectedEvent(), TEXT("Note.Intro")));
	TestEqual(TEXT("First note should enter objective audio."), AudioSubsystem->GetDay1AudioStage(), EHorrorDay1AudioStage::Objective);
	TestEqual(TEXT("First note source should be retained."), AudioSubsystem->GetLastDay1AudioSourceId(), FName(TEXT("Note.Intro")));
	TestEqual(TEXT("First note event tag should be retained."), AudioSubsystem->GetLastDay1AudioEventTag(), HorrorFoundFootageTags::FirstNoteCollectedEvent());
	TestEqual(TEXT("Objective stage should map to interaction audio."), AudioSubsystem->GetDay1AudioStageCategory(), EHorrorAudioCategory::Interaction);

	TestTrue(TEXT("First anomaly event should be accepted."), AudioSubsystem->HandleDay1Event(HorrorFoundFootageTags::FirstAnomalyRecordedEvent(), TEXT("Evidence.Anomaly01")));
	TestEqual(TEXT("First anomaly should enter anomaly audio."), AudioSubsystem->GetDay1AudioStage(), EHorrorDay1AudioStage::Anomaly);
	TestEqual(TEXT("Anomaly stage should map to anomaly audio."), AudioSubsystem->GetDay1AudioStageCategory(), EHorrorAudioCategory::Anomaly);
	TestEqual(TEXT("Anomaly source should be retained."), AudioSubsystem->GetLastDay1AudioSourceId(), FName(TEXT("Evidence.Anomaly01")));

	const FGameplayTag EncounterRevealedTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Encounter.Revealed")), false);
	TestTrue(TEXT("Encounter reveal tag should be accepted."), AudioSubsystem->HandleDay1Event(EncounterRevealedTag, TEXT("Encounter.GolemReveal01")));
	TestEqual(TEXT("Encounter reveal should enter chase audio."), AudioSubsystem->GetDay1AudioStage(), EHorrorDay1AudioStage::Chase);
	TestEqual(TEXT("Chase stage should map to escape audio."), AudioSubsystem->GetDay1AudioStageCategory(), EHorrorAudioCategory::Escape);

	const FGameplayTag EncounterResolvedTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Encounter.Resolved")), false);
	TestTrue(TEXT("Encounter resolved tag should be accepted."), AudioSubsystem->HandleDay1Event(EncounterResolvedTag, TEXT("Encounter.GolemReveal01")));
	TestEqual(TEXT("Encounter resolution should enter resolved audio."), AudioSubsystem->GetDay1AudioStage(), EHorrorDay1AudioStage::Resolved);
	TestEqual(TEXT("Resolved stage should map to ambient audio."), AudioSubsystem->GetDay1AudioStageCategory(), EHorrorAudioCategory::Ambient);

	TestTrue(TEXT("Exit unlock should be accepted."), AudioSubsystem->HandleDay1Event(HorrorFoundFootageTags::ExitUnlockedEvent(), TEXT("FoundFootageContract")));
	TestEqual(TEXT("Exit unlock should enter escape audio."), AudioSubsystem->GetDay1AudioStage(), EHorrorDay1AudioStage::Escape);

	TestTrue(TEXT("Day1 completion should be accepted."), AudioSubsystem->HandleDay1Event(HorrorDay1Tags::Day1CompletedEvent(), TEXT("Day1")));
	TestEqual(TEXT("Day1 completion should enter complete audio."), AudioSubsystem->GetDay1AudioStage(), EHorrorDay1AudioStage::Complete);
	TestEqual(TEXT("Complete stage should map to music audio."), AudioSubsystem->GetDay1AudioStageCategory(), EHorrorAudioCategory::Music);

	TestTrue(TEXT("Player failure event should be accepted."), AudioSubsystem->HandleDay1Event(HorrorDay1Tags::PlayerFailureEvent(), TEXT("Death.Test")));
	TestEqual(TEXT("Player failure event should enter failure audio."), AudioSubsystem->GetDay1AudioStage(), EHorrorDay1AudioStage::Failure);
	TestEqual(TEXT("Player failure event source should be retained."), AudioSubsystem->GetLastDay1AudioSourceId(), FName(TEXT("Death.Test")));

	TestTrue(TEXT("Failure names should be accepted for future GameMode calls."), AudioSubsystem->HandleDay1EventName(TEXT("Day1.Failure"), TEXT("Day1")));
	TestEqual(TEXT("Failure name should enter failure audio."), AudioSubsystem->GetDay1AudioStage(), EHorrorDay1AudioStage::Failure);
	TestEqual(TEXT("Failure stage should map to anomaly audio."), AudioSubsystem->GetDay1AudioStageCategory(), EHorrorAudioCategory::Anomaly);
	TestEqual(TEXT("Failure source should be retained."), AudioSubsystem->GetLastDay1AudioSourceId(), FName(TEXT("Day1")));

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

#endif
