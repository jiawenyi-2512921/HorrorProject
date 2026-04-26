// Copyright Epic Games, Inc. All Rights Reserved.

#include "ScreenEffectManager.h"
#include "Camera/PlayerCameraManager.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"

UScreenEffectManager::UScreenEffectManager()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PostUpdateWork;
}

void UScreenEffectManager::BeginPlay()
{
	Super::BeginPlay();

	// Get camera manager
	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Warning, TEXT("ScreenEffectManager: World is null in BeginPlay"));
		return;
	}

	if (APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0))
	{
		CameraManager = PC->PlayerCameraManager;
	}
}

void UScreenEffectManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	UpdateScreenEffects(DeltaTime);
}

void UScreenEffectManager::ApplyCameraShake(ECameraShakeType ShakeType, float Intensity)
{
	if (!bEnableScreenEffects)
	{
		return;
	}

	if (!CameraManager)
	{
		UE_LOG(LogTemp, Warning, TEXT("ScreenEffectManager: CameraManager is null in ApplyCameraShake"));
		return;
	}

	if (CameraShakeClasses.Contains(ShakeType) && CameraShakeClasses[ShakeType])
	{
		float ClampedIntensity = FMath::Clamp(Intensity, 0.0f, MaxShakeIntensity);
		ApplyShakeInternal(CameraShakeClasses[ShakeType], ClampedIntensity);
	}
}

void UScreenEffectManager::ApplyCustomCameraShake(const FCameraShakeSettings& Settings)
{
	if (!bEnableScreenEffects)
	{
		return;
	}

	if (!CameraManager)
	{
		UE_LOG(LogTemp, Warning, TEXT("ScreenEffectManager: CameraManager is null in ApplyCustomCameraShake"));
		return;
	}

	// Apply generic shake with custom settings
	ApplyCameraShake(ECameraShakeType::Impact, Settings.Intensity);
}

void UScreenEffectManager::StopAllCameraShakes()
{
	if (!CameraManager)
	{
		UE_LOG(LogTemp, Warning, TEXT("ScreenEffectManager: CameraManager is null in StopAllCameraShakes"));
		return;
	}

	CameraManager->StopAllCameraShakes(true);
}

void UScreenEffectManager::ApplyImpactFeedback(FVector ImpactLocation, float Strength)
{
	if (!bEnableScreenEffects)
	{
		return;
	}

	// Calculate distance-based intensity
	if (AActor* Owner = GetOwner())
	{
		float Distance = FVector::Dist(Owner->GetActorLocation(), ImpactLocation);
		float DistanceFactor = FMath::Clamp(1.0f - (Distance / 1000.0f), 0.0f, 1.0f);
		float FinalIntensity = Strength * DistanceFactor;

		ApplyCameraShake(ECameraShakeType::Impact, FinalIntensity);
	}
}

void UScreenEffectManager::ApplyPressureFeedback(float PressureLevel)
{
	CurrentPressure = FMath::Clamp(PressureLevel, 0.0f, 1.0f);

	if (CurrentPressure > 0.7f)
	{
		// High pressure shake
		float ShakeIntensity = (CurrentPressure - 0.7f) * 3.0f;
		ApplyCameraShake(ECameraShakeType::Pressure, ShakeIntensity);
	}

	// Update screen distortion
	SetScreenDistortion(CurrentPressure * 0.3f);
}

void UScreenEffectManager::ApplyDrowningFeedback(float OxygenPercent)
{
	CurrentOxygen = FMath::Clamp(OxygenPercent, 0.0f, 100.0f);
	float DrowningLevel = 1.0f - (CurrentOxygen / 100.0f);

	if (DrowningLevel > 0.3f)
	{
		// Apply drowning shake
		ApplyCameraShake(ECameraShakeType::Drowning, DrowningLevel);

		// Apply screen blur
		SetScreenBlur(DrowningLevel * 0.5f);
	}
}

void UScreenEffectManager::ApplyFearFeedback(float FearLevel)
{
	CurrentFear = FMath::Clamp(FearLevel, 0.0f, 1.0f);

	if (CurrentFear > 0.5f)
	{
		// Apply fear shake
		float ShakeIntensity = (CurrentFear - 0.5f) * 2.0f;
		ApplyCameraShake(ECameraShakeType::Fear, ShakeIntensity);

		// Apply screen distortion
		SetScreenDistortion(CurrentFear * 0.2f);
	}
}

void UScreenEffectManager::ApplyDeathFeedback()
{
	// Apply death shake
	ApplyCameraShake(ECameraShakeType::Death, 1.5f);

	// Apply maximum blur
	SetScreenBlur(1.0f);
}

void UScreenEffectManager::SetScreenDistortion(float Amount)
{
	DistortionAmount = FMath::Clamp(Amount, 0.0f, 1.0f);
}

void UScreenEffectManager::SetScreenBlur(float Amount)
{
	BlurAmount = FMath::Clamp(Amount, 0.0f, 1.0f);
}

void UScreenEffectManager::ClearAllScreenEffects()
{
	StopAllCameraShakes();
	SetScreenDistortion(0.0f);
	SetScreenBlur(0.0f);

	CurrentPressure = 0.0f;
	CurrentFear = 0.0f;
	CurrentOxygen = 100.0f;
}

void UScreenEffectManager::UpdateScreenEffects(float DeltaTime)
{
	// Gradually reduce effects over time
	if (DistortionAmount > 0.0f)
	{
		DistortionAmount = FMath::Max(0.0f, DistortionAmount - DeltaTime * 0.5f);
	}

	if (BlurAmount > 0.0f && CurrentOxygen >= 50.0f)
	{
		BlurAmount = FMath::Max(0.0f, BlurAmount - DeltaTime * 0.3f);
	}
}

void UScreenEffectManager::ApplyShakeInternal(TSubclassOf<UCameraShakeBase> ShakeClass, float Scale)
{
	if (!CameraManager)
	{
		UE_LOG(LogTemp, Warning, TEXT("ScreenEffectManager: CameraManager is null in ApplyShakeInternal"));
		return;
	}

	if (!ShakeClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("ScreenEffectManager: ShakeClass is null in ApplyShakeInternal"));
		return;
	}

	CameraManager->StartCameraShake(ShakeClass, Scale);
}
