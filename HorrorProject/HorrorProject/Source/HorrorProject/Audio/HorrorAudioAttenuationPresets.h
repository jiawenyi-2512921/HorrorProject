// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Sound/SoundAttenuation.h"
#include "HorrorAudioAttenuationPresets.generated.h"

UCLASS()
class HORRORPROJECT_API UHorrorAudioAttenuationPresets : public UObject
{
	GENERATED_BODY()

public:
	static FSoundAttenuationSettings CreateAmbientAttenuation();
	static FSoundAttenuationSettings CreateFootstepAttenuation();
	static FSoundAttenuationSettings CreateInteractionAttenuation();
	static FSoundAttenuationSettings CreateHorrorEffectAttenuation();
	static FSoundAttenuationSettings CreateDistantAttenuation();
	static FSoundAttenuationSettings CreateCloseRangeAttenuation();
	static FSoundAttenuationSettings CreateUnderwaterAttenuation();
};
