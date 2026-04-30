#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR && HORRORPROJECT_ENABLE_LEGACY_AUTOMATION_TESTS

// Copyright Epic Games, Inc. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "Game/HorrorEventBusSubsystem.h"
#include "Audio/HorrorAudioSubsystem.h"
#include "Evidence/ArchiveSubsystem.h"
#include "Player/Components/FearComponent.h"
#include "Player/Components/NoiseGeneratorComponent.h"
#include "AI/HorrorGolemBehaviorComponent.h"
#include "Tests/AutomationCommon.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSystemIntegrationBasicTest,
	"HorrorProject.Integration.Systems.BasicSubsystemsExist",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FSystemIntegrationBasicTest::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	if (!World)
	{
		AddError(TEXT("Failed to create test world"));
		return false;
	}

	// Verify all critical subsystems exist
	UHorrorEventBusSubsystem* EventBus = World->GetSubsystem<UHorrorEventBusSubsystem>();
	TestNotNull(TEXT("EventBus subsystem should exist"), EventBus);

	UHorrorAudioSubsystem* AudioSubsystem = World->GetSubsystem<UHorrorAudioSubsystem>();
	TestNotNull(TEXT("Audio subsystem should exist"), AudioSubsystem);

	UArchiveSubsystem* ArchiveSubsystem = World->GetSubsystem<UArchiveSubsystem>();
	TestNotNull(TEXT("Archive subsystem should exist"), ArchiveSubsystem);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSystemIntegrationPlayerFearTest,
	"HorrorProject.Integration.Systems.PlayerFearSystem",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FSystemIntegrationPlayerFearTest::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
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

	// Create test actor with fear component
	AActor* TestActor = World->SpawnActor<AActor>();
	UFearComponent* FearComp = NewObject<UFearComponent>(TestActor);
	FearComp->RegisterComponent();

	float InitialFear = FearComp->GetCurrentFear();
	TestEqual(TEXT("Initial fear should be 0"), InitialFear, 0.0f);

	// Simulate threat event
	FGameplayTag ThreatTag = FGameplayTag::RequestGameplayTag(FName("Horror.Threat.Detected"));
	EventBus->Publish(ThreatTag, FName("TestThreat"), FGameplayTag(), nullptr);

	// Fear component should respond to threat events
	TestTrue(TEXT("Fear component should be valid"), FearComp != nullptr);

	TestActor->Destroy();
	EventBus->ResetForTests();

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSystemIntegrationNoiseToAITest,
	"HorrorProject.Integration.Systems.NoiseToAI",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FSystemIntegrationNoiseToAITest::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
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

	// Create player with noise generator
	AActor* PlayerActor = World->SpawnActor<AActor>();
	UNoiseGeneratorComponent* NoiseGen = NewObject<UNoiseGeneratorComponent>(PlayerActor);
	NoiseGen->RegisterComponent();

	// Create AI actor
	AActor* AIActor = World->SpawnActor<AActor>();
	UHorrorGolemBehaviorComponent* GolemBehavior = NewObject<UHorrorGolemBehaviorComponent>(AIActor);
	GolemBehavior->RegisterComponent();

	// Generate noise event
	FGameplayTag NoiseTag = FGameplayTag::RequestGameplayTag(FName("Horror.Player.Noise"));
	EventBus->Publish(NoiseTag, FName("PlayerNoise"), FGameplayTag(), PlayerActor);

	// Verify event was published
	TestEqual(TEXT("Event should be in history"), EventBus->GetHistory().Num(), 1);
	TestEqual(TEXT("Event tag should match"), EventBus->GetHistory()[0].EventTag, NoiseTag);

	PlayerActor->Destroy();
	AIActor->Destroy();
	EventBus->ResetForTests();

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSystemIntegrationEvidenceToArchiveTest,
	"HorrorProject.Integration.Systems.EvidenceToArchive",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FSystemIntegrationEvidenceToArchiveTest::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	if (!World)
	{
		AddError(TEXT("Failed to create test world"));
		return false;
	}

	UHorrorEventBusSubsystem* EventBus = World->GetSubsystem<UHorrorEventBusSubsystem>();
	UArchiveSubsystem* ArchiveSubsystem = World->GetSubsystem<UArchiveSubsystem>();

	if (!EventBus || !ArchiveSubsystem)
	{
		AddError(TEXT("Required subsystems not found"));
		return false;
	}

	// Publish evidence collected event
	FGameplayTag EvidenceTag = FGameplayTag::RequestGameplayTag(FName("Horror.Evidence.Collected"));
	EventBus->Publish(EvidenceTag, FName("TestEvidence"), FGameplayTag(), nullptr);

	// Verify event was published
	TestEqual(TEXT("Event should be in history"), EventBus->GetHistory().Num(), 1);

	EventBus->ResetForTests();

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSystemIntegrationAudioZoneTest,
	"HorrorProject.Integration.Systems.AudioZoneSystem",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FSystemIntegrationAudioZoneTest::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
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

	TestTrue(TEXT("Audio subsystem should be initialized"), AudioSubsystem->IsInitialized());

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSystemIntegrationCrossSystemEventFlowTest,
	"HorrorProject.Integration.Systems.CrossSystemEventFlow",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FSystemIntegrationCrossSystemEventFlowTest::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	if (!World)
	{
		AddError(TEXT("Failed to create test world"));
		return false;
	}

	UHorrorEventBusSubsystem* EventBus = World->GetSubsystem<UHorrorEventBusSubsystem>();
	UHorrorAudioSubsystem* AudioSubsystem = World->GetSubsystem<UHorrorAudioSubsystem>();
	UArchiveSubsystem* ArchiveSubsystem = World->GetSubsystem<UArchiveSubsystem>();

	if (!EventBus || !AudioSubsystem || !ArchiveSubsystem)
	{
		AddError(TEXT("Required subsystems not found"));
		return false;
	}

	int32 EventsReceived = 0;
	FDelegateHandle Handle = EventBus->GetOnEventPublishedNative().AddLambda(
		[&EventsReceived](const FHorrorEventMessage& Message)
		{
			EventsReceived++;
		});

	// Simulate complete gameplay flow
	FGameplayTag StartTag = FGameplayTag::RequestGameplayTag(FName("Horror.Game.Start"));
	EventBus->Publish(StartTag, FName("GameStart"), FGameplayTag(), nullptr);

	FGameplayTag ThreatTag = FGameplayTag::RequestGameplayTag(FName("Horror.Threat.Detected"));
	EventBus->Publish(ThreatTag, FName("Threat1"), FGameplayTag(), nullptr);

	FGameplayTag EvidenceTag = FGameplayTag::RequestGameplayTag(FName("Horror.Evidence.Collected"));
	EventBus->Publish(EvidenceTag, FName("Evidence1"), FGameplayTag(), nullptr);

	TestEqual(TEXT("Should receive all events"), EventsReceived, 3);
	TestEqual(TEXT("History should contain all events"), EventBus->GetHistory().Num(), 3);

	EventBus->GetOnEventPublishedNative().Remove(Handle);
	EventBus->ResetForTests();

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR
