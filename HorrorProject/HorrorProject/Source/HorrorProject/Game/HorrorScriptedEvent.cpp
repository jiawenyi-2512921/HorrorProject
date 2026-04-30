// Copyright HorrorProject. All Rights Reserved.

#include "Game/HorrorScriptedEvent.h"
#include "Game/HorrorEncounterDirector.h"
#include "Game/HorrorAnomalyEvent.h"
#include "Game/HorrorEventBusSubsystem.h"
#include "Components/SceneComponent.h"
#include "Components/BoxComponent.h"
#include "Sound/SoundBase.h"
#include "Camera/CameraShakeBase.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "GameplayTagContainer.h"
#include "HorrorProject.h"

AHorrorScriptedEvent::AHorrorScriptedEvent()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	RootComponent = SceneRoot;

	TriggerZone = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerZone"));
	TriggerZone->SetupAttachment(SceneRoot);
	TriggerZone->SetCollisionProfileName(TEXT("Trigger"));
	TriggerZone->SetBoxExtent(FVector(300.0f, 300.0f, 200.0f));
}

void AHorrorScriptedEvent::BeginPlay()
{
	Super::BeginPlay();

	TriggerZone->OnComponentBeginOverlap.AddDynamic(this, &AHorrorScriptedEvent::OnTriggerOverlapBegin);
}

void AHorrorScriptedEvent::PrimeEvent()
{
	if (EventState != EScriptedEventState::Idle)
	{
		return;
	}

	TransitionState(EScriptedEventState::Primed);
}

void AHorrorScriptedEvent::ExecuteEvent(AActor* InstigatorActor)
{
	if (bSingleUse && bHasTriggered)
	{
		return;
	}

	if (bRequiresPriming && EventState != EScriptedEventState::Primed)
	{
		return;
	}

	if (EventState == EScriptedEventState::Executing)
	{
		return;
	}

	const UWorld* World = GetWorld();
	if (World && ReTriggerCooldown > 0.0f && (World->GetTimeSeconds() - LastTriggerTime) < ReTriggerCooldown)
	{
		return;
	}

	TransitionState(EScriptedEventState::Executing);
	bHasTriggered = true;
	CurrentStepIndex = 0;

	if (World)
	{
		LastTriggerTime = World->GetTimeSeconds();
	}

	// Link to encounter/anomaly if configured
	if (LinkedEncounter)
	{
		LinkedEncounter->PrimeEncounter(LinkedEncounter->GetEncounterId());
	}

	if (LinkedAnomaly)
	{
		LinkedAnomaly->ArmAnomaly();
	}

	BP_OnEventExecuted(InstigatorActor);

	if (EventSteps.Num() > 0)
	{
		ExecuteStep(0);
	}
	else
	{
		TransitionState(EScriptedEventState::Completed);
		BP_OnEventCompleted();
	}
}

void AHorrorScriptedEvent::CancelEvent()
{
	if (EventState != EScriptedEventState::Executing)
	{
		return;
	}

	GetWorldTimerManager().ClearTimer(StepTimerHandle);
	GetWorldTimerManager().ClearTimer(DelayTimerHandle);
	CurrentStepIndex = -1;

	TransitionState(EScriptedEventState::Idle);
}

void AHorrorScriptedEvent::ResetEvent()
{
	GetWorldTimerManager().ClearTimer(StepTimerHandle);
	GetWorldTimerManager().ClearTimer(DelayTimerHandle);

	CurrentStepIndex = -1;
	bHasTriggered = false;
	TransitionState(EScriptedEventState::Idle);
}

void AHorrorScriptedEvent::OnTriggerOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
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

	ExecuteEvent(OtherActor);
}

void AHorrorScriptedEvent::ExecuteStep(int32 StepIndex)
{
	if (StepIndex < 0 || StepIndex >= EventSteps.Num())
	{
		TransitionState(EScriptedEventState::Completed);
		BP_OnEventCompleted();

		// Trigger linked encounter reveal if configured
		if (LinkedEncounter && LinkedEncounter->GetEncounterPhase() == EHorrorEncounterPhase::Primed)
		{
			APlayerController* PC = GetWorld()->GetFirstPlayerController();
			if (PC && PC->GetPawn())
			{
				LinkedEncounter->TriggerReveal(PC->GetPawn());
			}
		}

		if (LinkedAnomaly && LinkedAnomaly->GetAnomalyState() == EAnomalyState::Arming)
		{
			LinkedAnomaly->TriggerAnomaly();
		}

		return;
	}

	CurrentStepIndex = StepIndex;
	const FScriptedEventStep& Step = EventSteps[StepIndex];

	OnStepStarted.Broadcast(StepIndex, Step);
	BP_OnStepExecuted(StepIndex, Step);

	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	AActor* PlayerActor = PC ? PC->GetPawn() : nullptr;

	ApplyStepEffects(Step, PlayerActor);

	if (Step.StepDuration > 0.0f)
	{
		GetWorldTimerManager().SetTimer(StepTimerHandle, [this, StepIndex]()
		{
			const int32 NextStep = StepIndex + 1;
			if (NextStep < EventSteps.Num())
			{
				const float NextDelay = EventSteps[NextStep].DelayBeforeStep;
				if (NextDelay > 0.0f)
				{
					GetWorldTimerManager().SetTimer(DelayTimerHandle, [this, NextStep]()
					{
						ExecuteStep(NextStep);
					}, NextDelay, false);
				}
				else
				{
					ExecuteStep(NextStep);
				}
			}
			else
			{
				TransitionState(EScriptedEventState::Completed);
				BP_OnEventCompleted();
			}
		}, Step.StepDuration, false);
	}
	else
	{
		// No duration, immediately go to next step
		const int32 NextStep = StepIndex + 1;
		if (NextStep < EventSteps.Num())
		{
			ExecuteStep(NextStep);
		}
		else
		{
			TransitionState(EScriptedEventState::Completed);
			BP_OnEventCompleted();
		}
	}
}

void AHorrorScriptedEvent::ApplyStepEffects(const FScriptedEventStep& Step, AActor* PlayerActor)
{
	if (Step.Sound)
	{
		PlayStepSound(Step.Sound);
	}

	if (Step.CameraShake)
	{
		TriggerStepCameraShake(Step.CameraShake, Step.CameraShakeScale);
	}

	if (Step.EventToPublish.IsValid())
	{
		PublishStepEvent(Step.EventToPublish);
	}
}

void AHorrorScriptedEvent::PlayStepSound(USoundBase* Sound)
{
	if (!Sound)
	{
		return;
	}

	UGameplayStatics::PlaySoundAtLocation(this, Sound, GetActorLocation());
}

void AHorrorScriptedEvent::TriggerStepCameraShake(TSubclassOf<UCameraShakeBase> ShakeClass, float Scale)
{
	if (!ShakeClass)
	{
		return;
	}

	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (PC)
	{
		PC->ClientStartCameraShake(ShakeClass, Scale);
	}
}

void AHorrorScriptedEvent::PublishStepEvent(FGameplayTag EventTag)
{
	if (!bPublishToEventBus || !EventTag.IsValid())
	{
		return;
	}

	UHorrorEventBusSubsystem* EventBus = GetWorld()->GetSubsystem<UHorrorEventBusSubsystem>();
	if (EventBus)
	{
		EventBus->Publish(EventTag, EventBusSourceId, FGameplayTag::EmptyTag, this);
	}
}

void AHorrorScriptedEvent::TransitionState(EScriptedEventState NewState)
{
	if (EventState == NewState)
	{
		return;
	}

	EventState = NewState;
	OnEventStateChanged.Broadcast(NewState);
}
