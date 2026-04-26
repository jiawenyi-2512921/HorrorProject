// Copyright Epic Games, Inc. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "Tests/TestHelpers.h"
#include "Tests/HorrorTestMacros.h"
#include "Game/HorrorGameModeBase.h"
#include "Game/HorrorEventBusSubsystem.h"
#include "ProfilingDebugging/CsvProfiler.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFramerateStabilityTest,
    "HorrorProject.Integration.Performance.FramerateStability",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FFramerateStabilityTest::RunTest(const FString& Parameters)
{
    using namespace HorrorTestHelpers;
    FTestWorldScope TestWorld;
    HORROR_TEST_REQUIRE_WORLD(TestWorld.GetWorld(), TestWorld, "Test world creation failed");

    UWorld* World = TestWorld.GetWorld();
    UHorrorEventBusSubsystem* EventBus = World->GetSubsystem<UHorrorEventBusSubsystem>();
    HORROR_TEST_REQUIRE(EventBus != nullptr, "EventBus not found");

    // Test 1: Frame time consistency
    TArray<float> FrameTimes;
    const int32 NumFrames = 100;
    const float TargetFrameTime = 1.0f / 60.0f; // 60 FPS

    for (int32 i = 0; i < NumFrames; ++i)
    {
        const double StartTime = FPlatformTime::Seconds();

        // Simulate frame work
        FHorrorEventPayload Payload;
        Payload.EventName = "Frame.Tick";
        Payload.IntData.Add("FrameNumber", i);
        EventBus->Publish(Payload);

        const double EndTime = FPlatformTime::Seconds();
        FrameTimes.Add((EndTime - StartTime) * 1000.0f); // Convert to ms
    }

    // Calculate statistics
    float MinFrameTime = FLT_MAX;
    float MaxFrameTime = 0.0f;
    float AvgFrameTime = 0.0f;

    for (float FrameTime : FrameTimes)
    {
        MinFrameTime = FMath::Min(MinFrameTime, FrameTime);
        MaxFrameTime = FMath::Max(MaxFrameTime, FrameTime);
        AvgFrameTime += FrameTime;
    }
    AvgFrameTime /= NumFrames;

    // Calculate variance
    float Variance = 0.0f;
    for (float FrameTime : FrameTimes)
    {
        float Diff = FrameTime - AvgFrameTime;
        Variance += Diff * Diff;
    }
    Variance /= NumFrames;
    float StdDev = FMath::Sqrt(Variance);

    AddInfo(FString::Printf(TEXT("Frame Time Stats: Min=%.2fms, Max=%.2fms, Avg=%.2fms, StdDev=%.2fms"),
        MinFrameTime, MaxFrameTime, AvgFrameTime, StdDev));

    TestTrue("Frame time variance acceptable", StdDev < 5.0f); // < 5ms standard deviation
    TestTrue("Average frame time acceptable", AvgFrameTime < 20.0f); // < 20ms avg

    // Test 2: Frame drops detection
    int32 FrameDrops = 0;
    const float FrameDropThreshold = 33.0f; // 30 FPS threshold

    for (float FrameTime : FrameTimes)
    {
        if (FrameTime > FrameDropThreshold)
        {
            FrameDrops++;
        }
    }

    float FrameDropPercentage = (float)FrameDrops / NumFrames * 100.0f;
    AddInfo(FString::Printf(TEXT("Frame drops: %d (%.1f%%)"), FrameDrops, FrameDropPercentage));

    TestTrue("Frame drop rate acceptable", FrameDropPercentage < 5.0f); // < 5% drops

    // Test 3: Sustained load test
    const int32 SustainedFrames = 300; // 5 seconds at 60fps
    int32 StableFrames = 0;

    for (int32 i = 0; i < SustainedFrames; ++i)
    {
        const double StartTime = FPlatformTime::Seconds();

        FHorrorEventPayload Payload;
        Payload.EventName = "Frame.SustainedLoad";
        Payload.IntData.Add("FrameNumber", i);
        EventBus->Publish(Payload);

        const double EndTime = FPlatformTime::Seconds();
        float FrameTime = (EndTime - StartTime) * 1000.0f;

        if (FrameTime < TargetFrameTime * 1000.0f * 1.5f) // Within 150% of target
        {
            StableFrames++;
        }
    }

    float StabilityPercentage = (float)StableFrames / SustainedFrames * 100.0f;
    AddInfo(FString::Printf(TEXT("Sustained stability: %.1f%%"), StabilityPercentage));

    TestTrue("Sustained framerate stable", StabilityPercentage > 90.0f);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFramerateSpikeTest,
    "HorrorProject.Integration.Performance.FramerateSpikes",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FFramerateSpikeTest::RunTest(const FString& Parameters)
{
    using namespace HorrorTestHelpers;
    FTestWorldScope TestWorld;
    HORROR_TEST_REQUIRE_WORLD(TestWorld.GetWorld(), TestWorld, "Test world creation failed");

    UWorld* World = TestWorld.GetWorld();
    UHorrorEventBusSubsystem* EventBus = World->GetSubsystem<UHorrorEventBusSubsystem>();
    HORROR_TEST_REQUIRE(EventBus != nullptr, "EventBus not found");

    // Test spike detection during heavy events
    TArray<float> SpikeTimes;
    const int32 NumTests = 50;

    for (int32 i = 0; i < NumTests; ++i)
    {
        const double StartTime = FPlatformTime::Seconds();

        // Simulate heavy event load
        for (int32 j = 0; j < 100; ++j)
        {
            FHorrorEventPayload Payload;
            Payload.EventName = "Heavy.Event";
            Payload.IntData.Add("Index", j);
            EventBus->Publish(Payload);
        }

        const double EndTime = FPlatformTime::Seconds();
        SpikeTimes.Add((EndTime - StartTime) * 1000.0f);
    }

    // Find max spike
    float MaxSpike = 0.0f;
    for (float SpikeTime : SpikeTimes)
    {
        MaxSpike = FMath::Max(MaxSpike, SpikeTime);
    }

    AddInfo(FString::Printf(TEXT("Max spike time: %.2fms"), MaxSpike));
    TestTrue("Spike time acceptable", MaxSpike < 50.0f); // < 50ms max spike

    return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
