// Copyright Epic Games, Inc. All Rights Reserved.

#include "AI/HorrorGolemBehaviorComponent.h"
#include "AI/HorrorThreatCharacter.h"
#include "AIController.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetMathLibrary.h"
#include "Navigation/PathFollowingComponent.h"
#include "NavigationPath.h"
#include "NavigationSystem.h"

namespace HorrorGolemBehavior
{
	constexpr float BehaviorTickIntervalSeconds = 0.1f;
	constexpr float ChaseTriggerMinimumSeconds = 3.0f;
	constexpr float PatrolPauseChancePerTick = 0.01f;
	constexpr float FullChaseDestructionChancePerTick = 0.05f;
	constexpr float DirectionalBlendWeight = 0.5f;
	constexpr float PatrolDistanceToleranceCm = 100.0f;
	constexpr float DistantSightRotationInterpSpeed = 2.0f;
	constexpr float MoveRotationInterpSpeed = 5.0f;
	constexpr float PatrolRotationInterpSpeed = 3.0f;
	constexpr float NavigationAcceptanceRadiusCm = 75.0f;
	constexpr float PatrolNavigationStepCm = 300.0f;
	constexpr float FinalImpactTriggerGraceSeconds = 0.1f;
	constexpr float EnvironmentDestructionForwardOffsetCm = 200.0f;
	constexpr float DebugStateHeightOffsetCm = 200.0f;
	constexpr float DebugTextHeightOffsetCm = 250.0f;
	constexpr float DebugStateSphereRadiusCm = 50.0f;
	constexpr int32 DebugSphereSegments = 16;
	constexpr float DebugLifetimeSeconds = 1.0f;
	constexpr float DebugLineThickness = 2.0f;
}

UHorrorGolemBehaviorComponent::UHorrorGolemBehaviorComponent()
{
	// Performance optimization: Reduce tick frequency for AI behavior
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	PrimaryComponentTick.TickInterval = HorrorGolemBehavior::BehaviorTickIntervalSeconds;
}

void UHorrorGolemBehaviorComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerThreat = Cast<AHorrorThreatCharacter>(GetOwner());
}

void UHorrorGolemBehaviorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bBehaviorActive || !TargetActor.IsValid() || !OwnerThreat.IsValid())
	{
		return;
	}

	StateTimer += DeltaTime;
	UpdateStateMachine(DeltaTime);

	if (bDrawDebugInfo)
	{
		DrawDebugState();
	}
}

void UHorrorGolemBehaviorComponent::ActivateBehavior(AActor* InTargetActor)
{
	if (!IsValid(InTargetActor) || bBehaviorActive)
	{
		return;
	}

	TargetActor = InTargetActor;
	bBehaviorActive = true;
	StateTimer = 0.0f;
	LostTargetTimer = 0.0f;

	SetComponentTickEnabled(true);
	TransitionToState(EGolemEncounterState::DistantSighting);

	if (OwnerThreat.IsValid())
	{
		OwnerThreat->ActivateThreat();
		OwnerThreat->UpdateDetectedTarget(InTargetActor);
	}
}

void UHorrorGolemBehaviorComponent::DeactivateBehavior()
{
	if (!bBehaviorActive)
	{
		return;
	}

	bBehaviorActive = false;
	SetComponentTickEnabled(false);
	StopNavigationMove();
	TransitionToState(EGolemEncounterState::Dormant);

	if (OwnerThreat.IsValid())
	{
		OwnerThreat->DeactivateThreat();
	}

	TargetActor.Reset();
}

float UHorrorGolemBehaviorComponent::GetDistanceToTarget() const
{
	if (!TargetActor.IsValid() || !OwnerThreat.IsValid())
	{
		return -1.0f;
	}

	return FVector::Dist(OwnerThreat->GetActorLocation(), TargetActor->GetActorLocation());
}

void UHorrorGolemBehaviorComponent::ForceStateTransition(EGolemEncounterState NewState)
{
	TransitionToState(NewState);
}

void UHorrorGolemBehaviorComponent::UpdateStateMachine(float DeltaTime)
{
	const float DistanceToTarget = GetDistanceToTarget();
	if (DistanceToTarget < 0.0f)
	{
		return;
	}

	// State transition logic
	switch (CurrentState)
	{
		case EGolemEncounterState::DistantSighting:
			UpdateDistantSightingState(DistanceToTarget, DeltaTime);
			break;

		case EGolemEncounterState::CloseStalking:
			UpdateCloseStalkingState(DistanceToTarget, DeltaTime);
			break;

		case EGolemEncounterState::ChaseTriggered:
			UpdateChaseTriggeredState(DistanceToTarget, DeltaTime);
			break;

		case EGolemEncounterState::FullChase:
			UpdateFullChaseState(DistanceToTarget, DeltaTime);
			break;

		case EGolemEncounterState::FinalImpact:
			UpdateFinalImpact(DeltaTime);
			break;

		default:
			break;
	}
}

void UHorrorGolemBehaviorComponent::UpdateDistantSightingState(float DistanceToTarget, float DeltaTime)
{
	if (DistanceToTarget <= CloseStalking_MaxDistance)
	{
		TransitionToState(EGolemEncounterState::CloseStalking);
		return;
	}

	UpdateDistantSighting(DeltaTime);
}

void UHorrorGolemBehaviorComponent::UpdateCloseStalkingState(float DistanceToTarget, float DeltaTime)
{
	if (DistanceToTarget <= ChaseTriggered_StartDistance && StateTimer >= HorrorGolemBehavior::ChaseTriggerMinimumSeconds)
	{
		TransitionToState(EGolemEncounterState::ChaseTriggered);
		return;
	}

	if (DistanceToTarget > CloseStalking_MaxDistance)
	{
		TransitionToState(EGolemEncounterState::DistantSighting);
		return;
	}

	UpdateCloseStalking(DeltaTime);
}

void UHorrorGolemBehaviorComponent::UpdateChaseTriggeredState(float DistanceToTarget, float DeltaTime)
{
	if (HandleLostTarget(DistanceToTarget, DeltaTime))
	{
		return;
	}

	if (DistanceToTarget <= FullChase_MaxDistance)
	{
		TransitionToState(EGolemEncounterState::FullChase);
		return;
	}

	UpdateChaseTriggered(DeltaTime);
}

void UHorrorGolemBehaviorComponent::UpdateFullChaseState(float DistanceToTarget, float DeltaTime)
{
	if (HandleLostTarget(DistanceToTarget, DeltaTime))
	{
		return;
	}

	if (DistanceToTarget <= FinalImpact_TriggerDistance)
	{
		TransitionToState(EGolemEncounterState::FinalImpact);
		return;
	}

	UpdateFullChase(DeltaTime);
}

void UHorrorGolemBehaviorComponent::TransitionToState(EGolemEncounterState NewState)
{
	if (CurrentState == NewState)
	{
		return;
	}

	const EGolemEncounterState OldState = CurrentState;
	CurrentState = NewState;
	StateTimer = 0.0f;
	LostTargetTimer = 0.0f;
	PatrolPauseTimer = 0.0f;
	bPatrolPaused = false;

	OnStateChanged.Broadcast(OldState, NewState);
	BP_OnStateChanged(OldState, NewState);
}

void UHorrorGolemBehaviorComponent::UpdateDistantSighting(float DeltaTime)
{
	if (!OwnerThreat.IsValid() || !TargetActor.IsValid())
	{
		return;
	}

	// Stationary or slow movement
	if (!bDistantSightingStationary)
	{
		MoveTowardsTarget(DistantSightingMoveSpeed, DeltaTime);
	}

	// Performance optimization: Cache locations to avoid multiple GetActorLocation calls
	const FVector OwnerLocation = OwnerThreat->GetActorLocation();
	const FVector TargetLocation = TargetActor->GetActorLocation();
	const FVector DirectionToTarget = (TargetLocation - OwnerLocation).GetSafeNormal();
	const FRotator TargetRotation = DirectionToTarget.Rotation();
	const FRotator NewRotation = FMath::RInterpTo(OwnerThreat->GetActorRotation(), TargetRotation, DeltaTime, HorrorGolemBehavior::DistantSightRotationInterpSpeed);
	OwnerThreat->SetActorRotation(NewRotation);

	BP_OnDistantSightingUpdate(DeltaTime);
}

void UHorrorGolemBehaviorComponent::UpdateCloseStalking(float DeltaTime)
{
	if (!OwnerThreat.IsValid() || !TargetActor.IsValid())
	{
		return;
	}

	// Patrol behavior with pauses
	if (bPatrolPaused)
	{
		PatrolPauseTimer += DeltaTime;
		if (PatrolPauseTimer >= CloseStalking_PatrolPauseTime)
		{
			bPatrolPaused = false;
			PatrolPauseTimer = 0.0f;
		}
	}
	else
	{
		PatrolAroundTarget(DeltaTime);

		// Random pause
		if (FMath::FRand() < HorrorGolemBehavior::PatrolPauseChancePerTick)
		{
			bPatrolPaused = true;
		}
	}

	BP_OnCloseStalking_PatrolUpdate(DeltaTime);
}

void UHorrorGolemBehaviorComponent::UpdateChaseTriggered(float DeltaTime)
{
	if (!OwnerThreat.IsValid())
	{
		return;
	}

	const float ChaseSpeed = ChaseTriggered_BaseSpeed * ChaseTriggered_SpeedMultiplier;
	MoveTowardsTarget(ChaseSpeed, DeltaTime);

	BP_OnChaseUpdate(DeltaTime, ChaseSpeed);
}

void UHorrorGolemBehaviorComponent::UpdateFullChase(float DeltaTime)
{
	if (!OwnerThreat.IsValid())
	{
		return;
	}

	MoveTowardsTarget(FullChase_Speed, DeltaTime);

	// Environment destruction check
	if (bFullChase_EnableDestruction && FMath::FRand() < HorrorGolemBehavior::FullChaseDestructionChancePerTick)
	{
		CheckEnvironmentDestruction();
	}

	BP_OnChaseUpdate(DeltaTime, FullChase_Speed);
}

void UHorrorGolemBehaviorComponent::UpdateFinalImpact(float DeltaTime)
{
	if (StateTimer < HorrorGolemBehavior::FinalImpactTriggerGraceSeconds)
	{
		BP_OnFinalImpactTriggered();
	}

	// Stop movement, trigger attack animation/cutscene
	if (OwnerThreat.IsValid())
	{
		StopNavigationMove();

		ACharacter* ThreatCharacter = Cast<ACharacter>(OwnerThreat.Get());
		if (ThreatCharacter && ThreatCharacter->GetCharacterMovement())
		{
			ThreatCharacter->GetCharacterMovement()->StopMovementImmediately();
		}
	}
}

bool UHorrorGolemBehaviorComponent::HandleLostTarget(float DistanceToTarget, float DeltaTime)
{
	if (ChaseLostTargetDistance <= 0.0f || DistanceToTarget <= ChaseLostTargetDistance)
	{
		LostTargetTimer = 0.0f;
		return false;
	}

	LostTargetTimer += DeltaTime;
	if (LostTargetTimer < ChaseLostTargetGraceTime)
	{
		return false;
	}

	StopNavigationMove();
	TransitionToState(EGolemEncounterState::CloseStalking);
	return true;
}

