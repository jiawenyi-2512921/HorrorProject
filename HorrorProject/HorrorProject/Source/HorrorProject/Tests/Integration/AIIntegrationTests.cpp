#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR && HORRORPROJECT_ENABLE_LEGACY_AUTOMATION_TESTS

// Copyright Epic Games, Inc. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "Game/HorrorEventBusSubsystem.h"
#include "AI/HorrorGolemBehaviorComponent.h"
#include "AI/HorrorThreatAIController.h"
#include "AI/HorrorThreatCharacter.h"
#include "Player/Components/NoiseGeneratorComponent.h"
#include "Tests/AutomationCommon.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAIIntegrationBasicTest,
	"HorrorProject.Integration.AI.BasicAISystem",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FAIIntegrationBasicTest::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	if (!World)
	{
		AddError(TEXT("Failed to create test world"));
		return false;
	}

	AActor* TestActor = World->SpawnActor<AActor>();
	UHorrorGolemBehaviorComponent* GolemBehavior = NewObject<UHorrorGolemBehaviorComponent>(TestActor);
	GolemBehavior->RegisterComponent();

	TestNotNull(TEXT("Golem behavior component should be created"), GolemBehavior);
	TestTrue(TEXT("Golem behavior component should be registered"), GolemBehavior->IsRegistered());

	TestActor->Destroy();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAIIntegrationNoisePerceptionTest,
	"HorrorProject.Integration.AI.NoisePerception",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FAIIntegrationNoisePerceptionTest::RunTest(const FString& Parameters)
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

	// Create AI actor
	AActor* AIActor = World->SpawnActor<AActor>();
	UHorrorGolemBehaviorComponent* GolemBehavior = NewObject<UHorrorGolemBehaviorComponent>(AIActor);
	GolemBehavior->RegisterComponent();

	// Create player with noise generator
	AActor* PlayerActor = World->SpawnActor<AActor>();
	UNoiseGeneratorComponent* NoiseGen = NewObject<UNoiseGeneratorComponent>(PlayerActor);
	NoiseGen->RegisterComponent();

	// Publish noise event
	FGameplayTag NoiseTag = FGameplayTag::RequestGameplayTag(FName("Horror.Player.Noise"));
	EventBus->Publish(NoiseTag, FName("PlayerNoise"), FGameplayTag(), PlayerActor);

	TestEqual(TEXT("Noise event should be published"), EventBus->GetHistory().Num(), 1);

	AIActor->Destroy();
	PlayerActor->Destroy();
	EventBus->ResetForTests();

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAIIntegrationStateTransitionTest,
	"HorrorProject.Integration.AI.StateTransition",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FAIIntegrationStateTransitionTest::RunTest(const FString& Parameters)
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

	AActor* AIActor = World->SpawnActor<AActor>();
	UHorrorGolemBehaviorComponent* GolemBehavior = NewObject<UHorrorGolemBehaviorComponent>(AIActor);
	GolemBehavior->RegisterComponent();

	// Publish state change event
	FGameplayTag StateTag = FGameplayTag::RequestGameplayTag(FName("Horror.AI.StateChange"));
	EventBus->Publish(StateTag, FName("AIState"), FGameplayTag(), AIActor);

	TestEqual(TEXT("State change event should be published"), EventBus->GetHistory().Num(), 1);

	AIActor->Destroy();
	EventBus->ResetForTests();

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAIIntegrationEncounterDirectorTest,
	"HorrorProject.Integration.AI.EncounterDirector",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FAIIntegrationEncounterDirectorTest::RunTest(const FString& Parameters)
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

	// Publish encounter event
	FGameplayTag EncounterTag = FGameplayTag::RequestGameplayTag(FName("Horror.Encounter.Start"));
	EventBus->Publish(EncounterTag, FName("Encounter1"), FGameplayTag(), nullptr);

	TestEqual(TEXT("Encounter event should be published"), EventBus->GetHistory().Num(), 1);

	EventBus->ResetForTests();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAIIntegrationPlayerDetectionTest,
	"HorrorProject.Integration.AI.PlayerDetection",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FAIIntegrationPlayerDetectionTest::RunTest(const FString& Parameters)
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

	AActor* AIActor = World->SpawnActor<AActor>();
	UHorrorGolemBehaviorComponent* GolemBehavior = NewObject<UHorrorGolemBehaviorComponent>(AIActor);
	GolemBehavior->RegisterComponent();

	AActor* PlayerActor = World->SpawnActor<AActor>();

	// Publish player detected event
	FGameplayTag DetectedTag = FGameplayTag::RequestGameplayTag(FName("Horror.Player.Detected"));
	EventBus->Publish(DetectedTag, FName("AIDetection"), FGameplayTag(), PlayerActor);

	TestEqual(TEXT("Detection event should be published"), EventBus->GetHistory().Num(), 1);

	AIActor->Destroy();
	PlayerActor->Destroy();
	EventBus->ResetForTests();

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAIIntegrationMultipleAITest,
	"HorrorProject.Integration.AI.MultipleAI",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FAIIntegrationMultipleAITest::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	if (!World)
	{
		AddError(TEXT("Failed to create test world"));
		return false;
	}

	// Create multiple AI actors
	AActor* AI1 = World->SpawnActor<AActor>();
	UHorrorGolemBehaviorComponent* Golem1 = NewObject<UHorrorGolemBehaviorComponent>(AI1);
	Golem1->RegisterComponent();

	AActor* AI2 = World->SpawnActor<AActor>();
	UHorrorGolemBehaviorComponent* Golem2 = NewObject<UHorrorGolemBehaviorComponent>(AI2);
	Golem2->RegisterComponent();

	AActor* AI3 = World->SpawnActor<AActor>();
	UHorrorGolemBehaviorComponent* Golem3 = NewObject<UHorrorGolemBehaviorComponent>(AI3);
	Golem3->RegisterComponent();

	TestTrue(TEXT("All AI components should be registered"),
		Golem1->IsRegistered() && Golem2->IsRegistered() && Golem3->IsRegistered());

	AI1->Destroy();
	AI2->Destroy();
	AI3->Destroy();

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR
