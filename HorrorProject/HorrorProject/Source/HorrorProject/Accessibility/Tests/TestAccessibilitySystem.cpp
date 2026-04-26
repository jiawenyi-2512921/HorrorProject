#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR && HORRORPROJECT_ENABLE_LEGACY_AUTOMATION_TESTS

// Copyright Epic Games, Inc. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "../AccessibilitySubsystem.h"
#include "Tests/AutomationCommon.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAccessibilitySubsystemTest, "HorrorProject.Accessibility.SubsystemTest", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FAccessibilitySubsystemTest::RunTest(const FString& Parameters)
{
    // Test accessibility subsystem initialization
    UAccessibilitySubsystem* Subsystem = NewObject<UAccessibilitySubsystem>();
    TestNotNull(TEXT("Subsystem created"), Subsystem);

    // Test default settings
    FAccessibilitySettings DefaultSettings;
    TestTrue(TEXT("Subtitles enabled by default"), DefaultSettings.bSubtitlesEnabled);
    TestEqual(TEXT("Default subtitle size is Medium"), DefaultSettings.SubtitleSize, ESubtitleSize::Medium);
    TestEqual(TEXT("Default color blind mode is None"), DefaultSettings.ColorBlindMode, EColorBlindMode::None);

    // Test settings application
    FAccessibilitySettings NewSettings;
    NewSettings.bSubtitlesEnabled = false;
    NewSettings.SubtitleSize = ESubtitleSize::Large;
    NewSettings.ColorBlindMode = EColorBlindMode::Protanopia;

    Subsystem->ApplyAccessibilitySettings(NewSettings);
    FAccessibilitySettings AppliedSettings = Subsystem->GetAccessibilitySettings();

    TestFalse(TEXT("Subtitles disabled"), AppliedSettings.bSubtitlesEnabled);
    TestEqual(TEXT("Subtitle size changed to Large"), AppliedSettings.SubtitleSize, ESubtitleSize::Large);
    TestEqual(TEXT("Color blind mode changed to Protanopia"), AppliedSettings.ColorBlindMode, EColorBlindMode::Protanopia);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAccessibilityWCAGComplianceTest, "HorrorProject.Accessibility.WCAGCompliance", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FAccessibilityWCAGComplianceTest::RunTest(const FString& Parameters)
{
    UAccessibilitySubsystem* Subsystem = NewObject<UAccessibilitySubsystem>();

    // Test contrast ratio compliance
    FAccessibilitySettings Settings;
    Settings.SubtitleColor = FLinearColor::White;
    Settings.SubtitleBackgroundColor = FLinearColor::Black;

    Subsystem->ApplyAccessibilitySettings(Settings);
    TestTrue(TEXT("WCAG compliance with high contrast"), Subsystem->ValidateWCAGCompliance());

    // Test low contrast (should fail)
    Settings.SubtitleColor = FLinearColor(0.6f, 0.6f, 0.6f);
    Settings.SubtitleBackgroundColor = FLinearColor(0.5f, 0.5f, 0.5f);
    Subsystem->ApplyAccessibilitySettings(Settings);
    TestFalse(TEXT("WCAG compliance fails with low contrast"), Subsystem->ValidateWCAGCompliance());

    // Test UI scale bounds
    Settings.UIScale = 0.3f; // Too small
    Subsystem->ApplyAccessibilitySettings(Settings);
    TestFalse(TEXT("WCAG compliance fails with invalid UI scale"), Subsystem->ValidateWCAGCompliance());

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAccessibilityColorBlindTest, "HorrorProject.Accessibility.ColorBlindMode", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FAccessibilityColorBlindTest::RunTest(const FString& Parameters)
{
    UAccessibilitySubsystem* Subsystem = NewObject<UAccessibilitySubsystem>();

    // Test color adjustment for protanopia
    Subsystem->SetColorBlindMode(EColorBlindMode::Protanopia);
    FLinearColor OriginalColor = FLinearColor::Red;
    FLinearColor AdjustedColor = Subsystem->AdjustColorForColorBlindness(OriginalColor);

    TestNotEqual(TEXT("Color adjusted for protanopia"), AdjustedColor, OriginalColor);

    // Test no adjustment when mode is None
    Subsystem->SetColorBlindMode(EColorBlindMode::None);
    FLinearColor NoAdjustment = Subsystem->AdjustColorForColorBlindness(OriginalColor);
    TestEqual(TEXT("No color adjustment when mode is None"), NoAdjustment, OriginalColor);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAccessibilityMotionSettingsTest, "HorrorProject.Accessibility.MotionSettings", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FAccessibilityMotionSettingsTest::RunTest(const FString& Parameters)
{
    UAccessibilitySubsystem* Subsystem = NewObject<UAccessibilitySubsystem>();

    // Test motion blur toggle
    Subsystem->SetMotionBlurEnabled(false);
    FAccessibilitySettings Settings = Subsystem->GetAccessibilitySettings();
    TestFalse(TEXT("Motion blur disabled"), Settings.bMotionBlurEnabled);

    // Test camera shake intensity
    Subsystem->SetCameraShakeIntensity(0.5f);
    TestEqual(TEXT("Camera shake intensity set to 0.5"), Subsystem->GetAdjustedCameraShakeIntensity(), 0.5f);

    // Test camera shake clamping
    Subsystem->SetCameraShakeIntensity(2.0f); // Should clamp to 1.0
    TestEqual(TEXT("Camera shake intensity clamped to 1.0"), Subsystem->GetAdjustedCameraShakeIntensity(), 1.0f);

    Subsystem->SetCameraShakeIntensity(-0.5f); // Should clamp to 0.0
    TestEqual(TEXT("Camera shake intensity clamped to 0.0"), Subsystem->GetAdjustedCameraShakeIntensity(), 0.0f);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAccessibilityUIScaleTest, "HorrorProject.Accessibility.UIScale", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FAccessibilityUIScaleTest::RunTest(const FString& Parameters)
{
    UAccessibilitySubsystem* Subsystem = NewObject<UAccessibilitySubsystem>();

    // Test UI scale setting
    Subsystem->SetUIScale(1.5f);
    TestEqual(TEXT("UI scale set to 1.5"), Subsystem->GetUIScale(), 1.5f);

    // Test UI scale clamping
    Subsystem->SetUIScale(3.0f); // Should clamp to 2.0
    TestEqual(TEXT("UI scale clamped to 2.0"), Subsystem->GetUIScale(), 2.0f);

    Subsystem->SetUIScale(0.2f); // Should clamp to 0.5
    TestEqual(TEXT("UI scale clamped to 0.5"), Subsystem->GetUIScale(), 0.5f);

    return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR
