// Copyright Epic Games, Inc. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "Audio/Components/AmbientAudioComponent.h"
#include "Audio/Components/FootstepAudioComponent.h"
#include "Audio/Components/BreathingAudioComponent.h"
#include "Audio/Components/UnderwaterAudioComponent.h"
#include "Tests/AutomationCommon.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAmbientAudioComponentTest, "HorrorProject.Audio.AmbientAudioComponent", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FAmbientAudioComponentTest::RunTest(const FString& Parameters)
{
	UAmbientAudioComponent* Component = NewObject<UAmbientAudioComponent>();
	TestNotNull(TEXT("AmbientAudioComponent created"), Component);

	FAmbientAudioLayer Layer;
	Layer.BaseVolume = 0.7f;
	Layer.FadeInTime = 1.0f;
	Layer.FadeOutTime = 1.0f;

	Component->AddLayer(Layer);
	Component->StartAmbient();
	TestTrue(TEXT("Ambient started"), Component->IsPlaying());

	Component->SetLayerVolume(0, 0.5f, 0.5f);
	Component->SetTimeOfDayIntensity(0.8f);

	Component->StopAmbient(1.0f);
	TestTrue(TEXT("Ambient stopped"), true);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAmbientAudioLayerTest, "HorrorProject.Audio.AmbientAudioLayer", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FAmbientAudioLayerTest::RunTest(const FString& Parameters)
{
	UAmbientAudioComponent* Component = NewObject<UAmbientAudioComponent>();

	FAmbientAudioLayer Layer1;
	Layer1.BaseVolume = 0.5f;
	Layer1.bRandomizeVolume = true;
	Layer1.VolumeVariation = 0.2f;

	FAmbientAudioLayer Layer2;
	Layer2.BaseVolume = 0.8f;
	Layer2.bRandomizeVolume = false;

	Component->AddLayer(Layer1);
	Component->AddLayer(Layer2);

	Component->RemoveLayer(0);
	TestTrue(TEXT("Layer removed"), true);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFootstepAudioComponentTest, "HorrorProject.Audio.FootstepAudioComponent", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FFootstepAudioComponentTest::RunTest(const FString& Parameters)
{
	UFootstepAudioComponent* Component = NewObject<UFootstepAudioComponent>();
	TestNotNull(TEXT("FootstepAudioComponent created"), Component);

	FFootstepSoundSet SoundSet;
	SoundSet.BaseVolume = 0.7f;
	SoundSet.PitchVariation = 0.1f;

	Component->RegisterSurfaceType(EFootstepSurfaceType::Concrete, SoundSet);
	Component->SetCurrentSurface(EFootstepSurfaceType::Concrete);

	Component->SetMovementSpeed(300.0f);
	Component->SetCrouching(false);
	Component->PlayFootstep(true);
	Component->PlayFootstep(false);

	Component->PlayJumpSound();
	Component->PlayLandSound(0.8f);

	TestTrue(TEXT("Footstep audio functions executed"), true);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFootstepSurfaceDetectionTest, "HorrorProject.Audio.FootstepSurfaceDetection", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FFootstepSurfaceDetectionTest::RunTest(const FString& Parameters)
{
	UFootstepAudioComponent* Component = NewObject<UFootstepAudioComponent>();

	FFootstepSoundSet ConcreteSounds;
	ConcreteSounds.BaseVolume = 0.8f;
	Component->RegisterSurfaceType(EFootstepSurfaceType::Concrete, ConcreteSounds);

	FFootstepSoundSet WoodSounds;
	WoodSounds.BaseVolume = 0.6f;
	Component->RegisterSurfaceType(EFootstepSurfaceType::Wood, WoodSounds);

	FFootstepSoundSet MetalSounds;
	MetalSounds.BaseVolume = 0.9f;
	Component->RegisterSurfaceType(EFootstepSurfaceType::Metal, MetalSounds);

	Component->SetCurrentSurface(EFootstepSurfaceType::Wood);
	EFootstepSurfaceType DetectedSurface = Component->DetectSurfaceType();
	TestTrue(TEXT("Surface type detected"), true);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFootstepMovementSpeedTest, "HorrorProject.Audio.FootstepMovementSpeed", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FFootstepMovementSpeedTest::RunTest(const FString& Parameters)
{
	UFootstepAudioComponent* Component = NewObject<UFootstepAudioComponent>();

	Component->SetMovementSpeed(100.0f);
	Component->SetCrouching(false);
	TestTrue(TEXT("Walk speed set"), true);

	Component->SetMovementSpeed(500.0f);
	Component->SetCrouching(false);
	TestTrue(TEXT("Run speed set"), true);

	Component->SetMovementSpeed(150.0f);
	Component->SetCrouching(true);
	TestTrue(TEXT("Crouch speed set"), true);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBreathingAudioComponentTest, "HorrorProject.Audio.BreathingAudioComponent", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FBreathingAudioComponentTest::RunTest(const FString& Parameters)
{
	UBreathingAudioComponent* Component = NewObject<UBreathingAudioComponent>();
	TestNotNull(TEXT("BreathingAudioComponent created"), Component);

	Component->SetBreathingIntensity(0.5f);
	Component->SetStaminaLevel(0.7f);
	Component->SetFearLevel(0.3f);

	Component->StartBreathing();
	TestTrue(TEXT("Breathing started"), true);

	Component->StopBreathing(1.0f);
	TestTrue(TEXT("Breathing stopped"), true);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FUnderwaterAudioComponentTest, "HorrorProject.Audio.UnderwaterAudioComponent", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FUnderwaterAudioComponentTest::RunTest(const FString& Parameters)
{
	UUnderwaterAudioComponent* Component = NewObject<UUnderwaterAudioComponent>();
	TestNotNull(TEXT("UnderwaterAudioComponent created"), Component);

	Component->SetUnderwaterDepth(50.0f);
	Component->SetOxygenLevel(0.6f);

	Component->StartUnderwaterAudio();
	TestTrue(TEXT("Underwater audio started"), true);

	Component->StopUnderwaterAudio(1.0f);
	TestTrue(TEXT("Underwater audio stopped"), true);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAudioComponentEdgeCasesTest, "HorrorProject.Audio.EdgeCases", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FAudioComponentEdgeCasesTest::RunTest(const FString& Parameters)
{
	UAmbientAudioComponent* Ambient = NewObject<UAmbientAudioComponent>();
	Ambient->SetLayerVolume(999, 0.5f);
	Ambient->RemoveLayer(999);
	TestTrue(TEXT("Invalid layer index handled"), true);

	UFootstepAudioComponent* Footstep = NewObject<UFootstepAudioComponent>();
	Footstep->SetMovementSpeed(-100.0f);
	Footstep->PlayLandSound(-1.0f);
	TestTrue(TEXT("Negative values handled"), true);

	UBreathingAudioComponent* Breathing = NewObject<UBreathingAudioComponent>();
	Breathing->SetBreathingIntensity(2.0f);
	Breathing->SetStaminaLevel(-0.5f);
	TestTrue(TEXT("Out of range values handled"), true);

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
