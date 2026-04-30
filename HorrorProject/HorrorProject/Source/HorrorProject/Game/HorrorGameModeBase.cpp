// Copyright Epic Games, Inc. All Rights Reserved.

#include "Game/HorrorGameModeBase.h"

#include "Audio/HorrorAudioSubsystem.h"
#include "Camera/PlayerCameraManager.h"
#include "EngineUtils.h"
#include "Game/DeepWaterStationRouteKit.h"
#include "Game/HorrorCampaignBossActor.h"
#include "Game/HorrorCampaignObjectiveActor.h"
#include "Game/HorrorEncounterDirector.h"
#include "Game/HorrorDirectorSubsystem.h"
#include "Game/HorrorEventBusSubsystem.h"
#include "Game/HorrorMapChain.h"
#include "Game/HorrorMapChainExit.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/Light.h"
#include "GameFramework/PlayerStart.h"
#include "Engine/PlayerStartPIE.h"
#include "Engine/PointLight.h"
#include "Engine/PostProcessVolume.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/LightComponent.h"
#include "Components/LocalLightComponent.h"
#include "GameFramework/PlayerController.h"
#include "HorrorProject.h"
#include "Kismet/GameplayStatics.h"
#include "LevelSequenceActor.h"
#include "LevelSequencePlayer.h"
#include "Player/HorrorPlayerCharacter.h"
#include "Player/Components/QuantumCameraComponent.h"
#include "Player/Components/VHSEffectComponent.h"
#include "Player/HorrorPlayerController.h"
#include "Save/HorrorSaveSubsystem.h"
#include "TimerManager.h"
#include "UI/Day1SliceHUD.h"

namespace
{
	constexpr float SpawnSearchVerticalStepCm = 50.0f;
	constexpr int32 SpawnSearchVerticalSteps = 8;
	constexpr float SpawnSearchRingStepCm = 100.0f;
	constexpr int32 SpawnSearchRingCount = 8;
	const FName CampaignAtmosphereLightTag(TEXT("HorrorCampaignAtmosphereDimmed"));
	const FName CampaignAtmosphereVolumeTag(TEXT("HorrorCampaignAtmosphereVolume"));
	const FName CampaignVisibilityRescueLightTag(TEXT("HorrorCampaignVisibilityRescue"));
	constexpr float ScrapopolisReadableFogDensity = 0.035f;
	constexpr float ScrapopolisReadableFogMaxOpacity = 0.3f;
	constexpr float ScrapopolisRescueLightIntensity = 4200.0f;
	constexpr float ScrapopolisRescueLightRadius = 2200.0f;

	struct FHorrorGameModeDefaultMilestoneMetadata
	{
		FGameplayTag EventTag;
		FName SourceId;
		FName TrailerBeatId;
		const TCHAR* HintFallback;
		const TCHAR* DebugLabel;
	};

	void PublishFoundFootageEvents(
		UWorld* World,
		const FHorrorFoundFootageContract& Contract,
		const TArray<FHorrorFoundFootageRecordedEvent>& RecordedEvents,
		UObject* SourceObject)
	{
		if (!World)
		{
			return;
		}

		UHorrorEventBusSubsystem* EventBus = World->GetSubsystem<UHorrorEventBusSubsystem>();
		if (!EventBus)
		{
			return;
		}

		for (const FHorrorFoundFootageRecordedEvent& RecordedEvent : RecordedEvents)
		{
			EventBus->Publish(RecordedEvent.EventTag, RecordedEvent.SourceId, Contract.GetStateForEvent(RecordedEvent.EventTag), SourceObject);
		}
	}

	void RegisterHorrorGameModeMilestoneMetadata(
		UHorrorEventBusSubsystem& EventBus,
		const FHorrorGameModeDefaultMilestoneMetadata& Default)
	{
		FHorrorObjectiveMessageMetadata Metadata;
		Metadata.TrailerBeatId = Default.TrailerBeatId;
		Metadata.ObjectiveHint = FText::AsCultureInvariant(Default.HintFallback);
		Metadata.DebugLabel = FText::AsCultureInvariant(Default.DebugLabel);
		EventBus.RegisterObjectiveMetadata(Default.EventTag, Default.SourceId, Metadata);
	}

	bool TryFindClearSpawnLocation(
		UWorld& World,
		const APawn& PawnToFit,
		const FVector& BaseLocation,
		const FRotator& SpawnRotation,
		FVector& OutLocation)
	{
		FVector TestLocation = BaseLocation;
		if (World.FindTeleportSpot(&PawnToFit, TestLocation, SpawnRotation))
		{
			OutLocation = TestLocation;
			return true;
		}

		for (int32 ZStep = 1; ZStep <= SpawnSearchVerticalSteps; ++ZStep)
		{
			TestLocation = BaseLocation + FVector(0.0f, 0.0f, SpawnSearchVerticalStepCm * ZStep);
			if (World.FindTeleportSpot(&PawnToFit, TestLocation, SpawnRotation))
			{
				OutLocation = TestLocation;
				return true;
			}
		}

		for (int32 RingIndex = 1; RingIndex <= SpawnSearchRingCount; ++RingIndex)
		{
			const float Radius = SpawnSearchRingStepCm * RingIndex;
			for (int32 DirectionIndex = 0; DirectionIndex < 8; ++DirectionIndex)
			{
				const float AngleRadians = UE_TWO_PI * static_cast<float>(DirectionIndex) / 8.0f;
				const FVector RingOffset(FMath::Cos(AngleRadians) * Radius, FMath::Sin(AngleRadians) * Radius, 0.0f);

				for (int32 ZStep = 0; ZStep <= SpawnSearchVerticalSteps; ++ZStep)
				{
					TestLocation = BaseLocation + RingOffset + FVector(0.0f, 0.0f, SpawnSearchVerticalStepCm * ZStep);
					if (World.FindTeleportSpot(&PawnToFit, TestLocation, SpawnRotation))
					{
						OutLocation = TestLocation;
						return true;
					}
				}
			}
		}

		return false;
	}

}

namespace HorrorObjectiveMilestoneCheckpoints
{
	const FName BodycamAcquired(TEXT("Checkpoint.Day1.BodycamAcquired"));
	const FName FirstNoteCollected(TEXT("Checkpoint.Day1.FirstNoteCollected"));
	const FName FirstAnomalyRecorded(TEXT("Checkpoint.Day1.FirstAnomalyRecorded"));
	const FName Day1Complete(TEXT("Checkpoint.Day1.Complete"));
}

namespace HorrorObjectiveMilestoneSources
{
	const FName Bodycam(TEXT("Bodycam"));
	const FName FirstNote(TEXT("FirstNote"));
	const FName FirstAnomaly(TEXT("FirstAnomaly"));
	const FName Archive(TEXT("Archive"));
	const FName Exit(TEXT("Exit"));
	const FName Day1Complete(TEXT("Day1Complete"));
}

namespace HorrorPlayerFailureDefaults
{
	const FName UnknownDeathCause(TEXT("Death.Unknown"));
	const FName UnknownFailureCause(TEXT("Failure.Unknown"));
	const FName BossAttackFailureCause(TEXT("Failure.Boss.StoneGolemAttack"));
	const FText DeathFeedback = NSLOCTEXT("HorrorGameMode", "DeathFeedback", "信号丢失，正在恢复到上一个检查点...");
	const FText FailureFeedback = NSLOCTEXT("HorrorGameMode", "FailureFeedback", "任务失败，正在恢复到上一个检查点...");
	const FText MissingCheckpointFeedback = NSLOCTEXT("HorrorGameMode", "MissingCheckpoint", "没有可用检查点。");
}

namespace HorrorCampaignAudioEvents
{
	FGameplayTag ObjectiveCompleted()
	{
		return FGameplayTag::RequestGameplayTag(FName(TEXT("Event.Campaign.ObjectiveCompleted")), false);
	}

	FGameplayTag ChapterCompleted()
	{
		return FGameplayTag::RequestGameplayTag(FName(TEXT("Event.Campaign.ChapterCompleted")), false);
	}

	FGameplayTag BossWeakPoint()
	{
		return FGameplayTag::RequestGameplayTag(FName(TEXT("Event.Campaign.BossWeakPoint")), false);
	}
}

namespace
{
	void EnsureRouteKitObjectives(ADeepWaterStationRouteKit& RouteKit)
	{
		if (RouteKit.ObjectiveNodes.IsEmpty())
		{
			RouteKit.ConfigureDefaultFirstLoopObjectiveNodes();
		}
		else
		{
			RouteKit.EnsureDefaultFirstLoopObjectiveNodes();
		}
		RouteKit.SpawnObjectiveNodes();
		RouteKit.SpawnEncounterDirector();
	}

	FName GetHorrorGameModeMilestoneCheckpoint(FGameplayTag StateTag)
	{
		if (StateTag == HorrorFoundFootageTags::BodycamAcquiredState())
		{
			return HorrorObjectiveMilestoneCheckpoints::BodycamAcquired;
		}
		if (StateTag == HorrorFoundFootageTags::FirstNoteCollectedState())
		{
			return HorrorObjectiveMilestoneCheckpoints::FirstNoteCollected;
		}
		if (StateTag == HorrorFoundFootageTags::FirstAnomalyRecordedState())
		{
			return HorrorObjectiveMilestoneCheckpoints::FirstAnomalyRecorded;
		}

		return NAME_None;
	}
}

AHorrorGameModeBase::AHorrorGameModeBase()
{
	DefaultPawnClass = AHorrorPlayerCharacter::StaticClass();
	PlayerControllerClass = AHorrorPlayerController::StaticClass();
	HUDClass = ADay1SliceHUD::StaticClass();
	RuntimeEncounterDirectorClass = AHorrorEncounterDirector::StaticClass();
	RuntimeMapChainExitClass = AHorrorMapChainExit::StaticClass();
	RuntimeCampaignObjectiveClass = AHorrorCampaignObjectiveActor::StaticClass();
	RuntimeCampaignBossClass = AHorrorCampaignBossActor::StaticClass();
}

void AHorrorGameModeBase::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	HUDClass = ADay1SliceHUD::StaticClass();
	PlayerControllerClass = AHorrorPlayerController::StaticClass();
	Super::InitGame(MapName, Options, ErrorMessage);
	HUDClass = ADay1SliceHUD::StaticClass();
	PlayerControllerClass = AHorrorPlayerController::StaticClass();
}

void AHorrorGameModeBase::BeginPlay()
{
	Super::BeginPlay();

	RegisterDefaultObjectiveMetadata();

	if (bAutoSpawnRouteKitOnBeginPlay && ShouldAutoSpawnLegacyRouteKitInCurrentMap())
	{
		EnsureRouteKit();
	}

	if (bAutoSpawnEncounterDirectorOnBeginPlay)
	{
		EnsureEncounterDirector();
	}

	// Register encounter director with HorrorDirectorSubsystem
	if (RuntimeEncounterDirector)
	{
		if (UWorld* World = GetWorld())
		{
			if (UHorrorDirectorSubsystem* Director = World->GetSubsystem<UHorrorDirectorSubsystem>())
			{
				Director->RegisterEncounter(RuntimeEncounterDirector);
			}
		}
	}

	bool bCampaignRuntimeReady = false;
	if (bAutoSpawnCampaignRuntimeOnBeginPlay)
	{
		bCampaignRuntimeReady = EnsureCampaignRuntime();
	}

	if (bAutoApplyCampaignHorrorAtmosphere && bCampaignRuntimeReady)
	{
		ApplyCampaignHorrorAtmosphere();
	}

	if (bAutoSpawnMapChainExitOnBeginPlay)
	{
		EnsureMapChainExit();
	}

	if (bSanitizeImportedMapShowcaseRuntimeOnBeginPlay)
	{
		SanitizeImportedMapChainRuntime();
	}

	if (bAutoStartDefaultHorrorAmbience)
	{
		if (UWorld* World = GetWorld())
		{
			if (UHorrorAudioSubsystem* AudioSubsystem = World->GetSubsystem<UHorrorAudioSubsystem>())
			{
				AudioSubsystem->StartDefaultHorrorAmbience();
			}
		}
	}
}

AActor* AHorrorGameModeBase::ChoosePlayerStart_Implementation(AController* Player)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return Super::ChoosePlayerStart_Implementation(Player);
	}

	APlayerStart* FirstPlacedPlayerStart = nullptr;
	for (TActorIterator<APlayerStart> It(World); It; ++It)
	{
		APlayerStart* PlayerStart = *It;
		if (!PlayerStart)
		{
			continue;
		}

		if (PlayerStart->IsA<APlayerStartPIE>())
		{
			UE_LOG(LogHorrorProject, Log, TEXT("ChoosePlayerStart: using PIE PlayerStart %s."), *GetNameSafe(PlayerStart));
			return PlayerStart;
		}

		if (!FirstPlacedPlayerStart)
		{
			FirstPlacedPlayerStart = PlayerStart;
		}

		if (!PreferredPlayerStartTag.IsNone() && PlayerStart->PlayerStartTag == PreferredPlayerStartTag)
		{
			UE_LOG(LogHorrorProject, Log, TEXT("ChoosePlayerStart: using preferred PlayerStart %s with tag %s."),
				*GetNameSafe(PlayerStart),
				*PreferredPlayerStartTag.ToString());
			return PlayerStart;
		}
	}

	if (FirstPlacedPlayerStart)
	{
		UE_LOG(LogHorrorProject, Log, TEXT("ChoosePlayerStart: using first placed PlayerStart %s because preferred tag %s was not found."),
			*GetNameSafe(FirstPlacedPlayerStart),
			*PreferredPlayerStartTag.ToString());
		return FirstPlacedPlayerStart;
	}

	return Super::ChoosePlayerStart_Implementation(Player);
}

APawn* AHorrorGameModeBase::SpawnDefaultPawnAtTransform_Implementation(AController* NewPlayer, const FTransform& SpawnTransform)
{
	UWorld* World = GetWorld();
	UClass* PawnClass = GetDefaultPawnClassForController(NewPlayer);
	if (!World || !PawnClass)
	{
		return Super::SpawnDefaultPawnAtTransform_Implementation(NewPlayer, SpawnTransform);
	}

	FTransform ResolvedSpawnTransform = SpawnTransform;
	const APawn* PawnToFit = PawnClass->GetDefaultObject<APawn>();
	if (PawnToFit)
	{
		FVector ClearLocation = SpawnTransform.GetLocation();
		const FRotator SpawnRotation = SpawnTransform.Rotator();
		if (TryFindClearSpawnLocation(*World, *PawnToFit, SpawnTransform.GetLocation(), SpawnRotation, ClearLocation))
		{
			ResolvedSpawnTransform.SetLocation(ClearLocation);
			if (!ClearLocation.Equals(SpawnTransform.GetLocation(), KINDA_SMALL_NUMBER))
			{
				UE_LOG(LogHorrorProject, Log, TEXT("SpawnDefaultPawnAtTransform: adjusted pawn spawn from %s to %s."),
					*SpawnTransform.GetLocation().ToCompactString(),
					*ClearLocation.ToCompactString());
			}
		}
		else
		{
			UE_LOG(LogHorrorProject, Warning, TEXT("SpawnDefaultPawnAtTransform: no clear pawn spawn found near %s; trying requested transform."),
				*SpawnTransform.GetLocation().ToCompactString());
		}
	}

	FActorSpawnParameters SpawnInfo;
	SpawnInfo.Instigator = GetInstigator();
	SpawnInfo.ObjectFlags |= RF_Transient;
	SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;

	APawn* ResultPawn = World->SpawnActor<APawn>(PawnClass, ResolvedSpawnTransform, SpawnInfo);
	if (!ResultPawn)
	{
		UE_LOG(LogGameMode, Warning, TEXT("SpawnDefaultPawnAtTransform: Couldn't spawn Pawn of type %s at %s"),
			*GetNameSafe(PawnClass),
			*ResolvedSpawnTransform.ToHumanReadableString());
	}
	return ResultPawn;
}

bool AHorrorGameModeBase::TryAcquireBodycam(FName SourceId, bool bEnableBodycam)
{
	const FName ResolvedSourceId = SourceId.IsNone() ? FName(TEXT("Bodycam")) : SourceId;
	const bool bRecorded = RecordFoundFootageEvent(HorrorFoundFootageTags::BodycamAcquiredEvent(), ResolvedSourceId);

	if (AHorrorPlayerCharacter* PlayerCharacter = ResolveLeadPlayerCharacter())
	{
		if (UVHSEffectComponent* VHSEffect = PlayerCharacter->GetVHSEffectComponent())
		{
			VHSEffect->SetBodycamAcquired(true);
			VHSEffect->SetBodycamEnabled(bEnableBodycam);
		}

		if (UQuantumCameraComponent* QuantumCamera = PlayerCharacter->GetQuantumCameraComponent())
		{
			QuantumCamera->SetCameraAcquired(true);
			QuantumCamera->SetCameraEnabled(bEnableBodycam);
		}
	}

	return bRecorded;
}

bool AHorrorGameModeBase::TryCollectFirstNote(FName SourceId)
{
	if (!CanCollectFirstNote())
	{
		return false;
	}

	return RecordFoundFootageEvent(
		HorrorFoundFootageTags::FirstNoteCollectedEvent(),
		SourceId.IsNone() ? FName(TEXT("FirstNote")) : SourceId);
}

bool AHorrorGameModeBase::BeginFirstAnomalyCandidate(FName SourceId)
{
	if (!CanBeginFirstAnomalyCandidate())
	{
		return false;
	}

	return AnomalyDirector.BeginFirstAnomalyCandidate(SourceId, FoundFootageContract);
}

bool AHorrorGameModeBase::TryRecordFirstAnomaly(bool bIsRecording)
{
	if (!CanRecordFirstAnomaly(bIsRecording))
	{
		return false;
	}

	TArray<FHorrorFoundFootageRecordedEvent> RecordedEvents;
	TArray<FHorrorFoundFootageStateChange> StateChanges;
	const bool bRecorded = AnomalyDirector.TryRecordFirstAnomaly(bIsRecording, FoundFootageContract, &RecordedEvents, &StateChanges);
	if (bRecorded)
	{
		PublishFoundFootageEvents(GetWorld(), FoundFootageContract, RecordedEvents, this);
		for (const FHorrorFoundFootageStateChange& StateChange : StateChanges)
		{
			if (StateChange.bCompleted)
			{
				HandleObjectiveStateChange(StateChange.StateTag);
			}
		}
	}
	return bRecorded;
}

bool AHorrorGameModeBase::TryReviewArchive(FName SourceId)
{
	if (!CanReviewArchive())
	{
		return false;
	}

	return RecordFoundFootageEvent(
		HorrorFoundFootageTags::ArchiveReviewedEvent(),
		SourceId.IsNone() ? FName(TEXT("Archive")) : SourceId);
}

bool AHorrorGameModeBase::HasBodycamAcquired() const
{
	return FoundFootageContract.HasRecordedEvent(HorrorFoundFootageTags::BodycamAcquiredEvent());
}

bool AHorrorGameModeBase::HasCollectedFirstNote() const
{
	return FoundFootageContract.HasRecordedEvent(HorrorFoundFootageTags::FirstNoteCollectedEvent());
}

bool AHorrorGameModeBase::HasRecordedFirstAnomaly() const
{
	return FoundFootageContract.HasRecordedEvent(HorrorFoundFootageTags::FirstAnomalyRecordedEvent());
}

bool AHorrorGameModeBase::HasPendingFirstAnomalyCandidate() const
{
	return AnomalyDirector.HasPendingFirstAnomalyCandidate();
}

FName AHorrorGameModeBase::GetPendingFirstAnomalySourceId() const
{
	return AnomalyDirector.GetPendingFirstAnomalySourceId();
}

bool AHorrorGameModeBase::HasReviewedArchive() const
{
	return FoundFootageContract.HasRecordedEvent(HorrorFoundFootageTags::ArchiveReviewedEvent());
}

bool AHorrorGameModeBase::HasFoundFootageRecordedEvent(FGameplayTag EventTag) const
{
	return FoundFootageContract.HasRecordedEvent(EventTag);
}

bool AHorrorGameModeBase::HasFoundFootageCompletedState(FGameplayTag StateTag) const
{
	return FoundFootageContract.HasCompletedState(StateTag);
}

bool AHorrorGameModeBase::CanCollectFirstNote() const
{
	return HasBodycamAcquired() && !HasCollectedFirstNote();
}

bool AHorrorGameModeBase::CanBeginFirstAnomalyCandidate() const
{
	return HasCollectedFirstNote()
		&& !HasPendingFirstAnomalyCandidate()
		&& !HasRecordedFirstAnomaly();
}

bool AHorrorGameModeBase::CanRecordFirstAnomaly(bool bIsRecording) const
{
	return bIsRecording
		&& HasPendingFirstAnomalyCandidate()
		&& !HasRecordedFirstAnomaly();
}

bool AHorrorGameModeBase::CanReviewArchive() const
{
	return HasRecordedFirstAnomaly() && !HasReviewedArchive();
}

bool AHorrorGameModeBase::IsExitUnlocked() const
{
	return FoundFootageContract.IsExitUnlocked();
}

bool AHorrorGameModeBase::TryCompleteDay1(FName SourceId)
{
	if (!IsExitUnlocked() || bDay1Complete)
	{
		return false;
	}

	bDay1Complete = true;

	if (UWorld* World = GetWorld())
	{
		if (UHorrorEventBusSubsystem* EventBus = World->GetSubsystem<UHorrorEventBusSubsystem>())
		{
			const FName ResolvedSourceId = SourceId.IsNone() ? HorrorObjectiveMilestoneSources::Day1Complete : SourceId;
			EventBus->Publish(
				HorrorDay1Tags::Day1CompletedEvent(),
				ResolvedSourceId,
				HorrorDay1Tags::Day1CompletedState(),
				this);
		}
	}

	TryAutosaveOnMilestone(HorrorObjectiveMilestoneCheckpoints::Day1Complete);
	return true;
}

void AHorrorGameModeBase::ImportDay1CompleteState(bool bInDay1Complete)
{
	bDay1Complete = bInDay1Complete;
}

FHorrorFoundFootageProgressSnapshot AHorrorGameModeBase::BuildFoundFootageProgressSnapshot() const
{
	return FoundFootageContract.BuildSnapshot();
}

FHorrorObjectiveTrackerSnapshot AHorrorGameModeBase::BuildObjectiveTrackerSnapshot() const
{
	FHorrorObjectiveTrackerSnapshot Tracker;
	const auto MakeChecklistItem = [](EHorrorObjectiveTrackerStage Stage, const FText& Label, bool bComplete, bool bActive, bool bRequiresRecording)
	{
		FHorrorObjectiveChecklistItem Item;
		Item.Stage = Stage;
		Item.Label = Label;
		Item.bComplete = bComplete;
		Item.bActive = bActive;
		Item.bRequiresRecording = bRequiresRecording;
		return Item;
	};
	const auto PopulateDay1Checklist = [this, &Tracker, &MakeChecklistItem]()
	{
		const bool bHasBodycam = HasBodycamAcquired();
		const bool bHasFirstNote = HasCollectedFirstNote();
		const bool bHasPendingAnomaly = HasPendingFirstAnomalyCandidate();
		const bool bHasAnomalyRecording = HasRecordedFirstAnomaly();
		const bool bHasArchiveReview = HasReviewedArchive();
		const bool bHasExit = IsExitUnlocked();
		const bool bHasDay1Complete = IsDay1Complete();

		Tracker.ChecklistItems.Reset();
		Tracker.ChecklistItems.Add(MakeChecklistItem(
			EHorrorObjectiveTrackerStage::RecoverBodycam,
			NSLOCTEXT("HorrorObjectiveChecklist", "RecoverBodycam", "取回随身摄像机"),
			bHasBodycam || bHasDay1Complete,
			Tracker.Stage == EHorrorObjectiveTrackerStage::RecoverBodycam,
			false));
		Tracker.ChecklistItems.Add(MakeChecklistItem(
			EHorrorObjectiveTrackerStage::ReadFirstNote,
			NSLOCTEXT("HorrorObjectiveChecklist", "ReadFirstNote", "阅读第一份站内备忘录"),
			bHasFirstNote || bHasDay1Complete,
			Tracker.Stage == EHorrorObjectiveTrackerStage::ReadFirstNote,
			false));
		Tracker.ChecklistItems.Add(MakeChecklistItem(
			EHorrorObjectiveTrackerStage::FrameFirstAnomaly,
			NSLOCTEXT("HorrorObjectiveChecklist", "FrameFirstAnomaly", "对准第一个异常"),
			bHasPendingAnomaly || bHasAnomalyRecording || bHasDay1Complete,
			Tracker.Stage == EHorrorObjectiveTrackerStage::FrameFirstAnomaly,
			false));
		Tracker.ChecklistItems.Add(MakeChecklistItem(
			EHorrorObjectiveTrackerStage::RecordFirstAnomaly,
			NSLOCTEXT("HorrorObjectiveChecklist", "RecordFirstAnomaly", "录像锁定第一个异常"),
			bHasAnomalyRecording || bHasDay1Complete,
			Tracker.Stage == EHorrorObjectiveTrackerStage::RecordFirstAnomaly,
			true));
		Tracker.ChecklistItems.Add(MakeChecklistItem(
			EHorrorObjectiveTrackerStage::ReviewArchive,
			NSLOCTEXT("HorrorObjectiveChecklist", "ReviewArchive", "复查档案终端"),
			bHasArchiveReview || bHasDay1Complete,
			Tracker.Stage == EHorrorObjectiveTrackerStage::ReviewArchive,
			false));
		Tracker.ChecklistItems.Add(MakeChecklistItem(
			EHorrorObjectiveTrackerStage::Escape,
			NSLOCTEXT("HorrorObjectiveChecklist", "Escape", "穿过维修闸门撤离"),
			bHasDay1Complete,
			Tracker.Stage == EHorrorObjectiveTrackerStage::Escape && bHasExit,
			false));
	};

	const FHorrorFoundFootageProgressSnapshot ProgressSnapshot = BuildFoundFootageProgressSnapshot();
	Tracker.CompletedMilestoneCount = ProgressSnapshot.CompletedMilestoneCount;
	Tracker.RequiredMilestoneCount = ProgressSnapshot.Milestones.Num();
	Tracker.ProgressFraction = Tracker.RequiredMilestoneCount > 0
		? FMath::Clamp(
			static_cast<float>(Tracker.CompletedMilestoneCount) / static_cast<float>(Tracker.RequiredMilestoneCount),
			0.0f,
			1.0f)
		: 0.0f;
	Tracker.bExitUnlocked = ProgressSnapshot.bExitUnlocked;
	Tracker.ProgressLabel = FText::Format(
		NSLOCTEXT("HorrorObjectiveTracker", "ProgressLabel", "证据链 {0}/{1}"),
		FText::AsNumber(Tracker.CompletedMilestoneCount),
		FText::AsNumber(Tracker.RequiredMilestoneCount));

	if (IsDay1Complete())
	{
		Tracker.Stage = EHorrorObjectiveTrackerStage::Day1Complete;
		Tracker.Title = NSLOCTEXT("HorrorObjectiveTracker", "Day1CompleteTitle", "第一天完成");
		Tracker.PrimaryInstruction = NSLOCTEXT("HorrorObjectiveTracker", "Day1CompletePrimary", "证据已保存。");
		Tracker.SecondaryInstruction = NSLOCTEXT("HorrorObjectiveTracker", "Day1CompleteSecondary", "等待信号归档，准备进入下一段记录。");
		Tracker.ProgressFraction = 1.0f;
		Tracker.bComplete = true;
		Tracker.bExitUnlocked = true;
		PopulateDay1Checklist();
		return Tracker;
	}

	const FText CampaignObjectivePrompt = GetCurrentCampaignObjectivePromptText();
	if (!CampaignObjectivePrompt.IsEmpty())
	{
		Tracker.Stage = EHorrorObjectiveTrackerStage::CampaignObjective;
		Tracker.Title = NSLOCTEXT("HorrorObjectiveTracker", "CampaignObjectiveTitle", "当前任务");
		Tracker.PrimaryInstruction = CampaignObjectivePrompt;
		Tracker.SecondaryInstruction = NSLOCTEXT("HorrorObjectiveTracker", "CampaignObjectiveSecondary", "跟随目标方向推进，留意环境异常。");
		PopulateDay1Checklist();
		return Tracker;
	}

	if (!HasBodycamAcquired())
	{
		Tracker.Stage = EHorrorObjectiveTrackerStage::RecoverBodycam;
		Tracker.Title = NSLOCTEXT("HorrorObjectiveTracker", "RecoverBodycamTitle", "建立录像链路");
		Tracker.PrimaryInstruction = NSLOCTEXT("HorrorObjectiveTracker", "RecoverBodycamPrimary", "取回随身摄像机。");
		Tracker.SecondaryInstruction = NSLOCTEXT("HorrorObjectiveTracker", "RecoverBodycamSecondary", "没有摄像机，后续证据无法归档。");
		PopulateDay1Checklist();
		return Tracker;
	}

	if (!HasCollectedFirstNote())
	{
		Tracker.Stage = EHorrorObjectiveTrackerStage::ReadFirstNote;
		Tracker.Title = NSLOCTEXT("HorrorObjectiveTracker", "ReadFirstNoteTitle", "恢复站内线索");
		Tracker.PrimaryInstruction = NSLOCTEXT("HorrorObjectiveTracker", "ReadFirstNotePrimary", "找到并阅读第一份站内备忘录。");
		Tracker.SecondaryInstruction = NSLOCTEXT("HorrorObjectiveTracker", "ReadFirstNoteSecondary", "备忘录会记录门禁码和异常位置线索。");
		PopulateDay1Checklist();
		return Tracker;
	}

	if (HasPendingFirstAnomalyCandidate() && !HasRecordedFirstAnomaly())
	{
		Tracker.Stage = EHorrorObjectiveTrackerStage::RecordFirstAnomaly;
		Tracker.Title = NSLOCTEXT("HorrorObjectiveTracker", "RecordFirstAnomalyTitle", "捕捉第一个异常");
		Tracker.PrimaryInstruction = NSLOCTEXT("HorrorObjectiveTracker", "RecordFirstAnomalyPrimary", "保持随身摄像机录像并捕捉异常。");
		Tracker.SecondaryInstruction = NSLOCTEXT("HorrorObjectiveTracker", "RecordFirstAnomalySecondary", "异常已对准，保持录像直到锁定完成。");
		Tracker.bRequiresRecording = true;
		Tracker.bUrgent = true;
		PopulateDay1Checklist();
		return Tracker;
	}

	if (!HasRecordedFirstAnomaly())
	{
		Tracker.Stage = EHorrorObjectiveTrackerStage::FrameFirstAnomaly;
		Tracker.Title = NSLOCTEXT("HorrorObjectiveTracker", "FrameFirstAnomalyTitle", "搜索异常信号");
		Tracker.PrimaryInstruction = NSLOCTEXT("HorrorObjectiveTracker", "FrameFirstAnomalyPrimary", "寻找并对准第一个异常。");
		Tracker.SecondaryInstruction = NSLOCTEXT("HorrorObjectiveTracker", "FrameFirstAnomalySecondary", "镜头对准异常后，系统会进入捕捉准备状态。");
		PopulateDay1Checklist();
		return Tracker;
	}

	if (!HasReviewedArchive())
	{
		Tracker.Stage = EHorrorObjectiveTrackerStage::ReviewArchive;
		Tracker.Title = NSLOCTEXT("HorrorObjectiveTracker", "ReviewArchiveTitle", "复查证据");
		Tracker.PrimaryInstruction = NSLOCTEXT("HorrorObjectiveTracker", "ReviewArchivePrimary", "前往档案终端复查录像。");
		Tracker.SecondaryInstruction = NSLOCTEXT("HorrorObjectiveTracker", "ReviewArchiveSecondary", "档案确认后，出口闸门才会恢复指令。");
		PopulateDay1Checklist();
		return Tracker;
	}

	if (IsExitUnlocked())
	{
		Tracker.Stage = EHorrorObjectiveTrackerStage::Escape;
		Tracker.Title = NSLOCTEXT("HorrorObjectiveTracker", "EscapeTitle", "撤离路线已开启");
		Tracker.PrimaryInstruction = NSLOCTEXT("HorrorObjectiveTracker", "EscapePrimary", "穿过维修闸门离开。");
		Tracker.SecondaryInstruction = NSLOCTEXT("HorrorObjectiveTracker", "EscapeSecondary", "不要停留，站内信号正在恶化。");
		Tracker.bUrgent = true;
		Tracker.bExitUnlocked = true;
		PopulateDay1Checklist();
		return Tracker;
	}

	Tracker.Stage = EHorrorObjectiveTrackerStage::Escape;
	Tracker.Title = NSLOCTEXT("HorrorObjectiveTracker", "FallbackTitle", "继续推进");
	Tracker.PrimaryInstruction = NSLOCTEXT("HorrorObjectiveTracker", "FallbackPrimary", "继续深入站内。");
	Tracker.SecondaryInstruction = NSLOCTEXT("HorrorObjectiveTracker", "FallbackSecondary", "寻找下一条可验证线索。");
	PopulateDay1Checklist();
	return Tracker;
}

const FHorrorFoundFootageContract& AHorrorGameModeBase::GetFoundFootageContract() const
{
	return FoundFootageContract;
}

FHorrorFoundFootageSaveState AHorrorGameModeBase::ExportFoundFootageSaveState() const
{
	return FoundFootageContract.ExportSaveState();
}

void AHorrorGameModeBase::ImportFoundFootageSaveState(const FHorrorFoundFootageSaveState& SaveState)
{
	FoundFootageContract.ImportSaveState(SaveState);
	AnomalyDirector.ImportPendingFirstAnomalyCandidate(NAME_None, FoundFootageContract);
	SyncFoundFootageRuntimeStateToPlayer();
	RestoreEncounterStateFromFoundFootageCheckpoint();
}

void AHorrorGameModeBase::ImportPendingFirstAnomalyCandidate(FName SourceId)
{
	AnomalyDirector.ImportPendingFirstAnomalyCandidate(SourceId, FoundFootageContract);
}

void AHorrorGameModeBase::SyncFoundFootageRuntimeStateToPlayer()
{
	AHorrorPlayerCharacter* PlayerCharacter = ResolveLeadPlayerCharacter();
	if (!PlayerCharacter)
	{
		return;
	}

	const bool bHasBodycam = HasBodycamAcquired();
	if (UVHSEffectComponent* VHSEffect = PlayerCharacter->GetVHSEffectComponent())
	{
		VHSEffect->SetBodycamAcquired(bHasBodycam);
		VHSEffect->SetBodycamEnabled(bHasBodycam);
	}

	if (UQuantumCameraComponent* QuantumCamera = PlayerCharacter->GetQuantumCameraComponent())
	{
		QuantumCamera->SetCameraAcquired(bHasBodycam);
		QuantumCamera->SetCameraEnabled(bHasBodycam);
	}
}

bool AHorrorGameModeBase::CanCompleteCampaignObjective(FName ChapterId, FName ObjectiveId) const
{
	if (ChapterId.IsNone() || ObjectiveId.IsNone() || CampaignProgress.GetActiveChapterId() != ChapterId)
	{
		return false;
	}

	return CampaignProgress.CanCompleteObjective(ObjectiveId);
}

