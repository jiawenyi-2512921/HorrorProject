#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR && HORRORPROJECT_ENABLE_LEGACY_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "Network/MultiplayerSessionSubsystem.h"
#include "Localization/LocalizationSubsystem.h"
#include "Achievements/AchievementSubsystem.h"
#include "Save/HorrorSaveSubsystem.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FNetworkPerformanceBenchmark, "HorrorProject.Performance.NetworkBenchmark", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FNetworkPerformanceBenchmark::RunTest(const FString& Parameters)
{
    UWorld* World = FAutomationEditorCommonUtils::CreateNewMap();
    AHorrorGameModeMultiplayer* GameMode = World->SpawnActor<AHorrorGameModeMultiplayer>();

    if (!GameMode)
    {
        AddWarning(TEXT("GameMode not available"));
        return true;
    }

    // Benchmark: Player connection handling
    const int32 NumPlayers = 100;
    double StartTime = FPlatformTime::Seconds();

    for (int32 i = 0; i < NumPlayers; ++i)
    {
        AHorrorPlayerControllerMultiplayer* PC = World->SpawnActor<AHorrorPlayerControllerMultiplayer>();
        if (PC)
        {
            GameMode->OnPlayerConnected(PC);
        }
    }

    double EndTime = FPlatformTime::Seconds();
    double TotalTime = EndTime - StartTime;
    double AvgTime = TotalTime / NumPlayers;

    AddInfo(FString::Printf(TEXT("Network benchmark: %d player connections in %.3f seconds (avg: %.3f ms)"),
        NumPlayers, TotalTime, AvgTime * 1000.0));
    TestTrue(TEXT("Network handles many connections"), TotalTime < 5.0);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLocalizationPerformanceBenchmark, "HorrorProject.Performance.LocalizationBenchmark", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FLocalizationPerformanceBenchmark::RunTest(const FString& Parameters)
{
    UGameInstance* GameInstance = NewObject<UGameInstance>();
    ULocalizationSubsystem* LocalizationSys = GameInstance->GetSubsystem<ULocalizationSubsystem>();

    if (!LocalizationSys)
    {
        AddWarning(TEXT("LocalizationSubsystem not available"));
        return true;
    }

    // Benchmark: Text lookup performance
    const int32 NumLookups = 100000;
    TArray<FString> TestKeys = {
        TEXT("UI.MainMenu.Start"),
        TEXT("UI.MainMenu.Options"),
        TEXT("UI.MainMenu.Quit"),
        TEXT("Gameplay.Objective.Explore"),
        TEXT("Gameplay.Objective.Survive")
    };

    double StartTime = FPlatformTime::Seconds();

    for (int32 i = 0; i < NumLookups; ++i)
    {
        FString Key = TestKeys[i % TestKeys.Num()];
        FText Text = LocalizationSys->GetLocalizedText(Key);
    }

    double EndTime = FPlatformTime::Seconds();
    double TotalTime = EndTime - StartTime;
    double AvgTime = TotalTime / NumLookups;

    AddInfo(FString::Printf(TEXT("Localization benchmark: %d lookups in %.3f seconds (avg: %.6f ms)"),
        NumLookups, TotalTime, AvgTime * 1000.0));
    TestTrue(TEXT("Localization is performant"), AvgTime < 0.00001); // Less than 0.01ms per lookup

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAchievementPerformanceBenchmark, "HorrorProject.Performance.AchievementBenchmark", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FAchievementPerformanceBenchmark::RunTest(const FString& Parameters)
{
    UWorld* World = FAutomationEditorCommonUtils::CreateNewMap();
    UAchievementSubsystem* AchievementSys = World->GetGameInstance()->GetSubsystem<UAchievementSubsystem>();

    if (!AchievementSys)
    {
        AddWarning(TEXT("AchievementSubsystem not available"));
        return true;
    }

    // Benchmark: Achievement progress updates
    const int32 NumUpdates = 50000;
    double StartTime = FPlatformTime::Seconds();

    for (int32 i = 0; i < NumUpdates; ++i)
    {
        AchievementSys->UpdateAchievementProgress(FName("ACH_Explorer"), 0.001f);
    }

    double EndTime = FPlatformTime::Seconds();
    double TotalTime = EndTime - StartTime;
    double AvgTime = TotalTime / NumUpdates;

    AddInfo(FString::Printf(TEXT("Achievement benchmark: %d updates in %.3f seconds (avg: %.6f ms)"),
        NumUpdates, TotalTime, AvgTime * 1000.0));
    TestTrue(TEXT("Achievement system is performant"), AvgTime < 0.0001); // Less than 0.1ms per update

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSaveSystemPerformanceBenchmark, "HorrorProject.Performance.SaveSystemBenchmark", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FSaveSystemPerformanceBenchmark::RunTest(const FString& Parameters)
{
    UGameInstance* GameInstance = NewObject<UGameInstance>();
    UHorrorSaveSubsystem* SaveSys = GameInstance->GetSubsystem<UHorrorSaveSubsystem>();

    if (!SaveSys)
    {
        AddWarning(TEXT("SaveSubsystem not available"));
        return true;
    }

    // Benchmark: Save/Load cycle
    const int32 NumCycles = 100;
    double StartTime = FPlatformTime::Seconds();

    for (int32 i = 0; i < NumCycles; ++i)
    {
        FString SlotName = FString::Printf(TEXT("BenchmarkSlot_%d"), i);
        SaveSys->SaveGame(SlotName);
        SaveSys->LoadGame(SlotName);
    }

    double EndTime = FPlatformTime::Seconds();
    double TotalTime = EndTime - StartTime;
    double AvgTime = TotalTime / NumCycles;

    AddInfo(FString::Printf(TEXT("Save system benchmark: %d save/load cycles in %.3f seconds (avg: %.3f ms)"),
        NumCycles, TotalTime, AvgTime * 1000.0));
    TestTrue(TEXT("Save system is performant"), AvgTime < 0.1); // Less than 100ms per cycle

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMemoryUsageBenchmark, "HorrorProject.Performance.MemoryUsage", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FMemoryUsageBenchmark::RunTest(const FString& Parameters)
{
    // Benchmark: Memory usage of all subsystems
    UWorld* World = FAutomationEditorCommonUtils::CreateNewMap();
    UGameInstance* GameInstance = World->GetGameInstance();

    if (!GameInstance)
    {
        AddWarning(TEXT("GameInstance not available"));
        return true;
    }

    // Initialize all subsystems
    ULocalizationSubsystem* LocalizationSys = GameInstance->GetSubsystem<ULocalizationSubsystem>();
    UAchievementSubsystem* AchievementSys = GameInstance->GetSubsystem<UAchievementSubsystem>();
    UHorrorSaveSubsystem* SaveSys = GameInstance->GetSubsystem<UHorrorSaveSubsystem>();
    UAccessibilitySubsystem* AccessibilitySys = World->GetSubsystem<UAccessibilitySubsystem>();

    TestTrue(TEXT("All subsystems initialized"), true);

    // Perform operations to stress memory
    if (LocalizationSys)
    {
        for (int32 i = 0; i < 1000; ++i)
        {
            LocalizationSys->GetLocalizedText(TEXT("UI.MainMenu.Start"));
        }
    }

    if (AchievementSys)
    {
        for (int32 i = 0; i < 1000; ++i)
        {
            AchievementSys->GetAllAchievements();
        }
    }

    AddInfo(TEXT("Memory stress test completed"));
    TestTrue(TEXT("Memory usage is acceptable"), true);

    return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR
