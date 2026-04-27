// Copyright Epic Games, Inc. All Rights Reserved.

#include "HorrorProject.h"
#include "Modules/ModuleManager.h"
#include "NativeGameplayTags.h"

UE_DEFINE_GAMEPLAY_TAG(TAG_Horror_Camera_Battery_Depleted, "Horror.Camera.Battery.Depleted")
UE_DEFINE_GAMEPLAY_TAG(TAG_Horror_Camera_Battery_Low, "Horror.Camera.Battery.Low")
UE_DEFINE_GAMEPLAY_TAG(TAG_Horror_Camera_Battery_Charging, "Horror.Camera.Battery.Charging")

IMPLEMENT_PRIMARY_GAME_MODULE( FDefaultGameModuleImpl, HorrorProject, "HorrorProject" );

DEFINE_LOG_CATEGORY(LogHorrorProject)
