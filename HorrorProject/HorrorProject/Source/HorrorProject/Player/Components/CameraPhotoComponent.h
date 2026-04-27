// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "CameraPhotoComponent.generated.h"

class UTexture2D;
class UTextureRenderTarget2D;
class USceneCaptureComponent2D;
class USoundBase;
class UQuantumCameraComponent;
class UCameraComponent;

USTRUCT(BlueprintType)
struct HORRORPROJECT_API FCameraPhotoMetadata
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category="Photo")
	FGuid PhotoId = FGuid();

	UPROPERTY(BlueprintReadOnly, Category="Photo")
	FDateTime CaptureTimestamp = FDateTime();

	UPROPERTY(BlueprintReadOnly, Category="Photo")
	FVector CaptureLocation = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Category="Photo")
	FRotator CaptureRotation = FRotator::ZeroRotator;

	UPROPERTY(BlueprintReadOnly, Category="Photo")
	FGameplayTagContainer ContextTags;

	UPROPERTY(BlueprintReadOnly, Category="Photo")
	TArray<FName> DetectedEvidenceIds;

	UPROPERTY(BlueprintReadOnly, Category="Photo")
	float ExposureValue = 1.0f;

	UPROPERTY(BlueprintReadOnly, Category="Photo")
	bool bFlashUsed = false;

	FCameraPhotoMetadata() = default;
};

USTRUCT(BlueprintType)
struct HORRORPROJECT_API FCameraPhoto
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category="Photo")
	FCameraPhotoMetadata Metadata;

	UPROPERTY(BlueprintReadOnly, Category="Photo")
	TObjectPtr<UTexture2D> PhotoTexture = nullptr;

	UPROPERTY(BlueprintReadOnly, Category="Photo")
	TObjectPtr<UTexture2D> ThumbnailTexture = nullptr;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPhotoTakenSignature, const FCameraPhoto&, Photo);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFlashFiredSignature, float, Intensity);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPhotoStoredSignature, FGuid, PhotoId);

/**
 * Adds Camera Photo Component behavior to its owning actor in the Player module.
 */
UCLASS(ClassGroup=(Horror), BlueprintType, Blueprintable, meta=(BlueprintSpawnableComponent))
class HORRORPROJECT_API UCameraPhotoComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	static constexpr int32 DefaultPhotoResolutionWidth = 1920;
	static constexpr int32 DefaultPhotoResolutionHeight = 1080;
	static constexpr int32 DefaultThumbnailResolutionWidth = 256;
	static constexpr int32 DefaultThumbnailResolutionHeight = 144;
	static constexpr int32 DefaultMaxPhotoCapacity = 36;
	static constexpr float DefaultFlashIntensity = 5000.0f;
	static constexpr float DefaultEvidenceDetectionRadius = 2000.0f;
	static constexpr float InitialLastPhotoTime = -999.0f;

	UCameraPhotoComponent();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION(BlueprintCallable, Category="CameraPhoto")
	bool TakePhoto(bool bUseFlash = true);

	UFUNCTION(BlueprintCallable, Category="CameraPhoto")
	bool CanTakePhoto() const;

	UFUNCTION(BlueprintPure, Category="CameraPhoto")
	int32 GetPhotoCount() const { return StoredPhotos.Num(); }

	UFUNCTION(BlueprintPure, Category="CameraPhoto")
	int32 GetMaxPhotoCapacity() const { return MaxPhotoCapacity; }

	UFUNCTION(BlueprintPure, Category="CameraPhoto")
	bool IsPhotoStorageFull() const { return StoredPhotos.Num() >= MaxPhotoCapacity; }

	UFUNCTION(BlueprintCallable, Category="CameraPhoto")
	FCameraPhoto GetPhoto(FGuid PhotoId) const;

	UFUNCTION(BlueprintCallable, Category="CameraPhoto")
	TArray<FCameraPhoto> GetAllPhotos() const { return StoredPhotos; }

	UFUNCTION(BlueprintCallable, Category="CameraPhoto")
	bool DeletePhoto(FGuid PhotoId);

	UFUNCTION(BlueprintCallable, Category="CameraPhoto")
	void ClearAllPhotos();

	UFUNCTION(BlueprintCallable, Category="CameraPhoto")
	void SetFlashEnabled(bool bEnabled) { bFlashEnabled = bEnabled; }

	UFUNCTION(BlueprintPure, Category="CameraPhoto")
	bool IsFlashEnabled() const { return bFlashEnabled; }

	UFUNCTION(BlueprintCallable, Category="CameraPhoto")
	void SetAutoDetectEvidence(bool bEnabled) { bAutoDetectEvidence = bEnabled; }

	UFUNCTION(BlueprintPure, Category="CameraPhoto")
	bool IsAutoDetectEvidenceEnabled() const { return bAutoDetectEvidence; }

	UPROPERTY(BlueprintAssignable, Category="CameraPhoto")
	FOnPhotoTakenSignature OnPhotoTaken;

	UPROPERTY(BlueprintAssignable, Category="CameraPhoto")
	FOnFlashFiredSignature OnFlashFired;

	UPROPERTY(BlueprintAssignable, Category="CameraPhoto")
	FOnPhotoStoredSignature OnPhotoStored;

protected:
	UPROPERTY(EditDefaultsOnly, Category="CameraPhoto|Settings")
	int32 PhotoResolutionWidth = DefaultPhotoResolutionWidth;

	UPROPERTY(EditDefaultsOnly, Category="CameraPhoto|Settings")
	int32 PhotoResolutionHeight = DefaultPhotoResolutionHeight;

	UPROPERTY(EditDefaultsOnly, Category="CameraPhoto|Settings")
	int32 ThumbnailResolutionWidth = DefaultThumbnailResolutionWidth;

	UPROPERTY(EditDefaultsOnly, Category="CameraPhoto|Settings")
	int32 ThumbnailResolutionHeight = DefaultThumbnailResolutionHeight;

	UPROPERTY(EditDefaultsOnly, Category="CameraPhoto|Settings")
	int32 MaxPhotoCapacity = DefaultMaxPhotoCapacity;

	UPROPERTY(EditDefaultsOnly, Category="CameraPhoto|Settings")
	float PhotoCooldownTime = 0.5f;

	UPROPERTY(EditDefaultsOnly, Category="CameraPhoto|Flash")
	bool bFlashEnabled = true;

	UPROPERTY(EditDefaultsOnly, Category="CameraPhoto|Flash")
	float FlashIntensity = DefaultFlashIntensity;

	UPROPERTY(EditDefaultsOnly, Category="CameraPhoto|Flash")
	float FlashDuration = 0.1f;

	UPROPERTY(EditDefaultsOnly, Category="CameraPhoto|Flash")
	float FlashRadius = 1000.0f;

	UPROPERTY(EditDefaultsOnly, Category="CameraPhoto|Audio")
	TObjectPtr<USoundBase> ShutterSound;

	UPROPERTY(EditDefaultsOnly, Category="CameraPhoto|Audio")
	TObjectPtr<USoundBase> FlashChargeSound;

	UPROPERTY(EditDefaultsOnly, Category="CameraPhoto|Audio")
	float ShutterVolume = 1.0f;

	UPROPERTY(EditDefaultsOnly, Category="CameraPhoto|Evidence")
	bool bAutoDetectEvidence = true;

	UPROPERTY(EditDefaultsOnly, Category="CameraPhoto|Evidence")
	float EvidenceDetectionRadius = DefaultEvidenceDetectionRadius;

	UPROPERTY(EditDefaultsOnly, Category="CameraPhoto|Evidence")
	FGameplayTag PhotoEvidenceTag;

private:
	UPROPERTY(Transient)
	TArray<FCameraPhoto> StoredPhotos;

	UPROPERTY(Transient)
	TObjectPtr<UTextureRenderTarget2D> PhotoRenderTarget;

	UPROPERTY(Transient)
	TObjectPtr<UTextureRenderTarget2D> ThumbnailRenderTarget;

	UPROPERTY(Transient)
	TObjectPtr<USceneCaptureComponent2D> CaptureComponent;

	UPROPERTY(Transient)
	TObjectPtr<UQuantumCameraComponent> QuantumCamera;

	float LastPhotoTime = InitialLastPhotoTime;
	bool bIsCapturing = false;

	void InitializeCaptureComponents();
	void SetupCaptureComponent();
	UCameraComponent* BeginPhotoCapture();
	void CompletePhotoCapture(const FCameraPhoto& Photo);
	bool AreCaptureComponentsReady() const;
	UCameraComponent* ResolvePlayerCamera() const;
	void SyncCaptureComponentToCamera(const UCameraComponent* PlayerCamera);
	void CaptureSceneToRenderTarget(UTextureRenderTarget2D* RenderTarget);
	UTexture2D* CaptureToTexture(UTextureRenderTarget2D* RenderTarget);
	FCameraPhoto BuildCapturedPhoto(bool bFlashUsed, UTexture2D* PhotoTexture, UTexture2D* ThumbnailTexture);
	void TriggerFlashEffect(float Intensity);
	void PlayShutterSound();
	TArray<FName> DetectEvidenceInView();
	TArray<FHitResult> TraceEvidenceInView(const UCameraComponent* PlayerCamera) const;
	void CollectEvidenceTagsFromHits(const TArray<FHitResult>& HitResults, TArray<FName>* OutDetectedEvidence) const;
	FCameraPhotoMetadata BuildPhotoMetadata(bool bFlashUsed);
	void StorePhoto(const FCameraPhoto& Photo);
};
