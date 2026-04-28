// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Interaction/BaseInteractable.h"
#include "ExaminableInteractable.generated.h"

class UStaticMeshComponent;
class UBoxComponent;
class APlayerController;
class UCameraComponent;

UENUM(BlueprintType)
enum class EExamineState : uint8
{
	Idle UMETA(DisplayName="空闲"),
	Examining UMETA(DisplayName="查看中"),
	Returning UMETA(DisplayName="归位中")
};

/**
 * Examinable interactable that allows close-up inspection with rotation and zoom.
 * Player can rotate and examine the object in detail.
 */
UCLASS(BlueprintType, Blueprintable)
class HORRORPROJECT_API AExaminableInteractable : public ABaseInteractable
{
	GENERATED_BODY()

public:
	static constexpr float DefaultMinZoomDistance = 50.0f;
	static constexpr float DefaultMaxZoomDistance = 200.0f;

	AExaminableInteractable();

	virtual void Tick(float DeltaTime) override;
	virtual bool CanInteract_Implementation(AActor* InstigatorActor, const FHitResult& Hit) const override;

	UFUNCTION(BlueprintCallable, Category="Examine")
	void StartExamining(APlayerController* PlayerController);

	UFUNCTION(BlueprintCallable, Category="Examine")
	void StopExamining();

	UFUNCTION(BlueprintPure, Category="Examine")
	bool IsBeingExamined() const { return ExamineState == EExamineState::Examining; }

	UFUNCTION(BlueprintCallable, Category="Examine")
	void RotateObject(float Yaw, float Pitch);

	UFUNCTION(BlueprintCallable, Category="Examine")
	void ZoomObject(float ZoomDelta);

protected:
	virtual void OnInteract(AActor* InstigatorActor, const FHitResult& Hit) override;
	virtual void BeginPlay() override;

private:
	void UpdateExamineCamera(float DeltaTime);
	void ProcessPlayerInput(float DeltaTime);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Examine", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UStaticMeshComponent> ExamineMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Examine", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UBoxComponent> InteractionVolume;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Examine|Camera", meta=(AllowPrivateAccess="true"))
	float ExamineDistance = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Examine|Camera", meta=(AllowPrivateAccess="true"))
	float MinZoomDistance = DefaultMinZoomDistance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Examine|Camera", meta=(AllowPrivateAccess="true"))
	float MaxZoomDistance = DefaultMaxZoomDistance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Examine|Camera", meta=(AllowPrivateAccess="true"))
	float ZoomSpeed = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Examine|Rotation", meta=(AllowPrivateAccess="true"))
	float RotationSpeed = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Examine|Rotation", meta=(AllowPrivateAccess="true"))
	bool bAllowRotation = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Examine|Rotation", meta=(AllowPrivateAccess="true"))
	bool bAllowZoom = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Examine|Audio", meta=(AllowPrivateAccess="true"))
	TObjectPtr<USoundBase> ExamineStartSound = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Examine|Audio", meta=(AllowPrivateAccess="true"))
	TObjectPtr<USoundBase> ExamineEndSound = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Examine|Behavior", meta=(AllowPrivateAccess="true"))
	bool bDisablePlayerMovement = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Examine|Behavior", meta=(AllowPrivateAccess="true"))
	bool bHideOriginalMesh = true;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Examine|State", meta=(AllowPrivateAccess="true"))
	EExamineState ExamineState = EExamineState::Idle;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Examine|State", meta=(AllowPrivateAccess="true"))
	float CurrentZoomDistance = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Examine|State", meta=(AllowPrivateAccess="true"))
	FRotator CurrentExamineRotation = FRotator::ZeroRotator;

	UPROPERTY()
	TObjectPtr<APlayerController> ExaminingPlayerController = nullptr;

	FVector OriginalLocation;
	FRotator OriginalRotation;
	FVector OriginalScale;
	bool bWasPlayerMovementEnabled = true;
};
