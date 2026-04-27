// Copyright Epic Games, Inc. All Rights Reserved.

#include "Audio/HorrorAudioSubsystem.h"
#include "Game/HorrorEventBusSubsystem.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "Sound/SoundAttenuation.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "DrawDebugHelpers.h"

void UHorrorAudioSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	InitializeDefaultVolumes();

	UWorld* World = GetWorld();
	if (World)
	{
		if (UHorrorEventBusSubsystem* EventBus = World->GetSubsystem<UHorrorEventBusSubsystem>())
		{
			EventBus->GetOnEventPublishedNative().AddUObject(this, &UHorrorAudioSubsystem::OnEventPublished);
		}
	}
}

void UHorrorAudioSubsystem::Deinitialize()
{
	StopAllAmbient(0.0f);

	for (USoundBase* Sound : PreloadedSounds)
	{
		if (Sound && Sound->IsRooted())
		{
			Sound->RemoveFromRoot();
		}
	}
	PreloadedSounds.Empty();
	ComponentBaseVolumes.Empty();

	UWorld* World = GetWorld();
	if (World)
	{
		if (UHorrorEventBusSubsystem* EventBus = World->GetSubsystem<UHorrorEventBusSubsystem>())
		{
			EventBus->GetOnEventPublishedNative().RemoveAll(this);
		}
	}

	Super::Deinitialize();
}

/**
 * Initializes default volume levels for each audio category
 * These values are tuned for optimal horror atmosphere balance
 */
void UHorrorAudioSubsystem::InitializeDefaultVolumes()
{
	// Default volume constants for audio categories
	constexpr float DefaultAmbientVolume = 0.6f;      // Background atmosphere
	constexpr float DefaultAnomalyVolume = 0.8f;      // Supernatural events (high priority)
	constexpr float DefaultSiteVolume = 0.7f;         // Environmental sounds
	constexpr float DefaultInteractionVolume = 0.75f; // Player interaction feedback
	constexpr float DefaultEscapeVolume = 0.9f;       // Chase/escape sequences (highest priority)
	constexpr float DefaultMusicVolume = 0.5f;        // Background music (lowest to not mask gameplay)

	CategoryVolumes.Add(EHorrorAudioCategory::Ambient, DefaultAmbientVolume);
	CategoryVolumes.Add(EHorrorAudioCategory::Anomaly, DefaultAnomalyVolume);
	CategoryVolumes.Add(EHorrorAudioCategory::Site, DefaultSiteVolume);
	CategoryVolumes.Add(EHorrorAudioCategory::Interaction, DefaultInteractionVolume);
	CategoryVolumes.Add(EHorrorAudioCategory::Escape, DefaultEscapeVolume);
	CategoryVolumes.Add(EHorrorAudioCategory::Music, DefaultMusicVolume);
}

/**
 * Plays a sound at a specific world location with optional volume and pitch modulation
 * @param Sound - The sound asset to play
 * @param Location - World space position for the sound
 * @param VolumeMultiplier - Volume scale factor (1.0 = default)
 * @param PitchMultiplier - Pitch scale factor (1.0 = default)
 * @return The spawned audio component, or nullptr if failed
 */
UAudioComponent* UHorrorAudioSubsystem::PlaySoundAtLocation(USoundBase* Sound, FVector Location, float VolumeMultiplier, float PitchMultiplier)
{
	UWorld* World = GetWorld();
	if (!Sound || !World)
	{
		return nullptr;
	}

	return UGameplayStatics::SpawnSoundAtLocation(
		World,
		Sound,
		Location,
		FRotator::ZeroRotator,
		VolumeMultiplier,
		PitchMultiplier,
		0.0f,
		nullptr,
		nullptr,
		true);
}

UAudioComponent* UHorrorAudioSubsystem::PlaySoundAttached(USoundBase* Sound, USceneComponent* AttachComponent, FName AttachPointName, float VolumeMultiplier)
{
	if (!Sound || !AttachComponent || !GetWorld())
	{
		return nullptr;
	}

	return UGameplayStatics::SpawnSoundAttached(
		Sound,
		AttachComponent,
		AttachPointName,
		FVector::ZeroVector,
		FRotator::ZeroRotator,
		EAttachLocation::KeepRelativeOffset,
		false,
		VolumeMultiplier,
		1.0f,
		0.0f,
		nullptr,
		nullptr,
		true);
}

UAudioComponent* UHorrorAudioSubsystem::PlaySound2D(USoundBase* Sound, float VolumeMultiplier)
{
	UWorld* World = GetWorld();
	if (!Sound || !World)
	{
		return nullptr;
	}

	return UGameplayStatics::SpawnSound2D(
		World,
		Sound,
		VolumeMultiplier,
		1.0f,
		0.0f,
		nullptr,
		true);
}

/**
 * Plays event-mapped audio based on gameplay tags
 * Automatically handles 2D/3D positioning and attachment based on mapping configuration
 * @param EventTag - Gameplay tag identifying the audio event
 * @param SourceObject - Optional source actor/component for spatial audio
 * @return True if sound was successfully triggered
 */
bool UHorrorAudioSubsystem::PlayEventSound(FGameplayTag EventTag, UObject* SourceObject)
{
	const FHorrorAudioEventMapping* Mapping = EventMappings.Find(EventTag);
	if (!Mapping || !Mapping->Sound)
	{
		return false;
	}

	UAudioComponent* AudioComp = nullptr;

	if (Mapping->bAttachToSource && SourceObject)
	{
		if (AActor* SourceActor = Cast<AActor>(SourceObject))
		{
			AudioComp = PlaySoundAttached(Mapping->Sound, SourceActor->GetRootComponent(), NAME_None, Mapping->VolumeMultiplier);
		}
		else if (USceneComponent* SceneComp = Cast<USceneComponent>(SourceObject))
		{
			AudioComp = PlaySoundAttached(Mapping->Sound, SceneComp, NAME_None, Mapping->VolumeMultiplier);
		}
	}
	else if (Mapping->bUse3DAttenuation && SourceObject)
	{
		FVector Location = FVector::ZeroVector;
		if (AActor* SourceActor = Cast<AActor>(SourceObject))
		{
			Location = SourceActor->GetActorLocation();
		}
		else if (USceneComponent* SceneComp = Cast<USceneComponent>(SourceObject))
		{
			Location = SceneComp->GetComponentLocation();
		}

		AudioComp = PlaySoundAtLocation(Mapping->Sound, Location, Mapping->VolumeMultiplier);
	}
	else
	{
		AudioComp = PlaySound2D(Mapping->Sound, Mapping->VolumeMultiplier);
	}

	return AudioComp != nullptr;
}

