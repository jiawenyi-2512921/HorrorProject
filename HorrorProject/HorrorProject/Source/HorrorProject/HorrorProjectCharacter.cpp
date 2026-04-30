// Copyright Epic Games, Inc. All Rights Reserved.

#include "HorrorProjectCharacter.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "EnhancedInputComponent.h"
#include "InputActionValue.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "ControlSettings.h"
#include "GameSettingsSubsystem.h"
#include "HorrorProject.h"

namespace
{
	constexpr float InitialCapsuleRadius = 55.0f;
	constexpr float CapsuleHalfHeight = 96.0f;
	constexpr float CollisionCapsuleRadius = 34.0f;
	constexpr float FirstPersonCameraYaw = 90.0f;
	constexpr float FirstPersonCameraRoll = -90.0f;
	constexpr float FirstPersonFieldOfView = 70.0f;
	constexpr float FallingBrakeDeceleration = 1500.0f;
	constexpr float MinimumJumpZVelocity = 520.0f;
	constexpr float MinimumMaxStepHeight = 55.0f;
	const FName FirstPersonCameraSocketName(TEXT("head"));
}

AHorrorProjectCharacter::AHorrorProjectCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(InitialCapsuleRadius, CapsuleHalfHeight);

	// Create the first person mesh that will be viewed only by this character's owner
	FirstPersonMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("First Person Mesh"));

	FirstPersonMesh->SetupAttachment(GetMesh());
	FirstPersonMesh->SetOnlyOwnerSee(true);
	FirstPersonMesh->FirstPersonPrimitiveType = EFirstPersonPrimitiveType::FirstPerson;
	FirstPersonMesh->SetCollisionProfileName(FName("NoCollision"));

	// Create the Camera Component
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("First Person Camera"));
	FirstPersonCameraComponent->SetupAttachment(FirstPersonMesh);
	FirstPersonCameraComponent->SetRelativeLocationAndRotation(
		FVector(-2.8f, 5.89f, 0.0f),
		FRotator(0.0f, FirstPersonCameraYaw, FirstPersonCameraRoll));
	FirstPersonCameraComponent->bUsePawnControlRotation = true;
	FirstPersonCameraComponent->bEnableFirstPersonFieldOfView = true;
	FirstPersonCameraComponent->bEnableFirstPersonScale = true;
	FirstPersonCameraComponent->FirstPersonFieldOfView = FirstPersonFieldOfView;
	FirstPersonCameraComponent->FirstPersonScale = 0.6f;

	// configure the character comps
	GetMesh()->SetOwnerNoSee(true);
	GetMesh()->FirstPersonPrimitiveType = EFirstPersonPrimitiveType::WorldSpaceRepresentation;

	GetCapsuleComponent()->SetCapsuleSize(CollisionCapsuleRadius, CapsuleHalfHeight);

	// Configure character movement
	bUseControllerRotationYaw = true;
	GetCharacterMovement()->bOrientRotationToMovement = false;
	GetCharacterMovement()->BrakingDecelerationFalling = FallingBrakeDeceleration;
	GetCharacterMovement()->AirControl = 0.5f;
	ConfigureJumpMovementDefaults();
}

void AHorrorProjectCharacter::BeginPlay()
{
	Super::BeginPlay();
	if (FirstPersonMesh
		&& FirstPersonCameraComponent
		&& FirstPersonMesh->DoesSocketExist(FirstPersonCameraSocketName)
		&& FirstPersonCameraComponent->GetAttachSocketName() != FirstPersonCameraSocketName)
	{
		FirstPersonCameraComponent->AttachToComponent(
			FirstPersonMesh,
			FAttachmentTransformRules::KeepRelativeTransform,
			FirstPersonCameraSocketName);
	}
	ConfigureJumpMovementDefaults();
}

void AHorrorProjectCharacter::ConfigureJumpMovementDefaults()
{
	if (UCharacterMovementComponent* MovementComponent = GetCharacterMovement())
	{
		MovementComponent->NavAgentProps.bCanJump = true;
		MovementComponent->JumpZVelocity = FMath::Max(MovementComponent->JumpZVelocity, MinimumJumpZVelocity);
		MovementComponent->MaxStepHeight = FMath::Max(MovementComponent->MaxStepHeight, MinimumMaxStepHeight);
	}
}

void AHorrorProjectCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &AHorrorProjectCharacter::DoJumpStart);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &AHorrorProjectCharacter::DoJumpEnd);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AHorrorProjectCharacter::MoveInput);

		// Looking/Aiming
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AHorrorProjectCharacter::LookInput);
		EnhancedInputComponent->BindAction(MouseLookAction, ETriggerEvent::Triggered, this, &AHorrorProjectCharacter::LookInput);
	}
	else
	{
		UE_LOG(LogHorrorProject, Error, TEXT("'%s' Failed to find an Enhanced Input Component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}


void AHorrorProjectCharacter::MoveInput(const FInputActionValue& Value)
{
	// get the Vector2D move axis
	FVector2D MovementVector = Value.Get<FVector2D>();

	// pass the axis values to the move input
	DoMove(MovementVector.X, MovementVector.Y);

}

void AHorrorProjectCharacter::LookInput(const FInputActionValue& Value)
{
	// get the Vector2D look axis
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	// pass the axis values to the aim input
	DoAim(LookAxisVector.X, LookAxisVector.Y);

}

void AHorrorProjectCharacter::DoAim(float Yaw, float Pitch)
{
	if (GetController())
	{
		float MouseSensitivity = 1.0f;
		bool bInvertMouseX = false;
		bool bInvertMouseY = false;
		if (UGameInstance* GameInstance = GetGameInstance())
		{
			if (const UGameSettingsSubsystem* SettingsSubsystem = GameInstance->GetSubsystem<UGameSettingsSubsystem>())
			{
				if (const UControlSettings* ControlSettings = SettingsSubsystem->GetControlSettings())
				{
					MouseSensitivity = FMath::Clamp(ControlSettings->MouseSensitivity, 0.1f, 5.0f);
					bInvertMouseX = ControlSettings->bInvertMouseX;
					bInvertMouseY = ControlSettings->bInvertMouseY;
				}
			}
		}

		const float AdjustedYaw = Yaw * MouseSensitivity * (bInvertMouseX ? -1.0f : 1.0f);
		const float AdjustedPitch = Pitch * MouseSensitivity * (bInvertMouseY ? -1.0f : 1.0f);

		// pass the rotation inputs
		AddControllerYawInput(AdjustedYaw);
		AddControllerPitchInput(-AdjustedPitch);
	}
}

void AHorrorProjectCharacter::DoMove(float Right, float Forward)
{
	if (GetController())
	{
		// pass the move inputs
		AddMovementInput(GetActorRightVector(), Right);
		AddMovementInput(GetActorForwardVector(), Forward);
	}
}

void AHorrorProjectCharacter::DoJumpStart()
{
	// pass Jump to the character
	Jump();
}

void AHorrorProjectCharacter::DoJumpEnd()
{
	// pass StopJumping to the character
	StopJumping();
}
