// Copyright Epic Games, Inc. All Rights Reserved.

#include "EvidenceActor.h"
#include "Evidence/EvidenceCollectionComponent.h"
#include "Components/StaticMeshComponent.h"

AEvidenceActor::AEvidenceActor()
{
	PrimaryActorTick.bCanEverTick = false;

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	RootComponent = MeshComponent;
}

void AEvidenceActor::BeginPlay()
{
	Super::BeginPlay();

	if (bAutoRegisterMetadata && !EvidenceMetadata.EvidenceId.IsNone())
	{
		// Metadata will be registered when collected
	}
}

bool AEvidenceActor::Interact_Implementation(AActor* InstigatorActor, const FHitResult& Hit)
{
	if (!InstigatorActor || bIsCollected)
	{
		return false;
	}

	UEvidenceCollectionComponent* EvidenceCollection = InstigatorActor->FindComponentByClass<UEvidenceCollectionComponent>();
	if (!EvidenceCollection)
	{
		return false;
	}

	if (EvidenceCollection->CollectPhysicalEvidence(EvidenceMetadata.EvidenceId, EvidenceMetadata))
	{
		SetCollected(true);
		OnEvidenceCollected(InstigatorActor);
		BP_OnEvidenceCollected(InstigatorActor);

		if (bDestroyOnCollect)
		{
			Destroy();
		}
		else if (bHideOnCollect)
		{
			SetActorHiddenInGame(true);
			SetActorEnableCollision(false);
		}

		return true;
	}

	return false;
}

FText AEvidenceActor::GetInteractionPrompt() const
{
	if (!InteractionPromptOverride.IsEmpty())
	{
		return InteractionPromptOverride;
	}

	if (!EvidenceMetadata.DisplayName.IsEmpty())
	{
		return FText::Format(NSLOCTEXT("EvidenceActor", "CollectFormat", "收集 {0}"), EvidenceMetadata.DisplayName);
	}

	return NSLOCTEXT("EvidenceActor", "CollectEvidence", "收集证据");
}

bool AEvidenceActor::CanInteract_Implementation(AActor* InstigatorActor, const FHitResult& Hit) const
{
	if (bIsCollected)
	{
		return false;
	}

	if (!InstigatorActor)
	{
		return false;
	}

	UEvidenceCollectionComponent* EvidenceCollection = InstigatorActor->FindComponentByClass<UEvidenceCollectionComponent>();
	return EvidenceCollection != nullptr;
}

void AEvidenceActor::SetEvidenceMetadata(const FExtendedEvidenceMetadata& Metadata)
{
	EvidenceMetadata = Metadata;
}

void AEvidenceActor::SetCollected(bool bCollected)
{
	bIsCollected = bCollected;
}

void AEvidenceActor::OnEvidenceCollected_Implementation(AActor* Collector)
{
	// Default implementation - can be overridden in Blueprint or C++
}
