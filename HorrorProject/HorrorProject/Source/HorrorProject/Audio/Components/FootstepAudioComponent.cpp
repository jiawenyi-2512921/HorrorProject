// Copyright Epic Games, Inc. All Rights Reserved.

#include "Audio/Components/FootstepAudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "GameFramework/Character.h"
#include "Components/CapsuleComponent.h"
#include "PhysicalMaterials/PhysicalMaterial.h"

namespace
{
	struct FFootstepSurfaceNameMatch
	{
		const TCHAR* Token;
		EFootstepSurfaceType SurfaceType;
	};

	const FFootstepSurfaceNameMatch SurfaceNameMatches[] =
	{
		{ TEXT("Wood"), EFootstepSurfaceType::Wood },
		{ TEXT("Metal"), EFootstepSurfaceType::Metal },
		{ TEXT("Water"), EFootstepSurfaceType::Water },
		{ TEXT("Grass"), EFootstepSurfaceType::Grass },
		{ TEXT("Gravel"), EFootstepSurfaceType::Gravel },
		{ TEXT("Carpet"), EFootstepSurfaceType::Carpet },
		{ TEXT("Mud"), EFootstepSurfaceType::Mud },
	};

	EFootstepSurfaceType ResolveSurfaceTypeFromMaterialName(const FString& MaterialName)
	{
		for (const FFootstepSurfaceNameMatch& Match : SurfaceNameMatches)
		{
			if (MaterialName.Contains(Match.Token))
			{
				return Match.SurfaceType;
			}
		}

		return EFootstepSurfaceType::Concrete;
	}
}

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

void UFootstepAudioComponent::PlayFootstep(bool bInIsLeftFoot)
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
	UWorld* World = GetWorld();
	if (!Owner || !World)
	{
		return CurrentSurface;
	}

	FVector StartLocation = Owner->GetActorLocation();
	FVector EndLocation = StartLocation - FVector(0.0f, 0.0f, TraceDistance);

	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(Owner);

	bool bHit = World->LineTraceSingleByChannel(
		HitResult,
		StartLocation,
		EndLocation,
		ECC_Visibility,
		QueryParams
	);

	if (bHit && HitResult.PhysMaterial.IsValid())
	{
		UPhysicalMaterial* PhysMat = HitResult.PhysMaterial.Get();
		CurrentSurface = ResolveSurfaceTypeFromMaterialName(PhysMat->GetName());
	}

	return CurrentSurface;
}

void UFootstepAudioComponent::PlaySound(const TArray<TObjectPtr<USoundBase>>& Sounds, float VolumeMultiplier)
{
	UWorld* World = GetWorld();
	AActor* Owner = GetOwner();
	if (Sounds.Num() == 0 || !World || !Owner)
	{
		return;
	}

	int32 RandomIndex = FMath::RandRange(0, Sounds.Num() - 1);
	USoundBase* Sound = (Sounds.GetData() + RandomIndex)->Get();

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
		World,
		Sound,
		Owner->GetActorLocation(),
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
