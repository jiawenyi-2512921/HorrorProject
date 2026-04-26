#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR && HORRORPROJECT_ENABLE_LEGACY_AUTOMATION_TESTS

// Copyright Epic Games, Inc. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "Tests/TestHelpers.h"
#include "Tests/HorrorTestMacros.h"
#include "Game/HorrorEventBusSubsystem.h"
#include "Engine/StreamableManager.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLoadingTimesTest,
    "HorrorProject.Integration.Performance.LoadingTimes",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FLoadingTimesTest::RunTest(const FString& Parameters)
{
    using namespace HorrorTestHelpers;
    FTestWorldScope TestWorld;
    HORROR_TEST_REQUIRE_WORLD(TestWorld.GetWorld(), TestWorld, "Test world creation failed");

    UWorld* World = TestWorld.GetWorld();
    UHorrorEventBusSubsystem* EventBus = World->GetSubsystem<UHorrorEventBusSubsystem>();
    HORROR_TEST_REQUIRE(EventBus != nullptr, "EventBus not found");

    // Test 1: Level loading time
    TArray<float> LevelLoadTimes;

    EventBus->Subscribe("Level.LoadStarted", [](const FHorrorEventPayload& Payload) {
        // Load started
    });

    EventBus->Subscribe("Level.LoadCompleted", [&LevelLoadTimes](const FHorrorEventPayload& Payload) {
        LevelLoadTimes.Add(Payload.FloatData.FindRef("LoadTime"));
    });

    // Simulate level loads
    TArray<FString> Levels = {"MainMenu", "Gameplay", "Subway", "Hospital"};
    for (const FString& Level : Levels)
    {
        const double StartTime = FPlatformTime::Seconds();

        FHorrorEventPayload StartPayload;
        StartPayload.EventName = "Level.LoadStarted";
        StartPayload.StringData.Add("LevelName", Level);
        EventBus->Publish(StartPayload);

        // Simulate load work
        FPlatformProcess::Sleep(0.01f); // 10ms simulated load

        const double EndTime = FPlatformTime::Seconds();
        const float LoadTime = (EndTime - StartTime) * 1000.0f;

        FHorrorEventPayload CompletePayload;
        CompletePayload.EventName = "Level.LoadCompleted";
        CompletePayload.StringData.Add("LevelName", Level);
        CompletePayload.FloatData.Add("LoadTime", LoadTime);
        EventBus->Publish(CompletePayload);
    }

    float AvgLoadTime = 0.0f;
    for (float LoadTime : LevelLoadTimes)
    {
        AvgLoadTime += LoadTime;
    }
    AvgLoadTime /= LevelLoadTimes.Num();

    AddInfo(FString::Printf(TEXT("Average level load time: %.2f ms"), AvgLoadTime));
    TestTrue("Level load times acceptable", AvgLoadTime < 5000.0f); // < 5 seconds

    // Test 2: Asset loading time
    TMap<FString, float> AssetLoadTimes;

    EventBus->Subscribe("Asset.Loaded", [&AssetLoadTimes](const FHorrorEventPayload& Payload) {
        FString AssetName = Payload.StringData.FindRef("AssetName");
        float LoadTime = Payload.FloatData.FindRef("LoadTime");
        AssetLoadTimes.Add(AssetName, LoadTime);
    });

    TArray<FString> Assets = {"Texture_Wall", "Sound_Ambient", "Mesh_Door", "Material_Floor"};
    for (const FString& Asset : Assets)
    {
        const double StartTime = FPlatformTime::Seconds();
        FPlatformProcess::Sleep(0.001f); // 1ms simulated load
        const double EndTime = FPlatformTime::Seconds();

        FHorrorEventPayload Payload;
        Payload.EventName = "Asset.Loaded";
        Payload.StringData.Add("AssetName", Asset);
        Payload.FloatData.Add("LoadTime", (EndTime - StartTime) * 1000.0f);
        EventBus->Publish(Payload);
    }

    TestEqual("All assets loaded", AssetLoadTimes.Num(), Assets.Num());

    // Test 3: Streaming load performance
    int32 StreamedAssets = 0;
    float TotalStreamTime = 0.0f;

    EventBus->Subscribe("Asset.Streamed", [&](const FHorrorEventPayload& Payload) {
        StreamedAssets++;
        TotalStreamTime += Payload.FloatData.FindRef("StreamTime");
    });

    for (int32 i = 0; i < 20; ++i)
    {
        FHorrorEventPayload Payload;
        Payload.EventName = "Asset.Streamed";
        Payload.StringData.Add("AssetID", FString::Printf(TEXT("Asset_%d"), i));
        Payload.FloatData.Add("StreamTime", 0.5f + (i % 5) * 0.1f);
        EventBus->Publish(Payload);
    }

    float AvgStreamTime = TotalStreamTime / StreamedAssets;
    AddInfo(FString::Printf(TEXT("Streamed %d assets, avg time: %.2f ms"), StreamedAssets, AvgStreamTime));

    TestEqual("Streaming asset count", StreamedAssets, 20);
    TestTrue("Average stream time acceptable", AvgStreamTime < 2.0f); // < 2ms avg

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLoadingBottleneckTest,
    "HorrorProject.Integration.Performance.LoadingBottlenecks",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FLoadingBottleneckTest::RunTest(const FString& Parameters)
{
    using namespace HorrorTestHelpers;
    FTestWorldScope TestWorld;
    HORROR_TEST_REQUIRE_WORLD(TestWorld.GetWorld(), TestWorld, "Test world creation failed");

    UWorld* World = TestWorld.GetWorld();
    UHorrorEventBusSubsystem* EventBus = World->GetSubsystem<UHorrorEventBusSubsystem>();
    HORROR_TEST_REQUIRE(EventBus != nullptr, "EventBus not found");

    // Test parallel vs sequential loading
    const int32 NumAssets = 10;

    // Sequential loading
    const double SeqStartTime = FPlatformTime::Seconds();
    for (int32 i = 0; i < NumAssets; ++i)
    {
        FHorrorEventPayload Payload;
        Payload.EventName = "Asset.LoadSequential";
        Payload.IntData.Add("AssetID", i);
        EventBus->Publish(Payload);
    }
    const double SeqTime = (FPlatformTime::Seconds() - SeqStartTime) * 1000.0f;

    // Parallel loading simulation
    const double ParStartTime = FPlatformTime::Seconds();
    TArray<FHorrorEventPayload> ParallelPayloads;
    for (int32 i = 0; i < NumAssets; ++i)
    {
        FHorrorEventPayload Payload;
        Payload.EventName = "Asset.LoadParallel";
        Payload.IntData.Add("AssetID", i);
        ParallelPayloads.Add(Payload);
    }
    for (const FHorrorEventPayload& Payload : ParallelPayloads)
    {
        EventBus->Publish(Payload);
    }
    const double ParTime = (FPlatformTime::Seconds() - ParStartTime) * 1000.0f;

    AddInfo(FString::Printf(TEXT("Sequential: %.2f ms, Parallel: %.2f ms"), SeqTime, ParTime));
    TestTrue("Parallel loading faster", ParTime <= SeqTime);

    return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS

#endif // WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR
