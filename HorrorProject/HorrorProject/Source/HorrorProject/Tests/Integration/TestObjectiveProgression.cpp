#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR && HORRORPROJECT_ENABLE_LEGACY_AUTOMATION_TESTS

// Copyright Epic Games, Inc. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "Tests/TestHelpers.h"
#include "Tests/HorrorTestMacros.h"
#include "Game/HorrorGameModeBase.h"
#include "Game/HorrorFoundFootageContract.h"
#include "Game/HorrorEventBusSubsystem.h"
#include "Game/FoundFootageObjectiveInteractable.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FObjectiveProgressionTest,
    "HorrorProject.Integration.Gameplay.ObjectiveProgression",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FObjectiveProgressionTest::RunTest(const FString& Parameters)
{
    using namespace HorrorTestHelpers;
    FTestWorldScope TestWorld;
    HORROR_TEST_REQUIRE_WORLD(TestWorld.GetWorld(), TestWorld, "Test world creation failed");

    UWorld* World = TestWorld.GetWorld();
    AHorrorGameModeBase* GameMode = TestWorld.SpawnActor<AHorrorGameModeBase>();
    HORROR_TEST_REQUIRE(GameMode != nullptr, "GameMode spawn failed");

    UHorrorEventBusSubsystem* EventBus = World->GetSubsystem<UHorrorEventBusSubsystem>();
    HORROR_TEST_REQUIRE(EventBus != nullptr, "EventBus not found");

    // Create contract with multiple objectives
    UHorrorFoundFootageContract* Contract = NewObject<UHorrorFoundFootageContract>(GameMode);
    Contract->ContractID = FName("ObjectiveTest");
    Contract->RequiredEvidenceCount = 5;

    // Test 1: Sequential objective completion
    TArray<bool> ObjectiveStates = {false, false, false, false, false};

    for (int32 i = 0; i < 5; ++i)
    {
        EventBus->Subscribe(FString::Printf(TEXT("Objective.%d.Completed"), i),
            [&ObjectiveStates, i](const FHorrorEventPayload& Payload) {
                ObjectiveStates[i] = true;
            });
    }

    // Complete objectives in order
    for (int32 i = 0; i < 5; ++i)
    {
        FHorrorEventPayload Payload;
        Payload.EventName = FString::Printf(TEXT("Objective.%d.Completed"), i);
        Payload.IntData.Add("ObjectiveID", i);
        EventBus->Publish(Payload);

        TestTrue(FString::Printf(TEXT("Objective %d completed"), i), ObjectiveStates[i]);
    }

    // Test 2: Objective dependency chain
    bool bChainCompleted = false;
    int32 ChainProgress = 0;

    EventBus->Subscribe("ObjectiveChain.Progress", [&ChainProgress](const FHorrorEventPayload& Payload) {
        ChainProgress = Payload.IntData.FindRef("Progress");
    });

    EventBus->Subscribe("ObjectiveChain.Completed", [&bChainCompleted](const FHorrorEventPayload& Payload) {
        bChainCompleted = true;
    });

    // Simulate chain progression
    for (int32 i = 1; i <= 5; ++i)
    {
        FHorrorEventPayload Payload;
        Payload.EventName = "ObjectiveChain.Progress";
        Payload.IntData.Add("Progress", i);
        EventBus->Publish(Payload);
    }

    TestEqual("Chain progress tracked", ChainProgress, 5);

    FHorrorEventPayload ChainComplete;
    ChainComplete.EventName = "ObjectiveChain.Completed";
    EventBus->Publish(ChainComplete);
    TestTrue("Objective chain completed", bChainCompleted);

    // Test 3: Optional objectives
    int32 OptionalCompleted = 0;
    EventBus->Subscribe("Objective.Optional.Completed", [&OptionalCompleted](const FHorrorEventPayload& Payload) {
        OptionalCompleted++;
    });

    for (int32 i = 0; i < 3; ++i)
    {
        FHorrorEventPayload Payload;
        Payload.EventName = "Objective.Optional.Completed";
        Payload.IntData.Add("OptionalID", i);
        EventBus->Publish(Payload);
    }

    TestEqual("Optional objectives completed", OptionalCompleted, 3);

    // Test 4: Objective failure handling
    bool bObjectiveFailed = false;
    EventBus->Subscribe("Objective.Failed", [&bObjectiveFailed](const FHorrorEventPayload& Payload) {
        bObjectiveFailed = true;
    });

    FHorrorEventPayload FailPayload;
    FailPayload.EventName = "Objective.Failed";
    FailPayload.StringData.Add("Reason", "TimeExpired");
    EventBus->Publish(FailPayload);

    TestTrue("Objective failure handled", bObjectiveFailed);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FObjectiveInteractionTest,
    "HorrorProject.Integration.Gameplay.ObjectiveInteraction",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FObjectiveInteractionTest::RunTest(const FString& Parameters)
{
    using namespace HorrorTestHelpers;
    FTestWorldScope TestWorld;
    HORROR_TEST_REQUIRE_WORLD(TestWorld.GetWorld(), TestWorld, "Test world creation failed");

    UWorld* World = TestWorld.GetWorld();

    // Spawn objective interactable
    AFoundFootageObjectiveInteractable* Objective = TestWorld.SpawnActor<AFoundFootageObjectiveInteractable>();
    HORROR_TEST_REQUIRE(Objective != nullptr, "Objective spawn failed");

    UHorrorEventBusSubsystem* EventBus = World->GetSubsystem<UHorrorEventBusSubsystem>();
    HORROR_TEST_REQUIRE(EventBus != nullptr, "EventBus not found");

    // Test interaction flow
    bool bInteractionStarted = false;
    bool bInteractionCompleted = false;

    EventBus->Subscribe("Interaction.Started", [&bInteractionStarted](const FHorrorEventPayload& Payload) {
        bInteractionStarted = true;
    });

    EventBus->Subscribe("Interaction.Completed", [&bInteractionCompleted](const FHorrorEventPayload& Payload) {
        bInteractionCompleted = true;
    });

    // Simulate interaction
    FHorrorEventPayload StartPayload;
    StartPayload.EventName = "Interaction.Started";
    StartPayload.StringData.Add("ObjectiveID", Objective->GetName());
    EventBus->Publish(StartPayload);

    TestTrue("Interaction started", bInteractionStarted);

    // Complete interaction
    FHorrorEventPayload CompletePayload;
    CompletePayload.EventName = "Interaction.Completed";
    CompletePayload.StringData.Add("ObjectiveID", Objective->GetName());
    EventBus->Publish(CompletePayload);

    TestTrue("Interaction completed", bInteractionCompleted);

    return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS

#endif // WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR
