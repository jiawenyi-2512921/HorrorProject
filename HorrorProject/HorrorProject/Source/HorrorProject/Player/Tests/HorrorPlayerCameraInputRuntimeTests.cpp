// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/HorrorPlayerCharacter.h"

#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR

#include "Camera/CameraComponent.h"
#include "Misc/AutomationTest.h"
#include "Player/Components/CameraPhotoComponent.h"
#include "Player/Components/CameraRecordingComponent.h"
#include "Player/Components/QuantumCameraComponent.h"
#include "Tests/AutomationCommon.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorPlayerCameraInputStartsRecordingBufferTest,
	"HorrorProject.Player.CameraInput.StartsRecordingBuffer",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorPlayerCameraInputStartsRecordingBufferTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for camera input coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	AHorrorPlayerCharacter* PlayerCharacter = World->SpawnActor<AHorrorPlayerCharacter>();
	TestNotNull(TEXT("Camera input test should spawn a horror player character."), PlayerCharacter);
	if (!PlayerCharacter)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	UQuantumCameraComponent* QuantumCamera = PlayerCharacter->GetQuantumCameraComponent();
	UCameraRecordingComponent* Recording = PlayerCharacter->GetCameraRecordingComponent();
	TestNotNull(TEXT("Camera input test should expose the quantum camera."), QuantumCamera);
	TestNotNull(TEXT("Camera input test should expose the recording buffer."), Recording);
	if (!QuantumCamera || !Recording)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	TestTrue(TEXT("Camera input test should acquire the camera."), QuantumCamera->SetCameraAcquired(true));
	TestTrue(TEXT("Camera input test should enable the camera."), QuantumCamera->SetCameraEnabled(true));

	PlayerCharacter->StartRecordForTests();
	TestTrue(TEXT("Record input should switch the quantum camera to recording."), QuantumCamera->IsCameraMode(EQuantumCameraMode::Recording));
	TestTrue(TEXT("Record input should start the recording buffer."), Recording->IsRecording());

	Recording->TickComponent(0.1f, ELevelTick::LEVELTICK_All, nullptr);
	TestTrue(TEXT("Recording buffer should capture frames while the input is held."), Recording->HasRecording());

	PlayerCharacter->StopRecordForTests();
	TestFalse(TEXT("Stop record input should stop the recording buffer."), Recording->IsRecording());
	TestTrue(TEXT("Stopped recording should keep the captured buffer for rewind/archive use."), Recording->HasRecording());

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorPlayerCameraInputTakePhotoStoresPhotoTest,
	"HorrorProject.Player.CameraInput.TakePhotoStoresPhoto",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorPlayerCameraInputTakePhotoStoresPhotoTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for photo input coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	AHorrorPlayerCharacter* PlayerCharacter = World->SpawnActor<AHorrorPlayerCharacter>();
	TestNotNull(TEXT("Photo input test should spawn a horror player character."), PlayerCharacter);
	if (!PlayerCharacter)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	TestNotNull(TEXT("Photo input test should expose a first-person camera."), PlayerCharacter->FindComponentByClass<UCameraComponent>());
	UQuantumCameraComponent* QuantumCamera = PlayerCharacter->GetQuantumCameraComponent();
	UCameraPhotoComponent* CameraPhoto = PlayerCharacter->GetCameraPhotoComponent();
	TestNotNull(TEXT("Photo input test should expose the quantum camera."), QuantumCamera);
	TestNotNull(TEXT("Photo input test should expose the photo component."), CameraPhoto);
	if (!QuantumCamera || !CameraPhoto)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	TestTrue(TEXT("Photo input test should acquire the camera."), QuantumCamera->SetCameraAcquired(true));
	TestTrue(TEXT("Photo input test should enable the camera."), QuantumCamera->SetCameraEnabled(true));
	CameraPhoto->SetFlashEnabled(false);
	CameraPhoto->SetAutoDetectEvidence(false);

	PlayerCharacter->TakePhotoForTests();
	TestEqual(TEXT("Photo input should store one photo."), CameraPhoto->GetPhotoCount(), 1);
	TestEqual(TEXT("Photo input should return the quantum camera to viewfinder."), QuantumCamera->GetCameraMode(), EQuantumCameraMode::Viewfinder);

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorPlayerCameraInputRecordingBufferFollowsQuantumCameraModeTest,
	"HorrorProject.Player.CameraInput.RecordingBufferFollowsQuantumCameraMode",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorPlayerCameraInputRecordingBufferFollowsQuantumCameraModeTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for camera mode recording sync coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	AHorrorPlayerCharacter* PlayerCharacter = World->SpawnActor<AHorrorPlayerCharacter>();
	TestNotNull(TEXT("Camera mode sync test should spawn a horror player character."), PlayerCharacter);
	if (!PlayerCharacter)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	UQuantumCameraComponent* QuantumCamera = PlayerCharacter->GetQuantumCameraComponent();
	UCameraRecordingComponent* Recording = PlayerCharacter->GetCameraRecordingComponent();
	TestNotNull(TEXT("Camera mode sync test should expose the quantum camera."), QuantumCamera);
	TestNotNull(TEXT("Camera mode sync test should expose the recording buffer."), Recording);
	if (!QuantumCamera || !Recording)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	TestTrue(TEXT("Camera mode sync test should acquire the camera."), QuantumCamera->SetCameraAcquired(true));
	TestTrue(TEXT("Camera mode sync test should enable the camera."), QuantumCamera->SetCameraEnabled(true));

	TestTrue(TEXT("Starting the quantum camera recording mode should succeed."), QuantumCamera->StartRecording());
	TestTrue(TEXT("Recording buffer should follow the quantum camera recording mode."), Recording->IsRecording());

	Recording->TickComponent(0.1f, ELevelTick::LEVELTICK_All, nullptr);
	TestTrue(TEXT("Recording buffer should retain captured frames after mode-driven recording."), Recording->HasRecording());

	TestTrue(TEXT("Stopping the quantum camera recording mode should succeed."), QuantumCamera->StopRecording());
	TestFalse(TEXT("Recording buffer should stop when quantum camera leaves recording mode."), Recording->IsRecording());
	TestTrue(TEXT("Stopping mode-driven recording should keep the captured buffer."), Recording->HasRecording());

	TestTrue(TEXT("Starting a second quantum camera recording session should succeed."), QuantumCamera->StartRecording());
	TestTrue(TEXT("Recording buffer should restart for the second mode-driven session."), Recording->IsRecording());
	TestTrue(TEXT("Disabling the camera should leave recording mode."), QuantumCamera->SetCameraEnabled(false));
	TestFalse(TEXT("Recording buffer should stop when camera is disabled."), Recording->IsRecording());

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

#endif
