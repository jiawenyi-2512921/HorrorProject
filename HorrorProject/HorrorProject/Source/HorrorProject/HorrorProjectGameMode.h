// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "HorrorProjectGameMode.generated.h"

/**
 *  DEPRECATED: legacy template GameMode from the FirstPerson template.
 *  Live GameMode is `AHorrorGameModeBase` (Game/HorrorGameModeBase.h),
 *  bound via `[/Script/EngineSettings.GameMapsSettings] GlobalDefaultGameMode`.
 *  Kept only as a graveyard so old BP class references don't fail to load.
 */
UCLASS(abstract, Hidden, NotBlueprintable, NotPlaceable)
class AHorrorProjectGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AHorrorProjectGameMode();
};



