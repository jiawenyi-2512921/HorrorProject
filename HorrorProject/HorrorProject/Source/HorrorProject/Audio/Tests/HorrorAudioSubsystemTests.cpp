#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR && HORRORPROJECT_ENABLE_LEGACY_AUTOMATION_TESTS

// Copyright Epic Games, Inc. All Rights Reserved.

#include "Audio/HorrorAudioSubsystem.h"
#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "Engine/World.h"
#include "GameplayTagsManager.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorAudioSubsystemInitializationTest,
	"HorrorProject.Audio.Subsystem.Initialization",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorAudioSubsystemInitializationTest::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	TestNotNull(TEXT("World should be created"), World);

	UHorrorAudioSubsystem* AudioSubsystem = World->GetSubsystem<UHorrorAudioSubsystem>();
	TestNotNull(TEXT("Audio subsystem should exist"), AudioSubsystem);

	TestEqual(TEXT("Current zone should be None initially"), AudioSubsystem->GetCurrentZoneId(), NAME_None);

	World->DestroyWorld(false);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorAudioSubsystemCategoryVolumeTest,
	"HorrorProject.Audio.Subsystem.CategoryVolume",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorAudioSubsystemCategoryVolumeTest::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	UHorrorAudioSubsystem* AudioSubsystem = World->GetSubsystem<UHorrorAudioSubsystem>();

	AudioSubsystem->SetCategoryVolume(EHorrorAudioCategory::Ambient, 0.75f);
	TestEqual(TEXT("Ambient volume should be set"), AudioSubsystem->GetCategoryVolume(EHorrorAudioCategory::Ambient), 0.75f);

	AudioSubsystem->SetCategoryVolume(EHorrorAudioCategory::Anomaly, 0.5f);
	TestEqual(TEXT("Anomaly volume should be set"), AudioSubsystem->GetCategoryVolume(EHorrorAudioCategory::Anomaly), 0.5f);

	AudioSubsystem->SetCategoryVolume(EHorrorAudioCategory::Music, 0.0f);
	TestEqual(TEXT("Music volume should be muted"), AudioSubsystem->GetCategoryVolume(EHorrorAudioCategory::Music), 0.0f);

	World->DestroyWorld(false);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorAudioSubsystemZoneConfigTest,
	"HorrorProject.Audio.Subsystem.ZoneConfig",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorAudioSubsystemZoneConfigTest::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	UHorrorAudioSubsystem* AudioSubsystem = World->GetSubsystem<UHorrorAudioSubsystem>();

	FHorrorAudioZoneConfig ZoneConfig;
	ZoneConfig.ZoneId = FName("TestZone");
	ZoneConfig.AmbientVolume = 0.6f;
	ZoneConfig.FadeInDuration = 3.0f;
	ZoneConfig.FadeOutDuration = 2.0f;

	AudioSubsystem->RegisterZoneConfig(ZoneConfig);

	bool bEntered = AudioSubsystem->EnterAudioZone(FName("TestZone"));
	TestTrue(TEXT("Should enter registered zone"), bEntered);
	TestEqual(TEXT("Current zone should be TestZone"), AudioSubsystem->GetCurrentZoneId(), FName("TestZone"));

	bool bExited = AudioSubsystem->ExitAudioZone(FName("TestZone"));
	TestTrue(TEXT("Should exit current zone"), bExited);
	TestEqual(TEXT("Current zone should be None after exit"), AudioSubsystem->GetCurrentZoneId(), NAME_None);

	AudioSubsystem->UnregisterZoneConfig(FName("TestZone"));
	bool bEnteredUnregistered = AudioSubsystem->EnterAudioZone(FName("TestZone"));
	TestFalse(TEXT("Should not enter unregistered zone"), bEnteredUnregistered);

	World->DestroyWorld(false);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorAudioSubsystemEventMappingTest,
	"HorrorProject.Audio.Subsystem.EventMapping",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorAudioSubsystemEventMappingTest::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	UHorrorAudioSubsystem* AudioSubsystem = World->GetSubsystem<UHorrorAudioSubsystem>();

	FGameplayTag TestTag = FGameplayTag::RequestGameplayTag(FName("Horror.Event.Test"));

	FHorrorAudioEventMapping EventMapping;
	EventMapping.EventTag = TestTag;
	EventMapping.VolumeMultiplier = 0.8f;
	EventMapping.bUse3DAttenuation = true;
	EventMapping.AttenuationRadius = 1500.0f;

	AudioSubsystem->RegisterEventMapping(EventMapping);

	AudioSubsystem->UnregisterEventMapping(TestTag);

	World->DestroyWorld(false);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorAudioSubsystemZoneTransitionTest,
	"HorrorProject.Audio.Subsystem.ZoneTransition",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorAudioSubsystemZoneTransitionTest::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	UHorrorAudioSubsystem* AudioSubsystem = World->GetSubsystem<UHorrorAudioSubsystem>();

	FHorrorAudioZoneConfig Zone1;
	Zone1.ZoneId = FName("Zone1");
	Zone1.AmbientVolume = 0.5f;

	FHorrorAudioZoneConfig Zone2;
	Zone2.ZoneId = FName("Zone2");
	Zone2.AmbientVolume = 0.7f;

	AudioSubsystem->RegisterZoneConfig(Zone1);
	AudioSubsystem->RegisterZoneConfig(Zone2);

	AudioSubsystem->EnterAudioZone(FName("Zone1"));
	TestEqual(TEXT("Should be in Zone1"), AudioSubsystem->GetCurrentZoneId(), FName("Zone1"));

	AudioSubsystem->EnterAudioZone(FName("Zone2"));
	TestEqual(TEXT("Should transition to Zone2"), AudioSubsystem->GetCurrentZoneId(), FName("Zone2"));

	AudioSubsystem->StopAllAmbient(0.5f);

	World->DestroyWorld(false);
	return true;
}

#endif

#endif // WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR
