// Copyright Epic Games, Inc. All Rights Reserved.

#include "Interaction/ExaminableInteractable.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "Camera/CameraComponent.h"
#include "InputCoreTypes.h"

namespace
{
	const FVector ExamineInteractionExtent(100.0f, 100.0f, 100.0f);
	constexpr float MaxExaminePitchDegrees = 89.0f;
}

AExaminableInteractable::AExaminableInteractable()
{
	PrimaryActorTick.bCanEverTick = true;

	// Create components
	ExamineMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ExamineMesh"));
	RootComponent = ExamineMesh;
	ExamineMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	ExamineMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	ExamineMesh->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

	InteractionVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractionVolume"));
	InteractionVolume->SetupAttachment(ExamineMesh);
	InteractionVolume->SetBoxExtent(ExamineInteractionExtent);
	InteractionVolume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	InteractionVolume->SetCollisionResponseToAllChannels(ECR_Ignore);
	InteractionVolume->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

	// Default settings
	InteractionPrompt = FText::FromString("Examine");
	bCanInteractMultipleTimes = true;
}

void AExaminableInteractable::BeginPlay()
{
	Super::BeginPlay();

	OriginalLocation = GetActorLocation();
	OriginalRotation = GetActorRotation();
	OriginalScale = GetActorScale3D();
	CurrentZoomDistance = ExamineDistance;
}

void AExaminableInteractable::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (ExamineState == EExamineState::Examining)
	{
		UpdateExamineCamera(DeltaTime);
		ProcessPlayerInput(DeltaTime);
	}
}

bool AExaminableInteractable::CanInteract_Implementation(AActor* InstigatorActor, const FHitResult& Hit) const
{
	if (!Super::CanInteract_Implementation(InstigatorActor, Hit))
	{
		return false;
	}

	// Can't start examining if already examining
	if (ExamineState == EExamineState::Examining)
	{
		return false;
	}

	return true;
}

void AExaminableInteractable::OnInteract(AActor* InstigatorActor, const FHitResult& Hit)
{
	Super::OnInteract(InstigatorActor, Hit);

	APlayerController* PC = Cast<APlayerController>(InstigatorActor->GetInstigatorController());
	if (PC)
	{
		StartExamining(PC);
	}
}

void AExaminableInteractable::StartExamining(APlayerController* PlayerController)
{
	if (!PlayerController || ExamineState == EExamineState::Examining)
	{
		return;
	}

	ExaminingPlayerController = PlayerController;
	ExamineState = EExamineState::Examining;
	CurrentExamineRotation = FRotator::ZeroRotator;

	// Play start sound
	if (ExamineStartSound && GetWorld())
	{
		UGameplayStatics::PlaySound2D(GetWorld(), ExamineStartSound);
	}

	// Disable player movement if configured
	if (bDisablePlayerMovement)
	{
		if (ACharacter* Character = Cast<ACharacter>(PlayerController->GetPawn()))
		{
			if (UCharacterMovementComponent* Movement = Character->GetCharacterMovement())
			{
				bWasPlayerMovementEnabled = Movement->MovementMode == MOVE_Walking;
				Movement->DisableMovement();
			}
		}
	}

	// Hide original mesh if configured
	if (bHideOriginalMesh)
	{
		ExamineMesh->SetVisibility(false);
	}
}

void AExaminableInteractable::StopExamining()
{
	if (ExamineState != EExamineState::Examining)
	{
		return;
	}

	ExamineState = EExamineState::Idle;

	// Play end sound
	if (ExamineEndSound && GetWorld())
	{
		UGameplayStatics::PlaySound2D(GetWorld(), ExamineEndSound);
	}

	// Re-enable player movement
	if (bDisablePlayerMovement && ExaminingPlayerController)
	{
		if (ACharacter* Character = Cast<ACharacter>(ExaminingPlayerController->GetPawn()))
		{
			if (UCharacterMovementComponent* Movement = Character->GetCharacterMovement())
			{
				if (bWasPlayerMovementEnabled)
				{
					Movement->SetMovementMode(MOVE_Walking);
				}
			}
		}
	}

	// Show original mesh
	if (bHideOriginalMesh)
	{
		ExamineMesh->SetVisibility(true);
	}

	// Reset state
	SetActorLocation(OriginalLocation);
	SetActorRotation(OriginalRotation);
	CurrentZoomDistance = ExamineDistance;
	ExaminingPlayerController = nullptr;
}

void AExaminableInteractable::RotateObject(float Yaw, float Pitch)
{
	if (!bAllowRotation || ExamineState != EExamineState::Examining)
	{
		return;
	}

	CurrentExamineRotation.Yaw += Yaw;
	CurrentExamineRotation.Pitch += Pitch;

	// Clamp pitch to avoid flipping
	CurrentExamineRotation.Pitch = FMath::Clamp(CurrentExamineRotation.Pitch, -MaxExaminePitchDegrees, MaxExaminePitchDegrees);
}

void AExaminableInteractable::ZoomObject(float ZoomDelta)
{
	if (!bAllowZoom || ExamineState != EExamineState::Examining)
	{
		return;
	}

	CurrentZoomDistance = FMath::Clamp(
		CurrentZoomDistance + ZoomDelta,
		MinZoomDistance,
		MaxZoomDistance
	);
}

void AExaminableInteractable::UpdateExamineCamera(float DeltaTime)
{
	if (!ExaminingPlayerController)
	{
		return;
	}

	// Position object in front of camera
	APawn* Pawn = ExaminingPlayerController->GetPawn();
	if (!Pawn)
	{
		return;
	}

	// Get camera location and rotation
	FVector CameraLocation;
	FRotator CameraRotation;
	ExaminingPlayerController->GetPlayerViewPoint(CameraLocation, CameraRotation);

	// Calculate examine position
	const FVector ForwardVector = CameraRotation.Vector();
	const FVector ExamineLocation = CameraLocation + (ForwardVector * CurrentZoomDistance);

	// Apply rotation
	const FRotator ExamineRotation = CameraRotation + CurrentExamineRotation;

	// Update actor transform
	SetActorLocation(ExamineLocation);
	SetActorRotation(ExamineRotation);
}

void AExaminableInteractable::ProcessPlayerInput(float DeltaTime)
{
	if (!ExaminingPlayerController)
	{
		return;
	}

	if (ExaminingPlayerController->WasInputKeyJustPressed(EKeys::Escape)
		|| ExaminingPlayerController->WasInputKeyJustPressed(EKeys::Gamepad_FaceButton_Right))
	{
		StopExamining();
		return;
	}

	float MouseDeltaX = 0.0f;
	float MouseDeltaY = 0.0f;
	ExaminingPlayerController->GetInputMouseDelta(MouseDeltaX, MouseDeltaY);

	const bool bRotateWithMouse = ExaminingPlayerController->IsInputKeyDown(EKeys::LeftMouseButton)
		|| ExaminingPlayerController->IsInputKeyDown(EKeys::RightMouseButton);
	if (bRotateWithMouse && (!FMath::IsNearlyZero(MouseDeltaX) || !FMath::IsNearlyZero(MouseDeltaY)))
	{
		constexpr float MouseRotationScale = 0.01f;
		RotateObject(MouseDeltaX * RotationSpeed * MouseRotationScale, MouseDeltaY * RotationSpeed * MouseRotationScale);
	}

	const float GamepadYaw = ExaminingPlayerController->GetInputAnalogKeyState(EKeys::Gamepad_RightX);
	const float GamepadPitch = ExaminingPlayerController->GetInputAnalogKeyState(EKeys::Gamepad_RightY);
	if (!FMath::IsNearlyZero(GamepadYaw) || !FMath::IsNearlyZero(GamepadPitch))
	{
		RotateObject(GamepadYaw * RotationSpeed * DeltaTime, GamepadPitch * RotationSpeed * DeltaTime);
	}

	if (ExaminingPlayerController->WasInputKeyJustPressed(EKeys::MouseScrollUp))
	{
		ZoomObject(-ZoomSpeed);
	}
	else if (ExaminingPlayerController->WasInputKeyJustPressed(EKeys::MouseScrollDown))
	{
		ZoomObject(ZoomSpeed);
	}

	const float ZoomAxis = ExaminingPlayerController->GetInputAnalogKeyState(EKeys::Gamepad_RightTriggerAxis)
		- ExaminingPlayerController->GetInputAnalogKeyState(EKeys::Gamepad_LeftTriggerAxis);
	if (!FMath::IsNearlyZero(ZoomAxis))
	{
		ZoomObject(-ZoomAxis * ZoomSpeed * DeltaTime);
	}
}
