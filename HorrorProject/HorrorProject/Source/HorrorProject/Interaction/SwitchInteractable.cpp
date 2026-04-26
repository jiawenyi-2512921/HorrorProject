// Copyright Epic Games, Inc. All Rights Reserved.

#include "Interaction/SwitchInteractable.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"

ASwitchInteractable::ASwitchInteractable()
{
	PrimaryActorTick.bCanEverTick = true;

	// Create components
	SwitchBase = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SwitchBase"));
	RootComponent = SwitchBase;
	SwitchBase->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

	SwitchHandle = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SwitchHandle"));
	SwitchHandle->SetupAttachment(SwitchBase);
	SwitchHandle->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	InteractionVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractionVolume"));
	InteractionVolume->SetupAttachment(SwitchBase);
	InteractionVolume->SetBoxExtent(FVector(50.0f, 50.0f, 50.0f));
	InteractionVolume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	InteractionVolume->SetCollisionResponseToAllChannels(ECR_Ignore);
	InteractionVolume->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

	// Default settings
	InteractionPrompt = FText::FromString("Use Switch");
	bCanInteractMultipleTimes = true;
}

void ASwitchInteractable::BeginPlay()
{
	Super::BeginPlay();

	// Set initial state
	bIsSwitchOn = bStartOn;
	CurrentRotation = bIsSwitchOn ? OnRotation : OffRotation;
	TargetRotation = CurrentRotation;

	if (SwitchHandle)
	{
		SwitchHandle->SetRelativeRotation(CurrentRotation);
	}

	// Update interaction prompt based on type
	if (SwitchType == ESwitchType::OneTime)
	{
		bCanInteractMultipleTimes = false;
	}
}

bool ASwitchInteractable::CanInteract_Implementation(AActor* InstigatorActor, const FHitResult& Hit) const
{
	if (!Super::CanInteract_Implementation(InstigatorActor, Hit))
	{
		return false;
	}

	// One-time switches can't be used again
	if (SwitchType == ESwitchType::OneTime && bHasBeenInteracted)
	{
		return false;
	}

	// Can't interact while animating
	if (bIsAnimating)
	{
		return false;
	}

	return true;
}

void ASwitchInteractable::OnInteract(AActor* InstigatorActor, const FHitResult& Hit)
{
	Super::OnInteract(InstigatorActor, Hit);

	if (SwitchType == ESwitchType::Toggle)
	{
		// Toggle state
		SetSwitchState(!bIsSwitchOn);
	}
	else if (SwitchType == ESwitchType::Momentary)
	{
		// Activate temporarily
		ActivateSwitch();

		// Schedule deactivation
		if (MomentaryDuration > 0.0f)
		{
			GetWorld()->GetTimerManager().SetTimer(
				MomentaryTimer,
				this,
				&ASwitchInteractable::DeactivateSwitch,
				MomentaryDuration,
				false
			);
		}
	}
	else if (SwitchType == ESwitchType::OneTime)
	{
		// Activate once
		ActivateSwitch();
	}
}

void ASwitchInteractable::SetSwitchState(bool bNewState)
{
	if (bIsSwitchOn == bNewState)
	{
		return;
	}

	bIsSwitchOn = bNewState;

	if (bIsSwitchOn)
	{
		ActivateSwitch();
	}
	else
	{
		DeactivateSwitch();
	}
}

void ASwitchInteractable::ActivateSwitch()
{
	bIsSwitchOn = true;
	TargetRotation = OnRotation;
	bIsAnimating = true;

	// Play sound
	if (ActivateSound && GetWorld())
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), ActivateSound, GetActorLocation());
	}

	// Trigger connected actors
	TriggerConnectedActors();

	// Broadcast event
	OnSwitchActivated.Broadcast(this, true);

	// Update visuals
	UpdateVisuals();
}

void ASwitchInteractable::DeactivateSwitch()
{
	bIsSwitchOn = false;
	TargetRotation = OffRotation;
	bIsAnimating = true;

	// Play sound
	if (DeactivateSound && GetWorld())
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), DeactivateSound, GetActorLocation());
	}

	// Broadcast event
	OnSwitchActivated.Broadcast(this, false);

	// Update visuals
	UpdateVisuals();
}

void ASwitchInteractable::TriggerConnectedActors()
{
	for (AActor* ConnectedActor : ConnectedActors)
	{
		if (!ConnectedActor)
		{
			continue;
		}

		// Try to call the activation function on the connected actor
		if (UFunction* Function = ConnectedActor->FindFunction(ActivationFunctionName))
		{
			struct FActivationParams
			{
				bool bIsOn;
			};

			FActivationParams Params;
			Params.bIsOn = bIsSwitchOn;

			ConnectedActor->ProcessEvent(Function, &Params);
		}
	}
}

void ASwitchInteractable::UpdateVisuals()
{
	if (!SwitchHandle)
	{
		return;
	}

	// Animate to target rotation
	const float DeltaTime = GetWorld()->GetDeltaSeconds();
	CurrentRotation = FMath::RInterpTo(CurrentRotation, TargetRotation, DeltaTime, AnimationSpeed);

	SwitchHandle->SetRelativeRotation(CurrentRotation);

	// Check if animation is complete
	if (CurrentRotation.Equals(TargetRotation, 1.0f))
	{
		bIsAnimating = false;
	}
}

void ASwitchInteractable::SaveState(TMap<FName, bool>& OutStateMap) const
{
	Super::SaveState(OutStateMap);

	if (InteractableId != NAME_None)
	{
		const FName OnStateKey = FName(*(InteractableId.ToString() + TEXT("_On")));
		OutStateMap.Add(OnStateKey, bIsSwitchOn);
	}
}

void ASwitchInteractable::LoadState(const TMap<FName, bool>& InStateMap)
{
	Super::LoadState(InStateMap);

	if (InteractableId != NAME_None)
	{
		const FName OnStateKey = FName(*(InteractableId.ToString() + TEXT("_On")));

		if (const bool* OnState = InStateMap.Find(OnStateKey))
		{
			SetSwitchState(*OnState);
		}
	}
}
