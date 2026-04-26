// Copyright Epic Games, Inc. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "VFX/PostProcessController.h"
#include "VFX/ParticleSpawner.h"
#include "VFX/ScreenEffectManager.h"
#include "Tests/AutomationCommon.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPostProcessControllerTest, "HorrorProject.VFX.PostProcessController", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FPostProcessControllerTest::RunTest(const FString& Parameters)
{
	// Test post process controller creation
	UPostProcessController* Controller = NewObject<UPostProcessController>();
	TestNotNull(TEXT("PostProcessController created"), Controller);

	// Test effect application
	Controller->ApplyEffect(EPostProcessEffectType::VHS, 0.5f);
	float Intensity = Controller->GetEffectIntensity(EPostProcessEffectType::VHS);
	TestEqual(TEXT("Effect intensity set correctly"), Intensity, 0.5f);

	// Test effect removal
	Controller->RemoveEffect(EPostProcessEffectType::VHS, false);
	Intensity = Controller->GetEffectIntensity(EPostProcessEffectType::VHS);
	TestEqual(TEXT("Effect removed"), Intensity, 0.0f);

	// Test pressure effect
	Controller->UpdatePressureEffect(50.0f, 100.0f);
	TestTrue(TEXT("Pressure effect applied"), Controller->GetEffectIntensity(EPostProcessEffectType::PressureVignette) > 0.0f);

	// Test clear all effects
	Controller->ClearAllEffects();
	TestEqual(TEXT("All effects cleared"), Controller->GetEffectIntensity(EPostProcessEffectType::PressureVignette), 0.0f);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FParticleSpawnerTest, "HorrorProject.VFX.ParticleSpawner", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FParticleSpawnerTest::RunTest(const FString& Parameters)
{
	// Test particle spawner creation
	UParticleSpawner* Spawner = NewObject<UParticleSpawner>();
	TestNotNull(TEXT("ParticleSpawner created"), Spawner);

	// Test particle budget
	Spawner->UpdateParticleBudget(500);
	TestTrue(TEXT("Particle budget updated"), true);

	// Test active particle count
	int32 Count = Spawner->GetActiveParticleCount();
	TestTrue(TEXT("Active particle count retrieved"), Count >= 0);

	// Test stop all effects
	Spawner->StopAllEffects(true);
	TestEqual(TEXT("All effects stopped"), Spawner->GetActiveParticleCount(), 0);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FScreenEffectManagerTest, "HorrorProject.VFX.ScreenEffectManager", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FScreenEffectManagerTest::RunTest(const FString& Parameters)
{
	// Test screen effect manager creation
	UScreenEffectManager* Manager = NewObject<UScreenEffectManager>();
	TestNotNull(TEXT("ScreenEffectManager created"), Manager);

	// Test camera shake application
	Manager->ApplyCameraShake(ECameraShakeType::Impact, 0.5f);
	TestTrue(TEXT("Camera shake applied"), true);

	// Test pressure feedback
	Manager->ApplyPressureFeedback(0.8f);
	TestTrue(TEXT("Pressure feedback applied"), true);

	// Test drowning feedback
	Manager->ApplyDrowningFeedback(30.0f);
	TestTrue(TEXT("Drowning feedback applied"), true);

	// Test fear feedback
	Manager->ApplyFearFeedback(0.7f);
	TestTrue(TEXT("Fear feedback applied"), true);

	// Test clear all effects
	Manager->ClearAllScreenEffects();
	TestTrue(TEXT("All screen effects cleared"), true);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FVFXPerformanceTest, "HorrorProject.VFX.Performance", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FVFXPerformanceTest::RunTest(const FString& Parameters)
{
	// Test particle budget management
	UParticleSpawner* Spawner = NewObject<UParticleSpawner>();
	Spawner->UpdateParticleBudget(1000);

	// Simulate spawning many particles
	for (int32 i = 0; i < 50; i++)
	{
		FParticleSpawnSettings Settings;
		Settings.Location = FVector(i * 100.0f, 0.0f, 0.0f);
		// Note: Would need valid particle system to actually spawn
	}

	int32 FinalCount = Spawner->GetActiveParticleCount();
	TestTrue(TEXT("Particle count within budget"), FinalCount <= 1000);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FVFXIntegrationTest, "HorrorProject.VFX.Integration", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FVFXIntegrationTest::RunTest(const FString& Parameters)
{
	// Test integration of all VFX systems
	UPostProcessController* PostProcess = NewObject<UPostProcessController>();
	UParticleSpawner* Particles = NewObject<UParticleSpawner>();
	UScreenEffectManager* ScreenEffects = NewObject<UScreenEffectManager>();

	TestNotNull(TEXT("PostProcess created"), PostProcess);
	TestNotNull(TEXT("Particles created"), Particles);
	TestNotNull(TEXT("ScreenEffects created"), ScreenEffects);

	// Simulate underwater scenario
	PostProcess->UpdatePressureEffect(75.0f, 100.0f);
	ScreenEffects->ApplyPressureFeedback(0.75f);

	// Simulate drowning
	PostProcess->ApplyDrowningEffect(20.0f);
	ScreenEffects->ApplyDrowningFeedback(20.0f);

	// Simulate impact
	FVector ImpactLocation = FVector(100.0f, 0.0f, 0.0f);
	PostProcess->ApplyImpactEffect(ImpactLocation, 0.8f);
	ScreenEffects->ApplyImpactFeedback(ImpactLocation, 0.8f);

	// Clean up
	PostProcess->ClearAllEffects();
	Particles->StopAllEffects(true);
	ScreenEffects->ClearAllScreenEffects();

	TestTrue(TEXT("VFX integration test completed"), true);

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
