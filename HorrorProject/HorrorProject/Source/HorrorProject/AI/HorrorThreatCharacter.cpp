// Copyright Epic Games, Inc. All Rights Reserved.

#include "AI/HorrorThreatCharacter.h"

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
}

bool AHorrorThreatCharacter::ActivateThreat()
{
	if (bThreatActive)
	{
		return false;
	}

	bThreatActive = true;
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
