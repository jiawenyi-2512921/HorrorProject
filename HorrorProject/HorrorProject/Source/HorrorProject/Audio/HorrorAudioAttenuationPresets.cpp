// Copyright Epic Games, Inc. All Rights Reserved.

#include "Audio/HorrorAudioAttenuationPresets.h"

FSoundAttenuationSettings UHorrorAudioAttenuationPresets::CreateAmbientAttenuation()
{
	FSoundAttenuationSettings Settings;
	Settings.bAttenuate = true;
	Settings.bSpatialize = true;
	Settings.AttenuationShape = EAttenuationShape::Sphere;
	Settings.AttenuationShapeExtents = FVector(5000.0f, 0.0f, 0.0f);
	Settings.FalloffDistance = 4000.0f;
	Settings.DistanceAlgorithm = EAttenuationDistanceModel::Linear;
	Settings.bEnableOcclusion = true;
	Settings.OcclusionTraceChannel = ECC_Visibility;
	Settings.OcclusionLowPassFilterFrequency = 1000.0f;
	Settings.OcclusionVolumeAttenuation = 0.5f;
	Settings.bEnableReverbSend = true;
	Settings.ReverbWetLevelMin = 0.3f;
	Settings.ReverbWetLevelMax = 0.8f;
	return Settings;
}

FSoundAttenuationSettings UHorrorAudioAttenuationPresets::CreateFootstepAttenuation()
{
	FSoundAttenuationSettings Settings;
	Settings.bAttenuate = true;
	Settings.bSpatialize = true;
	Settings.AttenuationShape = EAttenuationShape::Sphere;
	Settings.AttenuationShapeExtents = FVector(1500.0f, 0.0f, 0.0f);
	Settings.FalloffDistance = 1200.0f;
	Settings.DistanceAlgorithm = EAttenuationDistanceModel::Logarithmic;
	Settings.bEnableOcclusion = true;
	Settings.OcclusionTraceChannel = ECC_Visibility;
	Settings.OcclusionLowPassFilterFrequency = 800.0f;
	Settings.OcclusionVolumeAttenuation = 0.4f;
	Settings.bEnableReverbSend = true;
	Settings.ReverbWetLevelMin = 0.1f;
	Settings.ReverbWetLevelMax = 0.4f;
	return Settings;
}

FSoundAttenuationSettings UHorrorAudioAttenuationPresets::CreateInteractionAttenuation()
{
	FSoundAttenuationSettings Settings;
	Settings.bAttenuate = true;
	Settings.bSpatialize = true;
	Settings.AttenuationShape = EAttenuationShape::Sphere;
	Settings.AttenuationShapeExtents = FVector(800.0f, 0.0f, 0.0f);
	Settings.FalloffDistance = 600.0f;
	Settings.DistanceAlgorithm = EAttenuationDistanceModel::Logarithmic;
	Settings.bEnableOcclusion = true;
	Settings.OcclusionTraceChannel = ECC_Visibility;
	Settings.OcclusionLowPassFilterFrequency = 600.0f;
	Settings.OcclusionVolumeAttenuation = 0.3f;
	Settings.bEnableReverbSend = true;
	Settings.ReverbWetLevelMin = 0.2f;
	Settings.ReverbWetLevelMax = 0.5f;
	return Settings;
}

FSoundAttenuationSettings UHorrorAudioAttenuationPresets::CreateHorrorEffectAttenuation()
{
	FSoundAttenuationSettings Settings;
	Settings.bAttenuate = true;
	Settings.bSpatialize = true;
	Settings.AttenuationShape = EAttenuationShape::Sphere;
	Settings.AttenuationShapeExtents = FVector(3000.0f, 0.0f, 0.0f);
	Settings.FalloffDistance = 2500.0f;
	Settings.DistanceAlgorithm = EAttenuationDistanceModel::NaturalSound;
	Settings.bEnableOcclusion = false;
	Settings.bEnableReverbSend = true;
	Settings.ReverbWetLevelMin = 0.5f;
	Settings.ReverbWetLevelMax = 1.0f;
	Settings.bEnableListenerFocus = true;
	Settings.FocusAzimuth = 30.0f;
	Settings.NonFocusAzimuth = 60.0f;
	Settings.FocusDistanceScale = 1.0f;
	Settings.NonFocusDistanceScale = 1.0f;
	Settings.FocusPriorityScale = 1.0f;
	Settings.NonFocusPriorityScale = 0.6f;
	Settings.FocusVolumeAttenuation = 1.0f;
	Settings.NonFocusVolumeAttenuation = 0.7f;
	return Settings;
}

FSoundAttenuationSettings UHorrorAudioAttenuationPresets::CreateDistantAttenuation()
{
	FSoundAttenuationSettings Settings;
	Settings.bAttenuate = true;
	Settings.bSpatialize = true;
	Settings.AttenuationShape = EAttenuationShape::Sphere;
	Settings.AttenuationShapeExtents = FVector(10000.0f, 0.0f, 0.0f);
	Settings.FalloffDistance = 8000.0f;
	Settings.DistanceAlgorithm = EAttenuationDistanceModel::Linear;
	Settings.bEnableOcclusion = true;
	Settings.OcclusionTraceChannel = ECC_Visibility;
	Settings.OcclusionLowPassFilterFrequency = 500.0f;
	Settings.OcclusionVolumeAttenuation = 0.6f;
	Settings.bEnableReverbSend = true;
	Settings.ReverbWetLevelMin = 0.4f;
	Settings.ReverbWetLevelMax = 0.9f;
	return Settings;
}

FSoundAttenuationSettings UHorrorAudioAttenuationPresets::CreateCloseRangeAttenuation()
{
	FSoundAttenuationSettings Settings;
	Settings.bAttenuate = true;
	Settings.bSpatialize = true;
	Settings.AttenuationShape = EAttenuationShape::Sphere;
	Settings.AttenuationShapeExtents = FVector(500.0f, 0.0f, 0.0f);
	Settings.FalloffDistance = 400.0f;
	Settings.DistanceAlgorithm = EAttenuationDistanceModel::Logarithmic;
	Settings.bEnableOcclusion = true;
	Settings.OcclusionTraceChannel = ECC_Visibility;
	Settings.OcclusionLowPassFilterFrequency = 700.0f;
	Settings.OcclusionVolumeAttenuation = 0.35f;
	Settings.bEnableReverbSend = true;
	Settings.ReverbWetLevelMin = 0.15f;
	Settings.ReverbWetLevelMax = 0.35f;
	return Settings;
}

FSoundAttenuationSettings UHorrorAudioAttenuationPresets::CreateUnderwaterAttenuation()
{
	FSoundAttenuationSettings Settings;
	Settings.bAttenuate = true;
	Settings.bSpatialize = true;
	Settings.AttenuationShape = EAttenuationShape::Sphere;
	Settings.AttenuationShapeExtents = FVector(2000.0f, 0.0f, 0.0f);
	Settings.FalloffDistance = 1500.0f;
	Settings.DistanceAlgorithm = EAttenuationDistanceModel::Linear;
	Settings.bEnableOcclusion = true;
	Settings.OcclusionTraceChannel = ECC_Visibility;
	Settings.OcclusionLowPassFilterFrequency = 400.0f;
	Settings.OcclusionVolumeAttenuation = 0.7f;
	Settings.bEnableReverbSend = true;
	Settings.ReverbWetLevelMin = 0.6f;
	Settings.ReverbWetLevelMax = 1.0f;
	Settings.bAttenuateWithLPF = true;
	Settings.LPFRadiusMin = 0.0f;
	Settings.LPFRadiusMax = Settings.FalloffDistance;
	Settings.LPFFrequencyAtMin = 800.0f;
	Settings.LPFFrequencyAtMax = 400.0f;
	return Settings;
}
