// Copyright Epic Games, Inc. All Rights Reserved.

#include "Game/HorrorEncounterDirector.h"

#include "AI/HorrorThreatCharacter.h"
#include "AI/HorrorGolemBehaviorComponent.h"
#include "Game/HorrorEventBusSubsystem.h"
#include "Audio/HorrorAudioSubsystem.h"

#include "Components/SceneComponent.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/CameraShakeBase.h"
#include "Sound/SoundBase.h"
#include "TimerManager.h"
#include "GameplayTagContainer.h"
#include "HorrorProject.h"

void UHorrorEncounterPhaseDelegateProbe::HandleEncounterPhaseChanged(EHorrorEncounterPhase NewPhase, FName EncounterId)
{
	PhaseValues.Add(NewPhase);
	EncounterIds.Add(EncounterId);
}

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

	if (PrimeSound)
	{
		PlayEncounterSound(PrimeSound);
	}

	PublishEncounterEvent(TEXT("Encounter.Primed"), ActiveEncounterId);
	BP_OnEncounterPrimed(ActiveEncounterId);
	return true;
}

bool AHorrorEncounterDirector::TriggerReveal(AActor* PlayerActor)
{
	if (!CanTriggerReveal(PlayerActor))
	{
		return false;
	}

	if (RevealDelaySeconds > 0.0f)
	{
		BP_OnRevealSequenceStart(PlayerActor);
		GetWorldTimerManager().SetTimer(
			RevealDelayTimerHandle,
			FTimerDelegate::CreateUObject(this, &AHorrorEncounterDirector::ExecuteDelayedReveal, PlayerActor),
			RevealDelaySeconds,
			false);
		return true;
	}

	ExecuteDelayedReveal(PlayerActor);
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

	if (ResolveSound)
	{
		PlayEncounterSound(ResolveSound);
	}

	OnEncounterPhaseChanged.Broadcast(EncounterPhase, ActiveEncounterId);
	PublishEncounterEvent(TEXT("Encounter.Resolved"), ActiveEncounterId);
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

void AHorrorEncounterDirector::ExecuteDelayedReveal(AActor* PlayerActor)
{
	if (EncounterPhase != EHorrorEncounterPhase::Primed)
	{
		return;
	}

	AHorrorThreatCharacter* RevealThreat = SpawnThreatActor();
	EncounterPhase = EHorrorEncounterPhase::Revealed;
	LastRevealTarget = PlayerActor;

	if (RevealSound)
	{
		PlayEncounterSound(RevealSound);
	}

	if (RevealCameraShake)
	{
		TriggerCameraShake(RevealCameraShake, RevealCameraShakeScale);
	}

	OnEncounterPhaseChanged.Broadcast(EncounterPhase, ActiveEncounterId);
	PublishEncounterEvent(TEXT("Encounter.Revealed"), ActiveEncounterId);

	if (RevealThreat)
	{
		RevealThreat->ActivateThreat();
		RevealThreat->UpdateDetectedTarget(PlayerActor);
	}

	BP_OnRevealSequenceComplete(PlayerActor, RevealThreat);
	BP_OnEncounterRevealed(ActiveEncounterId, PlayerActor, RevealThreat);
}

void AHorrorEncounterDirector::PublishEncounterEvent(FName EventName, FName PhaseTag)
{
	if (!bPublishToEventBus)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	UHorrorEventBusSubsystem* EventBus = World->GetSubsystem<UHorrorEventBusSubsystem>();
	if (!EventBus)
	{
		return;
	}

	const FGameplayTag EventTag = FGameplayTag::RequestGameplayTag(EventName, false);
	const FGameplayTag StateTag = FGameplayTag::RequestGameplayTag(PhaseTag, false);
	if (!EventTag.IsValid() || !StateTag.IsValid())
	{
		UE_LOG(LogHorrorProject, Warning, TEXT("Skipping encounter event publish because tag registration is missing. Event=%s Phase=%s"),
			*EventName.ToString(),
			*PhaseTag.ToString());
		return;
	}

	EventBus->Publish(EventTag, EventBusSourceId, StateTag, this);
}

void AHorrorEncounterDirector::PlayEncounterSound(USoundBase* Sound, float VolumeMultiplier)
{
	if (!Sound)
	{
		return;
	}

	if (bUseAudioSubsystem)
	{
		if (UWorld* World = GetWorld())
		{
			if (UHorrorAudioSubsystem* AudioSys = World->GetSubsystem<UHorrorAudioSubsystem>())
			{
				AudioSys->PlaySoundAtLocation(Sound, GetActorLocation(), VolumeMultiplier);
				return;
			}
		}
	}

	UGameplayStatics::PlaySoundAtLocation(this, Sound, GetActorLocation(), VolumeMultiplier);
}

void AHorrorEncounterDirector::TriggerCameraShake(TSubclassOf<UCameraShakeBase> ShakeClass, float Scale)
{
	if (!ShakeClass)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	APlayerController* PC = World->GetFirstPlayerController();
	if (!PC)
	{
		return;
	}

	PC->ClientStartCameraShake(ShakeClass, Scale);
}

void AHorrorEncounterDirector::BP_OnRevealSequenceStart_Implementation(AActor* PlayerActor)
{
}

void AHorrorEncounterDirector::BP_OnRevealSequenceComplete_Implementation(AActor* PlayerActor, AHorrorThreatCharacter* RevealedThreat)
{
}

bool AHorrorEncounterDirector::ActivateGolemBehavior(AActor* TargetActor)
{
	if (!ThreatActor || !IsValid(TargetActor))
	{
		return false;
	}

	UHorrorGolemBehaviorComponent* GolemBehavior = ThreatActor->GetGolemBehavior();
	if (!GolemBehavior)
	{
		return false;
	}

	GolemBehavior->ActivateBehavior(TargetActor);
	return true;
}

bool AHorrorEncounterDirector::DeactivateGolemBehavior()
{
	if (!ThreatActor)
	{
		return false;
	}

	UHorrorGolemBehaviorComponent* GolemBehavior = ThreatActor->GetGolemBehavior();
	if (!GolemBehavior)
	{
		return false;
	}

	GolemBehavior->DeactivateBehavior();
	return true;
}
