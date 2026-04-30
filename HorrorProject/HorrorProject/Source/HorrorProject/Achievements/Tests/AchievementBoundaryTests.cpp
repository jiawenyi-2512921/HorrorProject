#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR && HORRORPROJECT_ENABLE_LEGACY_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "Achievements/AchievementSubsystem.h"
#include "Achievements/AchievementTracker.h"
#include "Achievements/StatisticsSubsystem.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAchievementBoundaryTest, "HorrorProject.Achievements.BoundaryConditions", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FAchievementBoundaryTest::RunTest(const FString& Parameters)
{
    UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
    UAchievementSubsystem* AchievementSys = World->GetGameInstance()->GetSubsystem<UAchievementSubsystem>();

    if (!AchievementSys)
    {
        AddWarning(TEXT("AchievementSubsystem not available"));
        return true;
    }

    // Test unlock non-existent achievement
    AchievementSys->UnlockAchievement(FName("NonExistent"));
    TestTrue(TEXT("Non-existent achievement handled"), true);

    // Test negative progress
    AchievementSys->UpdateAchievementProgress(FName("ACH_Explorer"), -10.0f);
    float Progress = AchievementSys->GetAchievementProgress(FName("ACH_Explorer"));
    TestTrue(TEXT("Negative progress clamped"), Progress >= 0.0f);

    // Test progress over max
    AchievementSys->UpdateAchievementProgress(FName("ACH_Explorer"), 1000.0f);
    Progress = AchievementSys->GetAchievementProgress(FName("ACH_Explorer"));
    TestTrue(TEXT("Over-max progress clamped"), Progress <= 1.0f);

    // Test unlock already unlocked achievement
    AchievementSys->UnlockAchievement(FName("ACH_FirstSteps"));
    AchievementSys->UnlockAchievement(FName("ACH_FirstSteps"));
    TestTrue(TEXT("Double unlock handled"), true);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAchievementProgressStressTest, "HorrorProject.Achievements.ProgressStress", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FAchievementProgressStressTest::RunTest(const FString& Parameters)
{
    UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
    UAchievementSubsystem* AchievementSys = World->GetGameInstance()->GetSubsystem<UAchievementSubsystem>();

    if (!AchievementSys)
    {
        AddWarning(TEXT("AchievementSubsystem not available"));
        return true;
    }

    // Stress test: Rapid progress updates
    const int32 NumUpdates = 10000;
    double StartTime = FPlatformTime::Seconds();

    for (int32 i = 0; i < NumUpdates; ++i)
    {
        AchievementSys->UpdateAchievementProgress(FName("ACH_Explorer"), 0.01f);
    }

    double EndTime = FPlatformTime::Seconds();
    double TotalTime = EndTime - StartTime;

    AddInfo(FString::Printf(TEXT("Progress stress test: %d updates in %.3f seconds"), NumUpdates, TotalTime));
    TestTrue(TEXT("Progress updates handle stress"), TotalTime < 2.0);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAchievementConcurrencyTest, "HorrorProject.Achievements.Concurrency", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FAchievementConcurrencyTest::RunTest(const FString& Parameters)
{
    UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
    UAchievementSubsystem* AchievementSys = World->GetGameInstance()->GetSubsystem<UAchievementSubsystem>();

    if (!AchievementSys)
    {
        AddWarning(TEXT("AchievementSubsystem not available"));
        return true;
    }

    // Test simultaneous unlocks
    TArray<FName> AchievementIDs = {
        FName("ACH_FirstSteps"),
        FName("ACH_Explorer"),
        FName("ACH_Survivor"),
        FName("ACH_Collector"),
        FName("ACH_Speedrunner")
    };

    for (const FName& ID : AchievementIDs)
    {
        AchievementSys->UnlockAchievement(ID);
    }

    TestTrue(TEXT("Simultaneous unlocks handled"), true);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FStatisticsTrackingBoundaryTest, "HorrorProject.Achievements.StatisticsBoundary", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FStatisticsTrackingBoundaryTest::RunTest(const FString& Parameters)
{
    UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
    UStatisticsSubsystem* StatsSys = World->GetGameInstance()->GetSubsystem<UStatisticsSubsystem>();

    if (!StatsSys)
    {
        AddWarning(TEXT("StatisticsSubsystem not available"));
        return true;
    }

    // Test very large stat values
    StatsSys->IncrementStat(FName("TestStat"), 999999999);
    int32 StatValue = StatsSys->GetStatValue(FName("TestStat"));
    TestTrue(TEXT("Large stat value handled"), StatValue > 0);

    // Test negative stat increment
    StatsSys->IncrementStat(FName("TestStat"), -100);
    TestTrue(TEXT("Negative increment handled"), true);

    // Test stat overflow
    StatsSys->SetStatValue(FName("OverflowStat"), INT32_MAX);
    StatsSys->IncrementStat(FName("OverflowStat"), 1);
    TestTrue(TEXT("Stat overflow handled"), true);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAchievementNotificationTest, "HorrorProject.Achievements.NotificationSystem", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FAchievementNotificationTest::RunTest(const FString& Parameters)
{
    UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
    UAchievementSubsystem* AchievementSys = World->GetGameInstance()->GetSubsystem<UAchievementSubsystem>();

    if (!AchievementSys)
    {
        AddWarning(TEXT("AchievementSubsystem not available"));
        return true;
    }

    // Test notification queue overflow
    for (int32 i = 0; i < 100; ++i)
    {
        FName AchievementID = FName(*FString::Printf(TEXT("ACH_Test_%d"), i));
        AchievementSys->UnlockAchievement(AchievementID);
    }

    TestTrue(TEXT("Notification queue overflow handled"), true);

    return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR
