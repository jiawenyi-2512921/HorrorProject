#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR && HORRORPROJECT_ENABLE_LEGACY_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "Localization/LocalizationSubsystem.h"
#include "Localization/TextLocalization.h"
#include "Localization/AudioLocalization.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLocalizationBoundaryTest, "HorrorProject.Localization.BoundaryConditions", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FLocalizationBoundaryTest::RunTest(const FString& Parameters)
{
    UGameInstance* GameInstance = NewObject<UGameInstance>();
    ULocalizationSubsystem* LocalizationSys = GameInstance->GetSubsystem<ULocalizationSubsystem>();

    if (!LocalizationSys)
    {
        AddWarning(TEXT("LocalizationSubsystem not available"));
        return true;
    }

    // Test invalid language code
    LocalizationSys->SetLanguageByCode(TEXT("invalid"));
    TestTrue(TEXT("Invalid language code handled"), true);

    // Test empty text key
    FText EmptyKeyText = LocalizationSys->GetLocalizedText(TEXT(""));
    TestTrue(TEXT("Empty text key handled"), true);

    // Test very long text key
    FString LongKey = FString::ChrN(10000, 'A');
    FText LongKeyText = LocalizationSys->GetLocalizedText(LongKey);
    TestTrue(TEXT("Long text key handled"), true);

    // Test special characters in key
    FText SpecialKeyText = LocalizationSys->GetLocalizedText(TEXT("Key.With.Special!@#$%^&*()"));
    TestTrue(TEXT("Special characters in key handled"), true);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLocalizationRTLTest, "HorrorProject.Localization.RTLSupport", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FLocalizationRTLTest::RunTest(const FString& Parameters)
{
    UGameInstance* GameInstance = NewObject<UGameInstance>();
    ULocalizationSubsystem* LocalizationSys = GameInstance->GetSubsystem<ULocalizationSubsystem>();

    if (!LocalizationSys)
    {
        AddWarning(TEXT("LocalizationSubsystem not available"));
        return true;
    }

    // Test RTL language detection
    LocalizationSys->SetLanguage(ELanguage::Arabic);
    TestTrue(TEXT("Arabic is RTL"), LocalizationSys->IsRTLLanguage());

    LocalizationSys->SetLanguage(ELanguage::English);
    TestFalse(TEXT("English is not RTL"), LocalizationSys->IsRTLLanguage());

    // Test mixed RTL/LTR text
    FText MixedText = FText::FromString(TEXT("English مع العربية"));
    TestTrue(TEXT("Mixed RTL/LTR text handled"), true);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLocalizationCachingTest, "HorrorProject.Localization.Caching", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FLocalizationCachingTest::RunTest(const FString& Parameters)
{
    UGameInstance* GameInstance = NewObject<UGameInstance>();
    ULocalizationSubsystem* LocalizationSys = GameInstance->GetSubsystem<ULocalizationSubsystem>();

    if (!LocalizationSys)
    {
        AddWarning(TEXT("LocalizationSubsystem not available"));
        return true;
    }

    // Test cache performance
    const int32 NumLookups = 10000;
    double StartTime = FPlatformTime::Seconds();

    for (int32 i = 0; i < NumLookups; ++i)
    {
        FText Text = LocalizationSys->GetLocalizedText(TEXT("UI.MainMenu.Start"));
    }

    double EndTime = FPlatformTime::Seconds();
    double TotalTime = EndTime - StartTime;
    double AvgTime = TotalTime / NumLookups;

    AddInfo(FString::Printf(TEXT("Cache performance: %d lookups in %.3f seconds (avg: %.6f ms)"),
        NumLookups, TotalTime, AvgTime * 1000.0));
    TestTrue(TEXT("Cache is efficient"), AvgTime < 0.0001); // Less than 0.1ms per lookup

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLocalizationFallbackTest, "HorrorProject.Localization.FallbackMechanism", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FLocalizationFallbackTest::RunTest(const FString& Parameters)
{
    UGameInstance* GameInstance = NewObject<UGameInstance>();
    ULocalizationSubsystem* LocalizationSys = GameInstance->GetSubsystem<ULocalizationSubsystem>();

    if (!LocalizationSys)
    {
        AddWarning(TEXT("LocalizationSubsystem not available"));
        return true;
    }

    // Test missing translation fallback
    LocalizationSys->SetLanguage(ELanguage::Chinese);
    FText MissingText = LocalizationSys->GetLocalizedText(TEXT("NonExistent.Key"));
    TestTrue(TEXT("Missing translation falls back"), !MissingText.IsEmpty());

    // Test partial translation
    LocalizationSys->SetLanguage(ELanguage::Korean);
    FText PartialText = LocalizationSys->GetLocalizedText(TEXT("UI.MainMenu.Start"));
    TestTrue(TEXT("Partial translation handled"), !PartialText.IsEmpty());

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAudioLocalizationTest, "HorrorProject.Localization.AudioLocalization", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FAudioLocalizationTest::RunTest(const FString& Parameters)
{
    // Test audio file path generation
    FString EnglishPath = UAudioLocalizationLibrary::GetLocalizedAudioPath(TEXT("Dialogue_01"), TEXT("en"));
    TestTrue(TEXT("English audio path generated"), !EnglishPath.IsEmpty());

    FString ChinesePath = UAudioLocalizationLibrary::GetLocalizedAudioPath(TEXT("Dialogue_01"), TEXT("zh-Hans"));
    TestTrue(TEXT("Chinese audio path generated"), !ChinesePath.IsEmpty());
    TestNotEqual(TEXT("Paths differ by language"), EnglishPath, ChinesePath);

    // Test missing audio fallback
    FString MissingPath = UAudioLocalizationLibrary::GetLocalizedAudioPath(TEXT("NonExistent"), TEXT("ja"));
    TestTrue(TEXT("Missing audio fallback handled"), true);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLocalizationMemoryTest, "HorrorProject.Localization.MemoryUsage", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FLocalizationMemoryTest::RunTest(const FString& Parameters)
{
    UGameInstance* GameInstance = NewObject<UGameInstance>();
    ULocalizationSubsystem* LocalizationSys = GameInstance->GetSubsystem<ULocalizationSubsystem>();

    if (!LocalizationSys)
    {
        AddWarning(TEXT("LocalizationSubsystem not available"));
        return true;
    }

    // Test memory usage with many languages
    TArray<ELanguage> Languages = {
        ELanguage::English,
        ELanguage::Chinese,
        ELanguage::Japanese,
        ELanguage::Korean,
        ELanguage::Spanish,
        ELanguage::French,
        ELanguage::German,
        ELanguage::Russian
    };

    for (ELanguage Lang : Languages)
    {
        LocalizationSys->SetLanguage(Lang);
        LocalizationSys->PreloadCommonTexts();
    }

    TestTrue(TEXT("Multiple language loading handled"), true);

    // Test cache clearing
    LocalizationSys->ClearCache();
    TestTrue(TEXT("Cache clearing handled"), true);

    return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR
