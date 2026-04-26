// Copyright Epic Games, Inc. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "Tests/TestHelpers.h"
#include "Tests/HorrorTestMacros.h"
#include "Game/HorrorGameModeBase.h"
#include "Game/HorrorFoundFootageContract.h"
#include "Game/HorrorEventBusSubsystem.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCompleteGameplayLoopTest,
    "HorrorProject.Integration.Gameplay.CompleteLoop",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FCompleteGameplayLoopTest::RunTest(const FString& Parameters)
{
    using namespace HorrorTestHelpers;
    FTestWorldScope TestWorld;
    HORROR_TEST_REQUIRE_WORLD(TestWorld.GetWorld(), TestWorld, "Test world creation failed");

    UWorld* World = TestWorld.GetWorld();

    // Spawn game mode
    AHorrorGameModeBase* GameMode = TestWorld.SpawnActor<AHorrorGameModeBase>();
    HORROR_TEST_REQUIRE(GameMode != nullptr, "GameMode spawn failed");
    World->SetGameMode(GameMode);

    // Get EventBus
    UHorrorEventBusSubsystem* EventBus = World->GetSubsystem<UHorrorEventBusSubsystem>();
    HORROR_TEST_REQUIRE(EventBus != nullptr, "EventBus subsystem not found");

    // Test 1: Game initialization
    GameMode->InitGame(TEXT(""), TEXT(""), TEXT(""));
    TestTrue("GameMode initialized", GameMode->HasBegunPlay());

    // Test 2: Contract creation
    UHorrorFoundFootageContract* Contract = NewObject<UHorrorFoundFootageContract>(GameMode);
    HORROR_TEST_REQUIRE(Contract != nullptr, "Contract creation failed");

    Contract->ContractID = FName("TestContract");
    Contract->RequiredEvidenceCount = 3;
    Contract->TimeLimit = 600.0f;

    // Test 3: Start gameplay session
    bool bSessionStarted = false;
    EventBus->Subscribe("GameSession.Started", [&bSessionStarted](const FHorrorEventPayload& Payload) {
        bSessionStarted = true;
    });

    GameMode->StartPlay();
    TestTrue("Game session started", bSessionStarted);

    // Test 4: Objective progression
    int32 ObjectivesCompleted = 0;
    EventBus->Subscribe("Objective.Completed", [&ObjectivesCompleted](const FHorrorEventPayload& Payload) {
        ObjectivesCompleted++;
    });

    // Simulate completing objectives
    for (int32 i = 0; i < 3; ++i)
    {
        FHorrorEventPayload Payload;
        Payload.EventName = "Objective.Completed";
        Payload.IntData.Add("ObjectiveIndex", i);
        EventBus->Publish(Payload);
    }

    TestEqual("All objectives completed", ObjectivesCompleted, 3);

    // Test 5: Evidence collection
    int32 EvidenceCollected = 0;
    EventBus->Subscribe("Evidence.Collected", [&EvidenceCollected](const FHorrorEventPayload& Payload) {
        EvidenceCollected++;
    });

    for (int32 i = 0; i < Contract->RequiredEvidenceCount; ++i)
    {
        FHorrorEventPayload Payload;
        Payload.EventName = "Evidence.Collected";
        Payload.StringData.Add("EvidenceType", FString::Printf(TEXT("Evidence_%d"), i));
        EventBus->Publish(Payload);
    }

    TestEqual("Required evidence collected", EvidenceCollected, Contract->RequiredEvidenceCount);

    // Test 6: Contract completion
    bool bContractCompleted = false;
    EventBus->Subscribe("Contract.Completed", [&bContractCompleted](const FHorrorEventPayload& Payload) {
        bContractCompleted = true;
    });

    FHorrorEventPayload CompletionPayload;
    CompletionPayload.EventName = "Contract.Completed";
    CompletionPayload.StringData.Add("ContractID", Contract->ContractID.ToString());
    EventBus->Publish(CompletionPayload);

    TestTrue("Contract completed successfully", bContractCompleted);

    // Test 7: Session end
    bool bSessionEnded = false;
    EventBus->Subscribe("GameSession.Ended", [&bSessionEnded](const FHorrorEventPayload& Payload) {
        bSessionEnded = true;
    });

    GameMode->EndPlay(EEndPlayReason::Quit);
    TestTrue("Game session ended", bSessionEnded);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGameplayLoopPerformanceTest,
    "HorrorProject.Integration.Gameplay.LoopPerformance",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FGameplayLoopPerformanceTest::RunTest(const FString& Parameters)
{
    using namespace HorrorTestHelpers;
    FTestWorldScope TestWorld;
    HORROR_TEST_REQUIRE_WORLD(TestWorld.GetWorld(), TestWorld, "Test world creation failed");

    UWorld* World = TestWorld.GetWorld();
    AHorrorGameModeBase* GameMode = TestWorld.SpawnActor<AHorrorGameModeBase>();
    HORROR_TEST_REQUIRE(GameMode != nullptr, "GameMode spawn failed");

    UHorrorEventBusSubsystem* EventBus = World->GetSubsystem<UHorrorEventBusSubsystem>();
    HORROR_TEST_REQUIRE(EventBus != nullptr, "EventBus not found");

    // Performance benchmark: 1000 gameplay events
    const int32 NumEvents = 1000;
    const double StartTime = FPlatformTime::Seconds();

    for (int32 i = 0; i < NumEvents; ++i)
    {
        FHorrorEventPayload Payload;
        Payload.EventName = "Performance.Test";
        Payload.IntData.Add("EventIndex", i);
        EventBus->Publish(Payload);
    }

    const double EndTime = FPlatformTime::Seconds();
    const double ElapsedMs = (EndTime - StartTime) * 1000.0;
    const double AvgTimePerEvent = ElapsedMs / NumEvents;

    AddInfo(FString::Printf(TEXT("Processed %d events in %.2f ms (avg: %.4f ms/event)"),
        NumEvents, ElapsedMs, AvgTimePerEvent));

    TestTrue("Event processing performance acceptable", AvgTimePerEvent < 0.1); // < 0.1ms per event

    return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
