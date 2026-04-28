// Copyright Epic Games, Inc. All Rights Reserved.

#include "AI/HorrorThreatCharacter.h"

#include "AI/HorrorGolemBehaviorComponent.h"
#include "AI/HorrorThreatAIController.h"
#include "Animation/AnimationAsset.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "UObject/ConstructorHelpers.h"

namespace
{
	constexpr float DefaultThreatCapsuleRadiusCm = 48.0f;
	constexpr float DefaultThreatCapsuleHalfHeightCm = 110.0f;
	const FVector DefaultGolemMeshRelativeLocation(0.0f, 0.0f, -DefaultThreatCapsuleHalfHeightCm);
	const FRotator DefaultGolemMeshRelativeRotation(0.0f, -90.0f, 0.0f);
	const FVector DefaultGolemMeshRelativeScale(1.0f, 1.0f, 1.0f);
}

void UHorrorThreatDelegateProbe::HandleThreatActiveChanged(bool bIsActive)
{
	ActiveValues.Add(bIsActive);
}

void UHorrorThreatDelegateProbe::HandleDetectedTargetChanged(AActor* DetectedTarget)
{
	TargetValues.Add(DetectedTarget);
}

AHorrorThreatCharacter::AHorrorThreatCharacter()
{
	PrimaryActorTick.bCanEverTick = false;
	AIControllerClass = AHorrorThreatAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	GetCapsuleComponent()->InitCapsuleSize(DefaultThreatCapsuleRadiusCm, DefaultThreatCapsuleHalfHeightCm);

	static ConstructorHelpers::FObjectFinder<USkeletalMesh> GolemMesh(
		TEXT("/Game/Stone_Golem/mesh/SKM_Stone_Golem.SKM_Stone_Golem"));
	static ConstructorHelpers::FObjectFinder<UAnimationAsset> GolemIdleAnimation(
		TEXT("/Game/Stone_Golem/demo/animations/ThirdPersonIdle.ThirdPersonIdle"));
	static ConstructorHelpers::FObjectFinder<UAnimationAsset> GolemRunAnimation(
		TEXT("/Game/Stone_Golem/demo/animations/ThirdPersonRun.ThirdPersonRun"));

	IdleAnimation = GolemIdleAnimation.Object;
	RunAnimation = GolemRunAnimation.Object;

	if (GolemMesh.Succeeded() && GetMesh())
	{
		GetMesh()->SetSkeletalMesh(GolemMesh.Object);
		GetMesh()->SetRelativeLocation(DefaultGolemMeshRelativeLocation);
		GetMesh()->SetRelativeRotation(DefaultGolemMeshRelativeRotation);
		GetMesh()->SetRelativeScale3D(DefaultGolemMeshRelativeScale);
		GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	ApplyThreatAnimation(IdleAnimation);

	GolemBehavior = CreateDefaultSubobject<UHorrorGolemBehaviorComponent>(TEXT("GolemBehavior"));
}

bool AHorrorThreatCharacter::ActivateThreat()
{
	if (bThreatActive)
	{
		return false;
	}

	bThreatActive = true;
	ApplyThreatAnimation(RunAnimation);
	OnThreatActiveChanged.Broadcast(bThreatActive);
	return true;
}

bool AHorrorThreatCharacter::DeactivateThreat()
{
	if (!bThreatActive)
	{
		return false;
	}

	bThreatActive = false;
	ClearDetectedTarget();
	ApplyThreatAnimation(IdleAnimation);
	OnThreatActiveChanged.Broadcast(bThreatActive);
	return true;
}

bool AHorrorThreatCharacter::IsThreatActive() const
{
	return bThreatActive;
}

bool AHorrorThreatCharacter::CanDetectActor(const AActor* TargetActor) const
{
	if (!bThreatActive || !IsValid(TargetActor) || TargetActor == this || DetectionRadius <= 0.0f)
	{
		return false;
	}

	return FVector::DistSquared(GetActorLocation(), TargetActor->GetActorLocation()) <= FMath::Square(DetectionRadius);
}

bool AHorrorThreatCharacter::UpdateDetectedTarget(AActor* TargetActor)
{
	if (!CanDetectActor(TargetActor))
	{
		return ClearDetectedTarget();
	}

	if (DetectedTarget.Get() == TargetActor)
	{
		return false;
	}

	DetectedTarget = TargetActor;
	OnDetectedTargetChanged.Broadcast(TargetActor);
	return true;
}

AActor* AHorrorThreatCharacter::GetDetectedTarget() const
{
	return DetectedTarget.Get();
}

bool AHorrorThreatCharacter::ClearDetectedTarget()
{
	if (!DetectedTarget.IsValid())
	{
		DetectedTarget.Reset();
		return false;
	}

	DetectedTarget.Reset();
	OnDetectedTargetChanged.Broadcast(nullptr);
	return true;
}

UHorrorGolemBehaviorComponent* AHorrorThreatCharacter::GetGolemBehavior() const
{
	return GolemBehavior ? GolemBehavior.Get() : FindComponentByClass<UHorrorGolemBehaviorComponent>();
}

void AHorrorThreatCharacter::ApplyThreatAnimation(UAnimationAsset* AnimationAsset)
{
	if (!GetMesh() || !AnimationAsset)
	{
		return;
	}

	GetMesh()->OverrideAnimationData(AnimationAsset, true, true);
}
