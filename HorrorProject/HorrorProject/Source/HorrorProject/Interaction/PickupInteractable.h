// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Interaction/BaseInteractable.h"
#include "Player/Components/InventoryComponent.h"
#include "PickupInteractable.generated.h"

class UStaticMeshComponent;
class USphereComponent;
class UInventoryComponent;

UENUM(BlueprintType)
enum class EPickupType : uint8
{
	Evidence UMETA(DisplayName="证据"),
	Key UMETA(DisplayName="钥匙"),
	Tool UMETA(DisplayName="工具"),
	Document UMETA(DisplayName="文档"),
	Battery UMETA(DisplayName="电池"),
	Generic UMETA(DisplayName="普通物品")
};

/**
 * Pickup interactable that adds items to the player's inventory and disappears.
 * Supports evidence collection and inventory integration.
 */
UCLASS(BlueprintType, Blueprintable)
class HORRORPROJECT_API APickupInteractable : public ABaseInteractable
{
	GENERATED_BODY()

public:
	static constexpr float DefaultRotationSpeed = 90.0f;

	APickupInteractable();

	virtual bool CanInteract_Implementation(AActor* InstigatorActor, const FHitResult& Hit) const override;

	UFUNCTION(BlueprintPure, Category="Pickup")
	EPickupType GetPickupType() const { return PickupType; }

	UFUNCTION(BlueprintPure, Category="Pickup")
	FName GetItemId() const { return ItemId; }

protected:
	virtual void OnInteract(AActor* InstigatorActor, const FHitResult& Hit) override;
	virtual void BeginPlay() override;

private:
	bool TryAddToInventory(AActor* InstigatorActor);
	void DestroyPickup();
	void DestroyPickupActor();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Pickup", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UStaticMeshComponent> PickupMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Pickup", meta=(AllowPrivateAccess="true"))
	TObjectPtr<USphereComponent> InteractionVolume;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Pickup|Item", meta=(AllowPrivateAccess="true"))
	EPickupType PickupType = EPickupType::Generic;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Pickup|Item", meta=(AllowPrivateAccess="true"))
	FName ItemId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Pickup|Item", meta=(AllowPrivateAccess="true"))
	FHorrorEvidenceMetadata EvidenceMetadata;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Pickup|Visual", meta=(AllowPrivateAccess="true"))
	bool bRotateInPlace = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Pickup|Visual", meta=(AllowPrivateAccess="true", EditCondition="bRotateInPlace"))
	float RotationSpeed = DefaultRotationSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Pickup|Visual", meta=(AllowPrivateAccess="true"))
	bool bFloatInPlace = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Pickup|Visual", meta=(AllowPrivateAccess="true", EditCondition="bFloatInPlace"))
	float FloatAmplitude = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Pickup|Visual", meta=(AllowPrivateAccess="true", EditCondition="bFloatInPlace"))
	float FloatSpeed = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Pickup|Audio", meta=(AllowPrivateAccess="true"))
	TObjectPtr<USoundBase> PickupSound = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Pickup|Behavior", meta=(AllowPrivateAccess="true"))
	bool bDestroyOnPickup = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Pickup|Behavior", meta=(AllowPrivateAccess="true"))
	float DestroyDelay = 0.0f;

	FVector InitialLocation;
	float FloatTime = 0.0f;
};
