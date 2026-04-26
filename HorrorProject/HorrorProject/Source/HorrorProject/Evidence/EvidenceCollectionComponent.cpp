// Copyright Epic Games, Inc. All Rights Reserved.

#include "EvidenceCollectionComponent.h"
#include "Player/Components/QuantumCameraComponent.h"
#include "Player/Components/InventoryComponent.h"
#include "Engine/Texture2D.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"

UEvidenceCollectionComponent::UEvidenceCollectionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UEvidenceCollectionComponent::BeginPlay()
{
	Super::BeginPlay();

	AActor* Owner = GetOwner();
	if (Owner)
	{
		CachedCameraComponent = Owner->FindComponentByClass<UQuantumCameraComponent>();
		CachedInventoryComponent = Owner->FindComponentByClass<UInventoryComponent>();
	}
}

bool UEvidenceCollectionComponent::CapturePhotoEvidence(FName EvidenceId, const FExtendedEvidenceMetadata& Metadata)
{
	if (EvidenceId.IsNone())
	{
		return false;
	}

	if (PhotoCount >= MaxPhotos)
	{
		return false;
	}

	if (CachedCameraComponent.IsValid() && !CachedCameraComponent->IsCameraMode(EQuantumCameraMode::Photo))
	{
		return false;
	}

	FExtendedEvidenceMetadata NewMetadata = Metadata;
	NewMetadata.EvidenceId = EvidenceId;
	NewMetadata.Type = EEvidenceType::Photo;
	NewMetadata.CollectionTimestamp = FDateTime::Now();

	if (AActor* Owner = GetOwner())
	{
		NewMetadata.CollectionLocation = Owner->GetActorLocation();
	}

	NewMetadata.ThumbnailImage = CaptureScreenshot();

	RegisterEvidenceMetadata(NewMetadata);

	if (CachedInventoryComponent.IsValid())
	{
		CachedInventoryComponent->AddCollectedEvidenceId(EvidenceId);
	}

	FEvidenceCaptureData CaptureData = CreateCaptureData(EvidenceId, EEvidenceType::Photo);
	CaptureData.CapturedImage = NewMetadata.ThumbnailImage;
	BroadcastEvidenceCapture(EvidenceId, CaptureData);

	OnPhotoTaken.Broadcast(EvidenceId, NewMetadata.ThumbnailImage, NewMetadata.CollectionLocation);

	PhotoCount++;
	return true;
}

bool UEvidenceCollectionComponent::StartVideoCapture(FName EvidenceId)
{
	if (EvidenceId.IsNone() || bIsRecordingVideo)
	{
		return false;
	}

	if (VideoCount >= MaxVideos)
	{
		return false;
	}

	if (CachedCameraComponent.IsValid() && !CachedCameraComponent->IsCameraMode(EQuantumCameraMode::Recording))
	{
		return false;
	}

	bIsRecordingVideo = true;
	CurrentVideoEvidenceId = EvidenceId;
	VideoRecordingStartTime = GetWorld()->GetTimeSeconds();

	return true;
}

bool UEvidenceCollectionComponent::StopVideoCapture(FName& OutEvidenceId, float& OutDuration)
{
	if (!bIsRecordingVideo)
	{
		return false;
	}

	OutEvidenceId = CurrentVideoEvidenceId;
	OutDuration = GetWorld()->GetTimeSeconds() - VideoRecordingStartTime;

	FExtendedEvidenceMetadata VideoMetadata;
	VideoMetadata.EvidenceId = CurrentVideoEvidenceId;
	VideoMetadata.Type = EEvidenceType::Video;
	VideoMetadata.CollectionTimestamp = FDateTime::Now();
	VideoMetadata.DisplayName = FText::FromString(FString::Printf(TEXT("Video Recording %d"), VideoCount + 1));

	if (AActor* Owner = GetOwner())
	{
		VideoMetadata.CollectionLocation = Owner->GetActorLocation();
	}

	RegisterEvidenceMetadata(VideoMetadata);

	if (CachedInventoryComponent.IsValid())
	{
		CachedInventoryComponent->AddCollectedEvidenceId(CurrentVideoEvidenceId);
	}

	FEvidenceCaptureData CaptureData = CreateCaptureData(CurrentVideoEvidenceId, EEvidenceType::Video);
	CaptureData.VideoDuration = OutDuration;
	BroadcastEvidenceCapture(CurrentVideoEvidenceId, CaptureData);

	OnVideoRecorded.Broadcast(CurrentVideoEvidenceId, OutDuration, VideoMetadata.CollectionLocation);

	bIsRecordingVideo = false;
	CurrentVideoEvidenceId = NAME_None;
	VideoRecordingStartTime = 0.0f;
	VideoCount++;

	return true;
}