bool UHorrorGolemBehaviorComponent::MoveToNavigableLocation(const FVector& Destination, float Speed, float AcceptanceRadius)
{
	if (!OwnerThreat.IsValid())
	{
		return false;
	}

	ACharacter* ThreatCharacter = Cast<ACharacter>(OwnerThreat.Get());
	if (!ThreatCharacter || !ThreatCharacter->GetCharacterMovement())
	{
		return false;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	AAIController* AIController = Cast<AAIController>(ThreatCharacter->GetController());
	if (!AIController)
	{
		ThreatCharacter->SpawnDefaultController();
		AIController = Cast<AAIController>(ThreatCharacter->GetController());
	}

	if (!AIController)
	{
		return false;
	}

	UNavigationPath* NavigationPath = UNavigationSystemV1::FindPathToLocationSynchronously(
		World,
		ThreatCharacter->GetActorLocation(),
		Destination,
		ThreatCharacter);

	if (!NavigationPath || !NavigationPath->IsValid() || NavigationPath->PathPoints.Num() == 0)
	{
		AIController->StopMovement();
		return false;
	}

	ThreatCharacter->GetCharacterMovement()->MaxWalkSpeed = Speed;
	const FVector MoveDestination = NavigationPath->PathPoints.Last();
	const EPathFollowingRequestResult::Type MoveResult = AIController->MoveToLocation(
		MoveDestination,
		AcceptanceRadius,
		true,
		true,
		true,
		false,
		nullptr,
		true);

	return MoveResult != EPathFollowingRequestResult::Failed;
}

void UHorrorGolemBehaviorComponent::MoveTowardsTarget(float Speed, float DeltaTime)
{
	if (!OwnerThreat.IsValid() || !TargetActor.IsValid())
	{
		return;
	}

	ACharacter* ThreatCharacter = Cast<ACharacter>(OwnerThreat.Get());
	if (!ThreatCharacter || !ThreatCharacter->GetCharacterMovement())
	{
		return;
	}

	const FVector CurrentLocation = OwnerThreat->GetActorLocation();
	const FVector TargetLocation = TargetActor->GetActorLocation();
	const FVector Direction = (TargetLocation - CurrentLocation).GetSafeNormal();

	MoveToNavigableLocation(TargetLocation, Speed, HorrorGolemBehavior::NavigationAcceptanceRadiusCm);

	// Face target
	const FRotator TargetRotation = Direction.Rotation();
	const FRotator NewRotation = FMath::RInterpTo(OwnerThreat->GetActorRotation(), TargetRotation, DeltaTime, HorrorGolemBehavior::MoveRotationInterpSpeed);
	OwnerThreat->SetActorRotation(NewRotation);
}

void UHorrorGolemBehaviorComponent::PatrolAroundTarget(float DeltaTime)
{
	if (!OwnerThreat.IsValid() || !TargetActor.IsValid())
	{
		return;
	}

	ACharacter* ThreatCharacter = Cast<ACharacter>(OwnerThreat.Get());
	if (!ThreatCharacter || !ThreatCharacter->GetCharacterMovement())
	{
		return;
	}

	const FVector TargetLocation = TargetActor->GetActorLocation();
	const FVector CurrentLocation = OwnerThreat->GetActorLocation();

	// Circle around target at stalking distance
	const float DesiredDistance = (CloseStalking_MinDistance + CloseStalking_MaxDistance) * 0.5f;
	const FVector ToTarget = TargetLocation - CurrentLocation;
	const float CurrentDistance = ToTarget.Size();

	// Tangent direction for circling
	const FVector CircleDirection = FVector::CrossProduct(ToTarget.GetSafeNormal(), FVector::UpVector);

	// Blend between moving closer/away and circling
	FVector MoveDirection = CircleDirection;
	if (CurrentDistance > DesiredDistance + HorrorGolemBehavior::PatrolDistanceToleranceCm)
	{
		MoveDirection += ToTarget.GetSafeNormal() * HorrorGolemBehavior::DirectionalBlendWeight;
	}
	else if (CurrentDistance < DesiredDistance - HorrorGolemBehavior::PatrolDistanceToleranceCm)
	{
		MoveDirection -= ToTarget.GetSafeNormal() * HorrorGolemBehavior::DirectionalBlendWeight;
	}

	MoveDirection.Normalize();

	ThreatCharacter->GetCharacterMovement()->MaxWalkSpeed = CloseStalking_PatrolSpeed;
	const FVector PatrolDestination = CurrentLocation + MoveDirection * HorrorGolemBehavior::PatrolNavigationStepCm;
	MoveToNavigableLocation(PatrolDestination, CloseStalking_PatrolSpeed, HorrorGolemBehavior::NavigationAcceptanceRadiusCm);

	// Face movement direction
	const FRotator TargetRotation = MoveDirection.Rotation();
	const FRotator NewRotation = FMath::RInterpTo(OwnerThreat->GetActorRotation(), TargetRotation, DeltaTime, HorrorGolemBehavior::PatrolRotationInterpSpeed);
	OwnerThreat->SetActorRotation(NewRotation);
}

void UHorrorGolemBehaviorComponent::StopNavigationMove()
{
	if (!OwnerThreat.IsValid())
	{
		return;
	}

	if (ACharacter* ThreatCharacter = Cast<ACharacter>(OwnerThreat.Get()))
	{
		if (AAIController* AIController = Cast<AAIController>(ThreatCharacter->GetController()))
		{
			AIController->StopMovement();
		}
	}
}

void UHorrorGolemBehaviorComponent::CheckEnvironmentDestruction()
{
	if (!OwnerThreat.IsValid())
	{
		return;
	}

	const FVector DestructionLocation =
		OwnerThreat->GetActorLocation() + OwnerThreat->GetActorForwardVector() * HorrorGolemBehavior::EnvironmentDestructionForwardOffsetCm;
	BP_OnEnvironmentDestruction(DestructionLocation);
}

void UHorrorGolemBehaviorComponent::DrawDebugState()
{
	if (!OwnerThreat.IsValid())
	{
		return;
	}

	const UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const FVector GolemLocation = OwnerThreat->GetActorLocation();
	const FColor StateColor = GetDebugStateColor();

	DrawDebugStateMarker(World, GolemLocation, StateColor);
	DrawDebugTargetInfo(World, GolemLocation, StateColor);
	DrawDebugDistanceThresholds(World, GolemLocation);
}

FColor UHorrorGolemBehaviorComponent::GetDebugStateColor() const
{
	switch (CurrentState)
	{
		case EGolemEncounterState::Dormant:
			return FColor::Black;
		case EGolemEncounterState::DistantSighting:
			return FColor::Blue;
		case EGolemEncounterState::CloseStalking:
			return FColor::Yellow;
		case EGolemEncounterState::ChaseTriggered:
			return FColor::Orange;
		case EGolemEncounterState::FullChase:
			return FColor::Red;
		case EGolemEncounterState::FinalImpact:
			return FColor::Magenta;
	}

	return FColor::White;
}

void UHorrorGolemBehaviorComponent::DrawDebugStateMarker(const UWorld* World, const FVector& GolemLocation, FColor StateColor) const
{
	DrawDebugSphere(
		World,
		GolemLocation + FVector(0.0f, 0.0f, HorrorGolemBehavior::DebugStateHeightOffsetCm),
		HorrorGolemBehavior::DebugStateSphereRadiusCm,
		HorrorGolemBehavior::DebugSphereSegments,
		StateColor,
		false,
		-1.0f,
		0,
		HorrorGolemBehavior::DebugLineThickness);
}

void UHorrorGolemBehaviorComponent::DrawDebugTargetInfo(const UWorld* World, const FVector& GolemLocation, FColor StateColor) const
{
	if (!TargetActor.IsValid())
	{
		return;
	}

	const FVector TargetLocation = TargetActor->GetActorLocation();
	DrawDebugLine(World, GolemLocation, TargetLocation, StateColor, false, -1.0f, 0, 2.0f);

	const float Distance = GetDistanceToTarget();
	const FString DebugText = FString::Printf(TEXT("State: %d\nDist: %.0f cm\nTimer: %.1f s"),
		static_cast<int32>(CurrentState), Distance, StateTimer);
	DrawDebugString(
		World,
		GolemLocation + FVector(0.0f, 0.0f, HorrorGolemBehavior::DebugTextHeightOffsetCm),
		DebugText,
		nullptr,
		StateColor,
		0.0f,
		true);
}

void UHorrorGolemBehaviorComponent::DrawDebugDistanceThresholds(const UWorld* World, const FVector& GolemLocation) const
{
	DrawDebugSphere(World, GolemLocation, DistantSightingMinDistance, HorrorGolemBehavior::DebugSphereSegments, FColor::Blue, false, -1.0f, 0, HorrorGolemBehavior::DebugLifetimeSeconds);
	DrawDebugSphere(World, GolemLocation, CloseStalking_MaxDistance, HorrorGolemBehavior::DebugSphereSegments, FColor::Yellow, false, -1.0f, 0, HorrorGolemBehavior::DebugLifetimeSeconds);
	DrawDebugSphere(World, GolemLocation, ChaseTriggered_StartDistance, HorrorGolemBehavior::DebugSphereSegments, FColor::Orange, false, -1.0f, 0, HorrorGolemBehavior::DebugLifetimeSeconds);
	DrawDebugSphere(World, GolemLocation, FullChase_MaxDistance, HorrorGolemBehavior::DebugSphereSegments, FColor::Red, false, -1.0f, 0, HorrorGolemBehavior::DebugLifetimeSeconds);
	DrawDebugSphere(World, GolemLocation, FinalImpact_TriggerDistance, HorrorGolemBehavior::DebugSphereSegments, FColor::Magenta, false, -1.0f, 0, HorrorGolemBehavior::DebugLifetimeSeconds);
}
