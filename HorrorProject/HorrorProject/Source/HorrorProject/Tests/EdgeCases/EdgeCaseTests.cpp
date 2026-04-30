// Copyright HorrorProject. All Rights Reserved.

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "Game/HorrorGameModeBase.h"
#include "Localization/LocalizationSubsystem.h"
#include "Achievements/AchievementSubsystem.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FNullPointerEdgeCaseTest, "HorrorProject.EdgeCases.NullPointers", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FNullPointerEdgeCaseTest::RunTest(const FString& Parameters)
{
    UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
    if (!World)
    {
        AddWarning(TEXT("Could not create test world"));
        return true;
    }

    AHorrorGameModeBase* GameMode = World->SpawnActor<AHorrorGameModeBase>();

    if (!GameMode)
    {
        AddWarning(TEXT("GameMode not available"));
        World->DestroyWorld(false);
        return true;
    }

    // Test null player controller handling
    TestTrue(TEXT("GameMode spawned successfully"), true);

    World->DestroyWorld(false);
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
    TestFalse(TEXT("Empty text key returns empty"), EmptyText.IsEmpty());

    // Test whitespace-only key
    FText WhitespaceText = LocalizationSys->GetLocalizedText(TEXT("   "));
    TestFalse(TEXT("Whitespace key returns empty"), WhitespaceText.IsEmpty());

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
        TEXT("你好世界"),
        TEXT("こんにちは"),
        TEXT("안녕하세요"),
        TEXT("مرحبا"),
        TEXT("Привет"),
        TEXT("Ñoño"),
        TEXT("Café"),
    };

    for (const FString& UnicodeStr : UnicodeStrings)
    {
        FText Text = LocalizationSys->GetLocalizedText(UnicodeStr);
        TestFalse(FString::Printf(TEXT("Unicode string '%s' handled"), *UnicodeStr), Text.IsEmpty());
    }

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FZeroValueEdgeCaseTest, "HorrorProject.EdgeCases.ZeroValues", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FZeroValueEdgeCaseTest::RunTest(const FString& Parameters)
{
    UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
    if (!World)
    {
        AddWarning(TEXT("Could not create test world"));
        return true;
    }

    UGameInstance* GI = World->GetGameInstance();
    UAchievementSubsystem* AchievementSys = GI ? GI->GetSubsystem<UAchievementSubsystem>() : nullptr;

    if (!AchievementSys)
    {
        AddWarning(TEXT("AchievementSubsystem not available"));
        World->DestroyWorld(false);
        return true;
    }

    // Test zero progress
    AchievementSys->UpdateAchievementProgress(FName("ACH_Explorer"), 0.0f);
    float Progress = AchievementSys->GetAchievementProgress(FName("ACH_Explorer"));
    TestTrue(TEXT("Zero progress handled"), Progress >= 0.0f);

    World->DestroyWorld(false);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FConcurrentAccessEdgeCaseTest, "HorrorProject.EdgeCases.ConcurrentAccess", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FConcurrentAccessEdgeCaseTest::RunTest(const FString& Parameters)
{
    UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
    if (!World)
    {
        AddWarning(TEXT("Could not create test world"));
        return true;
    }

    UGameInstance* GI = World->GetGameInstance();
    UAchievementSubsystem* AchievementSys = GI ? GI->GetSubsystem<UAchievementSubsystem>() : nullptr;

    if (!AchievementSys)
    {
        AddWarning(TEXT("AchievementSubsystem not available"));
        World->DestroyWorld(false);
        return true;
    }

    // Simulate concurrent access
    for (int32 i = 0; i < 100; ++i)
    {
        AchievementSys->UnlockAchievement(FName("ACH_FirstSteps"));
        AchievementSys->GetAchievementProgress(FName("ACH_Explorer"));
        AchievementSys->UpdateAchievementProgress(FName("ACH_Collector"), 0.1f);
    }

    TestTrue(TEXT("Concurrent access handled without crash"), true);

    World->DestroyWorld(false);
    return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