bool UEvidenceCollectionComponent::CollectPhysicalEvidence(FName EvidenceId, const FExtendedEvidenceMetadata& Metadata)
{
	if (EvidenceId.IsNone())
	{
		return false;
	}

	FExtendedEvidenceMetadata NewMetadata = Metadata;
	NewMetadata.EvidenceId = EvidenceId;
	NewMetadata.Type = EEvidenceType::Physical;
	NewMetadata.CollectionTimestamp = FDateTime::Now();

	if (AActor* Owner = GetOwner())
	{
		NewMetadata.CollectionLocation = Owner->GetActorLocation();
	}

	RegisterEvidenceMetadata(NewMetadata);

	if (CachedInventoryComponent.IsValid())
	{
		CachedInventoryComponent->AddCollectedEvidenceId(EvidenceId);
	}

	FEvidenceCaptureData CaptureData = CreateCaptureData(EvidenceId, EEvidenceType::Physical);
	BroadcastEvidenceCapture(EvidenceId, CaptureData);

	return true;
}

bool UEvidenceCollectionComponent::RegisterEvidenceMetadata(const FExtendedEvidenceMetadata& Metadata)
{
	if (Metadata.EvidenceId.IsNone())
	{
		return false;
	}

	EvidenceMetadataMap.Add(Metadata.EvidenceId, Metadata);
	return true;
}

bool UEvidenceCollectionComponent::GetEvidenceMetadata(FName EvidenceId, FExtendedEvidenceMetadata& OutMetadata) const
{
	if (const FExtendedEvidenceMetadata* Found = EvidenceMetadataMap.Find(EvidenceId))
	{
		OutMetadata = *Found;
		return true;
	}
	return false;
}

TArray<FExtendedEvidenceMetadata> UEvidenceCollectionComponent::GetAllEvidenceMetadata() const
{
	TArray<FExtendedEvidenceMetadata> Result;
	EvidenceMetadataMap.GenerateValueArray(Result);
	return Result;
}

TArray<FExtendedEvidenceMetadata> UEvidenceCollectionComponent::GetEvidenceByType(EEvidenceType Type) const
{
	TArray<FExtendedEvidenceMetadata> Result;
	for (const auto& Pair : EvidenceMetadataMap)
	{
		if (Pair.Value.Type == Type)
		{
			Result.Add(Pair.Value);
		}
	}
	return Result;
}

TArray<FExtendedEvidenceMetadata> UEvidenceCollectionComponent::GetEvidenceByCategory(EEvidenceCategory Category) const
{
	TArray<FExtendedEvidenceMetadata> Result;
	for (const auto& Pair : EvidenceMetadataMap)
	{
		if (Pair.Value.Category == Category)
		{
			Result.Add(Pair.Value);
		}
	}
	return Result;
}

TArray<FExtendedEvidenceMetadata> UEvidenceCollectionComponent::GetEvidenceByTags(FGameplayTagContainer Tags, bool bRequireAll) const
{
	TArray<FExtendedEvidenceMetadata> Result;
	for (const auto& Pair : EvidenceMetadataMap)
	{
		bool bMatches = bRequireAll
			? Pair.Value.Tags.HasAll(Tags)
			: Pair.Value.Tags.HasAny(Tags);

		if (bMatches)
		{
			Result.Add(Pair.Value);
		}
	}
	return Result;
}

float UEvidenceCollectionComponent::GetCurrentVideoDuration() const
{
	if (!bIsRecordingVideo)
	{
		return 0.0f;
	}
	return GetWorld()->GetTimeSeconds() - VideoRecordingStartTime;
}

FEvidenceCaptureData UEvidenceCollectionComponent::CreateCaptureData(FName EvidenceId, EEvidenceType Type) const
{
	FEvidenceCaptureData CaptureData;
	CaptureData.EvidenceId = EvidenceId;
	CaptureData.Type = Type;
	CaptureData.CaptureTimestamp = FDateTime::Now();

	if (AActor* Owner = GetOwner())
	{
		CaptureData.CaptureLocation = Owner->GetActorLocation();
		CaptureData.CaptureRotation = Owner->GetActorRotation();
	}

	return CaptureData;
}

void UEvidenceCollectionComponent::BroadcastEvidenceCapture(FName EvidenceId, const FEvidenceCaptureData& CaptureData)
{
	OnEvidenceCaptured.Broadcast(EvidenceId, CaptureData);
}

UTexture2D* UEvidenceCollectionComponent::CaptureScreenshot() const
{
	// Placeholder - real implementation would use render target capture
	// For now return nullptr, Blueprint can override or use render targets
	return nullptr;
}
