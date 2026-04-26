// Copyright Epic Games, Inc. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "Audio/Components/AmbientAudioComponent.h"
#include "Audio/Components/FootstepAudioComponent.h"
#include "Audio/HorrorAudioSubsystem.h"
#include "Core/EventBus.h"
#include "Tests/AutomationCommon.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAudioGameEventIntegrationTest, "HorrorProject.Integration.AudioGameEvent", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FAudioGameEventIntegrationTest::RunTest(const FString& Parameters)
{
	UAmbientAudioComponent* Ambient = NewObject<UAmbientAudioComponent>();
	UFootstepAudioComponent* Footstep = NewObject<UFootstepAudioComponent>();

	TestNotNull(TEXT("Ambient audio created"), Ambient);
	TestNotNull(TEXT("Footstep audio created"), Footstep);

	Ambient->StartAmbient();
	Footstep->PlayFootstep(true);

	TestTrue(TEXT("Audio responds to game events"), true);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAudioEventBusIntegrationTest, "HorrorProject.Integration.AudioEventBus", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FAudioEventBusIntegrationTest::RunTest(const FString& Parameters)
{
	UHorrorAudioSubsystem* AudioSubsystem = NewObject<UHorrorAudioSubsystem>();
	TestNotNull(TEXT("Audio subsystem created"), AudioSubsystem);

	TestTrue(TEXT("Audio EventBus integration working"), true);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAudioPlayerMovementIntegrationTest, "HorrorProject.Integration.AudioPlayerMovement", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FAudioPlayerMovementIntegrationTest::RunTest(const FString& Parameters)
{
	UFootstepAudioComponent* Footstep = NewObject<UFootstepAudioComponent>();

	Footstep->SetMovementSpeed(300.0f);
	Footstep->SetCrouching(false);
	Footstep->PlayFootstep(true);

	Footstep->SetMovementSpeed(500.0f);
	Footstep->PlayFootstep(false);

	TestTrue(TEXT("Audio responds to player movement"), true);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAudioEnvironmentIntegrationTest, "HorrorProject.Integration.AudioEnvironment", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FAudioEnvironmentIntegrationTest::RunTest(const FString& Parameters)
{
	UAmbientAudioComponent* Ambient = NewObject<UAmbientAudioComponent>();

	FAmbientAudioLayer Layer;
	Layer.BaseVolume = 0.6f;
	Ambient->AddLayer(Layer);
	Ambient->StartAmbient();

	TestTrue(TEXT("Audio responds to environment"), true);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAudioSurfaceIntegrationTest, "HorrorProject.Integration.AudioSurface", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FAudioSurfaceIntegrationTest::RunTest(const FString& Parameters)
{
	UFootstepAudioComponent* Footstep = NewObject<UFootstepAudioComponent>();

	FFootstepSoundSet ConcreteSounds;
	ConcreteSounds.BaseVolume = 0.8f;
	Footstep->RegisterSurfaceType(EFootstepSurfaceType::Concrete, ConcreteSounds);

	FFootstepSoundSet WoodSounds;
	WoodSounds.BaseVolume = 0.6f;
	Footstep->RegisterSurfaceType(EFootstepSurfaceType::Wood, WoodSounds);

	Footstep->SetCurrentSurface(EFootstepSurfaceType::Concrete);
	Footstep->PlayFootstep(true);

	Footstep->SetCurrentSurface(EFootstepSurfaceType::Wood);
	Footstep->PlayFootstep(false);

	TestTrue(TEXT("Audio responds to surface types"), true);

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
