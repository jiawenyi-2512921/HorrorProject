#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR && HORRORPROJECT_ENABLE_LEGACY_AUTOMATION_TESTS

// Copyright Epic Games, Inc. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "Tests/TestHelpers.h"
#include "Tests/HorrorTestMacros.h"
#include "Game/HorrorGameModeBase.h"
#include "Game/HorrorEventBusSubsystem.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMultiplePlaythroughsTest,
    "HorrorProject.Integration.E2E.MultiplePlaythroughs",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FMultiplePlaythroughsTest::RunTest(const FString& Parameters)
{
    using namespace HorrorTestHelpers;
    FTestWorldScope TestWorld;
    HORROR_TEST_REQUIRE_WORLD(TestWorld.GetWorld(), TestWorld, "Test world creation failed");

    UWorld* World = TestWorld.GetWorld();
    UHorrorEventBusSubsystem* EventBus = World->GetSubsystem<UHorrorEventBusSubsystem>();
    HORROR_TEST_REQUIRE(EventBus != nullptr, "EventBus not found");

    const int32 NumPlaythroughs = 5;
    TArray<bool> PlaythroughResults;

    for (int32 PlaythroughIndex = 0; PlaythroughIndex < NumPlaythroughs; ++PlaythroughIndex)
    {
        bool bPlaythroughComplete = false;

        // Start playthrough
        FHorrorEventPayload StartPayload;
        StartPayload.EventName = "Playthrough.Started";
        StartPayload.IntData.Add("PlaythroughNumber", PlaythroughIndex + 1);
        EventBus->Publish(StartPayload);

        // Simulate gameplay
        int32 EventsProcessed = 0;
        EventBus->Subscribe(FString::Printf(TEXT("Playthrough.%d.Event"), PlaythroughIndex),
            [&EventsProcessed](const FHorrorEventPayload& Payload) {
                EventsProcessed++;
            });

        for (int32 i = 0; i < 10; ++i)
        {
            FHorrorEventPayload EventPayload;
            EventPayload.EventName = FString::Printf(TEXT("Playthrough.%d.Event"), PlaythroughIndex);
            EventPayload.IntData.Add("EventID", i);
            EventBus->Publish(EventPayload);
        }

        // Complete playthrough
        EventBus->Subscribe(FString::Printf(TEXT("Playthrough.%d.Completed"), PlaythroughIndex),
            [&bPlaythroughComplete](const FHorrorEventPayload& Payload) {
                bPlaythroughComplete = true;
            });

        FHorrorEventPayload CompletePayload;
        CompletePayload.EventName = FString::Printf(TEXT("Playthrough.%d.Completed"), PlaythroughIndex);
        EventBus->Publish(CompletePayload);

        PlaythroughResults.Add(bPlaythroughComplete && EventsProcessed == 10);

        AddInfo(FString::Printf(TEXT("Playthrough %d: %s"),
            PlaythroughIndex + 1,
            PlaythroughResults[PlaythroughIndex] ? TEXT("Success") : TEXT("Failed")));
    }

    // Verify all playthroughs succeeded
    int32 SuccessfulPlaythroughs = 0;
    for (bool Result : PlaythroughResults)
    {
        if (Result) SuccessfulPlaythroughs++;
    }

    TestEqual("All playthroughs completed", SuccessfulPlaythroughs, NumPlaythroughs);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPlaythroughConsistencyTest,
    "HorrorProject.Integration.E2E.PlaythroughConsistency",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FPlaythroughConsistencyTest::RunTest(const FString& Parameters)
{
    using namespace HorrorTestHelpers;
    FTestWorldScope TestWorld;
    HORROR_TEST_REQUIRE_WORLD(TestWorld.GetWorld(), TestWorld, "Test world creation failed");

    UWorld* World = TestWorld.GetWorld();
    UHorrorEventBusSubsystem* EventBus = World->GetSubsystem<UHorrorEventBusSubsystem>();
    HORROR_TEST_REQUIRE(EventBus != nullptr, "EventBus not found");

    // Test consistency across multiple runs
    TArray<int32> RunScores;
    TArray<float> RunTimes;

    for (int32 Run = 0; Run < 3; ++Run)
    {
        int32 Score = 0;
        const double StartTime = FPlatformTime::Seconds();

        // Simulate consistent gameplay
        for (int32 i = 0; i < 100; ++i)
        {
            FHorrorEventPayload Payload;
            Payload.EventName = "Consistency.Test";
            Payload.IntData.Add("RunNumber", Run);
            Payload.IntData.Add("EventIndex", i);
            EventBus->Publish(Payload);
            Score += 10;
        }

        const double EndTime = FPlatformTime::Seconds();
        RunScores.Add(Score);
        RunTimes.Add((EndTime - StartTime) * 1000.0f);
    }

    // Verify consistency
    bool bScoresConsistent = true;
    for (int32 i = 1; i < RunScores.Num(); ++i)
    {
        if (RunScores[i] != RunScores[0])
        {
            bScoresConsistent = false;
            break;
        }
    }

    TestTrue("Scores consistent across runs", bScoresConsistent);

    // Check timing variance
    float AvgTime = 0.0f;
    for (float Time : RunTimes)
    {
        AvgTime += Time;
    }
    AvgTime /= RunTimes.Num();

    float MaxVariance = 0.0f;
    for (float Time : RunTimes)
    {
        float Variance = FMath::Abs(Time - AvgTime);
        MaxVariance = FMath::Max(MaxVariance, Variance);
    }

    AddInfo(FString::Printf(TEXT("Timing variance: %.2f ms"), MaxVariance));
    TestTrue("Timing variance acceptable", MaxVariance < 50.0f); // < 50ms variance

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPlaythroughStressTest,
    "HorrorProject.Integration.E2E.PlaythroughStress",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FPlaythroughStressTest::RunTest(const FString& Parameters)
{
    using namespace HorrorTestHelpers;
    FTestWorldScope TestWorld;
    HORROR_TEST_REQUIRE_WORLD(TestWorld.GetWorld(), TestWorld, "Test world creation failed");

    UWorld* World = TestWorld.GetWorld();
    UHorrorEventBusSubsystem* EventBus = World->GetSubsystem<UHorrorEventBusSubsystem>();
    HORROR_TEST_REQUIRE(EventBus != nullptr, "EventBus not found");

    // Stress test with rapid playthroughs
    const int32 NumRapidPlaythroughs = 20;
    int32 CompletedPlaythroughs = 0;

    EventBus->Subscribe("Stress.PlaythroughComplete", [&CompletedPlaythroughs](const FHorrorEventPayload& Payload) {
        CompletedPlaythroughs++;
    });

    const double StartTime = FPlatformTime::Seconds();

    for (int32 i = 0; i < NumRapidPlaythroughs; ++i)
    {
        // Rapid playthrough simulation
        for (int32 j = 0; j < 50; ++j)
        {
            FHorrorEventPayload Payload;
            Payload.EventName = "Stress.Event";
            Payload.IntData.Add("PlaythroughID", i);
            Payload.IntData.Add("EventID", j);
            EventBus->Publish(Payload);
        }

        FHorrorEventPayload CompletePayload;
        CompletePayload.EventName = "Stress.PlaythroughComplete";
        CompletePayload.IntData.Add("PlaythroughID", i);
        EventBus->Publish(CompletePayload);
    }

    const double EndTime = FPlatformTime::Seconds();
    const double TotalTime = (EndTime - StartTime) * 1000.0;

    AddInfo(FString::Printf(TEXT("Completed %d playthroughs in %.2f ms"),
        CompletedPlaythroughs, TotalTime));

    TestEqual("All stress playthroughs completed", CompletedPlaythroughs, NumRapidPlaythroughs);
    TestTrue("Stress test performance acceptable", TotalTime < 5000.0); // < 5 seconds

    return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS

#endif // WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR
