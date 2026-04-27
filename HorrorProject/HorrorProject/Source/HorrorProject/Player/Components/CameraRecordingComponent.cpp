// Copyright Epic Games, Inc. All Rights Reserved.

#include "CameraRecordingComponent.h"
#include "QuantumCameraComponent.h"
#include "Audio/HorrorAudioSubsystem.h"
#include "Game/HorrorEventBusSubsystem.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"

UCameraRecordingComponent::UCameraRecordingComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UCameraRecordingComponent::BeginPlay()
{
	Super::BeginPlay();

	AActor* Owner = GetOwner();
	CameraComponent = Owner ? Owner->FindComponentByClass<UQuantumCameraComponent>() : nullptr;
	if (!CameraComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("CameraRecordingComponent: No QuantumCameraComponent found on %s"), Owner ? *Owner->GetName() : TEXT("unknown owner"));
	}
}

void UCameraRecordingComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bIsRecording)
	{
		CaptureFrame(DeltaTime);
	}
	else if (bIsRewinding)
	{
		ProcessRewind(DeltaTime);
	}
}

bool UCameraRecordingComponent::StartRecording()
{
	if (bIsRecording)
	{
		return false;
	}

	if (!CameraComponent || !CameraComponent->IsCameraEnabled())
	{
		return false;
	}

	if (bIsRewinding)
	{
		StopRewind();
	}

	ClearRecording();

	bIsRecording = true;
	CurrentRecordingDuration = 0.0f;
	FrameCaptureAccumulator = 0.0f;
	RecordingStartTimestamp = FDateTime::Now();

	SetComponentTickEnabled(true);

	PlayRecordingSound(RecordingStartSound);
	PublishEvent(RecordingStartedEventTag);
	OnRecordingStarted.Broadcast();

	return true;
}

bool UCameraRecordingComponent::StopRecording()
{
	if (!bIsRecording)
	{
		return false;
	}

	bIsRecording = false;
	float FinalDuration = CurrentRecordingDuration;

	if (!bIsRewinding)
	{
		SetComponentTickEnabled(false);
	}

	PlayRecordingSound(RecordingStopSound);
	PublishEvent(RecordingStoppedEventTag);
	OnRecordingStopped.Broadcast(FinalDuration);

	return true;
}

bool UCameraRecordingComponent::StartRewind()
{
	if (bIsRewinding || RecordingBuffer.Num() == 0)
	{
		return false;
	}

	if (!CameraComponent || !CameraComponent->IsCameraEnabled())
	{
		return false;
	}

	if (bIsRecording)
	{
		StopRecording();
	}

	bIsRewinding = true;
	RewindPlaybackTime = 0.0f;
	CurrentRewindFrameIndex = RecordingBuffer.Num() - 1;

	SetComponentTickEnabled(true);

	PlayRecordingSound(RewindSound);
	PublishEvent(RewindStartedEventTag);
	OnRewindStarted.Broadcast();

	return true;
}

bool UCameraRecordingComponent::StopRewind()
{
	if (!bIsRewinding)
	{
		return false;
	}

	bIsRewinding = false;
	RewindPlaybackTime = 0.0f;
	CurrentRewindFrameIndex = 0;

	if (!bIsRecording)
	{
		SetComponentTickEnabled(false);
	}

	PublishEvent(RewindStoppedEventTag);
	OnRewindStopped.Broadcast();

	return true;
}

float UCameraRecordingComponent::GetRecordingProgress() const
{
	if (MaxRecordingDuration <= 0.0f)
	{
		return 0.0f;
	}
	return FMath::Clamp(CurrentRecordingDuration / MaxRecordingDuration, 0.0f, 1.0f);
}

void UCameraRecordingComponent::ClearRecording()
{
	RecordingBuffer.Empty();
	CurrentRecordingDuration = 0.0f;
	FrameCaptureAccumulator = 0.0f;
	RewindPlaybackTime = 0.0f;
	CurrentRewindFrameIndex = 0;
}

FCameraRecordingMetadata UCameraRecordingComponent::GetRecordingMetadata() const
{
	FCameraRecordingMetadata Metadata;
	Metadata.TotalDuration = CurrentRecordingDuration;
	Metadata.FrameCount = RecordingBuffer.Num();
	Metadata.RecordingStartTime = RecordingStartTimestamp;
	Metadata.bHasAudio = RecordingStartSound != nullptr;

	if (!RecordingBuffer.IsEmpty())
	{
		Metadata.StartLocation = RecordingBuffer.GetData()->Location;
	}

	return Metadata;
}

void UCameraRecordingComponent::SetMaxRecordingDuration(float NewDuration)
{
	MaxRecordingDuration = FMath::Max(1.0f, NewDuration);
}

void UCameraRecordingComponent::CaptureFrame(float DeltaTime)
{
	CurrentRecordingDuration += DeltaTime;
	FrameCaptureAccumulator += DeltaTime;

	if (CurrentRecordingDuration >= MaxRecordingDuration)
	{
		StopRecording();
		return;
	}

	const float FrameInterval = 1.0f / FrameCaptureRate;
	if (FrameCaptureAccumulator >= FrameInterval)
	{
		FrameCaptureAccumulator -= FrameInterval;

		if (RecordingBuffer.Num() >= MaxBufferFrames)
		{
			RecordingBuffer.RemoveAt(0);
		}

		AActor* Owner = GetOwner();
		if (!Owner)
		{
			StopRecording();
			return;
		}

		FCameraRecordingFrame NewFrame;
		NewFrame.Location = Owner->GetActorLocation();
		NewFrame.Rotation = Owner->GetActorRotation();
		NewFrame.Timestamp = CurrentRecordingDuration;

		RecordingBuffer.Add(NewFrame);

		OnRecordingProgress.Broadcast(CurrentRecordingDuration);
	}
}

void UCameraRecordingComponent::ProcessRewind(float DeltaTime)
{
	if (RecordingBuffer.Num() == 0)
	{
		StopRewind();
		return;
	}

	RewindPlaybackTime += DeltaTime * RewindSpeed;

	const float FrameInterval = 1.0f / FrameCaptureRate;
	const int32 FramesToRewind = FMath::FloorToInt(RewindPlaybackTime / FrameInterval);

	if (FramesToRewind > 0)
	{
		RewindPlaybackTime -= FramesToRewind * FrameInterval;
		CurrentRewindFrameIndex = FMath::Max(0, CurrentRewindFrameIndex - FramesToRewind);

		if (CurrentRewindFrameIndex <= 0)
		{
			CurrentRewindFrameIndex = 0;
			StopRewind();
			return;
		}

		if (!RecordingBuffer.IsValidIndex(CurrentRewindFrameIndex))
		{
			StopRewind();
			return;
		}

		const FCameraRecordingFrame& Frame = *(RecordingBuffer.GetData() + CurrentRewindFrameIndex);
		OnRewindProgress.Broadcast(Frame.Timestamp);
	}
}

void UCameraRecordingComponent::PublishEvent(FGameplayTag EventTag)
{
	if (!EventTag.IsValid())
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	UHorrorEventBusSubsystem* EventBus = World->GetSubsystem<UHorrorEventBusSubsystem>();
	if (EventBus)
	{
		EventBus->Publish(EventTag, FName("CameraRecording"), FGameplayTag::EmptyTag, GetOwner());
	}
}

void UCameraRecordingComponent::PlayRecordingSound(USoundBase* Sound)
{
	if (!Sound)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	UHorrorAudioSubsystem* AudioSubsystem = World->GetSubsystem<UHorrorAudioSubsystem>();
	if (AudioSubsystem)
	{
		AudioSubsystem->PlaySound2D(Sound, 1.0f);
	}
	else
	{
		UGameplayStatics::PlaySound2D(World, Sound);
	}
}
