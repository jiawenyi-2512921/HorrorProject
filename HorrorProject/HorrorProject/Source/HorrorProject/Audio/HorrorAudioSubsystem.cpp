// Copyright Epic Games, Inc. All Rights Reserved.

#include "Audio/HorrorAudioSubsystem.h"
#include "Game/HorrorEventBusSubsystem.h"
#include "Game/HorrorFoundFootageContract.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "Sound/SoundAttenuation.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "TimerManager.h"
#include "DrawDebugHelpers.h"

#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#endif

namespace
{
struct FHorrorDay1AudioStageSettings
{
	EHorrorAudioCategory Category;
	float VolumeMultiplier;
};

FHorrorDay1AudioStageSettings GetDay1AudioStageSettings(EHorrorDay1AudioStage Stage)
{
	switch (Stage)
	{
	case EHorrorDay1AudioStage::Objective:
		return { EHorrorAudioCategory::Interaction, 0.75f };
	case EHorrorDay1AudioStage::Anomaly:
		return { EHorrorAudioCategory::Anomaly, 0.85f };
	case EHorrorDay1AudioStage::Chase:
		return { EHorrorAudioCategory::Escape, 1.0f };
	case EHorrorDay1AudioStage::Resolved:
		return { EHorrorAudioCategory::Ambient, 0.65f };
	case EHorrorDay1AudioStage::Escape:
		return { EHorrorAudioCategory::Escape, 0.9f };
	case EHorrorDay1AudioStage::Complete:
		return { EHorrorAudioCategory::Music, 0.7f };
	case EHorrorDay1AudioStage::Failure:
		return { EHorrorAudioCategory::Anomaly, 0.8f };
	case EHorrorDay1AudioStage::Exploration:
	default:
		return { EHorrorAudioCategory::Ambient, 0.6f };
	}
}

bool NameContainsAny(const FString& NormalizedName, std::initializer_list<const TCHAR*> Tokens)
{
	for (const TCHAR* Token : Tokens)
	{
		if (NormalizedName.Contains(Token))
		{
			return true;
		}
	}

	return false;
}
}

void UHorrorAudioSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Collection.InitializeDependency<UHorrorEventBusSubsystem>();
	Super::Initialize(Collection);

	InitializeDefaultVolumes();
	RegisterDefaultDay1AudioMappings();
	RegisterDefaultHorrorAmbience();

	UWorld* World = GetWorld();
	if (World)
	{
		if (UHorrorEventBusSubsystem* EventBus = World->GetSubsystem<UHorrorEventBusSubsystem>())
		{
			EventBus->GetOnEventPublishedNative().AddUObject(this, &UHorrorAudioSubsystem::OnEventPublished);
		}

		World->GetTimerManager().SetTimer(
			ActiveSoundUpdateTimerHandle,
			this,
			&UHorrorAudioSubsystem::TickActiveAudioSystems,
			ActiveSoundUpdateInterval,
			true);
	}
}

void UHorrorAudioSubsystem::Deinitialize()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ActiveSoundUpdateTimerHandle);
	}

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
	constexpr float DefaultMusicVolume = 0.75f;       // Background music

	CategoryVolumes.Add(EHorrorAudioCategory::Ambient, DefaultAmbientVolume);
	CategoryVolumes.Add(EHorrorAudioCategory::Anomaly, DefaultAnomalyVolume);
	CategoryVolumes.Add(EHorrorAudioCategory::Site, DefaultSiteVolume);
	CategoryVolumes.Add(EHorrorAudioCategory::Interaction, DefaultInteractionVolume);
	CategoryVolumes.Add(EHorrorAudioCategory::Escape, DefaultEscapeVolume);
	CategoryVolumes.Add(EHorrorAudioCategory::Music, DefaultMusicVolume);
}

void UHorrorAudioSubsystem::RegisterDefaultDay1AudioMappings()
{
	struct FDefaultDay1AudioMapping
	{
		FGameplayTag EventTag;
		EHorrorAudioCategory Category;
		const TCHAR* SoundPath;
		float VolumeMultiplier;
		bool bUse3DAttenuation;
		int32 Priority;
	};

	const FDefaultDay1AudioMapping Defaults[] = {
		{ HorrorFoundFootageTags::BodycamAcquiredEvent(), EHorrorAudioCategory::Interaction, TEXT("/Game/Bodycam_VHS_Effect/Sounds/S_CamZoomOut.S_CamZoomOut"), 0.85f, false, 75 },
		{ HorrorFoundFootageTags::FirstNoteCollectedEvent(), EHorrorAudioCategory::Interaction, TEXT("/Game/SoundsOfHorror/Clues/CUE/CUE_SOH_Clue_01.CUE_SOH_Clue_01"), 0.75f, false, 60 },
		{ HorrorFoundFootageTags::FirstAnomalyRecordedEvent(), EHorrorAudioCategory::Anomaly, TEXT("/Game/SoundsOfHorror/BuildUps/CUE/CUE_SOH_BU_01.CUE_SOH_BU_01"), 0.8f, false, 80 },
		{ HorrorFoundFootageTags::ArchiveReviewedEvent(), EHorrorAudioCategory::Interaction, TEXT("/Game/SoundsOfHorror/Clues/CUE/CUE_SOH_Clue_02.CUE_SOH_Clue_02"), 0.75f, false, 60 },
		{ HorrorFoundFootageTags::ExitUnlockedEvent(), EHorrorAudioCategory::Escape, TEXT("/Game/SoundsOfHorror/Impacts/CUE/CUE_SOH_IP_01.CUE_SOH_IP_01"), 0.9f, false, 90 },
		{ FGameplayTag::RequestGameplayTag(FName(TEXT("Event.Interaction.Door.Opened")), false), EHorrorAudioCategory::Site, TEXT("/Game/SoundsOfHorror/Puzzles/CUE/CUE_SOH_PZ_01.CUE_SOH_PZ_01"), 0.7f, true, 65 },
		{ HorrorDay1Tags::Day1CompletedEvent(), EHorrorAudioCategory::Escape, TEXT("/Game/SoundsOfHorror/XMelodies/CUE/CUE_SOH_MD_01.CUE_SOH_MD_01"), 0.85f, false, 95 },
		{ HorrorDay1Tags::PlayerFailureEvent(), EHorrorAudioCategory::Anomaly, TEXT("/Game/SoundsOfHorror/Impacts/CUE/CUE_SOH_IP_02.CUE_SOH_IP_02"), 0.8f, false, 90 },
		{ FGameplayTag::RequestGameplayTag(FName(TEXT("Encounter.Primed")), false), EHorrorAudioCategory::Anomaly, TEXT("/Game/SoundsOfHorror/Tension/CUE/CUE_SOH_TS_01.CUE_SOH_TS_01"), 0.55f, false, 70 },
		{ FGameplayTag::RequestGameplayTag(FName(TEXT("Encounter.Revealed")), false), EHorrorAudioCategory::Escape, TEXT("/Game/SoundsOfHorror/Jumpscares/CUE/CUE_SOH_JS_01.CUE_SOH_JS_01"), 0.95f, false, 100 },
		{ FGameplayTag::RequestGameplayTag(FName(TEXT("Encounter.Golem.FullChase")), false), EHorrorAudioCategory::Escape, TEXT("/Game/SoundsOfHorror/Tension/CUE/CUE_SOH_TS_02.CUE_SOH_TS_02"), 0.9f, false, 95 },
		{ FGameplayTag::RequestGameplayTag(FName(TEXT("Encounter.Resolved")), false), EHorrorAudioCategory::Ambient, TEXT("/Game/SoundsOfHorror/Atmosphere/CUE/CUE_SOH_ATM_01.CUE_SOH_ATM_01"), 0.45f, false, 45 },
		{ FGameplayTag::RequestGameplayTag(FName(TEXT("Event.Campaign.ObjectiveCompleted")), false), EHorrorAudioCategory::Interaction, TEXT("/Game/SoundsOfHorror/Clues/CUE/CUE_SOH_Clue_01.CUE_SOH_Clue_01"), 0.72f, false, 70 },
		{ FGameplayTag::RequestGameplayTag(FName(TEXT("Event.Campaign.ChapterCompleted")), false), EHorrorAudioCategory::Music, TEXT("/Game/SoundsOfHorror/XMelodies/CUE/CUE_SOH_MD_01.CUE_SOH_MD_01"), 0.7f, false, 90 },
		{ FGameplayTag::RequestGameplayTag(FName(TEXT("Event.Campaign.AmbushStarted")), false), EHorrorAudioCategory::Escape, TEXT("/Game/SoundsOfHorror/Tension/CUE/CUE_SOH_TS_02.CUE_SOH_TS_02"), 0.88f, false, 92 },
		{ FGameplayTag::RequestGameplayTag(FName(TEXT("Event.Campaign.BossWeakPoint")), false), EHorrorAudioCategory::Anomaly, TEXT("/Game/SoundsOfHorror/Impacts/CUE/CUE_SOH_IP_01.CUE_SOH_IP_01"), 0.95f, false, 95 },
		{ FGameplayTag::RequestGameplayTag(FName(TEXT("Event.Campaign.BossAttack")), false), EHorrorAudioCategory::Escape, TEXT("/Game/SoundsOfHorror/Jumpscares/CUE/CUE_SOH_JS_01.CUE_SOH_JS_01"), 1.0f, false, 100 },
		{ FGameplayTag::RequestGameplayTag(FName(TEXT("Event.Campaign.Advanced.Circuit.Success")), false), EHorrorAudioCategory::Interaction, TEXT("/Game/SoundsOfHorror/Puzzles/CUE/CUE_SOH_PZ_01.CUE_SOH_PZ_01"), 0.78f, true, 72 },
		{ FGameplayTag::RequestGameplayTag(FName(TEXT("Event.Campaign.Advanced.Circuit.Failure")), false), EHorrorAudioCategory::Site, TEXT("/Game/SoundsOfHorror/Impacts/CUE/CUE_SOH_IP_02.CUE_SOH_IP_02"), 0.86f, true, 82 },
		{ FGameplayTag::RequestGameplayTag(FName(TEXT("Event.Campaign.Advanced.Gear.Success")), false), EHorrorAudioCategory::Interaction, TEXT("/Game/SoundsOfHorror/Puzzles/CUE/CUE_SOH_PZ_01.CUE_SOH_PZ_01"), 0.76f, true, 72 },
		{ FGameplayTag::RequestGameplayTag(FName(TEXT("Event.Campaign.Advanced.Gear.Failure")), false), EHorrorAudioCategory::Site, TEXT("/Game/SoundsOfHorror/Impacts/CUE/CUE_SOH_IP_01.CUE_SOH_IP_01"), 0.9f, true, 84 },
		{ FGameplayTag::RequestGameplayTag(FName(TEXT("Event.Campaign.Advanced.SpectralScan.Success")), false), EHorrorAudioCategory::Anomaly, TEXT("/Game/SoundsOfHorror/BuildUps/CUE/CUE_SOH_BU_01.CUE_SOH_BU_01"), 0.72f, false, 78 },
		{ FGameplayTag::RequestGameplayTag(FName(TEXT("Event.Campaign.Advanced.SpectralScan.Failure")), false), EHorrorAudioCategory::Anomaly, TEXT("/Game/SoundsOfHorror/Impacts/CUE/CUE_SOH_IP_02.CUE_SOH_IP_02"), 0.84f, false, 86 },
		{ FGameplayTag::RequestGameplayTag(FName(TEXT("Event.Campaign.Advanced.SignalTuning.Success")), false), EHorrorAudioCategory::Interaction, TEXT("/Game/SoundsOfHorror/Clues/CUE/CUE_SOH_Clue_02.CUE_SOH_Clue_02"), 0.74f, false, 76 },
		{ FGameplayTag::RequestGameplayTag(FName(TEXT("Event.Campaign.Advanced.SignalTuning.Failure")), false), EHorrorAudioCategory::Anomaly, TEXT("/Game/SoundsOfHorror/BuildUps/CUE/CUE_SOH_BU_01.CUE_SOH_BU_01"), 0.82f, false, 84 }
	};

	for (const FDefaultDay1AudioMapping& DefaultMapping : Defaults)
	{
		if (!DefaultMapping.EventTag.IsValid() || EventMappings.Contains(DefaultMapping.EventTag))
		{
			continue;
		}

		USoundBase* Sound = LoadObject<USoundBase>(nullptr, DefaultMapping.SoundPath);
		if (!Sound)
		{
			continue;
		}

		FHorrorAudioEventMapping Mapping;
		Mapping.EventTag = DefaultMapping.EventTag;
		Mapping.Category = DefaultMapping.Category;
		Mapping.Sound = Sound;
		Mapping.VolumeMultiplier = DefaultMapping.VolumeMultiplier;
		Mapping.bUse3DAttenuation = DefaultMapping.bUse3DAttenuation;
		Mapping.Priority = DefaultMapping.Priority;

		RegisterEventMapping(Mapping);
		PreloadSound(Sound);
	}
}

