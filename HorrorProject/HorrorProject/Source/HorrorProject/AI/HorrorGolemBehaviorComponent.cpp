// Copyright Epic Games, Inc. All Rights Reserved.

#include "AI/HorrorGolemBehaviorComponent.h"
#include "AI/HorrorThreatCharacter.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetMathLibrary.h"

UHorrorGolemBehaviorComponent::UHorrorGolemBehaviorComponent()
{
	// Performance optimization: Reduce tick frequency for AI behavior
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	PrimaryComponentTick.TickInterval = 0.1f; // Update at 10Hz instead of 60Hz
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
			if (DistanceToTarget <= CloseStalking_MaxDistance)
			{
				TransitionToState(EGolemEncounterState::CloseStalking);
			}
			else
			{
				UpdateDistantSighting(DeltaTime);
			}
			break;

		case EGolemEncounterState::CloseStalking:
			if (DistanceToTarget <= ChaseTriggered_StartDistance && StateTimer >= 3.0f)
			{
				TransitionToState(EGolemEncounterState::ChaseTriggered);
			}
			else if (DistanceToTarget > CloseStalking_MaxDistance)
			{
				TransitionToState(EGolemEncounterState::DistantSighting);
			}
			else
			{
				UpdateCloseStalking(DeltaTime);
			}
			break;

		case EGolemEncounterState::ChaseTriggered:
			if (DistanceToTarget <= FullChase_MaxDistance)
			{
				TransitionToState(EGolemEncounterState::FullChase);
			}
			else
			{
				UpdateChaseTriggered(DeltaTime);
			}
			break;

		case EGolemEncounterState::FullChase:
			if (DistanceToTarget <= FinalImpact_TriggerDistance)
			{
				TransitionToState(EGolemEncounterState::FinalImpact);
			}
			else
			{
				UpdateFullChase(DeltaTime);
			}
			break;

		case EGolemEncounterState::FinalImpact:
			UpdateFinalImpact(DeltaTime);
			break;

		default:
			break;
	}
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
	const FRotator NewRotation = FMath::RInterpTo(OwnerThreat->GetActorRotation(), TargetRotation, DeltaTime, 2.0f);
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
		if (FMath::FRand() < 0.01f)
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
	if (bFullChase_EnableDestruction && FMath::FRand() < 0.05f)
	{
		CheckEnvironmentDestruction();
	}

	BP_OnChaseUpdate(DeltaTime, FullChase_Speed);
}

void UHorrorGolemBehaviorComponent::UpdateFinalImpact(float DeltaTime)
{
	if (StateTimer < 0.1f)
	{
		BP_OnFinalImpactTriggered();
	}

	// Stop movement, trigger attack animation/cutscene
	if (OwnerThreat.IsValid())
	{
		ACharacter* ThreatCharacter = Cast<ACharacter>(OwnerThreat.Get());
		if (ThreatCharacter && ThreatCharacter->GetCharacterMovement())
		{
			ThreatCharacter->GetCharacterMovement()->StopMovementImmediately();
		}
	}
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

	// Set movement speed
	ThreatCharacter->GetCharacterMovement()->MaxWalkSpeed = Speed;

	// Move towards target
	const FVector NewLocation = CurrentLocation + Direction * Speed * DeltaTime;
	ThreatCharacter->SetActorLocation(NewLocation, true);

	// Face target
	const FRotator TargetRotation = Direction.Rotation();
	const FRotator NewRotation = FMath::RInterpTo(OwnerThreat->GetActorRotation(), TargetRotation, DeltaTime, 5.0f);
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
	if (CurrentDistance > DesiredDistance + 100.0f)
	{
		MoveDirection += ToTarget.GetSafeNormal() * 0.5f;
	}
	else if (CurrentDistance < DesiredDistance - 100.0f)
	{
		MoveDirection -= ToTarget.GetSafeNormal() * 0.5f;
	}

	MoveDirection.Normalize();

	ThreatCharacter->GetCharacterMovement()->MaxWalkSpeed = CloseStalking_PatrolSpeed;
	const FVector NewLocation = CurrentLocation + MoveDirection * CloseStalking_PatrolSpeed * DeltaTime;
	ThreatCharacter->SetActorLocation(NewLocation, true);

	// Face movement direction
	const FRotator TargetRotation = MoveDirection.Rotation();
	const FRotator NewRotation = FMath::RInterpTo(OwnerThreat->GetActorRotation(), TargetRotation, DeltaTime, 3.0f);
	OwnerThreat->SetActorRotation(NewRotation);
}

void UHorrorGolemBehaviorComponent::CheckEnvironmentDestruction()
{
	if (!OwnerThreat.IsValid())
	{
		return;
	}

	const FVector DestructionLocation = OwnerThreat->GetActorLocation() + OwnerThreat->GetActorForwardVector() * 200.0f;
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
	FColor StateColor = FColor::White;

	switch (CurrentState)
	{
		case EGolemEncounterState::Dormant:
			StateColor = FColor::Black;
			break;
		case EGolemEncounterState::DistantSighting:
			StateColor = FColor::Blue;
			break;
		case EGolemEncounterState::CloseStalking:
			StateColor = FColor::Yellow;
			break;
		case EGolemEncounterState::ChaseTriggered:
			StateColor = FColor::Orange;
			break;
		case EGolemEncounterState::FullChase:
			StateColor = FColor::Red;
			break;
		case EGolemEncounterState::FinalImpact:
			StateColor = FColor::Magenta;
			break;
	}

	// Draw state sphere
	DrawDebugSphere(World, GolemLocation + FVector(0, 0, 200), 50.0f, 12, StateColor, false, -1.0f, 0, 2.0f);

	// Draw distance to target
	if (TargetActor.IsValid())
	{
		const FVector TargetLocation = TargetActor->GetActorLocation();
		DrawDebugLine(World, GolemLocation, TargetLocation, StateColor, false, -1.0f, 0, 2.0f);

		const float Distance = GetDistanceToTarget();
		const FString DebugText = FString::Printf(TEXT("State: %d\nDist: %.0f cm\nTimer: %.1f s"),
			static_cast<int32>(CurrentState), Distance, StateTimer);
		DrawDebugString(World, GolemLocation + FVector(0, 0, 250), DebugText, nullptr, StateColor, 0.0f, true);
	}

	// Draw phase distance thresholds
	DrawDebugSphere(World, GolemLocation, DistantSightingMinDistance, 16, FColor::Blue, false, -1.0f, 0, 1.0f);
	DrawDebugSphere(World, GolemLocation, CloseStalking_MaxDistance, 16, FColor::Yellow, false, -1.0f, 0, 1.0f);
	DrawDebugSphere(World, GolemLocation, ChaseTriggered_StartDistance, 16, FColor::Orange, false, -1.0f, 0, 1.0f);
	DrawDebugSphere(World, GolemLocation, FullChase_MaxDistance, 16, FColor::Red, false, -1.0f, 0, 1.0f);
	DrawDebugSphere(World, GolemLocation, FinalImpact_TriggerDistance, 16, FColor::Magenta, false, -1.0f, 0, 1.0f);
}
