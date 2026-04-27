// Copyright Epic Games, Inc. All Rights Reserved.

#include "Game/HorrorNavigationNode.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Character.h"

namespace
{
	constexpr float NavigationTriggerVolumeHalfHeight = 200.0f;
}

AHorrorNavigationNode::AHorrorNavigationNode()
{
	NodeType = EHorrorObjectiveNodeType::Navigation;
}

void AHorrorNavigationNode::BeginPlay()
{
	Super::BeginPlay();

	if (TriggerVolume && bAutoCompleteOnOverlap)
	{
		TriggerVolume->OnComponentBeginOverlap.AddDynamic(this, &AHorrorNavigationNode::OnTriggerBeginOverlap);
	}
}

void AHorrorNavigationNode::OnNodeActivated_Implementation()
{
	Super::OnNodeActivated_Implementation();

	if (TriggerVolume)
	{
		TriggerVolume->SetBoxExtent(FVector(ActivationRadius, ActivationRadius, NavigationTriggerVolumeHalfHeight));
	}
}

void AHorrorNavigationNode::OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!IsActive() || !OtherActor || !OtherActor->IsA<ACharacter>())
	{
		return;
	}

	CompleteNode(OtherActor);
}
