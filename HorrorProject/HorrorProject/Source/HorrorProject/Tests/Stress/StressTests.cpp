// Copyright HorrorProject. All Rights Reserved.

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "Game/HorrorGameModeBase.h"
#include "Localization/LocalizationSubsystem.h"
#include "Achievements/AchievementSubsystem.h"
#include "Save/HorrorSaveSubsystem.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLocalizationStressTest, "HorrorProject.Stress.LocalizationLoad", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FLocalizationStressTest::RunTest(const FString& Parameters)
{
    UGameInstance* GameInstance = NewObject<UGameInstance>();
    ULocalizationSubsystem* LocalizationSys = GameInstance->GetSubsystem<ULocalizationSubsystem>();

    if (!LocalizationSys)
    {
        AddWarning(TEXT("LocalizationSubsystem not available"));
        return true;
    }

    const int32 NumSwitches = 10000;
    TArray<ELanguage> Languages = {
        ELanguage::English,
        ELanguage::Chinese,
        ELanguage::Japanese,
        ELanguage::Korean,
        ELanguage::Spanish
    };

    double StartTime = FPlatformTime::Seconds();

    for (int32 i = 0; i < NumSwitches; ++i)
    {
        ELanguage Lang = Languages[i % Languages.Num()];
        LocalizationSys->SetLanguage(Lang);
    }

    double EndTime = FPlatformTime::Seconds();
    double TotalTime = EndTime - StartTime;

    AddInfo(FString::Printf(TEXT("Language switching stress: %d switches in %.3f seconds"), NumSwitches, TotalTime));
    TestTrue(TEXT("System handles rapid language switching"), TotalTime < 10.0);

    const int32 NumLookups = 1000000;
    StartTime = FPlatformTime::Seconds();

    for (int32 i = 0; i < NumLookups; ++i)
    {
        LocalizationSys->GetLocalizedText(TEXT("UI.MainMenu.Start"));
    }

    EndTime = FPlatformTime::Seconds();
    TotalTime = EndTime - StartTime;

    AddInfo(FString::Printf(TEXT("Text lookup stress: %d lookups in %.3f seconds"), NumLookups, TotalTime));
    TestTrue(TEXT("System handles massive text lookups"), TotalTime < 5.0);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAchievementStressTest, "HorrorProject.Stress.AchievementLoad", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FAchievementStressTest::RunTest(const FString& Parameters)
{
    UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
    UAchievementSubsystem* AchievementSys = World->GetGameInstance()->GetSubsystem<UAchievementSubsystem>();

    if (!AchievementSys)
    {
        AddWarning(TEXT("AchievementSubsystem not available"));
        return true;
    }

    const int32 NumUnlocks = 100000;
    double StartTime = FPlatformTime::Seconds();

    for (int32 i = 0; i < NumUnlocks; ++i)
    {
        FName AchievementID = FName(*FString::Printf(TEXT("ACH_Test_%d"), i % 100));
        AchievementSys->UnlockAchievement(AchievementID);
    }

    double EndTime = FPlatformTime::Seconds();
    double TotalTime = EndTime - StartTime;

    AddInfo(FString::Printf(TEXT("Achievement unlock stress: %d unlocks in %.3f seconds"), NumUnlocks, TotalTime));
    TestTrue(TEXT("System handles rapid unlocks"), TotalTime < 10.0);

    const int32 NumUpdates = 1000000;
    StartTime = FPlatformTime::Seconds();

    for (int32 i = 0; i < NumUpdates; ++i)
    {
        AchievementSys->UpdateAchievementProgress(FName("ACH_Explorer"), 0.0001f);
    }

    EndTime = FPlatformTime::Seconds();
    TotalTime = EndTime - StartTime;

    AddInfo(FString::Printf(TEXT("Progress update stress: %d updates in %.3f seconds"), NumUpdates, TotalTime));
    TestTrue(TEXT("System handles massive progress updates"), TotalTime < 10.0);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSaveSystemStressTest, "HorrorProject.Stress.SaveSystemLoad", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FSaveSystemStressTest::RunTest(const FString& Parameters)
{
    UGameInstance* GameInstance = NewObject<UGameInstance>();
    UHorrorSaveSubsystem* SaveSys = GameInstance->GetSubsystem<UHorrorSaveSubsystem>();

    if (!SaveSys)
    {
        AddWarning(TEXT("SaveSubsystem not available"));
        return true;
    }

    // Test HasSave rapid queries
    const int32 NumIterations = 1000;
    double StartTime = FPlatformTime::Seconds();

    for (int32 i = 0; i < NumIterations; ++i)
    {
        SaveSys->HasSave();
    }

    double EndTime = FPlatformTime::Seconds();
    double TotalTime = EndTime - StartTime;

    AddInfo(FString::Printf(TEXT("Save query stress: %d queries in %.3f seconds"), NumIterations, TotalTime));
    TestTrue(TEXT("System handles rapid save queries"), TotalTime < 5.0);

    SaveSys->ClearCachedSaveOnly();

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCombinedSystemStressTest, "HorrorProject.Stress.CombinedSystems", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FCombinedSystemStressTest::RunTest(const FString& Parameters)
{
    UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
    if (!World)
    {
        AddWarning(TEXT("Could not create test world"));
        return true;
    }

    UGameInstance* GameInstance = World->GetGameInstance();

    if (!GameInstance)
    {
        AddWarning(TEXT("GameInstance not available"));
        World->DestroyWorld(false);
        return true;
    }

    ULocalizationSubsystem* LocalizationSys = GameInstance->GetSubsystem<ULocalizationSubsystem>();
    UAchievementSubsystem* AchievementSys = GameInstance->GetSubsystem<UAchievementSubsystem>();

    const int32 NumIterations = 1000;
    double StartTime = FPlatformTime::Seconds();

    for (int32 i = 0; i < NumIterations; ++i)
    {
        if (LocalizationSys)
        {
            LocalizationSys->SetLanguage(static_cast<ELanguage>(i % 5));
            LocalizationSys->GetLocalizedText(TEXT("UI.MainMenu.Start"));
        }

        if (AchievementSys)
        {
            AchievementSys->UpdateAchievementProgress(FName("ACH_Explorer"), 0.01f);
        }
    }

    double EndTime = FPlatformTime::Seconds();
    double TotalTime = EndTime - StartTime;

    AddInfo(FString::Printf(TEXT("Combined stress: %d iterations in %.3f seconds"), NumIterations, TotalTime));
    TestTrue(TEXT("All systems handle combined stress"), TotalTime < 20.0);

    return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
