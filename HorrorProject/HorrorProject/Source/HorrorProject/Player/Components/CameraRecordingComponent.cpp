// Copyright Epic Games, Inc. All Rights Reserved.

#include "CameraRecordingComponent.h"
#include "QuantumCameraComponent.h"
#include "Audio/HorrorAudioSubsystem.h"
#include "Game/HorrorEventBusSubsystem.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"

namespace
{
	FGameplayTag ResolveRecordingTag(const TCHAR* TagName)
	{
		return FGameplayTag::RequestGameplayTag(FName(TagName), false);
	}
}

UCameraRecordingComponent::UCameraRecordingComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	RecordingStartedEventTag = ResolveRecordingTag(TEXT("Horror.Camera.Recording.Started"));
	RecordingStoppedEventTag = ResolveRecordingTag(TEXT("Horror.Camera.Recording.Stopped"));
	RewindStartedEventTag = ResolveRecordingTag(TEXT("Horror.Camera.Recording.RewindStarted"));
	RewindStoppedEventTag = ResolveRecordingTag(TEXT("Horror.Camera.Recording.RewindStopped"));
}

void UCameraRecordingComponent::OnRegister()
{
	Super::OnRegister();
	ResolveCameraComponent();
}

void UCameraRecordingComponent::BeginPlay()
{
	Super::BeginPlay();

	ResolveCameraComponent();
	if (!CameraComponent)
	{
		AActor* Owner = GetOwner();
		UE_LOG(LogTemp, Warning, TEXT("CameraRecordingComponent: No QuantumCameraComponent found on %s"), Owner ? *Owner->GetName() : TEXT("unknown owner"));
	}
}

void UCameraRecordingComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (CameraComponent)
	{
		CameraComponent->OnCameraModeChanged.RemoveDynamic(this, &UCameraRecordingComponent::HandleQuantumCameraModeChanged);
	}
	CameraComponent = nullptr;
	Super::EndPlay(EndPlayReason);
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

	if (!CameraComponent)
	{
		ResolveCameraComponent();
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
	if (bIsRewinding || RecordingFrameCount == 0)
	{
		return false;
	}

	if (!CameraComponent)
	{
		ResolveCameraComponent();
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
	CurrentRewindFrameIndex = RecordingFrameCount - 1;

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
	RecordingStartIndex = 0;
	RecordingFrameCount = 0;
	CurrentRecordingDuration = 0.0f;
	FrameCaptureAccumulator = 0.0f;
	RewindPlaybackTime = 0.0f;
	CurrentRewindFrameIndex = 0;
}

FCameraRecordingMetadata UCameraRecordingComponent::GetRecordingMetadata() const
{
	FCameraRecordingMetadata Metadata;
	Metadata.TotalDuration = CurrentRecordingDuration;
	Metadata.FrameCount = RecordingFrameCount;
	Metadata.RecordingStartTime = RecordingStartTimestamp;
	Metadata.bHasAudio = RecordingStartSound != nullptr;

	if (const FCameraRecordingFrame* FirstFrame = GetRecordingFrameChronological(0))
	{
		Metadata.StartLocation = FirstFrame->Location;
	}

	return Metadata;
}

void UCameraRecordingComponent::SetMaxRecordingDuration(float NewDuration)
{
	MaxRecordingDuration = FMath::Max(1.0f, NewDuration);
}

#if WITH_DEV_AUTOMATION_TESTS
bool UCameraRecordingComponent::GetRecordedFrameForTests(int32 ChronologicalIndex, FCameraRecordingFrame& OutFrame) const
{
	if (const FCameraRecordingFrame* Frame = GetRecordingFrameChronological(ChronologicalIndex))
	{
		OutFrame = *Frame;
		return true;
	}

	return false;
}

void UCameraRecordingComponent::SetMaxBufferFramesForTests(int32 NewMaxBufferFrames)
{
	MaxBufferFrames = FMath::Max(1, NewMaxBufferFrames);
	EnsureRecordingBufferCapacity();
}
#endif

void UCameraRecordingComponent::HandleQuantumCameraModeChanged(EQuantumCameraMode NewMode)
{
	if (NewMode == EQuantumCameraMode::Recording)
	{
		if (!bIsRecording)
		{
			StartRecording();
		}
		return;
	}

	if (bIsRecording)
	{
		StopRecording();
	}

	if (NewMode == EQuantumCameraMode::Rewind)
	{
		if (!bIsRewinding)
		{
			StartRewind();
		}
		return;
	}

	if (bIsRewinding)
	{
		StopRewind();
	}
}

void UCameraRecordingComponent::ResolveCameraComponent()
{
	AActor* Owner = GetOwner();
	UQuantumCameraComponent* ResolvedCameraComponent = Owner ? Owner->FindComponentByClass<UQuantumCameraComponent>() : nullptr;
	if (CameraComponent == ResolvedCameraComponent)
	{
		return;
	}

	if (CameraComponent)
	{
		CameraComponent->OnCameraModeChanged.RemoveDynamic(this, &UCameraRecordingComponent::HandleQuantumCameraModeChanged);
	}

	CameraComponent = ResolvedCameraComponent;
	if (CameraComponent)
	{
		CameraComponent->OnCameraModeChanged.RemoveDynamic(this, &UCameraRecordingComponent::HandleQuantumCameraModeChanged);
		CameraComponent->OnCameraModeChanged.AddDynamic(this, &UCameraRecordingComponent::HandleQuantumCameraModeChanged);
	}
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

		AppendRecordingFrame(MoveTemp(NewFrame));

		OnRecordingProgress.Broadcast(CurrentRecordingDuration);
	}
}

