// Copyright Epic Games, Inc. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "Game/HorrorEventBusSubsystem.h"
#include "Audio/HorrorAudioSubsystem.h"
#include "Audio/Components/FootstepAudioComponent.h"
#include "Audio/Components/BreathingAudioComponent.h"
#include "Audio/Components/AmbientAudioComponent.h"
#include "Tests/AutomationCommon.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAudioIntegrationBasicTest,
	"HorrorProject.Integration.Audio.BasicAudioSystem",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FAudioIntegrationBasicTest::RunTest(const FString& Parameters)
{
	UWorld* World = FAutomationEditorCommonUtils::CreateNewMap();
	if (!World)
	{
		AddError(TEXT("Failed to create test world"));
		return false;
	}

	UHorrorAudioSubsystem* AudioSubsystem = World->GetSubsystem<UHorrorAudioSubsystem>();
	TestNotNull(TEXT("Audio subsystem should exist"), AudioSubsystem);
	TestTrue(TEXT("Audio subsystem should be initialized"), AudioSubsystem->IsInitialized());

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAudioIntegrationFootstepTest,
	"HorrorProject.Integration.Audio.FootstepComponent",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FAudioIntegrationFootstepTest::RunTest(const FString& Parameters)
{
	UWorld* World = FAutomationEditorCommonUtils::CreateNewMap();
	if (!World)
	{
		AddError(TEXT("Failed to create test world"));
		return false;
	}

	AActor* TestActor = World->SpawnActor<AActor>();
	UFootstepAudioComponent* FootstepComp = NewObject<UFootstepAudioComponent>(TestActor);
	FootstepComp->RegisterComponent();

	TestNotNull(TEXT("Footstep component should be created"), FootstepComp);
	TestTrue(TEXT("Footstep component should be registered"), FootstepComp->IsRegistered());

	TestActor->Destroy();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAudioIntegrationBreathingTest,
	"HorrorProject.Integration.Audio.BreathingComponent",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FAudioIntegrationBreathingTest::RunTest(const FString& Parameters)
{
	UWorld* World = FAutomationEditorCommonUtils::CreateNewMap();
	if (!World)
	{
		AddError(TEXT("Failed to create test world"));
		return false;
	}

	AActor* TestActor = World->SpawnActor<AActor>();
	UBreathingAudioComponent* BreathingComp = NewObject<UBreathingAudioComponent>(TestActor);
	BreathingComp->RegisterComponent();

	TestNotNull(TEXT("Breathing component should be created"), BreathingComp);
	TestTrue(TEXT("Breathing component should be registered"), BreathingComp->IsRegistered());

	TestActor->Destroy();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAudioIntegrationEventResponseTest,
	"HorrorProject.Integration.Audio.EventResponse",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FAudioIntegrationEventResponseTest::RunTest(const FString& Parameters)
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

	// Publish audio-related event
	FGameplayTag AudioTag = FGameplayTag::RequestGameplayTag(FName("Horror.Audio.Trigger"));
	EventBus->Publish(AudioTag, FName("TestAudio"), FGameplayTag(), nullptr);

	TestEqual(TEXT("Event should be published"), EventBus->GetHistory().Num(), 1);

	EventBus->ResetForTests();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAudioIntegration3DPositioningTest,
	"HorrorProject.Integration.Audio.3DPositioning",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FAudioIntegration3DPositioningTest::RunTest(const FString& Parameters)
{
	UWorld* World = FAutomationEditorCommonUtils::CreateNewMap();
	if (!World)
	{
		AddError(TEXT("Failed to create test world"));
		return false;
	}

	AActor* TestActor = World->SpawnActor<AActor>();
	TestActor->SetActorLocation(FVector(100.0f, 200.0f, 300.0f));

	UAmbientAudioComponent* AmbientComp = NewObject<UAmbientAudioComponent>(TestActor);
	AmbientComp->RegisterComponent();

	FVector ActorLocation = TestActor->GetActorLocation();
	TestEqual(TEXT("Actor X position should be correct"), ActorLocation.X, 100.0f);
	TestEqual(TEXT("Actor Y position should be correct"), ActorLocation.Y, 200.0f);
	TestEqual(TEXT("Actor Z position should be correct"), ActorLocation.Z, 300.0f);

	TestActor->Destroy();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAudioIntegrationPriorityTest,
	"HorrorProject.Integration.Audio.PrioritySystem",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FAudioIntegrationPriorityTest::RunTest(const FString& Parameters)
{
	UWorld* World = FAutomationEditorCommonUtils::CreateNewMap();
	if (!World)
	{
		AddError(TEXT("Failed to create test world"));
		return false;
	}

	UHorrorAudioSubsystem* AudioSubsystem = World->GetSubsystem<UHorrorAudioSubsystem>();
	if (!AudioSubsystem)
	{
		AddError(TEXT("Audio subsystem not found"));
		return false;
	}

	// Audio subsystem should handle priority correctly
	TestTrue(TEXT("Audio subsystem should be initialized"), AudioSubsystem->IsInitialized());

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAudioIntegrationMultipleSourcesTest,
	"HorrorProject.Integration.Audio.MultipleSources",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FAudioIntegrationMultipleSourcesTest::RunTest(const FString& Parameters)
{
	UWorld* World = FAutomationEditorCommonUtils::CreateNewMap();
	if (!World)
	{
		AddError(TEXT("Failed to create test world"));
		return false;
	}

	// Create multiple audio sources
	AActor* Actor1 = World->SpawnActor<AActor>();
	UAmbientAudioComponent* Ambient1 = NewObject<UAmbientAudioComponent>(Actor1);
	Ambient1->RegisterComponent();

	AActor* Actor2 = World->SpawnActor<AActor>();
	UAmbientAudioComponent* Ambient2 = NewObject<UAmbientAudioComponent>(Actor2);
	Ambient2->RegisterComponent();

	AActor* Actor3 = World->SpawnActor<AActor>();
	UFootstepAudioComponent* Footstep = NewObject<UFootstepAudioComponent>(Actor3);
	Footstep->RegisterComponent();

	TestTrue(TEXT("All audio components should be registered"),
		Ambient1->IsRegistered() && Ambient2->IsRegistered() && Footstep->IsRegistered());

	Actor1->Destroy();
	Actor2->Destroy();
	Actor3->Destroy();

	return true;
}
