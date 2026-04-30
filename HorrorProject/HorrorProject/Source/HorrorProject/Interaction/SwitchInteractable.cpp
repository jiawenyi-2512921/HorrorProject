// Copyright Epic Games, Inc. All Rights Reserved.

#include "Interaction/SwitchInteractable.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "UObject/FieldIterator.h"
#include "UObject/UnrealType.h"

namespace
{
const FVector SwitchInteractionExtent(50.0f, 50.0f, 50.0f);

bool CanInvokeActivationFunction(const UFunction* Function)
{
	if (!Function || Function->NumParms != 1)
	{
		return false;
	}

	const FBoolProperty* BoolParameter = nullptr;
	for (TFieldIterator<FProperty> PropertyIt(Function); PropertyIt; ++PropertyIt)
	{
		const FProperty* Property = *PropertyIt;
		if (!Property->HasAnyPropertyFlags(CPF_Parm))
		{
			continue;
		}

		if (Property->HasAnyPropertyFlags(CPF_ReturnParm) || BoolParameter)
		{
			return false;
		}

		BoolParameter = CastField<FBoolProperty>(Property);
		if (!BoolParameter)
		{
			return false;
		}
	}

	return BoolParameter != nullptr;
}
}

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
	InteractionVolume->SetBoxExtent(SwitchInteractionExtent);
	InteractionVolume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	InteractionVolume->SetCollisionResponseToAllChannels(ECR_Ignore);
	InteractionVolume->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

	// Default settings
	InteractionPrompt = NSLOCTEXT("SwitchInteractable", "UseSwitch", "使用开关");
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
			if (UWorld* World = GetWorld())
			{
				World->GetTimerManager().SetTimer(
					MomentaryTimer,
					this,
					&ASwitchInteractable::DeactivateSwitch,
					MomentaryDuration,
					false
				);
			}
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
	UWorld* World = GetWorld();
	if (ActivateSound && World)
	{
		UGameplayStatics::PlaySoundAtLocation(World, ActivateSound, GetActorLocation());
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
	UWorld* World = GetWorld();
	if (DeactivateSound && World)
	{
		UGameplayStatics::PlaySoundAtLocation(World, DeactivateSound, GetActorLocation());
	}

	// Broadcast event
	OnSwitchActivated.Broadcast(this, false);

	// Update visuals
	UpdateVisuals();
}

void ASwitchInteractable::TriggerConnectedActors()
{
	if (ActivationFunctionName.IsNone())
	{
		return;
	}

	for (AActor* ConnectedActor : ConnectedActors)
	{
		if (!ConnectedActor)
		{
			continue;
		}

		// Try to call the activation function on the connected actor
		if (UFunction* Function = ConnectedActor->FindFunction(ActivationFunctionName))
		{
			if (!CanInvokeActivationFunction(Function))
			{
				UE_LOG(LogTemp, Warning, TEXT("Switch activation skipped for %s: function %s must take exactly one bool parameter."),
					*GetNameSafe(ConnectedActor), *ActivationFunctionName.ToString());
				continue;
			}

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
	UWorld* World = GetWorld();
	if (!SwitchHandle || !World)
	{
		return;
	}

	// Animate to target rotation
	const float DeltaTime = World->GetDeltaSeconds();
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
