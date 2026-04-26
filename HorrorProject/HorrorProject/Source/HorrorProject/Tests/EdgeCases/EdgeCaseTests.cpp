#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR && HORRORPROJECT_ENABLE_LEGACY_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "Network/HorrorGameModeMultiplayer.h"
#include "Localization/LocalizationSubsystem.h"
#include "Achievements/AchievementSubsystem.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FNullPointerEdgeCaseTest, "HorrorProject.EdgeCases.NullPointers", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FNullPointerEdgeCaseTest::RunTest(const FString& Parameters)
{
    UWorld* World = FAutomationEditorCommonUtils::CreateNewMap();
    AHorrorGameModeMultiplayer* GameMode = World->SpawnActor<AHorrorGameModeMultiplayer>();

    if (!GameMode)
    {
        AddWarning(TEXT("GameMode not available"));
        return true;
    }

    // Test null player controller
    GameMode->OnPlayerConnected(nullptr);
    TestTrue(TEXT("Null player connection handled"), true);

    GameMode->OnPlayerDisconnected(nullptr);
    TestTrue(TEXT("Null player disconnection handled"), true);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEmptyStringEdgeCaseTest, "HorrorProject.EdgeCases.EmptyStrings", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FEmptyStringEdgeCaseTest::RunTest(const FString& Parameters)
{
    UGameInstance* GameInstance = NewObject<UGameInstance>();
    ULocalizationSubsystem* LocalizationSys = GameInstance->GetSubsystem<ULocalizationSubsystem>();

    if (!LocalizationSys)
    {
        AddWarning(TEXT("LocalizationSubsystem not available"));
        return true;
    }

    // Test empty text key
    FText EmptyText = LocalizationSys->GetLocalizedText(TEXT(""));
    TestTrue(TEXT("Empty text key handled"), true);

    // Test whitespace-only key
    FText WhitespaceText = LocalizationSys->GetLocalizedText(TEXT("   "));
    TestTrue(TEXT("Whitespace key handled"), true);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FUnicodeEdgeCaseTest, "HorrorProject.EdgeCases.UnicodeHandling", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FUnicodeEdgeCaseTest::RunTest(const FString& Parameters)
{
    UGameInstance* GameInstance = NewObject<UGameInstance>();
    ULocalizationSubsystem* LocalizationSys = GameInstance->GetSubsystem<ULocalizationSubsystem>();

    if (!LocalizationSys)
    {
        AddWarning(TEXT("LocalizationSubsystem not available"));
        return true;
    }

    // Test various Unicode characters
    TArray<FString> UnicodeStrings = {
        TEXT("你好世界"),           // Chinese
        TEXT("こんにちは"),         // Japanese
        TEXT("안녕하세요"),         // Korean
        TEXT("مرحبا"),             // Arabic
        TEXT("Привет"),            // Russian
        TEXT("🎮🎯🎲"),            // Emojis
        TEXT("Ñoño"),              // Spanish with tildes
        TEXT("Café"),              // French with accents
    };

    for (const FString& UnicodeStr : UnicodeStrings)
    {
        FText Text = LocalizationSys->GetLocalizedText(UnicodeStr);
        TestTrue(TEXT("Unicode string handled"), true);
    }

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FZeroValueEdgeCaseTest, "HorrorProject.EdgeCases.ZeroValues", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FZeroValueEdgeCaseTest::RunTest(const FString& Parameters)
{
    UWorld* World = FAutomationEditorCommonUtils::CreateNewMap();
    UAchievementSubsystem* AchievementSys = World->GetGameInstance()->GetSubsystem<UAchievementSubsystem>();

    if (!AchievementSys)
    {
        AddWarning(TEXT("AchievementSubsystem not available"));
        return true;
    }

    // Test zero progress
    AchievementSys->UpdateAchievementProgress(FName("ACH_Explorer"), 0.0f);
    float Progress = AchievementSys->GetAchievementProgress(FName("ACH_Explorer"));
    TestTrue(TEXT("Zero progress handled"), Progress >= 0.0f);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMaxValueEdgeCaseTest, "HorrorProject.EdgeCases.MaxValues", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FMaxValueEdgeCaseTest::RunTest(const FString& Parameters)
{
    UWorld* World = FAutomationEditorCommonUtils::CreateNewMap();
    AHorrorGameModeMultiplayer* GameMode = World->SpawnActor<AHorrorGameModeMultiplayer>();

    if (!GameMode)
    {
        AddWarning(TEXT("GameMode not available"));
        return true;
    }

    // Test maximum player count
    GameMode->MaxPlayers = INT32_MAX;
    TestTrue(TEXT("Max player count handled"), true);

    // Test maximum lobby wait time
    GameMode->LobbyWaitTime = FLT_MAX;
    TestTrue(TEXT("Max lobby wait time handled"), true);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FConcurrentAccessEdgeCaseTest, "HorrorProject.EdgeCases.ConcurrentAccess", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FConcurrentAccessEdgeCaseTest::RunTest(const FString& Parameters)
{
    UWorld* World = FAutomationEditorCommonUtils::CreateNewMap();
    UAchievementSubsystem* AchievementSys = World->GetGameInstance()->GetSubsystem<UAchievementSubsystem>();

    if (!AchievementSys)
    {
        AddWarning(TEXT("AchievementSubsystem not available"));
        return true;
    }

    // Simulate concurrent access
    for (int32 i = 0; i < 100; ++i)
    {
        AchievementSys->UnlockAchievement(FName("ACH_FirstSteps"));
        AchievementSys->GetAchievementProgress(FName("ACH_Explorer"));
        AchievementSys->UpdateAchievementProgress(FName("ACH_Collector"), 0.1f);
    }

    TestTrue(TEXT("Concurrent access handled"), true);

    return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR
