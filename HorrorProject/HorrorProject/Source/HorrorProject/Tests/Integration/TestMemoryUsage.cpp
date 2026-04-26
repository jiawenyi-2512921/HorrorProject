// Copyright Epic Games, Inc. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "Tests/TestHelpers.h"
#include "Tests/HorrorTestMacros.h"
#include "Game/HorrorGameModeBase.h"
#include "Game/HorrorEventBusSubsystem.h"
#include "HAL/PlatformMemory.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMemoryUsageTest,
    "HorrorProject.Integration.Performance.MemoryUsage",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FMemoryUsageTest::RunTest(const FString& Parameters)
{
    using namespace HorrorTestHelpers;
    FTestWorldScope TestWorld;
    HORROR_TEST_REQUIRE_WORLD(TestWorld.GetWorld(), TestWorld, "Test world creation failed");

    UWorld* World = TestWorld.GetWorld();
    UHorrorEventBusSubsystem* EventBus = World->GetSubsystem<UHorrorEventBusSubsystem>();
    HORROR_TEST_REQUIRE(EventBus != nullptr, "EventBus not found");

    // Test 1: Baseline memory usage
    FPlatformMemoryStats BaselineStats = FPlatformMemory::GetStats();
    const uint64 BaselineUsedPhysical = BaselineStats.UsedPhysical;

    AddInfo(FString::Printf(TEXT("Baseline memory: %.2f MB"),
        BaselineUsedPhysical / (1024.0 * 1024.0)));

    // Test 2: Memory usage during gameplay
    AHorrorGameModeBase* GameMode = TestWorld.SpawnActor<AHorrorGameModeBase>();
    HORROR_TEST_REQUIRE(GameMode != nullptr, "GameMode spawn failed");

    // Simulate gameplay activity
    for (int32 i = 0; i < 1000; ++i)
    {
        FHorrorEventPayload Payload;
        Payload.EventName = "Gameplay.Activity";
        Payload.IntData.Add("ActivityID", i);
        EventBus->Publish(Payload);
    }

    FPlatformMemoryStats GameplayStats = FPlatformMemory::GetStats();
    const uint64 GameplayUsedPhysical = GameplayStats.UsedPhysical;
    const int64 MemoryDelta = GameplayUsedPhysical - BaselineUsedPhysical;

    AddInfo(FString::Printf(TEXT("Gameplay memory: %.2f MB (delta: %.2f MB)"),
        GameplayUsedPhysical / (1024.0 * 1024.0),
        MemoryDelta / (1024.0 * 1024.0)));

    TestTrue("Memory increase reasonable", MemoryDelta < 100 * 1024 * 1024); // < 100MB increase

    // Test 3: Memory leak detection
    TArray<uint64> MemorySamples;
    const int32 NumSamples = 10;

    for (int32 i = 0; i < NumSamples; ++i)
    {
        // Perform operations
        for (int32 j = 0; j < 100; ++j)
        {
            FHorrorEventPayload Payload;
            Payload.EventName = "Memory.Test";
            Payload.IntData.Add("Iteration", j);
            EventBus->Publish(Payload);
        }

        FPlatformMemoryStats CurrentStats = FPlatformMemory::GetStats();
        MemorySamples.Add(CurrentStats.UsedPhysical);
    }

    // Check for consistent memory growth (potential leak)
    bool bConsistentGrowth = true;
    for (int32 i = 1; i < NumSamples; ++i)
    {
        if (MemorySamples[i] <= MemorySamples[i - 1])
        {
            bConsistentGrowth = false;
            break;
        }
    }

    TestFalse("No consistent memory growth detected", bConsistentGrowth);

    // Test 4: Peak memory usage
    uint64 PeakMemory = 0;
    for (int32 i = 0; i < 100; ++i)
    {
        // Create temporary objects
        TArray<FHorrorEventPayload> TempPayloads;
        for (int32 j = 0; j < 50; ++j)
        {
            FHorrorEventPayload Payload;
            Payload.EventName = "Peak.Test";
            Payload.IntData.Add("Index", j);
            TempPayloads.Add(Payload);
        }

        FPlatformMemoryStats CurrentStats = FPlatformMemory::GetStats();
        PeakMemory = FMath::Max(PeakMemory, CurrentStats.UsedPhysical);
    }

    AddInfo(FString::Printf(TEXT("Peak memory: %.2f MB"),
        PeakMemory / (1024.0 * 1024.0)));

    const int64 PeakDelta = PeakMemory - BaselineUsedPhysical;
    TestTrue("Peak memory acceptable", PeakDelta < 200 * 1024 * 1024); // < 200MB peak

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMemoryAllocationTest,
    "HorrorProject.Integration.Performance.MemoryAllocation",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FMemoryAllocationTest::RunTest(const FString& Parameters)
{
    using namespace HorrorTestHelpers;
    FTestWorldScope TestWorld;
    HORROR_TEST_REQUIRE_WORLD(TestWorld.GetWorld(), TestWorld, "Test world creation failed");

    UWorld* World = TestWorld.GetWorld();
    UHorrorEventBusSubsystem* EventBus = World->GetSubsystem<UHorrorEventBusSubsystem>();
    HORROR_TEST_REQUIRE(EventBus != nullptr, "EventBus not found");

    // Test allocation performance
    const int32 NumAllocations = 1000;
    const double StartTime = FPlatformTime::Seconds();

    TArray<FHorrorEventPayload*> Allocations;
    for (int32 i = 0; i < NumAllocations; ++i)
    {
        FHorrorEventPayload* Payload = new FHorrorEventPayload();
        Payload->EventName = "Allocation.Test";
        Payload->IntData.Add("Index", i);
        Allocations.Add(Payload);
    }

    const double AllocTime = FPlatformTime::Seconds() - StartTime;

    // Cleanup
    for (FHorrorEventPayload* Payload : Allocations)
    {
        delete Payload;
    }

    const double TotalTime = FPlatformTime::Seconds() - StartTime;
    const double AvgAllocTime = (AllocTime / NumAllocations) * 1000000.0; // microseconds

    AddInfo(FString::Printf(TEXT("Allocation performance: %.2f μs/alloc (total: %.2f ms)"),
        AvgAllocTime, TotalTime * 1000.0));

    TestTrue("Allocation performance acceptable", AvgAllocTime < 10.0); // < 10μs per allocation

    return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
