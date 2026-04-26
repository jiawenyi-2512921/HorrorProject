// Copyright Epic Games, Inc. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "Tests/TestHelpers.h"
#include "Tests/HorrorTestMacros.h"
#include "Game/HorrorGameModeBase.h"
#include "Game/HorrorFoundFootageContract.h"
#include "Game/HorrorEventBusSubsystem.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFullGameSessionTest,
    "HorrorProject.Integration.E2E.FullGameSession",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FFullGameSessionTest::RunTest(const FString& Parameters)
{
    using namespace HorrorTestHelpers;
    FTestWorldScope TestWorld;
    HORROR_TEST_REQUIRE_WORLD(TestWorld.GetWorld(), TestWorld, "Test world creation failed");

    UWorld* World = TestWorld.GetWorld();
    AHorrorGameModeBase* GameMode = TestWorld.SpawnActor<AHorrorGameModeBase>();
    HORROR_TEST_REQUIRE(GameMode != nullptr, "GameMode spawn failed");

    UHorrorEventBusSubsystem* EventBus = World->GetSubsystem<UHorrorEventBusSubsystem>();
    HORROR_TEST_REQUIRE(EventBus != nullptr, "EventBus not found");

    // Track complete game session
    bool bSessionStarted = false;
    bool bContractAccepted = false;
    bool bObjectivesCompleted = false;
    bool bEvidenceCollected = false;
    bool bContractCompleted = false;
    bool bSessionEnded = false;

    // Session start
    EventBus->Subscribe("Session.Started", [&bSessionStarted](const FHorrorEventPayload& Payload) {
        bSessionStarted = true;
    });

    FHorrorEventPayload StartPayload;
    StartPayload.EventName = "Session.Started";
    EventBus->Publish(StartPayload);
    TestTrue("Session started", bSessionStarted);

    // Contract acceptance
    EventBus->Subscribe("Contract.Accepted", [&bContractAccepted](const FHorrorEventPayload& Payload) {
        bContractAccepted = true;
    });

    UHorrorFoundFootageContract* Contract = NewObject<UHorrorFoundFootageContract>(GameMode);
    Contract->ContractID = FName("E2E_Contract");
    Contract->RequiredEvidenceCount = 5;

    FHorrorEventPayload ContractPayload;
    ContractPayload.EventName = "Contract.Accepted";
    ContractPayload.StringData.Add("ContractID", Contract->ContractID.ToString());
    EventBus->Publish(ContractPayload);
    TestTrue("Contract accepted", bContractAccepted);

    // Complete objectives
    int32 ObjectiveCount = 0;
    EventBus->Subscribe("Objective.Completed", [&ObjectiveCount](const FHorrorEventPayload& Payload) {
        ObjectiveCount++;
    });

    for (int32 i = 0; i < 3; ++i)
    {
        FHorrorEventPayload ObjPayload;
        ObjPayload.EventName = "Objective.Completed";
        ObjPayload.IntData.Add("ObjectiveID", i);
        EventBus->Publish(ObjPayload);
    }

    bObjectivesCompleted = (ObjectiveCount == 3);
    TestTrue("Objectives completed", bObjectivesCompleted);

    // Collect evidence
    int32 EvidenceCount = 0;
    EventBus->Subscribe("Evidence.Collected", [&EvidenceCount](const FHorrorEventPayload& Payload) {
        EvidenceCount++;
    });

    for (int32 i = 0; i < Contract->RequiredEvidenceCount; ++i)
    {
        FHorrorEventPayload EvidencePayload;
        EvidencePayload.EventName = "Evidence.Collected";
        EvidencePayload.IntData.Add("EvidenceID", i);
        EventBus->Publish(EvidencePayload);
    }

    bEvidenceCollected = (EvidenceCount == Contract->RequiredEvidenceCount);
    TestTrue("Evidence collected", bEvidenceCollected);

    // Complete contract
    EventBus->Subscribe("Contract.Completed", [&bContractCompleted](const FHorrorEventPayload& Payload) {
        bContractCompleted = true;
    });

    FHorrorEventPayload CompletePayload;
    CompletePayload.EventName = "Contract.Completed";
    CompletePayload.StringData.Add("ContractID", Contract->ContractID.ToString());
    EventBus->Publish(CompletePayload);
    TestTrue("Contract completed", bContractCompleted);

    // End session
    EventBus->Subscribe("Session.Ended", [&bSessionEnded](const FHorrorEventPayload& Payload) {
        bSessionEnded = true;
    });

    FHorrorEventPayload EndPayload;
    EndPayload.EventName = "Session.Ended";
    EventBus->Publish(EndPayload);
    TestTrue("Session ended", bSessionEnded);

    // Verify complete flow
    TestTrue("Complete game session flow",
        bSessionStarted && bContractAccepted && bObjectivesCompleted &&
        bEvidenceCollected && bContractCompleted && bSessionEnded);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGameSessionWithSaveTest,
    "HorrorProject.Integration.E2E.GameSessionWithSave",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FGameSessionWithSaveTest::RunTest(const FString& Parameters)
{
    using namespace HorrorTestHelpers;
    FTestWorldScope TestWorld;
    HORROR_TEST_REQUIRE_WORLD(TestWorld.GetWorld(), TestWorld, "Test world creation failed");

    UWorld* World = TestWorld.GetWorld();
    UHorrorEventBusSubsystem* EventBus = World->GetSubsystem<UHorrorEventBusSubsystem>();
    HORROR_TEST_REQUIRE(EventBus != nullptr, "EventBus not found");

    // Start session
    FHorrorEventPayload StartPayload;
    StartPayload.EventName = "Session.Started";
    EventBus->Publish(StartPayload);

    // Make progress
    int32 Progress = 0;
    EventBus->Subscribe("Progress.Updated", [&Progress](const FHorrorEventPayload& Payload) {
        Progress = Payload.IntData.FindRef("Progress");
    });

    for (int32 i = 1; i <= 5; ++i)
    {
        FHorrorEventPayload ProgressPayload;
        ProgressPayload.EventName = "Progress.Updated";
        ProgressPayload.IntData.Add("Progress", i * 20);
        EventBus->Publish(ProgressPayload);
    }

    TestEqual("Progress tracked", Progress, 100);

    // Save game
    bool bSaved = false;
    EventBus->Subscribe("Save.Completed", [&bSaved](const FHorrorEventPayload& Payload) {
        bSaved = true;
    });

    FHorrorEventPayload SavePayload;
    SavePayload.EventName = "Save.Completed";
    SavePayload.StringData.Add("SlotName", "E2E_Save");
    EventBus->Publish(SavePayload);

    TestTrue("Game saved", bSaved);

    // Load game
    bool bLoaded = false;
    int32 LoadedProgress = 0;

    EventBus->Subscribe("Load.Completed", [&](const FHorrorEventPayload& Payload) {
        bLoaded = true;
        LoadedProgress = Payload.IntData.FindRef("Progress");
    });

    FHorrorEventPayload LoadPayload;
    LoadPayload.EventName = "Load.Completed";
    LoadPayload.StringData.Add("SlotName", "E2E_Save");
    LoadPayload.IntData.Add("Progress", Progress);
    EventBus->Publish(LoadPayload);

    TestTrue("Game loaded", bLoaded);
    TestEqual("Progress preserved", LoadedProgress, Progress);

    return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
