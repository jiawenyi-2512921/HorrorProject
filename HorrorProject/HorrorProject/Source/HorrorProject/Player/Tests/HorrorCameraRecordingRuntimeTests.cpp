// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/HorrorPlayerCharacter.h"

#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR

#include "Misc/AutomationTest.h"
#include "Player/Components/CameraRecordingComponent.h"
#include "Player/Components/QuantumCameraComponent.h"
#include "Tests/AutomationCommon.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorCameraRecordingKeepsChronologicalFramesWhenBufferWrapsTest,
	"HorrorProject.Player.CameraRecording.KeepsChronologicalFramesWhenBufferWraps",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorCameraRecordingKeepsChronologicalFramesWhenBufferWrapsTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for camera recording ring-buffer coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	AHorrorPlayerCharacter* PlayerCharacter = World->SpawnActor<AHorrorPlayerCharacter>();
	TestNotNull(TEXT("Camera recording ring-buffer test should spawn a horror player character."), PlayerCharacter);
	if (!PlayerCharacter)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	UQuantumCameraComponent* QuantumCamera = PlayerCharacter->GetQuantumCameraComponent();
	UCameraRecordingComponent* Recording = PlayerCharacter->GetCameraRecordingComponent();
	TestNotNull(TEXT("Camera recording ring-buffer test should expose the quantum camera."), QuantumCamera);
	TestNotNull(TEXT("Camera recording ring-buffer test should expose the recording component."), Recording);
	if (!QuantumCamera || !Recording)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	TestTrue(TEXT("Camera recording ring-buffer test should acquire the camera."), QuantumCamera->SetCameraAcquired(true));
	TestTrue(TEXT("Camera recording ring-buffer test should enable the camera."), QuantumCamera->SetCameraEnabled(true));
	Recording->SetMaxRecordingDuration(10.0f);
	Recording->SetMaxBufferFramesForTests(3);

	TestTrue(TEXT("Camera recording ring-buffer test should start recording."), Recording->StartRecording());
	for (int32 FrameIndex = 0; FrameIndex < 5; ++FrameIndex)
	{
		const float FrameX = static_cast<float>((FrameIndex + 1) * 100);
		PlayerCharacter->SetActorLocation(FVector(FrameX, 0.0f, 0.0f));
		Recording->TickComponent(0.05f, ELevelTick::LEVELTICK_All, nullptr);
	}
	TestTrue(TEXT("Camera recording ring-buffer test should stop recording."), Recording->StopRecording());

	TestEqual(TEXT("Wrapped recording buffer should keep only the newest frames."), Recording->GetRecordedFrameCountForTests(), 3);

	FCameraRecordingFrame OldestKeptFrame;
	TestTrue(TEXT("Wrapped recording buffer should expose the oldest kept frame."), Recording->GetRecordedFrameForTests(0, OldestKeptFrame));
	TestEqual(TEXT("Oldest kept frame should be frame 3 in chronological order."), OldestKeptFrame.Location.X, 300.0);

	FCameraRecordingFrame MiddleKeptFrame;
	TestTrue(TEXT("Wrapped recording buffer should expose the middle kept frame."), Recording->GetRecordedFrameForTests(1, MiddleKeptFrame));
	TestEqual(TEXT("Middle kept frame should be frame 4 in chronological order."), MiddleKeptFrame.Location.X, 400.0);

	FCameraRecordingFrame NewestKeptFrame;
	TestTrue(TEXT("Wrapped recording buffer should expose the newest kept frame."), Recording->GetRecordedFrameForTests(2, NewestKeptFrame));
	TestEqual(TEXT("Newest kept frame should be frame 5 in chronological order."), NewestKeptFrame.Location.X, 500.0);

	const FCameraRecordingMetadata Metadata = Recording->GetRecordingMetadata();
	TestEqual(TEXT("Recording metadata should report the retained frame count."), Metadata.FrameCount, 3);
	TestEqual(TEXT("Recording metadata should start at the oldest retained frame."), Metadata.StartLocation.X, 300.0);

	TestTrue(TEXT("Wrapped recording buffer should remain rewindable."), Recording->StartRewind());
	Recording->TickComponent(0.05f, ELevelTick::LEVELTICK_All, nullptr);

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

#endif
