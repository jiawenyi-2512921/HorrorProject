// Copyright Epic Games, Inc. All Rights Reserved.

#include "EvidenceEventBridge.h"
#include "Evidence/EvidenceCollectionComponent.h"
#include "Evidence/ArchiveSubsystem.h"
#include "Game/HorrorEventBusSubsystem.h"
#include "Engine/World.h"

UEvidenceEventBridge::UEvidenceEventBridge()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UEvidenceEventBridge::BeginPlay()
{
	Super::BeginPlay();

	if (UWorld* World = GetWorld())
	{
		if (UGameInstance* GameInstance = World->GetGameInstance())
		{
			CachedArchiveSubsystem = GameInstance->GetSubsystem<UArchiveSubsystem>();
		}
		CachedEventBus = World->GetSubsystem<UHorrorEventBusSubsystem>();
	}

	if (bAutoBindOnBeginPlay)
	{
		if (AActor* Owner = GetOwner())
		{
			if (UEvidenceCollectionComponent* CollectionComp = Owner->FindComponentByClass<UEvidenceCollectionComponent>())
			{
				BindToEvidenceCollection(CollectionComp);
			}
		}
	}
}

void UEvidenceEventBridge::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UnbindFromEvidenceCollection();
	Super::EndPlay(EndPlayReason);
}

void UEvidenceEventBridge::BindToEvidenceCollection(UEvidenceCollectionComponent* CollectionComponent)
{
	if (!CollectionComponent)
	{
		return;
	}

	UnbindFromEvidenceCollection();

	BoundCollectionComponent = CollectionComponent;

	CollectionComponent->OnEvidenceCaptured.AddDynamic(this, &UEvidenceEventBridge::OnEvidenceCaptured);
	CollectionComponent->OnPhotoTaken.AddDynamic(this, &UEvidenceEventBridge::OnPhotoTaken);
	CollectionComponent->OnVideoRecorded.AddDynamic(this, &UEvidenceEventBridge::OnVideoRecorded);
}

void UEvidenceEventBridge::UnbindFromEvidenceCollection()
{
	if (BoundCollectionComponent.IsValid())
	{
		BoundCollectionComponent->OnEvidenceCaptured.RemoveDynamic(this, &UEvidenceEventBridge::OnEvidenceCaptured);
		BoundCollectionComponent->OnPhotoTaken.RemoveDynamic(this, &UEvidenceEventBridge::OnPhotoTaken);
		BoundCollectionComponent->OnVideoRecorded.RemoveDynamic(this, &UEvidenceEventBridge::OnVideoRecorded);
		BoundCollectionComponent.Reset();
	}
}

void UEvidenceEventBridge::OnEvidenceCaptured(FName EvidenceId, const FEvidenceCaptureData& CaptureData)
{
	if (bAddToArchive && CachedArchiveSubsystem.IsValid())
	{
		if (BoundCollectionComponent.IsValid())
		{
			FExtendedEvidenceMetadata Metadata;
			if (BoundCollectionComponent->GetEvidenceMetadata(EvidenceId, Metadata))
			{
				CachedArchiveSubsystem->AddArchiveEntry(Metadata);
			}
		}
	}

	if (bPublishToEventBus)
	{
		PublishEvidenceEvent(EvidenceId, CaptureData.Type);
	}
}

void UEvidenceEventBridge::OnPhotoTaken(FName PhotoId, UTexture2D* Photo, FVector Location)
{
	// Additional photo-specific handling can go here
}

void UEvidenceEventBridge::OnVideoRecorded(FName VideoId, float Duration, FVector Location)
{
	// Additional video-specific handling can go here
}

void UEvidenceEventBridge::PublishEvidenceEvent(FName EvidenceId, EEvidenceType Type)
{
	if (!CachedEventBus.IsValid())
	{
		return;
	}

	// Create event message for evidence collection
	// This would integrate with the existing HorrorEventBusSubsystem
	// For now, this is a placeholder for the integration point
}
