// Copyright Epic Games, Inc. All Rights Reserved.

#include "AI/HorrorThreatCharacter.h"

#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR

#include "AI/HorrorGolemBehaviorComponent.h"
#include "AI/HorrorThreatAIController.h"
#include "Animation/AnimationAsset.h"
#include "Audio/HorrorAudioSubsystem.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "Game/HorrorEventBusSubsystem.h"
#include "Game/HorrorGameModeBase.h"
#include "GameplayTagContainer.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/WorldSettings.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/AutomationTest.h"
#include "Player/HorrorPlayerCharacter.h"
#include "Tests/AutomationCommon.h"

namespace
{
	const FString Day1ThreatAutosaveSlotName(TEXT("SM13_Day1_Autosave"));
	constexpr int32 Day1ThreatAutosaveUserIndex = 0;

	AHorrorGameModeBase* CreateThreatRuntimeGameMode(FAutomationTestBase& Test, FTestWorldWrapper& TestWorld, UWorld*& OutWorld)
	{
		OutWorld = nullptr;
		Test.TestTrue(TEXT("Transient game world should be created for Day 1 threat runtime coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
		OutWorld = TestWorld.GetTestWorld();
		if (!OutWorld)
		{
			return nullptr;
		}

		OutWorld->GetWorldSettings()->DefaultGameMode = AHorrorGameModeBase::StaticClass();
		Test.TestTrue(TEXT("Transient world should create the Day 1 game mode."), OutWorld->SetGameMode(FURL()));

		AHorrorGameModeBase* GameMode = OutWorld->GetAuthGameMode<AHorrorGameModeBase>();
		Test.TestNotNull(TEXT("Transient world should expose the Day 1 game mode."), GameMode);
		if (!GameMode)
		{
			TestWorld.DestroyTestWorld(false);
		}

		return GameMode;
	}

	AHorrorPlayerCharacter* SpawnThreatRuntimePlayer(FAutomationTestBase& Test, UWorld* World)
	{
		if (!World)
		{
			return nullptr;
		}

		APlayerController* PlayerController = World->SpawnActor<APlayerController>();
		AHorrorPlayerCharacter* PlayerCharacter = World->SpawnActor<AHorrorPlayerCharacter>();
		Test.TestNotNull(TEXT("Threat runtime test should spawn a player controller."), PlayerController);
		Test.TestNotNull(TEXT("Threat runtime test should spawn a horror player character."), PlayerCharacter);

		if (PlayerController)
		{
			PlayerController->PlayerState = World->SpawnActor<APlayerState>();
			World->AddController(PlayerController);
		}

		if (PlayerController && PlayerCharacter)
		{
			PlayerController->Possess(PlayerCharacter);
		}

		return PlayerCharacter;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorThreatDefaultsWireStoneGolemRuntimeTest,
	"HorrorProject.AI.Threat.Day1.DefaultsWireStoneGolem",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorThreatDefaultsWireStoneGolemRuntimeTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for Day 1 threat defaults coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AHorrorThreatCharacter* Threat = World->SpawnActor<AHorrorThreatCharacter>(FVector::ZeroVector, FRotator::ZeroRotator, SpawnParameters);
	TestNotNull(TEXT("Threat character should spawn."), Threat);
	if (!Threat)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	TestEqual(TEXT("Threat should use the native horror AI controller by default."), Threat->AIControllerClass.Get(), AHorrorThreatAIController::StaticClass());
	TestEqual(TEXT("Threat should auto possess AI when spawned or placed."), Threat->AutoPossessAI, EAutoPossessAI::PlacedInWorldOrSpawned);
	TestNotNull(TEXT("Threat should always expose its golem behavior component."), Threat->GetGolemBehavior());
	TestNotNull(TEXT("Threat should keep its inherited skeletal mesh component."), Threat->GetMesh());
	TestTrue(TEXT("Threat should load a Stone Golem skeletal asset by default."), Threat->GetMesh() && Threat->GetMesh()->GetSkinnedAsset() != nullptr);
	TestTrue(TEXT("Threat capsule should be large enough for a visible creature."), Threat->GetCapsuleComponent() && Threat->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() >= 100.0f);

	UAnimationAsset* IdleAnimation = LoadObject<UAnimationAsset>(nullptr, TEXT("/Game/Stone_Golem/demo/animations/ThirdPersonIdle.ThirdPersonIdle"));
	UAnimationAsset* RunAnimation = LoadObject<UAnimationAsset>(nullptr, TEXT("/Game/Stone_Golem/demo/animations/ThirdPersonRun.ThirdPersonRun"));
	TestNotNull(TEXT("Stone Golem idle animation asset should be loadable."), IdleAnimation);
	TestNotNull(TEXT("Stone Golem run animation asset should be loadable."), RunAnimation);
	TestEqual(TEXT("Threat should use single-node animation playback by default."), Threat->GetMesh()->GetAnimationMode(), EAnimationMode::AnimationSingleNode);
	TestEqual(TEXT("Inactive threat should default to Stone Golem idle animation."), Threat->GetMesh()->AnimationData.AnimToPlay.Get(), IdleAnimation);
	TestTrue(TEXT("Activating the threat should succeed before animation mode coverage."), Threat->ActivateThreat());
	TestEqual(TEXT("Active threat should switch to Stone Golem run animation."), Threat->GetMesh()->AnimationData.AnimToPlay.Get(), RunAnimation);
	TestTrue(TEXT("Active threat should scale run animation playback to chase speed instead of sliding at a fixed loop."), Threat->GetMesh()->GetPlayRate() > 1.0f);
	TestTrue(TEXT("Deactivating the threat should succeed before animation mode coverage."), Threat->DeactivateThreat());
	TestEqual(TEXT("Inactive threat should return to Stone Golem idle animation."), Threat->GetMesh()->AnimationData.AnimToPlay.Get(), IdleAnimation);
	TestEqual(TEXT("Inactive threat idle playback should reset to a natural rate."), Threat->GetMesh()->GetPlayRate(), 1.0f);

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorThreatFullChasePublishesPressureEventTest,
	"HorrorProject.AI.Threat.Day1.FullChasePublishesPressureEvent",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorThreatFullChasePublishesPressureEventTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for golem chase event coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	UHorrorEventBusSubsystem* EventBus = World->GetSubsystem<UHorrorEventBusSubsystem>();
	UHorrorAudioSubsystem* AudioSubsystem = World->GetSubsystem<UHorrorAudioSubsystem>();
	TestNotNull(TEXT("Golem chase event test should expose the event bus."), EventBus);
	TestNotNull(TEXT("Golem chase event test should expose the Day1 audio subsystem."), AudioSubsystem);
	if (!EventBus || !AudioSubsystem)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	const FGameplayTag FullChaseTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Encounter.Golem.FullChase")), false);
	TestTrue(TEXT("Full chase golem event tag should be registered."), FullChaseTag.IsValid());

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AHorrorThreatCharacter* Threat = World->SpawnActor<AHorrorThreatCharacter>(FVector::ZeroVector, FRotator::ZeroRotator, SpawnParameters);
	AHorrorPlayerCharacter* PlayerCharacter = World->SpawnActor<AHorrorPlayerCharacter>(FVector(1200.0f, 0.0f, 0.0f), FRotator::ZeroRotator, SpawnParameters);
	TestNotNull(TEXT("Threat should spawn for golem chase event coverage."), Threat);
	TestNotNull(TEXT("Player target should spawn for golem chase event coverage."), PlayerCharacter);
	UHorrorGolemBehaviorComponent* GolemBehavior = Threat ? Threat->GetGolemBehavior() : nullptr;
	TestNotNull(TEXT("Threat should expose golem behavior for golem chase event coverage."), GolemBehavior);
	if (!Threat || !PlayerCharacter || !GolemBehavior)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	GolemBehavior->ActivateBehavior(PlayerCharacter);
	GolemBehavior->ForceStateTransition(EGolemEncounterState::FullChase);

	const TArray<FHorrorEventMessage>& History = EventBus->GetHistory();
	TestTrue(TEXT("Entering full chase should publish a golem pressure event."), History.ContainsByPredicate(
		[FullChaseTag](const FHorrorEventMessage& Message)
		{
			return Message.EventTag == FullChaseTag;
		}));
	TestEqual(TEXT("Full chase pressure event should move Day1 audio into chase stage."), AudioSubsystem->GetDay1AudioStage(), EHorrorDay1AudioStage::Chase);
	TestEqual(TEXT("Full chase pressure event source should identify the golem actor."), AudioSubsystem->GetLastDay1AudioSourceId(), Threat->GetFName());

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorThreatFinalImpactRequestsCheckpointRecoveryTest,
	"HorrorProject.AI.Threat.Day1.FinalImpactRequestsCheckpointRecovery",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorThreatFinalImpactRequestsCheckpointRecoveryTest::RunTest(const FString& Parameters)
{
	UGameplayStatics::DeleteGameInSlot(Day1ThreatAutosaveSlotName, Day1ThreatAutosaveUserIndex);

	FTestWorldWrapper TestWorld;
	UWorld* World = nullptr;
	AHorrorGameModeBase* GameMode = CreateThreatRuntimeGameMode(*this, TestWorld, World);
	if (!GameMode || !World)
	{
		UGameplayStatics::DeleteGameInSlot(Day1ThreatAutosaveSlotName, Day1ThreatAutosaveUserIndex);
		return false;
	}

	AHorrorPlayerCharacter* PlayerCharacter = SpawnThreatRuntimePlayer(*this, World);
	if (!PlayerCharacter)
	{
		TestWorld.DestroyTestWorld(false);
		UGameplayStatics::DeleteGameInSlot(Day1ThreatAutosaveSlotName, Day1ThreatAutosaveUserIndex);
		return false;
	}

	const FTransform CheckpointTransform(FRotator(0.0f, 10.0f, 0.0f), FVector(25.0f, 50.0f, 120.0f));
	PlayerCharacter->SetActorTransform(CheckpointTransform);
	TestTrue(TEXT("Final impact test should save a Day1 checkpoint first."), GameMode->SaveDay1Checkpoint(TEXT("Checkpoint.Day1.FinalImpact")));

	const FTransform FailedTransform(FRotator(0.0f, -120.0f, 0.0f), FVector(900.0f, -600.0f, 80.0f));
	PlayerCharacter->SetActorTransform(FailedTransform);

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AHorrorThreatCharacter* Threat = World->SpawnActor<AHorrorThreatCharacter>(FailedTransform.GetLocation() + FVector(100.0f, 0.0f, 0.0f), FRotator::ZeroRotator, SpawnParameters);
	TestNotNull(TEXT("Threat should spawn for final impact coverage."), Threat);
	UHorrorGolemBehaviorComponent* GolemBehavior = Threat ? Threat->GetGolemBehavior() : nullptr;
	TestNotNull(TEXT("Threat should expose golem behavior for final impact coverage."), GolemBehavior);
	if (!Threat || !GolemBehavior)
	{
		TestWorld.DestroyTestWorld(false);
		UGameplayStatics::DeleteGameInSlot(Day1ThreatAutosaveSlotName, Day1ThreatAutosaveUserIndex);
		return false;
	}

	GolemBehavior->ActivateBehavior(PlayerCharacter);
	GolemBehavior->ForceStateTransition(EGolemEncounterState::FinalImpact);
	GolemBehavior->TickComponent(0.05f, LEVELTICK_All, nullptr);

	TestEqual(TEXT("Final impact should record the golem death cause."), GameMode->GetLastPlayerFailureCause(), FName(TEXT("Death.Golem.FinalImpact")));
	TestTrue(TEXT("Final impact death should recover from the latest Day1 checkpoint."), GameMode->DidLastPlayerFailureRecoverFromCheckpoint());
	TestTrue(TEXT("Final impact should move the player back to the saved checkpoint."), PlayerCharacter->GetActorLocation().Equals(CheckpointTransform.GetLocation(), KINDA_SMALL_NUMBER));
	TestFalse(TEXT("Final impact should deactivate golem behavior after requesting recovery."), GolemBehavior->IsBehaviorActive());

	UGameplayStatics::DeleteGameInSlot(Day1ThreatAutosaveSlotName, Day1ThreatAutosaveUserIndex);
	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorThreatRevealActivationIsObservableOutsideDetectionRadiusTest,
	"HorrorProject.AI.Threat.Day1.RevealActivationIsObservableOutsideDetectionRadius",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorThreatRevealActivationIsObservableOutsideDetectionRadiusTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for reveal activation observability coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AHorrorThreatCharacter* Threat = World->SpawnActor<AHorrorThreatCharacter>(FVector::ZeroVector, FRotator::ZeroRotator, SpawnParameters);
	AHorrorPlayerCharacter* PlayerCharacter = World->SpawnActor<AHorrorPlayerCharacter>(FVector(2400.0f, 0.0f, 0.0f), FRotator::ZeroRotator, SpawnParameters);
	TestNotNull(TEXT("Threat should spawn for reveal activation observability coverage."), Threat);
	TestNotNull(TEXT("Player target should spawn for reveal activation observability coverage."), PlayerCharacter);
	UHorrorGolemBehaviorComponent* GolemBehavior = Threat ? Threat->GetGolemBehavior() : nullptr;
	TestNotNull(TEXT("Threat should expose golem behavior for reveal activation observability coverage."), GolemBehavior);
	if (!Threat || !PlayerCharacter || !GolemBehavior)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	Threat->DetectionRadius = 100.0f;
	GolemBehavior->ActivateBehavior(PlayerCharacter);

	TestTrue(TEXT("Reveal activation should make the threat visibly active even when the target starts outside sensor radius."), Threat->IsThreatActive());
	TestTrue(TEXT("Reveal activation should make golem behavior active."), GolemBehavior->IsBehaviorActive());
	TestTrue(TEXT("Reveal activation should keep the player as the golem pressure target."), GolemBehavior->GetTargetActor() == PlayerCharacter);
	TestEqual(TEXT("Reveal activation should enter an observable pressure state immediately."), GolemBehavior->GetCurrentState(), EGolemEncounterState::DistantSighting);

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorThreatCloseStalkingUsesFallbackWhenPatrolDirectionDegeneratesTest,
	"HorrorProject.AI.Threat.Day1.CloseStalkingUsesFallbackWhenPatrolDirectionDegenerates",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorThreatCloseStalkingUsesFallbackWhenPatrolDirectionDegeneratesTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for close stalking fallback coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AHorrorThreatCharacter* Threat = World->SpawnActor<AHorrorThreatCharacter>(FVector::ZeroVector, FRotator::ZeroRotator, SpawnParameters);
	AHorrorPlayerCharacter* PlayerCharacter = World->SpawnActor<AHorrorPlayerCharacter>(FVector::ZeroVector, FRotator::ZeroRotator, SpawnParameters);
	TestNotNull(TEXT("Threat should spawn for close stalking fallback coverage."), Threat);
	TestNotNull(TEXT("Player target should spawn for close stalking fallback coverage."), PlayerCharacter);
	UHorrorGolemBehaviorComponent* GolemBehavior = Threat ? Threat->GetGolemBehavior() : nullptr;
	TestNotNull(TEXT("Threat should expose golem behavior for close stalking fallback coverage."), GolemBehavior);
	if (!Threat || !PlayerCharacter || !GolemBehavior)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	GolemBehavior->ActivateBehavior(PlayerCharacter);
	GolemBehavior->ForceStateTransition(EGolemEncounterState::CloseStalking);

	const FVector InitialLocation = Threat->GetActorLocation();
	GolemBehavior->TickComponent(0.1f, LEVELTICK_All, nullptr);
	Threat->Tick(0.1f);

	TestEqual(TEXT("Degenerate close stalking fallback should stay in the pressure state."), GolemBehavior->GetCurrentState(), EGolemEncounterState::CloseStalking);
	TestTrue(TEXT("Degenerate close stalking fallback should keep behavior active."), GolemBehavior->IsBehaviorActive());
	TestTrue(
		TEXT("Degenerate close stalking fallback should still push movement input when no patrol point/path direction is available."),
		Threat->GetPendingMovementInputVector().SizeSquared() > 0.0f);
	TestTrue(
		TEXT("Degenerate close stalking fallback should avoid teleport-style transform pushes while movement is queued."),
		FVector::Dist2D(Threat->GetActorLocation(), InitialLocation) <= KINDA_SMALL_NUMBER);

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorThreatUsesDirectChaseFallbackWithoutNavMeshTest,
	"HorrorProject.AI.Threat.Day1.DirectChaseFallbackWithoutNavMesh",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorThreatChaseTriggeredStageIsPlayerReadableTest,
	"HorrorProject.AI.Threat.Day1.ChaseTriggeredStageIsPlayerReadable",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorThreatChaseTriggeredDoesNotEnterFullChaseInsideMinDistanceTest,
	"HorrorProject.AI.Threat.Day1.ChaseTriggeredDoesNotEnterFullChaseInsideMinDistance",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorThreatChaseTriggeredCanStillEnterFinalImpactInsideAttackDistanceTest,
	"HorrorProject.AI.Threat.Day1.ChaseTriggeredCanStillEnterFinalImpactInsideAttackDistance",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorThreatChaseTriggeredDoesNotEnterFullChaseInsideMinDistanceTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for close chase trigger threshold coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AHorrorThreatCharacter* Threat = World->SpawnActor<AHorrorThreatCharacter>(FVector::ZeroVector, FRotator::ZeroRotator, SpawnParameters);
	AHorrorPlayerCharacter* PlayerCharacter = World->SpawnActor<AHorrorPlayerCharacter>(FVector(800.0f, 0.0f, 0.0f), FRotator::ZeroRotator, SpawnParameters);
	TestNotNull(TEXT("Threat should spawn for close chase trigger threshold coverage."), Threat);
	TestNotNull(TEXT("Player target should spawn for close chase trigger threshold coverage."), PlayerCharacter);
	UHorrorGolemBehaviorComponent* GolemBehavior = Threat ? Threat->GetGolemBehavior() : nullptr;
	TestNotNull(TEXT("Threat should expose golem behavior for close chase trigger threshold coverage."), GolemBehavior);
	if (!Threat || !PlayerCharacter || !GolemBehavior)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	UCharacterMovementComponent* Movement = Threat->GetCharacterMovement();
	TestNotNull(TEXT("Threat should expose movement for close chase trigger threshold coverage."), Movement);
	if (!Movement)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	GolemBehavior->ActivateBehavior(PlayerCharacter);
	GolemBehavior->ForceStateTransition(EGolemEncounterState::ChaseTriggered);

	for (int32 TickIndex = 0; TickIndex < 16; ++TickIndex)
	{
		GolemBehavior->TickComponent(0.1f, LEVELTICK_All, nullptr);
	}

	TestEqual(
		TEXT("Chase triggered should not escalate to full chase while the golem is already inside the configured minimum full-chase distance."),
		GolemBehavior->GetCurrentState(),
		EGolemEncounterState::ChaseTriggered);
	TestTrue(
		TEXT("Close chase trigger should keep the readable speed ramp instead of switching to full chase speed."),
		FMath::IsNearlyEqual(
			Movement->MaxWalkSpeed,
			GolemBehavior->ChaseTriggered_BaseSpeed * GolemBehavior->ChaseTriggered_SpeedMultiplier,
			0.01f));

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

bool FHorrorThreatChaseTriggeredCanStillEnterFinalImpactInsideAttackDistanceTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	UWorld* World = nullptr;
	AHorrorGameModeBase* GameMode = CreateThreatRuntimeGameMode(*this, TestWorld, World);
	if (!GameMode || !World)
	{
		return false;
	}

	AHorrorPlayerCharacter* PlayerCharacter = SpawnThreatRuntimePlayer(*this, World);
	if (!PlayerCharacter)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	PlayerCharacter->SetActorLocation(FVector(400.0f, 0.0f, 0.0f));

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AHorrorThreatCharacter* Threat = World->SpawnActor<AHorrorThreatCharacter>(FVector::ZeroVector, FRotator::ZeroRotator, SpawnParameters);
	UHorrorGolemBehaviorComponent* GolemBehavior = Threat ? Threat->GetGolemBehavior() : nullptr;
	TestNotNull(TEXT("Threat should spawn for close final impact threshold coverage."), Threat);
	TestNotNull(TEXT("Threat should expose golem behavior for close final impact threshold coverage."), GolemBehavior);
	if (!Threat || !GolemBehavior)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	GolemBehavior->ActivateBehavior(PlayerCharacter);
	GolemBehavior->ForceStateTransition(EGolemEncounterState::ChaseTriggered);

	for (int32 TickIndex = 0; TickIndex < 16; ++TickIndex)
	{
		GolemBehavior->TickComponent(0.1f, LEVELTICK_All, nullptr);
	}

	TestEqual(
		TEXT("Chase triggered should still resolve into final impact when the target is inside the configured attack distance."),
		GameMode->GetLastPlayerFailureCause(),
		FName(TEXT("Death.Golem.FinalImpact")));
	TestFalse(TEXT("Final impact should deactivate golem behavior after the close-range attack resolves."), GolemBehavior->IsBehaviorActive());

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

bool FHorrorThreatChaseTriggeredStageIsPlayerReadableTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for readable chase trigger coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AHorrorThreatCharacter* Threat = World->SpawnActor<AHorrorThreatCharacter>(FVector::ZeroVector, FRotator::ZeroRotator, SpawnParameters);
	AHorrorPlayerCharacter* PlayerCharacter = World->SpawnActor<AHorrorPlayerCharacter>(FVector(1400.0f, 0.0f, 0.0f), FRotator::ZeroRotator, SpawnParameters);
	TestNotNull(TEXT("Threat should spawn for readable chase trigger coverage."), Threat);
	TestNotNull(TEXT("Player target should spawn for readable chase trigger coverage."), PlayerCharacter);
	UHorrorGolemBehaviorComponent* GolemBehavior = Threat ? Threat->GetGolemBehavior() : nullptr;
	TestNotNull(TEXT("Threat should expose golem behavior for readable chase trigger coverage."), GolemBehavior);
	if (!Threat || !PlayerCharacter || !GolemBehavior)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	UCharacterMovementComponent* Movement = Threat->GetCharacterMovement();
	TestNotNull(TEXT("Threat should expose movement for readable chase trigger coverage."), Movement);
	if (!Movement)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	GolemBehavior->ActivateBehavior(PlayerCharacter);
	GolemBehavior->ForceStateTransition(EGolemEncounterState::ChaseTriggered);

	GolemBehavior->TickComponent(0.1f, LEVELTICK_All, nullptr);
	TestEqual(
		TEXT("Chase triggered should remain readable instead of immediately escalating to full chase."),
		GolemBehavior->GetCurrentState(),
		EGolemEncounterState::ChaseTriggered);
	TestTrue(
		TEXT("Readable chase trigger should use the configured 70 percent speed ramp."),
		FMath::IsNearlyEqual(
			Movement->MaxWalkSpeed,
			GolemBehavior->ChaseTriggered_BaseSpeed * GolemBehavior->ChaseTriggered_SpeedMultiplier,
			0.01f));
	const float TriggeredRunPlayRate = Threat->GetMesh()->GetPlayRate();
	TestTrue(
		TEXT("Readable chase trigger should slow animation playback to match the 70 percent speed ramp."),
		TriggeredRunPlayRate > 0.7f && TriggeredRunPlayRate < 1.0f);

	for (int32 TickIndex = 0; TickIndex < 14; ++TickIndex)
	{
		GolemBehavior->TickComponent(0.1f, LEVELTICK_All, nullptr);
	}
	TestEqual(
		TEXT("Chase triggered should still escalate to full chase after the readable ramp window."),
		GolemBehavior->GetCurrentState(),
		EGolemEncounterState::FullChase);
	GolemBehavior->TickComponent(0.1f, LEVELTICK_All, nullptr);
	TestTrue(
		TEXT("Full chase should speed animation playback back up to match the faster pursuit phase."),
		Threat->GetMesh()->GetPlayRate() > TriggeredRunPlayRate);

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

bool FHorrorThreatUsesDirectChaseFallbackWithoutNavMeshTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for direct chase fallback coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AHorrorThreatCharacter* Threat = World->SpawnActor<AHorrorThreatCharacter>(FVector::ZeroVector, FRotator::ZeroRotator, SpawnParameters);
	AHorrorPlayerCharacter* PlayerCharacter = World->SpawnActor<AHorrorPlayerCharacter>(FVector(1500.0f, 0.0f, 0.0f), FRotator::ZeroRotator, SpawnParameters);
	TestNotNull(TEXT("Threat should spawn for direct chase fallback coverage."), Threat);
	TestNotNull(TEXT("Player target should spawn for direct chase fallback coverage."), PlayerCharacter);
	UHorrorGolemBehaviorComponent* GolemBehavior = Threat ? Threat->GetGolemBehavior() : nullptr;
	TestNotNull(TEXT("Threat should expose golem behavior for direct chase fallback coverage."), GolemBehavior);
	if (!Threat || !PlayerCharacter || !GolemBehavior)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	GolemBehavior->ActivateBehavior(PlayerCharacter);
	GolemBehavior->ForceStateTransition(EGolemEncounterState::FullChase);

	const float InitialDistance = FVector::Dist2D(Threat->GetActorLocation(), PlayerCharacter->GetActorLocation());
	GolemBehavior->TickComponent(0.1f, LEVELTICK_All, nullptr);
	Threat->Tick(0.1f);

	TestTrue(
		TEXT("Without navmesh, golem chase should still push direct movement input toward the player."),
		Threat->GetPendingMovementInputVector().SizeSquared() > 0.0f);
	TestTrue(
		TEXT("Without navmesh, direct chase fallback should face the player without teleport-style transform pushes."),
		FMath::Abs(Threat->GetActorRotation().Yaw) <= 2.0f);
	TestTrue(
		TEXT("Without navmesh, direct chase fallback should not manually change actor location in the same behavior tick."),
		FMath::IsNearlyEqual(
			static_cast<float>(FVector::Dist2D(Threat->GetActorLocation(), PlayerCharacter->GetActorLocation())),
			InitialDistance,
			KINDA_SMALL_NUMBER));

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorThreatFullChaseFallsBackToStalkingAfterLostTargetGraceTest,
	"HorrorProject.AI.Threat.Day1.FullChaseFallsBackToStalkingAfterLostTargetGrace",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorThreatFullChaseFallsBackToStalkingAfterLostTargetGraceTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for lost target grace coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AHorrorThreatCharacter* Threat = World->SpawnActor<AHorrorThreatCharacter>(FVector::ZeroVector, FRotator::ZeroRotator, SpawnParameters);
	AHorrorPlayerCharacter* PlayerCharacter = World->SpawnActor<AHorrorPlayerCharacter>(FVector(10000.0f, 0.0f, 0.0f), FRotator::ZeroRotator, SpawnParameters);
	TestNotNull(TEXT("Threat should spawn for lost target coverage."), Threat);
	TestNotNull(TEXT("Player target should spawn for lost target coverage."), PlayerCharacter);
	UHorrorGolemBehaviorComponent* GolemBehavior = Threat ? Threat->GetGolemBehavior() : nullptr;
	TestNotNull(TEXT("Threat should expose golem behavior for lost target coverage."), GolemBehavior);
	if (!Threat || !PlayerCharacter || !GolemBehavior)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	GolemBehavior->ChaseLostTargetDistance = 2000.0f;
	GolemBehavior->ChaseLostTargetGraceTime = 0.25f;
	GolemBehavior->ActivateBehavior(PlayerCharacter);
	GolemBehavior->ForceStateTransition(EGolemEncounterState::FullChase);

	GolemBehavior->TickComponent(0.1f, LEVELTICK_All, nullptr);
	TestEqual(
		TEXT("Lost target grace should keep full chase active until the grace window expires."),
		GolemBehavior->GetCurrentState(),
		EGolemEncounterState::FullChase);

	GolemBehavior->TickComponent(0.2f, LEVELTICK_All, nullptr);
	TestEqual(
		TEXT("Full chase should fall back to close stalking once the target stays lost beyond the grace window."),
		GolemBehavior->GetCurrentState(),
		EGolemEncounterState::CloseStalking);
	TestTrue(TEXT("Lost target fallback should keep golem behavior active for reacquisition."), GolemBehavior->IsBehaviorActive());

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorThreatDeactivatesWhenChaseTargetIsDestroyedTest,
	"HorrorProject.AI.Threat.Day1.DeactivatesWhenChaseTargetIsDestroyed",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorThreatDeactivatesWhenChaseTargetIsDestroyedTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for destroyed chase target coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AHorrorThreatCharacter* Threat = World->SpawnActor<AHorrorThreatCharacter>(FVector::ZeroVector, FRotator::ZeroRotator, SpawnParameters);
	AHorrorPlayerCharacter* PlayerCharacter = World->SpawnActor<AHorrorPlayerCharacter>(FVector(1500.0f, 0.0f, 0.0f), FRotator::ZeroRotator, SpawnParameters);
	TestNotNull(TEXT("Threat should spawn for destroyed target coverage."), Threat);
	TestNotNull(TEXT("Player target should spawn for destroyed target coverage."), PlayerCharacter);
	UHorrorGolemBehaviorComponent* GolemBehavior = Threat ? Threat->GetGolemBehavior() : nullptr;
	TestNotNull(TEXT("Threat should expose golem behavior for destroyed target coverage."), GolemBehavior);
	if (!Threat || !PlayerCharacter || !GolemBehavior)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	GolemBehavior->ActivateBehavior(PlayerCharacter);
	GolemBehavior->ForceStateTransition(EGolemEncounterState::FullChase);
	TestTrue(TEXT("Behavior should be active before the chase target is destroyed."), GolemBehavior->IsBehaviorActive());

	PlayerCharacter->Destroy();
	GolemBehavior->TickComponent(0.1f, LEVELTICK_All, nullptr);

	TestFalse(TEXT("Destroyed chase targets should deactivate golem behavior instead of leaving a stale active chase."), GolemBehavior->IsBehaviorActive());
	TestEqual(TEXT("Destroyed chase target deactivation should return the golem to dormant."), GolemBehavior->GetCurrentState(), EGolemEncounterState::Dormant);
	TestNull(TEXT("Destroyed chase target deactivation should clear the target actor."), GolemBehavior->GetTargetActor());

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorThreatFinalImpactDeactivatesOnceAndStopsMovementTest,
	"HorrorProject.AI.Threat.Day1.FinalImpactDeactivatesOnceAndStopsMovement",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorThreatFinalImpactDeactivatesOnceAndStopsMovementTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	UWorld* World = nullptr;
	AHorrorGameModeBase* GameMode = CreateThreatRuntimeGameMode(*this, TestWorld, World);
	if (!GameMode || !World)
	{
		return false;
	}

	AHorrorPlayerCharacter* PlayerCharacter = SpawnThreatRuntimePlayer(*this, World);
	if (!PlayerCharacter)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AHorrorThreatCharacter* Threat = World->SpawnActor<AHorrorThreatCharacter>(FVector(100.0f, 0.0f, 0.0f), FRotator::ZeroRotator, SpawnParameters);
	UHorrorGolemBehaviorComponent* GolemBehavior = Threat ? Threat->GetGolemBehavior() : nullptr;
	TestNotNull(TEXT("Threat should spawn for final impact movement coverage."), Threat);
	TestNotNull(TEXT("Threat should expose golem behavior for final impact movement coverage."), GolemBehavior);
	if (!Threat || !GolemBehavior)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	UCharacterMovementComponent* Movement = Threat->GetCharacterMovement();
	TestNotNull(TEXT("Threat should expose character movement for final impact movement coverage."), Movement);
	if (!Movement)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	Movement->Velocity = FVector(450.0f, 0.0f, 0.0f);
	GolemBehavior->ActivateBehavior(PlayerCharacter);
	GolemBehavior->ForceStateTransition(EGolemEncounterState::FinalImpact);
	GolemBehavior->TickComponent(0.05f, LEVELTICK_All, nullptr);
	GolemBehavior->TickComponent(0.05f, LEVELTICK_All, nullptr);

	TestEqual(TEXT("Final impact should request the golem death cause exactly once."), GameMode->GetLastPlayerFailureCause(), FName(TEXT("Death.Golem.FinalImpact")));
	TestFalse(TEXT("Final impact should deactivate golem behavior after requesting failure recovery."), GolemBehavior->IsBehaviorActive());
	TestEqual(TEXT("Final impact should return the golem to dormant after deactivation."), GolemBehavior->GetCurrentState(), EGolemEncounterState::Dormant);
	TestTrue(TEXT("Final impact should stop threat movement immediately."), Movement->Velocity.IsNearlyZero());

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

#endif
