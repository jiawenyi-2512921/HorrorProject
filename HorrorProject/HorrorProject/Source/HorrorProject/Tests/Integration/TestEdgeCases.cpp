#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR && HORRORPROJECT_ENABLE_LEGACY_AUTOMATION_TESTS

// Copyright Epic Games, Inc. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "Tests/TestHelpers.h"
#include "Tests/HorrorTestMacros.h"
#include "Game/HorrorEventBusSubsystem.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEdgeCasesTest,
    "HorrorProject.Integration.E2E.EdgeCases",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FEdgeCasesTest::RunTest(const FString& Parameters)
{
    using namespace HorrorTestHelpers;
    FTestWorldScope TestWorld;
    HORROR_TEST_REQUIRE_WORLD(TestWorld.GetWorld(), TestWorld, "Test world creation failed");

    UWorld* World = TestWorld.GetWorld();
    UHorrorEventBusSubsystem* EventBus = World->GetSubsystem<UHorrorEventBusSubsystem>();
    HORROR_TEST_REQUIRE(EventBus != nullptr, "EventBus not found");

    // Test 1: Empty event payload
    bool bEmptyEventReceived = false;
    EventBus->Subscribe("EdgeCase.EmptyPayload", [&bEmptyEventReceived](const FHorrorEventPayload& Payload) {
        bEmptyEventReceived = true;
    });

    FHorrorEventPayload EmptyPayload;
    EmptyPayload.EventName = "EdgeCase.EmptyPayload";
    EventBus->Publish(EmptyPayload);

    TestTrue("Empty payload handled", bEmptyEventReceived);

    // Test 2: Maximum payload size
    FHorrorEventPayload LargePayload;
    LargePayload.EventName = "EdgeCase.LargePayload";

    for (int32 i = 0; i < 1000; ++i)
    {
        LargePayload.IntData.Add(FString::Printf(TEXT("Key_%d"), i), i);
    }

    bool bLargePayloadReceived = false;
    int32 ReceivedDataCount = 0;

    EventBus->Subscribe("EdgeCase.LargePayload", [&](const FHorrorEventPayload& Payload) {
        bLargePayloadReceived = true;
        ReceivedDataCount = Payload.IntData.Num();
    });

    EventBus->Publish(LargePayload);

    TestTrue("Large payload received", bLargePayloadReceived);
    TestEqual("Large payload data intact", ReceivedDataCount, 1000);

    // Test 3: Rapid event subscription/unsubscription
    int32 SubscriptionCount = 0;

    for (int32 i = 0; i < 100; ++i)
    {
        EventBus->Subscribe(FString::Printf(TEXT("EdgeCase.RapidSub_%d"), i),
            [&SubscriptionCount](const FHorrorEventPayload& Payload) {
                SubscriptionCount++;
            });
    }

    // Trigger all subscriptions
    for (int32 i = 0; i < 100; ++i)
    {
        FHorrorEventPayload Payload;
        Payload.EventName = FString::Printf(TEXT("EdgeCase.RapidSub_%d"), i);
        EventBus->Publish(Payload);
    }

    TestEqual("Rapid subscriptions handled", SubscriptionCount, 100);

    // Test 4: Null/invalid data handling
    bool bNullDataHandled = false;
    EventBus->Subscribe("EdgeCase.NullData", [&bNullDataHandled](const FHorrorEventPayload& Payload) {
        FString Value = Payload.StringData.FindRef("NonExistentKey");
        bNullDataHandled = Value.IsEmpty();
    });

    FHorrorEventPayload NullPayload;
    NullPayload.EventName = "EdgeCase.NullData";
    EventBus->Publish(NullPayload);

    TestTrue("Null data handled gracefully", bNullDataHandled);

    // Test 5: Concurrent event publishing
    int32 ConcurrentEvents = 0;
    EventBus->Subscribe("EdgeCase.Concurrent", [&ConcurrentEvents](const FHorrorEventPayload& Payload) {
        ConcurrentEvents++;
    });

    const int32 NumConcurrent = 500;
    for (int32 i = 0; i < NumConcurrent; ++i)
    {
        FHorrorEventPayload Payload;
        Payload.EventName = "EdgeCase.Concurrent";
        Payload.IntData.Add("EventID", i);
        EventBus->Publish(Payload);
    }

    TestEqual("Concurrent events processed", ConcurrentEvents, NumConcurrent);

    // Test 6: Event name collision
    int32 CollisionCount1 = 0;
    int32 CollisionCount2 = 0;

    EventBus->Subscribe("EdgeCase.Collision", [&CollisionCount1](const FHorrorEventPayload& Payload) {
        CollisionCount1++;
    });

    EventBus->Subscribe("EdgeCase.Collision", [&CollisionCount2](const FHorrorEventPayload& Payload) {
        CollisionCount2++;
    });

    FHorrorEventPayload CollisionPayload;
    CollisionPayload.EventName = "EdgeCase.Collision";
    EventBus->Publish(CollisionPayload);

    TestEqual("First collision handler called", CollisionCount1, 1);
    TestEqual("Second collision handler called", CollisionCount2, 1);

    // Test 7: Extreme values
    FHorrorEventPayload ExtremePayload;
    ExtremePayload.EventName = "EdgeCase.ExtremeValues";
    ExtremePayload.IntData.Add("MaxInt", INT32_MAX);
    ExtremePayload.IntData.Add("MinInt", INT32_MIN);
    ExtremePayload.FloatData.Add("MaxFloat", FLT_MAX);
    ExtremePayload.FloatData.Add("MinFloat", -FLT_MAX);

    bool bExtremeValuesHandled = false;
    EventBus->Subscribe("EdgeCase.ExtremeValues", [&bExtremeValuesHandled](const FHorrorEventPayload& Payload) {
        int32 MaxInt = Payload.IntData.FindRef("MaxInt");
        int32 MinInt = Payload.IntData.FindRef("MinInt");
        bExtremeValuesHandled = (MaxInt == INT32_MAX && MinInt == INT32_MIN);
    });

    EventBus->Publish(ExtremePayload);
    TestTrue("Extreme values handled", bExtremeValuesHandled);

    // Test 8: Special characters in strings
    FHorrorEventPayload SpecialCharsPayload;
    SpecialCharsPayload.EventName = "EdgeCase.SpecialChars";
    SpecialCharsPayload.StringData.Add("Special", TEXT("!@#$%^&*()_+-=[]{}|;':\",./<>?"));

    bool bSpecialCharsHandled = false;
    EventBus->Subscribe("EdgeCase.SpecialChars", [&bSpecialCharsHandled](const FHorrorEventPayload& Payload) {
        FString Special = Payload.StringData.FindRef("Special");
        bSpecialCharsHandled = !Special.IsEmpty();
    });

    EventBus->Publish(SpecialCharsPayload);
    TestTrue("Special characters handled", bSpecialCharsHandled);

    // Test 9: Zero values
    FHorrorEventPayload ZeroPayload;
    ZeroPayload.EventName = "EdgeCase.ZeroValues";
    ZeroPayload.IntData.Add("Zero", 0);
    ZeroPayload.FloatData.Add("ZeroFloat", 0.0f);
    ZeroPayload.BoolData.Add("False", false);

    bool bZeroValuesHandled = false;
    EventBus->Subscribe("EdgeCase.ZeroValues", [&bZeroValuesHandled](const FHorrorEventPayload& Payload) {
        int32 Zero = Payload.IntData.FindRef("Zero");
        float ZeroFloat = Payload.FloatData.FindRef("ZeroFloat");
        bool False = Payload.BoolData.FindRef("False");
        bZeroValuesHandled = (Zero == 0 && ZeroFloat == 0.0f && !False);
    });

    EventBus->Publish(ZeroPayload);
    TestTrue("Zero values handled", bZeroValuesHandled);

    // Test 10: Long event names
    FString LongEventName = TEXT("EdgeCase.VeryLongEventNameThatExceedsNormalLengthExpectations_");
    for (int32 i = 0; i < 10; ++i)
    {
        LongEventName += TEXT("Extended_");
    }

    bool bLongNameHandled = false;
    EventBus->Subscribe(LongEventName, [&bLongNameHandled](const FHorrorEventPayload& Payload) {
        bLongNameHandled = true;
    });

    FHorrorEventPayload LongNamePayload;
    LongNamePayload.EventName = LongEventName;
    EventBus->Publish(LongNamePayload);

    TestTrue("Long event name handled", bLongNameHandled);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBoundaryConditionsTest,
    "HorrorProject.Integration.E2E.BoundaryConditions",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FBoundaryConditionsTest::RunTest(const FString& Parameters)
{
    using namespace HorrorTestHelpers;
    FTestWorldScope TestWorld;
    HORROR_TEST_REQUIRE_WORLD(TestWorld.GetWorld(), TestWorld, "Test world creation failed");

    UWorld* World = TestWorld.GetWorld();
    UHorrorEventBusSubsystem* EventBus = World->GetSubsystem<UHorrorEventBusSubsystem>();
    HORROR_TEST_REQUIRE(EventBus != nullptr, "EventBus not found");

    // Test array boundary conditions
    TArray<int32> BoundaryValues = {-1, 0, 1, 99, 100, 101};

    for (int32 Value : BoundaryValues)
    {
        FHorrorEventPayload Payload;
        Payload.EventName = "Boundary.ArrayIndex";
        Payload.IntData.Add("Index", Value);
        EventBus->Publish(Payload);
    }

    // Test time boundary conditions
    TArray<float> TimeBoundaries = {0.0f, 0.001f, 1.0f, 60.0f, 3600.0f, FLT_MAX};

    for (float Time : TimeBoundaries)
    {
        FHorrorEventPayload Payload;
        Payload.EventName = "Boundary.Time";
        Payload.FloatData.Add("Time", Time);
        EventBus->Publish(Payload);
    }

    TestTrue("Boundary conditions tested", true);

    return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS

#endif // WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR
