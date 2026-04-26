// Copyright Epic Games, Inc. All Rights Reserved.

#include "AI/HorrorThreatAIController.h"

#include "AI/HorrorThreatCharacter.h"

AHorrorThreatAIController::AHorrorThreatAIController()
{
	PrimaryActorTick.bCanEverTick = false;
}

AHorrorThreatCharacter* AHorrorThreatAIController::GetControlledThreat() const
{
	return Cast<AHorrorThreatCharacter>(GetPawn());
}
