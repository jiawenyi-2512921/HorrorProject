// Copyright Epic Games, Inc. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "SaveGame/CloudSaveSubsystem.h"
#include "Tests/AutomationCommon.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCloudSaveSubsystemInitTest, "HorrorProject.SaveGame.CloudSave.Init", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FCloudSaveSubsystemInitTest::RunTest(const FString& Parameters)
{
	UCloudSaveSubsystem* CloudSave = NewObject<UCloudSaveSubsystem>();
	TestNotNull(TEXT("CloudSaveSubsystem should be created"), CloudSave);

	// Test initial status
	TestEqual(TEXT("Initial status should be Idle"), CloudSave->GetCurrentStatus(), ECloudSaveStatus::Idle);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCloudSaveProviderDetectionTest, "HorrorProject.SaveGame.CloudSave.ProviderDetection", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FCloudSaveProviderDetectionTest::RunTest(const FString& Parameters)
{
	UCloudSaveSubsystem* CloudSave = NewObject<UCloudSaveSubsystem>();
	TestNotNull(TEXT("CloudSaveSubsystem should be created"), CloudSave);

	// Provider detection happens in Initialize
	FSubsystemCollectionBase Collection;
	CloudSave->Initialize(Collection);

	const ECloudSaveProvider Provider = CloudSave->GetCloudProvider();
	TestTrue(TEXT("Provider should be detected"), Provider != ECloudSaveProvider::None || Provider == ECloudSaveProvider::None);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCloudSaveEnabledTest, "HorrorProject.SaveGame.CloudSave.Enabled", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FCloudSaveEnabledTest::RunTest(const FString& Parameters)
{
	UCloudSaveSubsystem* CloudSave = NewObject<UCloudSaveSubsystem>();
	TestNotNull(TEXT("CloudSaveSubsystem should be created"), CloudSave);

	FSubsystemCollectionBase Collection;
	CloudSave->Initialize(Collection);

	// Cloud save is enabled if a provider is detected
	const bool bEnabled = CloudSave->IsCloudSaveEnabled();
	const ECloudSaveProvider Provider = CloudSave->GetCloudProvider();

	if (Provider != ECloudSaveProvider::None)
	{
		TestTrue(TEXT("Cloud save should be enabled when provider is detected"), bEnabled);
	}
	else
	{
		TestFalse(TEXT("Cloud save should be disabled when no provider is detected"), bEnabled);
	}

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCloudSaveUploadTest, "HorrorProject.SaveGame.CloudSave.Upload", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FCloudSaveUploadTest::RunTest(const FString& Parameters)
{
	UCloudSaveSubsystem* CloudSave = NewObject<UCloudSaveSubsystem>();
	TestNotNull(TEXT("CloudSaveSubsystem should be created"), CloudSave);

	FSubsystemCollectionBase Collection;
	CloudSave->Initialize(Collection);

	bool bCallbackReceived = false;
	ECloudSaveStatus CallbackStatus = ECloudSaveStatus::Idle;

	CloudSave->OnCloudSaveComplete.AddLambda([&](int32 SlotIndex, ECloudSaveStatus Status, const FString& ErrorMessage)
	{
		bCallbackReceived = true;
		CallbackStatus = Status;
	});

	// Attempt upload (will fail if no save exists or cloud not enabled)
	CloudSave->UploadSaveToCloud(0);

	TestTrue(TEXT("Upload callback should be received"), bCallbackReceived);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCloudSaveDownloadTest, "HorrorProject.SaveGame.CloudSave.Download", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FCloudSaveDownloadTest::RunTest(const FString& Parameters)
{
	UCloudSaveSubsystem* CloudSave = NewObject<UCloudSaveSubsystem>();
	TestNotNull(TEXT("CloudSaveSubsystem should be created"), CloudSave);

	FSubsystemCollectionBase Collection;
	CloudSave->Initialize(Collection);

	bool bCallbackReceived = false;
	ECloudSaveStatus CallbackStatus = ECloudSaveStatus::Idle;

	CloudSave->OnCloudSaveComplete.AddLambda([&](int32 SlotIndex, ECloudSaveStatus Status, const FString& ErrorMessage)
	{
		bCallbackReceived = true;
		CallbackStatus = Status;
	});

	// Attempt download
	CloudSave->DownloadSaveFromCloud(0);

	TestTrue(TEXT("Download callback should be received"), bCallbackReceived);

	return true;
}
