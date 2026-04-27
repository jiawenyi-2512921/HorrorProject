// Copyright Epic Games, Inc. All Rights Reserved.

#include "Interaction/RecorderInteractable.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Components/AudioComponent.h"
#include "Player/Components/InventoryComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"

namespace
{
	const FVector RecorderInteractionExtent(50.0f, 50.0f, 50.0f);
}

ARecorderInteractable::ARecorderInteractable()
{
	PrimaryActorTick.bCanEverTick = true;

	// Create components
	RecorderMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RecorderMesh"));
	RootComponent = RecorderMesh;
	RecorderMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	RecorderMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	RecorderMesh->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

	InteractionVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractionVolume"));
	InteractionVolume->SetupAttachment(RecorderMesh);
	InteractionVolume->SetBoxExtent(RecorderInteractionExtent);
	InteractionVolume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	InteractionVolume->SetCollisionResponseToAllChannels(ECR_Ignore);
	InteractionVolume->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

	AudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AudioComponent"));
	AudioComponent->SetupAttachment(RecorderMesh);
	AudioComponent->bAutoActivate = false;

	// Default settings
	InteractionPrompt = FText::FromString("Play Audio Log");
	bCanInteractMultipleTimes = true;
}

void ARecorderInteractable::BeginPlay()
{
	Super::BeginPlay();

	// Configure audio component
	if (AudioComponent && AudioLog)
	{
		AudioComponent->SetSound(AudioLog);
		AudioComponent->bIsUISound = !bUse3DAudio;

		if (bUse3DAudio)
		{
			AudioComponent->bOverrideAttenuation = true;
			AudioComponent->AttenuationOverrides.AttenuationShape = EAttenuationShape::Sphere;
			AudioComponent->AttenuationOverrides.FalloffDistance = MaxAudibleDistance;
		}
	}

	// Auto-generate evidence metadata if not set
	if (bIsEvidence && EvidenceMetadata.EvidenceId == NAME_None && InteractableId != NAME_None)
	{
		EvidenceMetadata.EvidenceId = InteractableId;
		EvidenceMetadata.DisplayName = AudioLogTitle;
		EvidenceMetadata.Description = FText::FromString("An audio recording containing important information.");
	}
}

void ARecorderInteractable::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (AudioFinishedTimer.IsValid())
	{
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(AudioFinishedTimer);
		}
	}

	Super::EndPlay(EndPlayReason);
}

void ARecorderInteractable::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Update playback state
	if (RecorderState == ERecorderState::Playing && AudioComponent && !AudioComponent->IsPlaying())
	{
		OnAudioFinished();
	}
}

bool ARecorderInteractable::CanInteract_Implementation(AActor* InstigatorActor, const FHitResult& Hit) const
{
	if (!Super::CanInteract_Implementation(InstigatorActor, Hit))
	{
		return false;
	}

	// Check if audio log is set
	if (!AudioLog)
	{
		return false;
	}

	return true;
}

void ARecorderInteractable::OnInteract(AActor* InstigatorActor, const FHitResult& Hit)
{
	Super::OnInteract(InstigatorActor, Hit);

	// Add to evidence if configured
	if (bAutoCollectAsEvidence && bIsEvidence && !bHasBeenListened)
	{
		TryAddToEvidence(InstigatorActor);
	}

	// Handle playback
	if (RecorderState == ERecorderState::Playing && bStopOnReInteract)
	{
		StopAudioLog();
	}
	else if (bAutoPlayOnInteract)
	{
		PlayAudioLog();
	}
}

void ARecorderInteractable::PlayAudioLog()
{
	if (!AudioComponent || !AudioLog)
	{
		return;
	}

	if (RecorderState == ERecorderState::Paused)
	{
		AudioComponent->SetPaused(false);
	}
	else
	{
		AudioComponent->Play();
	}

	RecorderState = ERecorderState::Playing;
	bHasBeenListened = true;
}

void ARecorderInteractable::StopAudioLog()
{
	if (!AudioComponent)
	{
		return;
	}

	AudioComponent->Stop();
	RecorderState = ERecorderState::Stopped;
}

void ARecorderInteractable::PauseAudioLog()
{
	if (!AudioComponent || RecorderState != ERecorderState::Playing)
	{
		return;
	}

	AudioComponent->SetPaused(true);
	RecorderState = ERecorderState::Paused;
}

float ARecorderInteractable::GetPlaybackProgress() const
{
	if (!AudioComponent || !AudioLog)
	{
		return 0.0f;
	}

	const float Duration = AudioLog->GetDuration();
	if (Duration <= 0.0f)
	{
		return 0.0f;
	}

	// Note: GetPlaybackPercent() might not be available in all UE versions
	// This is a simplified implementation
	return AudioComponent->IsPlaying() ? 0.5f : 0.0f;
}

bool ARecorderInteractable::TryAddToEvidence(AActor* InstigatorActor)
{
	if (!InstigatorActor || !bIsEvidence)
	{
		return false;
	}

	UInventoryComponent* Inventory = InstigatorActor->FindComponentByClass<UInventoryComponent>();
	if (!Inventory)
	{
		return false;
	}

	// Register metadata
	Inventory->RegisterEvidenceMetadata(EvidenceMetadata);

	// Add to collected evidence
	return Inventory->AddCollectedEvidenceId(EvidenceMetadata.EvidenceId);
}

void ARecorderInteractable::OnAudioFinished()
{
	if (bLoopAudio)
	{
		PlayAudioLog();
	}
	else
	{
		RecorderState = ERecorderState::Idle;
	}
}

void ARecorderInteractable::SaveState(TMap<FName, bool>& OutStateMap) const
{
	Super::SaveState(OutStateMap);

	if (InteractableId != NAME_None)
	{
		const FName ListenedStateKey = FName(*(InteractableId.ToString() + TEXT("_Listened")));
		OutStateMap.Add(ListenedStateKey, bHasBeenListened);
	}
}

void ARecorderInteractable::LoadState(const TMap<FName, bool>& InStateMap)
{
	Super::LoadState(InStateMap);

	if (InteractableId != NAME_None)
	{
		const FName ListenedStateKey = FName(*(InteractableId.ToString() + TEXT("_Listened")));

		if (const bool* ListenedState = InStateMap.Find(ListenedStateKey))
		{
			bHasBeenListened = *ListenedState;
		}
	}
}
