// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/Components/FearComponent.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFearComponentInitializationTest,
	"HorrorProject.Player.Fear.Initialization",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FFearComponentInitializationTest::RunTest(const FString& Parameters)
{
	UFearComponent* FearComp = NewObject<UFearComponent>();

	TestEqual(TEXT("Fear should start at 0"), FearComp->GetFearValue(), 0.0f);
	TestEqual(TEXT("Fear percent should be 0"), FearComp->GetFearPercent(), 0.0f);
	TestTrue(TEXT("Fear level should be Calm"), FearComp->IsFearLevel(EFearLevel::Calm));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFearComponentAddFearTest,
	"HorrorProject.Player.Fear.AddFear",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FFearComponentAddFearTest::RunTest(const FString& Parameters)
{
	UFearComponent* FearComp = NewObject<UFearComponent>();

	bool bAdded = FearComp->AddFear(25.0f);
	TestTrue(TEXT("Fear should be added"), bAdded);
	TestEqual(TEXT("Fear value should be 25"), FearComp->GetFearValue(), 25.0f);

	FearComp->AddFear(30.0f, FName("TestSource"));
	TestEqual(TEXT("Fear value should accumulate"), FearComp->GetFearValue(), 55.0f);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFearComponentRemoveFearTest,
	"HorrorProject.Player.Fear.RemoveFear",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FFearComponentRemoveFearTest::RunTest(const FString& Parameters)
{
	UFearComponent* FearComp = NewObject<UFearComponent>();

	FearComp->AddFear(50.0f);
	bool bRemoved = FearComp->RemoveFear(20.0f);
	TestTrue(TEXT("Fear should be removed"), bRemoved);
	TestEqual(TEXT("Fear value should be 30"), FearComp->GetFearValue(), 30.0f);

	FearComp->RemoveFear(100.0f);
	TestEqual(TEXT("Fear should not go below 0"), FearComp->GetFearValue(), 0.0f);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFearComponentFearLevelsTest,
	"HorrorProject.Player.Fear.FearLevels",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FFearComponentFearLevelsTest::RunTest(const FString& Parameters)
{
	UFearComponent* FearComp = NewObject<UFearComponent>();

	FearComp->SetFearValue(0.0f);
	TestTrue(TEXT("Should be Calm at 0%"), FearComp->IsFearLevel(EFearLevel::Calm));

	FearComp->SetFearValue(25.0f);
	TestTrue(TEXT("Should be Uneasy at 25%"), FearComp->IsFearLevel(EFearLevel::Uneasy));

	FearComp->SetFearValue(45.0f);
	TestTrue(TEXT("Should be Afraid at 45%"), FearComp->IsFearLevel(EFearLevel::Afraid));

	FearComp->SetFearValue(65.0f);
	TestTrue(TEXT("Should be Terrified at 65%"), FearComp->IsFearLevel(EFearLevel::Terrified));

	FearComp->SetFearValue(85.0f);
	TestTrue(TEXT("Should be Panicked at 85%"), FearComp->IsFearLevel(EFearLevel::Panicked));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFearComponentDecayTest,
	"HorrorProject.Player.Fear.Decay",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FFearComponentDecayTest::RunTest(const FString& Parameters)
{
	UFearComponent* FearComp = NewObject<UFearComponent>();

	FearComp->AddFear(50.0f);
	float InitialFear = FearComp->GetFearValue();

	FearComp->TickComponent(5.0f, ELevelTick::LEVELTICK_All, nullptr);

	TestTrue(TEXT("Fear should decay over time"), FearComp->GetFearValue() < InitialFear);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFearComponentEffectsTest,
	"HorrorProject.Player.Fear.Effects",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FFearComponentEffectsTest::RunTest(const FString& Parameters)
{
	UFearComponent* FearComp = NewObject<UFearComponent>();

	FearComp->SetFearValue(0.0f);
	float CalmSpeed = FearComp->GetMovementSpeedMultiplier();
	float CalmShake = FearComp->GetCameraShakeIntensity();
	float CalmVignette = FearComp->GetVignetteIntensity();
	float CalmHeartbeat = FearComp->GetHeartbeatVolume();

	FearComp->SetFearValue(100.0f);
	TestTrue(TEXT("Speed should decrease with fear"), FearComp->GetMovementSpeedMultiplier() < CalmSpeed);
	TestTrue(TEXT("Camera shake should increase with fear"), FearComp->GetCameraShakeIntensity() > CalmShake);
	TestTrue(TEXT("Vignette should increase with fear"), FearComp->GetVignetteIntensity() > CalmVignette);
	TestTrue(TEXT("Heartbeat should increase with fear"), FearComp->GetHeartbeatVolume() > CalmHeartbeat);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFearComponentMaxCapTest,
	"HorrorProject.Player.Fear.MaxCap",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FFearComponentMaxCapTest::RunTest(const FString& Parameters)
{
	UFearComponent* FearComp = NewObject<UFearComponent>();

	float MaxFear = 100.0f;
	FearComp->AddFear(MaxFear * 2.0f);

	TestEqual(TEXT("Fear should cap at max"), FearComp->GetFearValue(), MaxFear);
	TestEqual(TEXT("Fear percent should be 100%"), FearComp->GetFearPercent(), 100.0f);

	return true;
}

#endif