bool AHorrorGameModeBase::TryCompleteCampaignObjective(FName ChapterId, FName ObjectiveId, AActor* InstigatorActor)
{
	if (!CanCompleteCampaignObjective(ChapterId, ObjectiveId))
	{
		ShowCampaignMessage(NSLOCTEXT("HorrorGameMode", "PrereqNotMet", "请先完成前置目标。"), FLinearColor(1.0f, 0.72f, 0.2f), 2.0f);
		return false;
	}

	const FHorrorCampaignChapterDefinition* Chapter = GetCurrentCampaignChapterDefinition();
	const FHorrorCampaignObjectiveDefinition* Objective = Chapter
		? FHorrorCampaign::FindObjectiveById(*Chapter, ObjectiveId)
		: nullptr;
	if (!Objective || !CampaignProgress.TryCompleteObjective(ObjectiveId))
	{
		return false;
	}

	ShowCampaignMessage(
		Objective->CompletionText.IsEmpty() ? NSLOCTEXT("HorrorGameMode", "ObjectiveComplete", "目标已完成。") : Objective->CompletionText,
		FLinearColor(0.48f, 1.0f, 0.62f),
		2.25f);

	if (UWorld* World = GetWorld())
	{
		if (UHorrorEventBusSubsystem* EventBus = World->GetSubsystem<UHorrorEventBusSubsystem>())
		{
			EventBus->Publish(
				HorrorCampaignAudioEvents::ObjectiveCompleted(),
				Objective->ObjectiveId,
				HorrorCampaignAudioEvents::ObjectiveCompleted(),
				InstigatorActor ? InstigatorActor : this);
		}
	}

	if (RuntimeCampaignBoss && Chapter && Chapter->bRequiresBoss && Objective->ObjectiveType == EHorrorCampaignObjectiveType::RestorePower)
	{
		RuntimeCampaignBoss->SetBossAwake(true);
	}

	if (RuntimeCampaignBoss && Objective->ObjectiveType == EHorrorCampaignObjectiveType::BossWeakPoint)
	{
		RuntimeCampaignBoss->RegisterWeakPointResolved();
		if (UWorld* World = GetWorld())
		{
			if (UHorrorEventBusSubsystem* EventBus = World->GetSubsystem<UHorrorEventBusSubsystem>())
			{
				EventBus->Publish(
					HorrorCampaignAudioEvents::BossWeakPoint(),
					Objective->ObjectiveId,
					HorrorCampaignAudioEvents::BossWeakPoint(),
					InstigatorActor ? InstigatorActor : RuntimeCampaignBoss.Get());
			}
		}
	}
	else if (RuntimeCampaignBoss && CampaignProgress.IsBossDefeated())
	{
		RuntimeCampaignBoss->SetBossDefeated(true);
	}

	RefreshCampaignObjectiveActors();

	if (CampaignProgress.IsChapterComplete())
	{
		ShowCampaignMessage(
			Chapter && Chapter->bIsFinalChapter
				? NSLOCTEXT("HorrorGameMode", "FinalChapterComplete", "终章完成，临时结局出口已开启。")
				: NSLOCTEXT("HorrorGameMode", "ChapterComplete", "章节完成，出口已开启。"),
			FLinearColor(0.45f, 0.9f, 1.0f),
			4.0f);
		if (UWorld* World = GetWorld())
		{
			if (UHorrorEventBusSubsystem* EventBus = World->GetSubsystem<UHorrorEventBusSubsystem>())
			{
				EventBus->Publish(
					HorrorCampaignAudioEvents::ChapterCompleted(),
					Chapter ? Chapter->ChapterId : ChapterId,
					HorrorCampaignAudioEvents::ChapterCompleted(),
					InstigatorActor ? InstigatorActor : this);
			}
		}
		QueueCampaignAutoTravelIfNeeded();
	}
	else
	{
		ShowCurrentCampaignObjectiveHint();
	}

	(void)InstigatorActor;
	return true;
}

bool AHorrorGameModeBase::IsCurrentCampaignChapterComplete() const
{
	if (!CampaignProgress.HasActiveChapter())
	{
		return true;
	}

	return CampaignProgress.IsChapterComplete();
}

const FHorrorCampaignChapterDefinition* AHorrorGameModeBase::GetCurrentCampaignChapterDefinition() const
{
	return FHorrorCampaign::FindChapterById(CampaignProgress.GetActiveChapterId());
}

bool AHorrorGameModeBase::TryGetCurrentCampaignObjectiveWorldLocation(FVector& OutLocation) const
{
	OutLocation = FVector::ZeroVector;

	const FHorrorCampaignObjectiveDefinition* CurrentObjective = CampaignProgress.GetNextObjective();
	if (!CurrentObjective)
	{
		return false;
	}

	const FName ActiveChapterId = CampaignProgress.GetActiveChapterId();
	const auto MatchesCurrentObjective = [ActiveChapterId, CurrentObjective](const AHorrorCampaignObjectiveActor* ObjectiveActor)
	{
		return ObjectiveActor
			&& !ObjectiveActor->IsActorBeingDestroyed()
			&& ObjectiveActor->GetChapterId() == ActiveChapterId
			&& ObjectiveActor->GetObjectiveId() == CurrentObjective->ObjectiveId;
	};

	for (const AHorrorCampaignObjectiveActor* ObjectiveActor : RuntimeCampaignObjectiveActorViews)
	{
		if (MatchesCurrentObjective(ObjectiveActor))
		{
			OutLocation = ObjectiveActor->GetActorLocation();
			return true;
		}
	}

	const UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	for (TActorIterator<AHorrorCampaignObjectiveActor> It(World); It; ++It)
	{
		const AHorrorCampaignObjectiveActor* ObjectiveActor = *It;
		if (MatchesCurrentObjective(ObjectiveActor))
		{
			OutLocation = ObjectiveActor->GetActorLocation();
			return true;
		}
	}

	return false;
}

bool AHorrorGameModeBase::ShouldUseCampaignFailureRecovery(FName FailureCause) const
{
	if (!CampaignProgress.HasActiveChapter())
	{
		return false;
	}

	const FHorrorCampaignChapterDefinition* Chapter = GetCurrentCampaignChapterDefinition();
	if (!Chapter)
	{
		return false;
	}

	return Chapter->bRequiresBoss
		|| FailureCause == HorrorPlayerFailureDefaults::BossAttackFailureCause
		|| FailureCause.ToString().StartsWith(TEXT("Failure.Boss."));
}

FTransform AHorrorGameModeBase::ResolveCampaignFailureRecoveryTransform() const
{
	FVector ObjectiveLocation = FVector::ZeroVector;
	if (TryGetCurrentCampaignObjectiveWorldLocation(ObjectiveLocation))
	{
		return FTransform(FRotator::ZeroRotator, ObjectiveLocation + FVector(-280.0f, 0.0f, 35.0f), FVector::OneVector);
	}

	const FHorrorCampaignObjectiveDefinition* CurrentObjective = CampaignProgress.GetNextObjective();
	if (CurrentObjective)
	{
		const FTransform ObjectiveTransform = ResolveCampaignObjectiveTransform(*CurrentObjective);
		return FTransform(FRotator::ZeroRotator, ObjectiveTransform.GetLocation() + FVector(-280.0f, 0.0f, 35.0f), FVector::OneVector);
	}

	return ResolveCampaignRuntimeAnchorTransform();
}

bool AHorrorGameModeBase::TryRecoverFromCampaignFailure(FName FailureCause)
{
	UWorld* World = GetWorld();
	AHorrorPlayerCharacter* PlayerCharacter = ResolveLeadPlayerCharacter();
	if (!World || !PlayerCharacter)
	{
		return false;
	}

	FTransform RecoveryTransform = ResolveCampaignFailureRecoveryTransform();
	FVector RecoveryLocation = RecoveryTransform.GetLocation();
	const FRotator RecoveryRotation = RecoveryTransform.Rotator();
	if (!TryFindClearSpawnLocation(*World, *PlayerCharacter, RecoveryLocation, RecoveryRotation, RecoveryLocation))
	{
		UE_LOG(
			LogHorrorProject,
			Warning,
			TEXT("Campaign failure recovery could not find a clear teleport spot near %s for cause %s; using requested recovery transform."),
			*RecoveryTransform.GetLocation().ToCompactString(),
			*FailureCause.ToString());
	}

	RecoveryTransform.SetLocation(RecoveryLocation);
	PlayerCharacter->SetActorTransform(RecoveryTransform, false, nullptr, ETeleportType::TeleportPhysics);
	if (APlayerController* PlayerController = Cast<APlayerController>(PlayerCharacter->GetController()))
	{
		PlayerController->SetControlRotation(RecoveryRotation);
	}

	RefreshCampaignObjectiveActors();
	return true;
}

FText AHorrorGameModeBase::GetCurrentCampaignObjectivePromptText() const
{
	const FHorrorCampaignObjectiveDefinition* CurrentObjective = CampaignProgress.GetNextObjective();
	return CurrentObjective ? CurrentObjective->PromptText : FText::GetEmpty();
}

FText AHorrorGameModeBase::GetCurrentCampaignObjectiveActionText() const
{
	const FHorrorCampaignObjectiveDefinition* CurrentObjective = CampaignProgress.GetNextObjective();
	if (!CurrentObjective)
	{
		return FText::GetEmpty();
	}

	switch (CurrentObjective->InteractionMode)
	{
	case EHorrorCampaignInteractionMode::CircuitWiring:
		return NSLOCTEXT("HorrorGameMode", "CampaignActionCircuitWiring", "接线");
	case EHorrorCampaignInteractionMode::GearCalibration:
		return NSLOCTEXT("HorrorGameMode", "CampaignActionGearCalibration", "齿轮校准");
	case EHorrorCampaignInteractionMode::TimedPursuit:
		return NSLOCTEXT("HorrorGameMode", "CampaignActionTimedPursuit", "追逐");
	case EHorrorCampaignInteractionMode::MultiStep:
		return NSLOCTEXT("HorrorGameMode", "CampaignActionMultiStep", "多段互动");
	case EHorrorCampaignInteractionMode::Instant:
	default:
		break;
	}

	switch (CurrentObjective->ObjectiveType)
	{
	case EHorrorCampaignObjectiveType::AcquireSignal:
		return NSLOCTEXT("HorrorGameMode", "CampaignActionAcquireSignal", "获取信号");
	case EHorrorCampaignObjectiveType::ScanAnomaly:
		return NSLOCTEXT("HorrorGameMode", "CampaignActionScanAnomaly", "扫描");
	case EHorrorCampaignObjectiveType::RecoverRelic:
		return NSLOCTEXT("HorrorGameMode", "CampaignActionRecoverRelic", "回收");
	case EHorrorCampaignObjectiveType::RestorePower:
		return NSLOCTEXT("HorrorGameMode", "CampaignActionRestorePower", "供电");
	case EHorrorCampaignObjectiveType::PlantBeacon:
		return NSLOCTEXT("HorrorGameMode", "CampaignActionPlantBeacon", "信标");
	case EHorrorCampaignObjectiveType::SurviveAmbush:
		return NSLOCTEXT("HorrorGameMode", "CampaignActionSurviveAmbush", "伏击");
	case EHorrorCampaignObjectiveType::DisableSeal:
		return NSLOCTEXT("HorrorGameMode", "CampaignActionDisableSeal", "解除封印");
	case EHorrorCampaignObjectiveType::BossWeakPoint:
		return NSLOCTEXT("HorrorGameMode", "CampaignActionBossWeakPoint", "弱点");
	case EHorrorCampaignObjectiveType::FinalTerminal:
		return NSLOCTEXT("HorrorGameMode", "CampaignActionFinalTerminal", "终端");
	default:
		return NSLOCTEXT("HorrorGameMode", "CampaignActionObjective", "目标");
	}
}

int32 AHorrorGameModeBase::ApplyCampaignHorrorAtmosphere()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return 0;
	}

	int32 DimmedLightCount = 0;
	const FHorrorCampaignAtmosphereTuning AtmosphereTuning = ResolveCampaignAtmosphereTuningForMap(World->GetPackage()->GetName());
	const float LightScale = FMath::Clamp(AtmosphereTuning.LightScale, 0.0f, 1.0f);
	for (TActorIterator<ALight> It(World); It; ++It)
	{
		ALight* LightActor = *It;
		if (!LightActor
			|| LightActor->Tags.Contains(CampaignAtmosphereLightTag)
			|| LightActor->Tags.Contains(CampaignVisibilityRescueLightTag))
		{
			continue;
		}

		ULightComponent* LightComponent = LightActor->GetLightComponent();
		if (!LightComponent)
		{
			continue;
		}

		LightComponent->SetIntensity(LightComponent->Intensity * LightScale);
		LightActor->Tags.Add(CampaignAtmosphereLightTag);
		++DimmedLightCount;
	}

	APostProcessVolume* RuntimeVolume = nullptr;
	for (TActorIterator<APostProcessVolume> It(World); It; ++It)
	{
		APostProcessVolume* ExistingVolume = *It;
		if (ExistingVolume && ExistingVolume->Tags.Contains(CampaignAtmosphereVolumeTag))
		{
			RuntimeVolume = ExistingVolume;
			break;
		}
	}

	if (!RuntimeVolume)
	{
		FActorSpawnParameters SpawnParameters;
		SpawnParameters.Owner = this;
		SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		RuntimeVolume = World->SpawnActor<APostProcessVolume>(APostProcessVolume::StaticClass(), FTransform::Identity, SpawnParameters);
		if (RuntimeVolume)
		{
			RuntimeVolume->Tags.Add(CampaignAtmosphereVolumeTag);
		}
	}

	if (RuntimeVolume)
	{
		RuntimeVolume->bUnbound = true;
		RuntimeVolume->BlendWeight = 1.0f;
		RuntimeVolume->Settings.bOverride_AutoExposureBias = true;
		RuntimeVolume->Settings.AutoExposureBias = AtmosphereTuning.AutoExposureBias;
		RuntimeVolume->Settings.bOverride_VignetteIntensity = true;
		RuntimeVolume->Settings.VignetteIntensity = FMath::Clamp(AtmosphereTuning.VignetteIntensity, 0.0f, 1.0f);
	}

	DimmedLightCount += SanitizeImportedMapVisualObstructions(World->GetPackage()->GetName());

	return DimmedLightCount;
}

bool AHorrorGameModeBase::StartCampaignAmbushThreat(FName SourceId, AActor* ThreatAnchor)
{
	if (!bSpawnCampaignAmbushThreats)
	{
		return false;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	if (RuntimeCampaignAmbushThreat && RuntimeCampaignAmbushThreat->IsActorBeingDestroyed())
	{
		RuntimeCampaignAmbushThreat = nullptr;
		ActiveCampaignAmbushSourceId = NAME_None;
	}

	if (!RuntimeCampaignAmbushThreat)
	{
		const TSubclassOf<AHorrorCampaignBossActor> SpawnClass = RuntimeCampaignBossClass
			? RuntimeCampaignBossClass
			: TSubclassOf<AHorrorCampaignBossActor>(AHorrorCampaignBossActor::StaticClass());

		FActorSpawnParameters SpawnParameters;
		SpawnParameters.Owner = this;
		SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		RuntimeCampaignAmbushThreat = World->SpawnActor<AHorrorCampaignBossActor>(
			SpawnClass,
			ResolveCampaignAmbushThreatTransform(ThreatAnchor),
			SpawnParameters);
	}
	else
	{
		RuntimeCampaignAmbushThreat->SetActorTransform(ResolveCampaignAmbushThreatTransform(ThreatAnchor));
	}

	if (!RuntimeCampaignAmbushThreat)
	{
		return false;
	}

	const FHorrorCampaignAmbushThreatTuning ThreatTuning = ResolveCampaignAmbushThreatTuningForMap(World->GetPackage()->GetName());
	ActiveCampaignAmbushSourceId = SourceId;
	RuntimeCampaignAmbushThreat->ConfigureBoss(
		SourceId.IsNone() ? FName(TEXT("Campaign.AmbushThreat")) : SourceId,
		NSLOCTEXT("HorrorGameMode", "Chaser", "追猎者"),
		1);
	RuntimeCampaignAmbushThreat->ConfigureChasePressure(
		ThreatTuning.MoveSpeed,
		ThreatTuning.EngageRadius,
		ThreatTuning.AttackRadius,
		ThreatTuning.FearPressureRadius,
		ThreatTuning.ActorScale);
	RuntimeCampaignAmbushThreat->SetBossAwake(true);
	ShowCampaignMessage(NSLOCTEXT("HorrorGameMode", "AmbushWarning", "有东西被信标引来了，跑向亮起的目标点。"), FLinearColor(1.0f, 0.28f, 0.18f), 2.5f);
	return true;
}

void AHorrorGameModeBase::StopCampaignAmbushThreat(FName SourceId)
{
	if (!RuntimeCampaignAmbushThreat)
	{
		ActiveCampaignAmbushSourceId = NAME_None;
		return;
	}

	if (!SourceId.IsNone() && !ActiveCampaignAmbushSourceId.IsNone() && SourceId != ActiveCampaignAmbushSourceId)
	{
		return;
	}

	if (!RuntimeCampaignAmbushThreat->IsActorBeingDestroyed())
	{
		RuntimeCampaignAmbushThreat->SetBossAwake(false);
		RuntimeCampaignAmbushThreat->Destroy();
	}

	RuntimeCampaignAmbushThreat = nullptr;
	ActiveCampaignAmbushSourceId = NAME_None;
}

bool AHorrorGameModeBase::ClearImportedMapCameraFade(APlayerController* PlayerController)
{
	APlayerCameraManager* CameraManager = PlayerController ? PlayerController->PlayerCameraManager : nullptr;
	if (!CameraManager)
	{
		return false;
	}

	CameraManager->StopCameraFade();
	CameraManager->FadeAmount = 0.0f;
	CameraManager->FadeAlpha = FVector2D::ZeroVector;
	CameraManager->FadeTime = 0.0f;
	CameraManager->FadeTimeRemaining = 0.0f;
	CameraManager->FadeColor = FLinearColor::Transparent;
	CameraManager->bEnableFading = false;
	CameraManager->bFadeAudio = false;
	return true;
}

FHorrorCampaignAtmosphereTuning AHorrorGameModeBase::ResolveCampaignAtmosphereTuningForMapForTests(const FString& MapPackageName) const
{
	return ResolveCampaignAtmosphereTuningForMap(MapPackageName);
}

FHorrorCampaignAmbushThreatTuning AHorrorGameModeBase::ResolveCampaignAmbushThreatTuningForMapForTests(const FString& MapPackageName) const
{
	return ResolveCampaignAmbushThreatTuningForMap(MapPackageName);
}

int32 AHorrorGameModeBase::SanitizeImportedMapVisualObstructionsForTests(const FString& MapPackageName)
{
	return SanitizeImportedMapVisualObstructions(MapPackageName);
}

void AHorrorGameModeBase::ResetCampaignProgressForChapterForTests(FName ChapterId)
{
	if (const FHorrorCampaignChapterDefinition* Chapter = FHorrorCampaign::FindChapterById(ChapterId))
	{
		CampaignProgress.ResetForChapter(*Chapter);
	}
}

bool AHorrorGameModeBase::SaveDay1Checkpoint(FName CheckpointId)
{
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UHorrorSaveSubsystem* SaveSubsystem = GameInstance->GetSubsystem<UHorrorSaveSubsystem>())
		{
			return SaveSubsystem->SaveCheckpoint(this, CheckpointId);
		}
	}

	return false;
}

bool AHorrorGameModeBase::LoadDay1Checkpoint()
{
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UHorrorSaveSubsystem* SaveSubsystem = GameInstance->GetSubsystem<UHorrorSaveSubsystem>())
		{
			return SaveSubsystem->LoadCheckpoint(this);
		}
	}

	return false;
}

bool AHorrorGameModeBase::RequestPlayerDeath(FName DeathCause)
{
	return RequestPlayerFailure(
		DeathCause.IsNone() ? HorrorPlayerFailureDefaults::UnknownDeathCause : DeathCause,
		HorrorPlayerFailureDefaults::DeathFeedback);
}

bool AHorrorGameModeBase::RequestPlayerFailure(FName FailureCause, FText FeedbackText)
{
	const FName ResolvedFailureCause = FailureCause.IsNone()
		? HorrorPlayerFailureDefaults::UnknownFailureCause
		: FailureCause;
	const FText ResolvedFeedbackText = FeedbackText.IsEmpty()
		? HorrorPlayerFailureDefaults::FailureFeedback
		: FeedbackText;

	LastPlayerFailureCause = ResolvedFailureCause;
	++PlayerFailureCount;
	bLastPlayerFailureRecoveredFromCheckpoint = false;

	UWorld* World = GetWorld();
	if (World)
	{
		UGameplayStatics::SetGamePaused(World, false);
	}

	if (AHorrorPlayerController* PlayerController = Cast<AHorrorPlayerController>(UGameplayStatics::GetPlayerController(this, 0)))
	{
		PlayerController->ShowPlayerMessage(ResolvedFeedbackText, FLinearColor(0.9f, 0.05f, 0.04f), 2.0f);
	}

	UE_LOG(
		LogHorrorProject,
		Warning,
		TEXT("Player failure requested: %s. Attempting contextual recovery."),
		*ResolvedFailureCause.ToString());

	if (World)
	{
		if (UHorrorAudioSubsystem* AudioSubsystem = World->GetSubsystem<UHorrorAudioSubsystem>())
		{
			AudioSubsystem->HandleDay1EventName(TEXT("Day1.Failure"), ResolvedFailureCause);
		}
	}

	bLastPlayerFailureRecoveredFromCheckpoint = ShouldUseCampaignFailureRecovery(ResolvedFailureCause)
		? TryRecoverFromCampaignFailure(ResolvedFailureCause)
		: LoadDay1Checkpoint();
	if (World)
	{
		if (UHorrorEventBusSubsystem* EventBus = World->GetSubsystem<UHorrorEventBusSubsystem>())
		{
			EventBus->Publish(
				HorrorDay1Tags::PlayerFailureEvent(),
				ResolvedFailureCause,
				bLastPlayerFailureRecoveredFromCheckpoint
					? HorrorDay1Tags::CheckpointRestoredState()
					: HorrorDay1Tags::CheckpointMissingState(),
				this);
		}
	}

	if (!bLastPlayerFailureRecoveredFromCheckpoint)
	{
		if (AHorrorPlayerController* PlayerController = Cast<AHorrorPlayerController>(UGameplayStatics::GetPlayerController(this, 0)))
		{
			PlayerController->ResetDay1ModalInputState();
			PlayerController->ShowPlayerMessage(HorrorPlayerFailureDefaults::MissingCheckpointFeedback, FLinearColor(1.0f, 0.78f, 0.15f), 2.0f);
		}

		UE_LOG(
			LogHorrorProject,
			Warning,
			TEXT("Player failure recovery could not find a valid Day1 checkpoint for cause: %s."),
			*ResolvedFailureCause.ToString());
	}

	return bLastPlayerFailureRecoveredFromCheckpoint;
}

bool AHorrorGameModeBase::RecordFoundFootageEvent(FGameplayTag EventTag, FName SourceId)
{
	TArray<FHorrorFoundFootageRecordedEvent> RecordedEvents;
	TArray<FHorrorFoundFootageStateChange> StateChanges;
	const bool bRecorded = FoundFootageContract.RecordEvent(EventTag, SourceId, &RecordedEvents, &StateChanges);
	if (bRecorded)
	{
		PublishFoundFootageEvents(GetWorld(), FoundFootageContract, RecordedEvents, this);
		for (const FHorrorFoundFootageStateChange& StateChange : StateChanges)
		{
			if (StateChange.bCompleted)
			{
				HandleObjectiveStateChange(StateChange.StateTag);
			}
		}
	}
	return bRecorded;
}

AHorrorPlayerCharacter* AHorrorGameModeBase::ResolveLeadPlayerCharacter() const
{
	const UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	if (AHorrorPlayerCharacter* PlayerCharacter = Cast<AHorrorPlayerCharacter>(UGameplayStatics::GetPlayerCharacter(World, 0)))
	{
		return PlayerCharacter;
	}

	for (TActorIterator<AHorrorPlayerCharacter> PlayerIt(World); PlayerIt; ++PlayerIt)
	{
		return *PlayerIt;
	}

	return nullptr;
}

ADeepWaterStationRouteKit* AHorrorGameModeBase::EnsureRouteKit()
{
	if (RuntimeRouteKit)
	{
		return RuntimeRouteKit.Get();
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	for (TActorIterator<ADeepWaterStationRouteKit> It(World); It; ++It)
	{
		RuntimeRouteKit = *It;
		EnsureRouteKitObjectives(*RuntimeRouteKit);
		return RuntimeRouteKit.Get();
	}

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = this;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	RuntimeRouteKit = World->SpawnActor<ADeepWaterStationRouteKit>(
		ADeepWaterStationRouteKit::StaticClass(),
		ResolveRuntimeRouteKitSpawnTransform(),
		SpawnParameters);
	if (RuntimeRouteKit)
	{
		EnsureRouteKitObjectives(*RuntimeRouteKit);
	}
	return RuntimeRouteKit.Get();
}

FTransform AHorrorGameModeBase::ResolveRuntimeRouteKitSpawnTransform() const
{
	if (!bAnchorRuntimeRouteKitToPlayerStart)
	{
		return RuntimeRouteKitTransform;
	}

	const UWorld* World = GetWorld();
	if (!World)
	{
		return RuntimeRouteKitTransform;
	}

	for (TActorIterator<APlayerStart> It(World); It; ++It)
	{
		const FRotator PlayerStartRotation = It->GetActorRotation();
		return FTransform(
			FRotator(0.0f, PlayerStartRotation.Yaw, 0.0f),
			It->GetActorLocation(),
			RuntimeRouteKitTransform.GetScale3D());
	}

	return RuntimeRouteKitTransform;
}

bool AHorrorGameModeBase::IsLeadPlayerRecording() const
{
	const AHorrorPlayerCharacter* PlayerCharacter = ResolveLeadPlayerCharacter();
	const UQuantumCameraComponent* QuantumCamera = PlayerCharacter ? PlayerCharacter->GetQuantumCameraComponent() : nullptr;
	return QuantumCamera && QuantumCamera->IsCameraMode(EQuantumCameraMode::Recording);
}

AHorrorEncounterDirector* AHorrorGameModeBase::EnsureEncounterDirector()
{
	if (RuntimeEncounterDirector)
	{
		return RuntimeEncounterDirector.Get();
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	for (TActorIterator<AHorrorEncounterDirector> It(World); It; ++It)
	{
		RuntimeEncounterDirector = *It;
		return RuntimeEncounterDirector.Get();
	}

	const TSubclassOf<AHorrorEncounterDirector> SpawnClass = RuntimeEncounterDirectorClass
		? RuntimeEncounterDirectorClass
		: TSubclassOf<AHorrorEncounterDirector>(AHorrorEncounterDirector::StaticClass());

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = this;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	RuntimeEncounterDirector = World->SpawnActor<AHorrorEncounterDirector>(SpawnClass, RuntimeEncounterDirectorTransform, SpawnParameters);
	if (RuntimeEncounterDirector)
	{
		RuntimeEncounterDirector->RevealRadius = 0.0f;
	}
	return RuntimeEncounterDirector.Get();
}

AHorrorMapChainExit* AHorrorGameModeBase::EnsureMapChainExit()
{
	if (RuntimeMapChainExit)
	{
		return RuntimeMapChainExit.Get();
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	const FString CurrentMapPackageName = FHorrorMapChain::NormalizeMapPackageName(World->GetPackage()->GetName());
	if (!FHorrorMapChain::IsMapInChain(CurrentMapPackageName))
	{
		return nullptr;
	}

	for (TActorIterator<AHorrorMapChainExit> It(World); It; ++It)
	{
		RuntimeMapChainExit = *It;

		FString NextMapPackageName;
		const bool bHasNextMap = FHorrorMapChain::TryGetNextMapPackage(CurrentMapPackageName, NextMapPackageName);
		RuntimeMapChainExit->ConfigureForMapChain(CurrentMapPackageName, NextMapPackageName, !bHasNextMap);
		return RuntimeMapChainExit.Get();
	}

	const TSubclassOf<AHorrorMapChainExit> SpawnClass = RuntimeMapChainExitClass
		? RuntimeMapChainExitClass
		: TSubclassOf<AHorrorMapChainExit>(AHorrorMapChainExit::StaticClass());

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = this;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	RuntimeMapChainExit = World->SpawnActor<AHorrorMapChainExit>(
		SpawnClass,
		ResolveMapChainExitSpawnTransform(),
		SpawnParameters);

	if (RuntimeMapChainExit)
	{
		FString NextMapPackageName;
		const bool bHasNextMap = FHorrorMapChain::TryGetNextMapPackage(CurrentMapPackageName, NextMapPackageName);
		RuntimeMapChainExit->ConfigureForMapChain(CurrentMapPackageName, NextMapPackageName, !bHasNextMap);
	}

	return RuntimeMapChainExit.Get();
}

FTransform AHorrorGameModeBase::ResolveMapChainExitSpawnTransform() const
{
	const UWorld* World = GetWorld();
	APlayerStart* FirstPlacedPlayerStart = nullptr;
	APlayerStart* PreferredPlayerStart = nullptr;

	if (World)
	{
		for (TActorIterator<APlayerStart> It(World); It; ++It)
		{
			APlayerStart* PlayerStart = *It;
			if (!PlayerStart)
			{
				continue;
			}

			if (PlayerStart->IsA<APlayerStartPIE>())
			{
				FirstPlacedPlayerStart = PlayerStart;
				break;
			}

			if (!FirstPlacedPlayerStart)
			{
				FirstPlacedPlayerStart = PlayerStart;
			}

			if (!PreferredPlayerStart && !PreferredPlayerStartTag.IsNone() && PlayerStart->PlayerStartTag == PreferredPlayerStartTag)
			{
				PreferredPlayerStart = PlayerStart;
			}
		}
	}

	const AActor* AnchorActor = PreferredPlayerStart ? PreferredPlayerStart : FirstPlacedPlayerStart;
	const FRotator AnchorRotation = AnchorActor ? AnchorActor->GetActorRotation() : FRotator::ZeroRotator;
	const FVector AnchorLocation = AnchorActor ? AnchorActor->GetActorLocation() : FVector::ZeroVector;
	const FRotationMatrix AnchorRotationMatrix(FRotator(0.0f, AnchorRotation.Yaw, 0.0f));
	const FVector Offset =
		AnchorRotationMatrix.GetUnitAxis(EAxis::X) * RuntimeMapChainExitOffset.X
		+ AnchorRotationMatrix.GetUnitAxis(EAxis::Y) * RuntimeMapChainExitOffset.Y
		+ FVector(0.0f, 0.0f, RuntimeMapChainExitOffset.Z);

	return FTransform(
		FRotator(0.0f, AnchorRotation.Yaw + 180.0f, 0.0f),
		AnchorLocation + Offset,
		FVector::OneVector);
}

bool AHorrorGameModeBase::EnsureCampaignRuntime()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	const FHorrorCampaignChapterDefinition* Chapter = FHorrorCampaign::FindChapterForMap(World->GetPackage()->GetName());
	if (!Chapter)
	{
		return false;
	}

	bool bResetChapterProgress = false;
	if (CampaignProgress.GetActiveChapterId() != Chapter->ChapterId)
	{
		ClearCampaignRuntimeActors();
		CampaignProgress.ResetForChapter(*Chapter);
		bCampaignAutoTravelQueued = false;
		bResetChapterProgress = true;
	}

	if (RuntimeCampaignObjectiveActors.IsEmpty())
	{
		SpawnCampaignObjectives(*Chapter);
	}

	if (Chapter->bRequiresBoss && !RuntimeCampaignBoss)
	{
		SpawnCampaignBossIfNeeded(*Chapter);
	}

	if (bResetChapterProgress)
	{
		UE_LOG(LogHorrorProject, Log, TEXT("Campaign runtime initialized chapter %s on map %s with %d objectives%s."),
			*Chapter->ChapterId.ToString(),
			*World->GetPackage()->GetName(),
			Chapter->Objectives.Num(),
			Chapter->bRequiresBoss ? TEXT(" and a boss encounter") : TEXT(""));
		ShowCampaignChapterIntro(*Chapter);
		ShowCurrentCampaignObjectiveHint();
	}

	return true;
}

bool AHorrorGameModeBase::ShouldAutoSpawnLegacyRouteKitInCurrentMap() const
{
	const UWorld* World = GetWorld();
	if (!World)
	{
		return true;
	}

	return !FHorrorMapChain::IsMapInChain(World->GetPackage()->GetName());
}

FTransform AHorrorGameModeBase::ResolveCampaignRuntimeAnchorTransform() const
{
	const UWorld* World = GetWorld();
	APlayerStart* FirstPlacedPlayerStart = nullptr;
	APlayerStart* PreferredPlayerStart = nullptr;

	if (World)
	{
		for (TActorIterator<APlayerStart> It(World); It; ++It)
		{
			APlayerStart* PlayerStart = *It;
			if (!PlayerStart)
			{
				continue;
			}

			if (PlayerStart->IsA<APlayerStartPIE>())
			{
				FirstPlacedPlayerStart = PlayerStart;
				break;
			}

			if (!FirstPlacedPlayerStart)
			{
				FirstPlacedPlayerStart = PlayerStart;
			}

			if (!PreferredPlayerStart && !PreferredPlayerStartTag.IsNone() && PlayerStart->PlayerStartTag == PreferredPlayerStartTag)
			{
				PreferredPlayerStart = PlayerStart;
			}
		}
	}

	const AActor* AnchorActor = PreferredPlayerStart ? PreferredPlayerStart : FirstPlacedPlayerStart;
	return AnchorActor
		? FTransform(FRotator(0.0f, AnchorActor->GetActorRotation().Yaw, 0.0f), AnchorActor->GetActorLocation(), FVector::OneVector)
		: FTransform::Identity;
}

FTransform AHorrorGameModeBase::ResolveCampaignObjectiveTransform(const FHorrorCampaignObjectiveDefinition& Objective) const
{
	const FTransform AnchorTransform = ResolveCampaignRuntimeAnchorTransform();
	const FRotator AnchorYawRotation(0.0f, AnchorTransform.Rotator().Yaw, 0.0f);
	const FRotationMatrix AnchorRotationMatrix(AnchorYawRotation);
	const FVector RelativeLocation =
		AnchorRotationMatrix.GetUnitAxis(EAxis::X) * Objective.RelativeLocation.X
		+ AnchorRotationMatrix.GetUnitAxis(EAxis::Y) * Objective.RelativeLocation.Y
		+ FVector(0.0f, 0.0f, Objective.RelativeLocation.Z);

	return FTransform(
		FRotator(0.0f, AnchorYawRotation.Yaw + 180.0f, 0.0f),
		AnchorTransform.GetLocation() + RelativeLocation,
		FVector::OneVector);
}

FTransform AHorrorGameModeBase::ResolveCampaignBossTransform() const
{
	const FTransform AnchorTransform = ResolveCampaignRuntimeAnchorTransform();
	const FRotator AnchorYawRotation(0.0f, AnchorTransform.Rotator().Yaw, 0.0f);
	const FRotationMatrix AnchorRotationMatrix(AnchorYawRotation);
	const FVector RelativeLocation =
		AnchorRotationMatrix.GetUnitAxis(EAxis::X) * RuntimeCampaignBossOffset.X
		+ AnchorRotationMatrix.GetUnitAxis(EAxis::Y) * RuntimeCampaignBossOffset.Y
		+ FVector(0.0f, 0.0f, RuntimeCampaignBossOffset.Z);

	return FTransform(
		FRotator(0.0f, AnchorYawRotation.Yaw + 180.0f, 0.0f),
		AnchorTransform.GetLocation() + RelativeLocation,
		FVector::OneVector);
}

FTransform AHorrorGameModeBase::ResolveCampaignAmbushThreatTransform(const AActor* ThreatAnchor) const
{
	const FTransform AnchorTransform = ThreatAnchor
		? FTransform(FRotator(0.0f, ThreatAnchor->GetActorRotation().Yaw, 0.0f), ThreatAnchor->GetActorLocation(), FVector::OneVector)
		: ResolveCampaignRuntimeAnchorTransform();
	const FRotator AnchorYawRotation(0.0f, AnchorTransform.Rotator().Yaw, 0.0f);
	const FRotationMatrix AnchorRotationMatrix(AnchorYawRotation);
	const FVector RelativeLocation =
		AnchorRotationMatrix.GetUnitAxis(EAxis::X) * RuntimeCampaignAmbushThreatOffset.X
		+ AnchorRotationMatrix.GetUnitAxis(EAxis::Y) * RuntimeCampaignAmbushThreatOffset.Y
		+ FVector(0.0f, 0.0f, RuntimeCampaignAmbushThreatOffset.Z);

	return FTransform(
		FRotator(0.0f, AnchorYawRotation.Yaw, 0.0f),
		AnchorTransform.GetLocation() + RelativeLocation,
		FVector::OneVector);
}

FHorrorCampaignAtmosphereTuning AHorrorGameModeBase::ResolveCampaignAtmosphereTuningForMap(const FString& MapPackageName) const
{
	FHorrorCampaignAtmosphereTuning Tuning;
	Tuning.LightScale = FMath::Clamp(CampaignHorrorAtmosphereLightScale, 0.0f, 1.0f);
	Tuning.AutoExposureBias = CampaignHorrorAtmosphereAutoExposureBias;
	Tuning.VignetteIntensity = FMath::Clamp(CampaignHorrorAtmosphereVignetteIntensity, 0.0f, 1.0f);

	const FString NormalizedMapPackageName = FHorrorMapChain::NormalizeMapPackageName(MapPackageName);
	if (NormalizedMapPackageName.Contains(TEXT("/Scrapopolis/")) || NormalizedMapPackageName.Contains(TEXT("Level_Scrapopolis_Demo")))
	{
		Tuning.LightScale = FMath::Clamp(ScrapopolisAtmosphereLightScale, 0.0f, 1.0f);
		Tuning.AutoExposureBias = ScrapopolisAtmosphereAutoExposureBias;
		Tuning.VignetteIntensity = FMath::Clamp(ScrapopolisAtmosphereVignetteIntensity, 0.0f, 1.0f);
	}

	return Tuning;
}

FHorrorCampaignAmbushThreatTuning AHorrorGameModeBase::ResolveCampaignAmbushThreatTuningForMap(const FString& MapPackageName) const
{
	FHorrorCampaignAmbushThreatTuning Tuning;

	const FString NormalizedMapPackageName = FHorrorMapChain::NormalizeMapPackageName(MapPackageName);
	if (NormalizedMapPackageName.Contains(TEXT("/Bodycam_VHS_Effect/")) || NormalizedMapPackageName.Contains(TEXT("LVL_Showcase_01")))
	{
		Tuning.ActorScale = 0.72f;
		Tuning.MoveSpeed = 150.0f;
		Tuning.EngageRadius = 3600.0f;
		Tuning.AttackRadius = 90.0f;
		Tuning.FearPressureRadius = 1100.0f;
	}
	else if (NormalizedMapPackageName.Contains(TEXT("/ForestOfSpikes/")) || NormalizedMapPackageName.Contains(TEXT("Level_ForestOfSpikes_Demo_Night")))
	{
		Tuning.ActorScale = 1.12f;
		Tuning.MoveSpeed = 190.0f;
		Tuning.EngageRadius = 4600.0f;
		Tuning.AttackRadius = 130.0f;
		Tuning.FearPressureRadius = 1550.0f;
	}
	else if (NormalizedMapPackageName.Contains(TEXT("/Scrapopolis/")) || NormalizedMapPackageName.Contains(TEXT("Level_Scrapopolis_Demo")))
	{
		Tuning.ActorScale = 0.9f;
		Tuning.MoveSpeed = 185.0f;
		Tuning.EngageRadius = 4000.0f;
		Tuning.AttackRadius = 130.0f;
		Tuning.FearPressureRadius = 1550.0f;
	}
	else if (NormalizedMapPackageName.Contains(TEXT("/Fantastic_Dungeon_Pack/")) || NormalizedMapPackageName.Contains(TEXT("map_dungeon_")))
	{
		Tuning.ActorScale = 0.95f;
		Tuning.MoveSpeed = 125.0f;
		Tuning.EngageRadius = 3400.0f;
		Tuning.AttackRadius = 105.0f;
		Tuning.FearPressureRadius = 1300.0f;
	}

	return Tuning;
}

bool AHorrorGameModeBase::ShouldSanitizeImportedMapVisualObstructions(const FString& MapPackageName) const
{
	const FString NormalizedMapPackageName = FHorrorMapChain::NormalizeMapPackageName(MapPackageName);
	return NormalizedMapPackageName.Contains(TEXT("/Scrapopolis/"))
		|| NormalizedMapPackageName.Contains(TEXT("Level_Scrapopolis_Demo"));
}

int32 AHorrorGameModeBase::SanitizeImportedMapVisualObstructions(const FString& MapPackageName)
{
	if (!ShouldSanitizeImportedMapVisualObstructions(MapPackageName))
	{
		return 0;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return 0;
	}

	int32 SanitizedActorCount = 0;
	for (TActorIterator<APostProcessVolume> It(World); It; ++It)
	{
		APostProcessVolume* PostProcessVolume = *It;
		if (!PostProcessVolume || PostProcessVolume->Tags.Contains(CampaignAtmosphereVolumeTag))
		{
			continue;
		}

		bool bChanged = false;
		if (PostProcessVolume->bEnabled)
		{
			PostProcessVolume->bEnabled = false;
			bChanged = true;
		}
		if (!FMath::IsNearlyZero(PostProcessVolume->BlendWeight))
		{
			PostProcessVolume->BlendWeight = 0.0f;
			bChanged = true;
		}
		if (PostProcessVolume->bUnbound)
		{
			PostProcessVolume->bUnbound = false;
			bChanged = true;
		}

		FPostProcessSettings& Settings = PostProcessVolume->Settings;
		if (Settings.bOverride_AutoExposureBias || Settings.bOverride_AutoExposureMinBrightness || Settings.bOverride_AutoExposureMaxBrightness)
		{
			Settings.bOverride_AutoExposureBias = false;
			Settings.bOverride_AutoExposureMinBrightness = false;
			Settings.bOverride_AutoExposureMaxBrightness = false;
			Settings.AutoExposureBias = 0.0f;
			bChanged = true;
		}
		if (Settings.bOverride_ColorGain || Settings.bOverride_ColorGamma || Settings.bOverride_ColorSaturation || Settings.bOverride_SceneColorTint)
		{
			Settings.bOverride_ColorGain = false;
			Settings.bOverride_ColorGamma = false;
			Settings.bOverride_ColorSaturation = false;
			Settings.bOverride_SceneColorTint = false;
			Settings.ColorGain = FVector4(1.0f, 1.0f, 1.0f, 1.0f);
			Settings.ColorGamma = FVector4(1.0f, 1.0f, 1.0f, 1.0f);
			Settings.ColorSaturation = FVector4(1.0f, 1.0f, 1.0f, 1.0f);
			Settings.SceneColorTint = FLinearColor::White;
			bChanged = true;
		}
		if (Settings.bOverride_BloomIntensity)
		{
			Settings.bOverride_BloomIntensity = false;
			bChanged = true;
		}
		if (Settings.bOverride_VignetteIntensity)
		{
			Settings.bOverride_VignetteIntensity = false;
			bChanged = true;
		}

		if (bChanged)
		{
			++SanitizedActorCount;
			UE_LOG(LogHorrorProject, Log, TEXT("Disabled imported post-process volume %s for Scrapopolis readability."), *GetNameSafe(PostProcessVolume));
		}
	}

	for (TActorIterator<AExponentialHeightFog> It(World); It; ++It)
	{
		AExponentialHeightFog* FogActor = *It;
		UExponentialHeightFogComponent* FogComponent = FogActor ? FogActor->GetComponent() : nullptr;
		if (!FogComponent)
		{
			continue;
		}

		bool bChanged = false;
		if (FogComponent->FogDensity > ScrapopolisReadableFogDensity)
		{
			FogComponent->SetFogDensity(ScrapopolisReadableFogDensity);
			bChanged = true;
		}
		if (FogComponent->SecondFogData.FogDensity > ScrapopolisReadableFogDensity)
		{
			FogComponent->SetSecondFogDensity(ScrapopolisReadableFogDensity);
			bChanged = true;
		}
		if (FogComponent->FogMaxOpacity > ScrapopolisReadableFogMaxOpacity)
		{
			FogComponent->SetFogMaxOpacity(ScrapopolisReadableFogMaxOpacity);
			bChanged = true;
		}
		if (FogComponent->bEnableVolumetricFog)
		{
			FogComponent->SetVolumetricFog(false);
			bChanged = true;
		}
		if (FogComponent->VolumetricFogExtinctionScale > 1.0f)
		{
			FogComponent->SetVolumetricFogExtinctionScale(1.0f);
			bChanged = true;
		}

		if (bChanged)
		{
			++SanitizedActorCount;
			UE_LOG(LogHorrorProject, Log, TEXT("Clamped imported fog actor %s for Scrapopolis readability."), *GetNameSafe(FogActor));
		}
	}

	APointLight* RescueLight = nullptr;
	for (TActorIterator<APointLight> It(World); It; ++It)
	{
		APointLight* PointLight = *It;
		if (PointLight && PointLight->Tags.Contains(CampaignVisibilityRescueLightTag))
		{
			RescueLight = PointLight;
			break;
		}
	}

	bool bRescueLightChanged = false;
	if (!RescueLight)
	{
		const FTransform AnchorTransform = ResolveCampaignRuntimeAnchorTransform();
		FActorSpawnParameters SpawnParameters;
		SpawnParameters.Owner = this;
		SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		RescueLight = World->SpawnActor<APointLight>(
			APointLight::StaticClass(),
			FTransform(FRotator::ZeroRotator, AnchorTransform.GetLocation() + FVector(0.0f, 0.0f, 260.0f)),
			SpawnParameters);
		if (RescueLight)
		{
			RescueLight->Tags.Add(CampaignVisibilityRescueLightTag);
			RescueLight->Tags.Add(CampaignAtmosphereLightTag);
			bRescueLightChanged = true;
		}
	}

	if (RescueLight)
	{
		if (ULightComponent* LightComponent = RescueLight->GetLightComponent())
		{
			if (LightComponent->Intensity < ScrapopolisRescueLightIntensity)
			{
				LightComponent->SetIntensity(ScrapopolisRescueLightIntensity);
				bRescueLightChanged = true;
			}
			LightComponent->SetLightColor(FLinearColor(0.82f, 0.9f, 1.0f));
		}
		if (ULocalLightComponent* LocalLightComponent = Cast<ULocalLightComponent>(RescueLight->GetLightComponent()))
		{
			if (LocalLightComponent->AttenuationRadius < ScrapopolisRescueLightRadius)
			{
				LocalLightComponent->SetAttenuationRadius(ScrapopolisRescueLightRadius);
				LocalLightComponent->AttenuationRadius = ScrapopolisRescueLightRadius;
				bRescueLightChanged = true;
			}
		}
	}

	if (bRescueLightChanged)
	{
		++SanitizedActorCount;
	}

	return SanitizedActorCount;
}

void AHorrorGameModeBase::ClearCampaignRuntimeActors()
{
	for (AHorrorCampaignObjectiveActor* ObjectiveActor : RuntimeCampaignObjectiveActorViews)
	{
		if (ObjectiveActor && !ObjectiveActor->IsActorBeingDestroyed())
		{
			ObjectiveActor->Destroy();
		}
	}

	RuntimeCampaignObjectiveActorViews.Reset();
	RuntimeCampaignObjectiveActors.Reset();

	if (RuntimeCampaignBoss && !RuntimeCampaignBoss->IsActorBeingDestroyed())
	{
		RuntimeCampaignBoss->Destroy();
	}
	RuntimeCampaignBoss = nullptr;

	if (RuntimeCampaignAmbushThreat && !RuntimeCampaignAmbushThreat->IsActorBeingDestroyed())
	{
		RuntimeCampaignAmbushThreat->Destroy();
	}
	RuntimeCampaignAmbushThreat = nullptr;
	ActiveCampaignAmbushSourceId = NAME_None;
}

void AHorrorGameModeBase::SpawnCampaignObjectives(const FHorrorCampaignChapterDefinition& Chapter)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const TSubclassOf<AHorrorCampaignObjectiveActor> SpawnClass = RuntimeCampaignObjectiveClass
		? RuntimeCampaignObjectiveClass
		: TSubclassOf<AHorrorCampaignObjectiveActor>(AHorrorCampaignObjectiveActor::StaticClass());

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = this;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	for (const FHorrorCampaignObjectiveDefinition& Objective : Chapter.Objectives)
	{
		AHorrorCampaignObjectiveActor* ObjectiveActor = World->SpawnActor<AHorrorCampaignObjectiveActor>(
			SpawnClass,
			ResolveCampaignObjectiveTransform(Objective),
			SpawnParameters);
		if (!ObjectiveActor)
		{
			continue;
		}

		ObjectiveActor->ConfigureObjective(Chapter.ChapterId, Objective);
		RuntimeCampaignObjectiveActors.Add(ObjectiveActor);
		RuntimeCampaignObjectiveActorViews.Add(ObjectiveActor);
	}

	RefreshCampaignObjectiveActors();
}

void AHorrorGameModeBase::SpawnCampaignBossIfNeeded(const FHorrorCampaignChapterDefinition& Chapter)
{
	if (!Chapter.bRequiresBoss || RuntimeCampaignBoss)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const TSubclassOf<AHorrorCampaignBossActor> SpawnClass = RuntimeCampaignBossClass
		? RuntimeCampaignBossClass
		: TSubclassOf<AHorrorCampaignBossActor>(AHorrorCampaignBossActor::StaticClass());

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = this;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	RuntimeCampaignBoss = World->SpawnActor<AHorrorCampaignBossActor>(
		SpawnClass,
		ResolveCampaignBossTransform(),
		SpawnParameters);
	if (RuntimeCampaignBoss)
	{
		RuntimeCampaignBoss->ConfigureBoss(
			Chapter.ChapterId,
			NSLOCTEXT("HorrorGameMode", "StoneGiant", "石像巨人"),
			FHorrorCampaign::CountObjectivesOfType(Chapter, EHorrorCampaignObjectiveType::BossWeakPoint));
	}
}

void AHorrorGameModeBase::RefreshCampaignObjectiveActors()
{
	for (AHorrorCampaignObjectiveActor* ObjectiveActor : RuntimeCampaignObjectiveActorViews)
	{
		if (ObjectiveActor && !ObjectiveActor->IsActorBeingDestroyed())
		{
			ObjectiveActor->RefreshObjectiveState();
		}
	}
}

void AHorrorGameModeBase::ShowCampaignMessage(FText Message, FLinearColor Color, float DurationSeconds) const
{
	if (Message.IsEmpty())
	{
		return;
	}

	if (GetWorld())
	{
		if (AHorrorPlayerController* PlayerController = Cast<AHorrorPlayerController>(UGameplayStatics::GetPlayerController(this, 0)))
		{
			PlayerController->ShowPlayerMessage(Message, Color, DurationSeconds);
			return;
		}
	}

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(INDEX_NONE, DurationSeconds, Color.ToFColor(true), Message.ToString());
	}
}

void AHorrorGameModeBase::ShowCampaignChapterIntro(const FHorrorCampaignChapterDefinition& Chapter) const
{
	const FText IntroText = Chapter.StoryBrief.IsEmpty()
		? Chapter.Title
		: FText::Format(FText::FromString(TEXT("{0}\n{1}")), Chapter.Title, Chapter.StoryBrief);

	ShowCampaignMessage(IntroText, FLinearColor(0.4f, 0.85f, 1.0f), 5.0f);
}

void AHorrorGameModeBase::ShowCurrentCampaignObjectiveHint() const
{
	const FHorrorCampaignObjectiveDefinition* NextObjective = CampaignProgress.GetNextObjective();
	if (!NextObjective)
	{
		return;
	}

	ShowCampaignMessage(
		FText::Format(NSLOCTEXT("HorrorGameMode", "CurrentObjective", "当前目标：{0}"), NextObjective->PromptText),
		FLinearColor(0.86f, 0.95f, 1.0f),
		3.0f);
}

void AHorrorGameModeBase::QueueCampaignAutoTravelIfNeeded()
{
	if (!bAutoTravelToNextCampaignMapOnChapterComplete || bCampaignAutoTravelQueued)
	{
		return;
	}

	const FHorrorCampaignChapterDefinition* Chapter = GetCurrentCampaignChapterDefinition();
	if (!Chapter || Chapter->bIsFinalChapter)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	FString NextMapPackageName;
	if (!FHorrorMapChain::TryGetNextMapPackage(World->GetPackage()->GetName(), NextMapPackageName))
	{
		return;
	}

	bCampaignAutoTravelQueued = true;
	ShowCampaignMessage(
		Chapter->CompletionBridgeText.IsEmpty()
			? NSLOCTEXT("HorrorGameMode", "TravelingToNextMap", "信号稳定，正在前往下一张地图...")
			: Chapter->CompletionBridgeText,
		FLinearColor(0.5f, 0.85f, 1.0f),
		CampaignAutoTravelDelaySeconds);

	World->GetTimerManager().SetTimer(
		CampaignAutoTravelTimerHandle,
		this,
		&AHorrorGameModeBase::ExecuteCampaignAutoTravel,
		FMath::Max(0.05f, CampaignAutoTravelDelaySeconds),
		false);
}

void AHorrorGameModeBase::ExecuteCampaignAutoTravel()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		bCampaignAutoTravelQueued = false;
		return;
	}

	FString NextMapPackageName;
	if (!FHorrorMapChain::TryGetNextMapPackage(World->GetPackage()->GetName(), NextMapPackageName))
	{
		bCampaignAutoTravelQueued = false;
		return;
	}

	const FString TravelOptions = FHorrorMapChain::BuildTravelOptionsForConfiguredGameMode();
	UGameplayStatics::OpenLevel(World, FName(*NextMapPackageName), true, TravelOptions);
}

void AHorrorGameModeBase::SanitizeImportedMapChainRuntime()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const FString CurrentMapPackageName = FHorrorMapChain::NormalizeMapPackageName(World->GetPackage()->GetName());
	const int32 CurrentMapIndex = FHorrorMapChain::FindMapIndex(CurrentMapPackageName);
	if (CurrentMapIndex <= 0)
	{
		return;
	}

	StopImportedMapShowcaseSequences();
	RestoreLeadPlayerViewAndInput();

	World->GetTimerManager().SetTimerForNextTick(this, &AHorrorGameModeBase::StopImportedMapShowcaseSequences);
	World->GetTimerManager().SetTimerForNextTick(this, &AHorrorGameModeBase::RestoreLeadPlayerViewAndInput);

	FTimerHandle DelayedSequenceCleanupTimer;
	World->GetTimerManager().SetTimer(DelayedSequenceCleanupTimer, this, &AHorrorGameModeBase::StopImportedMapShowcaseSequences, 0.35f, false);

	FTimerHandle DelayedPlayerViewRestoreTimer;
	World->GetTimerManager().SetTimer(DelayedPlayerViewRestoreTimer, this, &AHorrorGameModeBase::RestoreLeadPlayerViewAndInput, 0.35f, false);
}

void AHorrorGameModeBase::StopImportedMapShowcaseSequences()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	for (TActorIterator<ALevelSequenceActor> It(World); It; ++It)
	{
		ALevelSequenceActor* SequenceActor = *It;
		ULevelSequencePlayer* SequencePlayer = SequenceActor ? SequenceActor->GetSequencePlayer() : nullptr;
		if (!SequencePlayer)
		{
			continue;
		}

		if (SequencePlayer->IsPlaying())
		{
			SequencePlayer->Stop();
			UE_LOG(LogHorrorProject, Log, TEXT("Stopped imported map showcase sequence %s so player view stays possessed."),
				*GetNameSafe(SequenceActor));
		}
	}
}

void AHorrorGameModeBase::RestoreLeadPlayerViewAndInput()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	UGameplayStatics::SetGamePaused(World, false);

	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(World, 0);
	if (!PlayerController)
	{
		return;
	}

	PlayerController->SetIgnoreMoveInput(false);
	PlayerController->SetIgnoreLookInput(false);
	PlayerController->SetInputMode(FInputModeGameOnly());
	PlayerController->bShowMouseCursor = false;
	ClearImportedMapCameraFade(PlayerController);

	if (APawn* PossessedPawn = PlayerController->GetPawn())
	{
		PlayerController->SetViewTarget(PossessedPawn);
	}
}

void AHorrorGameModeBase::RegisterDefaultObjectiveMetadata()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	UHorrorEventBusSubsystem* EventBus = World->GetSubsystem<UHorrorEventBusSubsystem>();
	if (!EventBus)
	{
		return;
	}

	const FHorrorGameModeDefaultMilestoneMetadata Defaults[] = {
		{ HorrorFoundFootageTags::BodycamAcquiredEvent(), HorrorObjectiveMilestoneSources::Bodycam, TEXT("Trailer.Beat.BodycamAcquired"), TEXT("随身摄像机已上线，请保持举起。"), TEXT("已取得随身摄像机") },
		{ HorrorFoundFootageTags::FirstNoteCollectedEvent(), HorrorObjectiveMilestoneSources::FirstNote, TEXT("Trailer.Beat.FirstNote"), TEXT("备忘录已记录，寻找第一个异常。"), TEXT("第一份备忘录已记录") },
		{ HorrorFoundFootageTags::FirstAnomalyRecordedEvent(), HorrorObjectiveMilestoneSources::FirstAnomaly, TEXT("Trailer.Beat.FirstAnomaly"), TEXT("异常已捕捉，前往档案终端。"), TEXT("第一异常已记录") },
		{ HorrorFoundFootageTags::ArchiveReviewedEvent(), HorrorObjectiveMilestoneSources::Archive, TEXT("Trailer.Beat.ArchiveReviewed"), TEXT("档案复查完成，出口正在解锁。"), TEXT("档案已复查") },
		{ HorrorFoundFootageTags::ExitUnlockedEvent(), HorrorObjectiveMilestoneSources::Exit, TEXT("Trailer.Beat.ExitUnlocked"), TEXT("出口已解锁，立刻离开。"), TEXT("出口已解锁") },
		{ HorrorDay1Tags::Day1CompletedEvent(), HorrorObjectiveMilestoneSources::Day1Complete, TEXT("Trailer.Beat.Day1Complete"), TEXT("第一天完成。"), TEXT("第一天完成") }
	};

	for (const FHorrorGameModeDefaultMilestoneMetadata& Default : Defaults)
	{
		RegisterHorrorGameModeMilestoneMetadata(*EventBus, Default);
	}
}

void AHorrorGameModeBase::HandleObjectiveStateChange(FGameplayTag StateTag)
{
	if (!StateTag.IsValid())
	{
		return;
	}

	HandleEncounterObjectiveStateChange(StateTag);
	HandleAutosaveObjectiveStateChange(StateTag);
}

void AHorrorGameModeBase::HandleEncounterObjectiveStateChange(FGameplayTag StateTag)
{
	if (AHorrorEncounterDirector* EncounterDirector = EnsureEncounterDirector())
	{
		if (StateTag == HorrorFoundFootageTags::FirstNoteCollectedState())
		{
			EncounterDirector->PrimeEncounter(NAME_None);
		}
		else if (StateTag == HorrorFoundFootageTags::ArchiveReviewedState())
		{
			EncounterDirector->ResolveEncounter();
		}
	}
}

void AHorrorGameModeBase::HandleAutosaveObjectiveStateChange(FGameplayTag StateTag)
{
	const FName CheckpointId = GetHorrorGameModeMilestoneCheckpoint(StateTag);
	if (!CheckpointId.IsNone())
	{
		TryAutosaveOnMilestone(CheckpointId);
	}
}

void AHorrorGameModeBase::RestoreEncounterStateFromFoundFootageCheckpoint()
{
	AHorrorEncounterDirector* EncounterDirector = EnsureEncounterDirector();
	if (!EncounterDirector)
	{
		return;
	}

	EHorrorEncounterPhase RestoredPhase = EHorrorEncounterPhase::Dormant;
	if (HasReviewedArchive() || IsExitUnlocked() || IsDay1Complete())
	{
		RestoredPhase = EHorrorEncounterPhase::Resolved;
	}
	else if (HasCollectedFirstNote())
	{
		RestoredPhase = EHorrorEncounterPhase::Primed;
	}

	EncounterDirector->RestoreForCheckpoint(
		RestoredPhase,
		RestoredPhase == EHorrorEncounterPhase::Dormant ? NAME_None : EncounterDirector->DefaultEncounterId,
		ResolveLeadPlayerCharacter());
}

void AHorrorGameModeBase::TryAutosaveOnMilestone(FName CheckpointId)
{
	if (!bAutosaveOnObjectiveMilestone || CheckpointId.IsNone())
	{
		return;
	}

	const bool bSaved = SaveDay1Checkpoint(CheckpointId);
	if (UWorld* World = GetWorld())
	{
		if (UHorrorEventBusSubsystem* EventBus = World->GetSubsystem<UHorrorEventBusSubsystem>())
		{
			EventBus->Publish(
				bSaved ? HorrorSaveTags::CheckpointSavedEvent() : HorrorSaveTags::CheckpointSaveFailedEvent(),
				CheckpointId,
				bSaved ? HorrorSaveTags::CheckpointSavedState() : HorrorSaveTags::CheckpointSaveFailedState(),
				this);
		}
	}
}
