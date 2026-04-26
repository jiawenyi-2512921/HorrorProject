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

USTRUCT(BlueprintType)
struct HORRORPROJECT_API FCameraPhotoMetadata
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category="Photo")
	FGuid PhotoId;

	UPROPERTY(BlueprintReadOnly, Category="Photo")
	FDateTime CaptureTimestamp;

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

	FCameraPhotoMetadata()
	{
		PhotoId = FGuid::NewGuid();
		CaptureTimestamp = FDateTime::Now();
	}
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

UCLASS(ClassGroup=(Horror), BlueprintType, Blueprintable, meta=(BlueprintSpawnableComponent))
class HORRORPROJECT_API UCameraPhotoComponent : public UActorComponent
{
	GENERATED_BODY()

public:
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
	int32 PhotoResolutionWidth = 1920;

	UPROPERTY(EditDefaultsOnly, Category="CameraPhoto|Settings")
	int32 PhotoResolutionHeight = 1080;

	UPROPERTY(EditDefaultsOnly, Category="CameraPhoto|Settings")
	int32 ThumbnailResolutionWidth = 256;

	UPROPERTY(EditDefaultsOnly, Category="CameraPhoto|Settings")
	int32 ThumbnailResolutionHeight = 144;

	UPROPERTY(EditDefaultsOnly, Category="CameraPhoto|Settings")
	int32 MaxPhotoCapacity = 36;

	UPROPERTY(EditDefaultsOnly, Category="CameraPhoto|Settings")
	float PhotoCooldownTime = 0.5f;

	UPROPERTY(EditDefaultsOnly, Category="CameraPhoto|Flash")
	bool bFlashEnabled = true;

	UPROPERTY(EditDefaultsOnly, Category="CameraPhoto|Flash")
	float FlashIntensity = 5000.0f;

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
	float EvidenceDetectionRadius = 2000.0f;

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

	float LastPhotoTime = -999.0f;
	bool bIsCapturing = false;

	void InitializeCaptureComponents();
	void SetupCaptureComponent();
	UTexture2D* CaptureToTexture(UTextureRenderTarget2D* RenderTarget);
	void TriggerFlashEffect(float Intensity);
	void PlayShutterSound();
	TArray<FName> DetectEvidenceInView();
	FCameraPhotoMetadata BuildPhotoMetadata(bool bFlashUsed);
	void StorePhoto(const FCameraPhoto& Photo);
};
