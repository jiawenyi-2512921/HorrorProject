#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR && HORRORPROJECT_ENABLE_LEGACY_AUTOMATION_TESTS

// Copyright Epic Games, Inc. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "../ColorBlindMode.h"
#include "Tests/AutomationCommon.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FColorBlindTransformTest, "HorrorProject.Accessibility.ColorBlindTransform", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FColorBlindTransformTest::RunTest(const FString& Parameters)
{
    // Test Protanopia transformation
    FLinearColor RedColor = FLinearColor::Red;
    FLinearColor ProtanopiaRed = AColorBlindMode::TransformColorProtanopia(RedColor, 1.0f);
    TestNotEqual(TEXT("Protanopia transforms red"), ProtanopiaRed, RedColor);

    // Test Deuteranopia transformation
    FLinearColor GreenColor = FLinearColor::Green;
    FLinearColor DeuteranopiaGreen = AColorBlindMode::TransformColorDeuteranopia(GreenColor, 1.0f);
    TestNotEqual(TEXT("Deuteranopia transforms green"), DeuteranopiaGreen, GreenColor);

    // Test Tritanopia transformation
    FLinearColor BlueColor = FLinearColor::Blue;
    FLinearColor TritanopiaBlue = AColorBlindMode::TransformColorTritanopia(BlueColor, 1.0f);
    TestNotEqual(TEXT("Tritanopia transforms blue"), TritanopiaBlue, BlueColor);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FColorBlindSeverityTest, "HorrorProject.Accessibility.ColorBlindSeverity", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FColorBlindSeverityTest::RunTest(const FString& Parameters)
{
    FLinearColor OriginalColor = FLinearColor::Red;

    // Test severity 0 (no transformation)
    FLinearColor NoTransform = AColorBlindMode::TransformColorProtanopia(OriginalColor, 0.0f);
    TestEqual(TEXT("Severity 0 produces no transformation"), NoTransform, OriginalColor);

    // Test severity 0.5 (partial transformation)
    FLinearColor PartialTransform = AColorBlindMode::TransformColorProtanopia(OriginalColor, 0.5f);
    FLinearColor FullTransform = AColorBlindMode::TransformColorProtanopia(OriginalColor, 1.0f);

    TestNotEqual(TEXT("Partial transformation differs from original"), PartialTransform, OriginalColor);
    TestNotEqual(TEXT("Partial transformation differs from full"), PartialTransform, FullTransform);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FColorDistinguishabilityTest, "HorrorProject.Accessibility.ColorDistinguishability", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FColorDistinguishabilityTest::RunTest(const FString& Parameters)
{
    // Test distinguishable colors
    FLinearColor Black = FLinearColor::Black;
    FLinearColor White = FLinearColor::White;

    TestTrue(TEXT("Black and white are distinguishable for protanopia"),
        AColorBlindMode::IsColorDistinguishable(Black, White, 1));

    // Test similar colors
    FLinearColor Red = FLinearColor::Red;
    FLinearColor DarkRed = FLinearColor(0.5f, 0.0f, 0.0f);

    bool AreSimilarColorsDistinguishable = AColorBlindMode::IsColorDistinguishable(Red, DarkRed, 1);
    AddInfo(FString::Printf(TEXT("Similar reds distinguishability: %s"),
        AreSimilarColorsDistinguishable ? TEXT("true") : TEXT("false")));

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FColorAccessibleAlternativeTest, "HorrorProject.Accessibility.AccessibleAlternative", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FColorAccessibleAlternativeTest::RunTest(const FString& Parameters)
{
    // Test light color gets dark alternative
    FLinearColor LightColor = FLinearColor(0.8f, 0.8f, 0.8f);
    FLinearColor DarkAlternative = AColorBlindMode::GetAccessibleAlternative(LightColor, 1);
    TestTrue(TEXT("Light color gets dark alternative"), DarkAlternative.GetLuminance() < 0.5f);

    // Test dark color gets light alternative
    FLinearColor DarkColor = FLinearColor(0.2f, 0.2f, 0.2f);
    FLinearColor LightAlternative = AColorBlindMode::GetAccessibleAlternative(DarkColor, 1);
    TestTrue(TEXT("Dark color gets light alternative"), LightAlternative.GetLuminance() > 0.5f);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FColorBlindSimulationTest, "HorrorProject.Accessibility.ColorBlindSimulation", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FColorBlindSimulationTest::RunTest(const FString& Parameters)
{
    FLinearColor TestColor = FLinearColor(0.5f, 0.5f, 0.5f);

    // Test type 0 (None) - no transformation
    FLinearColor NoChange = AColorBlindMode::SimulateColorBlindness(TestColor, 0, 1.0f);
    TestEqual(TEXT("Type 0 produces no change"), NoChange, TestColor);

    // Test type 1 (Protanopia)
    FLinearColor Protanopia = AColorBlindMode::SimulateColorBlindness(TestColor, 1, 1.0f);
    TestNotEqual(TEXT("Type 1 produces protanopia transformation"), Protanopia, TestColor);

    // Test type 2 (Deuteranopia)
    FLinearColor Deuteranopia = AColorBlindMode::SimulateColorBlindness(TestColor, 2, 1.0f);
    TestNotEqual(TEXT("Type 2 produces deuteranopia transformation"), Deuteranopia, TestColor);

    // Test type 3 (Tritanopia)
    FLinearColor Tritanopia = AColorBlindMode::SimulateColorBlindness(TestColor, 3, 1.0f);
    TestNotEqual(TEXT("Type 3 produces tritanopia transformation"), Tritanopia, TestColor);

    return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR
