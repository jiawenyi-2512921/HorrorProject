// Copyright Epic Games, Inc. All Rights Reserved.

#if WITH_DEV_AUTOMATION_TESTS

#include "Player/Components/InteractionComponent.h"

#include "Camera/CameraComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Components/BoxComponent.h"
#include "Components/TimelineComponent.h"
#include "Engine/World.h"
#include "Game/FoundFootageObjectiveInteractable.h"
#include "Game/HorrorGameModeBase.h"
#include "Game/HorrorFoundFootageContract.h"
#include "GameFramework/WorldSettings.h"
#include "Interaction/InteractableInterface.h"
#include "Misc/AutomationTest.h"
#include "Player/HorrorPlayerCharacter.h"
#include "Tests/AutomationCommon.h"

namespace
{
	bool CreateInteractionTestWorld(FTestWorldWrapper& TestWorld, UWorld*& OutWorld, AHorrorGameModeBase*& OutGameMode)
	{
		OutWorld = nullptr;
		OutGameMode = nullptr;
		if (!TestWorld.CreateTestWorld(EWorldType::Game))
		{
			return false;
		}

		OutWorld = TestWorld.GetTestWorld();
		if (!OutWorld)
		{
			return false;
		}

		OutWorld->GetWorldSettings()->DefaultGameMode = AHorrorGameModeBase::StaticClass();
		if (!OutWorld->SetGameMode(FURL()))
		{
			return false;
		}

		OutGameMode = OutWorld->GetAuthGameMode<AHorrorGameModeBase>();
		if (OutGameMode)
		{
			OutGameMode->SetActorEnableCollision(false);
		}

		return OutGameMode != nullptr;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FInteractionComponentObjectiveHitTest,
	"HorrorProject.Player.Interaction.ObjectiveHit",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FInteractionComponentObjectiveHitTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	UWorld* World = nullptr;
	AHorrorGameModeBase* GameMode = nullptr;
	TestTrue(TEXT("Transient game world should be created for interaction coverage."), CreateInteractionTestWorld(TestWorld, World, GameMode));
	if (!World || !GameMode)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	AHorrorPlayerCharacter* PlayerCharacter = World->SpawnActor<AHorrorPlayerCharacter>(FVector::ZeroVector, FRotator::ZeroRotator);
	AFoundFootageObjectiveInteractable* Bodycam = World->SpawnActor<AFoundFootageObjectiveInteractable>(FVector(64.0f, 0.0f, 64.0f), FRotator::ZeroRotator);
	TestNotNull(TEXT("Interaction test player should spawn."), PlayerCharacter);
	TestNotNull(TEXT("Interaction test bodycam objective should spawn."), Bodycam);
	if (!PlayerCharacter || !Bodycam)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	Bodycam->Objective = EFoundFootageInteractableObjective::Bodycam;
	Bodycam->SourceId = TEXT("Evidence.Bodycam");

	FHitResult ObjectiveHit;
	ObjectiveHit.HitObjectHandle = FActorInstanceHandle(Bodycam);
	ObjectiveHit.Component = Bodycam->FindComponentByClass<UBoxComponent>();

	UInteractionComponent* Interaction = PlayerCharacter->GetInteractionComponent();
	TestNotNull(TEXT("Player should expose an interaction component."), Interaction);
	if (!Interaction)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	TestTrue(TEXT("Objective hit should resolve to an actor."), ObjectiveHit.GetActor() == Bodycam);
	TestTrue(TEXT("Objective hit should resolve to a component."), ObjectiveHit.GetComponent() != nullptr);
	TestTrue(TEXT("Bodycam should allow direct completion before interaction."), Bodycam->CanCompleteObjective(GameMode));
	TestTrue(TEXT("Bodycam direct completion should work before interaction routing."), Bodycam->TryCompleteObjective(GameMode));
	GameMode->ImportFoundFootageSaveState(FHorrorFoundFootageSaveState());
	TestTrue(TEXT("Bodycam should allow completion after state reset."), Bodycam->CanCompleteObjective(GameMode));
	TestNotNull(TEXT("Bodycam world should expose the interaction test game mode."), Bodycam->GetWorld() ? Bodycam->GetWorld()->GetAuthGameMode<AHorrorGameModeBase>() : nullptr);
	TestTrue(TEXT("Bodycam class should implement the interactable interface."), Bodycam->GetClass()->ImplementsInterface(UInteractableInterface::StaticClass()));
	TestTrue(TEXT("Bodycam native CanInteract_Implementation should accept the objective hit."), Bodycam->CanInteract_Implementation(PlayerCharacter, ObjectiveHit));
	TestTrue(TEXT("Bodycam native Interact_Implementation should complete the objective hit."), Bodycam->Interact_Implementation(PlayerCharacter, ObjectiveHit));
	TestTrue(TEXT("Objective interaction should update the game mode."), GameMode->HasBodycamAcquired());
	GameMode->ImportFoundFootageSaveState(FHorrorFoundFootageSaveState());
	TestTrue(TEXT("Interaction should complete a visible objective interactable."), Interaction->TryInteractWithHit(ObjectiveHit));
	TestTrue(TEXT("Objective interaction should update the game mode."), GameMode->HasBodycamAcquired());
	TestFalse(TEXT("Completed objectives should stop being interactable through the interaction component."), Interaction->TryInteractWithHit(ObjectiveHit));

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FInteractionComponentRejectsMissingCandidateTest,
	"HorrorProject.Player.Interaction.RejectsMissingCandidate",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FInteractionComponentRejectsMissingCandidateTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	UWorld* World = nullptr;
	AHorrorGameModeBase* GameMode = nullptr;
	TestTrue(TEXT("Transient game world should be created for interaction rejection coverage."), CreateInteractionTestWorld(TestWorld, World, GameMode));
	if (!World || !GameMode)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	AHorrorPlayerCharacter* PlayerCharacter = World->SpawnActor<AHorrorPlayerCharacter>(FVector::ZeroVector, FRotator::ZeroRotator);
	AFoundFootageObjectiveInteractable* Recorder = World->SpawnActor<AFoundFootageObjectiveInteractable>(FVector(64.0f, 0.0f, 64.0f), FRotator::ZeroRotator);
	TestNotNull(TEXT("Interaction rejection player should spawn."), PlayerCharacter);
	TestNotNull(TEXT("Interaction rejection recorder should spawn."), Recorder);
	if (!PlayerCharacter || !Recorder)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	Recorder->Objective = EFoundFootageInteractableObjective::FirstAnomalyRecord;
	Recorder->SourceId = TEXT("Evidence.Recorder");
	Recorder->bIsRecordingForFirstAnomalyRecord = true;

	FHitResult ObjectiveHit;
	ObjectiveHit.HitObjectHandle = FActorInstanceHandle(Recorder);
	ObjectiveHit.Component = Recorder->FindComponentByClass<UBoxComponent>();

	UInteractionComponent* Interaction = PlayerCharacter->GetInteractionComponent();
	TestNotNull(TEXT("Player should expose an interaction component."), Interaction);
	if (!Interaction)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	TestFalse(TEXT("Interaction should reject anomaly recording before a candidate exists."), Interaction->TryInteractWithHit(ObjectiveHit));
	TestFalse(TEXT("Rejected interaction should not record the first anomaly."), GameMode->HasRecordedFirstAnomaly());

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FInteractionComponentDispatchesInterfaceThroughExecuteTest,
	"HorrorProject.Player.Interaction.DispatchesInterfaceThroughExecute",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FInteractionComponentDispatchesInterfaceThroughExecuteTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	UWorld* World = nullptr;
	AHorrorGameModeBase* GameMode = nullptr;
	TestTrue(TEXT("Transient game world should be created for interface dispatch coverage."), CreateInteractionTestWorld(TestWorld, World, GameMode));
	if (!World || !GameMode)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	AHorrorPlayerCharacter* PlayerCharacter = World->SpawnActor<AHorrorPlayerCharacter>();
	AFoundFootageObjectiveInteractable* Bodycam = World->SpawnActor<AFoundFootageObjectiveInteractable>();
	TestNotNull(TEXT("Interface dispatch test player should spawn."), PlayerCharacter);
	TestNotNull(TEXT("Interface dispatch target should spawn."), Bodycam);
	if (!PlayerCharacter || !Bodycam)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	Bodycam->Objective = EFoundFootageInteractableObjective::Bodycam;
	Bodycam->SourceId = TEXT("Evidence.Bodycam");

	UInteractionComponent* Interaction = PlayerCharacter->GetInteractionComponent();
	TestNotNull(TEXT("Interface dispatch test player should expose interaction component."), Interaction);
	if (!Interaction)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	FHitResult Hit;
	Hit.HitObjectHandle = FActorInstanceHandle(Bodycam);
	Hit.Component = Bodycam->FindComponentByClass<UBoxComponent>();
	TestTrue(TEXT("Native interface target should dispatch through Execute helpers."), Bodycam->CanInteract_Implementation(PlayerCharacter, Hit));
	TestTrue(TEXT("Interaction routing should complete native interface targets through Execute-compatible routing."), Interaction->TryInteractWithHit(Hit));
	TestTrue(TEXT("Execute-routed objective interaction should update game mode."), GameMode->HasBodycamAcquired());

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FInteractionComponentComponentInterfaceTargetTest,
	"HorrorProject.Player.Interaction.ComponentInterfaceTarget",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FInteractionComponentComponentInterfaceTargetTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	UWorld* World = nullptr;
	AHorrorGameModeBase* GameMode = nullptr;
	TestTrue(TEXT("Transient game world should be created for component interface target coverage."), CreateInteractionTestWorld(TestWorld, World, GameMode));
	if (!World || !GameMode)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	UInteractionComponent* InteractionAccess = NewObject<UInteractionComponent>();
	AFoundFootageObjectiveInteractable* ActorTarget = World->SpawnActor<AFoundFootageObjectiveInteractable>();
	UBoxComponent* ComponentTarget = NewObject<UBoxComponent>(ActorTarget);
	ComponentTarget->RegisterComponent();
	ActorTarget->AddInstanceComponent(ComponentTarget);
	TestNotNull(TEXT("Component target actor should be created."), ActorTarget);
	TestNotNull(TEXT("Component target should be created."), ComponentTarget);
	if (!InteractionAccess || !ActorTarget || !ComponentTarget)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	FHitResult ActorOnlyHit;
	ActorOnlyHit.HitObjectHandle = FActorInstanceHandle(ActorTarget);
	TestTrue(TEXT("Actor-only hit should resolve actor interface target."), InteractionAccess->ResolveInterfaceTargetForTests(ActorOnlyHit) == ActorTarget);

	FHitResult ComponentHit;
	ComponentHit.HitObjectHandle = FActorInstanceHandle(ActorTarget);
	ComponentHit.Component = ComponentTarget;
	TestFalse(TEXT("Plain component should not implement the interactable interface."), ComponentTarget->GetClass()->ImplementsInterface(UInteractableInterface::StaticClass()));
	TestTrue(TEXT("Non-interactable hit component should fall back to actor interface target."), InteractionAccess->ResolveInterfaceTargetForTests(ComponentHit) == ActorTarget);

	FHitResult ObjectiveComponentHit;
	ObjectiveComponentHit.HitObjectHandle = FActorInstanceHandle(ActorTarget);
	ObjectiveComponentHit.Component = ActorTarget->FindComponentByClass<UBoxComponent>();
	TestNotNull(TEXT("Objective component hit should expose the actor box component."), ObjectiveComponentHit.GetComponent());
	TestTrue(TEXT("Objective actor should implement the interactable interface."), ActorTarget->GetClass()->ImplementsInterface(UInteractableInterface::StaticClass()));
	TestTrue(TEXT("Actor-owned component without its own interface should still resolve actor target."), InteractionAccess->ResolveInterfaceTargetForTests(ObjectiveComponentHit) == ActorTarget);

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FInteractionComponentLegacyDoorTimelineFallbackIsOptInTest,
	"HorrorProject.Player.Interaction.LegacyDoorTimelineFallbackIsOptIn",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FInteractionComponentLegacyDoorTimelineFallbackIsOptInTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	UWorld* World = nullptr;
	AHorrorGameModeBase* GameMode = nullptr;
	TestTrue(TEXT("Transient game world should be created for legacy fallback coverage."), CreateInteractionTestWorld(TestWorld, World, GameMode));
	if (!World || !GameMode)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	AHorrorPlayerCharacter* PlayerCharacter = World->SpawnActor<AHorrorPlayerCharacter>();
	AActor* DoorActor = World->SpawnActor<AActor>();
	AActor* NonDoorActor = World->SpawnActor<AActor>();
	TestNotNull(TEXT("Legacy fallback player should spawn."), PlayerCharacter);
	TestNotNull(TEXT("Legacy fallback door actor should spawn."), DoorActor);
	TestNotNull(TEXT("Legacy fallback non-door actor should spawn."), NonDoorActor);
	if (!PlayerCharacter || !DoorActor || !NonDoorActor)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	UTimelineComponent* DoorTimeline = NewObject<UTimelineComponent>(DoorActor, TEXT("Door Control Timeline"));
	DoorTimeline->RegisterComponent();
	DoorActor->AddInstanceComponent(DoorTimeline);
	UTimelineComponent* NonDoorTimeline = NewObject<UTimelineComponent>(NonDoorActor, TEXT("Ambient Flicker Timeline"));
	NonDoorTimeline->RegisterComponent();
	NonDoorActor->AddInstanceComponent(NonDoorTimeline);

	UInteractionComponent* Interaction = PlayerCharacter->GetInteractionComponent();
	TestNotNull(TEXT("Legacy fallback player should expose interaction component."), Interaction);
	if (!Interaction)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	FHitResult DoorHit;
	DoorHit.HitObjectHandle = FActorInstanceHandle(DoorActor);
	TestFalse(TEXT("Legacy door timeline fallback should be disabled by default."), Interaction->TryInteractWithHit(DoorHit));
	Interaction->SetLegacyDoorTimelineFallbackEnabledForTests(true);
	TestTrue(TEXT("Legacy door timeline fallback should be opt-in."), Interaction->TryInteractWithHit(DoorHit));

	FHitResult NonDoorHit;
	NonDoorHit.HitObjectHandle = FActorInstanceHandle(NonDoorActor);
	TestFalse(TEXT("Unrelated timelines should not satisfy the legacy door fallback."), Interaction->TryInteractWithHit(NonDoorHit));

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FInteractionComponentSweepScoringPrefersCenterlineTest,
	"HorrorProject.Player.Interaction.SweepScoringPrefersCenterline",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FInteractionComponentSweepScoringPrefersCenterlineTest::RunTest(const FString& Parameters)
{
	UInteractionComponent* InteractionAccess = NewObject<UInteractionComponent>();
	const FVector Start(0.0f, 0.0f, 0.0f);
	const FVector End(200.0f, 0.0f, 0.0f);
	const float NearOffAxisScore = InteractionAccess->CalculatePerpendicularDistanceToTraceForTests(Start, End, FVector(80.0f, 10.0f, 0.0f));
	const float FarCenterlineScore = InteractionAccess->CalculatePerpendicularDistanceToTraceForTests(Start, End, FVector(120.0f, 0.0f, 0.0f));

	TestTrue(TEXT("Sweep scoring should prefer a centerline target over a closer off-axis target."), FarCenterlineScore < NearOffAxisScore);
	return true;
}

#endif
