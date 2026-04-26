// Copyright Epic Games, Inc. All Rights Reserved.

#include "Interaction/DocumentInteractable.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Player/Components/InventoryComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"

ADocumentInteractable::ADocumentInteractable()
{
	PrimaryActorTick.bCanEverTick = false;

	// Create components
	DocumentMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DocumentMesh"));
	RootComponent = DocumentMesh;
	DocumentMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	DocumentMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	DocumentMesh->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

	InteractionVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractionVolume"));
	InteractionVolume->SetupAttachment(DocumentMesh);
	InteractionVolume->SetBoxExtent(FVector(50.0f, 50.0f, 10.0f));
	InteractionVolume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	InteractionVolume->SetCollisionResponseToAllChannels(ECR_Ignore);
	InteractionVolume->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

	// Default settings
	InteractionPrompt = FText::FromString("Read Document");
	bCanInteractMultipleTimes = true;
}

void ADocumentInteractable::BeginPlay()
{
	Super::BeginPlay();

	// Auto-generate evidence metadata if not set
	if (bIsEvidence && EvidenceMetadata.EvidenceId == NAME_None && InteractableId != NAME_None)
	{
		EvidenceMetadata.EvidenceId = InteractableId;
		EvidenceMetadata.DisplayName = DocumentTitle;
		EvidenceMetadata.Description = FText::FromString("A document containing important information.");
	}
}

bool ADocumentInteractable::CanInteract_Implementation(AActor* InstigatorActor, const FHitResult& Hit) const
{
	if (!Super::CanInteract_Implementation(InstigatorActor, Hit))
	{
		return false;
	}

	// Can't interact if already open
	if (bIsCurrentlyOpen)
	{
		return false;
	}

	return true;
}

void ADocumentInteractable::OnInteract(AActor* InstigatorActor, const FHitResult& Hit)
{
	Super::OnInteract(InstigatorActor, Hit);

	APlayerController* PC = Cast<APlayerController>(InstigatorActor->GetInstigatorController());
	if (!PC)
	{
		return;
	}

	ReadingPlayerController = PC;
	bIsCurrentlyOpen = true;

	// Add to evidence if configured
	if (bAutoCollectAsEvidence && bIsEvidence)
	{
		TryAddToEvidence(InstigatorActor);
	}

	// Play open sound
	if (OpenSound && GetWorld())
	{
		UGameplayStatics::PlaySound2D(GetWorld(), OpenSound);
	}

	// Disable player movement if configured
	if (bDisablePlayerMovement)
	{
		if (ACharacter* Character = Cast<ACharacter>(PC->GetPawn()))
		{
			if (UCharacterMovementComponent* Movement = Character->GetCharacterMovement())
			{
				Movement->DisableMovement();
			}
		}
	}

	// Pause game if configured
	if (bPauseGame)
	{
		UGameplayStatics::SetGamePaused(GetWorld(), true);
	}

	// Show document UI (would be implemented via Blueprint or UI system)
	ShowDocumentUI();
}

void ADocumentInteractable::CloseDocument()
{
	if (!bIsCurrentlyOpen)
	{
		return;
	}

	bIsCurrentlyOpen = false;

	// Play close sound
	if (CloseSound && GetWorld())
	{
		UGameplayStatics::PlaySound2D(GetWorld(), CloseSound);
	}

	// Re-enable player movement
	if (bDisablePlayerMovement && ReadingPlayerController)
	{
		if (ACharacter* Character = Cast<ACharacter>(ReadingPlayerController->GetPawn()))
		{
			if (UCharacterMovementComponent* Movement = Character->GetCharacterMovement())
			{
				Movement->SetMovementMode(MOVE_Walking);
			}
		}
	}

	// Unpause game
	if (bPauseGame)
	{
		UGameplayStatics::SetGamePaused(GetWorld(), false);
	}

	ReadingPlayerController = nullptr;
}

bool ADocumentInteractable::TryAddToEvidence(AActor* InstigatorActor)
{
	if (!InstigatorActor || !bIsEvidence)
	{
		return false;
	}

	UInventoryComponent* Inventory = InstigatorActor->FindComponentByClass<UInventoryComponent>();
	if (!Inventory)
	{
		return false;
	}

	// Register metadata
	Inventory->RegisterEvidenceMetadata(EvidenceMetadata);

	// Add to collected evidence
	return Inventory->AddCollectedEvidenceId(EvidenceMetadata.EvidenceId);
}

void ADocumentInteractable::ShowDocumentUI()
{
	// This would trigger a Blueprint event or UI widget
	// Implementation depends on the game's UI system
}
