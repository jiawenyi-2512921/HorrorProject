// Copyright Epic Games, Inc. All Rights Reserved.

#include "Interaction/DoorInteractable.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Curves/CurveFloat.h"
#include "Engine/Engine.h"
#include "Game/HorrorEventBusSubsystem.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Player/HorrorPlayerController.h"
#include "Sound/SoundBase.h"

namespace
{
	const FName DoorOpenedEventTagName(TEXT("Event.Interaction.Door.Opened"));
}

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
	InteractionPrompt = NSLOCTEXT("DoorInteractable", "OpenDoor", "开门");
	bCanInteractMultipleTimes = true;
}

void ADoorInteractable::BeginPlay()
{
	Super::BeginPlay();

	InitialDoorRotation = DoorMesh->GetRelativeRotation();

	if (ShouldGateWithPassword())
	{
		DoorState = EDoorState::Locked;
	}
	else if (bStartLocked)
	{
		DoorState = EDoorState::Locked;
	}

	UpdateDoorCollision();
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
		if (ShouldGateWithPassword())
		{
			RequestPasswordEntry(InstigatorActor);
			return;
		}

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
	if (bLocked && RequiresPassword())
	{
		bPasswordUnlocked = false;
	}

	if (bLocked)
	{
		DoorState = EDoorState::Locked;
	}
	else if (DoorState == EDoorState::Locked)
	{
		DoorState = EDoorState::Closed;
		if (RequiresPassword())
		{
			bPasswordUnlocked = true;
		}
	}

	UpdateDoorCollision();
}

void ADoorInteractable::ConfigurePassword(const FString& InPassword, const FText& InPasswordHint)
{
	RequiredPassword = InPassword;
	PasswordHint = InPasswordHint;
	bRequiresPassword = !RequiredPassword.IsEmpty();
	bPasswordUnlocked = !bRequiresPassword;

	if (bRequiresPassword)
	{
		DoorState = EDoorState::Locked;
	}
	else if (DoorState == EDoorState::Locked)
	{
		DoorState = EDoorState::Closed;
	}

	UpdateDoorCollision();
}

bool ADoorInteractable::SubmitPassword(AActor* InstigatorActor, const FString& SubmittedPassword)
{
	if (!RequiresPassword() || !InstigatorActor)
	{
		return false;
	}

	if (SubmittedPassword.TrimStartAndEnd() != RequiredPassword)
	{
		if (LockedSound && GetWorld())
		{
			UGameplayStatics::PlaySoundAtLocation(GetWorld(), LockedSound, GetActorLocation());
		}

		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				2.0f,
				FColor::Red,
				TEXT("门禁码错误。"));
		}
		return false;
	}

	bPasswordUnlocked = true;
	if (DoorState == EDoorState::Locked)
	{
		DoorState = EDoorState::Closed;
	}

	if (DoorState == EDoorState::Closed)
	{
		StartOpening();
	}

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			-1,
			2.0f,
			FColor::Green,
			TEXT("门禁码正确。"));
	}
	return true;
}

void ADoorInteractable::StartOpening()
{
	DoorState = EDoorState::Opening;
	TargetRotationAlpha = 1.0f;
	AutoCloseTimer = 0.0f;
	UpdateDoorCollision();
	PublishDoorOpenedEvent();

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
	UpdateDoorCollision();

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
			UpdateDoorCollision();
		}
	}
	else if (DoorState == EDoorState::Closing)
	{
		CurrentRotationAlpha = FMath::FInterpConstantTo(CurrentRotationAlpha, TargetRotationAlpha, DeltaTime, OpenSpeed);

		if (FMath::IsNearlyEqual(CurrentRotationAlpha, TargetRotationAlpha, 0.01f))
		{
			CurrentRotationAlpha = TargetRotationAlpha;
			DoorState = EDoorState::Closed;
			UpdateDoorCollision();
		}
	}

	// Apply rotation with curve
	const float CurveValue = GetCurveValue(CurrentRotationAlpha);
	FRotator NewRotation = InitialDoorRotation;
	NewRotation.Pitch += OpenRotationOffset.Pitch * CurveValue;
	NewRotation.Yaw += OpenRotationOffset.Yaw * CurveValue;
	NewRotation.Roll += OpenRotationOffset.Roll * CurveValue;
	DoorMesh->SetRelativeRotation(NewRotation);
}

void ADoorInteractable::UpdateDoorCollision() const
{
	if (!DoorMesh)
	{
		return;
	}

	const bool bShouldBlockPawn = DoorState == EDoorState::Closed || DoorState == EDoorState::Closing || DoorState == EDoorState::Locked;
	DoorMesh->SetCollisionEnabled(bShouldBlockPawn ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision);
}

void ADoorInteractable::RequestPasswordEntry(AActor* InstigatorActor) const
{
	AHorrorPlayerController* HorrorPlayerController = Cast<AHorrorPlayerController>(InstigatorActor);
	if (!HorrorPlayerController)
	{
		if (const APawn* InstigatorPawn = Cast<APawn>(InstigatorActor))
		{
			HorrorPlayerController = Cast<AHorrorPlayerController>(InstigatorPawn->GetController());
		}
	}

	if (HorrorPlayerController)
	{
		HorrorPlayerController->BeginDoorPasswordEntry(const_cast<ADoorInteractable*>(this));
		return;
	}

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			-1,
			3.0f,
			FColor::Yellow,
			TEXT("这扇门需要门禁码。"));
	}
}

bool ADoorInteractable::ShouldGateWithPassword() const
{
	return RequiresPassword() && !bPasswordUnlocked;
}

FName ADoorInteractable::ResolveDoorEventSourceId() const
{
	return InteractableId.IsNone() ? GetFName() : InteractableId;
}

void ADoorInteractable::PublishDoorOpenedEvent()
{
	UHorrorEventBusSubsystem* EventBus = GetEventBus();
	if (!EventBus)
	{
		return;
	}

	const FGameplayTag DoorOpenedTag = FGameplayTag::RequestGameplayTag(DoorOpenedEventTagName, false);
	if (!DoorOpenedTag.IsValid())
	{
		return;
	}

	EventBus->Publish(DoorOpenedTag, ResolveDoorEventSourceId(), FGameplayTag::EmptyTag, this);
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
		OutStateMap.Add(FName(*(InteractableId.ToString() + TEXT("_PasswordUnlocked"))), bPasswordUnlocked);
	}
}

void ADoorInteractable::LoadState(const TMap<FName, bool>& InStateMap)
{
	Super::LoadState(InStateMap);

	if (InteractableId != NAME_None)
	{
		const FName OpenStateKey = FName(*(InteractableId.ToString() + TEXT("_Open")));
		const FName LockedStateKey = FName(*(InteractableId.ToString() + TEXT("_Locked")));
		const FName PasswordUnlockedStateKey = FName(*(InteractableId.ToString() + TEXT("_PasswordUnlocked")));

		if (const bool* PasswordUnlockedState = InStateMap.Find(PasswordUnlockedStateKey))
		{
			bPasswordUnlocked = *PasswordUnlockedState;
		}

		if (const bool* LockedState = InStateMap.Find(LockedStateKey))
		{
			if (*LockedState || ShouldGateWithPassword())
			{
				DoorState = EDoorState::Locked;
				UpdateDoorCollision();
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
				NewRotation.Pitch += OpenRotationOffset.Pitch;
				NewRotation.Yaw += OpenRotationOffset.Yaw;
				NewRotation.Roll += OpenRotationOffset.Roll;
				DoorMesh->SetRelativeRotation(NewRotation);
			}
			else
			{
				DoorState = EDoorState::Closed;
				CurrentRotationAlpha = 0.0f;
				TargetRotationAlpha = 0.0f;
			}
			UpdateDoorCollision();
		}
		else if (ShouldGateWithPassword())
		{
			DoorState = EDoorState::Locked;
			UpdateDoorCollision();
		}
	}
}
