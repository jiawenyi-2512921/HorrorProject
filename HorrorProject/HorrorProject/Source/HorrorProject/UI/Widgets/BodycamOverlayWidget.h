// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Player/Components/QuantumCameraComponent.h"
#include "BodycamOverlayWidget.generated.h"

/**
 * Bodycam HUD overlay with recording indicator, timestamp, battery, and VHS effects
 */
UCLASS(Abstract, BlueprintType, Blueprintable)
class HORRORPROJECT_API UBodycamOverlayWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UFUNCTION(BlueprintCallable, Category="Bodycam|UI")
	void SetRecordingState(bool bIsRecording);

	UFUNCTION(BlueprintCallable, Category="Bodycam|UI")
	void SetBatteryLevel(float Percent);

	UFUNCTION(BlueprintCallable, Category="Bodycam|UI")
	void SetCameraMode(EQuantumCameraMode Mode);

	UFUNCTION(BlueprintCallable, Category="Bodycam|UI")
	void SetTimestamp(const FDateTime& Time);

	UFUNCTION(BlueprintCallable, Category="Bodycam|UI")
	void SetVHSIntensity(float Intensity);

	UFUNCTION(BlueprintCallable, Category="Bodycam|UI")
	void TriggerGlitchEffect(float Duration = 0.5f);

	UFUNCTION(BlueprintPure, Category="Bodycam|UI")
	bool IsRecording() const { return bIsRecording; }

	UFUNCTION(BlueprintPure, Category="Bodycam|UI")
	float GetBatteryLevel() const { return BatteryLevel; }

protected:
	UFUNCTION(BlueprintImplementableEvent, Category="Bodycam|UI", meta=(DisplayName="录制状态已变化"))
	void BP_RecordingStateChanged(bool bRecording);

	UFUNCTION(BlueprintImplementableEvent, Category="Bodycam|UI", meta=(DisplayName="电量已变化"))
	void BP_BatteryLevelChanged(float Percent);

	UFUNCTION(BlueprintImplementableEvent, Category="Bodycam|UI", meta=(DisplayName="摄像机模式已变化"))
	void BP_CameraModeChanged(EQuantumCameraMode Mode);

	UFUNCTION(BlueprintImplementableEvent, Category="Bodycam|UI", meta=(DisplayName="时间戳已更新"))
	void BP_TimestampUpdated(const FString& FormattedTime);

	UFUNCTION(BlueprintImplementableEvent, Category="Bodycam|UI", meta=(DisplayName="录像带强度已变化"))
	void BP_VHSIntensityChanged(float Intensity);

	UFUNCTION(BlueprintImplementableEvent, Category="Bodycam|UI", meta=(DisplayName="故障效果已触发"))
	void BP_GlitchEffectTriggered(float Duration);

	UPROPERTY(BlueprintReadOnly, Category="Bodycam|UI")
	bool bIsRecording = false;

	UPROPERTY(BlueprintReadOnly, Category="Bodycam|UI")
	float BatteryLevel = 1.0f;

	UPROPERTY(BlueprintReadOnly, Category="Bodycam|UI")
	EQuantumCameraMode CurrentCameraMode = EQuantumCameraMode::Disabled;

	UPROPERTY(BlueprintReadOnly, Category="Bodycam|UI")
	float VHSIntensity = 0.0f;

	UPROPERTY(EditDefaultsOnly, Category="Bodycam|UI")
	FString TimestampFormat = TEXT("{0:02d}:{1:02d}:{2:02d}");

	UPROPERTY(EditDefaultsOnly, Category="Bodycam|UI")
	float RecordingBlinkRate = 1.0f;

private:
	float RecordingBlinkTimer = 0.0f;
	float GlitchTimer = 0.0f;
	float GlitchDuration = 0.0f;
};
