// Copyright Epic Games, Inc. All Rights Reserved.

#include "Interaction/DoorInteractable.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Curves/CurveFloat.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"

ADoorInteractable::ADoorInteractable()
{
	PrimaryActorTick.bCanEverTick = true;

	// Create components
	DoorRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DoorRoot"));
	RootComponent = DoorRoot;

	DoorFrame = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DoorFrame"));
	DoorFrame->SetupAttachment(DoorRoot);
	DoorFrame->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

	DoorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DoorMesh"));
	DoorMesh->SetupAttachment(DoorRoot);
	DoorMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

	InteractionVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractionVolume"));
	InteractionVolume->SetupAttachment(DoorRoot);
	InteractionVolume->SetBoxExtent(FVector(100.0f, 100.0f, 100.0f));
	InteractionVolume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	InteractionVolume->SetCollisionResponseToAllChannels(ECR_Ignore);
	InteractionVolume->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

	// Default settings
	InteractionPrompt = FText::FromString("Open Door");
	bCanInteractMultipleTimes = true;
}

void ADoorInteractable::BeginPlay()
{
	Super::BeginPlay();

	InitialDoorRotation = DoorMesh->GetRelativeRotation();

	if (bStartLocked)
	{
		DoorState = EDoorState::Locked;
	}
}

void ADoorInteractable::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Update door rotation animation
	if (DoorState == EDoorState::Opening || DoorState == EDoorState::Closing)
	{
		UpdateDoorRotation(DeltaTime);
	}

	// Handle auto-close
	if (bAutoClose && DoorState == EDoorState::Open)
	{
		AutoCloseTimer += DeltaTime;
		if (AutoCloseTimer >= AutoCloseDelay)
		{
			StartClosing();
		}
	}
}

bool ADoorInteractable::CanInteract_Implementation(AActor* InstigatorActor, const FHitResult& Hit) const
{
	if (!Super::CanInteract_Implementation(InstigatorActor, Hit))
	{
		return false;
	}

	// Can't interact while animating
	if (DoorState == EDoorState::Opening || DoorState == EDoorState::Closing)
	{
		return false;
	}

	return true;
}

void ADoorInteractable::OnInteract(AActor* InstigatorActor, const FHitResult& Hit)
{
	Super::OnInteract(InstigatorActor, Hit);

	if (DoorState == EDoorState::Locked)
	{
		// Play locked sound
		if (LockedSound && GetWorld())
		{
			UGameplayStatics::PlaySoundAtLocation(GetWorld(), LockedSound, GetActorLocation());
		}
		return;
	}

	if (DoorState == EDoorState::Closed)
	{
		StartOpening();
	}
	else if (DoorState == EDoorState::Open)
	{
		StartClosing();
	}
}

void ADoorInteractable::SetLocked(bool bLocked)
{
	if (bLocked)
	{
		DoorState = EDoorState::Locked;
	}
	else if (DoorState == EDoorState::Locked)
	{
		DoorState = EDoorState::Closed;
	}
}

void ADoorInteractable::StartOpening()
{
	DoorState = EDoorState::Opening;
	TargetRotationAlpha = 1.0f;
	AutoCloseTimer = 0.0f;

	if (OpenSound && GetWorld())
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), OpenSound, GetActorLocation());
	}
}

void ADoorInteractable::StartClosing()
{
	DoorState = EDoorState::Closing;
	TargetRotationAlpha = 0.0f;
	AutoCloseTimer = 0.0f;

	if (CloseSound && GetWorld())
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), CloseSound, GetActorLocation());
	}
}

void ADoorInteractable::UpdateDoorRotation(float DeltaTime)
{
	const float Step = DeltaTime * OpenSpeed;

	if (DoorState == EDoorState::Opening)
	{
		CurrentRotationAlpha = FMath::FInterpConstantTo(CurrentRotationAlpha, TargetRotationAlpha, DeltaTime, OpenSpeed);

		if (FMath::IsNearlyEqual(CurrentRotationAlpha, TargetRotationAlpha, 0.01f))
		{
			CurrentRotationAlpha = TargetRotationAlpha;
			DoorState = EDoorState::Open;
		}
	}
	else if (DoorState == EDoorState::Closing)
	{
		CurrentRotationAlpha = FMath::FInterpConstantTo(CurrentRotationAlpha, TargetRotationAlpha, DeltaTime, OpenSpeed);

		if (FMath::IsNearlyEqual(CurrentRotationAlpha, TargetRotationAlpha, 0.01f))
		{
			CurrentRotationAlpha = TargetRotationAlpha;
			DoorState = EDoorState::Closed;
		}
	}

	// Apply rotation with curve
	const float CurveValue = GetCurveValue(CurrentRotationAlpha);
	const float CurrentAngle = CurveValue * OpenAngle;

	FRotator NewRotation = InitialDoorRotation;
	NewRotation.Yaw += CurrentAngle;
	DoorMesh->SetRelativeRotation(NewRotation);
}

float ADoorInteractable::GetCurveValue(float Alpha) const
{
	if (OpenCurve)
	{
		return OpenCurve->GetFloatValue(Alpha);
	}
	return Alpha; // Linear fallback
}

void ADoorInteractable::SaveState(TMap<FName, bool>& OutStateMap) const
{
	Super::SaveState(OutStateMap);

	if (InteractableId != NAME_None)
	{
		const FName OpenStateKey = FName(*(InteractableId.ToString() + TEXT("_Open")));
		const FName LockedStateKey = FName(*(InteractableId.ToString() + TEXT("_Locked")));

		OutStateMap.Add(OpenStateKey, DoorState == EDoorState::Open || DoorState == EDoorState::Opening);
		OutStateMap.Add(LockedStateKey, DoorState == EDoorState::Locked);
	}
}

void ADoorInteractable::LoadState(const TMap<FName, bool>& InStateMap)
{
	Super::LoadState(InStateMap);

	if (InteractableId != NAME_None)
	{
		const FName OpenStateKey = FName(*(InteractableId.ToString() + TEXT("_Open")));
		const FName LockedStateKey = FName(*(InteractableId.ToString() + TEXT("_Locked")));

		if (const bool* LockedState = InStateMap.Find(LockedStateKey))
		{
			if (*LockedState)
			{
				DoorState = EDoorState::Locked;
				return;
			}
		}

		if (const bool* OpenState = InStateMap.Find(OpenStateKey))
		{
			if (*OpenState)
			{
				DoorState = EDoorState::Open;
				CurrentRotationAlpha = 1.0f;
				TargetRotationAlpha = 1.0f;

				FRotator NewRotation = InitialDoorRotation;
				NewRotation.Yaw += OpenAngle;
				DoorMesh->SetRelativeRotation(NewRotation);
			}
			else
			{
				DoorState = EDoorState::Closed;
				CurrentRotationAlpha = 0.0f;
				TargetRotationAlpha = 0.0f;
			}
		}
	}
}
