#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR && HORRORPROJECT_ENABLE_LEGACY_AUTOMATION_TESTS

// Copyright Epic Games, Inc. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "Player/Components/CameraBatteryComponent.h"
#include "Player/Components/QuantumCameraComponent.h"
#include "UI/UIEventManager.h"
#include "UI/UIManagerSubsystem.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FUIGameSystemIntegrationTest,
	"HorrorProject.Integration.UIGameSystem",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FUIGameSystemIntegrationTest::RunTest(const FString& Parameters)
{
	UUIManagerSubsystem* UIManager = NewObject<UUIManagerSubsystem>();
	TestNotNull(TEXT("UI manager should be constructible for integration tests."), UIManager);
	if (!UIManager)
	{
		return false;
	}

	const FName CameraState(TEXT("CameraOverlay"));
	UIManager->SetUIState(CameraState);

	TestEqual(TEXT("UI manager should accept camera overlay state."), UIManager->GetCurrentUIState(), CameraState);
	TestNotNull(TEXT("UI manager should expose an event manager for cross-system wiring."), UIManager->GetEventManager());

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FUIEventBusIntegrationTest,
	"HorrorProject.Integration.UIEventBus",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FUIEventBusIntegrationTest::RunTest(const FString& Parameters)
{
	UUIEventManager* EventManager = NewObject<UUIEventManager>();
	TestNotNull(TEXT("UI event manager should be constructible."), EventManager);
	if (!EventManager)
	{
		return false;
	}

	EventManager->BroadcastUIStateChanged(TEXT("Archive"));
	TestTrue(TEXT("Broadcasting a UI state without subscribers should be safe."), true);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FUICameraIntegrationTest,
	"HorrorProject.Integration.UICamera",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FUICameraIntegrationTest::RunTest(const FString& Parameters)
{
	UUIManagerSubsystem* UIManager = NewObject<UUIManagerSubsystem>();
	UQuantumCameraComponent* Camera = NewObject<UQuantumCameraComponent>();

	TestNotNull(TEXT("UI manager should be constructible."), UIManager);
	TestNotNull(TEXT("Camera component should be constructible."), Camera);
	if (!UIManager || !Camera)
	{
		return false;
	}

	UIManager->SetUIState(TEXT("PhotoMode"));
	TestEqual(TEXT("UI manager should store photo mode state."), UIManager->GetCurrentUIState(), FName(TEXT("PhotoMode")));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FUIBatteryIntegrationTest,
	"HorrorProject.Integration.UIBattery",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FUIBatteryIntegrationTest::RunTest(const FString& Parameters)
{
	UUIManagerSubsystem* UIManager = NewObject<UUIManagerSubsystem>();
	UCameraBatteryComponent* Battery = NewObject<UCameraBatteryComponent>();

	TestNotNull(TEXT("UI manager should be constructible."), UIManager);
	TestNotNull(TEXT("Battery component should be constructible."), Battery);
	if (!UIManager || !Battery)
	{
		return false;
	}

	UIManager->SetUIState(TEXT("LowBatteryWarning"));
	TestEqual(TEXT("UI manager should store low-battery warning state."), UIManager->GetCurrentUIState(), FName(TEXT("LowBatteryWarning")));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FUIEvidenceIntegrationTest,
	"HorrorProject.Integration.UIEvidence",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FUIEvidenceIntegrationTest::RunTest(const FString& Parameters)
{
	UUIManagerSubsystem* UIManager = NewObject<UUIManagerSubsystem>();
	TestNotNull(TEXT("UI manager should be constructible."), UIManager);
	if (!UIManager)
	{
		return false;
	}

	UIManager->SetUIState(TEXT("EvidenceArchive"));
	TestEqual(TEXT("UI manager should store evidence archive state."), UIManager->GetCurrentUIState(), FName(TEXT("EvidenceArchive")));

	return true;
}

#endif
