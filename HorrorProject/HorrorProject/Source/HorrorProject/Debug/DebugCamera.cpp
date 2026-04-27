// Copyright Epic Games, Inc. All Rights Reserved.

#include "DebugCamera.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

namespace HorrorDebugCamera
{
	constexpr float DefaultMovementSpeedCmPerSecond = 1000.0f;
	constexpr float DefaultRotationSpeedDegreesPerSecond = 45.0f;
	constexpr float SpeedMultiplierStep = 2.0f;
	constexpr float MinMovementSpeedCmPerSecond = 100.0f;
	constexpr float MaxMovementSpeedCmPerSecond = 10000.0f;
	constexpr float MinRotationSpeedDegreesPerSecond = 10.0f;
	constexpr float MaxRotationSpeedDegreesPerSecond = 180.0f;
}

ADebugCamera::ADebugCamera()
{
	PrimaryActorTick.bCanEverTick = true;

	// Create camera component
	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComponent"));
	RootComponent = CameraComponent;

	MovementSpeed = HorrorDebugCamera::DefaultMovementSpeedCmPerSecond;
	RotationSpeed = HorrorDebugCamera::DefaultRotationSpeedDegreesPerSecond;
	SpeedMultiplier = HorrorDebugCamera::SpeedMultiplierStep;
	MinSpeed = HorrorDebugCamera::MinMovementSpeedCmPerSecond;
	MaxSpeed = HorrorDebugCamera::MaxMovementSpeedCmPerSecond;
	bIsActive = false;

	MovementInput = FVector::ZeroVector;
}

void ADebugCamera::BeginPlay()
{
	Super::BeginPlay();
	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);
}

void ADebugCamera::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!bIsActive) return;

	// Apply movement
	if (!MovementInput.IsNearlyZero())
	{
		FVector NewLocation = GetActorLocation() + MovementInput * MovementSpeed * DeltaTime;
		SetActorLocation(NewLocation);
	}
}

void ADebugCamera::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (!PlayerInputComponent) return;

	// Movement bindings
	PlayerInputComponent->BindAxis("MoveForward", this, &ADebugCamera::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ADebugCamera::MoveRight);
	PlayerInputComponent->BindAxis("MoveUp", this, &ADebugCamera::MoveUp);

	// Look bindings
	PlayerInputComponent->BindAxis("LookUp", this, &ADebugCamera::LookUp);
	PlayerInputComponent->BindAxis("Turn", this, &ADebugCamera::Turn);

	// Speed control
	PlayerInputComponent->BindAction("SpeedUp", IE_Pressed, this, &ADebugCamera::SpeedUp);
	PlayerInputComponent->BindAction("SlowDown", IE_Pressed, this, &ADebugCamera::SlowDown);
}

void ADebugCamera::ActivateDebugCamera()
{
	if (bIsActive) return;

	UWorld* World = GetWorld();
	if (!World) return;

	APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0);
	if (!PC) return;

	// Store original controller and pawn
	OriginalController = PC;
	OriginalPawn = PC->GetPawn();

	// Possess debug camera
	PC->Possess(this);

	// Set camera location to player location
	if (OriginalPawn)
	{
		SetActorLocation(OriginalPawn->GetActorLocation());
		SetActorRotation(OriginalPawn->GetActorRotation());
	}

	SetActorHiddenInGame(false);
	bIsActive = true;

	UE_LOG(LogTemp, Warning, TEXT("Debug Camera Activated"));
}

void ADebugCamera::DeactivateDebugCamera()
{
	if (!bIsActive) return;

	if (OriginalController && OriginalPawn)
	{
		OriginalController->Possess(OriginalPawn);
	}

	SetActorHiddenInGame(true);
	bIsActive = false;
	MovementInput = FVector::ZeroVector;
	OriginalController = nullptr;
	OriginalPawn = nullptr;

	UE_LOG(LogTemp, Warning, TEXT("Debug Camera Deactivated"));
}

void ADebugCamera::ToggleDebugCamera()
{
	if (bIsActive)
	{
		DeactivateDebugCamera();
	}
	else
	{
		ActivateDebugCamera();
	}
}

void ADebugCamera::SetMovementSpeed(float NewSpeed)
{
	MovementSpeed = FMath::Clamp(NewSpeed, MinSpeed, MaxSpeed);
	UE_LOG(LogTemp, Log, TEXT("Debug Camera Speed: %.2f"), MovementSpeed);
}

void ADebugCamera::SetRotationSpeed(float NewSpeed)
{
	RotationSpeed = FMath::Clamp(
		NewSpeed,
		HorrorDebugCamera::MinRotationSpeedDegreesPerSecond,
		HorrorDebugCamera::MaxRotationSpeedDegreesPerSecond);
	UE_LOG(LogTemp, Log, TEXT("Debug Camera Rotation Speed: %.2f"), RotationSpeed);
}

void ADebugCamera::MoveForward(float Value)
{
	if (!bIsActive) return;
	UWorld* World = GetWorld();
	if (!World) return;
	MovementInput.X = Value;
	FVector Forward = GetActorForwardVector();
	AddActorWorldOffset(Forward * Value * MovementSpeed * World->GetDeltaSeconds());
}

void ADebugCamera::MoveRight(float Value)
{
	if (!bIsActive) return;
	UWorld* World = GetWorld();
	if (!World) return;
	MovementInput.Y = Value;
	FVector Right = GetActorRightVector();
	AddActorWorldOffset(Right * Value * MovementSpeed * World->GetDeltaSeconds());
}

void ADebugCamera::MoveUp(float Value)
{
	if (!bIsActive) return;
	UWorld* World = GetWorld();
	if (!World) return;
	MovementInput.Z = Value;
	FVector Up = FVector::UpVector;
	AddActorWorldOffset(Up * Value * MovementSpeed * World->GetDeltaSeconds());
}

void ADebugCamera::LookUp(float Value)
{
	if (!bIsActive) return;
	UWorld* World = GetWorld();
	if (!World) return;
	AddActorLocalRotation(FRotator(Value * RotationSpeed * World->GetDeltaSeconds(), 0.0f, 0.0f));
}

void ADebugCamera::Turn(float Value)
{
	if (!bIsActive) return;
	UWorld* World = GetWorld();
	if (!World) return;
	AddActorLocalRotation(FRotator(0.0f, Value * RotationSpeed * World->GetDeltaSeconds(), 0.0f));
}

void ADebugCamera::SpeedUp()
{
	SetMovementSpeed(MovementSpeed * SpeedMultiplier);
}

void ADebugCamera::SlowDown()
{
	SetMovementSpeed(MovementSpeed / SpeedMultiplier);
}
