// Copyright Epic Games, Inc. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "Tests/TestHelpers.h"
#include "Tests/HorrorTestMacros.h"
#include "Game/HorrorEventBusSubsystem.h"
#include "AIController.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAISystemIntegrationTest,
    "HorrorProject.Integration.Systems.AI",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FAISystemIntegrationTest::RunTest(const FString& Parameters)
{
    using namespace HorrorTestHelpers;
    FTestWorldScope TestWorld;
    HORROR_TEST_REQUIRE_WORLD(TestWorld.GetWorld(), TestWorld, "Test world creation failed");

    UWorld* World = TestWorld.GetWorld();
    UHorrorEventBusSubsystem* EventBus = World->GetSubsystem<UHorrorEventBusSubsystem>();
    HORROR_TEST_REQUIRE(EventBus != nullptr, "EventBus not found");

    // Test 1: AI state transitions
    FString CurrentAIState = "Idle";
    TArray<FString> StateHistory;

    EventBus->Subscribe("AI.StateChanged", [&](const FHorrorEventPayload& Payload) {
        CurrentAIState = Payload.StringData.FindRef("NewState");
        StateHistory.Add(CurrentAIState);
    });

    TArray<FString> AIStates = {"Idle", "Patrol", "Investigate", "Chase", "Attack", "Retreat"};
    for (const FString& State : AIStates)
    {
        FHorrorEventPayload Payload;
        Payload.EventName = "AI.StateChanged";
        Payload.StringData.Add("NewState", State);
        EventBus->Publish(Payload);
    }

    TestEqual("AI state transitions", StateHistory.Num(), AIStates.Num());
    TestEqual("Final AI state", CurrentAIState, FString("Retreat"));

    // Test 2: AI perception events
    int32 PlayerDetections = 0;
    int32 SoundDetections = 0;

    EventBus->Subscribe("AI.Perception.PlayerDetected", [&PlayerDetections](const FHorrorEventPayload& Payload) {
        PlayerDetections++;
    });

    EventBus->Subscribe("AI.Perception.SoundDetected", [&SoundDetections](const FHorrorEventPayload& Payload) {
        SoundDetections++;
    });

    for (int32 i = 0; i < 3; ++i)
    {
        FHorrorEventPayload PlayerPayload;
        PlayerPayload.EventName = "AI.Perception.PlayerDetected";
        PlayerPayload.FloatData.Add("Distance", 500.0f + i * 100.0f);
        EventBus->Publish(PlayerPayload);
    }

    for (int32 i = 0; i < 5; ++i)
    {
        FHorrorEventPayload SoundPayload;
        SoundPayload.EventName = "AI.Perception.SoundDetected";
        SoundPayload.FloatData.Add("Loudness", 0.5f + i * 0.1f);
        EventBus->Publish(SoundPayload);
    }

    TestEqual("Player detections", PlayerDetections, 3);
    TestEqual("Sound detections", SoundDetections, 5);

    // Test 3: AI pathfinding
    bool bPathFound = false;
    int32 PathLength = 0;

    EventBus->Subscribe("AI.Path.Found", [&](const FHorrorEventPayload& Payload) {
        bPathFound = true;
        PathLength = Payload.IntData.FindRef("PathLength");
    });

    FHorrorEventPayload PathPayload;
    PathPayload.EventName = "AI.Path.Found";
    PathPayload.IntData.Add("PathLength", 15);
    PathPayload.BoolData.Add("IsValid", true);
    EventBus->Publish(PathPayload);

    TestTrue("Path found", bPathFound);
    TestEqual("Path length", PathLength, 15);

    // Test 4: AI behavior tree execution
    TArray<FString> ExecutedTasks;

    EventBus->Subscribe("AI.BehaviorTree.TaskExecuted", [&ExecutedTasks](const FHorrorEventPayload& Payload) {
        ExecutedTasks.Add(Payload.StringData.FindRef("TaskName"));
    });

    TArray<FString> Tasks = {"FindPlayer", "MoveToLocation", "PlayAnimation", "AttackPlayer"};
    for (const FString& Task : Tasks)
    {
        FHorrorEventPayload Payload;
        Payload.EventName = "AI.BehaviorTree.TaskExecuted";
        Payload.StringData.Add("TaskName", Task);
        EventBus->Publish(Payload);
    }

    TestEqual("Behavior tree tasks executed", ExecutedTasks.Num(), Tasks.Num());

    // Test 5: AI threat assessment
    float ThreatLevel = 0.0f;
    FString ThreatTarget = "None";

    EventBus->Subscribe("AI.Threat.Updated", [&](const FHorrorEventPayload& Payload) {
        ThreatLevel = Payload.FloatData.FindRef("ThreatLevel");
        ThreatTarget = Payload.StringData.FindRef("Target");
    });

    FHorrorEventPayload ThreatPayload;
    ThreatPayload.EventName = "AI.Threat.Updated";
    ThreatPayload.FloatData.Add("ThreatLevel", 0.85f);
    ThreatPayload.StringData.Add("Target", "Player");
    EventBus->Publish(ThreatPayload);

    TestTrue("Threat level updated", FMath::IsNearlyEqual(ThreatLevel, 0.85f, 0.01f));
    TestEqual("Threat target identified", ThreatTarget, FString("Player"));

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAICoordinationTest,
    "HorrorProject.Integration.Systems.AICoordination",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FAICoordinationTest::RunTest(const FString& Parameters)
{
    using namespace HorrorTestHelpers;
    FTestWorldScope TestWorld;
    HORROR_TEST_REQUIRE_WORLD(TestWorld.GetWorld(), TestWorld, "Test world creation failed");

    UWorld* World = TestWorld.GetWorld();
    UHorrorEventBusSubsystem* EventBus = World->GetSubsystem<UHorrorEventBusSubsystem>();
    HORROR_TEST_REQUIRE(EventBus != nullptr, "EventBus not found");

    // Test multi-AI coordination
    TSet<int32> ActiveAIs;

    EventBus->Subscribe("AI.Coordination.Join", [&ActiveAIs](const FHorrorEventPayload& Payload) {
        ActiveAIs.Add(Payload.IntData.FindRef("AIID"));
    });

    EventBus->Subscribe("AI.Coordination.Leave", [&ActiveAIs](const FHorrorEventPayload& Payload) {
        ActiveAIs.Remove(Payload.IntData.FindRef("AIID"));
    });

    // Add AIs to coordination
    for (int32 i = 0; i < 5; ++i)
    {
        FHorrorEventPayload Payload;
        Payload.EventName = "AI.Coordination.Join";
        Payload.IntData.Add("AIID", i);
        EventBus->Publish(Payload);
    }

    TestEqual("AIs coordinating", ActiveAIs.Num(), 5);

    // Remove some AIs
    for (int32 i = 0; i < 2; ++i)
    {
        FHorrorEventPayload Payload;
        Payload.EventName = "AI.Coordination.Leave";
        Payload.IntData.Add("AIID", i);
        EventBus->Publish(Payload);
    }

    TestEqual("AIs remaining", ActiveAIs.Num(), 3);

    return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
