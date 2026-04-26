// Copyright Epic Games, Inc. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "Tests/TestHelpers.h"
#include "Tests/HorrorTestMacros.h"
#include "Game/HorrorGameModeBase.h"
#include "Game/HorrorFoundFootageContract.h"
#include "Game/HorrorEventBusSubsystem.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEvidenceCollectionFlowTest,
    "HorrorProject.Integration.Gameplay.EvidenceCollection",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FEvidenceCollectionFlowTest::RunTest(const FString& Parameters)
{
    using namespace HorrorTestHelpers;
    FTestWorldScope TestWorld;
    HORROR_TEST_REQUIRE_WORLD(TestWorld.GetWorld(), TestWorld, "Test world creation failed");

    UWorld* World = TestWorld.GetWorld();
    AHorrorGameModeBase* GameMode = TestWorld.SpawnActor<AHorrorGameModeBase>();
    HORROR_TEST_REQUIRE(GameMode != nullptr, "GameMode spawn failed");

    UHorrorEventBusSubsystem* EventBus = World->GetSubsystem<UHorrorEventBusSubsystem>();
    HORROR_TEST_REQUIRE(EventBus != nullptr, "EventBus not found");

    // Test 1: Photo evidence collection
    int32 PhotosCollected = 0;
    TArray<FString> PhotoTypes;

    EventBus->Subscribe("Evidence.Photo.Captured", [&PhotosCollected, &PhotoTypes](const FHorrorEventPayload& Payload) {
        PhotosCollected++;
        PhotoTypes.Add(Payload.StringData.FindRef("PhotoType"));
    });

    // Capture different photo types
    TArray<FString> ExpectedTypes = {"Anomaly", "Entity", "Location", "Clue"};
    for (const FString& Type : ExpectedTypes)
    {
        FHorrorEventPayload Payload;
        Payload.EventName = "Evidence.Photo.Captured";
        Payload.StringData.Add("PhotoType", Type);
        Payload.FloatData.Add("Quality", 0.85f);
        EventBus->Publish(Payload);
    }

    TestEqual("Photos collected", PhotosCollected, 4);
    TestEqual("Photo types match", PhotoTypes.Num(), ExpectedTypes.Num());

    // Test 2: Audio evidence collection
    int32 AudioRecordings = 0;
    float TotalDuration = 0.0f;

    EventBus->Subscribe("Evidence.Audio.Recorded", [&AudioRecordings, &TotalDuration](const FHorrorEventPayload& Payload) {
        AudioRecordings++;
        TotalDuration += Payload.FloatData.FindRef("Duration");
    });

    for (int32 i = 0; i < 3; ++i)
    {
        FHorrorEventPayload Payload;
        Payload.EventName = "Evidence.Audio.Recorded";
        Payload.FloatData.Add("Duration", 5.0f + i * 2.0f);
        Payload.StringData.Add("AudioType", "Paranormal");
        EventBus->Publish(Payload);
    }

    TestEqual("Audio recordings collected", AudioRecordings, 3);
    TestTrue("Total duration tracked", FMath::IsNearlyEqual(TotalDuration, 21.0f, 0.1f));

    // Test 3: Evidence validation
    int32 ValidEvidence = 0;
    int32 InvalidEvidence = 0;

    EventBus->Subscribe("Evidence.Validated", [&ValidEvidence](const FHorrorEventPayload& Payload) {
        if (Payload.BoolData.FindRef("IsValid"))
        {
            ValidEvidence++;
        }
        else
        {
            InvalidEvidence++;
        }
    });

    // Submit valid evidence
    for (int32 i = 0; i < 5; ++i)
    {
        FHorrorEventPayload Payload;
        Payload.EventName = "Evidence.Validated";
        Payload.BoolData.Add("IsValid", i < 3); // First 3 valid
        Payload.IntData.Add("EvidenceID", i);
        EventBus->Publish(Payload);
    }

    TestEqual("Valid evidence count", ValidEvidence, 3);
    TestEqual("Invalid evidence count", InvalidEvidence, 2);

    // Test 4: Evidence quota completion
    UHorrorFoundFootageContract* Contract = NewObject<UHorrorFoundFootageContract>(GameMode);
    Contract->RequiredEvidenceCount = 10;

    int32 CurrentEvidence = 0;
    bool bQuotaMet = false;

    EventBus->Subscribe("Evidence.QuotaProgress", [&CurrentEvidence](const FHorrorEventPayload& Payload) {
        CurrentEvidence = Payload.IntData.FindRef("Current");
    });

    EventBus->Subscribe("Evidence.QuotaMet", [&bQuotaMet](const FHorrorEventPayload& Payload) {
        bQuotaMet = true;
    });

    // Collect evidence until quota met
    for (int32 i = 0; i < Contract->RequiredEvidenceCount; ++i)
    {
        FHorrorEventPayload ProgressPayload;
        ProgressPayload.EventName = "Evidence.QuotaProgress";
        ProgressPayload.IntData.Add("Current", i + 1);
        ProgressPayload.IntData.Add("Required", Contract->RequiredEvidenceCount);
        EventBus->Publish(ProgressPayload);
    }

    TestEqual("Evidence quota reached", CurrentEvidence, Contract->RequiredEvidenceCount);

    FHorrorEventPayload QuotaPayload;
    QuotaPayload.EventName = "Evidence.QuotaMet";
    EventBus->Publish(QuotaPayload);
    TestTrue("Quota completion triggered", bQuotaMet);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEvidenceStorageTest,
    "HorrorProject.Integration.Gameplay.EvidenceStorage",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FEvidenceStorageTest::RunTest(const FString& Parameters)
{
    using namespace HorrorTestHelpers;
    FTestWorldScope TestWorld;
    HORROR_TEST_REQUIRE_WORLD(TestWorld.GetWorld(), TestWorld, "Test world creation failed");

    UWorld* World = TestWorld.GetWorld();
    UHorrorEventBusSubsystem* EventBus = World->GetSubsystem<UHorrorEventBusSubsystem>();
    HORROR_TEST_REQUIRE(EventBus != nullptr, "EventBus not found");

    // Test evidence storage capacity
    TArray<int32> StoredEvidence;
    const int32 MaxCapacity = 50;

    EventBus->Subscribe("Evidence.Stored", [&StoredEvidence](const FHorrorEventPayload& Payload) {
        StoredEvidence.Add(Payload.IntData.FindRef("EvidenceID"));
    });

    bool bStorageFull = false;
    EventBus->Subscribe("Evidence.StorageFull", [&bStorageFull](const FHorrorEventPayload& Payload) {
        bStorageFull = true;
    });

    // Fill storage
    for (int32 i = 0; i < MaxCapacity + 5; ++i)
    {
        FHorrorEventPayload Payload;
        if (i < MaxCapacity)
        {
            Payload.EventName = "Evidence.Stored";
            Payload.IntData.Add("EvidenceID", i);
        }
        else
        {
            Payload.EventName = "Evidence.StorageFull";
        }
        EventBus->Publish(Payload);
    }

    TestEqual("Storage capacity respected", StoredEvidence.Num(), MaxCapacity);
    TestTrue("Storage full event triggered", bStorageFull);

    return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
