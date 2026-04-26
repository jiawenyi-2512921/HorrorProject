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

void AEvidenceActor::Interact_Implementation(AActor* Interactor)
{
	if (!Interactor || bIsCollected)
	{
		return;
	}

	UEvidenceCollectionComponent* EvidenceCollection = Interactor->FindComponentByClass<UEvidenceCollectionComponent>();
	if (!EvidenceCollection)
	{
		return;
	}

	if (EvidenceCollection->CollectPhysicalEvidence(EvidenceMetadata.EvidenceId, EvidenceMetadata))
	{
		SetCollected(true);
		OnEvidenceCollected(Interactor);
		BP_OnEvidenceCollected(Interactor);

		if (bDestroyOnCollect)
		{
			Destroy();
		}
		else if (bHideOnCollect)
		{
			SetActorHiddenInGame(true);
			SetActorEnableCollision(false);
		}
	}
}

FText AEvidenceActor::GetInteractionPrompt_Implementation() const
{
	if (!InteractionPromptOverride.IsEmpty())
	{
		return InteractionPromptOverride;
	}

	if (!EvidenceMetadata.DisplayName.IsEmpty())
	{
		return FText::Format(FText::FromString(TEXT("Collect {0}")), EvidenceMetadata.DisplayName);
	}

	return FText::FromString(TEXT("Collect Evidence"));
}

bool AEvidenceActor::CanInteract_Implementation(AActor* Interactor) const
{
	if (bIsCollected)
	{
		return false;
	}

	if (!Interactor)
	{
		return false;
	}

	UEvidenceCollectionComponent* EvidenceCollection = Interactor->FindComponentByClass<UEvidenceCollectionComponent>();
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
