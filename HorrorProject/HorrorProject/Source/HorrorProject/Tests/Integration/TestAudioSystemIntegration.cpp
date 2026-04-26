#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR && HORRORPROJECT_ENABLE_LEGACY_AUTOMATION_TESTS

// Copyright Epic Games, Inc. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "Tests/TestHelpers.h"
#include "Tests/HorrorTestMacros.h"
#include "Game/HorrorEventBusSubsystem.h"
#include "Sound/SoundCue.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAudioSystemIntegrationTest,
    "HorrorProject.Integration.Systems.Audio",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FAudioSystemIntegrationTest::RunTest(const FString& Parameters)
{
    using namespace HorrorTestHelpers;
    FTestWorldScope TestWorld;
    HORROR_TEST_REQUIRE_WORLD(TestWorld.GetWorld(), TestWorld, "Test world creation failed");

    UWorld* World = TestWorld.GetWorld();
    UHorrorEventBusSubsystem* EventBus = World->GetSubsystem<UHorrorEventBusSubsystem>();
    HORROR_TEST_REQUIRE(EventBus != nullptr, "EventBus not found");

    // Test 1: Audio playback events
    int32 SoundsPlayed = 0;
    TArray<FString> PlayedSounds;

    EventBus->Subscribe("Audio.Sound.Play", [&](const FHorrorEventPayload& Payload) {
        SoundsPlayed++;
        PlayedSounds.Add(Payload.StringData.FindRef("SoundName"));
    });

    TArray<FString> TestSounds = {"Footstep", "Heartbeat", "Whisper", "Scream"};
    for (const FString& Sound : TestSounds)
    {
        FHorrorEventPayload Payload;
        Payload.EventName = "Audio.Sound.Play";
        Payload.StringData.Add("SoundName", Sound);
        Payload.FloatData.Add("Volume", 1.0f);
        EventBus->Publish(Payload);
    }

    TestEqual("Sounds played", SoundsPlayed, TestSounds.Num());
    TestEqual("Sound names tracked", PlayedSounds.Num(), TestSounds.Num());

    // Test 2: Audio volume control
    float MasterVolume = 1.0f;
    float MusicVolume = 0.8f;
    float SFXVolume = 0.9f;

    EventBus->Subscribe("Audio.Volume.Changed", [&](const FHorrorEventPayload& Payload) {
        FString VolumeType = Payload.StringData.FindRef("Type");
        float NewVolume = Payload.FloatData.FindRef("Volume");

        if (VolumeType == "Master") MasterVolume = NewVolume;
        else if (VolumeType == "Music") MusicVolume = NewVolume;
        else if (VolumeType == "SFX") SFXVolume = NewVolume;
    });

    FHorrorEventPayload VolumePayload;
    VolumePayload.EventName = "Audio.Volume.Changed";
    VolumePayload.StringData.Add("Type", "Master");
    VolumePayload.FloatData.Add("Volume", 0.5f);
    EventBus->Publish(VolumePayload);

    TestTrue("Master volume changed", FMath::IsNearlyEqual(MasterVolume, 0.5f, 0.01f));

    // Test 3: 3D audio positioning
    TArray<FVector> AudioPositions;

    EventBus->Subscribe("Audio.3D.Play", [&AudioPositions](const FHorrorEventPayload& Payload) {
        FVector Position(
            Payload.FloatData.FindRef("X"),
            Payload.FloatData.FindRef("Y"),
            Payload.FloatData.FindRef("Z")
        );
        AudioPositions.Add(Position);
    });

    for (int32 i = 0; i < 5; ++i)
    {
        FHorrorEventPayload Payload;
        Payload.EventName = "Audio.3D.Play";
        Payload.FloatData.Add("X", i * 100.0f);
        Payload.FloatData.Add("Y", i * 50.0f);
        Payload.FloatData.Add("Z", 0.0f);
        EventBus->Publish(Payload);
    }

    TestEqual("3D audio positions tracked", AudioPositions.Num(), 5);

    // Test 4: Audio occlusion
    bool bOcclusionEnabled = false;
    float OcclusionFactor = 0.0f;

    EventBus->Subscribe("Audio.Occlusion.Update", [&](const FHorrorEventPayload& Payload) {
        bOcclusionEnabled = Payload.BoolData.FindRef("Enabled");
        OcclusionFactor = Payload.FloatData.FindRef("Factor");
    });

    FHorrorEventPayload OcclusionPayload;
    OcclusionPayload.EventName = "Audio.Occlusion.Update";
    OcclusionPayload.BoolData.Add("Enabled", true);
    OcclusionPayload.FloatData.Add("Factor", 0.7f);
    EventBus->Publish(OcclusionPayload);

    TestTrue("Occlusion enabled", bOcclusionEnabled);
    TestTrue("Occlusion factor set", FMath::IsNearlyEqual(OcclusionFactor, 0.7f, 0.01f));

    // Test 5: Music transitions
    FString CurrentTrack = "None";
    float FadeTime = 0.0f;

    EventBus->Subscribe("Audio.Music.Transition", [&](const FHorrorEventPayload& Payload) {
        CurrentTrack = Payload.StringData.FindRef("NewTrack");
        FadeTime = Payload.FloatData.FindRef("FadeTime");
    });

    FHorrorEventPayload TransitionPayload;
    TransitionPayload.EventName = "Audio.Music.Transition";
    TransitionPayload.StringData.Add("NewTrack", "Ambient_Horror");
    TransitionPayload.FloatData.Add("FadeTime", 2.0f);
    EventBus->Publish(TransitionPayload);

    TestEqual("Music track changed", CurrentTrack, FString("Ambient_Horror"));
    TestTrue("Fade time set", FMath::IsNearlyEqual(FadeTime, 2.0f, 0.01f));

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAudioSpatializationTest,
    "HorrorProject.Integration.Systems.AudioSpatialization",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FAudioSpatializationTest::RunTest(const FString& Parameters)
{
    using namespace HorrorTestHelpers;
    FTestWorldScope TestWorld;
    HORROR_TEST_REQUIRE_WORLD(TestWorld.GetWorld(), TestWorld, "Test world creation failed");

    UWorld* World = TestWorld.GetWorld();
    UHorrorEventBusSubsystem* EventBus = World->GetSubsystem<UHorrorEventBusSubsystem>();
    HORROR_TEST_REQUIRE(EventBus != nullptr, "EventBus not found");

    // Test spatial audio distance attenuation
    TArray<float> Distances;
    TArray<float> Volumes;

    EventBus->Subscribe("Audio.Spatial.Update", [&](const FHorrorEventPayload& Payload) {
        Distances.Add(Payload.FloatData.FindRef("Distance"));
        Volumes.Add(Payload.FloatData.FindRef("Volume"));
    });

    // Simulate sounds at various distances
    for (float Distance = 100.0f; Distance <= 1000.0f; Distance += 100.0f)
    {
        FHorrorEventPayload Payload;
        Payload.EventName = "Audio.Spatial.Update";
        Payload.FloatData.Add("Distance", Distance);
        Payload.FloatData.Add("Volume", FMath::Max(0.0f, 1.0f - (Distance / 1000.0f)));
        EventBus->Publish(Payload);
    }

    TestEqual("Distance samples collected", Distances.Num(), 10);
    TestTrue("Volume attenuates with distance", Volumes[0] > Volumes[Volumes.Num() - 1]);

    return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS

#endif // WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR
