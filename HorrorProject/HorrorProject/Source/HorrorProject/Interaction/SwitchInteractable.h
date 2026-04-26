// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Interaction/BaseInteractable.h"
#include "SwitchInteractable.generated.h"

class UStaticMeshComponent;
class UBoxComponent;

UENUM(BlueprintType)
enum class ESwitchType : uint8
{
	Toggle UMETA(DisplayName="Toggle Switch"),
	Momentary UMETA(DisplayName="Momentary Button"),
	OneTime UMETA(DisplayName="One-Time Trigger")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSwitchActivatedSignature, ABaseInteractable*, Switch, bool, bIsOn);

/**
 * Switch interactable that triggers events and controls other actors.
 * Supports toggle, momentary, and one-time activation modes.
 */
UCLASS(BlueprintType, Blueprintable)
class HORRORPROJECT_API ASwitchInteractable : public ABaseInteractable
{
	GENERATED_BODY()

public:
	ASwitchInteractable();

	virtual bool CanInteract_Implementation(AActor* InstigatorActor, const FHitResult& Hit) const override;
	virtual void SaveState(TMap<FName, bool>& OutStateMap) const override;
	virtual void LoadState(const TMap<FName, bool>& InStateMap) override;

	UFUNCTION(BlueprintCallable, Category="Switch")
	void SetSwitchState(bool bNewState);

	UFUNCTION(BlueprintPure, Category="Switch")
	bool IsSwitchOn() const { return bIsSwitchOn; }

	UFUNCTION(BlueprintPure, Category="Switch")
	ESwitchType GetSwitchType() const { return SwitchType; }

	UPROPERTY(BlueprintAssignable, Category="Switch")
	FSwitchActivatedSignature OnSwitchActivated;

protected:
	virtual void OnInteract(AActor* InstigatorActor, const FHitResult& Hit) override;
	virtual void BeginPlay() override;

private:
	void ActivateSwitch();
	void DeactivateSwitch();
	void TriggerConnectedActors();
	void UpdateVisuals();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Switch", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UStaticMeshComponent> SwitchBase;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Switch", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UStaticMeshComponent> SwitchHandle;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Switch", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UBoxComponent> InteractionVolume;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Switch|Type", meta=(AllowPrivateAccess="true"))
	ESwitchType SwitchType = ESwitchType::Toggle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Switch|Behavior", meta=(AllowPrivateAccess="true"))
	bool bStartOn = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Switch|Behavior", meta=(AllowPrivateAccess="true", EditCondition="SwitchType == ESwitchType::Momentary"))
	float MomentaryDuration = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Switch|Connections", meta=(AllowPrivateAccess="true"))
	TArray<AActor*> ConnectedActors;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Switch|Connections", meta=(AllowPrivateAccess="true"))
	FName ActivationFunctionName = TEXT("OnSwitchActivated");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Switch|Audio", meta=(AllowPrivateAccess="true"))
	TObjectPtr<USoundBase> ActivateSound = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Switch|Audio", meta=(AllowPrivateAccess="true"))
	TObjectPtr<USoundBase> DeactivateSound = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Switch|Visual", meta=(AllowPrivateAccess="true"))
	FRotator OnRotation = FRotator(0.0f, 0.0f, 45.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Switch|Visual", meta=(AllowPrivateAccess="true"))
	FRotator OffRotation = FRotator(0.0f, 0.0f, -45.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Switch|Visual", meta=(AllowPrivateAccess="true"))
	float AnimationSpeed = 5.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Switch|State", meta=(AllowPrivateAccess="true"))
	bool bIsSwitchOn = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Switch|State", meta=(AllowPrivateAccess="true"))
	bool bIsAnimating = false;

	FTimerHandle MomentaryTimer;
	FRotator TargetRotation;
	FRotator CurrentRotation;
};
