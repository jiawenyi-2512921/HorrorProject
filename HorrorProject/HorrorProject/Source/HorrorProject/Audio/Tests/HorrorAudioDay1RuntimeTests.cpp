// Copyright Epic Games, Inc. All Rights Reserved.

#include "Audio/HorrorAudioSubsystem.h"

#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR

#include "Game/HorrorEventBusSubsystem.h"
#include "Game/HorrorFoundFootageContract.h"
#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "UObject/UnrealType.h"

namespace
{
	float ReadReflectedFloatProperty(const UObject* Object, FName PropertyName)
	{
		const FFloatProperty* FloatProperty = Object
			? FindFProperty<FFloatProperty>(Object->GetClass(), PropertyName)
			: nullptr;
		return FloatProperty ? FloatProperty->GetPropertyValue_InContainer(Object) : 0.0f;
	}
}

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
	TestTrue(TEXT("Campaign objective completion should have default feedback audio."), AudioSubsystem->HasEventMappingForTests(FGameplayTag::RequestGameplayTag(FName(TEXT("Event.Campaign.ObjectiveCompleted")), false)));
	TestTrue(TEXT("Campaign chapter completion should have default transition audio."), AudioSubsystem->HasEventMappingForTests(FGameplayTag::RequestGameplayTag(FName(TEXT("Event.Campaign.ChapterCompleted")), false)));
	TestTrue(TEXT("Campaign ambush starts should have default pressure audio."), AudioSubsystem->HasEventMappingForTests(FGameplayTag::RequestGameplayTag(FName(TEXT("Event.Campaign.AmbushStarted")), false)));
	TestTrue(TEXT("Campaign boss weak points should have default impact audio."), AudioSubsystem->HasEventMappingForTests(FGameplayTag::RequestGameplayTag(FName(TEXT("Event.Campaign.BossWeakPoint")), false)));
	TestTrue(TEXT("Campaign boss attacks should have default scare audio."), AudioSubsystem->HasEventMappingForTests(FGameplayTag::RequestGameplayTag(FName(TEXT("Event.Campaign.BossAttack")), false)));
	TestTrue(TEXT("Circuit wiring success should have tactile feedback audio."), AudioSubsystem->HasEventMappingForTests(FGameplayTag::RequestGameplayTag(FName(TEXT("Event.Campaign.Advanced.Circuit.Success")), false)));
	TestTrue(TEXT("Circuit wiring failure should have spark failure audio."), AudioSubsystem->HasEventMappingForTests(FGameplayTag::RequestGameplayTag(FName(TEXT("Event.Campaign.Advanced.Circuit.Failure")), false)));
	TestTrue(TEXT("Gear calibration success should have mechanical feedback audio."), AudioSubsystem->HasEventMappingForTests(FGameplayTag::RequestGameplayTag(FName(TEXT("Event.Campaign.Advanced.Gear.Success")), false)));
	TestTrue(TEXT("Gear calibration failure should have jam feedback audio."), AudioSubsystem->HasEventMappingForTests(FGameplayTag::RequestGameplayTag(FName(TEXT("Event.Campaign.Advanced.Gear.Failure")), false)));
	TestTrue(TEXT("Spectral scan success should have scan-specific feedback audio."), AudioSubsystem->HasEventMappingForTests(FGameplayTag::RequestGameplayTag(FName(TEXT("Event.Campaign.Advanced.SpectralScan.Success")), false)));
	TestTrue(TEXT("Spectral scan failure should have scan-specific failure audio."), AudioSubsystem->HasEventMappingForTests(FGameplayTag::RequestGameplayTag(FName(TEXT("Event.Campaign.Advanced.SpectralScan.Failure")), false)));
	TestTrue(TEXT("Signal tuning success should have tuning-specific feedback audio."), AudioSubsystem->HasEventMappingForTests(FGameplayTag::RequestGameplayTag(FName(TEXT("Event.Campaign.Advanced.SignalTuning.Success")), false)));
	TestTrue(TEXT("Signal tuning failure should have tuning-specific failure audio."), AudioSubsystem->HasEventMappingForTests(FGameplayTag::RequestGameplayTag(FName(TEXT("Event.Campaign.Advanced.SignalTuning.Failure")), false)));
	TestTrue(TEXT("The user-provided default horror ambience should be configured as its own layer."), AudioSubsystem->HasDefaultHorrorAmbienceForTests());
	TestTrue(TEXT("Default horror ambience should use the imported MainTitles asset."), AudioSubsystem->GetDefaultHorrorAmbienceSoundPathForTests().Contains(TEXT("/Game/Horror/Audio/MainTitles")));
	const float DefaultAmbienceBaseVolume = ReadReflectedFloatProperty(AudioSubsystem, TEXT("DefaultHorrorAmbienceVolume"));
	const float DefaultAmbienceEffectiveVolume = DefaultAmbienceBaseVolume * AudioSubsystem->GetCategoryVolume(EHorrorAudioCategory::Music);
	TestTrue(TEXT("Default horror ambience should start at a clearly audible music volume."), DefaultAmbienceEffectiveVolume >= 0.45f);
	TestEqual(TEXT("Bodycam acquisition should resolve as interaction audio."), AudioSubsystem->GetEventMappingCategoryForTests(HorrorFoundFootageTags::BodycamAcquiredEvent()), EHorrorAudioCategory::Interaction);
	TestEqual(TEXT("First anomaly should resolve as anomaly audio."), AudioSubsystem->GetEventMappingCategoryForTests(HorrorFoundFootageTags::FirstAnomalyRecordedEvent()), EHorrorAudioCategory::Anomaly);
	TestEqual(TEXT("Encounter reveal should resolve as escape audio."), AudioSubsystem->GetEventMappingCategoryForTests(FGameplayTag::RequestGameplayTag(FName(TEXT("Encounter.Revealed")), false)), EHorrorAudioCategory::Escape);
	TestEqual(TEXT("Golem full chase should resolve as escape audio."), AudioSubsystem->GetEventMappingCategoryForTests(FGameplayTag::RequestGameplayTag(FName(TEXT("Encounter.Golem.FullChase")), false)), EHorrorAudioCategory::Escape);
	TestEqual(TEXT("Campaign ambush starts should resolve as escape audio."), AudioSubsystem->GetEventMappingCategoryForTests(FGameplayTag::RequestGameplayTag(FName(TEXT("Event.Campaign.AmbushStarted")), false)), EHorrorAudioCategory::Escape);
	TestEqual(TEXT("Campaign boss attacks should resolve as escape audio."), AudioSubsystem->GetEventMappingCategoryForTests(FGameplayTag::RequestGameplayTag(FName(TEXT("Event.Campaign.BossAttack")), false)), EHorrorAudioCategory::Escape);
	TestEqual(TEXT("Advanced circuit success should resolve as interaction audio."), AudioSubsystem->GetEventMappingCategoryForTests(FGameplayTag::RequestGameplayTag(FName(TEXT("Event.Campaign.Advanced.Circuit.Success")), false)), EHorrorAudioCategory::Interaction);
	TestEqual(TEXT("Advanced circuit failure should resolve as site audio."), AudioSubsystem->GetEventMappingCategoryForTests(FGameplayTag::RequestGameplayTag(FName(TEXT("Event.Campaign.Advanced.Circuit.Failure")), false)), EHorrorAudioCategory::Site);
	TestEqual(TEXT("Advanced gear success should resolve as interaction audio."), AudioSubsystem->GetEventMappingCategoryForTests(FGameplayTag::RequestGameplayTag(FName(TEXT("Event.Campaign.Advanced.Gear.Success")), false)), EHorrorAudioCategory::Interaction);
	TestEqual(TEXT("Advanced gear failure should resolve as site audio."), AudioSubsystem->GetEventMappingCategoryForTests(FGameplayTag::RequestGameplayTag(FName(TEXT("Event.Campaign.Advanced.Gear.Failure")), false)), EHorrorAudioCategory::Site);
	TestEqual(TEXT("Advanced spectral scan success should resolve as anomaly audio."), AudioSubsystem->GetEventMappingCategoryForTests(FGameplayTag::RequestGameplayTag(FName(TEXT("Event.Campaign.Advanced.SpectralScan.Success")), false)), EHorrorAudioCategory::Anomaly);
	TestEqual(TEXT("Advanced spectral scan failure should resolve as anomaly audio."), AudioSubsystem->GetEventMappingCategoryForTests(FGameplayTag::RequestGameplayTag(FName(TEXT("Event.Campaign.Advanced.SpectralScan.Failure")), false)), EHorrorAudioCategory::Anomaly);
	TestEqual(TEXT("Advanced signal tuning success should resolve as interaction audio."), AudioSubsystem->GetEventMappingCategoryForTests(FGameplayTag::RequestGameplayTag(FName(TEXT("Event.Campaign.Advanced.SignalTuning.Success")), false)), EHorrorAudioCategory::Interaction);
	TestEqual(TEXT("Advanced signal tuning failure should resolve as anomaly audio."), AudioSubsystem->GetEventMappingCategoryForTests(FGameplayTag::RequestGameplayTag(FName(TEXT("Event.Campaign.Advanced.SignalTuning.Failure")), false)), EHorrorAudioCategory::Anomaly);
	TestEqual(TEXT("Door opened should resolve as site audio."), AudioSubsystem->GetEventMappingCategoryForTests(FGameplayTag::RequestGameplayTag(FName(TEXT("Event.Interaction.Door.Opened")), false)), EHorrorAudioCategory::Site);
	TestEqual(TEXT("Day1 completion should resolve as escape audio."), AudioSubsystem->GetEventMappingCategoryForTests(HorrorDay1Tags::Day1CompletedEvent()), EHorrorAudioCategory::Escape);
	TestEqual(TEXT("Player failure should resolve as anomaly audio."), AudioSubsystem->GetEventMappingCategoryForTests(HorrorDay1Tags::PlayerFailureEvent()), EHorrorAudioCategory::Anomaly);
	TestTrue(TEXT("Default Day1 audio should register all critical gameplay feedback mappings."), AudioSubsystem->GetEventMappingCountForTests() >= 25);

	const int32 EventMappingCountBeforeAmbience = AudioSubsystem->GetEventMappingCountForTests();
	TestTrue(TEXT("Default horror ambience should start without replacing event sounds."), AudioSubsystem->StartDefaultHorrorAmbience());
	TestEqual(TEXT("Default horror ambience should own the current ambient zone."), AudioSubsystem->GetCurrentZoneId(), AudioSubsystem->GetDefaultHorrorAmbienceZoneIdForTests());
	TestEqual(TEXT("Starting ambience should preserve all original trigger-event mappings."), AudioSubsystem->GetEventMappingCountForTests(), EventMappingCountBeforeAmbience);
	TestTrue(TEXT("Objective completion trigger audio should remain available after ambience starts."), AudioSubsystem->HasEventMappingForTests(FGameplayTag::RequestGameplayTag(FName(TEXT("Event.Campaign.ObjectiveCompleted")), false)));

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorAudioDay1StageTransitionsTest,
	"HorrorProject.Audio.Day1.StageTransitions",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorAudioAdvancedInteractionOutcomeMixTest,
	"HorrorProject.Audio.Day1.AdvancedInteractionOutcomeMix",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorAudioAdvancedInteractionOutcomeMixTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for advanced interaction audio outcome coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	UHorrorAudioSubsystem* AudioSubsystem = World->GetSubsystem<UHorrorAudioSubsystem>();
	TestNotNull(TEXT("Advanced interaction audio test should create the horror audio subsystem."), AudioSubsystem);
	if (!AudioSubsystem)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	FHorrorEventMessage HazardMessage;
	HazardMessage.EventTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Event.Campaign.Advanced.Circuit.Failure")), false);
	HazardMessage.AdvancedOutcomeKind = EHorrorAdvancedInteractionOutcomeKind::Hazard;
	HazardMessage.AdvancedFaultId = TEXT("Fault.Advanced.CircuitHazard");
	TestEqual(TEXT("Hazard circuit failures should resolve to site feedback audio."), AudioSubsystem->ResolveAdvancedInteractionAudioCategoryForTests(HazardMessage), EHorrorAudioCategory::Site);
	TestTrue(TEXT("Hazard circuit failures should get a stronger spark mix."), AudioSubsystem->ResolveAdvancedInteractionAudioVolumeForTests(HazardMessage) > 1.0f);

	FHorrorEventMessage TimingMessage;
	TimingMessage.EventTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Event.Campaign.Advanced.Gear.Failure")), false);
	TimingMessage.AdvancedOutcomeKind = EHorrorAdvancedInteractionOutcomeKind::TimingFailure;
	TimingMessage.AdvancedFaultId = TEXT("Fault.Advanced.Timing");
	TestEqual(TEXT("Timing failures should stay in the site/mechanical feedback layer."), AudioSubsystem->ResolveAdvancedInteractionAudioCategoryForTests(TimingMessage), EHorrorAudioCategory::Site);
	TestTrue(TEXT("Timing failures should be audible but less explosive than hazards."), AudioSubsystem->ResolveAdvancedInteractionAudioVolumeForTests(TimingMessage) >= 0.9f && AudioSubsystem->ResolveAdvancedInteractionAudioVolumeForTests(TimingMessage) < AudioSubsystem->ResolveAdvancedInteractionAudioVolumeForTests(HazardMessage));

	FHorrorEventMessage SpectralMessage;
	SpectralMessage.EventTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Event.Campaign.Advanced.SpectralScan.Failure")), false);
	SpectralMessage.AdvancedOutcomeKind = EHorrorAdvancedInteractionOutcomeKind::SpectralConfidenceFailure;
	SpectralMessage.AdvancedFaultId = TEXT("Fault.Advanced.SpectralLowConfidence");
	TestEqual(TEXT("Spectral failures should route through anomaly audio."), AudioSubsystem->ResolveAdvancedInteractionAudioCategoryForTests(SpectralMessage), EHorrorAudioCategory::Anomaly);
	TestTrue(TEXT("Spectral failures should preserve pressure in the mix."), AudioSubsystem->ResolveAdvancedInteractionAudioVolumeForTests(SpectralMessage) >= 1.0f);

	FHorrorEventMessage SuccessMessage;
	SuccessMessage.EventTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Event.Campaign.Advanced.Circuit.Success")), false);
	SuccessMessage.AdvancedOutcomeKind = EHorrorAdvancedInteractionOutcomeKind::Success;
	TestEqual(TEXT("Advanced interaction successes should stay in interaction audio."), AudioSubsystem->ResolveAdvancedInteractionAudioCategoryForTests(SuccessMessage), EHorrorAudioCategory::Interaction);
	TestTrue(TEXT("Success feedback should stay lighter than hazard failure."), AudioSubsystem->ResolveAdvancedInteractionAudioVolumeForTests(SuccessMessage) < AudioSubsystem->ResolveAdvancedInteractionAudioVolumeForTests(HazardMessage));

	FHorrorEventMessage RuntimeHazardMessage = HazardMessage;
	TestTrue(TEXT("Runtime advanced hazard playback should use the message-aware event audio path."), AudioSubsystem->PlayEventSound(RuntimeHazardMessage));
	TestEqual(TEXT("Runtime advanced hazard playback should preserve the source event tag."), AudioSubsystem->GetLastResolvedEventAudioTagForTests(), HazardMessage.EventTag);
	TestEqual(TEXT("Runtime advanced hazard playback should resolve through the outcome category."), AudioSubsystem->GetLastResolvedEventAudioCategoryForTests(), EHorrorAudioCategory::Site);
	TestFalse(TEXT("Runtime advanced hazard playback should not be marked suppressed."), AudioSubsystem->WasLastResolvedEventAudioSuppressedForTests());
	TestTrue(TEXT("Runtime advanced hazard playback should apply the outcome volume boost on top of the mapping."), AudioSubsystem->GetLastResolvedEventAudioVolumeForTests() > 0.6f);

	FHorrorEventMessage RuntimeSuccessMessage = SuccessMessage;
	TestTrue(TEXT("Runtime advanced success playback should use the message-aware event audio path."), AudioSubsystem->PlayEventSound(RuntimeSuccessMessage));
	TestEqual(TEXT("Runtime advanced success playback should resolve through interaction audio."), AudioSubsystem->GetLastResolvedEventAudioCategoryForTests(), EHorrorAudioCategory::Interaction);
	TestTrue(TEXT("Runtime advanced success playback should be softer than hazard playback."), AudioSubsystem->GetLastResolvedEventAudioVolumeForTests() < AudioSubsystem->ResolveAdvancedInteractionAudioVolumeForTests(HazardMessage));

	FHorrorEventMessage CancelledMessage = HazardMessage;
	CancelledMessage.AdvancedOutcomeKind = EHorrorAdvancedInteractionOutcomeKind::Cancelled;
	TestFalse(TEXT("Cancelled advanced feedback should be suppressed instead of playing a stale spark."), AudioSubsystem->PlayEventSound(CancelledMessage));
	TestTrue(TEXT("Cancelled advanced feedback should record suppression for diagnostics."), AudioSubsystem->WasLastResolvedEventAudioSuppressedForTests());
	TestEqual(TEXT("Suppressed advanced feedback should keep effective volume silent."), AudioSubsystem->GetLastResolvedEventAudioVolumeForTests(), 0.0f);

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

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

	const FGameplayTag CampaignAmbushStartedTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Event.Campaign.AmbushStarted")), false);
	TestTrue(TEXT("Campaign ambush start should be accepted."), AudioSubsystem->HandleDay1Event(CampaignAmbushStartedTag, TEXT("Forest.HoldSpikeCircle")));
	TestEqual(TEXT("Campaign ambush should enter chase audio."), AudioSubsystem->GetDay1AudioStage(), EHorrorDay1AudioStage::Chase);

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
