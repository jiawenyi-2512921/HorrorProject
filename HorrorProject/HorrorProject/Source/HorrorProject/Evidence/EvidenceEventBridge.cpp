// Copyright Epic Games, Inc. All Rights Reserved.

#include "EvidenceEventBridge.h"
#include "Evidence/EvidenceCollectionComponent.h"
#include "Evidence/ArchiveSubsystem.h"
#include "Game/HorrorEventBusSubsystem.h"
#include "Engine/World.h"

namespace
{
	FGameplayTag GetEvidenceEventTag(EEvidenceType Type)
	{
		switch (Type)
		{
		case EEvidenceType::Physical:
			return FGameplayTag::RequestGameplayTag(TEXT("Event.Evidence.PhysicalCollected"), false);
		case EEvidenceType::Photo:
			return FGameplayTag::RequestGameplayTag(TEXT("Event.Evidence.PhotoCaptured"), false);
		case EEvidenceType::Video:
			return FGameplayTag::RequestGameplayTag(TEXT("Event.Evidence.VideoRecorded"), false);
		case EEvidenceType::Audio:
			return FGameplayTag::RequestGameplayTag(TEXT("Event.Evidence.AudioRecorded"), false);
		case EEvidenceType::Note:
			return FGameplayTag::RequestGameplayTag(TEXT("Event.Evidence.NoteCollected"), false);
		case EEvidenceType::Document:
			return FGameplayTag::RequestGameplayTag(TEXT("Event.Evidence.DocumentCollected"), false);
		default:
			return FGameplayTag();
		}
	}

	FGameplayTag GetEvidenceCapturedStateTag()
	{
		return FGameplayTag::RequestGameplayTag(TEXT("State.Evidence.Captured"), false);
	}
}

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
		if (UWorld* World = GetWorld())
		{
			CachedEventBus = World->GetSubsystem<UHorrorEventBusSubsystem>();
		}
	}

	if (!CachedEventBus.IsValid())
	{
		UE_LOG(LogTemp, Verbose, TEXT("Evidence event '%s' was not published: event bus unavailable"), *EvidenceId.ToString());
		return;
	}

	const FGameplayTag EventTag = GetEvidenceEventTag(Type);
	const FGameplayTag StateTag = GetEvidenceCapturedStateTag();
	if (!EventTag.IsValid() || !StateTag.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("Evidence event '%s' was not published: gameplay tags are not registered"), *EvidenceId.ToString());
		return;
	}

	UObject* SourceObject = BoundCollectionComponent.IsValid()
		? static_cast<UObject*>(BoundCollectionComponent.Get())
		: static_cast<UObject*>(this);
	if (!CachedEventBus->Publish(EventTag, EvidenceId, StateTag, SourceObject))
	{
		UE_LOG(LogTemp, Warning, TEXT("Evidence event '%s' failed to publish to the event bus"), *EvidenceId.ToString());
	}
}
