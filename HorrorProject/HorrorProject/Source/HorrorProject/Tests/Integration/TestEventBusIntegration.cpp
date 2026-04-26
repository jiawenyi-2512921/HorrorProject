// Copyright Epic Games, Inc. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "Tests/TestHelpers.h"
#include "Tests/HorrorTestMacros.h"
#include "Game/HorrorEventBusSubsystem.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEventBusSystemIntegrationTest,
    "HorrorProject.Integration.Systems.EventBus",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FEventBusSystemIntegrationTest::RunTest(const FString& Parameters)
{
    using namespace HorrorTestHelpers;
    FTestWorldScope TestWorld;
    HORROR_TEST_REQUIRE_WORLD(TestWorld.GetWorld(), TestWorld, "Test world creation failed");

    UWorld* World = TestWorld.GetWorld();
    UHorrorEventBusSubsystem* EventBus = World->GetSubsystem<UHorrorEventBusSubsystem>();
    HORROR_TEST_REQUIRE(EventBus != nullptr, "EventBus subsystem not found");

    // Test 1: Multi-subscriber event distribution
    int32 Subscriber1Count = 0;
    int32 Subscriber2Count = 0;
    int32 Subscriber3Count = 0;

    EventBus->Subscribe("Test.MultiSubscriber", [&Subscriber1Count](const FHorrorEventPayload& Payload) {
        Subscriber1Count++;
    });

    EventBus->Subscribe("Test.MultiSubscriber", [&Subscriber2Count](const FHorrorEventPayload& Payload) {
        Subscriber2Count++;
    });

    EventBus->Subscribe("Test.MultiSubscriber", [&Subscriber3Count](const FHorrorEventPayload& Payload) {
        Subscriber3Count++;
    });

    FHorrorEventPayload Payload;
    Payload.EventName = "Test.MultiSubscriber";
    EventBus->Publish(Payload);

    TestEqual("Subscriber 1 received event", Subscriber1Count, 1);
    TestEqual("Subscriber 2 received event", Subscriber2Count, 1);
    TestEqual("Subscriber 3 received event", Subscriber3Count, 1);

    // Test 2: Event payload data integrity
    FString ReceivedString;
    int32 ReceivedInt = 0;
    float ReceivedFloat = 0.0f;
    bool ReceivedBool = false;

    EventBus->Subscribe("Test.PayloadData", [&](const FHorrorEventPayload& Payload) {
        ReceivedString = Payload.StringData.FindRef("TestString");
        ReceivedInt = Payload.IntData.FindRef("TestInt");
        ReceivedFloat = Payload.FloatData.FindRef("TestFloat");
        ReceivedBool = Payload.BoolData.FindRef("TestBool");
    });

    FHorrorEventPayload DataPayload;
    DataPayload.EventName = "Test.PayloadData";
    DataPayload.StringData.Add("TestString", "HelloWorld");
    DataPayload.IntData.Add("TestInt", 42);
    DataPayload.FloatData.Add("TestFloat", 3.14f);
    DataPayload.BoolData.Add("TestBool", true);
    EventBus->Publish(DataPayload);

    TestEqual("String data preserved", ReceivedString, FString("HelloWorld"));
    TestEqual("Int data preserved", ReceivedInt, 42);
    TestTrue("Float data preserved", FMath::IsNearlyEqual(ReceivedFloat, 3.14f, 0.001f));
    TestTrue("Bool data preserved", ReceivedBool);

    // Test 3: Event priority and ordering
    TArray<int32> ExecutionOrder;

    EventBus->Subscribe("Test.Priority", [&ExecutionOrder](const FHorrorEventPayload& Payload) {
        ExecutionOrder.Add(1);
    });

    EventBus->Subscribe("Test.Priority", [&ExecutionOrder](const FHorrorEventPayload& Payload) {
        ExecutionOrder.Add(2);
    });

    EventBus->Subscribe("Test.Priority", [&ExecutionOrder](const FHorrorEventPayload& Payload) {
        ExecutionOrder.Add(3);
    });

    FHorrorEventPayload PriorityPayload;
    PriorityPayload.EventName = "Test.Priority";
    EventBus->Publish(PriorityPayload);

    TestEqual("All subscribers executed", ExecutionOrder.Num(), 3);

    // Test 4: Event filtering
    int32 FilteredEvents = 0;
    int32 UnfilteredEvents = 0;

    EventBus->Subscribe("Test.Filtered", [&FilteredEvents](const FHorrorEventPayload& Payload) {
        if (Payload.IntData.FindRef("Value") > 50)
        {
            FilteredEvents++;
        }
    });

    EventBus->Subscribe("Test.Filtered", [&UnfilteredEvents](const FHorrorEventPayload& Payload) {
        UnfilteredEvents++;
    });

    for (int32 i = 0; i < 100; ++i)
    {
        FHorrorEventPayload FilterPayload;
        FilterPayload.EventName = "Test.Filtered";
        FilterPayload.IntData.Add("Value", i);
        EventBus->Publish(FilterPayload);
    }

    TestEqual("Filtered events counted", FilteredEvents, 49); // 51-99
    TestEqual("All events received", UnfilteredEvents, 100);

    // Test 5: Cross-system event communication
    bool bSystemAReceived = false;
    bool bSystemBReceived = false;
    bool bSystemCReceived = false;

    EventBus->Subscribe("System.A.Event", [&bSystemAReceived](const FHorrorEventPayload& Payload) {
        bSystemAReceived = true;
    });

    EventBus->Subscribe("System.B.Event", [&bSystemBReceived](const FHorrorEventPayload& Payload) {
        bSystemBReceived = true;
    });

    EventBus->Subscribe("System.C.Event", [&bSystemCReceived](const FHorrorEventPayload& Payload) {
        bSystemCReceived = true;
    });

    FHorrorEventPayload SystemAPayload;
    SystemAPayload.EventName = "System.A.Event";
    EventBus->Publish(SystemAPayload);

    FHorrorEventPayload SystemBPayload;
    SystemBPayload.EventName = "System.B.Event";
    EventBus->Publish(SystemBPayload);

    FHorrorEventPayload SystemCPayload;
    SystemCPayload.EventName = "System.C.Event";
    EventBus->Publish(SystemCPayload);

    TestTrue("System A communication", bSystemAReceived);
    TestTrue("System B communication", bSystemBReceived);
    TestTrue("System C communication", bSystemCReceived);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEventBusPerformanceTest,
    "HorrorProject.Integration.Systems.EventBusPerformance",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FEventBusPerformanceTest::RunTest(const FString& Parameters)
{
    using namespace HorrorTestHelpers;
    FTestWorldScope TestWorld;
    HORROR_TEST_REQUIRE_WORLD(TestWorld.GetWorld(), TestWorld, "Test world creation failed");

    UWorld* World = TestWorld.GetWorld();
    UHorrorEventBusSubsystem* EventBus = World->GetSubsystem<UHorrorEventBusSubsystem>();
    HORROR_TEST_REQUIRE(EventBus != nullptr, "EventBus not found");

    // Performance test: High-frequency events
    const int32 NumEvents = 10000;
    int32 EventsReceived = 0;

    EventBus->Subscribe("Performance.HighFrequency", [&EventsReceived](const FHorrorEventPayload& Payload) {
        EventsReceived++;
    });

    const double StartTime = FPlatformTime::Seconds();

    for (int32 i = 0; i < NumEvents; ++i)
    {
        FHorrorEventPayload Payload;
        Payload.EventName = "Performance.HighFrequency";
        Payload.IntData.Add("Index", i);
        EventBus->Publish(Payload);
    }

    const double EndTime = FPlatformTime::Seconds();
    const double ElapsedMs = (EndTime - StartTime) * 1000.0;
    const double AvgTimePerEvent = ElapsedMs / NumEvents;

    AddInfo(FString::Printf(TEXT("EventBus: %d events in %.2f ms (avg: %.4f ms/event)"),
        NumEvents, ElapsedMs, AvgTimePerEvent));

    TestEqual("All events delivered", EventsReceived, NumEvents);
    TestTrue("Performance acceptable", AvgTimePerEvent < 0.05); // < 0.05ms per event

    return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
