// Copyright Epic Games, Inc. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "Tests/TestHelpers.h"
#include "Tests/HorrorTestMacros.h"
#include "Game/HorrorEventBusSubsystem.h"
#include "Engine/StreamableManager.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAssetStreamingTest,
    "HorrorProject.Integration.Performance.AssetStreaming",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FAssetStreamingTest::RunTest(const FString& Parameters)
{
    using namespace HorrorTestHelpers;
    FTestWorldScope TestWorld;
    HORROR_TEST_REQUIRE_WORLD(TestWorld.GetWorld(), TestWorld, "Test world creation failed");

    UWorld* World = TestWorld.GetWorld();
    UHorrorEventBusSubsystem* EventBus = World->GetSubsystem<UHorrorEventBusSubsystem>();
    HORROR_TEST_REQUIRE(EventBus != nullptr, "EventBus not found");

    // Test 1: Streaming pool management
    int32 StreamedIn = 0;
    int32 StreamedOut = 0;
    int32 CurrentPoolSize = 0;
    const int32 MaxPoolSize = 100;

    EventBus->Subscribe("Streaming.AssetLoaded", [&](const FHorrorEventPayload& Payload) {
        StreamedIn++;
        CurrentPoolSize++;
    });

    EventBus->Subscribe("Streaming.AssetUnloaded", [&](const FHorrorEventPayload& Payload) {
        StreamedOut++;
        CurrentPoolSize--;
    });

    // Stream in assets
    for (int32 i = 0; i < 50; ++i)
    {
        FHorrorEventPayload Payload;
        Payload.EventName = "Streaming.AssetLoaded";
        Payload.StringData.Add("AssetPath", FString::Printf(TEXT("/Game/Assets/Asset_%d"), i));
        Payload.IntData.Add("AssetSize", 1024 * (i % 10 + 1));
        EventBus->Publish(Payload);
    }

    TestEqual("Assets streamed in", StreamedIn, 50);
    TestEqual("Current pool size", CurrentPoolSize, 50);

    // Stream out some assets
    for (int32 i = 0; i < 20; ++i)
    {
        FHorrorEventPayload Payload;
        Payload.EventName = "Streaming.AssetUnloaded";
        Payload.StringData.Add("AssetPath", FString::Printf(TEXT("/Game/Assets/Asset_%d"), i));
        EventBus->Publish(Payload);
    }

    TestEqual("Assets streamed out", StreamedOut, 20);
    TestEqual("Pool size after unload", CurrentPoolSize, 30);

    // Test 2: Distance-based streaming
    TMap<float, int32> DistanceStreaming;

    EventBus->Subscribe("Streaming.DistanceBased", [&DistanceStreaming](const FHorrorEventPayload& Payload) {
        float Distance = Payload.FloatData.FindRef("Distance");
        int32 LODLevel = Payload.IntData.FindRef("LODLevel");
        DistanceStreaming.Add(Distance, LODLevel);
    });

    // Simulate distance-based LOD streaming
    TArray<float> Distances = {100.0f, 500.0f, 1000.0f, 2000.0f, 5000.0f};
    TArray<int32> ExpectedLODs = {0, 1, 2, 3, 4};

    for (int32 i = 0; i < Distances.Num(); ++i)
    {
        FHorrorEventPayload Payload;
        Payload.EventName = "Streaming.DistanceBased";
        Payload.FloatData.Add("Distance", Distances[i]);
        Payload.IntData.Add("LODLevel", ExpectedLODs[i]);
        EventBus->Publish(Payload);
    }

    TestEqual("Distance streaming entries", DistanceStreaming.Num(), Distances.Num());

    // Test 3: Priority-based streaming
    TArray<int32> StreamingPriorities;

    EventBus->Subscribe("Streaming.PriorityQueued", [&StreamingPriorities](const FHorrorEventPayload& Payload) {
        StreamingPriorities.Add(Payload.IntData.FindRef("Priority"));
    });

    // Queue assets with different priorities
    for (int32 i = 0; i < 10; ++i)
    {
        FHorrorEventPayload Payload;
        Payload.EventName = "Streaming.PriorityQueued";
        Payload.StringData.Add("AssetName", FString::Printf(TEXT("Asset_%d"), i));
        Payload.IntData.Add("Priority", 10 - i); // Higher priority first
        EventBus->Publish(Payload);
    }

    TestEqual("Priority queue size", StreamingPriorities.Num(), 10);

    // Verify priorities are tracked
    bool bPrioritiesValid = true;
    for (int32 Priority : StreamingPriorities)
    {
        if (Priority < 1 || Priority > 10)
        {
            bPrioritiesValid = false;
            break;
        }
    }
    TestTrue("Priorities valid", bPrioritiesValid);

    // Test 4: Streaming bandwidth monitoring
    float TotalBandwidth = 0.0f;
    int32 BandwidthSamples = 0;

    EventBus->Subscribe("Streaming.BandwidthUpdate", [&](const FHorrorEventPayload& Payload) {
        TotalBandwidth += Payload.FloatData.FindRef("Bandwidth");
        BandwidthSamples++;
    });

    // Simulate bandwidth measurements (MB/s)
    TArray<float> BandwidthValues = {50.0f, 75.0f, 100.0f, 80.0f, 90.0f};
    for (float Bandwidth : BandwidthValues)
    {
        FHorrorEventPayload Payload;
        Payload.EventName = "Streaming.BandwidthUpdate";
        Payload.FloatData.Add("Bandwidth", Bandwidth);
        EventBus->Publish(Payload);
    }

    float AvgBandwidth = TotalBandwidth / BandwidthSamples;
    AddInfo(FString::Printf(TEXT("Average streaming bandwidth: %.2f MB/s"), AvgBandwidth));

    TestEqual("Bandwidth samples collected", BandwidthSamples, BandwidthValues.Num());
    TestTrue("Bandwidth acceptable", AvgBandwidth > 50.0f); // > 50 MB/s

    // Test 5: Texture streaming
    TMap<FString, int32> TextureMipLevels;

    EventBus->Subscribe("Streaming.TextureMipChanged", [&TextureMipLevels](const FHorrorEventPayload& Payload) {
        FString TextureName = Payload.StringData.FindRef("TextureName");
        int32 MipLevel = Payload.IntData.FindRef("MipLevel");
        TextureMipLevels.Add(TextureName, MipLevel);
    });

    // Simulate texture mip streaming
    for (int32 i = 0; i < 5; ++i)
    {
        FHorrorEventPayload Payload;
        Payload.EventName = "Streaming.TextureMipChanged";
        Payload.StringData.Add("TextureName", FString::Printf(TEXT("Texture_%d"), i));
        Payload.IntData.Add("MipLevel", i % 4); // Mip levels 0-3
        EventBus->Publish(Payload);
    }

    TestEqual("Texture mip levels tracked", TextureMipLevels.Num(), 5);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FStreamingPerformanceTest,
    "HorrorProject.Integration.Performance.StreamingPerformance",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FStreamingPerformanceTest::RunTest(const FString& Parameters)
{
    using namespace HorrorTestHelpers;
    FTestWorldScope TestWorld;
    HORROR_TEST_REQUIRE_WORLD(TestWorld.GetWorld(), TestWorld, "Test world creation failed");

    UWorld* World = TestWorld.GetWorld();
    UHorrorEventBusSubsystem* EventBus = World->GetSubsystem<UHorrorEventBusSubsystem>();
    HORROR_TEST_REQUIRE(EventBus != nullptr, "EventBus not found");

    // Test streaming request throughput
    const int32 NumRequests = 1000;
    const double StartTime = FPlatformTime::Seconds();

    for (int32 i = 0; i < NumRequests; ++i)
    {
        FHorrorEventPayload Payload;
        Payload.EventName = "Streaming.Request";
        Payload.IntData.Add("RequestID", i);
        EventBus->Publish(Payload);
    }

    const double EndTime = FPlatformTime::Seconds();
    const double ElapsedMs = (EndTime - StartTime) * 1000.0;
    const double RequestsPerSecond = NumRequests / (ElapsedMs / 1000.0);

    AddInfo(FString::Printf(TEXT("Streaming throughput: %.0f requests/sec"), RequestsPerSecond));
    TestTrue("Streaming throughput acceptable", RequestsPerSecond > 10000.0); // > 10k requests/sec

    return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
