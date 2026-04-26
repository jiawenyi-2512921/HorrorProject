// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/Components/CameraBatteryComponent.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCameraBatteryComponentInitializationTest,
	"HorrorProject.Player.CameraBattery.Initialization",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FCameraBatteryComponentInitializationTest::RunTest(const FString& Parameters)
{
	UCameraBatteryComponent* BatteryComponent = NewObject<UCameraBatteryComponent>();

	TestEqual(TEXT("Battery should start at 100%."), BatteryComponent->GetBatteryPercentage(), 100.0f);
	TestFalse(TEXT("Battery should not be depleted at start."), BatteryComponent->IsBatteryDepleted());
	TestFalse(TEXT("Battery should not be low at start."), BatteryComponent->IsBatteryLow());
	TestFalse(TEXT("Battery should not be charging at start."), BatteryComponent->IsCharging());
	TestFalse(TEXT("Recording drain should not be active at start."), BatteryComponent->IsRecordingDraining());
	TestFalse(TEXT("Flashlight drain should not be active at start."), BatteryComponent->IsFlashlightDraining());

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCameraBatteryComponentRecordingDrainTest,
	"HorrorProject.Player.CameraBattery.RecordingDrain",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FCameraBatteryComponentRecordingDrainTest::RunTest(const FString& Parameters)
{
	UCameraBatteryComponent* BatteryComponent = NewObject<UCameraBatteryComponent>();
	BatteryComponent->SetBatteryPercentage(50.0f);

	float InitialPercentage = BatteryComponent->GetBatteryPercentage();

	BatteryComponent->StartRecordingDrain();
	TestTrue(TEXT("Recording drain should be active after start."), BatteryComponent->IsRecordingDraining());

	BatteryComponent->TickComponent(1.0f, ELevelTick::LEVELTICK_All, nullptr);

	TestTrue(TEXT("Battery should drain during recording."), BatteryComponent->GetBatteryPercentage() < InitialPercentage);

	BatteryComponent->StopRecordingDrain();
	TestFalse(TEXT("Recording drain should stop after stop call."), BatteryComponent->IsRecordingDraining());

	float StoppedPercentage = BatteryComponent->GetBatteryPercentage();
	BatteryComponent->TickComponent(1.0f, ELevelTick::LEVELTICK_All, nullptr);

	TestEqual(TEXT("Battery should not drain after stopping recording."), BatteryComponent->GetBatteryPercentage(), StoppedPercentage);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCameraBatteryComponentFlashlightDrainTest,
	"HorrorProject.Player.CameraBattery.FlashlightDrain",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FCameraBatteryComponentFlashlightDrainTest::RunTest(const FString& Parameters)
{
	UCameraBatteryComponent* BatteryComponent = NewObject<UCameraBatteryComponent>();
	BatteryComponent->SetBatteryPercentage(50.0f);

	float InitialPercentage = BatteryComponent->GetBatteryPercentage();

	BatteryComponent->StartFlashlightDrain();
	TestTrue(TEXT("Flashlight drain should be active after start."), BatteryComponent->IsFlashlightDraining());

	BatteryComponent->TickComponent(1.0f, ELevelTick::LEVELTICK_All, nullptr);

	TestTrue(TEXT("Battery should drain with flashlight on."), BatteryComponent->GetBatteryPercentage() < InitialPercentage);

	BatteryComponent->StopFlashlightDrain();
	TestFalse(TEXT("Flashlight drain should stop after stop call."), BatteryComponent->IsFlashlightDraining());

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCameraBatteryComponentCombinedDrainTest,
	"HorrorProject.Player.CameraBattery.CombinedDrain",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FCameraBatteryComponentCombinedDrainTest::RunTest(const FString& Parameters)
{
	UCameraBatteryComponent* BatteryComponent = NewObject<UCameraBatteryComponent>();
	BatteryComponent->SetBatteryPercentage(50.0f);

	BatteryComponent->StartRecordingDrain();
	float RecordingOnlyPercentage = BatteryComponent->GetBatteryPercentage();
	BatteryComponent->TickComponent(1.0f, ELevelTick::LEVELTICK_All, nullptr);
	float RecordingOnlyDrain = RecordingOnlyPercentage - BatteryComponent->GetBatteryPercentage();

	BatteryComponent->SetBatteryPercentage(50.0f);
	BatteryComponent->StopRecordingDrain();
	BatteryComponent->StartFlashlightDrain();
	float FlashlightOnlyPercentage = BatteryComponent->GetBatteryPercentage();
	BatteryComponent->TickComponent(1.0f, ELevelTick::LEVELTICK_All, nullptr);
	float FlashlightOnlyDrain = FlashlightOnlyPercentage - BatteryComponent->GetBatteryPercentage();

	BatteryComponent->SetBatteryPercentage(50.0f);
	BatteryComponent->StartRecordingDrain();
	BatteryComponent->StartFlashlightDrain();
	float CombinedPercentage = BatteryComponent->GetBatteryPercentage();
	BatteryComponent->TickComponent(1.0f, ELevelTick::LEVELTICK_All, nullptr);
	float CombinedDrain = CombinedPercentage - BatteryComponent->GetBatteryPercentage();

	TestTrue(TEXT("Combined drain should be greater than recording only."), CombinedDrain > RecordingOnlyDrain);
	TestTrue(TEXT("Combined drain should be greater than flashlight only."), CombinedDrain > FlashlightOnlyDrain);
	TestTrue(TEXT("Combined drain should approximately equal sum of individual drains."),
		FMath::IsNearlyEqual(CombinedDrain, RecordingOnlyDrain + FlashlightOnlyDrain, 0.1f));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCameraBatteryComponentChargingTest,
	"HorrorProject.Player.CameraBattery.Charging",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FCameraBatteryComponentChargingTest::RunTest(const FString& Parameters)
{
	UCameraBatteryComponent* BatteryComponent = NewObject<UCameraBatteryComponent>();
	BatteryComponent->SetBatteryPercentage(30.0f);

	float InitialPercentage = BatteryComponent->GetBatteryPercentage();

	BatteryComponent->StartCharging();
	TestTrue(TEXT("Battery should be charging after start."), BatteryComponent->IsCharging());

	BatteryComponent->TickComponent(1.0f, ELevelTick::LEVELTICK_All, nullptr);

	TestTrue(TEXT("Battery should increase during charging."), BatteryComponent->GetBatteryPercentage() > InitialPercentage);

	BatteryComponent->StopCharging();
	TestFalse(TEXT("Battery should not be charging after stop."), BatteryComponent->IsCharging());

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCameraBatteryComponentDepletionTest,
	"HorrorProject.Player.CameraBattery.Depletion",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FCameraBatteryComponentDepletionTest::RunTest(const FString& Parameters)
{
	UCameraBatteryComponent* BatteryComponent = NewObject<UCameraBatteryComponent>();
	BatteryComponent->SetBatteryPercentage(1.0f);

	TestFalse(TEXT("Battery should not be depleted at 1%."), BatteryComponent->IsBatteryDepleted());

	BatteryComponent->StartRecordingDrain();
	BatteryComponent->TickComponent(1.0f, ELevelTick::LEVELTICK_All, nullptr);

	TestTrue(TEXT("Battery should be depleted after draining to 0%."), BatteryComponent->IsBatteryDepleted());
	TestFalse(TEXT("Recording drain should stop when battery depletes."), BatteryComponent->IsRecordingDraining());

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCameraBatteryComponentLowWarningTest,
	"HorrorProject.Player.CameraBattery.LowWarning",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FCameraBatteryComponentLowWarningTest::RunTest(const FString& Parameters)
{
	UCameraBatteryComponent* BatteryComponent = NewObject<UCameraBatteryComponent>();
	BatteryComponent->SetBatteryPercentage(25.0f);

	TestFalse(TEXT("Battery should not be low at 25%."), BatteryComponent->IsBatteryLow());

	BatteryComponent->SetBatteryPercentage(20.0f);
	TestTrue(TEXT("Battery should be low at 20%."), BatteryComponent->IsBatteryLow());

	BatteryComponent->SetBatteryPercentage(10.0f);
	TestTrue(TEXT("Battery should still be low at 10%."), BatteryComponent->IsBatteryLow());

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCameraBatteryComponentFullChargeTest,
	"HorrorProject.Player.CameraBattery.FullCharge",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FCameraBatteryComponentFullChargeTest::RunTest(const FString& Parameters)
{
	UCameraBatteryComponent* BatteryComponent = NewObject<UCameraBatteryComponent>();
	BatteryComponent->SetBatteryPercentage(95.0f);

	BatteryComponent->StartCharging();
	TestTrue(TEXT("Battery should be charging."), BatteryComponent->IsCharging());

	BatteryComponent->TickComponent(1.0f, ELevelTick::LEVELTICK_All, nullptr);

	TestEqual(TEXT("Battery should cap at 100%."), BatteryComponent->GetBatteryPercentage(), 100.0f);
	TestFalse(TEXT("Charging should auto-stop at 100%."), BatteryComponent->IsCharging());

	return true;
}

#endif
