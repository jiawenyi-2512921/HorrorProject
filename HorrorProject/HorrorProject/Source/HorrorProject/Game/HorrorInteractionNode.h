// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Game/HorrorObjectiveNode.h"
#include "Interaction/InteractableInterface.h"
#include "HorrorInteractionNode.generated.h"

UCLASS(BlueprintType, Blueprintable, ClassGroup=(Horror))
class HORRORPROJECT_API AHorrorInteractionNode : public AHorrorObjectiveNode, public IInteractableInterface
{
	GENERATED_BODY()

public:
	AHorrorInteractionNode();

	virtual bool CanInteract_Implementation(AActor* InstigatorActor, const FHitResult& Hit) const override;
	virtual bool Interact_Implementation(AActor* InstigatorActor, const FHitResult& Hit) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Interaction")
	bool bRequiresHold = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Interaction", meta=(EditCondition="bRequiresHold", ClampMin="0.1"))
	float HoldDuration = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Interaction")
	FText InteractionPrompt;

protected:
	UFUNCTION(BlueprintNativeEvent, Category="Horror|Interaction")
	bool OnInteract(AActor* InstigatorActor);

	virtual bool OnInteract_Implementation(AActor* InstigatorActor);
};
