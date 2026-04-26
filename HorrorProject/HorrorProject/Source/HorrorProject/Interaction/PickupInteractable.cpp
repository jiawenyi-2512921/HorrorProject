// Copyright Epic Games, Inc. All Rights Reserved.

#include "Interaction/PickupInteractable.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Player/Components/InventoryComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "GameFramework/Actor.h"

APickupInteractable::APickupInteractable()
{
	PrimaryActorTick.bCanEverTick = true;

	// Create components
	PickupMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PickupMesh"));
	RootComponent = PickupMesh;
	PickupMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	PickupMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	PickupMesh->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

	InteractionVolume = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionVolume"));
	InteractionVolume->SetupAttachment(PickupMesh);
	InteractionVolume->SetSphereRadius(100.0f);
	InteractionVolume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	InteractionVolume->SetCollisionResponseToAllChannels(ECR_Ignore);
	InteractionVolume->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

	// Default settings
	InteractionPrompt = FText::FromString("Pick Up");
	bCanInteractMultipleTimes = false;
}

void APickupInteractable::BeginPlay()
{
	Super::BeginPlay();

	InitialLocation = GetActorLocation();
}

bool APickupInteractable::CanInteract_Implementation(AActor* InstigatorActor, const FHitResult& Hit) const
{
	if (!Super::CanInteract_Implementation(InstigatorActor, Hit))
	{
		return false;
	}

	// Check if already picked up
	if (bHasBeenInteracted && !bCanInteractMultipleTimes)
	{
		return false;
	}

	return true;
}

void APickupInteractable::OnInteract(AActor* InstigatorActor, const FHitResult& Hit)
{
	Super::OnInteract(InstigatorActor, Hit);

	// Try to add to inventory
	if (TryAddToInventory(InstigatorActor))
	{
		// Play pickup sound
		if (PickupSound && GetWorld())
		{
			UGameplayStatics::PlaySoundAtLocation(GetWorld(), PickupSound, GetActorLocation());
		}

		// Destroy pickup
		if (bDestroyOnPickup)
		{
			DestroyPickup();
		}
	}
}

bool APickupInteractable::TryAddToInventory(AActor* InstigatorActor)
{
	if (!InstigatorActor)
	{
		return false;
	}

	// Find inventory component
	UInventoryComponent* Inventory = InstigatorActor->FindComponentByClass<UInventoryComponent>();
	if (!Inventory)
	{
		return false;
	}

	// Add evidence if this is an evidence pickup
	if (PickupType == EPickupType::Evidence && EvidenceMetadata.EvidenceId != NAME_None)
	{
		// Register metadata
		Inventory->RegisterEvidenceMetadata(EvidenceMetadata);

		// Add to collected evidence
		return Inventory->AddCollectedEvidenceId(EvidenceMetadata.EvidenceId);
	}

	// For other pickup types, just mark as collected
	// (Could be extended to support other inventory systems)
	return true;
}

void APickupInteractable::DestroyPickup()
{
	// Hide mesh immediately
	PickupMesh->SetVisibility(false);
	SetActorEnableCollision(false);

	// Destroy after delay
	if (DestroyDelay > 0.0f)
	{
		FTimerHandle DestroyTimer;
		GetWorld()->GetTimerManager().SetTimer(
			DestroyTimer,
			this,
			&APickupInteractable::DestroyPickupActor,
			DestroyDelay,
			false
		);
	}
	else
	{
		Destroy();
	}
}

void APickupInteractable::DestroyPickupActor()
{
	Destroy();
}