bool UHorrorAudioSubsystem::EnterAudioZone(FName ZoneId)
{
	if (ZoneId.IsNone())
	{
		return false;
	}

	const FHorrorAudioZoneConfig* Config = ZoneConfigs.Find(ZoneId);
	if (!Config || !Config->AmbientSound)
	{
		return false;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	if (CurrentAmbientComponent && CurrentAmbientComponent->IsPlaying())
	{
		CurrentAmbientComponent->FadeOut(Config->FadeOutDuration, 0.0f);
	}

	CurrentZoneId = ZoneId;

	CurrentAmbientComponent = UGameplayStatics::SpawnSound2D(
		World,
		Config->AmbientSound,
		Config->AmbientVolume,
		1.0f,
		0.0f,
		nullptr,
		Config->bLoopAmbient,
		true);

	if (CurrentAmbientComponent)
	{
		CurrentAmbientComponent->FadeIn(Config->FadeInDuration, Config->AmbientVolume);
		return true;
	}

	return false;
}

bool UHorrorAudioSubsystem::ExitAudioZone(FName ZoneId)
{
	if (CurrentZoneId != ZoneId)
	{
		return false;
	}

	const FHorrorAudioZoneConfig* Config = ZoneConfigs.Find(ZoneId);
	float FadeOutDuration = Config ? Config->FadeOutDuration : 1.5f;

	if (CurrentAmbientComponent && CurrentAmbientComponent->IsPlaying())
	{
		CurrentAmbientComponent->FadeOut(FadeOutDuration, 0.0f);
	}

	CurrentZoneId = NAME_None;
	CurrentAmbientComponent = nullptr;
	return true;
}

void UHorrorAudioSubsystem::StopAllAmbient(float FadeOutDuration)
{
	if (CurrentAmbientComponent && CurrentAmbientComponent->IsPlaying())
	{
		if (FadeOutDuration > 0.0f)
		{
			CurrentAmbientComponent->FadeOut(FadeOutDuration, 0.0f);
		}
		else
		{
			CurrentAmbientComponent->Stop();
		}
	}

	CurrentZoneId = NAME_None;
	CurrentAmbientComponent = nullptr;
}

void UHorrorAudioSubsystem::SetCategoryVolume(EHorrorAudioCategory Category, float Volume)
{
	CategoryVolumes.Add(Category, FMath::Clamp(Volume, 0.0f, 1.0f));
}

float UHorrorAudioSubsystem::GetCategoryVolume(EHorrorAudioCategory Category) const
{
	const float* Volume = CategoryVolumes.Find(Category);
	return Volume ? *Volume : 1.0f;
}

void UHorrorAudioSubsystem::RegisterEventMapping(const FHorrorAudioEventMapping& Mapping)
{
	if (Mapping.EventTag.IsValid() && Mapping.Sound)
	{
		EventMappings.Add(Mapping.EventTag, Mapping);
	}
}

void UHorrorAudioSubsystem::RegisterZoneConfig(const FHorrorAudioZoneConfig& Config)
{
	if (!Config.ZoneId.IsNone() && Config.AmbientSound)
	{
		ZoneConfigs.Add(Config.ZoneId, Config);
	}
}

void UHorrorAudioSubsystem::UnregisterEventMapping(FGameplayTag EventTag)
{
	EventMappings.Remove(EventTag);
}

void UHorrorAudioSubsystem::UnregisterZoneConfig(FName ZoneId)
{
	ZoneConfigs.Remove(ZoneId);
}

void UHorrorAudioSubsystem::OnEventPublished(const FHorrorEventMessage& Message)
{
	PlayEventSound(Message.EventTag, Message.SourceObject);
}

UAudioComponent* UHorrorAudioSubsystem::PlaySoundWithPriority(USoundBase* Sound, FVector Location, int32 Priority, float VolumeMultiplier)
{
	UWorld* World = GetWorld();
	if (!Sound || !World)
	{
		return nullptr;
	}

	if (!CanPlaySound())
	{
		QueueSound(Sound, Location, Priority, VolumeMultiplier);
		return nullptr;
	}

	UAudioComponent* Component = GetPooledComponent(Sound);
	if (!Component)
	{
		Component = UGameplayStatics::SpawnSoundAtLocation(World, Sound, Location, FRotator::ZeroRotator, VolumeMultiplier);
	}
	else
	{
		Component->SetWorldLocation(Location);
		Component->SetVolumeMultiplier(VolumeMultiplier);
		Component->Play();
	}

	if (Component)
	{
		ActiveComponents.Add(Component);
		ComponentBaseVolumes.Add(Component, VolumeMultiplier);
	}

	return Component;
}

void UHorrorAudioSubsystem::QueueSound(USoundBase* Sound, FVector Location, int32 Priority, float VolumeMultiplier)
{
	if (!Sound)
	{
		return;
	}

	FHorrorAudioQueueEntry Entry;
	Entry.Sound = Sound;
	Entry.Location = Location;
	Entry.VolumeMultiplier = VolumeMultiplier;
	Entry.Priority = Priority;
	UWorld* World = GetWorld();
	Entry.QueueTime = World ? World->GetTimeSeconds() : 0.0f;
	Entry.bIs3D = true;

	AudioQueue.Add(Entry);
	AudioQueue.Sort([](const FHorrorAudioQueueEntry& A, const FHorrorAudioQueueEntry& B)
	{
		return A.Priority > B.Priority;
	});
}

void UHorrorAudioSubsystem::SetOcclusionEnabled(bool bEnabled)
{
	bEnableOcclusion = bEnabled;
}

void UHorrorAudioSubsystem::UpdateOcclusion(float DeltaTime)
{
	UWorld* World = GetWorld();
	if (!bEnableOcclusion || !World)
	{
		return;
	}

	LastOcclusionUpdateTime += DeltaTime;
	if (LastOcclusionUpdateTime < OcclusionUpdateRate)
	{
		return;
	}

	const float OcclusionDeltaTime = LastOcclusionUpdateTime;
	LastOcclusionUpdateTime = 0.0f;

	APlayerController* PC = World->GetFirstPlayerController();
	if (!PC || !PC->GetPawn())
	{
		return;
	}

	FVector ListenerLocation = PC->GetPawn()->GetActorLocation();

	for (UAudioComponent* Component : ActiveComponents)
	{
		if (!Component || !Component->IsPlaying())
		{
			continue;
		}

		FVector SoundLocation = Component->GetComponentLocation();
		FHitResult HitResult;
		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(PC->GetPawn());

		bool bHit = World->LineTraceSingleByChannel(
			HitResult,
			ListenerLocation,
			SoundLocation,
			ECC_Visibility,
			QueryParams
		);

		float* BaseVolume = ComponentBaseVolumes.Find(Component);
		if (!BaseVolume)
		{
			BaseVolume = &ComponentBaseVolumes.Add(Component, Component->VolumeMultiplier);
		}

		const float TargetVolume = *BaseVolume * (bHit ? OcclusionVolumeMultiplier : 1.0f);
		const float SmoothedVolume = FMath::FInterpTo(Component->VolumeMultiplier, TargetVolume, OcclusionDeltaTime, OcclusionInterpSpeed);
		Component->SetVolumeMultiplier(SmoothedVolume);
	}
}

int32 UHorrorAudioSubsystem::GetActiveAudioComponentCount() const
{
	return ActiveComponents.Num();
}

void UHorrorAudioSubsystem::PreloadSound(USoundBase* Sound)
{
	if (Sound && !PreloadedSounds.Contains(Sound) && !Sound->IsRooted())
	{
		Sound->AddToRoot();
		PreloadedSounds.Add(Sound);
	}
}

void UHorrorAudioSubsystem::UnloadSound(USoundBase* Sound)
{
	if (Sound && PreloadedSounds.Contains(Sound))
	{
		if (Sound->IsRooted())
		{
			Sound->RemoveFromRoot();
		}
		PreloadedSounds.Remove(Sound);
	}
}

void UHorrorAudioSubsystem::ProcessAudioQueue()
{
	if (AudioQueue.Num() == 0 || !CanPlaySound())
	{
		return;
	}

	FHorrorAudioQueueEntry Entry = *AudioQueue.GetData();
	AudioQueue.RemoveAt(0);

	PlaySoundWithPriority(Entry.Sound, Entry.Location, Entry.Priority, Entry.VolumeMultiplier);
}

void UHorrorAudioSubsystem::CleanupAudioPool()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		ActiveComponents.RemoveAll([](UAudioComponent* Component)
		{
			return !Component || !Component->IsPlaying();
		});
		return;
	}

	float CurrentTime = World->GetTimeSeconds();

	for (int32 i = AudioPool.Num() - 1; i >= 0; --i)
	{
		FHorrorAudioPoolEntry& Entry = *(AudioPool.GetData() + i);

		if (!Entry.bInUse && (CurrentTime - Entry.LastUsedTime) > HorrorAudioDefaults::PoolCleanupIdleSeconds)
		{
			if (Entry.Component)
			{
				ComponentBaseVolumes.Remove(Entry.Component);
				Entry.Component->DestroyComponent();
			}
			AudioPool.RemoveAt(i);
		}
	}

	ActiveComponents.RemoveAll([](UAudioComponent* Component)
	{
		return !Component || !Component->IsPlaying();
	});

	for (auto It = ComponentBaseVolumes.CreateIterator(); It; ++It)
	{
		UAudioComponent* Component = It.Key();
		if (!Component || !ActiveComponents.Contains(Component))
		{
			It.RemoveCurrent();
		}
	}
}

UAudioComponent* UHorrorAudioSubsystem::GetPooledComponent(USoundBase* Sound)
{
	UWorld* World = GetWorld();

	for (FHorrorAudioPoolEntry& Entry : AudioPool)
	{
		if (!Entry.bInUse && Entry.Sound == Sound)
		{
			Entry.bInUse = true;
			Entry.LastUsedTime = World ? World->GetTimeSeconds() : 0.0f;
			return Entry.Component;
		}
	}

	if (AudioPool.Num() < MaxPooledComponents)
	{
		if (!World)
		{
			return nullptr;
		}

		FHorrorAudioPoolEntry NewEntry;
		NewEntry.Sound = Sound;
		NewEntry.Component = NewObject<UAudioComponent>(this);
		NewEntry.Component->SetSound(Sound);
		NewEntry.Component->RegisterComponent();
		NewEntry.bInUse = true;
		NewEntry.LastUsedTime = World->GetTimeSeconds();

		AudioPool.Add(NewEntry);
		return NewEntry.Component;
	}

	return nullptr;
}

void UHorrorAudioSubsystem::ReturnComponentToPool(UAudioComponent* Component)
{
	UWorld* World = GetWorld();

	for (FHorrorAudioPoolEntry& Entry : AudioPool)
	{
		if (Entry.Component == Component)
		{
			Entry.bInUse = false;
			Entry.LastUsedTime = World ? World->GetTimeSeconds() : 0.0f;
			Component->Stop();
			ComponentBaseVolumes.Remove(Component);
			break;
		}
	}
}

bool UHorrorAudioSubsystem::CanPlaySound() const
{
	return ActiveComponents.Num() < MaxConcurrentSounds;
}

void UHorrorAudioSubsystem::UpdateActiveSounds(float DeltaTime)
{
	LastPoolCleanupTime += DeltaTime;
	if (LastPoolCleanupTime >= PoolCleanupInterval)
	{
		CleanupAudioPool();
		LastPoolCleanupTime = 0.0f;
	}

	ProcessAudioQueue();
	UpdateOcclusion(DeltaTime);
}
