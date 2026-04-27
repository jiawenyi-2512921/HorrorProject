// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EvidenceTypes.h"
#include "EvidenceCollectionComponent.generated.h"

class UQuantumCameraComponent;
class UInventoryComponent;
class FViewport;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FEvidenceCapturedSignature, FName, EvidenceId, const FEvidenceCaptureData&, CaptureData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FPhotoTakenSignature, FName, PhotoId, UTexture2D*, Photo, FVector, Location);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FVideoRecordedSignature, FName, VideoId, float, Duration, FVector, Location);

namespace HorrorEvidenceCollectionDefaults
{
	inline constexpr int32 MaxPhotos = 50;
	inline constexpr int32 MaxVideos = 10;
	inline constexpr float MaxVideoDurationSeconds = 30.0f;
}

/**
 * Adds Evidence Collection Component behavior to its owning actor in the Evidence module.
 */
UCLASS(ClassGroup=(Horror), BlueprintType, Blueprintable, meta=(BlueprintSpawnableComponent))
class HORRORPROJECT_API UEvidenceCollectionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UEvidenceCollectionComponent();

	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category="Evidence|Collection")
	bool CapturePhotoEvidence(FName EvidenceId, const FExtendedEvidenceMetadata& Metadata);

	UFUNCTION(BlueprintCallable, Category="Evidence|Collection")
	bool StartVideoCapture(FName EvidenceId);

	UFUNCTION(BlueprintCallable, Category="Evidence|Collection")
	bool StopVideoCapture(FName& OutEvidenceId, float& OutDuration);

	UFUNCTION(BlueprintCallable, Category="Evidence|Collection")
	bool CollectPhysicalEvidence(FName EvidenceId, const FExtendedEvidenceMetadata& Metadata);

	UFUNCTION(BlueprintCallable, Category="Evidence|Collection")
	bool RegisterEvidenceMetadata(const FExtendedEvidenceMetadata& Metadata);

	UFUNCTION(BlueprintPure, Category="Evidence|Collection")
	bool GetEvidenceMetadata(FName EvidenceId, FExtendedEvidenceMetadata& OutMetadata) const;

	UFUNCTION(BlueprintPure, Category="Evidence|Collection")
	TArray<FExtendedEvidenceMetadata> GetAllEvidenceMetadata() const;

	UFUNCTION(BlueprintPure, Category="Evidence|Collection")
	TArray<FExtendedEvidenceMetadata> GetEvidenceByType(EEvidenceType Type) const;

	UFUNCTION(BlueprintPure, Category="Evidence|Collection")
	TArray<FExtendedEvidenceMetadata> GetEvidenceByCategory(EEvidenceCategory Category) const;

	UFUNCTION(BlueprintPure, Category="Evidence|Collection")
	TArray<FExtendedEvidenceMetadata> GetEvidenceByTags(FGameplayTagContainer Tags, bool bRequireAll = false) const;

	UFUNCTION(BlueprintPure, Category="Evidence|Collection")
	bool IsCapturingVideo() const { return bIsRecordingVideo; }

	UFUNCTION(BlueprintPure, Category="Evidence|Collection")
	float GetCurrentVideoDuration() const;

	UPROPERTY(BlueprintAssignable, Category="Evidence|Collection")
	FEvidenceCapturedSignature OnEvidenceCaptured;

	UPROPERTY(BlueprintAssignable, Category="Evidence|Collection")
	FPhotoTakenSignature OnPhotoTaken;

	UPROPERTY(BlueprintAssignable, Category="Evidence|Collection")
	FVideoRecordedSignature OnVideoRecorded;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Evidence|Settings")
	int32 MaxPhotos = HorrorEvidenceCollectionDefaults::MaxPhotos;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Evidence|Settings")
	int32 MaxVideos = 10;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Evidence|Settings")
	float MaxVideoDuration = HorrorEvidenceCollectionDefaults::MaxVideoDurationSeconds;

private:
	FEvidenceCaptureData CreateCaptureData(FName EvidenceId, EEvidenceType Type) const;
	void BroadcastEvidenceCapture(FName EvidenceId, const FEvidenceCaptureData& CaptureData);
	UTexture2D* CaptureScreenshot() const;
	FViewport* ResolveGameViewport() const;
	bool ReadViewportPixels(FViewport* Viewport, TArray<FColor>* OutSurfaceData, FIntPoint* OutViewportSize) const;
	float CalculateThumbnailResizeScale(const FIntPoint& ViewportSize) const;
	FIntPoint CalculateThumbnailSize(const FIntPoint& ViewportSize, float ResizeScale) const;
	void BuildThumbnailPixels(
		const TArray<FColor>& SurfaceData,
		const FIntPoint& ViewportSize,
		const FIntPoint& TextureSize,
		float ResizeScale,
		TArray<FColor>* OutThumbnailPixels) const;
	UTexture2D* CreateThumbnailTexture(const FIntPoint& TextureSize, const TArray<FColor>& ThumbnailPixels) const;

	UPROPERTY()
	TMap<FName, FExtendedEvidenceMetadata> EvidenceMetadataMap;

	UPROPERTY()
	TWeakObjectPtr<UQuantumCameraComponent> CachedCameraComponent;

	UPROPERTY()
	TWeakObjectPtr<UInventoryComponent> CachedInventoryComponent;

	bool bIsRecordingVideo = false;
	FName CurrentVideoEvidenceId = NAME_None;
	float VideoRecordingStartTime = 0.0f;
	int32 PhotoCount = 0;
	int32 VideoCount = 0;
};
