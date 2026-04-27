// Copyright Epic Games, Inc. All Rights Reserved.

#include "EvidenceCollectionComponent.h"
#include "Player/Components/QuantumCameraComponent.h"
#include "Player/Components/InventoryComponent.h"
#include "Engine/Engine.h"
#include "Engine/Texture2D.h"
#include "Engine/World.h"
#include "Engine/GameViewportClient.h"
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

	UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	bIsRecordingVideo = true;
	CurrentVideoEvidenceId = EvidenceId;
	VideoRecordingStartTime = World->GetTimeSeconds();

	return true;
}

bool UEvidenceCollectionComponent::StopVideoCapture(FName& OutEvidenceId, float& OutDuration)
{
	if (!bIsRecordingVideo)
	{
		return false;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		OutEvidenceId = NAME_None;
		OutDuration = 0.0f;
		return false;
	}

	OutEvidenceId = CurrentVideoEvidenceId;
	OutDuration = World->GetTimeSeconds() - VideoRecordingStartTime;

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

	const UWorld* World = GetWorld();
	return World ? World->GetTimeSeconds() - VideoRecordingStartTime : 0.0f;
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
	TArray<FColor> SurfaceData;
	FIntPoint ViewportSize;
	if (!ReadViewportPixels(ResolveGameViewport(), &SurfaceData, &ViewportSize))
	{
		return nullptr;
	}

	const float ResizeScale = CalculateThumbnailResizeScale(ViewportSize);
	const FIntPoint TextureSize = CalculateThumbnailSize(ViewportSize, ResizeScale);

	TArray<FColor> ThumbnailPixels;
	BuildThumbnailPixels(SurfaceData, ViewportSize, TextureSize, ResizeScale, &ThumbnailPixels);
	return CreateThumbnailTexture(TextureSize, ThumbnailPixels);
}

FViewport* UEvidenceCollectionComponent::ResolveGameViewport() const
{
	return GEngine && GEngine->GameViewport ? GEngine->GameViewport->Viewport : nullptr;
}

bool UEvidenceCollectionComponent::ReadViewportPixels(FViewport* Viewport, TArray<FColor>* OutSurfaceData, FIntPoint* OutViewportSize) const
{
	if (!Viewport || !OutSurfaceData || !OutViewportSize)
	{
		return false;
	}

	const FIntPoint ViewportSize = Viewport->GetSizeXY();
	if (ViewportSize.X <= 0 || ViewportSize.Y <= 0)
	{
		return false;
	}

	TArray<FColor> SurfaceData;
	if (!Viewport->ReadPixels(SurfaceData) || SurfaceData.Num() != ViewportSize.X * ViewportSize.Y)
	{
		return false;
	}

	*OutSurfaceData = MoveTemp(SurfaceData);
	*OutViewportSize = ViewportSize;
	return true;
}

float UEvidenceCollectionComponent::CalculateThumbnailResizeScale(const FIntPoint& ViewportSize) const
{
	constexpr int32 MaxThumbnailDimension = 512;
	return FMath::Min(
		1.0f,
		static_cast<float>(MaxThumbnailDimension) / static_cast<float>(FMath::Max(ViewportSize.X, ViewportSize.Y)));
}

FIntPoint UEvidenceCollectionComponent::CalculateThumbnailSize(const FIntPoint& ViewportSize, float ResizeScale) const
{
	const float ClampedResizeScale = FMath::Min(
		1.0f, 
		FMath::Max(ResizeScale, 0.0f));
	return FIntPoint(
		FMath::Max(1, FMath::RoundToInt(static_cast<float>(ViewportSize.X) * ClampedResizeScale)),
		FMath::Max(1, FMath::RoundToInt(static_cast<float>(ViewportSize.Y) * ClampedResizeScale)));
}

void UEvidenceCollectionComponent::BuildThumbnailPixels(
	const TArray<FColor>& SurfaceData,
	const FIntPoint& ViewportSize,
	const FIntPoint& TextureSize,
	float ResizeScale,
	TArray<FColor>* OutThumbnailPixels) const
{
	if (!OutThumbnailPixels)
	{
		return;
	}

	const float SafeResizeScale = FMath::Max(ResizeScale, SMALL_NUMBER);
	OutThumbnailPixels->SetNum(TextureSize.X * TextureSize.Y);
	for (int32 Y = 0; Y < TextureSize.Y; ++Y)
	{
		const int32 SourceY = FMath::Clamp(
			FMath::FloorToInt(static_cast<float>(Y) / SafeResizeScale),
			0,
			ViewportSize.Y - 1);
		for (int32 X = 0; X < TextureSize.X; ++X)
		{
			const int32 SourceX = FMath::Clamp(
				FMath::FloorToInt(static_cast<float>(X) / SafeResizeScale),
				0,
				ViewportSize.X - 1);
			const int32 SourceIndex = SourceY * ViewportSize.X + SourceX;
			const int32 DestIndex = Y * TextureSize.X + X;
			if (SurfaceData.IsValidIndex(SourceIndex) && OutThumbnailPixels->IsValidIndex(DestIndex))
			{
				const FColor* SourcePixel = SurfaceData.GetData() + SourceIndex;
				FColor* DestinationPixel = OutThumbnailPixels->GetData() + DestIndex;
				*DestinationPixel = *SourcePixel;
			}
		}
	}
}

UTexture2D* UEvidenceCollectionComponent::CreateThumbnailTexture(const FIntPoint& TextureSize, const TArray<FColor>& ThumbnailPixels) const
{
	UTexture2D* ThumbnailTexture = UTexture2D::CreateTransient(TextureSize.X, TextureSize.Y, PF_B8G8R8A8);
	if (!ThumbnailTexture)
	{
		return nullptr;
	}

	ThumbnailTexture->SRGB = true;
	FTexturePlatformData* PlatformData = ThumbnailTexture->GetPlatformData();
	if (!PlatformData || PlatformData->Mips.Num() == 0)
	{
		return nullptr;
	}

	FTexture2DMipMap* Mip = *PlatformData->Mips.GetData();
	if (!Mip)
	{
		return nullptr;
	}

	void* TextureData = Mip->BulkData.Lock(LOCK_READ_WRITE);
	if (!TextureData)
	{
		Mip->BulkData.Unlock();
		return nullptr;
	}

	const int32 TextureDataSize = ThumbnailPixels.Num() * sizeof(FColor);
	FMemory::Memcpy(TextureData, ThumbnailPixels.GetData(), TextureDataSize);
	Mip->BulkData.Unlock();
	ThumbnailTexture->UpdateResource();

	return ThumbnailTexture;
}