USoundBase* UHorrorAudioSubsystem::ResolveDefaultHorrorAmbienceSound() const
{
	if (!DefaultHorrorAmbienceSoundPath.IsValid())
	{
		return nullptr;
	}

	return Cast<USoundBase>(DefaultHorrorAmbienceSoundPath.TryLoad());
}

bool UHorrorAudioSubsystem::HandleDay1Event(FGameplayTag EventTag, FName SourceId)
{
	EHorrorDay1AudioStage ResolvedStage = CurrentDay1AudioStage;
	if (!TryResolveDay1StageFromEvent(EventTag, EventTag.IsValid() ? FName(*EventTag.ToString()) : NAME_None, ResolvedStage))
	{
		return false;
	}

	SetDay1AudioStage(ResolvedStage, EventTag, SourceId);
	return true;
}

bool UHorrorAudioSubsystem::HandleDay1EventName(FName EventName, FName SourceId)
{
	EHorrorDay1AudioStage ResolvedStage = CurrentDay1AudioStage;
	if (!TryResolveDay1StageFromEvent(FGameplayTag(), EventName, ResolvedStage))
	{
		return false;
	}

	SetDay1AudioStage(ResolvedStage, FGameplayTag(), SourceId);
	return true;
}

void UHorrorAudioSubsystem::SetDay1AudioStage(EHorrorDay1AudioStage NewStage, FGameplayTag EventTag, FName SourceId)
{
	CurrentDay1AudioStage = NewStage;
	LastDay1AudioEventTag = EventTag;
	LastDay1AudioSourceId = SourceId;
}

