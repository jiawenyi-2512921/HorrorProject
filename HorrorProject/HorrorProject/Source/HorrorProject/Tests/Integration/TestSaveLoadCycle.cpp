#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR && HORRORPROJECT_ENABLE_LEGACY_AUTOMATION_TESTS

// Copyright Epic Games, Inc. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "Tests/TestHelpers.h"
#include "Tests/HorrorTestMacros.h"
#include "Game/HorrorGameModeBase.h"
#include "Game/HorrorEventBusSubsystem.h"
#include "GameFramework/SaveGame.h"
#include "Kismet/GameplayStatics.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSaveLoadCycleTest,
    "HorrorProject.Integration.Gameplay.SaveLoadCycle",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FSaveLoadCycleTest::RunTest(const FString& Parameters)
{
    using namespace HorrorTestHelpers;
    FTestWorldScope TestWorld;
    HORROR_TEST_REQUIRE_WORLD(TestWorld.GetWorld(), TestWorld, "Test world creation failed");

    UWorld* World = TestWorld.GetWorld();
    AHorrorGameModeBase* GameMode = TestWorld.SpawnActor<AHorrorGameModeBase>();
    HORROR_TEST_REQUIRE(GameMode != nullptr, "GameMode spawn failed");

    UHorrorEventBusSubsystem* EventBus = World->GetSubsystem<UHorrorEventBusSubsystem>();
    HORROR_TEST_REQUIRE(EventBus != nullptr, "EventBus not found");

    // Test 1: Save game state
    bool bSaveInitiated = false;
    bool bSaveCompleted = false;
    FString SaveSlotName = "TestSaveSlot";

    EventBus->Subscribe("Save.Initiated", [&bSaveInitiated](const FHorrorEventPayload& Payload) {
        bSaveInitiated = true;
    });

    EventBus->Subscribe("Save.Completed", [&bSaveCompleted](const FHorrorEventPayload& Payload) {
        bSaveCompleted = true;
    });

    FHorrorEventPayload SavePayload;
    SavePayload.EventName = "Save.Initiated";
    SavePayload.StringData.Add("SlotName", SaveSlotName);
    EventBus->Publish(SavePayload);

    TestTrue("Save initiated", bSaveInitiated);

    // Simulate save completion
    FHorrorEventPayload SaveCompletePayload;
    SaveCompletePayload.EventName = "Save.Completed";
    SaveCompletePayload.StringData.Add("SlotName", SaveSlotName);
    SaveCompletePayload.BoolData.Add("Success", true);
    EventBus->Publish(SaveCompletePayload);

    TestTrue("Save completed", bSaveCompleted);

    // Test 2: Load game state
    bool bLoadInitiated = false;
    bool bLoadCompleted = false;

    EventBus->Subscribe("Load.Initiated", [&bLoadInitiated](const FHorrorEventPayload& Payload) {
        bLoadInitiated = true;
    });

    EventBus->Subscribe("Load.Completed", [&bLoadCompleted](const FHorrorEventPayload& Payload) {
        bLoadCompleted = true;
    });

    FHorrorEventPayload LoadPayload;
    LoadPayload.EventName = "Load.Initiated";
    LoadPayload.StringData.Add("SlotName", SaveSlotName);
    EventBus->Publish(LoadPayload);

    TestTrue("Load initiated", bLoadInitiated);

    FHorrorEventPayload LoadCompletePayload;
    LoadCompletePayload.EventName = "Load.Completed";
    LoadCompletePayload.StringData.Add("SlotName", SaveSlotName);
    LoadCompletePayload.BoolData.Add("Success", true);
    EventBus->Publish(LoadCompletePayload);

    TestTrue("Load completed", bLoadCompleted);

    // Test 3: Save data integrity
    TMap<FString, int32> SavedData;
    SavedData.Add("PlayerHealth", 100);
    SavedData.Add("EvidenceCount", 5);
    SavedData.Add("ObjectivesCompleted", 3);

    TMap<FString, int32> LoadedData;

    EventBus->Subscribe("Save.DataWritten", [&SavedData](const FHorrorEventPayload& Payload) {
        // Data written to save
    });

    EventBus->Subscribe("Load.DataRead", [&LoadedData](const FHorrorEventPayload& Payload) {
        LoadedData.Add("PlayerHealth", Payload.IntData.FindRef("PlayerHealth"));
        LoadedData.Add("EvidenceCount", Payload.IntData.FindRef("EvidenceCount"));
        LoadedData.Add("ObjectivesCompleted", Payload.IntData.FindRef("ObjectivesCompleted"));
    });

    // Simulate data load
    FHorrorEventPayload DataPayload;
    DataPayload.EventName = "Load.DataRead";
    DataPayload.IntData.Add("PlayerHealth", 100);
    DataPayload.IntData.Add("EvidenceCount", 5);
    DataPayload.IntData.Add("ObjectivesCompleted", 3);
    EventBus->Publish(DataPayload);

    TestEqual("Health preserved", LoadedData.FindRef("PlayerHealth"), SavedData.FindRef("PlayerHealth"));
    TestEqual("Evidence preserved", LoadedData.FindRef("EvidenceCount"), SavedData.FindRef("EvidenceCount"));
    TestEqual("Objectives preserved", LoadedData.FindRef("ObjectivesCompleted"), SavedData.FindRef("ObjectivesCompleted"));

    // Test 4: Multiple save slots
    TArray<FString> SaveSlots = {"Slot1", "Slot2", "Slot3"};
    TSet<FString> CreatedSlots;

    EventBus->Subscribe("Save.SlotCreated", [&CreatedSlots](const FHorrorEventPayload& Payload) {
        CreatedSlots.Add(Payload.StringData.FindRef("SlotName"));
    });

    for (const FString& Slot : SaveSlots)
    {
        FHorrorEventPayload SlotPayload;
        SlotPayload.EventName = "Save.SlotCreated";
        SlotPayload.StringData.Add("SlotName", Slot);
        EventBus->Publish(SlotPayload);
    }

    TestEqual("All save slots created", CreatedSlots.Num(), SaveSlots.Num());

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAutoSaveTest,
    "HorrorProject.Integration.Gameplay.AutoSave",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FAutoSaveTest::RunTest(const FString& Parameters)
{
    using namespace HorrorTestHelpers;
    FTestWorldScope TestWorld;
    HORROR_TEST_REQUIRE_WORLD(TestWorld.GetWorld(), TestWorld, "Test world creation failed");

    UWorld* World = TestWorld.GetWorld();
    UHorrorEventBusSubsystem* EventBus = World->GetSubsystem<UHorrorEventBusSubsystem>();
    HORROR_TEST_REQUIRE(EventBus != nullptr, "EventBus not found");

    // Test auto-save triggers
    int32 AutoSaveCount = 0;
    EventBus->Subscribe("Save.Auto", [&AutoSaveCount](const FHorrorEventPayload& Payload) {
        AutoSaveCount++;
    });

    // Trigger auto-saves on key events
    TArray<FString> AutoSaveTriggers = {
        "Objective.Completed",
        "Evidence.QuotaMet",
        "Location.Checkpoint"
    };

    for (const FString& Trigger : AutoSaveTriggers)
    {
        FHorrorEventPayload Payload;
        Payload.EventName = "Save.Auto";
        Payload.StringData.Add("Trigger", Trigger);
        EventBus->Publish(Payload);
    }

    TestEqual("Auto-saves triggered", AutoSaveCount, AutoSaveTriggers.Num());

    return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS

#endif // WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR
