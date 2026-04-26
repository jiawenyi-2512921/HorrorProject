// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/Components/FlashlightComponent.h"
#include "Misc/AutomationTest.h"
#include "Components/SpotLightComponent.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFlashlightComponentInitializationTest,
	"HorrorProject.Player.Flashlight.Initialization",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FFlashlightComponentInitializationTest::RunTest(const FString& Parameters)
{
	UFlashlightComponent* Flashlight = NewObject<UFlashlightComponent>();

	TestFalse(TEXT("Flashlight should start off"), Flashlight->IsFlashlightOn());
	TestFalse(TEXT("Battery should not be depleted at start"), Flashlight->IsBatteryDepleted());
	TestEqual(TEXT("Battery should be at max"), Flashlight->GetBatteryPercent(), 100.0f);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFlashlightComponentToggleTest,
	"HorrorProject.Player.Flashlight.Toggle",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FFlashlightComponentToggleTest::RunTest(const FString& Parameters)
{
	UFlashlightComponent* Flashlight = NewObject<UFlashlightComponent>();

	bool bToggled = Flashlight->ToggleFlashlight();
	TestTrue(TEXT("Toggle should succeed"), bToggled);
	TestTrue(TEXT("Flashlight should be on after toggle"), Flashlight->IsFlashlightOn());

	bToggled = Flashlight->ToggleFlashlight();
	TestTrue(TEXT("Toggle off should succeed"), bToggled);
	TestFalse(TEXT("Flashlight should be off after second toggle"), Flashlight->IsFlashlightOn());

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFlashlightComponentBatteryDrainTest,
	"HorrorProject.Player.Flashlight.BatteryDrain",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FFlashlightComponentBatteryDrainTest::RunTest(const FString& Parameters)
{
	UFlashlightComponent* Flashlight = NewObject<UFlashlightComponent>();

	float InitialBattery = Flashlight->GetBatterySeconds();

	Flashlight->SetFlashlightEnabled(true);
	Flashlight->TickComponent(1.0f, ELevelTick::LEVELTICK_All, nullptr);

	TestTrue(TEXT("Battery should drain when flashlight is on"), Flashlight->GetBatterySeconds() < InitialBattery);

	Flashlight->SetFlashlightEnabled(false);
	float BatteryAfterOff = Flashlight->GetBatterySeconds();
	Flashlight->TickComponent(1.0f, ELevelTick::LEVELTICK_All, nullptr);

	TestEqual(TEXT("Battery should not drain when flashlight is off"), Flashlight->GetBatterySeconds(), BatteryAfterOff);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFlashlightComponentBatteryDepletionTest,
	"HorrorProject.Player.Flashlight.BatteryDepletion",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FFlashlightComponentBatteryDepletionTest::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	AActor* TestActor = World->SpawnActor<AActor>();

	UFlashlightComponent* Flashlight = NewObject<UFlashlightComponent>(TestActor);
	Flashlight->RegisterComponent();

	Flashlight->SetFlashlightEnabled(true);

	float MaxBattery = Flashlight->GetMaxBatterySeconds();
	Flashlight->TickComponent(MaxBattery + 1.0f, ELevelTick::LEVELTICK_All, nullptr);

	TestTrue(TEXT("Battery should be depleted"), Flashlight->IsBatteryDepleted());
	TestFalse(TEXT("Flashlight should auto-disable on depletion"), Flashlight->IsFlashlightOn());

	World->DestroyWorld(false);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFlashlightComponentBatteryRechargeTest,
	"HorrorProject.Player.Flashlight.BatteryRecharge",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FFlashlightComponentBatteryRechargeTest::RunTest(const FString& Parameters)
{
	UFlashlightComponent* Flashlight = NewObject<UFlashlightComponent>();

	Flashlight->SetFlashlightEnabled(true);
	Flashlight->TickComponent(10.0f, ELevelTick::LEVELTICK_All, nullptr);

	float BatteryBefore = Flashlight->GetBatterySeconds();

	bool bAdded = Flashlight->AddBatteryCharge(50.0f);
	TestTrue(TEXT("Battery charge should be added"), bAdded);
	TestTrue(TEXT("Battery should increase"), Flashlight->GetBatterySeconds() > BatteryBefore);

	float MaxBattery = Flashlight->GetMaxBatterySeconds();
	Flashlight->AddBatteryCharge(MaxBattery * 2.0f);
	TestEqual(TEXT("Battery should cap at max"), Flashlight->GetBatterySeconds(), MaxBattery);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFlashlightComponentSpotLightBindingTest,
	"HorrorProject.Player.Flashlight.SpotLightBinding",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FFlashlightComponentSpotLightBindingTest::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	AActor* TestActor = World->SpawnActor<AActor>();

	UFlashlightComponent* Flashlight = NewObject<UFlashlightComponent>(TestActor);
	USpotLightComponent* SpotLight = NewObject<USpotLightComponent>(TestActor);

	Flashlight->RegisterComponent();
	SpotLight->RegisterComponent();

	TestNull(TEXT("No spotlight should be bound initially"), Flashlight->GetBoundSpotLight());

	bool bBound = Flashlight->BindSpotLight(SpotLight);
	TestTrue(TEXT("Spotlight should bind successfully"), bBound);
	TestEqual(TEXT("Bound spotlight should be retrievable"), Flashlight->GetBoundSpotLight(), SpotLight);

	World->DestroyWorld(false);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFlashlightComponentBatteryPercentTest,
	"HorrorProject.Player.Flashlight.BatteryPercent",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FFlashlightComponentBatteryPercentTest::RunTest(const FString& Parameters)
{
	UFlashlightComponent* Flashlight = NewObject<UFlashlightComponent>();

	TestEqual(TEXT("Battery should start at 100%"), Flashlight->GetBatteryPercent(), 100.0f);

	Flashlight->SetFlashlightEnabled(true);
	float MaxBattery = Flashlight->GetMaxBatterySeconds();
	Flashlight->TickComponent(MaxBattery * 0.5f, ELevelTick::LEVELTICK_All, nullptr);

	TestTrue(TEXT("Battery percent should be around 50%"), FMath::IsNearlyEqual(Flashlight->GetBatteryPercent(), 50.0f, 5.0f));

	Flashlight->TickComponent(MaxBattery, ELevelTick::LEVELTICK_All, nullptr);
	TestEqual(TEXT("Battery percent should be 0% when depleted"), Flashlight->GetBatteryPercent(), 0.0f);

	return true;
}

#endif
