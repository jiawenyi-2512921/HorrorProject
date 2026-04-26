// Copyright Epic Games, Inc. All Rights Reserved.

#include "Game/HorrorEncounterDirector.h"

#include "AI/HorrorThreatCharacter.h"

#include "Components/SceneComponent.h"
#include "Engine/World.h"

#if WITH_DEV_AUTOMATION_TESTS
void UHorrorEncounterPhaseDelegateProbe::HandleEncounterPhaseChanged(EHorrorEncounterPhase NewPhase, FName EncounterId)
{
	PhaseValues.Add(NewPhase);
	EncounterIds.Add(EncounterId);
}
#endif

AHorrorEncounterDirector::AHorrorEncounterDirector()
{
	PrimaryActorTick.bCanEverTick = false;
	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	RootComponent = SceneRoot;
	ThreatClass = AHorrorThreatCharacter::StaticClass();
}

bool AHorrorEncounterDirector::PrimeEncounter(FName InEncounterId)
{
	if (EncounterPhase != EHorrorEncounterPhase::Dormant)
	{
		return false;
	}

	ActiveEncounterId = InEncounterId.IsNone() ? DefaultEncounterId : InEncounterId;
	EncounterPhase = EHorrorEncounterPhase::Primed;
	OnEncounterPhaseChanged.Broadcast(EncounterPhase, ActiveEncounterId);
	BP_OnEncounterPrimed(ActiveEncounterId);
	return true;
}

bool AHorrorEncounterDirector::TriggerReveal(AActor* PlayerActor)
{
	if (!CanTriggerReveal(PlayerActor))
	{
		return false;
	}

	AHorrorThreatCharacter* RevealThreat = SpawnThreatActor();
	EncounterPhase = EHorrorEncounterPhase::Revealed;
	LastRevealTarget = PlayerActor;
	OnEncounterPhaseChanged.Broadcast(EncounterPhase, ActiveEncounterId);
	if (RevealThreat)
	{
		RevealThreat->ActivateThreat();
		RevealThreat->UpdateDetectedTarget(PlayerActor);
	}
	BP_OnEncounterRevealed(ActiveEncounterId, PlayerActor, RevealThreat);
	return true;
}

bool AHorrorEncounterDirector::ResolveEncounter()
{
	if (EncounterPhase != EHorrorEncounterPhase::Revealed)
	{
		return false;
	}

	EncounterPhase = EHorrorEncounterPhase::Resolved;
	if (ThreatActor)
	{
		ThreatActor->DeactivateThreat();
	}
	OnEncounterPhaseChanged.Broadcast(EncounterPhase, ActiveEncounterId);
	BP_OnEncounterResolved(ActiveEncounterId);
	return true;
}

bool AHorrorEncounterDirector::ResetEncounter()
{
	if (EncounterPhase == EHorrorEncounterPhase::Dormant && ActiveEncounterId.IsNone() && !LastRevealTarget)
	{
		return false;
	}

	const FName ResetEncounterId = ActiveEncounterId;
	EncounterPhase = EHorrorEncounterPhase::Dormant;
	ActiveEncounterId = NAME_None;
	LastRevealTarget = nullptr;
	if (ThreatActor)
	{
		ThreatActor->DeactivateThreat();
	}
	OnEncounterPhaseChanged.Broadcast(EncounterPhase, ResetEncounterId);
	BP_OnEncounterReset(ResetEncounterId);
	return true;
}

bool AHorrorEncounterDirector::CanTriggerReveal(const AActor* PlayerActor) const
{
	if (EncounterPhase != EHorrorEncounterPhase::Primed || !PlayerActor)
	{
		return false;
	}

	if (RevealRadius <= 0.0f)
	{
		return true;
	}

	return FVector::DistSquared(GetActorLocation(), PlayerActor->GetActorLocation()) <= FMath::Square(RevealRadius);
}

bool AHorrorEncounterDirector::IsRouteGated() const
{
	return bGateRouteDuringReveal && EncounterPhase == EHorrorEncounterPhase::Revealed;
}

EHorrorEncounterPhase AHorrorEncounterDirector::GetEncounterPhase() const
{
	return EncounterPhase;
}

FName AHorrorEncounterDirector::GetEncounterId() const
{
	return ActiveEncounterId;
}

AActor* AHorrorEncounterDirector::GetLastRevealTarget() const
{
	return LastRevealTarget.Get();
}

AHorrorThreatCharacter* AHorrorEncounterDirector::SpawnThreatActor()
{
	if (ThreatActor)
	{
		return ThreatActor.Get();
	}

	if (!ThreatClass)
	{
		return nullptr;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = this;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ThreatActor = World->SpawnActor<AHorrorThreatCharacter>(
		ThreatClass,
		ThreatRelativeTransform * GetActorTransform(),
		SpawnParameters);

	return ThreatActor.Get();
}

AHorrorThreatCharacter* AHorrorEncounterDirector::GetThreatActor() const
{
	return ThreatActor.Get();
}
