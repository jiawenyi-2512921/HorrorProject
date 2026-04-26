// Copyright Epic Games, Inc. All Rights Reserved.

#include "BodycamOverlayWidget.h"

void UBodycamOverlayWidget::NativeConstruct()
{
	Super::NativeConstruct();

	BP_RecordingStateChanged(bIsRecording);
	BP_BatteryLevelChanged(BatteryLevel);
	BP_CameraModeChanged(CurrentCameraMode);
	BP_VHSIntensityChanged(VHSIntensity);
}

void UBodycamOverlayWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (bIsRecording)
	{
		RecordingBlinkTimer += InDeltaTime;
	}

	if (GlitchTimer > 0.0f)
	{
		GlitchTimer -= InDeltaTime;
		if (GlitchTimer <= 0.0f)
		{
			GlitchTimer = 0.0f;
		}
	}
}

void UBodycamOverlayWidget::SetRecordingState(bool bRecording)
{
	if (bIsRecording != bRecording)
	{
		bIsRecording = bRecording;
		RecordingBlinkTimer = 0.0f;
		BP_RecordingStateChanged(bIsRecording);
	}
}

void UBodycamOverlayWidget::SetBatteryLevel(float Percent)
{
	const float ClampedPercent = FMath::Clamp(Percent, 0.0f, 1.0f);
	if (!FMath::IsNearlyEqual(BatteryLevel, ClampedPercent))
	{
		BatteryLevel = ClampedPercent;
		BP_BatteryLevelChanged(BatteryLevel);
	}
}

void UBodycamOverlayWidget::SetCameraMode(EQuantumCameraMode Mode)
{
	if (CurrentCameraMode != Mode)
	{
		CurrentCameraMode = Mode;
		BP_CameraModeChanged(CurrentCameraMode);
	}
}

void UBodycamOverlayWidget::SetTimestamp(const FDateTime& Time)
{
	const FString FormattedTime = FString::Printf(TEXT("%02d:%02d:%02d"),
		Time.GetHour(), Time.GetMinute(), Time.GetSecond());
	BP_TimestampUpdated(FormattedTime);
}

void UBodycamOverlayWidget::SetVHSIntensity(float Intensity)
{
	const float ClampedIntensity = FMath::Clamp(Intensity, 0.0f, 1.0f);
	if (!FMath::IsNearlyEqual(VHSIntensity, ClampedIntensity))
	{
		VHSIntensity = ClampedIntensity;
		BP_VHSIntensityChanged(VHSIntensity);
	}
}

void UBodycamOverlayWidget::TriggerGlitchEffect(float Duration)
{
	GlitchDuration = FMath::Max(Duration, 0.1f);
	GlitchTimer = GlitchDuration;
	BP_GlitchEffectTriggered(GlitchDuration);
}
