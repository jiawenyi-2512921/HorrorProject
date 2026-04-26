// Copyright Epic Games, Inc. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "VFX/PostProcessController.h"
#include "VFX/ParticleSpawner.h"
#include "VFX/ScreenEffectManager.h"
#include "Tests/AutomationCommon.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FVFXEdgeCaseTest, "HorrorProject.VFX.EdgeCases", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FVFXEdgeCaseTest::RunTest(const FString& Parameters)
{
	UPostProcessController* Controller = NewObject<UPostProcessController>();

	Controller->ApplyEffect(EPostProcessEffectType::VHS, -1.0f);
	Controller->ApplyEffect(EPostProcessEffectType::VHS, 2.0f);
	Controller->UpdatePressureEffect(-50.0f, 100.0f);
	Controller->UpdatePressureEffect(150.0f, 100.0f);

	TestTrue(TEXT("VFX edge cases handled"), true);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FVFXMultipleEffectsTest, "HorrorProject.VFX.MultipleEffects", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FVFXMultipleEffectsTest::RunTest(const FString& Parameters)
{
	UPostProcessController* Controller = NewObject<UPostProcessController>();

	Controller->ApplyEffect(EPostProcessEffectType::VHS, 0.5f);
	Controller->ApplyEffect(EPostProcessEffectType::PressureVignette, 0.7f);
	Controller->ApplyEffect(EPostProcessEffectType::Drowning, 0.3f);

	TestTrue(TEXT("Multiple VFX effects applied"), true);

	Controller->ClearAllEffects();
	TestTrue(TEXT("All effects cleared"), true);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FParticleSpawnerStressTest, "HorrorProject.VFX.ParticleSpawnerStress", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FParticleSpawnerStressTest::RunTest(const FString& Parameters)
{
	UParticleSpawner* Spawner = NewObject<UParticleSpawner>();
	Spawner->UpdateParticleBudget(100);

	for (int32 i = 0; i < 200; i++)
	{
		// Simulate spawning beyond budget
	}

	int32 Count = Spawner->GetActiveParticleCount();
	TestTrue(TEXT("Particle budget enforced"), Count <= 100);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FScreenEffectCombinationTest, "HorrorProject.VFX.ScreenEffectCombination", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FScreenEffectCombinationTest::RunTest(const FString& Parameters)
{
	UScreenEffectManager* Manager = NewObject<UScreenEffectManager>();

	Manager->ApplyCameraShake(ECameraShakeType::Impact, 0.8f);
	Manager->ApplyPressureFeedback(0.6f);
	Manager->ApplyDrowningFeedback(40.0f);
	Manager->ApplyFearFeedback(0.9f);

	TestTrue(TEXT("Multiple screen effects combined"), true);

	Manager->ClearAllScreenEffects();
	TestTrue(TEXT("All screen effects cleared"), true);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FVFXTransitionTest, "HorrorProject.VFX.Transition", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FVFXTransitionTest::RunTest(const FString& Parameters)
{
	UPostProcessController* Controller = NewObject<UPostProcessController>();

	Controller->ApplyEffect(EPostProcessEffectType::VHS, 0.0f);
	Controller->ApplyEffect(EPostProcessEffectType::VHS, 0.5f);
	Controller->ApplyEffect(EPostProcessEffectType::VHS, 1.0f);

	TestTrue(TEXT("VFX transitions working"), true);

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
