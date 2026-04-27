// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Interaction/BaseInteractable.h"
#include "DoorInteractable.generated.h"

class UStaticMeshComponent;
class UBoxComponent;
class UCurveFloat;

UENUM(BlueprintType)
enum class EDoorState : uint8
{
	Closed UMETA(DisplayName="Closed"),
	Opening UMETA(DisplayName="Opening"),
	Open UMETA(DisplayName="Open"),
	Closing UMETA(DisplayName="Closing"),
	Locked UMETA(DisplayName="Locked")
};

/**
 * Door interactable that can be opened, closed, and locked.
 * Supports smooth animation via timeline and configurable rotation.
 */
UCLASS(BlueprintType, Blueprintable)
class HORRORPROJECT_API ADoorInteractable : public ABaseInteractable
{
	GENERATED_BODY()

public:
	static constexpr float DefaultOpenAngle = 90.0f;

	ADoorInteractable();

	virtual void Tick(float DeltaTime) override;
	virtual bool CanInteract_Implementation(AActor* InstigatorActor, const FHitResult& Hit) const override;
	virtual void SaveState(TMap<FName, bool>& OutStateMap) const override;
	virtual void LoadState(const TMap<FName, bool>& InStateMap) override;

	UFUNCTION(BlueprintCallable, Category="Door")
	void SetLocked(bool bLocked);

	UFUNCTION(BlueprintPure, Category="Door")
	bool IsLocked() const { return DoorState == EDoorState::Locked; }

	UFUNCTION(BlueprintPure, Category="Door")
	bool IsOpen() const { return DoorState == EDoorState::Open; }

	UFUNCTION(BlueprintPure, Category="Door")
	EDoorState GetDoorState() const { return DoorState; }

protected:
	virtual void OnInteract(AActor* InstigatorActor, const FHitResult& Hit) override;
	virtual void BeginPlay() override;

private:
	void UpdateDoorRotation(float DeltaTime);
	void StartOpening();
	void StartClosing();
	float GetCurveValue(float Alpha) const;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Door", meta=(AllowPrivateAccess="true"))
	TObjectPtr<USceneComponent> DoorRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Door", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UStaticMeshComponent> DoorFrame;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Door", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UStaticMeshComponent> DoorMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Door", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UBoxComponent> InteractionVolume;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Door|Animation", meta=(AllowPrivateAccess="true"))
	float OpenAngle = DefaultOpenAngle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Door|Animation", meta=(AllowPrivateAccess="true"))
	float OpenSpeed = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Door|Animation", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UCurveFloat> OpenCurve = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Door|Audio", meta=(AllowPrivateAccess="true"))
	TObjectPtr<USoundBase> OpenSound = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Door|Audio", meta=(AllowPrivateAccess="true"))
	TObjectPtr<USoundBase> CloseSound = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Door|Audio", meta=(AllowPrivateAccess="true"))
	TObjectPtr<USoundBase> LockedSound = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Door|Behavior", meta=(AllowPrivateAccess="true"))
	bool bAutoClose = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Door|Behavior", meta=(AllowPrivateAccess="true", EditCondition="bAutoClose"))
	float AutoCloseDelay = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Door|Behavior", meta=(AllowPrivateAccess="true"))
	bool bStartLocked = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Door|State", meta=(AllowPrivateAccess="true"))
	EDoorState DoorState = EDoorState::Closed;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Door|State", meta=(AllowPrivateAccess="true"))
	float CurrentRotationAlpha = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Door|State", meta=(AllowPrivateAccess="true"))
	float TargetRotationAlpha = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Door|State", meta=(AllowPrivateAccess="true"))
	float AutoCloseTimer = 0.0f;

	FRotator InitialDoorRotation;
};
