#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR && HORRORPROJECT_ENABLE_LEGACY_AUTOMATION_TESTS

// Copyright Epic Games, Inc. All Rights Reserved.

#include "AI/HorrorGolemBehaviorComponent.h"
#include "Misc/AutomationTest.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorGolemBehaviorInitializationTest,
	"HorrorProject.AI.Golem.Initialization",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorGolemBehaviorInitializationTest::RunTest(const FString& Parameters)
{
	UHorrorGolemBehaviorComponent* GolemBehavior = NewObject<UHorrorGolemBehaviorComponent>();

	TestFalse(TEXT("Behavior should not be active initially"), GolemBehavior->IsBehaviorActive());
	TestEqual(TEXT("Initial state should be Dormant"), GolemBehavior->GetCurrentState(), EGolemEncounterState::Dormant);
	TestNull(TEXT("Target actor should be null initially"), GolemBehavior->GetTargetActor());

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorGolemBehaviorActivationTest,
	"HorrorProject.AI.Golem.Activation",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorGolemBehaviorActivationTest::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	AActor* TargetActor = World->SpawnActor<AActor>();
	AActor* GolemActor = World->SpawnActor<AActor>();

	UHorrorGolemBehaviorComponent* GolemBehavior = NewObject<UHorrorGolemBehaviorComponent>(GolemActor);
	GolemBehavior->RegisterComponent();

	GolemBehavior->ActivateBehavior(TargetActor);
	TestTrue(TEXT("Behavior should be active after activation"), GolemBehavior->IsBehaviorActive());
	TestEqual(TEXT("Target actor should be set"), GolemBehavior->GetTargetActor(), TargetActor);

	GolemBehavior->DeactivateBehavior();
	TestFalse(TEXT("Behavior should be inactive after deactivation"), GolemBehavior->IsBehaviorActive());

	World->DestroyWorld(false);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorGolemBehaviorStateTransitionTest,
	"HorrorProject.AI.Golem.StateTransition",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorGolemBehaviorStateTransitionTest::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	AActor* TargetActor = World->SpawnActor<AActor>();
	AActor* GolemActor = World->SpawnActor<AActor>();

	UHorrorGolemBehaviorComponent* GolemBehavior = NewObject<UHorrorGolemBehaviorComponent>(GolemActor);
	GolemBehavior->RegisterComponent();

	GolemBehavior->ActivateBehavior(TargetActor);

	GolemBehavior->ForceStateTransition(EGolemEncounterState::DistantSighting);
	TestEqual(TEXT("State should transition to DistantSighting"), GolemBehavior->GetCurrentState(), EGolemEncounterState::DistantSighting);

	GolemBehavior->ForceStateTransition(EGolemEncounterState::CloseStalking);
	TestEqual(TEXT("State should transition to CloseStalking"), GolemBehavior->GetCurrentState(), EGolemEncounterState::CloseStalking);

	GolemBehavior->ForceStateTransition(EGolemEncounterState::ChaseTriggered);
	TestEqual(TEXT("State should transition to ChaseTriggered"), GolemBehavior->GetCurrentState(), EGolemEncounterState::ChaseTriggered);

	GolemBehavior->ForceStateTransition(EGolemEncounterState::FullChase);
	TestEqual(TEXT("State should transition to FullChase"), GolemBehavior->GetCurrentState(), EGolemEncounterState::FullChase);

	GolemBehavior->ForceStateTransition(EGolemEncounterState::FinalImpact);
	TestEqual(TEXT("State should transition to FinalImpact"), GolemBehavior->GetCurrentState(), EGolemEncounterState::FinalImpact);

	World->DestroyWorld(false);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorGolemBehaviorDistanceCalculationTest,
	"HorrorProject.AI.Golem.DistanceCalculation",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorGolemBehaviorDistanceCalculationTest::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	AActor* TargetActor = World->SpawnActor<AActor>(FVector(1000.0f, 0.0f, 0.0f), FRotator::ZeroRotator);
	AActor* GolemActor = World->SpawnActor<AActor>(FVector::ZeroVector, FRotator::ZeroRotator);

	UHorrorGolemBehaviorComponent* GolemBehavior = NewObject<UHorrorGolemBehaviorComponent>(GolemActor);
	GolemBehavior->RegisterComponent();

	GolemBehavior->ActivateBehavior(TargetActor);

	float Distance = GolemBehavior->GetDistanceToTarget();
	TestTrue(TEXT("Distance should be approximately 1000"), FMath::IsNearlyEqual(Distance, 1000.0f, 10.0f));

	World->DestroyWorld(false);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorGolemBehaviorPhaseConfigTest,
	"HorrorProject.AI.Golem.PhaseConfig",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorGolemBehaviorPhaseConfigTest::RunTest(const FString& Parameters)
{
	UHorrorGolemBehaviorComponent* GolemBehavior = NewObject<UHorrorGolemBehaviorComponent>();

	TestEqual(TEXT("Distant sighting min distance should be 3000"), GolemBehavior->DistantSightingMinDistance, 3000.0f);
	TestEqual(TEXT("Close stalking min distance should be 1000"), GolemBehavior->CloseStalking_MinDistance, 1000.0f);
	TestEqual(TEXT("Chase triggered start distance should be 2000"), GolemBehavior->ChaseTriggered_StartDistance, 2000.0f);
	TestEqual(TEXT("Full chase min distance should be 1000"), GolemBehavior->FullChase_MinDistance, 1000.0f);
	TestEqual(TEXT("Final impact trigger distance should be 500"), GolemBehavior->FinalImpact_TriggerDistance, 500.0f);

	TestEqual(TEXT("Chase triggered speed multiplier should be 0.7"), GolemBehavior->ChaseTriggered_SpeedMultiplier, 0.7f);
	TestTrue(TEXT("Castle golem triggered chase should stay slow enough for the player to outrun"), GolemBehavior->ChaseTriggered_BaseSpeed <= 220.0f);
	TestTrue(TEXT("Castle golem full chase should stay below sprint speed"), GolemBehavior->FullChase_Speed <= 260.0f);
	TestTrue(TEXT("Full chase destruction should be enabled"), GolemBehavior->bFullChase_EnableDestruction);
	TestTrue(TEXT("Final impact cutscene should be enabled"), GolemBehavior->bFinalImpact_TriggerCutscene);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorGolemBehaviorDeactivationTest,
	"HorrorProject.AI.Golem.Deactivation",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorGolemBehaviorDeactivationTest::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	AActor* TargetActor = World->SpawnActor<AActor>();
	AActor* GolemActor = World->SpawnActor<AActor>();

	UHorrorGolemBehaviorComponent* GolemBehavior = NewObject<UHorrorGolemBehaviorComponent>(GolemActor);
	GolemBehavior->RegisterComponent();

	GolemBehavior->ActivateBehavior(TargetActor);
	GolemBehavior->ForceStateTransition(EGolemEncounterState::FullChase);

	GolemBehavior->DeactivateBehavior();
	TestFalse(TEXT("Behavior should be inactive"), GolemBehavior->IsBehaviorActive());
	TestEqual(TEXT("State should return to Dormant"), GolemBehavior->GetCurrentState(), EGolemEncounterState::Dormant);

	World->DestroyWorld(false);
	return true;
}

#endif

#endif // WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR
