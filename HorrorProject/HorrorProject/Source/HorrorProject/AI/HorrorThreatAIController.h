// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "HorrorThreatAIController.generated.h"

class AHorrorThreatCharacter;

/**
 * Implements actor-level Horror Threat AIController behavior for the AI module.
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup=(Horror))
class HORRORPROJECT_API AHorrorThreatAIController : public AAIController
{
	GENERATED_BODY()

public:
	AHorrorThreatAIController();

	UFUNCTION(BlueprintPure, Category="Horror|Threat")
	AHorrorThreatCharacter* GetControlledThreat() const;
};
