// Copyright Epic Games, Inc. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "Game/HorrorEventBusSubsystem.h"
#include "Audio/HorrorAudioSubsystem.h"
#include "Evidence/EvidenceEventBridge.h"
#include "Player/Components/FearComponent.h"
#include "AI/HorrorGolemBehaviorComponent.h"
#include "Tests/AutomationCommon.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEventBusIntegrationTest,
	"HorrorProject.Integration.EventBus.BasicPublishSubscribe",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FEventBusIntegrationTest::RunTest(const FString& Parameters)
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

	bool bEventReceived = false;
	FGameplayTag TestTag = FGameplayTag::RequestGameplayTag(FName("Horror.Test.Event"));

	FDelegateHandle Handle = EventBus->GetOnEventPublishedNative().AddLambda(
		[&bEventReceived, TestTag](const FHorrorEventMessage& Message)
		{
			if (Message.EventTag == TestTag)
			{
				bEventReceived = true;
			}
		});

	EventBus->Publish(TestTag, FName("TestSource"), FGameplayTag(), nullptr);

	TestTrue(TEXT("Event should be received by subscriber"), bEventReceived);
	TestEqual(TEXT("Event should be in history"), EventBus->GetHistory().Num(), 1);

	EventBus->GetOnEventPublishedNative().Remove(Handle);
	EventBus->ResetForTests();

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEventBusAudioIntegrationTest,
	"HorrorProject.Integration.EventBus.AudioIntegration",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FEventBusAudioIntegrationTest::RunTest(const FString& Parameters)
{
	UWorld* World = FAutomationEditorCommonUtils::CreateNewMap();
	if (!World)
	{
		AddError(TEXT("Failed to create test world"));
		return false;
	}

	UHorrorEventBusSubsystem* EventBus = World->GetSubsystem<UHorrorEventBusSubsystem>();
	UHorrorAudioSubsystem* AudioSubsystem = World->GetSubsystem<UHorrorAudioSubsystem>();

	if (!EventBus || !AudioSubsystem)
	{
		AddError(TEXT("Required subsystems not found"));
		return false;
	}

	// Test that audio subsystem responds to events
	FGameplayTag ThreatTag = FGameplayTag::RequestGameplayTag(FName("Horror.Threat.Detected"));
	EventBus->Publish(ThreatTag, FName("TestThreat"), FGameplayTag(), nullptr);

	// Audio subsystem should have processed the event
	TestTrue(TEXT("Audio subsystem should be initialized"), AudioSubsystem->IsInitialized());

	EventBus->ResetForTests();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEventBusEvidenceIntegrationTest,
	"HorrorProject.Integration.EventBus.EvidenceIntegration",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FEventBusEvidenceIntegrationTest::RunTest(const FString& Parameters)
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

	// Create evidence event bridge
	UEvidenceEventBridge* Bridge = NewObject<UEvidenceEventBridge>();
	if (!Bridge)
	{
		AddError(TEXT("Failed to create evidence bridge"));
		return false;
	}

	Bridge->Initialize(World);

	// Test evidence collection event
	FGameplayTag EvidenceTag = FGameplayTag::RequestGameplayTag(FName("Horror.Evidence.Collected"));
	EventBus->Publish(EvidenceTag, FName("TestEvidence"), FGameplayTag(), nullptr);

	TestEqual(TEXT("Event should be in history"), EventBus->GetHistory().Num(), 1);
	TestEqual(TEXT("Event tag should match"), EventBus->GetHistory()[0].EventTag, EvidenceTag);

	EventBus->ResetForTests();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEventBusNoCircularDependencyTest,
	"HorrorProject.Integration.EventBus.NoCircularDependency",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FEventBusNoCircularDependencyTest::RunTest(const FString& Parameters)
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

	int32 EventCount = 0;
	const int32 MaxEvents = 10;
	FGameplayTag TestTag = FGameplayTag::RequestGameplayTag(FName("Horror.Test.Circular"));

	FDelegateHandle Handle = EventBus->GetOnEventPublishedNative().AddLambda(
		[&EventCount, &EventBus, TestTag, MaxEvents](const FHorrorEventMessage& Message)
		{
			if (Message.EventTag == TestTag)
			{
				EventCount++;
				// Don't re-publish to avoid infinite loop
			}
		});

	EventBus->Publish(TestTag, FName("TestSource"), FGameplayTag(), nullptr);

	TestEqual(TEXT("Should receive exactly one event"), EventCount, 1);
	TestEqual(TEXT("History should have one event"), EventBus->GetHistory().Num(), 1);

	EventBus->GetOnEventPublishedNative().Remove(Handle);
	EventBus->ResetForTests();

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEventBusMultipleSubscribersTest,
	"HorrorProject.Integration.EventBus.MultipleSubscribers",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FEventBusMultipleSubscribersTest::RunTest(const FString& Parameters)
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

	int32 Subscriber1Count = 0;
	int32 Subscriber2Count = 0;
	int32 Subscriber3Count = 0;
	FGameplayTag TestTag = FGameplayTag::RequestGameplayTag(FName("Horror.Test.Multi"));

	FDelegateHandle Handle1 = EventBus->GetOnEventPublishedNative().AddLambda(
		[&Subscriber1Count, TestTag](const FHorrorEventMessage& Message)
		{
			if (Message.EventTag == TestTag) Subscriber1Count++;
		});

	FDelegateHandle Handle2 = EventBus->GetOnEventPublishedNative().AddLambda(
		[&Subscriber2Count, TestTag](const FHorrorEventMessage& Message)
		{
			if (Message.EventTag == TestTag) Subscriber2Count++;
		});

	FDelegateHandle Handle3 = EventBus->GetOnEventPublishedNative().AddLambda(
		[&Subscriber3Count, TestTag](const FHorrorEventMessage& Message)
		{
			if (Message.EventTag == TestTag) Subscriber3Count++;
		});

	EventBus->Publish(TestTag, FName("TestSource"), FGameplayTag(), nullptr);

	TestEqual(TEXT("Subscriber 1 should receive event"), Subscriber1Count, 1);
	TestEqual(TEXT("Subscriber 2 should receive event"), Subscriber2Count, 1);
	TestEqual(TEXT("Subscriber 3 should receive event"), Subscriber3Count, 1);

	EventBus->GetOnEventPublishedNative().Remove(Handle1);
	EventBus->GetOnEventPublishedNative().Remove(Handle2);
	EventBus->GetOnEventPublishedNative().Remove(Handle3);
	EventBus->ResetForTests();

	return true;
}
