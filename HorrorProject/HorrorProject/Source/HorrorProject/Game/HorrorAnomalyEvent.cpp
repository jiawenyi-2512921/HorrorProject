// Copyright HorrorProject. All Rights Reserved.

#include "Game/HorrorAnomalyEvent.h"
#include "Game/HorrorEventBusSubsystem.h"
#include "Components/SceneComponent.h"
#include "Components/BoxComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundBase.h"
#include "Camera/CameraShakeBase.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "GameplayTagContainer.h"
#include "HorrorProject.h"

AHorrorAnomalyEvent::AHorrorAnomalyEvent()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	RootComponent = SceneRoot;

	TriggerZone = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerZone"));
	TriggerZone->SetupAttachment(SceneRoot);
	TriggerZone->SetCollisionProfileName(TEXT("Trigger"));
	TriggerZone->SetBoxExtent(FVector(200.0f, 200.0f, 200.0f));

	AnomalyParticles = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("AnomalyParticles"));
	AnomalyParticles->SetupAttachment(SceneRoot);
	AnomalyParticles->bAutoActivate = false;
}

void AHorrorAnomalyEvent::BeginPlay()
{
	Super::BeginPlay();

	TriggerZone->OnComponentBeginOverlap.AddDynamic(this, &AHorrorAnomalyEvent::OnTriggerOverlapBegin);

	if (bAutoTriggerOnPlayerEnter)
	{
		AnomalyState = EAnomalyState::Arming;
	}
}

void AHorrorAnomalyEvent::ArmAnomaly()
{
	if (AnomalyState != EAnomalyState::Dormant)
	{
		return;
	}

	TransitionState(EAnomalyState::Arming);
}

void AHorrorAnomalyEvent::TriggerAnomaly(AActor* InstigatorActor)
{
	if (AnomalyState != EAnomalyState::Dormant && AnomalyState != EAnomalyState::Arming)
	{
		return;
	}

	if (bSingleUse && bHasTriggered)
	{
		return;
	}

	TransitionState(EAnomalyState::Active);
	bHasTriggered = true;
	CurrentIntensity = MaxIntensity;

	if (AnomalyParticles)
	{
		AnomalyParticles->Activate(true);
	}

	PlayAnomalySound(AnomalySound);
	TriggerCameraShake();
	ApplyPlayerFeedback(InstigatorActor);
	PublishAnomalyEvent(TEXT("Anomaly.Triggered"));

	if (ActiveDuration > 0.0f)
	{
		GetWorldTimerManager().SetTimer(AnomalyTimerHandle, [this]()
		{
			if (AnomalyState == EAnomalyState::Active)
			{
				EscalateAnomaly();
			}
		}, ActiveDuration, false);
	}

	BP_OnAnomalyTriggered(InstigatorActor);
}

void AHorrorAnomalyEvent::EscalateAnomaly()
{
	if (AnomalyState != EAnomalyState::Active)
	{
		return;
	}

	TransitionState(EAnomalyState::Escalating);
	CurrentIntensity = FMath::Min(1.0f, CurrentIntensity * 1.5f);

	PlayAnomalySound(EscalationSound);
	PublishAnomalyEvent(TEXT("Anomaly.Escalated"));

	BP_OnAnomalyEscalated();

	if (EscalationDuration > 0.0f)
	{
		GetWorldTimerManager().SetTimer(AnomalyTimerHandle, [this]()
		{
			ResolveAnomaly();
		}, EscalationDuration, false);
	}
}

void AHorrorAnomalyEvent::ResolveAnomaly()
{
	if (AnomalyState != EAnomalyState::Active && AnomalyState != EAnomalyState::Escalating)
	{
		return;
	}

	GetWorldTimerManager().ClearTimer(AnomalyTimerHandle);

	TransitionState(EAnomalyState::Resolving);
	CurrentIntensity = 0.0f;

	if (AnomalyParticles)
	{
		AnomalyParticles->Deactivate();
	}

	PublishAnomalyEvent(TEXT("Anomaly.Resolved"));
	BP_OnAnomalyResolved();

	TransitionState(EAnomalyState::Resolved);
}

void AHorrorAnomalyEvent::ResetAnomaly()
{
	GetWorldTimerManager().ClearTimer(AnomalyTimerHandle);

	TransitionState(EAnomalyState::Dormant);
	CurrentIntensity = 0.0f;
	bHasTriggered = false;

	if (AnomalyParticles)
	{
		AnomalyParticles->Deactivate();
	}

	BP_OnAnomalyReset();
}

bool AHorrorAnomalyEvent::IsPlayerInZone() const
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	APlayerController* PC = World->GetFirstPlayerController();
	if (!PC || !PC->GetPawn())
	{
		return false;
	}

	return TriggerZone->IsOverlappingActor(PC->GetPawn());
}

void AHorrorAnomalyEvent::OnTriggerOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!bAutoTriggerOnPlayerEnter)
	{
		return;
	}

	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (!PC || OtherActor != PC->GetPawn())
	{
		return;
	}

	OnPlayerEnteredAnomalyZone.Broadcast(OtherActor);

	if (bRequiresRecording)
	{
		return;
	}

	TriggerAnomaly(OtherActor);
}

void AHorrorAnomalyEvent::UpdateAnomalyEffects(float DeltaTime)
{
	if (AnomalyState == EAnomalyState::Active || AnomalyState == EAnomalyState::Escalating)
	{
		CurrentIntensity = FMath::FInterpTo(CurrentIntensity, MaxIntensity, DeltaTime, 2.0f);
	}
}

void AHorrorAnomalyEvent::ApplyPlayerFeedback(AActor* PlayerActor)
{
	if (!PlayerActor)
	{
		return;
	}

	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (!PC)
	{
		return;
	}

	// Fear and sanity are handled by event bus subscribers
}

void AHorrorAnomalyEvent::PlayAnomalySound(USoundBase* Sound)
{
	if (!Sound)
	{
		return;
	}

	UGameplayStatics::PlaySoundAtLocation(this, Sound, GetActorLocation());
}

void AHorrorAnomalyEvent::TriggerCameraShake()
{
	if (!AnomalyCameraShake)
	{
		return;
	}

	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (PC)
	{
		PC->ClientStartCameraShake(AnomalyCameraShake, CameraShakeScale);
	}
}

void AHorrorAnomalyEvent::PublishAnomalyEvent(FName EventName)
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

	FGameplayTag EventTag;
	if (!AnomalyEventTag.IsValid())
	{
		EventTag = FGameplayTag::RequestGameplayTag(EventName, false);
	}
	else
	{
		EventTag = AnomalyEventTag;
	}

	if (EventTag.IsValid())
	{
		EventBus->Publish(EventTag, EventBusSourceId, FGameplayTag::EmptyTag, this);
	}
}

void AHorrorAnomalyEvent::TransitionState(EAnomalyState NewState)
{
	if (AnomalyState == NewState)
	{
		return;
	}

	const EAnomalyState OldState = AnomalyState;
	AnomalyState = NewState;
	OnAnomalyStateChanged.Broadcast(NewState, AnomalyType);
}
