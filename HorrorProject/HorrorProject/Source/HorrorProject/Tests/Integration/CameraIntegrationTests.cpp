// Copyright Epic Games, Inc. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "Game/HorrorEventBusSubsystem.h"
#include "Player/Components/CameraPhotoComponent.h"
#include "Player/Components/CameraRecordingComponent.h"
#include "Player/Components/CameraBatteryComponent.h"
#include "Tests/AutomationCommon.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCameraIntegrationBasicTest,
	"HorrorProject.Integration.Camera.BasicCameraSystem",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FCameraIntegrationBasicTest::RunTest(const FString& Parameters)
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

	TestNotNull(TEXT("Photo component should be created"), PhotoComp);
	TestTrue(TEXT("Photo component should be registered"), PhotoComp->IsRegistered());

	TestActor->Destroy();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCameraIntegrationBatteryTest,
	"HorrorProject.Integration.Camera.BatterySystem",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FCameraIntegrationBatteryTest::RunTest(const FString& Parameters)
{
	UWorld* World = FAutomationEditorCommonUtils::CreateNewMap();
	if (!World)
	{
		AddError(TEXT("Failed to create test world"));
		return false;
	}

	AActor* TestActor = World->SpawnActor<AActor>();
	UCameraBatteryComponent* BatteryComp = NewObject<UCameraBatteryComponent>(TestActor);
	BatteryComp->RegisterComponent();

	TestNotNull(TEXT("Battery component should be created"), BatteryComp);
	TestTrue(TEXT("Battery component should be registered"), BatteryComp->IsRegistered());

	float InitialCharge = BatteryComp->GetCurrentCharge();
	TestTrue(TEXT("Initial charge should be valid"), InitialCharge >= 0.0f && InitialCharge <= 100.0f);

	TestActor->Destroy();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCameraIntegrationRecordingTest,
	"HorrorProject.Integration.Camera.RecordingSystem",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FCameraIntegrationRecordingTest::RunTest(const FString& Parameters)
{
	UWorld* World = FAutomationEditorCommonUtils::CreateNewMap();
	if (!World)
	{
		AddError(TEXT("Failed to create test world"));
		return false;
	}

	AActor* TestActor = World->SpawnActor<AActor>();
	UCameraRecordingComponent* RecordingComp = NewObject<UCameraRecordingComponent>(TestActor);
	RecordingComp->RegisterComponent();

	TestNotNull(TEXT("Recording component should be created"), RecordingComp);
	TestTrue(TEXT("Recording component should be registered"), RecordingComp->IsRegistered());

	TestActor->Destroy();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCameraIntegrationPhotoWithBatteryTest,
	"HorrorProject.Integration.Camera.PhotoWithBattery",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FCameraIntegrationPhotoWithBatteryTest::RunTest(const FString& Parameters)
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

	UCameraBatteryComponent* BatteryComp = NewObject<UCameraBatteryComponent>(TestActor);
	BatteryComp->RegisterComponent();

	float InitialCharge = BatteryComp->GetCurrentCharge();
	TestTrue(TEXT("Battery should have charge"), InitialCharge > 0.0f);

	TestActor->Destroy();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCameraIntegrationEventPublishTest,
	"HorrorProject.Integration.Camera.EventPublish",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FCameraIntegrationEventPublishTest::RunTest(const FString& Parameters)
{
	UWorld* World = FAutomationEditorCommonUtils::CreateNewMap();
	if (!World)
	{
		AddError(TEXT("Failed to create test world"));
		return false;
	}

	UHorrorEventBusSubsystem* EventBus = World->GetSubsystem<UHorrorEventBusSubsystem>();
	if (!EventBus)
	{
		AddError(TEXT("EventBus subsystem not found"));
		return false;
	}

	AActor* TestActor = World->SpawnActor<AActor>();
	UCameraPhotoComponent* PhotoComp = NewObject<UCameraPhotoComponent>(TestActor);
	PhotoComp->RegisterComponent();

	// Simulate photo taken event
	FGameplayTag PhotoTag = FGameplayTag::RequestGameplayTag(FName("Horror.Camera.PhotoTaken"));
	EventBus->Publish(PhotoTag, FName("TestCamera"), FGameplayTag(), TestActor);

	TestEqual(TEXT("Event should be published"), EventBus->GetHistory().Num(), 1);
	TestEqual(TEXT("Event tag should match"), EventBus->GetHistory()[0].EventTag, PhotoTag);

	TestActor->Destroy();
	EventBus->ResetForTests();

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCameraIntegrationRecordingWithBatteryTest,
	"HorrorProject.Integration.Camera.RecordingWithBattery",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FCameraIntegrationRecordingWithBatteryTest::RunTest(const FString& Parameters)
{
	UWorld* World = FAutomationEditorCommonUtils::CreateNewMap();
	if (!World)
	{
		AddError(TEXT("Failed to create test world"));
		return false;
	}

	AActor* TestActor = World->SpawnActor<AActor>();

	UCameraRecordingComponent* RecordingComp = NewObject<UCameraRecordingComponent>(TestActor);
	RecordingComp->RegisterComponent();

	UCameraBatteryComponent* BatteryComp = NewObject<UCameraBatteryComponent>(TestActor);
	BatteryComp->RegisterComponent();

	float InitialCharge = BatteryComp->GetCurrentCharge();
	TestTrue(TEXT("Battery should have charge for recording"), InitialCharge > 0.0f);

	TestActor->Destroy();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCameraIntegrationAllComponentsTest,
	"HorrorProject.Integration.Camera.AllComponents",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FCameraIntegrationAllComponentsTest::RunTest(const FString& Parameters)
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

	UCameraRecordingComponent* RecordingComp = NewObject<UCameraRecordingComponent>(TestActor);
	RecordingComp->RegisterComponent();

	UCameraBatteryComponent* BatteryComp = NewObject<UCameraBatteryComponent>(TestActor);
	BatteryComp->RegisterComponent();

	TestTrue(TEXT("All camera components should be registered"),
		PhotoComp->IsRegistered() && RecordingComp->IsRegistered() && BatteryComp->IsRegistered());

	TestActor->Destroy();
	return true;
}
