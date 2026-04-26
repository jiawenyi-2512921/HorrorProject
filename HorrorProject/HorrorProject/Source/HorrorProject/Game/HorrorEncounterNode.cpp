// Copyright Epic Games, Inc. All Rights Reserved.

#include "Game/HorrorEncounterNode.h"
#include "AI/HorrorThreatCharacter.h"
#include "Components/BoxComponent.h"
#include "Engine/World.h"
#include "Game/HorrorEncounterDirector.h"
#include "GameFramework/Character.h"

AHorrorEncounterNode::AHorrorEncounterNode()
{
	NodeType = EHorrorObjectiveNodeType::Encounter;
}

void AHorrorEncounterNode::BeginPlay()
{
	Super::BeginPlay();

	if (TriggerVolume)
	{
		TriggerVolume->OnComponentBeginOverlap.AddDynamic(this, &AHorrorEncounterNode::OnTriggerBeginOverlap);
	}
}

bool AHorrorEncounterNode::TriggerEncounter(AActor* InstigatorActor)
{
	if (!IsActive() || bEncounterActive || !InstigatorActor)
	{
		return false;
	}

	bEncounterActive = true;

	if (EncounterDirectorClass)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnedEncounterDirector = GetWorld()->SpawnActor<AHorrorEncounterDirector>(EncounterDirectorClass, GetActorTransform(), SpawnParams);
	}

	if (ThreatClass)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		FTransform WorldThreatTransform = ThreatSpawnTransform * GetActorTransform();
		SpawnedThreat = GetWorld()->SpawnActor<AHorrorThreatCharacter>(ThreatClass, WorldThreatTransform, SpawnParams);
	}

	OnEncounterTriggered(InstigatorActor);
	return true;
}

bool AHorrorEncounterNode::ResolveEncounter()
{
	if (!bEncounterActive)
	{
		return false;
	}

	bEncounterActive = false;

	if (SpawnedThreat)
	{
		SpawnedThreat->Destroy();
		SpawnedThreat = nullptr;
	}

	OnEncounterResolved();

	if (bAutoCompleteOnResolve)
	{
		CompleteNode(nullptr);
	}

	return true;
}

void AHorrorEncounterNode::OnNodeActivated_Implementation()
{
	Super::OnNodeActivated_Implementation();

	if (TriggerVolume)
	{
		TriggerVolume->SetBoxExtent(FVector(TriggerRadius, TriggerRadius, 200.0f));
	}

	if (bAutoTriggerOnActivate)
	{
		TriggerEncounter(nullptr);
	}
}

void AHorrorEncounterNode::OnEncounterTriggered_Implementation(AActor* InstigatorActor)
{
}

void AHorrorEncounterNode::OnEncounterResolved_Implementation()
{
}

void AHorrorEncounterNode::OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!IsActive() || bEncounterActive || !OtherActor || !OtherActor->IsA<ACharacter>())
	{
		return;
	}

	TriggerEncounter(OtherActor);
}
