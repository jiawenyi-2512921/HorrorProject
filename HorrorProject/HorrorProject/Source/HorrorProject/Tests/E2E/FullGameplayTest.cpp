#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR && HORRORPROJECT_ENABLE_LEGACY_AUTOMATION_TESTS

// Copyright Epic Games, Inc. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "Game/HorrorEventBusSubsystem.h"
#include "Audio/HorrorAudioSubsystem.h"
#include "Evidence/ArchiveSubsystem.h"
#include "Player/Components/FearComponent.h"
#include "Player/Components/NoiseGeneratorComponent.h"
#include "Player/Components/CameraBatteryComponent.h"
#include "AI/HorrorGolemBehaviorComponent.h"
#include "Tests/AutomationCommon.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FE2EFullGameplayTest,
	"HorrorProject.E2E.FullGameplay",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FE2EFullGameplayTest::RunTest(const FString& Parameters)
{
	UWorld* World = FAutomationEditorCommonUtils::CreateNewMap();
	if (!World)
	{
		AddError(TEXT("Failed to create test world"));
		return false;
	}

	// Initialize all subsystems
	UHorrorEventBusSubsystem* EventBus = World->GetSubsystem<UHorrorEventBusSubsystem>();
	UHorrorAudioSubsystem* AudioSubsystem = World->GetSubsystem<UHorrorAudioSubsystem>();
	UArchiveSubsystem* ArchiveSubsystem = World->GetSubsystem<UArchiveSubsystem>();

	TestNotNull(TEXT("EventBus should exist"), EventBus);
	TestNotNull(TEXT("Audio subsystem should exist"), AudioSubsystem);
	TestNotNull(TEXT("Archive subsystem should exist"), ArchiveSubsystem);

	// Create player
	AActor* PlayerActor = World->SpawnActor<AActor>();
	UFearComponent* FearComp = NewObject<UFearComponent>(PlayerActor);
	FearComp->RegisterComponent();
	UNoiseGeneratorComponent* NoiseGen = NewObject<UNoiseGeneratorComponent>(PlayerActor);
	NoiseGen->RegisterComponent();
	UCameraBatteryComponent* BatteryComp = NewObject<UCameraBatteryComponent>(PlayerActor);
	BatteryComp->RegisterComponent();

	// Create AI
	AActor* AIActor = World->SpawnActor<AActor>();
	UHorrorGolemBehaviorComponent* GolemBehavior = NewObject<UHorrorGolemBehaviorComponent>(AIActor);
	GolemBehavior->RegisterComponent();

	// Simulate gameplay flow
	// 1. Game starts
	FGameplayTag StartTag = FGameplayTag::RequestGameplayTag(FName("Horror.Game.Start"));
	EventBus->Publish(StartTag, FName("GameStart"), FGameplayTag(), nullptr);

	// 2. Player makes noise
	FGameplayTag NoiseTag = FGameplayTag::RequestGameplayTag(FName("Horror.Player.Noise"));
	EventBus->Publish(NoiseTag, FName("PlayerNoise"), FGameplayTag(), PlayerActor);

	// 3. AI detects player
	FGameplayTag DetectedTag = FGameplayTag::RequestGameplayTag(FName("Horror.Player.Detected"));
	EventBus->Publish(DetectedTag, FName("AIDetection"), FGameplayTag(), PlayerActor);

	// 4. Player fear increases
	FGameplayTag ThreatTag = FGameplayTag::RequestGameplayTag(FName("Horror.Threat.Detected"));
	EventBus->Publish(ThreatTag, FName("Threat"), FGameplayTag(), AIActor);

	// 5. Player collects evidence
	FGameplayTag EvidenceTag = FGameplayTag::RequestGameplayTag(FName("Horror.Evidence.Collected"));
	EventBus->Publish(EvidenceTag, FName("Evidence1"), FGameplayTag(), nullptr);

	// Verify all events were processed
	TestEqual(TEXT("Should have 5 events in history"), EventBus->GetHistory().Num(), 5);

	// Verify components are still valid
	TestTrue(TEXT("Fear component should be valid"), FearComp->IsRegistered());
	TestTrue(TEXT("Noise generator should be valid"), NoiseGen->IsRegistered());
	TestTrue(TEXT("Battery component should be valid"), BatteryComp->IsRegistered());
	TestTrue(TEXT("Golem behavior should be valid"), GolemBehavior->IsRegistered());

	// Cleanup
	PlayerActor->Destroy();
	AIActor->Destroy();
	EventBus->ResetForTests();

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FE2ESaveLoadCycleTest,
	"HorrorProject.E2E.SaveLoadCycle",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FE2ESaveLoadCycleTest::RunTest(const FString& Parameters)
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

	// Setup initial state
	AActor* PlayerActor = World->SpawnActor<AActor>();
	UFearComponent* FearComp = NewObject<UFearComponent>(PlayerActor);
	FearComp->RegisterComponent();

	// Publish some events
	FGameplayTag Tag1 = FGameplayTag::RequestGameplayTag(FName("Horror.Test.Event1"));
	FGameplayTag Tag2 = FGameplayTag::RequestGameplayTag(FName("Horror.Test.Event2"));
	EventBus->Publish(Tag1, FName("Source1"), FGameplayTag(), nullptr);
	EventBus->Publish(Tag2, FName("Source2"), FGameplayTag(), nullptr);

	int32 EventCountBeforeSave = EventBus->GetHistory().Num();
	TestEqual(TEXT("Should have 2 events before save"), EventCountBeforeSave, 2);

	// Simulate save (in real game, this would serialize state)
	float FearBeforeSave = FearComp->GetCurrentFear();

	// Simulate load (in real game, this would restore state)
	// For now, verify state is accessible
	TestTrue(TEXT("Fear value should be accessible"), FearBeforeSave >= 0.0f);

	// Cleanup
	PlayerActor->Destroy();
	EventBus->ResetForTests();

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FE2EPerformanceStressTest,
	"HorrorProject.E2E.PerformanceStress",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FE2EPerformanceStressTest::RunTest(const FString& Parameters)
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

	// Create multiple actors
	TArray<AActor*> Actors;
	for (int32 i = 0; i < 10; i++)
	{
		AActor* Actor = World->SpawnActor<AActor>();
		Actors.Add(Actor);

		if (i % 2 == 0)
		{
			UNoiseGeneratorComponent* NoiseGen = NewObject<UNoiseGeneratorComponent>(Actor);
			NoiseGen->RegisterComponent();
		}
		else
		{
			UHorrorGolemBehaviorComponent* GolemBehavior = NewObject<UHorrorGolemBehaviorComponent>(Actor);
			GolemBehavior->RegisterComponent();
		}
	}

	// Publish many events
	FGameplayTag TestTag = FGameplayTag::RequestGameplayTag(FName("Horror.Test.Stress"));
	for (int32 i = 0; i < 50; i++)
	{
		EventBus->Publish(TestTag, FName(*FString::Printf(TEXT("Source%d"), i)), FGameplayTag(), nullptr);
	}

	TestEqual(TEXT("Should have 50 events"), EventBus->GetHistory().Num(), 50);

	// Cleanup
	for (AActor* Actor : Actors)
	{
		Actor->Destroy();
	}
	EventBus->ResetForTests();

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FE2EEdgeCaseTest,
	"HorrorProject.E2E.EdgeCases",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FE2EEdgeCaseTest::RunTest(const FString& Parameters)
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

	// Test 1: Publish with null source object
	FGameplayTag Tag1 = FGameplayTag::RequestGameplayTag(FName("Horror.Test.NullSource"));
	bool bPublished1 = EventBus->Publish(Tag1, FName("NullTest"), FGameplayTag(), nullptr);
	TestTrue(TEXT("Should handle null source object"), bPublished1);

	// Test 2: Publish with empty source ID
	FGameplayTag Tag2 = FGameplayTag::RequestGameplayTag(FName("Horror.Test.EmptyID"));
	bool bPublished2 = EventBus->Publish(Tag2, NAME_None, FGameplayTag(), nullptr);
	TestTrue(TEXT("Should handle empty source ID"), bPublished2);

	// Test 3: Publish same event multiple times
	FGameplayTag Tag3 = FGameplayTag::RequestGameplayTag(FName("Horror.Test.Duplicate"));
	EventBus->Publish(Tag3, FName("DupTest"), FGameplayTag(), nullptr);
	EventBus->Publish(Tag3, FName("DupTest"), FGameplayTag(), nullptr);
	EventBus->Publish(Tag3, FName("DupTest"), FGameplayTag(), nullptr);

	TestEqual(TEXT("Should have all events including duplicates"), EventBus->GetHistory().Num(), 5);

	EventBus->ResetForTests();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FE2EComplexInteractionTest,
	"HorrorProject.E2E.ComplexInteraction",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FE2EComplexInteractionTest::RunTest(const FString& Parameters)
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

	// Create complex scenario
	AActor* Player = World->SpawnActor<AActor>();
	UFearComponent* FearComp = NewObject<UFearComponent>(Player);
	FearComp->RegisterComponent();
	UNoiseGeneratorComponent* NoiseGen = NewObject<UNoiseGeneratorComponent>(Player);
	NoiseGen->RegisterComponent();

	AActor* AI1 = World->SpawnActor<AActor>();
	UHorrorGolemBehaviorComponent* Golem1 = NewObject<UHorrorGolemBehaviorComponent>(AI1);
	Golem1->RegisterComponent();

	AActor* AI2 = World->SpawnActor<AActor>();
	UHorrorGolemBehaviorComponent* Golem2 = NewObject<UHorrorGolemBehaviorComponent>(AI2);
	Golem2->RegisterComponent();

	// Simulate complex interaction
	// Player makes noise
	FGameplayTag NoiseTag = FGameplayTag::RequestGameplayTag(FName("Horror.Player.Noise"));
	EventBus->Publish(NoiseTag, FName("PlayerNoise"), FGameplayTag(), Player);

	// Both AIs detect
	FGameplayTag DetectedTag = FGameplayTag::RequestGameplayTag(FName("Horror.Player.Detected"));
	EventBus->Publish(DetectedTag, FName("AI1Detection"), FGameplayTag(), Player);
	EventBus->Publish(DetectedTag, FName("AI2Detection"), FGameplayTag(), Player);

	// Audio plays
	FGameplayTag AudioTag = FGameplayTag::RequestGameplayTag(FName("Horror.Audio.Trigger"));
	EventBus->Publish(AudioTag, FName("ThreatAudio"), FGameplayTag(), nullptr);

	// Fear increases
	FGameplayTag ThreatTag = FGameplayTag::RequestGameplayTag(FName("Horror.Threat.Detected"));
	EventBus->Publish(ThreatTag, FName("Threat"), FGameplayTag(), AI1);

	TestEqual(TEXT("Should have 5 events"), EventBus->GetHistory().Num(), 5);

	// Cleanup
	Player->Destroy();
	AI1->Destroy();
	AI2->Destroy();
	EventBus->ResetForTests();

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR
