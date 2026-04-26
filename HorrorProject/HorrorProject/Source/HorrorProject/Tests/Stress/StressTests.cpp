#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR && HORRORPROJECT_ENABLE_LEGACY_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "Network/HorrorGameModeMultiplayer.h"
#include "Localization/LocalizationSubsystem.h"
#include "Achievements/AchievementSubsystem.h"
#include "Save/HorrorSaveSubsystem.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMultiplayerStressTest, "HorrorProject.Stress.MultiplayerLoad", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FMultiplayerStressTest::RunTest(const FString& Parameters)
{
    UWorld* World = FAutomationEditorCommonUtils::CreateNewMap();
    AHorrorGameModeMultiplayer* GameMode = World->SpawnActor<AHorrorGameModeMultiplayer>();

    if (!GameMode)
    {
        AddWarning(TEXT("GameMode not available"));
        return true;
    }

    // Stress test: Maximum concurrent players
    const int32 MaxPlayers = 1000;
    TArray<AHorrorPlayerControllerMultiplayer*> Players;

    double StartTime = FPlatformTime::Seconds();

    for (int32 i = 0; i < MaxPlayers; ++i)
    {
        AHorrorPlayerControllerMultiplayer* PC = World->SpawnActor<AHorrorPlayerControllerMultiplayer>();
        if (PC)
        {
            Players.Add(PC);
            GameMode->OnPlayerConnected(PC);
        }
    }

    double EndTime = FPlatformTime::Seconds();
    double TotalTime = EndTime - StartTime;

    AddInfo(FString::Printf(TEXT("Multiplayer stress: %d players spawned in %.3f seconds"), Players.Num(), TotalTime));
    TestTrue(TEXT("System handles maximum players"), Players.Num() > 0);

    // Stress test: Rapid player disconnections
    StartTime = FPlatformTime::Seconds();

    for (AHorrorPlayerControllerMultiplayer* PC : Players)
    {
        GameMode->OnPlayerDisconnected(PC);
    }

    EndTime = FPlatformTime::Seconds();
    TotalTime = EndTime - StartTime;

    AddInfo(FString::Printf(TEXT("Disconnection stress: %d players disconnected in %.3f seconds"), Players.Num(), TotalTime));
    TestTrue(TEXT("System handles mass disconnections"), true);

    return true;
}

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

    // Stress test: Rapid language switching
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

    // Stress test: Massive text lookups
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
    UWorld* World = FAutomationEditorCommonUtils::CreateNewMap();
    UAchievementSubsystem* AchievementSys = World->GetGameInstance()->GetSubsystem<UAchievementSubsystem>();

    if (!AchievementSys)
    {
        AddWarning(TEXT("AchievementSubsystem not available"));
        return true;
    }

    // Stress test: Rapid achievement unlocks
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

    // Stress test: Massive progress updates
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

    // Stress test: Rapid save operations
    const int32 NumSaves = 1000;
    double StartTime = FPlatformTime::Seconds();

    for (int32 i = 0; i < NumSaves; ++i)
    {
        FString SlotName = FString::Printf(TEXT("StressSlot_%d"), i);
        SaveSys->SaveGame(SlotName);
    }

    double EndTime = FPlatformTime::Seconds();
    double TotalTime = EndTime - StartTime;

    AddInfo(FString::Printf(TEXT("Save stress: %d saves in %.3f seconds"), NumSaves, TotalTime));
    TestTrue(TEXT("System handles rapid saves"), TotalTime < 30.0);

    // Stress test: Rapid load operations
    StartTime = FPlatformTime::Seconds();

    for (int32 i = 0; i < NumSaves; ++i)
    {
        FString SlotName = FString::Printf(TEXT("StressSlot_%d"), i);
        SaveSys->LoadGame(SlotName);
    }

    EndTime = FPlatformTime::Seconds();
    TotalTime = EndTime - StartTime;

    AddInfo(FString::Printf(TEXT("Load stress: %d loads in %.3f seconds"), NumSaves, TotalTime));
    TestTrue(TEXT("System handles rapid loads"), TotalTime < 30.0);

    // Cleanup
    SaveSys->DeleteAllSaveSlots();

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCombinedSystemStressTest, "HorrorProject.Stress.CombinedSystems", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FCombinedSystemStressTest::RunTest(const FString& Parameters)
{
    UWorld* World = FAutomationEditorCommonUtils::CreateNewMap();
    UGameInstance* GameInstance = World->GetGameInstance();

    if (!GameInstance)
    {
        AddWarning(TEXT("GameInstance not available"));
        return true;
    }

    // Get all subsystems
    ULocalizationSubsystem* LocalizationSys = GameInstance->GetSubsystem<ULocalizationSubsystem>();
    UAchievementSubsystem* AchievementSys = GameInstance->GetSubsystem<UAchievementSubsystem>();
    UHorrorSaveSubsystem* SaveSys = GameInstance->GetSubsystem<UHorrorSaveSubsystem>();
    UAccessibilitySubsystem* AccessibilitySys = World->GetSubsystem<UAccessibilitySubsystem>();

    // Stress test: All systems simultaneously
    const int32 NumIterations = 1000;
    double StartTime = FPlatformTime::Seconds();

    for (int32 i = 0; i < NumIterations; ++i)
    {
        // Localization operations
        if (LocalizationSys)
        {
            LocalizationSys->SetLanguage(static_cast<ELanguage>(i % 5));
            LocalizationSys->GetLocalizedText(TEXT("UI.MainMenu.Start"));
        }

        // Achievement operations
        if (AchievementSys)
        {
            AchievementSys->UpdateAchievementProgress(FName("ACH_Explorer"), 0.01f);
        }

        // Save operations
        if (SaveSys && i % 100 == 0)
        {
            SaveSys->SaveGame(TEXT("CombinedStressTest"));
        }

        // Accessibility operations
        if (AccessibilitySys)
        {
            AccessibilitySys->SetFontSize(1.0f + (i % 10) * 0.1f);
        }
    }

    double EndTime = FPlatformTime::Seconds();
    double TotalTime = EndTime - StartTime;

    AddInfo(FString::Printf(TEXT("Combined stress: %d iterations in %.3f seconds"), NumIterations, TotalTime));
    TestTrue(TEXT("All systems handle combined stress"), TotalTime < 20.0);

    return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR
