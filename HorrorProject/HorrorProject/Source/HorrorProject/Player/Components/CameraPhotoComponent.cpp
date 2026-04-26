// Copyright Epic Games, Inc. All Rights Reserved.

#include "CameraPhotoComponent.h"
#include "QuantumCameraComponent.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Engine/Texture2D.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/CameraComponent.h"
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

	QuantumCamera = GetOwner()->FindComponentByClass<UQuantumCameraComponent>();
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
		CaptureComponent->PrimitiveRenderMode = ESceneCapturePrimitiveRenderMode::PRM_UseShowOnlyList;
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

	if (QuantumCamera && !QuantumCamera->IsCameraEnabled())
	{
		return false;
	}

	const float CurrentTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
	if (CurrentTime - LastPhotoTime < PhotoCooldownTime)
	{
		return false;
	}

	return true;
}

bool UCameraPhotoComponent::TakePhoto(bool bUseFlash)
{
	if (!CanTakePhoto())
	{
		return false;
	}

	if (!PhotoRenderTarget || !ThumbnailRenderTarget || !CaptureComponent)
	{
		UE_LOG(LogTemp, Error, TEXT("CameraPhotoComponent: Capture components not initialized"));
		return false;
	}

	bIsCapturing = true;
	LastPhotoTime = GetWorld()->GetTimeSeconds();

	UCameraComponent* PlayerCamera = nullptr;
	if (APlayerController* PC = Cast<APlayerController>(GetOwner()->GetInstigatorController()))
	{
		if (APawn* Pawn = PC->GetPawn())
		{
			PlayerCamera = Pawn->FindComponentByClass<UCameraComponent>();
		}
	}

	if (!PlayerCamera)
	{
		UE_LOG(LogTemp, Warning, TEXT("CameraPhotoComponent: No player camera found"));
		bIsCapturing = false;
		return false;
	}

	CaptureComponent->SetWorldLocationAndRotation(
		PlayerCamera->GetComponentLocation(),
		PlayerCamera->GetComponentRotation()
	);
	CaptureComponent->FOVAngle = PlayerCamera->FieldOfView;

	const bool bShouldFlash = bUseFlash && bFlashEnabled;
	if (bShouldFlash)
	{
		TriggerFlashEffect(FlashIntensity);
	}

	PlayShutterSound();

	CaptureComponent->TextureTarget = PhotoRenderTarget;
	CaptureComponent->CaptureScene();

	UTexture2D* PhotoTexture = CaptureToTexture(PhotoRenderTarget);

	CaptureComponent->TextureTarget = ThumbnailRenderTarget;
	CaptureComponent->CaptureScene();

	UTexture2D* ThumbnailTexture = CaptureToTexture(ThumbnailRenderTarget);

	FCameraPhoto NewPhoto;
	NewPhoto.Metadata = BuildPhotoMetadata(bShouldFlash);
	NewPhoto.PhotoTexture = PhotoTexture;
	NewPhoto.ThumbnailTexture = ThumbnailTexture;

	if (bAutoDetectEvidence)
	{
		NewPhoto.Metadata.DetectedEvidenceIds = DetectEvidenceInView();
	}

	StorePhoto(NewPhoto);

	OnPhotoTaken.Broadcast(NewPhoto);

	bIsCapturing = false;

	return true;
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

		void* TextureData = NewTexture->GetPlatformData()->Mips[0].BulkData.Lock(LOCK_READ_WRITE);
		const int32 TextureDataSize = SurfaceData.Num() * sizeof(FColor);
		FMemory::Memcpy(TextureData, SurfaceData.GetData(), TextureDataSize);
		NewTexture->GetPlatformData()->Mips[0].BulkData.Unlock();

		NewTexture->UpdateResource();
	}

	return NewTexture;
}

void UCameraPhotoComponent::TriggerFlashEffect(float Intensity)
{
	if (!GetWorld())
	{
		return;
	}

	OnFlashFired.Broadcast(Intensity);

	if (UHorrorAudioSubsystem* AudioSubsystem = GetWorld()->GetSubsystem<UHorrorAudioSubsystem>())
	{
		if (FlashChargeSound)
		{
			AudioSubsystem->PlaySound2D(FlashChargeSound, 0.8f);
		}
	}
}

void UCameraPhotoComponent::PlayShutterSound()
{
	if (!GetWorld() || !ShutterSound)
	{
		return;
	}

	if (UHorrorAudioSubsystem* AudioSubsystem = GetWorld()->GetSubsystem<UHorrorAudioSubsystem>())
	{
		AudioSubsystem->PlaySound2D(ShutterSound, ShutterVolume);
	}
}

TArray<FName> UCameraPhotoComponent::DetectEvidenceInView()
{
	TArray<FName> DetectedEvidence;

	if (!GetWorld())
	{
		return DetectedEvidence;
	}

	FVector CameraLocation = FVector::ZeroVector;
	FRotator CameraRotation = FRotator::ZeroRotator;

	if (APlayerController* PC = Cast<APlayerController>(GetOwner()->GetInstigatorController()))
	{
		if (APawn* Pawn = PC->GetPawn())
		{
			if (UCameraComponent* Camera = Pawn->FindComponentByClass<UCameraComponent>())
			{
				CameraLocation = Camera->GetComponentLocation();
				CameraRotation = Camera->GetComponentRotation();
			}
		}
	}

	TArray<FHitResult> HitResults;
	FCollisionQueryParams QueryParams;
	QueryParams.bTraceComplex = false;
	QueryParams.AddIgnoredActor(GetOwner());

	const FVector ForwardVector = CameraRotation.Vector();
	const FVector TraceEnd = CameraLocation + (ForwardVector * EvidenceDetectionRadius);

	GetWorld()->LineTraceMultiByChannel(
		HitResults,
		CameraLocation,
		TraceEnd,
		ECC_Visibility,
		QueryParams
	);

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
						DetectedEvidence.AddUnique(Tag);
					}
				}
			}
		}
	}

	return DetectedEvidence;
}

FCameraPhotoMetadata UCameraPhotoComponent::BuildPhotoMetadata(bool bFlashUsed)
{
	FCameraPhotoMetadata Metadata;
	Metadata.bFlashUsed = bFlashUsed;
	Metadata.ExposureValue = 1.0f;

	if (APlayerController* PC = Cast<APlayerController>(GetOwner()->GetInstigatorController()))
	{
		if (APawn* Pawn = PC->GetPawn())
		{
			if (UCameraComponent* Camera = Pawn->FindComponentByClass<UCameraComponent>())
			{
				Metadata.CaptureLocation = Camera->GetComponentLocation();
				Metadata.CaptureRotation = Camera->GetComponentRotation();
			}
		}
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
	for (int32 i = 0; i < StoredPhotos.Num(); ++i)
	{
		if (StoredPhotos[i].Metadata.PhotoId == PhotoId)
		{
			StoredPhotos.RemoveAt(i);
			UE_LOG(LogTemp, Log, TEXT("CameraPhotoComponent: Photo deleted. Remaining: %d"), StoredPhotos.Num());
			return true;
		}
	}

	return false;
}

void UCameraPhotoComponent::ClearAllPhotos()
{
	StoredPhotos.Empty();
	UE_LOG(LogTemp, Log, TEXT("CameraPhotoComponent: All photos cleared"));
}
