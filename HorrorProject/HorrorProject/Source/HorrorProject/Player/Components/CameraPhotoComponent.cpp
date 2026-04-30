// Copyright Epic Games, Inc. All Rights Reserved.

#include "CameraPhotoComponent.h"
#include "QuantumCameraComponent.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Engine/Texture2D.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Sound/SoundBase.h"
#include "Audio/HorrorAudioSubsystem.h"
#include "ImageUtils.h"

UCameraPhotoComponent::UCameraPhotoComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	bWantsInitializeComponent = false;
}

void UCameraPhotoComponent::BeginPlay()
{
	Super::BeginPlay();

	AActor* Owner = GetOwner();
	QuantumCamera = Owner ? Owner->FindComponentByClass<UQuantumCameraComponent>() : nullptr;
	if (!QuantumCamera)
	{
		UE_LOG(LogTemp, Warning, TEXT("CameraPhotoComponent: No QuantumCameraComponent found on owner"));
	}

	InitializeCaptureComponents();
}

void UCameraPhotoComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (CaptureComponent)
	{
		CaptureComponent->DestroyComponent();
		CaptureComponent = nullptr;
	}

	Super::EndPlay(EndPlayReason);
}

void UCameraPhotoComponent::InitializeCaptureComponents()
{
	if (!GetOwner())
	{
		return;
	}

	PhotoRenderTarget = NewObject<UTextureRenderTarget2D>(this);
	if (PhotoRenderTarget)
	{
		PhotoRenderTarget->InitAutoFormat(PhotoResolutionWidth, PhotoResolutionHeight);
		PhotoRenderTarget->ClearColor = FLinearColor::Black;
		PhotoRenderTarget->bAutoGenerateMips = false;
		PhotoRenderTarget->UpdateResourceImmediate(true);
	}

	ThumbnailRenderTarget = NewObject<UTextureRenderTarget2D>(this);
	if (ThumbnailRenderTarget)
	{
		ThumbnailRenderTarget->InitAutoFormat(ThumbnailResolutionWidth, ThumbnailResolutionHeight);
		ThumbnailRenderTarget->ClearColor = FLinearColor::Black;
		ThumbnailRenderTarget->bAutoGenerateMips = false;
		ThumbnailRenderTarget->UpdateResourceImmediate(true);
	}

	SetupCaptureComponent();
}

void UCameraPhotoComponent::SetupCaptureComponent()
{
	if (!GetOwner())
	{
		return;
	}

	CaptureComponent = NewObject<USceneCaptureComponent2D>(GetOwner(), TEXT("PhotoCaptureComponent"));
	if (CaptureComponent)
	{
		CaptureComponent->RegisterComponent();
		CaptureComponent->bCaptureEveryFrame = false;
		CaptureComponent->bCaptureOnMovement = false;
		CaptureComponent->PrimitiveRenderMode = ESceneCapturePrimitiveRenderMode::PRM_RenderScenePrimitives;
		CaptureComponent->ShowFlags.SetTemporalAA(true);
		CaptureComponent->ShowFlags.SetMotionBlur(false);
		CaptureComponent->ShowFlags.SetGrain(false);
		CaptureComponent->CaptureSource = SCS_FinalColorLDR;
	}
}

bool UCameraPhotoComponent::CanTakePhoto() const
{
	if (bIsCapturing)
	{
		return false;
	}

	if (IsPhotoStorageFull())
	{
		return false;
	}

	if (!QuantumCamera)
	{
		if (const AActor* Owner = GetOwner())
		{
			const_cast<UCameraPhotoComponent*>(this)->QuantumCamera = Owner->FindComponentByClass<UQuantumCameraComponent>();
		}
	}

	if (QuantumCamera && !QuantumCamera->IsCameraEnabled())
	{
		return false;
	}

	UWorld* World = GetWorld();
	const float CurrentTime = World ? World->GetTimeSeconds() : 0.0f;
	if (CurrentTime - LastPhotoTime < PhotoCooldownTime)
	{
		return false;
	}

	return true;
}

bool UCameraPhotoComponent::TakePhoto(bool bUseFlash)
{
	UCameraComponent* PlayerCamera = BeginPhotoCapture();
	if (!PlayerCamera)
	{
		return false;
	}

	SyncCaptureComponentToCamera(PlayerCamera);

	const bool bShouldFlash = bUseFlash && bFlashEnabled;
	if (bShouldFlash)
	{
		TriggerFlashEffect(FlashIntensity);
	}

	PlayShutterSound();

	CaptureSceneToRenderTarget(PhotoRenderTarget);
	UTexture2D* PhotoTexture = CaptureToTexture(PhotoRenderTarget);

	CaptureSceneToRenderTarget(ThumbnailRenderTarget);
	UTexture2D* ThumbnailTexture = CaptureToTexture(ThumbnailRenderTarget);

	FCameraPhoto NewPhoto = BuildCapturedPhoto(bShouldFlash, PhotoTexture, ThumbnailTexture);
	CompletePhotoCapture(NewPhoto);

	return true;
}

UCameraComponent* UCameraPhotoComponent::BeginPhotoCapture()
{
	if (!CanTakePhoto())
	{
		return nullptr;
	}

	bIsCapturing = true;
	UWorld* World = GetWorld();
	if (!World || !GetOwner())
	{
		bIsCapturing = false;
		return nullptr;
	}

	LastPhotoTime = World->GetTimeSeconds();
	UCameraComponent* PlayerCamera = ResolvePlayerCamera();
	if (!PlayerCamera)
	{
		UE_LOG(LogTemp, Warning, TEXT("CameraPhotoComponent: No player camera found"));
		bIsCapturing = false;
	}
	else if (!AreCaptureComponentsReady())
	{
		InitializeCaptureComponents();
		if (!AreCaptureComponentsReady())
		{
			UE_LOG(LogTemp, Warning, TEXT("CameraPhotoComponent: Render capture unavailable; storing photo metadata only"));
		}
	}
	return PlayerCamera;
}

void UCameraPhotoComponent::CompletePhotoCapture(const FCameraPhoto& Photo)
{
	StorePhoto(Photo);
	OnPhotoTaken.Broadcast(Photo);
	bIsCapturing = false;
}

bool UCameraPhotoComponent::AreCaptureComponentsReady() const
{
	return PhotoRenderTarget && ThumbnailRenderTarget && CaptureComponent;
}

UCameraComponent* UCameraPhotoComponent::ResolvePlayerCamera() const
{
	AActor* Owner = GetOwner();
	if (APawn* OwnerPawn = Cast<APawn>(Owner))
	{
		if (UCameraComponent* Camera = OwnerPawn->FindComponentByClass<UCameraComponent>())
		{
			return Camera;
		}
	}

	APlayerController* PlayerController = Owner ? Cast<APlayerController>(Owner->GetInstigatorController()) : nullptr;
	APawn* Pawn = PlayerController ? PlayerController->GetPawn() : nullptr;
	return Pawn ? Pawn->FindComponentByClass<UCameraComponent>() : nullptr;
}

void UCameraPhotoComponent::SyncCaptureComponentToCamera(const UCameraComponent* PlayerCamera)
{
	if (!CaptureComponent || !PlayerCamera)
	{
		return;
	}

	CaptureComponent->SetWorldLocationAndRotation(
		PlayerCamera->GetComponentLocation(),
		PlayerCamera->GetComponentRotation());
	CaptureComponent->FOVAngle = PlayerCamera->FieldOfView;
}

void UCameraPhotoComponent::CaptureSceneToRenderTarget(UTextureRenderTarget2D* RenderTarget)
{
	if (!CaptureComponent || !RenderTarget)
	{
		return;
	}

	CaptureComponent->TextureTarget = RenderTarget;
	CaptureComponent->CaptureScene();
}

UTexture2D* UCameraPhotoComponent::CaptureToTexture(UTextureRenderTarget2D* RenderTarget)
{
	if (!RenderTarget)
	{
		return nullptr;
	}

	UTexture2D* NewTexture = UTexture2D::CreateTransient(
		RenderTarget->SizeX,
		RenderTarget->SizeY,
		PF_B8G8R8A8
	);

	if (!NewTexture)
	{
		return nullptr;
	}

	TArray<FColor> SurfaceData;
	FRenderTarget* RenderTargetResource = RenderTarget->GameThread_GetRenderTargetResource();
	if (RenderTargetResource)
	{
		RenderTargetResource->ReadPixels(SurfaceData);

		FTexturePlatformData* PlatformData = NewTexture->GetPlatformData();
		if (PlatformData && PlatformData->Mips.Num() > 0)
		{
			FTexture2DMipMap* Mip = *PlatformData->Mips.GetData();
			if (Mip)
			{
				void* TextureData = Mip->BulkData.Lock(LOCK_READ_WRITE);
				const int32 TextureDataSize = SurfaceData.Num() * sizeof(FColor);
				FMemory::Memcpy(TextureData, SurfaceData.GetData(), TextureDataSize);
				Mip->BulkData.Unlock();

				NewTexture->UpdateResource();
			}
		}
	}

	return NewTexture;
}

FCameraPhoto UCameraPhotoComponent::BuildCapturedPhoto(bool bFlashUsed, UTexture2D* PhotoTexture, UTexture2D* ThumbnailTexture)
{
	FCameraPhoto NewPhoto;
	NewPhoto.Metadata = BuildPhotoMetadata(bFlashUsed);
	NewPhoto.PhotoTexture = PhotoTexture;
	NewPhoto.ThumbnailTexture = ThumbnailTexture;

	if (bAutoDetectEvidence)
	{
		NewPhoto.Metadata.DetectedEvidenceIds = DetectEvidenceInView();
	}

	return NewPhoto;
}

void UCameraPhotoComponent::TriggerFlashEffect(float Intensity)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	OnFlashFired.Broadcast(Intensity);

	if (UHorrorAudioSubsystem* AudioSubsystem = World->GetSubsystem<UHorrorAudioSubsystem>())
	{
		if (FlashChargeSound)
		{
			AudioSubsystem->PlaySound2D(FlashChargeSound, 0.8f);
		}
	}
}

void UCameraPhotoComponent::PlayShutterSound()
{
	UWorld* World = GetWorld();
	if (!World || !ShutterSound)
	{
		return;
	}

	if (UHorrorAudioSubsystem* AudioSubsystem = World->GetSubsystem<UHorrorAudioSubsystem>())
	{
		AudioSubsystem->PlaySound2D(ShutterSound, ShutterVolume);
	}
}

TArray<FName> UCameraPhotoComponent::DetectEvidenceInView()
{
	TArray<FName> DetectedEvidence;

	const UCameraComponent* PlayerCamera = ResolvePlayerCamera();
	if (!PlayerCamera)
	{
		return DetectedEvidence;
	}

	CollectEvidenceTagsFromHits(TraceEvidenceInView(PlayerCamera), &DetectedEvidence);
	return DetectedEvidence;
}

TArray<FHitResult> UCameraPhotoComponent::TraceEvidenceInView(const UCameraComponent* PlayerCamera) const
{
	TArray<FHitResult> HitResults;
	UWorld* World = GetWorld();
	AActor* Owner = GetOwner();
	if (!World || !Owner || !PlayerCamera)
	{
		return HitResults;
	}

	FCollisionQueryParams QueryParams;
	QueryParams.bTraceComplex = false;
	QueryParams.AddIgnoredActor(Owner);

	const FVector CameraLocation = PlayerCamera->GetComponentLocation();
	const FVector TraceEnd = CameraLocation + (PlayerCamera->GetForwardVector() * EvidenceDetectionRadius);

	World->LineTraceMultiByChannel(
		HitResults,
		CameraLocation,
		TraceEnd,
		ECC_Visibility,
		QueryParams
	);

	return HitResults;
}

void UCameraPhotoComponent::CollectEvidenceTagsFromHits(const TArray<FHitResult>& HitResults, TArray<FName>* OutDetectedEvidence) const
{
	if (!OutDetectedEvidence)
	{
		return;
	}

	for (const FHitResult& Hit : HitResults)
	{
		if (AActor* HitActor = Hit.GetActor())
		{
			if (HitActor->Tags.Num() > 0)
			{
				for (const FName& Tag : HitActor->Tags)
				{
					if (Tag.ToString().Contains(TEXT("Evidence")))
					{
						OutDetectedEvidence->AddUnique(Tag);
					}
				}
			}
		}
	}
}

FCameraPhotoMetadata UCameraPhotoComponent::BuildPhotoMetadata(bool bFlashUsed)
{
	FCameraPhotoMetadata Metadata;
	Metadata.PhotoId = FGuid::NewGuid();
	Metadata.CaptureTimestamp = FDateTime::Now();
	Metadata.bFlashUsed = bFlashUsed;
	Metadata.ExposureValue = 1.0f;

	if (const UCameraComponent* PlayerCamera = ResolvePlayerCamera())
	{
		Metadata.CaptureLocation = PlayerCamera->GetComponentLocation();
		Metadata.CaptureRotation = PlayerCamera->GetComponentRotation();
	}

	if (PhotoEvidenceTag.IsValid())
	{
		Metadata.ContextTags.AddTag(PhotoEvidenceTag);
	}

	return Metadata;
}

void UCameraPhotoComponent::StorePhoto(const FCameraPhoto& Photo)
{
	if (IsPhotoStorageFull())
	{
		UE_LOG(LogTemp, Warning, TEXT("CameraPhotoComponent: Photo storage is full"));
		return;
	}

	StoredPhotos.Add(Photo);
	OnPhotoStored.Broadcast(Photo.Metadata.PhotoId);

	UE_LOG(LogTemp, Log, TEXT("CameraPhotoComponent: Photo stored. Total: %d/%d"),
		StoredPhotos.Num(), MaxPhotoCapacity);
}

FCameraPhoto UCameraPhotoComponent::GetPhoto(FGuid PhotoId) const
{
	for (const FCameraPhoto& Photo : StoredPhotos)
	{
		if (Photo.Metadata.PhotoId == PhotoId)
		{
			return Photo;
		}
	}

	return FCameraPhoto();
}

bool UCameraPhotoComponent::DeletePhoto(FGuid PhotoId)
{
	const int32 PhotoIndex = StoredPhotos.IndexOfByPredicate([PhotoId](const FCameraPhoto& Photo)
	{
		return Photo.Metadata.PhotoId == PhotoId;
	});

	if (PhotoIndex != INDEX_NONE)
	{
		StoredPhotos.RemoveAt(PhotoIndex);
		UE_LOG(LogTemp, Log, TEXT("CameraPhotoComponent: Photo deleted. Remaining: %d"), StoredPhotos.Num());
		return true;
	}

	return false;
}

void UCameraPhotoComponent::ClearAllPhotos()
{
	StoredPhotos.Empty();
	UE_LOG(LogTemp, Log, TEXT("CameraPhotoComponent: All photos cleared"));
}