void UCameraRecordingComponent::ProcessRewind(float DeltaTime)
{
	if (RecordingFrameCount == 0)
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

		const FCameraRecordingFrame* Frame = GetRecordingFrameChronological(CurrentRewindFrameIndex);
		if (!Frame)
		{
			StopRewind();
			return;
		}

		OnRewindProgress.Broadcast(Frame->Timestamp);
	}
}

void UCameraRecordingComponent::AppendRecordingFrame(FCameraRecordingFrame&& NewFrame)
{
	const int32 BufferCapacity = FMath::Max(1, MaxBufferFrames);
	if (RecordingBuffer.Num() < BufferCapacity)
	{
		RecordingBuffer.Add(MoveTemp(NewFrame));
		++RecordingFrameCount;
		return;
	}

	if (RecordingBuffer.Num() > BufferCapacity)
	{
		EnsureRecordingBufferCapacity();
	}

	if (RecordingFrameCount < RecordingBuffer.Num())
	{
		const int32 WriteIndex = (RecordingStartIndex + RecordingFrameCount) % RecordingBuffer.Num();
		RecordingBuffer[WriteIndex] = MoveTemp(NewFrame);
		++RecordingFrameCount;
		return;
	}

	RecordingBuffer[RecordingStartIndex] = MoveTemp(NewFrame);
	RecordingStartIndex = (RecordingStartIndex + 1) % RecordingBuffer.Num();
	RecordingFrameCount = RecordingBuffer.Num();
}

void UCameraRecordingComponent::EnsureRecordingBufferCapacity()
{
	const int32 BufferCapacity = FMath::Max(1, MaxBufferFrames);
	if (RecordingFrameCount <= 0)
	{
		RecordingBuffer.Empty();
		RecordingStartIndex = 0;
		RecordingFrameCount = 0;
		return;
	}

	if (RecordingBuffer.Num() <= BufferCapacity && RecordingFrameCount <= BufferCapacity)
	{
		return;
	}

	TArray<FCameraRecordingFrame> PreservedFrames;
	const int32 FramesToKeep = FMath::Min(RecordingFrameCount, BufferCapacity);
	PreservedFrames.Reserve(FramesToKeep);
	const int32 FirstFrameToKeep = RecordingFrameCount - FramesToKeep;
	for (int32 FrameIndex = 0; FrameIndex < FramesToKeep; ++FrameIndex)
	{
		if (const FCameraRecordingFrame* Frame = GetRecordingFrameChronological(FirstFrameToKeep + FrameIndex))
		{
			PreservedFrames.Add(*Frame);
		}
	}

	RecordingBuffer = MoveTemp(PreservedFrames);
	RecordingStartIndex = 0;
	RecordingFrameCount = RecordingBuffer.Num();
}

const FCameraRecordingFrame* UCameraRecordingComponent::GetRecordingFrameChronological(int32 ChronologicalIndex) const
{
	if (ChronologicalIndex < 0 || ChronologicalIndex >= RecordingFrameCount || RecordingBuffer.IsEmpty())
	{
		return nullptr;
	}

	const int32 PhysicalIndex = (RecordingStartIndex + ChronologicalIndex) % RecordingBuffer.Num();
	if (!RecordingBuffer.IsValidIndex(PhysicalIndex))
	{
		return nullptr;
	}

	return &RecordingBuffer[PhysicalIndex];
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
