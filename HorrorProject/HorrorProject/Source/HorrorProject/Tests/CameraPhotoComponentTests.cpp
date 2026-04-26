#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR && HORRORPROJECT_ENABLE_LEGACY_AUTOMATION_TESTS

// Copyright Epic Games, Inc. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "Player/Components/CameraPhotoComponent.h"
#include "Player/Components/QuantumCameraComponent.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCameraPhotoComponentBasicTest, "HorrorProject.CameraPhoto.Basic", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FCameraPhotoComponentBasicTest::RunTest(const FString& Parameters)
{
	UWorld* World = FAutomationEditorCommonUtils::CreateNewMap();
	if (!World)
	{
		AddError(TEXT("Failed to create test world"));
		return false;
	}

	AActor* TestActor = World->SpawnActor<AActor>();
	UCameraPhotoComponent* PhotoComp = NewObject<UCameraPhotoComponent>(TestActor);
	PhotoComp->RegisterComponent();

	TestNotNull(TEXT("PhotoComponent should be created"), PhotoComp);
	TestEqual(TEXT("Initial photo count should be 0"), PhotoComp->GetPhotoCount(), 0);
	TestFalse(TEXT("Storage should not be full initially"), PhotoComp->IsPhotoStorageFull());
	TestTrue(TEXT("Flash should be enabled by default"), PhotoComp->IsFlashEnabled());
	TestTrue(TEXT("Auto-detect evidence should be enabled by default"), PhotoComp->IsAutoDetectEvidenceEnabled());

	World->DestroyActor(TestActor);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCameraPhotoComponentStorageTest, "HorrorProject.CameraPhoto.Storage", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FCameraPhotoComponentStorageTest::RunTest(const FString& Parameters)
{
	UWorld* World = FAutomationEditorCommonUtils::CreateNewMap();
	if (!World)
	{
		AddError(TEXT("Failed to create test world"));
		return false;
	}

	AActor* TestActor = World->SpawnActor<AActor>();
	UCameraPhotoComponent* PhotoComp = NewObject<UCameraPhotoComponent>(TestActor);
	PhotoComp->RegisterComponent();

	const int32 MaxCapacity = PhotoComp->GetMaxPhotoCapacity();
	TestTrue(TEXT("Max capacity should be positive"), MaxCapacity > 0);

	PhotoComp->ClearAllPhotos();
	TestEqual(TEXT("Photo count should be 0 after clear"), PhotoComp->GetPhotoCount(), 0);

	TArray<FCameraPhoto> AllPhotos = PhotoComp->GetAllPhotos();
	TestEqual(TEXT("GetAllPhotos should return empty array"), AllPhotos.Num(), 0);

	World->DestroyActor(TestActor);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCameraPhotoComponentFlashTest, "HorrorProject.CameraPhoto.Flash", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FCameraPhotoComponentFlashTest::RunTest(const FString& Parameters)
{
	UWorld* World = FAutomationEditorCommonUtils::CreateNewMap();
	if (!World)
	{
		AddError(TEXT("Failed to create test world"));
		return false;
	}

	AActor* TestActor = World->SpawnActor<AActor>();
	UCameraPhotoComponent* PhotoComp = NewObject<UCameraPhotoComponent>(TestActor);
	PhotoComp->RegisterComponent();

	PhotoComp->SetFlashEnabled(false);
	TestFalse(TEXT("Flash should be disabled"), PhotoComp->IsFlashEnabled());

	PhotoComp->SetFlashEnabled(true);
	TestTrue(TEXT("Flash should be enabled"), PhotoComp->IsFlashEnabled());

	World->DestroyActor(TestActor);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCameraPhotoComponentEvidenceDetectionTest, "HorrorProject.CameraPhoto.EvidenceDetection", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FCameraPhotoComponentEvidenceDetectionTest::RunTest(const FString& Parameters)
{
	UWorld* World = FAutomationEditorCommonUtils::CreateNewMap();
	if (!World)
	{
		AddError(TEXT("Failed to create test world"));
		return false;
	}

	AActor* TestActor = World->SpawnActor<AActor>();
	UCameraPhotoComponent* PhotoComp = NewObject<UCameraPhotoComponent>(TestActor);
	PhotoComp->RegisterComponent();

	PhotoComp->SetAutoDetectEvidence(false);
	TestFalse(TEXT("Auto-detect should be disabled"), PhotoComp->IsAutoDetectEvidenceEnabled());

	PhotoComp->SetAutoDetectEvidence(true);
	TestTrue(TEXT("Auto-detect should be enabled"), PhotoComp->IsAutoDetectEvidenceEnabled());

	World->DestroyActor(TestActor);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCameraPhotoComponentMetadataTest, "HorrorProject.CameraPhoto.Metadata", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FCameraPhotoComponentMetadataTest::RunTest(const FString& Parameters)
{
	FCameraPhotoMetadata Metadata;

	TestTrue(TEXT("PhotoId should be valid"), Metadata.PhotoId.IsValid());
	TestTrue(TEXT("CaptureTimestamp should be valid"), Metadata.CaptureTimestamp.GetTicks() > 0);
	TestEqual(TEXT("Default exposure should be 1.0"), Metadata.ExposureValue, 1.0f);
	TestFalse(TEXT("Flash should not be used by default"), Metadata.bFlashUsed);
	TestEqual(TEXT("DetectedEvidenceIds should be empty"), Metadata.DetectedEvidenceIds.Num(), 0);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCameraPhotoComponentIntegrationTest, "HorrorProject.CameraPhoto.Integration", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FCameraPhotoComponentIntegrationTest::RunTest(const FString& Parameters)
{
	UWorld* World = FAutomationEditorCommonUtils::CreateNewMap();
	if (!World)
	{
		AddError(TEXT("Failed to create test world"));
		return false;
	}

	AActor* TestActor = World->SpawnActor<AActor>();

	UQuantumCameraComponent* QuantumComp = NewObject<UQuantumCameraComponent>(TestActor);
	QuantumComp->RegisterComponent();

	UCameraPhotoComponent* PhotoComp = NewObject<UCameraPhotoComponent>(TestActor);
	PhotoComp->RegisterComponent();

	TestNotNull(TEXT("QuantumCameraComponent should be created"), QuantumComp);
	TestNotNull(TEXT("CameraPhotoComponent should be created"), PhotoComp);

	QuantumComp->SetCameraAcquired(true);
	QuantumComp->SetCameraEnabled(false);
	TestFalse(TEXT("Cannot take photo when camera disabled"), PhotoComp->CanTakePhoto());

	QuantumComp->SetCameraEnabled(true);
	TestTrue(TEXT("Can take photo when camera enabled"), PhotoComp->CanTakePhoto());

	World->DestroyActor(TestActor);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCameraPhotoComponentDelegateTest, "HorrorProject.CameraPhoto.Delegates", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FCameraPhotoComponentDelegateTest::RunTest(const FString& Parameters)
{
	UWorld* World = FAutomationEditorCommonUtils::CreateNewMap();
	if (!World)
	{
		AddError(TEXT("Failed to create test world"));
		return false;
	}

	AActor* TestActor = World->SpawnActor<AActor>();
	UCameraPhotoComponent* PhotoComp = NewObject<UCameraPhotoComponent>(TestActor);
	PhotoComp->RegisterComponent();

	int32 PhotoTakenCount = 0;
	int32 FlashFiredCount = 0;
	int32 PhotoStoredCount = 0;

	PhotoComp->OnPhotoTaken.AddLambda([&PhotoTakenCount](const FCameraPhoto& Photo) {
		PhotoTakenCount++;
	});

	PhotoComp->OnFlashFired.AddLambda([&FlashFiredCount](float Intensity) {
		FlashFiredCount++;
	});

	PhotoComp->OnPhotoStored.AddLambda([&PhotoStoredCount](FGuid PhotoId) {
		PhotoStoredCount++;
	});

	TestEqual(TEXT("PhotoTaken delegate should not fire yet"), PhotoTakenCount, 0);
	TestEqual(TEXT("FlashFired delegate should not fire yet"), FlashFiredCount, 0);
	TestEqual(TEXT("PhotoStored delegate should not fire yet"), PhotoStoredCount, 0);

	World->DestroyActor(TestActor);
	return true;
}

#endif

#endif // WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR
