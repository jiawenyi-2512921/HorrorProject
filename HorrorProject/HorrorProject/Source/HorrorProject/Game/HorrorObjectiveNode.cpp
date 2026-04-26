// Copyright Epic Games, Inc. All Rights Reserved.

#include "Game/HorrorObjectiveNode.h"
#include "Components/BoxComponent.h"
#include "Engine/World.h"
#include "Game/HorrorEventBusSubsystem.h"

AHorrorObjectiveNode::AHorrorObjectiveNode()
{
	PrimaryActorTick.bCanEverTick = false;

	TriggerVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerVolume"));
	RootComponent = TriggerVolume;
	TriggerVolume->SetBoxExtent(FVector(100.0f, 100.0f, 100.0f));
	TriggerVolume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TriggerVolume->SetCollisionResponseToAllChannels(ECR_Ignore);
	TriggerVolume->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
}

void AHorrorObjectiveNode::BeginPlay()
{
	Super::BeginPlay();

	if (bAutoActivateOnBeginPlay)
	{
		ActivateNode();
	}
}

bool AHorrorObjectiveNode::ActivateNode()
{
	if (!CanActivate())
	{
		return false;
	}

	NodeState = EHorrorObjectiveNodeState::Active;

	if (bPublishEventOnActivate)
	{
		if (UHorrorEventBusSubsystem* EventBus = GetEventBus())
		{
			EventBus->Publish(EventTag, NodeId, StateTag, this);
		}
	}

	OnNodeActivated();
	return true;
}

bool AHorrorObjectiveNode::CompleteNode(AActor* InstigatorActor)
{
	if (!CanComplete(InstigatorActor))
	{
		return false;
	}

	NodeState = EHorrorObjectiveNodeState::Completed;

	if (bPublishEventOnComplete)
	{
		if (UHorrorEventBusSubsystem* EventBus = GetEventBus())
		{
			EventBus->Publish(EventTag, NodeId, StateTag, this);
		}
	}

	OnNodeCompleted(InstigatorActor);
	return true;
}

bool AHorrorObjectiveNode::FailNode()
{
	if (NodeState == EHorrorObjectiveNodeState::Completed || NodeState == EHorrorObjectiveNodeState::Failed)
	{
		return false;
	}

	NodeState = EHorrorObjectiveNodeState::Failed;
	OnNodeFailed();
	return true;
}

bool AHorrorObjectiveNode::CanActivate() const
{
	return NodeState == EHorrorObjectiveNodeState::Inactive;
}

bool AHorrorObjectiveNode::CanComplete(AActor* InstigatorActor) const
{
	return NodeState == EHorrorObjectiveNodeState::Active && InstigatorActor != nullptr;
}

void AHorrorObjectiveNode::OnNodeActivated_Implementation()
{
}

void AHorrorObjectiveNode::OnNodeCompleted_Implementation(AActor* InstigatorActor)
{
}

void AHorrorObjectiveNode::OnNodeFailed_Implementation()
{
}

UHorrorEventBusSubsystem* AHorrorObjectiveNode::GetEventBus() const
{
	if (UWorld* World = GetWorld())
	{
		return World->GetSubsystem<UHorrorEventBusSubsystem>();
	}
	return nullptr;
}
