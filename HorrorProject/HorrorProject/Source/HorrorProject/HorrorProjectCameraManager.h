// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Camera/PlayerCameraManager.h"
#include "HorrorProjectCameraManager.generated.h"

/**
 *  First-person camera manager. Spawned at runtime by AHorrorPlayerController
 *  (Player/HorrorPlayerController.cpp) via PlayerCameraManagerClass.
 *  Hidden from BP class picker; instantiated by C++ only.
 */
UCLASS(Hidden, NotBlueprintable, NotPlaceable)
class AHorrorProjectCameraManager : public APlayerCameraManager
{
	GENERATED_BODY()
	
public:

	/** Constructor */
	AHorrorProjectCameraManager();
};