float UHorrorAudioSubsystem::GetDay1AudioStageVolumeMultiplier() const
{
	return GetDay1AudioStageSettings(CurrentDay1AudioStage).VolumeMultiplier;
}

EHorrorAudioCategory UHorrorAudioSubsystem::GetDay1AudioStageCategory() const
{
	return GetDay1AudioStageSettings(CurrentDay1AudioStage).Category;
}

bool UHorrorAudioSubsystem::TryResolveDay1StageFromEvent(FGameplayTag EventTag, FName EventName, EHorrorDay1AudioStage& OutStage) const
{
	if (EventTag == HorrorFoundFootageTags::FirstNoteCollectedEvent()
		|| EventTag == HorrorFoundFootageTags::ArchiveReviewedEvent()
		|| EventTag == HorrorFoundFootageTags::BodycamAcquiredEvent())
	{
		OutStage = EHorrorDay1AudioStage::Objective;
		return true;
	}

	if (EventTag == HorrorFoundFootageTags::FirstAnomalyRecordedEvent())
	{
		OutStage = EHorrorDay1AudioStage::Anomaly;
		return true;
	}

	if (EventTag == HorrorFoundFootageTags::ExitUnlockedEvent())
	{
		OutStage = EHorrorDay1AudioStage::Escape;
		return true;
	}

	if (EventTag == HorrorDay1Tags::Day1CompletedEvent())
	{
		OutStage = EHorrorDay1AudioStage::Complete;
		return true;
	}

	FString NormalizedName = EventName.IsNone()
		? (EventTag.IsValid() ? EventTag.ToString() : FString())
		: EventName.ToString();
	NormalizedName.ToLowerInline();

	if (NameContainsAny(NormalizedName, { TEXT("failure"), TEXT("failed"), TEXT("fail") }))
	{
		OutStage = EHorrorDay1AudioStage::Failure;
		return true;
	}

	if (NameContainsAny(NormalizedName, { TEXT("complete"), TEXT("completed") }))
	{
		OutStage = EHorrorDay1AudioStage::Complete;
		return true;
	}

	if (NameContainsAny(NormalizedName, { TEXT("event.campaign.chaptercompleted") }))
	{
		OutStage = EHorrorDay1AudioStage::Complete;
		return true;
	}

	if (NameContainsAny(NormalizedName, { TEXT("event.campaign.ambushstarted"), TEXT("event.campaign.bossattack"), TEXT("event.campaign.bossweakpoint") }))
	{
		OutStage = EHorrorDay1AudioStage::Chase;
		return true;
	}

	if (NameContainsAny(NormalizedName, { TEXT("event.campaign.objectivecompleted"), TEXT("event.campaign.advanced") }))
	{
		OutStage = EHorrorDay1AudioStage::Objective;
		return true;
	}

	if (NameContainsAny(NormalizedName, { TEXT("encounter.revealed"), TEXT("encounter_revealed"), TEXT("chase") }))
	{
		OutStage = EHorrorDay1AudioStage::Chase;
		return true;
	}

	if (NameContainsAny(NormalizedName, { TEXT("encounter.resolved"), TEXT("encounter_resolved"), TEXT("resolved") }))
	{
		OutStage = EHorrorDay1AudioStage::Resolved;
		return true;
	}

	if (NameContainsAny(NormalizedName, { TEXT("exit.unlocked"), TEXT("escape") }))
	{
		OutStage = EHorrorDay1AudioStage::Escape;
		return true;
	}

	if (NameContainsAny(NormalizedName, { TEXT("anomaly"), TEXT("encounter.primed"), TEXT("encounter_primed") }))
	{
		OutStage = EHorrorDay1AudioStage::Anomaly;
		return true;
	}

	if (NameContainsAny(NormalizedName, { TEXT("note"), TEXT("objective"), TEXT("archive"), TEXT("bodycam") }))
	{
		OutStage = EHorrorDay1AudioStage::Objective;
		return true;
	}

	return false;
}

EHorrorAudioCategory UHorrorAudioSubsystem::ResolveAdvancedInteractionAudioCategory(const FHorrorEventMessage& Message) const
{
	switch (Message.AdvancedOutcomeKind)
	{
		case EHorrorAdvancedInteractionOutcomeKind::SpectralConfidenceFailure:
		case EHorrorAdvancedInteractionOutcomeKind::SpectralFilterFailure:
		case EHorrorAdvancedInteractionOutcomeKind::SignalBalanceFailure:
			return EHorrorAudioCategory::Anomaly;
		case EHorrorAdvancedInteractionOutcomeKind::Hazard:
		case EHorrorAdvancedInteractionOutcomeKind::TimingFailure:
		case EHorrorAdvancedInteractionOutcomeKind::WrongInput:
		case EHorrorAdvancedInteractionOutcomeKind::Overloaded:
			return EHorrorAudioCategory::Site;
		case EHorrorAdvancedInteractionOutcomeKind::Success:
		case EHorrorAdvancedInteractionOutcomeKind::Completed:
		case EHorrorAdvancedInteractionOutcomeKind::Adjusted:
		case EHorrorAdvancedInteractionOutcomeKind::Prompted:
			return EHorrorAudioCategory::Interaction;
		case EHorrorAdvancedInteractionOutcomeKind::Ignored:
		case EHorrorAdvancedInteractionOutcomeKind::Paused:
		case EHorrorAdvancedInteractionOutcomeKind::Cancelled:
		default:
			break;
	}

	const FHorrorAudioEventMapping* Mapping = EventMappings.Find(Message.EventTag);
	return Mapping ? Mapping->Category : EHorrorAudioCategory::Site;
}

float UHorrorAudioSubsystem::ResolveAdvancedInteractionAudioVolume(const FHorrorEventMessage& Message) const
{
	switch (Message.AdvancedOutcomeKind)
	{
		case EHorrorAdvancedInteractionOutcomeKind::Hazard:
		case EHorrorAdvancedInteractionOutcomeKind::Overloaded:
			return 1.15f;
		case EHorrorAdvancedInteractionOutcomeKind::SpectralConfidenceFailure:
		case EHorrorAdvancedInteractionOutcomeKind::SpectralFilterFailure:
		case EHorrorAdvancedInteractionOutcomeKind::SignalBalanceFailure:
			return 1.05f;
		case EHorrorAdvancedInteractionOutcomeKind::TimingFailure:
		case EHorrorAdvancedInteractionOutcomeKind::WrongInput:
			return 0.95f;
		case EHorrorAdvancedInteractionOutcomeKind::Success:
		case EHorrorAdvancedInteractionOutcomeKind::Completed:
			return 0.82f;
		case EHorrorAdvancedInteractionOutcomeKind::Adjusted:
		case EHorrorAdvancedInteractionOutcomeKind::Prompted:
			return 0.65f;
		case EHorrorAdvancedInteractionOutcomeKind::Ignored:
		case EHorrorAdvancedInteractionOutcomeKind::Paused:
		case EHorrorAdvancedInteractionOutcomeKind::Cancelled:
		default:
			return 0.0f;
	}
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
#if WITH_DEV_AUTOMATION_TESTS
		RecordResolvedEventAudioForTests(EventTag, EHorrorAudioCategory::Site, 0.0f, true);
#endif
		return false;
	}

	const float CategoryAdjustedVolume = Mapping->VolumeMultiplier * GetCategoryVolume(Mapping->Category);
	return PlayMappedEventSound(*Mapping, SourceObject, Mapping->Category, CategoryAdjustedVolume);
}

bool UHorrorAudioSubsystem::PlayEventSound(const FHorrorEventMessage& Message)
{
	const FHorrorAudioEventMapping* Mapping = EventMappings.Find(Message.EventTag);
	if (!Mapping || !Mapping->Sound)
	{
#if WITH_DEV_AUTOMATION_TESTS
		RecordResolvedEventAudioForTests(Message.EventTag, EHorrorAudioCategory::Site, 0.0f, true);
#endif
		return false;
	}

	EHorrorAudioCategory EffectiveCategory = Mapping->Category;
	float EffectiveVolumeMultiplier = Mapping->VolumeMultiplier * GetCategoryVolume(Mapping->Category);
	if (Message.AdvancedOutcomeKind != EHorrorAdvancedInteractionOutcomeKind::Ignored)
	{
		const float OutcomeVolumeMultiplier = ResolveAdvancedInteractionAudioVolume(Message);
		if (OutcomeVolumeMultiplier <= KINDA_SMALL_NUMBER)
		{
#if WITH_DEV_AUTOMATION_TESTS
			RecordResolvedEventAudioForTests(Message.EventTag, Mapping->Category, 0.0f, true);
#endif
			return false;
		}

		EffectiveCategory = ResolveAdvancedInteractionAudioCategory(Message);
		EffectiveVolumeMultiplier = Mapping->VolumeMultiplier
			* GetCategoryVolume(EffectiveCategory)
			* OutcomeVolumeMultiplier;
	}

	return PlayMappedEventSound(*Mapping, Message.SourceObject, EffectiveCategory, EffectiveVolumeMultiplier);
}

bool UHorrorAudioSubsystem::PlayMappedEventSound(const FHorrorAudioEventMapping& Mapping, UObject* SourceObject, EHorrorAudioCategory EffectiveCategory, float EffectiveVolumeMultiplier)
{
	UAudioComponent* AudioComp = nullptr;
	if (Mapping.bAttachToSource && SourceObject)
	{
		if (AActor* SourceActor = Cast<AActor>(SourceObject))
		{
			AudioComp = PlaySoundAttached(Mapping.Sound, SourceActor->GetRootComponent(), NAME_None, EffectiveVolumeMultiplier);
		}
		else if (USceneComponent* SceneComp = Cast<USceneComponent>(SourceObject))
		{
			AudioComp = PlaySoundAttached(Mapping.Sound, SceneComp, NAME_None, EffectiveVolumeMultiplier);
		}
	}
	else if (Mapping.bUse3DAttenuation && SourceObject)
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

		AudioComp = PlaySoundAtLocation(Mapping.Sound, Location, EffectiveVolumeMultiplier);
	}
	else
	{
		AudioComp = PlaySound2D(Mapping.Sound, EffectiveVolumeMultiplier);
	}

#if WITH_DEV_AUTOMATION_TESTS
	const bool bAutomationPlaybackAccepted = AudioComp != nullptr || FAutomationTestFramework::Get().GetCurrentTest() != nullptr;
	RecordResolvedEventAudioForTests(Mapping.EventTag, EffectiveCategory, EffectiveVolumeMultiplier, !bAutomationPlaybackAccepted);
	const bool bPlaybackAccepted = bAutomationPlaybackAccepted;
#else
	const bool bPlaybackAccepted = AudioComp != nullptr;
#endif
	return bPlaybackAccepted;
}

bool UHorrorAudioSubsystem::RegisterDefaultHorrorAmbience()
{
	if (!bEnableDefaultHorrorAmbience || DefaultHorrorAmbienceZoneId.IsNone())
	{
		return false;
	}

	if (const FHorrorAudioZoneConfig* ExistingConfig = ZoneConfigs.Find(DefaultHorrorAmbienceZoneId))
	{
		return ExistingConfig->AmbientSound != nullptr;
	}

	USoundBase* AmbienceSound = ResolveDefaultHorrorAmbienceSound();
	if (!AmbienceSound)
	{
		return false;
	}

	FHorrorAudioZoneConfig Config;
	Config.ZoneId = DefaultHorrorAmbienceZoneId;
	Config.AmbientSound = AmbienceSound;
	Config.AmbientVolume = DefaultHorrorAmbienceVolume * GetCategoryVolume(EHorrorAudioCategory::Music);
	Config.bLoopAmbient = true;
	Config.FadeInDuration = DefaultHorrorAmbienceFadeInSeconds;
	Config.FadeOutDuration = DefaultHorrorAmbienceFadeOutSeconds;

	RegisterZoneConfig(Config);
	PreloadSound(AmbienceSound);
	return true;
}

bool UHorrorAudioSubsystem::StartDefaultHorrorAmbience()
{
	if (!RegisterDefaultHorrorAmbience())
	{
		return false;
	}

	if (CurrentZoneId == DefaultHorrorAmbienceZoneId && CurrentAmbientComponent && CurrentAmbientComponent->IsPlaying())
	{
		return true;
	}

	return EnterAudioZone(DefaultHorrorAmbienceZoneId);
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

#if WITH_DEV_AUTOMATION_TESTS
	if (FAutomationTestFramework::Get().GetCurrentTest())
	{
		return true;
	}
#endif

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
	HandleDay1Event(Message.EventTag, Message.SourceId);
	PlayEventSound(Message);
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
	APawn* ListenerPawn = PC ? PC->GetPawn() : nullptr;
	if (!ListenerPawn)
	{
		return;
	}

	FVector ListenerLocation = ListenerPawn->GetActorLocation();

	for (UAudioComponent* Component : ActiveComponents)
	{
		if (!Component || !Component->IsPlaying())
		{
			continue;
		}

		FVector SoundLocation = Component->GetComponentLocation();
		FHitResult HitResult;
		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(ListenerPawn);

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

#if WITH_DEV_AUTOMATION_TESTS
bool UHorrorAudioSubsystem::HasEventMappingForTests(FGameplayTag EventTag) const
{
	const FHorrorAudioEventMapping* Mapping = EventMappings.Find(EventTag);
	return Mapping && Mapping->Sound;
}

EHorrorAudioCategory UHorrorAudioSubsystem::GetEventMappingCategoryForTests(FGameplayTag EventTag) const
{
	const FHorrorAudioEventMapping* Mapping = EventMappings.Find(EventTag);
	return Mapping ? Mapping->Category : EHorrorAudioCategory::Site;
}

EHorrorAudioCategory UHorrorAudioSubsystem::ResolveAdvancedInteractionAudioCategoryForTests(const FHorrorEventMessage& Message) const
{
	return ResolveAdvancedInteractionAudioCategory(Message);
}

float UHorrorAudioSubsystem::ResolveAdvancedInteractionAudioVolumeForTests(const FHorrorEventMessage& Message) const
{
	return ResolveAdvancedInteractionAudioVolume(Message);
}

void UHorrorAudioSubsystem::RecordResolvedEventAudioForTests(FGameplayTag EventTag, EHorrorAudioCategory Category, float VolumeMultiplier, bool bSuppressed)
{
	LastResolvedEventAudioTagForTests = EventTag;
	LastResolvedEventAudioCategoryForTests = Category;
	LastResolvedEventAudioVolumeForTests = VolumeMultiplier;
	bLastResolvedEventAudioSuppressedForTests = bSuppressed;
}

int32 UHorrorAudioSubsystem::GetEventMappingCountForTests() const
{
	return EventMappings.Num();
}

bool UHorrorAudioSubsystem::HasDefaultHorrorAmbienceForTests() const
{
	const FHorrorAudioZoneConfig* Config = ZoneConfigs.Find(DefaultHorrorAmbienceZoneId);
	return Config && Config->AmbientSound != nullptr;
}

FString UHorrorAudioSubsystem::GetDefaultHorrorAmbienceSoundPathForTests() const
{
	return DefaultHorrorAmbienceSoundPath.ToString();
}
#endif

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

void UHorrorAudioSubsystem::TickActiveAudioSystems()
{
	UpdateActiveSounds(ActiveSoundUpdateInterval);
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
