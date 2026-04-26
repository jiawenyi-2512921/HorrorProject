#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR && HORRORPROJECT_ENABLE_LEGACY_AUTOMATION_TESTS

// Copyright Epic Games, Inc. All Rights Reserved.

#include "UIWidgetTests.h"
#include "UI/Widgets/BodycamOverlayWidget.h"
#include "UI/Widgets/EvidenceToastWidget.h"
#include "UI/Widgets/ObjectiveToastWidget.h"
#include "UI/Components/VHSVisualEffectComponent.h"
#include "UI/Components/NoiseOverlayComponent.h"
#include "UI/Components/ScanlineComponent.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FBodycamOverlayWidgetTest,
	"HorrorProject.UI.Widgets.BodycamOverlay",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FBodycamOverlayWidgetTest::RunTest(const FString& Parameters)
{
	UBodycamOverlayWidget* Widget = NewObject<UBodycamOverlayWidget>();
	TestNotNull(TEXT("Bodycam overlay widget should be created"), Widget);

	Widget->SetRecordingState(true);
	TestTrue(TEXT("Recording state should be set"), Widget->IsRecording());

	Widget->SetBatteryLevel(0.5f);
	TestEqual(TEXT("Battery level should be 0.5"), Widget->GetBatteryLevel(), 0.5f);

	Widget->SetBatteryLevel(1.5f);
	TestEqual(TEXT("Battery level should clamp to 1.0"), Widget->GetBatteryLevel(), 1.0f);

	Widget->SetBatteryLevel(-0.5f);
	TestEqual(TEXT("Battery level should clamp to 0.0"), Widget->GetBatteryLevel(), 0.0f);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FEvidenceToastWidgetTest,
	"HorrorProject.UI.Widgets.EvidenceToast",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FEvidenceToastWidgetTest::RunTest(const FString& Parameters)
{
	UEvidenceToastWidget* Widget = NewObject<UEvidenceToastWidget>();
	TestNotNull(TEXT("Evidence toast widget should be created"), Widget);

	TestFalse(TEXT("Toast should not be visible initially"), Widget->IsToastVisible());

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FObjectiveToastWidgetTest,
	"HorrorProject.UI.Widgets.ObjectiveToast",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FObjectiveToastWidgetTest::RunTest(const FString& Parameters)
{
	UObjectiveToastWidget* Widget = NewObject<UObjectiveToastWidget>();
	TestNotNull(TEXT("Objective toast widget should be created"), Widget);

	TestFalse(TEXT("Toast should not be visible initially"), Widget->IsToastVisible());

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FVHSEffectComponentTest,
	"HorrorProject.UI.Components.VHSEffect",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FVHSEffectComponentTest::RunTest(const FString& Parameters)
{
	UVHSVisualEffectComponent* Component = NewObject<UVHSVisualEffectComponent>();
	TestNotNull(TEXT("VHS effect component should be created"), Component);

	TestFalse(TEXT("Effect should be disabled initially"), Component->IsEffectEnabled());

	Component->EnableEffect(true);
	TestTrue(TEXT("Effect should be enabled"), Component->IsEffectEnabled());

	Component->SetEffectIntensity(0.5f);
	TestEqual(TEXT("Effect intensity should be 0.5"), Component->GetEffectIntensity(), 0.5f);

	Component->SetEffectIntensity(1.5f);
	TestEqual(TEXT("Effect intensity should clamp to 1.0"), Component->GetEffectIntensity(), 1.0f);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FNoiseOverlayComponentTest,
	"HorrorProject.UI.Components.NoiseOverlay",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FNoiseOverlayComponentTest::RunTest(const FString& Parameters)
{
	UNoiseOverlayComponent* Component = NewObject<UNoiseOverlayComponent>();
	TestNotNull(TEXT("Noise overlay component should be created"), Component);

	Component->SetNoiseIntensity(0.5f);
	TestEqual(TEXT("Noise intensity should be 0.5"), Component->GetNoiseIntensity(), 0.5f);

	Component->EnableNoise(false);
	TestEqual(TEXT("Noise value should be 0 when disabled"), Component->GetNoiseValue(), 0.0f);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FScanlineComponentTest,
	"HorrorProject.UI.Components.Scanline",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FScanlineComponentTest::RunTest(const FString& Parameters)
{
	UScanlineComponent* Component = NewObject<UScanlineComponent>();
	TestNotNull(TEXT("Scanline component should be created"), Component);

	Component->SetScanlineIntensity(0.5f);
	TestEqual(TEXT("Scanline intensity should be 0.5"), Component->GetScanlineIntensity(), 0.5f);

	Component->EnableScanlines(false);
	TestEqual(TEXT("Scanline offset should be 0 when disabled"), Component->GetScanlineOffset(), 0.0f);

	return true;
}

#endif

#endif // WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR
