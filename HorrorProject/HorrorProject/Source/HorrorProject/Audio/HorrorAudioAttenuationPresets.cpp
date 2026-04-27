// Copyright Epic Games, Inc. All Rights Reserved.

#include "Audio/HorrorAudioAttenuationPresets.h"

namespace HorrorAudioAttenuation
{
	constexpr float AmbientRadiusCm = 5000.0f;
	constexpr float AmbientFalloffCm = 4000.0f;
	constexpr float AmbientOcclusionLowPassHz = 1000.0f;
	constexpr float AmbientOcclusionVolumeAttenuation = 0.5f;
	constexpr float AmbientReverbMin = 0.3f;
	constexpr float AmbientReverbMax = 0.8f;

	constexpr float FootstepRadiusCm = 1500.0f;
	constexpr float FootstepFalloffCm = 1200.0f;
	constexpr float FootstepOcclusionLowPassHz = 800.0f;
	constexpr float FootstepOcclusionVolumeAttenuation = 0.4f;
	constexpr float FootstepReverbMin = 0.1f;
	constexpr float FootstepReverbMax = 0.4f;

	constexpr float InteractionRadiusCm = 800.0f;
	constexpr float InteractionFalloffCm = 600.0f;
	constexpr float InteractionOcclusionLowPassHz = 600.0f;
	constexpr float InteractionOcclusionVolumeAttenuation = 0.3f;
	constexpr float InteractionReverbMin = 0.2f;
	constexpr float InteractionReverbMax = 0.5f;

	constexpr float HorrorEffectRadiusCm = 3000.0f;
	constexpr float HorrorEffectFalloffCm = 2500.0f;
	constexpr float HorrorEffectReverbMin = 0.5f;
	constexpr float HorrorEffectReverbMax = 1.0f;
	constexpr float HorrorEffectFocusAzimuthDegrees = 30.0f;
	constexpr float HorrorEffectNonFocusAzimuthDegrees = 60.0f;
	constexpr float HorrorEffectNonFocusPriorityScale = 0.6f;
	constexpr float HorrorEffectNonFocusVolumeAttenuation = 0.7f;

	constexpr float DistantRadiusCm = 10000.0f;
	constexpr float DistantFalloffCm = 8000.0f;
	constexpr float DistantOcclusionLowPassHz = 500.0f;
	constexpr float DistantOcclusionVolumeAttenuation = 0.6f;
	constexpr float DistantReverbMin = 0.4f;
	constexpr float DistantReverbMax = 0.9f;

	constexpr float CloseRangeRadiusCm = 500.0f;
	constexpr float CloseRangeFalloffCm = 400.0f;
	constexpr float CloseRangeOcclusionLowPassHz = 700.0f;
	constexpr float CloseRangeOcclusionVolumeAttenuation = 0.35f;
	constexpr float CloseRangeReverbMin = 0.15f;
	constexpr float CloseRangeReverbMax = 0.35f;

	constexpr float UnderwaterRadiusCm = 2000.0f;
	constexpr float UnderwaterFalloffCm = 1500.0f;
	constexpr float UnderwaterOcclusionLowPassHz = 400.0f;
	constexpr float UnderwaterOcclusionVolumeAttenuation = 0.7f;
	constexpr float UnderwaterReverbMin = 0.6f;
	constexpr float UnderwaterReverbMax = 1.0f;
	constexpr float UnderwaterLpfFrequencyAtMinHz = 800.0f;
	constexpr float UnderwaterLpfFrequencyAtMaxHz = 400.0f;

	FSoundAttenuationSettings CreateBaseAttenuation(
		float RadiusCm,
		float FalloffCm,
		EAttenuationDistanceModel DistanceModel,
		bool bEnableOcclusion,
		float OcclusionLowPassHz,
		float OcclusionVolumeAttenuation,
		float ReverbWetLevelMin,
		float ReverbWetLevelMax)
	{
		FSoundAttenuationSettings Settings;
		Settings.bAttenuate = true;
		Settings.bSpatialize = true;
		Settings.AttenuationShape = EAttenuationShape::Sphere;
		Settings.AttenuationShapeExtents = FVector(RadiusCm, 0.0f, 0.0f);
		Settings.FalloffDistance = FalloffCm;
		Settings.DistanceAlgorithm = DistanceModel;
		Settings.bEnableOcclusion = bEnableOcclusion;
		Settings.OcclusionTraceChannel = ECC_Visibility;
		Settings.OcclusionLowPassFilterFrequency = OcclusionLowPassHz;
		Settings.OcclusionVolumeAttenuation = OcclusionVolumeAttenuation;
		Settings.bEnableReverbSend = true;
		Settings.ReverbWetLevelMin = ReverbWetLevelMin;
		Settings.ReverbWetLevelMax = ReverbWetLevelMax;
		return Settings;
	}
}

FSoundAttenuationSettings UHorrorAudioAttenuationPresets::CreateAmbientAttenuation()
{
	return HorrorAudioAttenuation::CreateBaseAttenuation(
		HorrorAudioAttenuation::AmbientRadiusCm,
		HorrorAudioAttenuation::AmbientFalloffCm,
		EAttenuationDistanceModel::Linear,
		true,
		HorrorAudioAttenuation::AmbientOcclusionLowPassHz,
		HorrorAudioAttenuation::AmbientOcclusionVolumeAttenuation,
		HorrorAudioAttenuation::AmbientReverbMin,
		HorrorAudioAttenuation::AmbientReverbMax);
}

FSoundAttenuationSettings UHorrorAudioAttenuationPresets::CreateFootstepAttenuation()
{
	return HorrorAudioAttenuation::CreateBaseAttenuation(
		HorrorAudioAttenuation::FootstepRadiusCm,
		HorrorAudioAttenuation::FootstepFalloffCm,
		EAttenuationDistanceModel::Logarithmic,
		true,
		HorrorAudioAttenuation::FootstepOcclusionLowPassHz,
		HorrorAudioAttenuation::FootstepOcclusionVolumeAttenuation,
		HorrorAudioAttenuation::FootstepReverbMin,
		HorrorAudioAttenuation::FootstepReverbMax);
}

FSoundAttenuationSettings UHorrorAudioAttenuationPresets::CreateInteractionAttenuation()
{
	return HorrorAudioAttenuation::CreateBaseAttenuation(
		HorrorAudioAttenuation::InteractionRadiusCm,
		HorrorAudioAttenuation::InteractionFalloffCm,
		EAttenuationDistanceModel::Logarithmic,
		true,
		HorrorAudioAttenuation::InteractionOcclusionLowPassHz,
		HorrorAudioAttenuation::InteractionOcclusionVolumeAttenuation,
		HorrorAudioAttenuation::InteractionReverbMin,
		HorrorAudioAttenuation::InteractionReverbMax);
}

FSoundAttenuationSettings UHorrorAudioAttenuationPresets::CreateHorrorEffectAttenuation()
{
	FSoundAttenuationSettings Settings = HorrorAudioAttenuation::CreateBaseAttenuation(
		HorrorAudioAttenuation::HorrorEffectRadiusCm,
		HorrorAudioAttenuation::HorrorEffectFalloffCm,
		EAttenuationDistanceModel::NaturalSound,
		false,
		0.0f,
		0.0f,
		HorrorAudioAttenuation::HorrorEffectReverbMin,
		HorrorAudioAttenuation::HorrorEffectReverbMax);
	Settings.bEnableListenerFocus = true;
	Settings.FocusAzimuth = HorrorAudioAttenuation::HorrorEffectFocusAzimuthDegrees;
	Settings.NonFocusAzimuth = HorrorAudioAttenuation::HorrorEffectNonFocusAzimuthDegrees;
	Settings.FocusDistanceScale = 1.0f;
	Settings.NonFocusDistanceScale = 1.0f;
	Settings.FocusPriorityScale = 1.0f;
	Settings.NonFocusPriorityScale = HorrorAudioAttenuation::HorrorEffectNonFocusPriorityScale;
	Settings.FocusVolumeAttenuation = 1.0f;
	Settings.NonFocusVolumeAttenuation = HorrorAudioAttenuation::HorrorEffectNonFocusVolumeAttenuation;
	return Settings;
}

FSoundAttenuationSettings UHorrorAudioAttenuationPresets::CreateDistantAttenuation()
{
	return HorrorAudioAttenuation::CreateBaseAttenuation(
		HorrorAudioAttenuation::DistantRadiusCm,
		HorrorAudioAttenuation::DistantFalloffCm,
		EAttenuationDistanceModel::Linear,
		true,
		HorrorAudioAttenuation::DistantOcclusionLowPassHz,
		HorrorAudioAttenuation::DistantOcclusionVolumeAttenuation,
		HorrorAudioAttenuation::DistantReverbMin,
		HorrorAudioAttenuation::DistantReverbMax);
}

FSoundAttenuationSettings UHorrorAudioAttenuationPresets::CreateCloseRangeAttenuation()
{
	return HorrorAudioAttenuation::CreateBaseAttenuation(
		HorrorAudioAttenuation::CloseRangeRadiusCm,
		HorrorAudioAttenuation::CloseRangeFalloffCm,
		EAttenuationDistanceModel::Logarithmic,
		true,
		HorrorAudioAttenuation::CloseRangeOcclusionLowPassHz,
		HorrorAudioAttenuation::CloseRangeOcclusionVolumeAttenuation,
		HorrorAudioAttenuation::CloseRangeReverbMin,
		HorrorAudioAttenuation::CloseRangeReverbMax);
}

FSoundAttenuationSettings UHorrorAudioAttenuationPresets::CreateUnderwaterAttenuation()
{
	FSoundAttenuationSettings Settings = HorrorAudioAttenuation::CreateBaseAttenuation(
		HorrorAudioAttenuation::UnderwaterRadiusCm,
		HorrorAudioAttenuation::UnderwaterFalloffCm,
		EAttenuationDistanceModel::Linear,
		true,
		HorrorAudioAttenuation::UnderwaterOcclusionLowPassHz,
		HorrorAudioAttenuation::UnderwaterOcclusionVolumeAttenuation,
		HorrorAudioAttenuation::UnderwaterReverbMin,
		HorrorAudioAttenuation::UnderwaterReverbMax);
	Settings.bAttenuateWithLPF = true;
	Settings.LPFRadiusMin = 0.0f;
	Settings.LPFRadiusMax = Settings.FalloffDistance;
	Settings.LPFFrequencyAtMin = HorrorAudioAttenuation::UnderwaterLpfFrequencyAtMinHz;
	Settings.LPFFrequencyAtMax = HorrorAudioAttenuation::UnderwaterLpfFrequencyAtMaxHz;
	return Settings;
}
