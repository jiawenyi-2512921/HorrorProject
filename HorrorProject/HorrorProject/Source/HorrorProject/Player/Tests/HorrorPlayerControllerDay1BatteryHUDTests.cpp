// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/HorrorPlayerController.h"

#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR

#include "GameFramework/PlayerState.h"
#include "Misc/AutomationTest.h"
#include "Player/HorrorPlayerCharacter.h"
#include "Player/Components/CameraBatteryComponent.h"
#include "Player/Components/QuantumCameraComponent.h"
#include "Tests/AutomationCommon.h"
#include "UI/Day1SliceHUD.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorPlayerControllerDay1BatteryHUDTest,
	"HorrorProject.Player.Controller.Day1BatteryHUD",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorPlayerControllerDay1BatteryHUDTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for Day1 battery HUD coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	AHorrorPlayerController* PlayerController = World->SpawnActor<AHorrorPlayerController>();
	ADay1SliceHUD* HUD = World->SpawnActor<ADay1SliceHUD>();
	AHorrorPlayerCharacter* PlayerCharacter = World->SpawnActor<AHorrorPlayerCharacter>();
	TestNotNull(TEXT("Battery HUD test should spawn a horror player controller."), PlayerController);
	TestNotNull(TEXT("Battery HUD test should attach the native Day1 HUD."), HUD);
	TestNotNull(TEXT("Battery HUD test should spawn a horror player character."), PlayerCharacter);
	if (!PlayerController || !HUD || !PlayerCharacter)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	PlayerController->PlayerState = World->SpawnActor<APlayerState>();
	World->AddController(PlayerController);
	PlayerController->MyHUD = HUD;
	PlayerController->Possess(PlayerCharacter);

	UQuantumCameraComponent* QuantumCamera = PlayerCharacter->GetQuantumCameraComponent();
	TestNotNull(TEXT("Battery HUD test should expose a quantum camera component."), QuantumCamera);
	UCameraBatteryComponent* Battery = QuantumCamera ? QuantumCamera->GetBatteryComponent() : nullptr;
	TestNotNull(TEXT("Quantum camera should provide a battery component for the native Day1 HUD."), Battery);
	if (!QuantumCamera || !Battery)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	Battery->SetBatteryPercentage(17.0f);
	QuantumCamera->SetCameraAcquired(true);
	QuantumCamera->SetCameraEnabled(true);

	PlayerController->RefreshDay1HUDStateForTests();
	TestTrue(TEXT("Day1 HUD should show battery status when the camera has a battery."), HUD->IsBodycamBatteryVisibleForTests());
	TestEqual(TEXT("Day1 HUD should receive the camera battery percentage."), HUD->GetBodycamBatteryPercentForTests(), 17.0f);
	TestTrue(TEXT("Day1 HUD should flag low battery using the battery component threshold."), HUD->IsBodycamBatteryLowForTests());

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorPlayerControllerDay1RealtimeHUDBypassesFullRefreshThrottleTest,
	"HorrorProject.Player.Controller.Day1RealtimeHUDBypassesFullRefreshThrottle",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorPlayerControllerDay1RealtimeHUDBypassesFullRefreshThrottleTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for Day1 realtime HUD refresh coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	AHorrorPlayerController* PlayerController = World->SpawnActor<AHorrorPlayerController>();
	ADay1SliceHUD* HUD = World->SpawnActor<ADay1SliceHUD>();
	AHorrorPlayerCharacter* PlayerCharacter = World->SpawnActor<AHorrorPlayerCharacter>();
	TestNotNull(TEXT("Realtime HUD test should spawn a horror player controller."), PlayerController);
	TestNotNull(TEXT("Realtime HUD test should attach the native Day1 HUD."), HUD);
	TestNotNull(TEXT("Realtime HUD test should spawn a horror player character."), PlayerCharacter);
	if (!PlayerController || !HUD || !PlayerCharacter)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	PlayerController->PlayerState = World->SpawnActor<APlayerState>();
	World->AddController(PlayerController);
	PlayerController->MyHUD = HUD;
	PlayerController->Possess(PlayerCharacter);

	UQuantumCameraComponent* QuantumCamera = PlayerCharacter->GetQuantumCameraComponent();
	UCameraBatteryComponent* Battery = QuantumCamera ? QuantumCamera->GetBatteryComponent() : nullptr;
	TestNotNull(TEXT("Realtime HUD test should expose a quantum camera component."), QuantumCamera);
	TestNotNull(TEXT("Realtime HUD test should expose a camera battery component."), Battery);
	if (!QuantumCamera || !Battery)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	QuantumCamera->SetCameraAcquired(true);
	QuantumCamera->SetCameraEnabled(true);
	Battery->SetBatteryPercentage(82.0f);
	PlayerController->RefreshDay1HUDStateForTests();
	TestEqual(TEXT("Initial full refresh should publish the starting battery value."), HUD->GetBodycamBatteryPercentForTests(), 82.0f);

	Battery->SetBatteryPercentage(41.0f);
	TestFalse(TEXT("Short runtime update should not require a full HUD refresh to run."), PlayerController->UpdateDay1RuntimeStateForTests(0.01f));
	TestEqual(TEXT("Realtime HUD should update battery percentage even while full refresh is throttled."), HUD->GetBodycamBatteryPercentForTests(), 41.0f);

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

#endif
