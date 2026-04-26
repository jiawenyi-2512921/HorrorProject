// Copyright Epic Games, Inc. All Rights Reserved.

#include "CameraRecordingComponent.h"
#include "QuantumCameraComponent.h"
#include "Misc/AutomationTest.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCameraRecordingComponentBasicRecordingTest,
	"HorrorProject.Player.CameraRecording.BasicRecording",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FCameraRecordingComponentBasicRecordingTest::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	AActor* TestActor = World->SpawnActor<AActor>();

	UQuantumCameraComponent* CameraComponent = NewObject<UQuantumCameraComponent>(TestActor);
	CameraComponent->RegisterComponent();

	UCameraRecordingComponent* RecordingComponent = NewObject<UCameraRecordingComponent>(TestActor);
	RecordingComponent->RegisterComponent();

	TestFalse(TEXT("Recording should not start without camera enabled"), RecordingComponent->StartRecording());
	TestFalse(TEXT("Recording state should be false"), RecordingComponent->IsRecording());

	CameraComponent->SetCameraAcquired(true);
	CameraComponent->SetCameraEnabled(true);

	TestTrue(TEXT("Recording should start with camera enabled"), RecordingComponent->StartRecording());
	TestTrue(TEXT("Recording state should be true"), RecordingComponent->IsRecording());
	TestEqual(TEXT("Initial recording duration should be 0"), RecordingComponent->GetCurrentRecordingDuration(), 0.0f);

	TestTrue(TEXT("Recording should stop successfully"), RecordingComponent->StopRecording());
	TestFalse(TEXT("Recording state should be false after stop"), RecordingComponent->IsRecording());

	World->DestroyWorld(false);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCameraRecordingComponentRecordingProgressTest,
	"HorrorProject.Player.CameraRecording.RecordingProgress",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FCameraRecordingComponentRecordingProgressTest::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	AActor* TestActor = World->SpawnActor<AActor>();

	UQuantumCameraComponent* CameraComponent = NewObject<UQuantumCameraComponent>(TestActor);
	CameraComponent->RegisterComponent();

	UCameraRecordingComponent* RecordingComponent = NewObject<UCameraRecordingComponent>(TestActor);
	RecordingComponent->RegisterComponent();
	RecordingComponent->SetMaxRecordingDuration(10.0f);

	CameraComponent->SetCameraAcquired(true);
	CameraComponent->SetCameraEnabled(true);
	RecordingComponent->StartRecording();

	RecordingComponent->TickComponent(1.0f, ELevelTick::LEVELTICK_All, nullptr);
	TestTrue(TEXT("Recording duration should increase"), RecordingComponent->GetCurrentRecordingDuration() > 0.0f);
	TestTrue(TEXT("Recording progress should be > 0"), RecordingComponent->GetRecordingProgress() > 0.0f);
	TestTrue(TEXT("Should have recording data"), RecordingComponent->HasRecording());

	World->DestroyWorld(false);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCameraRecordingComponentMaxDurationTest,
	"HorrorProject.Player.CameraRecording.MaxDuration",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FCameraRecordingComponentMaxDurationTest::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	AActor* TestActor = World->SpawnActor<AActor>();

	UQuantumCameraComponent* CameraComponent = NewObject<UQuantumCameraComponent>(TestActor);
	CameraComponent->RegisterComponent();

	UCameraRecordingComponent* RecordingComponent = NewObject<UCameraRecordingComponent>(TestActor);
	RecordingComponent->RegisterComponent();
	RecordingComponent->SetMaxRecordingDuration(2.0f);

	CameraComponent->SetCameraAcquired(true);
	CameraComponent->SetCameraEnabled(true);
	RecordingComponent->StartRecording();

	RecordingComponent->TickComponent(3.0f, ELevelTick::LEVELTICK_All, nullptr);
	TestFalse(TEXT("Recording should auto-stop at max duration"), RecordingComponent->IsRecording());

	World->DestroyWorld(false);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCameraRecordingComponentRewindTest,
	"HorrorProject.Player.CameraRecording.Rewind",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FCameraRecordingComponentRewindTest::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	AActor* TestActor = World->SpawnActor<AActor>();

	UQuantumCameraComponent* CameraComponent = NewObject<UQuantumCameraComponent>(TestActor);
	CameraComponent->RegisterComponent();

	UCameraRecordingComponent* RecordingComponent = NewObject<UCameraRecordingComponent>(TestActor);
	RecordingComponent->RegisterComponent();

	CameraComponent->SetCameraAcquired(true);
	CameraComponent->SetCameraEnabled(true);

	TestFalse(TEXT("Rewind should fail without recording"), RecordingComponent->StartRewind());

	RecordingComponent->StartRecording();
	RecordingComponent->TickComponent(1.0f, ELevelTick::LEVELTICK_All, nullptr);
	RecordingComponent->StopRecording();

	TestTrue(TEXT("Rewind should start with recording data"), RecordingComponent->StartRewind());
	TestTrue(TEXT("Rewind state should be true"), RecordingComponent->IsRewinding());

	TestTrue(TEXT("Rewind should stop successfully"), RecordingComponent->StopRewind());
	TestFalse(TEXT("Rewind state should be false after stop"), RecordingComponent->IsRewinding());

	World->DestroyWorld(false);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCameraRecordingComponentClearRecordingTest,
	"HorrorProject.Player.CameraRecording.ClearRecording",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FCameraRecordingComponentClearRecordingTest::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	AActor* TestActor = World->SpawnActor<AActor>();

	UQuantumCameraComponent* CameraComponent = NewObject<UQuantumCameraComponent>(TestActor);
	CameraComponent->RegisterComponent();

	UCameraRecordingComponent* RecordingComponent = NewObject<UCameraRecordingComponent>(TestActor);
	RecordingComponent->RegisterComponent();

	CameraComponent->SetCameraAcquired(true);
	CameraComponent->SetCameraEnabled(true);
	RecordingComponent->StartRecording();
	RecordingComponent->TickComponent(1.0f, ELevelTick::LEVELTICK_All, nullptr);
	RecordingComponent->StopRecording();

	TestTrue(TEXT("Should have recording before clear"), RecordingComponent->HasRecording());
	RecordingComponent->ClearRecording();
	TestFalse(TEXT("Should not have recording after clear"), RecordingComponent->HasRecording());
	TestEqual(TEXT("Duration should be 0 after clear"), RecordingComponent->GetCurrentRecordingDuration(), 0.0f);

	World->DestroyWorld(false);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCameraRecordingComponentMetadataTest,
	"HorrorProject.Player.CameraRecording.Metadata",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FCameraRecordingComponentMetadataTest::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	AActor* TestActor = World->SpawnActor<AActor>();

	UQuantumCameraComponent* CameraComponent = NewObject<UQuantumCameraComponent>(TestActor);
	CameraComponent->RegisterComponent();

	UCameraRecordingComponent* RecordingComponent = NewObject<UCameraRecordingComponent>(TestActor);
	RecordingComponent->RegisterComponent();

	CameraComponent->SetCameraAcquired(true);
	CameraComponent->SetCameraEnabled(true);
	RecordingComponent->StartRecording();
	RecordingComponent->TickComponent(1.0f, ELevelTick::LEVELTICK_All, nullptr);
	RecordingComponent->StopRecording();

	FCameraRecordingMetadata Metadata = RecordingComponent->GetRecordingMetadata();
	TestTrue(TEXT("Metadata should have duration > 0"), Metadata.TotalDuration > 0.0f);
	TestTrue(TEXT("Metadata should have frames"), Metadata.FrameCount > 0);

	World->DestroyWorld(false);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCameraRecordingComponentDelegatesTest,
	"HorrorProject.Player.CameraRecording.Delegates",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FCameraRecordingComponentDelegatesTest::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	AActor* TestActor = World->SpawnActor<AActor>();

	UQuantumCameraComponent* CameraComponent = NewObject<UQuantumCameraComponent>(TestActor);
	CameraComponent->RegisterComponent();

	UCameraRecordingComponent* RecordingComponent = NewObject<UCameraRecordingComponent>(TestActor);
	RecordingComponent->RegisterComponent();

	bool bRecordingStartedCalled = false;
	bool bRecordingStoppedCalled = false;
	float RecordedDuration = 0.0f;

	RecordingComponent->OnRecordingStarted.AddLambda([&bRecordingStartedCalled]() {
		bRecordingStartedCalled = true;
	});

	RecordingComponent->OnRecordingStopped.AddLambda([&bRecordingStoppedCalled, &RecordedDuration](float Duration) {
		bRecordingStoppedCalled = true;
		RecordedDuration = Duration;
	});

	CameraComponent->SetCameraAcquired(true);
	CameraComponent->SetCameraEnabled(true);
	RecordingComponent->StartRecording();

	TestTrue(TEXT("OnRecordingStarted should be called"), bRecordingStartedCalled);

	RecordingComponent->TickComponent(1.0f, ELevelTick::LEVELTICK_All, nullptr);
	RecordingComponent->StopRecording();

	TestTrue(TEXT("OnRecordingStopped should be called"), bRecordingStoppedCalled);
	TestTrue(TEXT("Recorded duration should be > 0"), RecordedDuration > 0.0f);

	World->DestroyWorld(false);
	return true;
}

#endif
