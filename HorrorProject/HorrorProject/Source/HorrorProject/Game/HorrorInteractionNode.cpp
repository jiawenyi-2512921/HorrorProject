// Copyright Epic Games, Inc. All Rights Reserved.

#include "Game/HorrorInteractionNode.h"

AHorrorInteractionNode::AHorrorInteractionNode()
{
	NodeType = EHorrorObjectiveNodeType::Interaction;
}

bool AHorrorInteractionNode::CanInteract_Implementation(AActor* InstigatorActor, const FHitResult& Hit) const
{
	return IsActive() && InstigatorActor != nullptr;
}

bool AHorrorInteractionNode::Interact_Implementation(AActor* InstigatorActor, const FHitResult& Hit)
{
	if (!CanInteract_Implementation(InstigatorActor, Hit))
	{
		return false;
	}

	if (OnInteract(InstigatorActor))
	{
		CompleteNode(InstigatorActor);
		return true;
	}

	return false;
}

bool AHorrorInteractionNode::OnInteract_Implementation(AActor* InstigatorActor)
{
	return true;
}
