// Copyright Epic Games, Inc. All Rights Reserved.

#include "Audio/Components/FootstepAudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "GameFramework/Character.h"
#include "Components/CapsuleComponent.h"
#include "PhysicalMaterials/PhysicalMaterial.h"

UFootstepAudioComponent::UFootstepAudioComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
}

void UFootstepAudioComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UFootstepAudioComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bAutoPlayFootsteps || CurrentSpeed < 10.0f)
	{
		return;
	}

	TimeSinceLastFootstep += DeltaTime;

	float CurrentInterval = GetCurrentFootstepInterval();
	if (TimeSinceLastFootstep >= CurrentInterval)
	{
		DetectSurfaceType();
		PlayFootstep(bIsLeftFoot);
		bIsLeftFoot = !bIsLeftFoot;
		TimeSinceLastFootstep = 0.0f;
	}
}

void UFootstepAudioComponent::PlayFootstep(bool bIsLeftFoot)
{
	const FFootstepSoundSet* SoundSet = SurfaceSounds.Find(CurrentSurface);
	if (!SoundSet)
	{
		return;
	}

	const TArray<TObjectPtr<USoundBase>>* SoundsToPlay = nullptr;

	if (bIsCrouching && SoundSet->CrouchSounds.Num() > 0)
	{
		SoundsToPlay = &SoundSet->CrouchSounds;
	}
	else if (CurrentSpeed >= RunSpeedThreshold && SoundSet->RunSounds.Num() > 0)
	{
		SoundsToPlay = &SoundSet->RunSounds;
	}
	else if (SoundSet->WalkSounds.Num() > 0)
	{
		SoundsToPlay = &SoundSet->WalkSounds;
	}

	if (SoundsToPlay && SoundsToPlay->Num() > 0)
	{
		PlaySound(*SoundsToPlay, GetCurrentVolumeMultiplier());
	}
}

void UFootstepAudioComponent::PlayJumpSound()
{
	const FFootstepSoundSet* SoundSet = SurfaceSounds.Find(CurrentSurface);
	if (SoundSet && SoundSet->JumpSounds.Num() > 0)
	{
		PlaySound(SoundSet->JumpSounds, 0.8f);
	}
}

void UFootstepAudioComponent::PlayLandSound(float ImpactIntensity)
{
	const FFootstepSoundSet* SoundSet = SurfaceSounds.Find(CurrentSurface);
	if (SoundSet && SoundSet->LandSounds.Num() > 0)
	{
		float VolumeMultiplier = FMath::Clamp(ImpactIntensity * 0.5f, 0.5f, 1.5f);
		PlaySound(SoundSet->LandSounds, VolumeMultiplier);
	}
}

void UFootstepAudioComponent::SetMovementSpeed(float Speed)
{
	CurrentSpeed = Speed;
}

void UFootstepAudioComponent::SetCrouching(bool bIsCrouchingNow)
{
	bIsCrouching = bIsCrouchingNow;
}

void UFootstepAudioComponent::RegisterSurfaceType(EFootstepSurfaceType SurfaceType, const FFootstepSoundSet& SoundSet)
{
	SurfaceSounds.Add(SurfaceType, SoundSet);
}

void UFootstepAudioComponent::SetCurrentSurface(EFootstepSurfaceType SurfaceType)
{
	CurrentSurface = SurfaceType;
}

EFootstepSurfaceType UFootstepAudioComponent::DetectSurfaceType()
{
	AActor* Owner = GetOwner();
	if (!Owner || !GetWorld())
	{
		return CurrentSurface;
	}

	FVector StartLocation = Owner->GetActorLocation();
	FVector EndLocation = StartLocation - FVector(0.0f, 0.0f, TraceDistance);

	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(Owner);

	bool bHit = GetWorld()->LineTraceSingleByChannel(
		HitResult,
		StartLocation,
		EndLocation,
		ECC_Visibility,
		QueryParams
	);

	if (bHit && HitResult.PhysMaterial.IsValid())
	{
		UPhysicalMaterial* PhysMat = HitResult.PhysMaterial.Get();

		if (PhysMat->GetName().Contains(TEXT("Wood")))
		{
			CurrentSurface = EFootstepSurfaceType::Wood;
		}
		else if (PhysMat->GetName().Contains(TEXT("Metal")))
		{
			CurrentSurface = EFootstepSurfaceType::Metal;
		}
		else if (PhysMat->GetName().Contains(TEXT("Water")))
		{
			CurrentSurface = EFootstepSurfaceType::Water;
		}
		else if (PhysMat->GetName().Contains(TEXT("Grass")))
		{
			CurrentSurface = EFootstepSurfaceType::Grass;
		}
		else if (PhysMat->GetName().Contains(TEXT("Gravel")))
		{
			CurrentSurface = EFootstepSurfaceType::Gravel;
		}
		else if (PhysMat->GetName().Contains(TEXT("Carpet")))
		{
			CurrentSurface = EFootstepSurfaceType::Carpet;
		}
		else if (PhysMat->GetName().Contains(TEXT("Mud")))
		{
			CurrentSurface = EFootstepSurfaceType::Mud;
		}
		else
		{
			CurrentSurface = EFootstepSurfaceType::Concrete;
		}
	}

	return CurrentSurface;
}

void UFootstepAudioComponent::PlaySound(const TArray<TObjectPtr<USoundBase>>& Sounds, float VolumeMultiplier)
{
	if (Sounds.Num() == 0 || !GetWorld())
	{
		return;
	}

	int32 RandomIndex = FMath::RandRange(0, Sounds.Num() - 1);
	USoundBase* Sound = Sounds[RandomIndex];

	if (!Sound)
	{
		return;
	}

	const FFootstepSoundSet* SoundSet = SurfaceSounds.Find(CurrentSurface);
	float BaseVolume = SoundSet ? SoundSet->BaseVolume : 0.7f;
	float PitchVar = SoundSet ? SoundSet->PitchVariation : 0.1f;

	float FinalVolume = BaseVolume * VolumeMultiplier;
	float FinalPitch = 1.0f + FMath::RandRange(-PitchVar, PitchVar);

	UGameplayStatics::PlaySoundAtLocation(
		GetWorld(),
		Sound,
		GetOwner()->GetActorLocation(),
		FinalVolume,
		FinalPitch
	);
}

float UFootstepAudioComponent::GetCurrentFootstepInterval() const
{
	if (bIsCrouching)
	{
		return CrouchFootstepInterval;
	}
	else if (CurrentSpeed >= RunSpeedThreshold)
	{
		return RunFootstepInterval;
	}
	else
	{
		return FootstepInterval;
	}
}

float UFootstepAudioComponent::GetCurrentVolumeMultiplier() const
{
	if (bIsCrouching)
	{
		return VolumeMultiplierCrouch;
	}
	else if (CurrentSpeed >= RunSpeedThreshold)
	{
		return VolumeMultiplierRun;
	}
	else
	{
		return VolumeMultiplierWalk;
	}
}
