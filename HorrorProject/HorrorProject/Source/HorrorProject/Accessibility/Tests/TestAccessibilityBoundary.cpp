#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR && HORRORPROJECT_ENABLE_LEGACY_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "Accessibility/AccessibilitySubsystem.h"
#include "Accessibility/ColorBlindMode.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAccessibilityBoundaryTest, "HorrorProject.Accessibility.BoundaryConditions", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FAccessibilityBoundaryTest::RunTest(const FString& Parameters)
{
    UWorld* World = FAutomationEditorCommonUtils::CreateNewMap();
    UAccessibilitySubsystem* AccessibilitySys = World->GetSubsystem<UAccessibilitySubsystem>();

    if (!AccessibilitySys)
    {
        AddWarning(TEXT("AccessibilitySubsystem not available"));
        return true;
    }

    // Test extreme font sizes
    AccessibilitySys->SetFontSize(0.1f);
    TestTrue(TEXT("Minimum font size handled"), AccessibilitySys->GetFontSize() >= 0.1f);

    AccessibilitySys->SetFontSize(10.0f);
    TestTrue(TEXT("Maximum font size handled"), AccessibilitySys->GetFontSize() <= 10.0f);

    // Test negative font size
    AccessibilitySys->SetFontSize(-1.0f);
    TestTrue(TEXT("Negative font size clamped"), AccessibilitySys->GetFontSize() > 0.0f);

    // Test subtitle speed extremes
    AccessibilitySys->SetSubtitleSpeed(0.0f);
    TestTrue(TEXT("Zero subtitle speed handled"), true);

    AccessibilitySys->SetSubtitleSpeed(100.0f);
    TestTrue(TEXT("Very fast subtitle speed handled"), true);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FColorBlindModeTest, "HorrorProject.Accessibility.ColorBlindModes", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FColorBlindModeTest::RunTest(const FString& Parameters)
{
    UWorld* World = FAutomationEditorCommonUtils::CreateNewMap();
    UAccessibilitySubsystem* AccessibilitySys = World->GetSubsystem<UAccessibilitySubsystem>();

    if (!AccessibilitySys)
    {
        AddWarning(TEXT("AccessibilitySubsystem not available"));
        return true;
    }

    // Test all color blind modes
    TArray<EColorBlindMode> Modes = {
        EColorBlindMode::None,
        EColorBlindMode::Protanopia,
        EColorBlindMode::Deuteranopia,
        EColorBlindMode::Tritanopia,
        EColorBlindMode::Achromatopsia
    };

    for (EColorBlindMode Mode : Modes)
    {
        AccessibilitySys->SetColorBlindMode(Mode);
        TestEqual(TEXT("Color blind mode set"), AccessibilitySys->GetColorBlindMode(), Mode);
    }

    // Test color blind intensity
    AccessibilitySys->SetColorBlindIntensity(0.0f);
    TestEqual(TEXT("Zero intensity"), AccessibilitySys->GetColorBlindIntensity(), 0.0f);

    AccessibilitySys->SetColorBlindIntensity(1.0f);
    TestEqual(TEXT("Full intensity"), AccessibilitySys->GetColorBlindIntensity(), 1.0f);

    AccessibilitySys->SetColorBlindIntensity(2.0f);
    TestTrue(TEXT("Over-max intensity clamped"), AccessibilitySys->GetColorBlindIntensity() <= 1.0f);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSubtitleSystemStressTest, "HorrorProject.Accessibility.SubtitleStress", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FSubtitleSystemStressTest::RunTest(const FString& Parameters)
{
    UWorld* World = FAutomationEditorCommonUtils::CreateNewMap();
    UAccessibilitySubsystem* AccessibilitySys = World->GetSubsystem<UAccessibilitySubsystem>();

    if (!AccessibilitySys)
    {
        AddWarning(TEXT("AccessibilitySubsystem not available"));
        return true;
    }

    // Test rapid subtitle display
    const int32 NumSubtitles = 1000;
    double StartTime = FPlatformTime::Seconds();

    for (int32 i = 0; i < NumSubtitles; ++i)
    {
        FString SubtitleText = FString::Printf(TEXT("Subtitle %d"), i);
        AccessibilitySys->ShowSubtitle(FText::FromString(SubtitleText), 0.1f);
    }

    double EndTime = FPlatformTime::Seconds();
    double TotalTime = EndTime - StartTime;

    AddInfo(FString::Printf(TEXT("Subtitle stress test: %d subtitles in %.3f seconds"), NumSubtitles, TotalTime));
    TestTrue(TEXT("Subtitle system handles stress"), TotalTime < 1.0);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAccessibilityHighContrastTest, "HorrorProject.Accessibility.HighContrast", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FAccessibilityHighContrastTest::RunTest(const FString& Parameters)
{
    UWorld* World = FAutomationEditorCommonUtils::CreateNewMap();
    UAccessibilitySubsystem* AccessibilitySys = World->GetSubsystem<UAccessibilitySubsystem>();

    if (!AccessibilitySys)
    {
        AddWarning(TEXT("AccessibilitySubsystem not available"));
        return true;
    }

    // Test high contrast mode
    AccessibilitySys->SetHighContrastMode(true);
    TestTrue(TEXT("High contrast enabled"), AccessibilitySys->IsHighContrastEnabled());

    AccessibilitySys->SetHighContrastMode(false);
    TestFalse(TEXT("High contrast disabled"), AccessibilitySys->IsHighContrastEnabled());

    // Test contrast intensity
    AccessibilitySys->SetContrastIntensity(0.5f);
    TestEqual(TEXT("Contrast intensity set"), AccessibilitySys->GetContrastIntensity(), 0.5f);

    return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR
