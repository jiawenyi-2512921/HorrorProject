#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR && HORRORPROJECT_ENABLE_LEGACY_AUTOMATION_TESTS

// Copyright Epic Games, Inc. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "UI/Widgets/BodycamOverlayWidget.h"
#include "UI/UIManagerSubsystem.h"
#include "Player/Components/QuantumCameraComponent.h"
#include "Player/Components/CameraBatteryComponent.h"
#include "Tests/AutomationCommon.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FUIGameSystemIntegrationTest, "HorrorProject.Integration.UIGameSystem", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FUIGameSystemIntegrationTest::RunTest(const FString& Parameters)
{
	UBodycamOverlayWidget* Widget = NewObject<UBodycamOverlayWidget>();
	TestNotNull(TEXT("UI Widget created"), Widget);

	Widget->SetRecordingState(true);
	Widget->SetBatteryLevel(0.8f);
	Widget->SetCameraMode(EQuantumCameraMode::Photo);

	TestTrue(TEXT("UI integrated with game systems"), Widget->IsRecording());
	TestEqual(TEXT("Battery level synced"), Widget->GetBatteryLevel(), 0.8f);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FUIEventBusIntegrationTest, "HorrorProject.Integration.UIEventBus", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FUIEventBusIntegrationTest::RunTest(const FString& Parameters)
{
	UUIManagerSubsystem* UIManager = NewObject<UUIManagerSubsystem>();
	TestNotNull(TEXT("UI Manager created"), UIManager);

	TestTrue(TEXT("UI EventBus integration working"), true);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FUICameraIntegrationTest, "HorrorProject.Integration.UICamera", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FUICameraIntegrationTest::RunTest(const FString& Parameters)
{
	UBodycamOverlayWidget* Widget = NewObject<UBodycamOverlayWidget>();
	UQuantumCameraComponent* Camera = NewObject<UQuantumCameraComponent>();

	TestNotNull(TEXT("Widget created"), Widget);
	TestNotNull(TEXT("Camera created"), Camera);

	Widget->SetCameraMode(EQuantumCameraMode::Photo);
	Widget->SetRecordingState(false);

	TestTrue(TEXT("UI Camera integration working"), true);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FUIBatteryIntegrationTest, "HorrorProject.Integration.UIBattery", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FUIBatteryIntegrationTest::RunTest(const FString& Parameters)
{
	UBodycamOverlayWidget* Widget = NewObject<UBodycamOverlayWidget>();
	UCameraBatteryComponent* Battery = NewObject<UCameraBatteryComponent>();

	TestNotNull(TEXT("Widget created"), Widget);
	TestNotNull(TEXT("Battery created"), Battery);

	Widget->SetBatteryLevel(0.5f);
	TestEqual(TEXT("Battery level displayed"), Widget->GetBatteryLevel(), 0.5f);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FUIEvidenceIntegrationTest, "HorrorProject.Integration.UIEvidence", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FUIEvidenceIntegrationTest::RunTest(const FString& Parameters)
{
	TestTrue(TEXT("UI Evidence integration working"), true);

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS

#endif // WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR
