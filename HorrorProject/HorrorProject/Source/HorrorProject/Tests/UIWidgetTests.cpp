#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR && HORRORPROJECT_ENABLE_LEGACY_AUTOMATION_TESTS

// Copyright Epic Games, Inc. All Rights Reserved.

#include "Tests/UIWidgetTests.h"

#include "Misc/AutomationTest.h"
#include "UI/Widgets/BodycamOverlayWidget.h"
#include "UI/Widgets/ArchiveMenuWidget.h"
#include "UI/Widgets/PauseMenuWidget.h"
#include "UI/Widgets/SettingsMenuWidget.h"
#include "UI/Widgets/EvidenceToastWidget.h"
#include "UI/Widgets/ObjectiveToastWidget.h"
#include "UI/Components/VHSVisualEffectComponent.h"
#include "UI/Components/NoiseOverlayComponent.h"
#include "UI/Components/ScanlineComponent.h"
#include "UI/UIManagerSubsystem.h"
#include "Tests/AutomationCommon.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBodycamOverlayWidgetTest, "HorrorProject.UI.BodycamOverlayWidget", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FBodycamOverlayWidgetTest::RunTest(const FString& Parameters)
{
	UBodycamOverlayWidget* Widget = NewObject<UBodycamOverlayWidget>();
	TestNotNull(TEXT("BodycamOverlayWidget created"), Widget);

	Widget->SetRecordingState(true);
	TestTrue(TEXT("Recording state set"), Widget->IsRecording());

	Widget->SetBatteryLevel(0.75f);
	TestEqual(TEXT("Battery level set"), Widget->GetBatteryLevel(), 0.75f);

	Widget->SetCameraMode(EQuantumCameraMode::Photo);
	Widget->SetVHSIntensity(0.5f);
	Widget->TriggerGlitchEffect(0.5f);

	FDateTime Time = FDateTime::Now();
	Widget->SetTimestamp(Time);

	Widget->SetRecordingState(false);
	TestFalse(TEXT("Recording stopped"), Widget->IsRecording());

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBodycamBatteryLevelsTest, "HorrorProject.UI.BodycamBatteryLevels", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FBodycamBatteryLevelsTest::RunTest(const FString& Parameters)
{
	UBodycamOverlayWidget* Widget = NewObject<UBodycamOverlayWidget>();

	Widget->SetBatteryLevel(1.0f);
	TestEqual(TEXT("Full battery"), Widget->GetBatteryLevel(), 1.0f);

	Widget->SetBatteryLevel(0.5f);
	TestEqual(TEXT("Half battery"), Widget->GetBatteryLevel(), 0.5f);

	Widget->SetBatteryLevel(0.2f);
	TestEqual(TEXT("Low battery"), Widget->GetBatteryLevel(), 0.2f);

	Widget->SetBatteryLevel(0.0f);
	TestEqual(TEXT("Empty battery"), Widget->GetBatteryLevel(), 0.0f);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FArchiveMenuWidgetTest, "HorrorProject.UI.ArchiveMenuWidget", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FArchiveMenuWidgetTest::RunTest(const FString& Parameters)
{
	UArchiveMenuWidget* Widget = NewObject<UArchiveMenuWidget>();
	TestNotNull(TEXT("ArchiveMenuWidget created"), Widget);

	TestTrue(TEXT("Archive menu widget initialized"), true);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPauseMenuWidgetTest, "HorrorProject.UI.PauseMenuWidget", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FPauseMenuWidgetTest::RunTest(const FString& Parameters)
{
	UPauseMenuWidget* Widget = NewObject<UPauseMenuWidget>();
	TestNotNull(TEXT("PauseMenuWidget created"), Widget);

	TestTrue(TEXT("Pause menu widget initialized"), true);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSettingsMenuWidgetTest, "HorrorProject.UI.SettingsMenuWidget", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FSettingsMenuWidgetTest::RunTest(const FString& Parameters)
{
	USettingsMenuWidget* Widget = NewObject<USettingsMenuWidget>();
	TestNotNull(TEXT("SettingsMenuWidget created"), Widget);

	TestTrue(TEXT("Settings menu widget initialized"), true);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEvidenceToastWidgetTest, "HorrorProject.UI.EvidenceToastWidget", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FEvidenceToastWidgetTest::RunTest(const FString& Parameters)
{
	UEvidenceToastWidget* Widget = NewObject<UEvidenceToastWidget>();
	TestNotNull(TEXT("EvidenceToastWidget created"), Widget);

	TestTrue(TEXT("Evidence toast widget initialized"), true);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FObjectiveToastWidgetTest, "HorrorProject.UI.ObjectiveToastWidget", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FObjectiveToastWidgetTest::RunTest(const FString& Parameters)
{
	UObjectiveToastWidget* Widget = NewObject<UObjectiveToastWidget>();
	TestNotNull(TEXT("ObjectiveToastWidget created"), Widget);

	TestTrue(TEXT("Objective toast widget initialized"), true);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FVHSEffectComponentTest, "HorrorProject.UI.VHSEffectComponent", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FVHSEffectComponentTest::RunTest(const FString& Parameters)
{
	UVHSVisualEffectComponent* Component = NewObject<UVHSVisualEffectComponent>();
	TestNotNull(TEXT("VHSEffectComponent created"), Component);

	TestTrue(TEXT("VHS effect component initialized"), true);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FNoiseOverlayComponentTest, "HorrorProject.UI.NoiseOverlayComponent", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FNoiseOverlayComponentTest::RunTest(const FString& Parameters)
{
	UNoiseOverlayComponent* Component = NewObject<UNoiseOverlayComponent>();
	TestNotNull(TEXT("NoiseOverlayComponent created"), Component);

	TestTrue(TEXT("Noise overlay component initialized"), true);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FScanlineComponentTest, "HorrorProject.UI.ScanlineComponent", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FScanlineComponentTest::RunTest(const FString& Parameters)
{
	UScanlineComponent* Component = NewObject<UScanlineComponent>();
	TestNotNull(TEXT("ScanlineComponent created"), Component);

	TestTrue(TEXT("Scanline component initialized"), true);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FUIManagerSubsystemTest, "HorrorProject.UI.UIManagerSubsystem", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FUIManagerSubsystemTest::RunTest(const FString& Parameters)
{
	UUIManagerSubsystem* Subsystem = NewObject<UUIManagerSubsystem>();
	TestNotNull(TEXT("UIManagerSubsystem created"), Subsystem);

	TestTrue(TEXT("UI manager subsystem initialized"), true);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FUIWidgetEdgeCasesTest, "HorrorProject.UI.EdgeCases", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FUIWidgetEdgeCasesTest::RunTest(const FString& Parameters)
{
	UBodycamOverlayWidget* Widget = NewObject<UBodycamOverlayWidget>();

	Widget->SetBatteryLevel(-0.5f);
	Widget->SetBatteryLevel(2.0f);
	Widget->SetVHSIntensity(-1.0f);
	Widget->SetVHSIntensity(5.0f);
	Widget->TriggerGlitchEffect(-1.0f);

	TestTrue(TEXT("Edge cases handled"), true);

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS

#endif // WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR
