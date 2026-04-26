// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "CameraRecordingComponent.generated.h"

class UQuantumCameraComponent;
class USoundBase;

USTRUCT(BlueprintType)
struct HORRORPROJECT_API FCameraRecordingFrame
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category="Recording")
	FVector Location = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Category="Recording")
	FRotator Rotation = FRotator::ZeroRotator;

	UPROPERTY(BlueprintReadOnly, Category="Recording")
	float Timestamp = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category="Recording")
	TArray<uint8> FrameData;
};

USTRUCT(BlueprintType)
struct HORRORPROJECT_API FCameraRecordingMetadata
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category="Recording")
	float TotalDuration = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category="Recording")
	int32 FrameCount = 0;

	UPROPERTY(BlueprintReadOnly, Category="Recording")
	FDateTime RecordingStartTime;

	UPROPERTY(BlueprintReadOnly, Category="Recording")
	FVector StartLocation = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Category="Recording")
	bool bHasAudio = false;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRecordingStartedSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRecordingStoppedSignature, float, Duration);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRecordingProgressSignature, float, CurrentDuration);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRewindStartedSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRewindStoppedSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRewindProgressSignature, float, RewindTime);

UCLASS(ClassGroup=(Horror), BlueprintType, Blueprintable, meta=(BlueprintSpawnableComponent))
class HORRORPROJECT_API UCameraRecordingComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCameraRecordingComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, Category="CameraRecording")
	bool StartRecording();

	UFUNCTION(BlueprintCallable, Category="CameraRecording")
	bool StopRecording();

	UFUNCTION(BlueprintCallable, Category="CameraRecording")
	bool StartRewind();

	UFUNCTION(BlueprintCallable, Category="CameraRecording")
	bool StopRewind();

	UFUNCTION(BlueprintPure, Category="CameraRecording")
	bool IsRecording() const { return bIsRecording; }

	UFUNCTION(BlueprintPure, Category="CameraRecording")
	bool IsRewinding() const { return bIsRewinding; }

	UFUNCTION(BlueprintPure, Category="CameraRecording")
	float GetCurrentRecordingDuration() const { return CurrentRecordingDuration; }

	UFUNCTION(BlueprintPure, Category="CameraRecording")
	float GetMaxRecordingDuration() const { return MaxRecordingDuration; }

	UFUNCTION(BlueprintPure, Category="CameraRecording")
	float GetRecordingProgress() const;

	UFUNCTION(BlueprintPure, Category="CameraRecording")
	bool HasRecording() const { return RecordingBuffer.Num() > 0; }

	UFUNCTION(BlueprintCallable, Category="CameraRecording")
	void ClearRecording();

	UFUNCTION(BlueprintPure, Category="CameraRecording")
	FCameraRecordingMetadata GetRecordingMetadata() const;

	UFUNCTION(BlueprintCallable, Category="CameraRecording")
	void SetMaxRecordingDuration(float NewDuration);

	UPROPERTY(BlueprintAssignable, Category="CameraRecording")
	FOnRecordingStartedSignature OnRecordingStarted;

	UPROPERTY(BlueprintAssignable, Category="CameraRecording")
	FOnRecordingStoppedSignature OnRecordingStopped;

	UPROPERTY(BlueprintAssignable, Category="CameraRecording")
	FOnRecordingProgressSignature OnRecordingProgress;

	UPROPERTY(BlueprintAssignable, Category="CameraRecording")
	FOnRewindStartedSignature OnRewindStarted;

	UPROPERTY(BlueprintAssignable, Category="CameraRecording")
	FOnRewindStoppedSignature OnRewindStopped;

	UPROPERTY(BlueprintAssignable, Category="CameraRecording")
	FOnRewindProgressSignature OnRewindProgress;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, Category="CameraRecording|Settings")
	float MaxRecordingDuration = 30.0f;

	UPROPERTY(EditDefaultsOnly, Category="CameraRecording|Settings")
	float FrameCaptureRate = 30.0f;

	UPROPERTY(EditDefaultsOnly, Category="CameraRecording|Settings")
	int32 MaxBufferFrames = 900;

	UPROPERTY(EditDefaultsOnly, Category="CameraRecording|Settings")
	float RewindSpeed = 2.0f;

	UPROPERTY(EditDefaultsOnly, Category="CameraRecording|Audio")
	TObjectPtr<USoundBase> RecordingStartSound;

	UPROPERTY(EditDefaultsOnly, Category="CameraRecording|Audio")
	TObjectPtr<USoundBase> RecordingStopSound;

	UPROPERTY(EditDefaultsOnly, Category="CameraRecording|Audio")
	TObjectPtr<USoundBase> RewindSound;

	UPROPERTY(EditDefaultsOnly, Category="CameraRecording|Events")
	FGameplayTag RecordingStartedEventTag;

	UPROPERTY(EditDefaultsOnly, Category="CameraRecording|Events")
	FGameplayTag RecordingStoppedEventTag;

	UPROPERTY(EditDefaultsOnly, Category="CameraRecording|Events")
	FGameplayTag RewindStartedEventTag;

	UPROPERTY(EditDefaultsOnly, Category="CameraRecording|Events")
	FGameplayTag RewindStoppedEventTag;

private:
	void CaptureFrame(float DeltaTime);
	void ProcessRewind(float DeltaTime);
	void PublishEvent(FGameplayTag EventTag);
	void PlayRecordingSound(USoundBase* Sound);

	UPROPERTY(Transient)
	TObjectPtr<UQuantumCameraComponent> CameraComponent;

	UPROPERTY(Transient)
	TArray<FCameraRecordingFrame> RecordingBuffer;

	UPROPERTY(Transient)
	bool bIsRecording = false;

	UPROPERTY(Transient)
	bool bIsRewinding = false;

	UPROPERTY(Transient)
	float CurrentRecordingDuration = 0.0f;

	UPROPERTY(Transient)
	float FrameCaptureAccumulator = 0.0f;

	UPROPERTY(Transient)
	float RewindPlaybackTime = 0.0f;

	UPROPERTY(Transient)
	int32 CurrentRewindFrameIndex = 0;

	UPROPERTY(Transient)
	FDateTime RecordingStartTimestamp;
};
