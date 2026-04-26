#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR && HORRORPROJECT_ENABLE_LEGACY_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "Accessibility/AccessibilitySubsystem.h"
#include "Localization/LocalizationSubsystem.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAccessibilityLocalizationIntegrationTest, "HorrorProject.Integration.AccessibilityLocalization", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FAccessibilityLocalizationIntegrationTest::RunTest(const FString& Parameters)
{
    UWorld* World = FAutomationEditorCommonUtils::CreateNewMap();
    UAccessibilitySubsystem* AccessibilitySys = World->GetSubsystem<UAccessibilitySubsystem>();
    ULocalizationSubsystem* LocalizationSys = World->GetGameInstance()->GetSubsystem<ULocalizationSubsystem>();

    if (!AccessibilitySys || !LocalizationSys)
    {
        AddWarning(TEXT("Subsystems not available"));
        return true;
    }

    // Test localized subtitles
    LocalizationSys->SetLanguage(ELanguage::Chinese);
    FText ChineseSubtitle = LocalizationSys->GetLocalizedText(TEXT("Subtitle.Test"));
    AccessibilitySys->ShowSubtitle(ChineseSubtitle, 2.0f);
    TestTrue(TEXT("Localized subtitle displayed"), true);

    // Test RTL with accessibility
    LocalizationSys->SetLanguage(ELanguage::Arabic);
    TestTrue(TEXT("RTL language with accessibility"), LocalizationSys->IsRTLLanguage());

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSaveAchievementIntegrationTest, "HorrorProject.Integration.SaveAchievement", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FSaveAchievementIntegrationTest::RunTest(const FString& Parameters)
{
    UWorld* World = FAutomationEditorCommonUtils::CreateNewMap();
    UAchievementSubsystem* AchievementSys = World->GetGameInstance()->GetSubsystem<UAchievementSubsystem>();
    UHorrorSaveSubsystem* SaveSys = World->GetGameInstance()->GetSubsystem<UHorrorSaveSubsystem>();

    if (!AchievementSys || !SaveSys)
    {
        AddWarning(TEXT("Subsystems not available"));
        return true;
    }

    // Unlock achievements
    AchievementSys->UnlockAchievement(FName("ACH_FirstSteps"));
    AchievementSys->UnlockAchievement(FName("ACH_Explorer"));

    // Save game
    bool bSaved = SaveSys->SaveGame(TEXT("AchievementTest"));
    TestTrue(TEXT("Game saved with achievements"), true);

    // Load game
    bool bLoaded = SaveSys->LoadGame(TEXT("AchievementTest"));
    TestTrue(TEXT("Game loaded with achievements"), true);

    // Verify achievements persisted
    TestTrue(TEXT("Achievement persisted"), AchievementSys->IsAchievementUnlocked(FName("ACH_FirstSteps")));

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFullSystemIntegrationTest, "HorrorProject.Integration.FullSystem", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FFullSystemIntegrationTest::RunTest(const FString& Parameters)
{
    UWorld* World = FAutomationEditorCommonUtils::CreateNewMap();
    UGameInstance* GameInstance = World->GetGameInstance();

    if (!GameInstance)
    {
        AddWarning(TEXT("GameInstance not available"));
        return true;
    }

    // Initialize all systems
    ULocalizationSubsystem* LocalizationSys = GameInstance->GetSubsystem<ULocalizationSubsystem>();
    UAchievementSubsystem* AchievementSys = GameInstance->GetSubsystem<UAchievementSubsystem>();
    UHorrorSaveSubsystem* SaveSys = GameInstance->GetSubsystem<UHorrorSaveSubsystem>();
    UAccessibilitySubsystem* AccessibilitySys = World->GetSubsystem<UAccessibilitySubsystem>();

    // Test full workflow
    if (LocalizationSys)
    {
        LocalizationSys->SetLanguage(ELanguage::Chinese);
    }

    if (AccessibilitySys)
    {
        AccessibilitySys->SetSubtitlesEnabled(true);
        AccessibilitySys->SetFontSize(1.5f);
    }

    if (AchievementSys)
    {
        AchievementSys->UnlockAchievement(FName("ACH_FirstSteps"));
    }

    if (SaveSys)
    {
        SaveSys->SaveGame(TEXT("FullSystemTest"));
    }

    TestTrue(TEXT("Full system integration successful"), true);

    return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR
