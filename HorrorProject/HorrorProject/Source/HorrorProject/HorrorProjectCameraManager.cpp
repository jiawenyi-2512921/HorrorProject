// Copyright Epic Games, Inc. All Rights Reserved.


#include "HorrorProjectCameraManager.h"

namespace
{
	constexpr float MinViewPitchDegrees = -70.0f;
	constexpr float MaxViewPitchDegrees = 80.0f;
}

AHorrorProjectCameraManager::AHorrorProjectCameraManager()
{
	// set the min/max pitch
	ViewPitchMin = MinViewPitchDegrees;
	ViewPitchMax = MaxViewPitchDegrees;
}
