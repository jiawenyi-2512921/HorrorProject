// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "DebugCamera.generated.h"

/**
 * Free-flying debug camera for level inspection
 */
UCLASS()
class HORRORPROJECT_API ADebugCamera : public APawn
{
	GENERATED_BODY()

public:
	ADebugCamera();

	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UFUNCTION(BlueprintCallable, Category = "Debug")
	void ActivateDebugCamera();

	UFUNCTION(BlueprintCallable, Category = "Debug")
	void DeactivateDebugCamera();

	UFUNCTION(BlueprintCallable, Category = "Debug")
	void ToggleDebugCamera();

	UFUNCTION(BlueprintCallable, Category = "Debug")
	void SetMovementSpeed(float NewSpeed);

	UFUNCTION(BlueprintCallable, Category = "Debug")
	void SetRotationSpeed(float NewSpeed);

protected:
	virtual void BeginPlay() override;

	void MoveForward(float Value);
	void MoveRight(float Value);
	void MoveUp(float Value);
	void LookUp(float Value);
	void Turn(float Value);
	void SpeedUp();
	void SlowDown();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	class UCameraComponent* CameraComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	float MovementSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	float RotationSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	float SpeedMultiplier;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	float MinSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	float MaxSpeed;

	UPROPERTY(BlueprintReadOnly, Category = "Debug")
	bool bIsActive;

private:
	UPROPERTY(Transient)
	TObjectPtr<APlayerController> OriginalController = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<APawn> OriginalPawn = nullptr;

	FVector MovementInput;
};
