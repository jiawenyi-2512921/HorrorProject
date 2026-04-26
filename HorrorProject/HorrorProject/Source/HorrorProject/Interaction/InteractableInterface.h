// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "InteractableInterface.generated.h"

UINTERFACE(BlueprintType)
class HORRORPROJECT_API UInteractableInterface : public UInterface
{
	GENERATED_BODY()
};

class HORRORPROJECT_API IInteractableInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Interaction")
	bool CanInteract(AActor* InstigatorActor, const FHitResult& Hit) const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Interaction")
	bool Interact(AActor* InstigatorActor, const FHitResult& Hit);
};
