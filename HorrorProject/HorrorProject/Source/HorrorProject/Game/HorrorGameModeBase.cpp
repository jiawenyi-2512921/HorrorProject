// Copyright Epic Games, Inc. All Rights Reserved.

#include "Game/HorrorGameModeBase.h"

#include "Audio/HorrorAudioSubsystem.h"
#include "EngineUtils.h"
#include "Game/DeepWaterStationRouteKit.h"
#include "Game/HorrorEncounterDirector.h"
#include "Game/HorrorEventBusSubsystem.h"
#include "Engine/GameInstance.h"
#include "GameFramework/PlayerStart.h"
#include "Engine/PlayerStartPIE.h"
#include "HorrorProject.h"
#include "Kismet/GameplayStatics.h"
#include "Player/HorrorPlayerCharacter.h"
#include "Player/Components/QuantumCameraComponent.h"
#include "Player/Components/VHSEffectComponent.h"
#include "Player/HorrorPlayerController.h"
#include "Save/HorrorSaveSubsystem.h"
#include "UI/Day1SliceHUD.h"

namespace
{
	constexpr float SpawnSearchVerticalStepCm = 50.0f;
	constexpr int32 SpawnSearchVerticalSteps = 8;
	constexpr float SpawnSearchRingStepCm = 100.0f;
	constexpr int32 SpawnSearchRingCount = 8;

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
	const FText DeathFeedback = FText::FromString(TEXT("信号丢失。正在恢复到上一个检查点……"));
	const FText FailureFeedback = FText::FromString(TEXT("行动失败。正在恢复到上一个检查点……"));
	const FText MissingCheckpointFeedback = FText::FromString(TEXT("没有可用检查点。"));
}

namespace
{
	void EnsureRouteKitObjectives(ADeepWaterStationRouteKit& RouteKit)
	{
		if (RouteKit.ObjectiveNodes.IsEmpty())
		{
			RouteKit.ConfigureDefaultFirstLoopObjectiveNodes();
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

	if (bAutoSpawnRouteKitOnBeginPlay)
	{
		EnsureRouteKit();
	}

	if (bAutoSpawnEncounterDirectorOnBeginPlay)
	{
		EnsureEncounterDirector();
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
		TEXT("Player failure requested: %s. Attempting Day1 checkpoint recovery."),
		*ResolvedFailureCause.ToString());

	if (World)
	{
		if (UHorrorAudioSubsystem* AudioSubsystem = World->GetSubsystem<UHorrorAudioSubsystem>())
		{
			AudioSubsystem->HandleDay1EventName(TEXT("Day1.Failure"), ResolvedFailureCause);
		}
	}

	bLastPlayerFailureRecoveredFromCheckpoint = LoadDay1Checkpoint();
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
		{ HorrorFoundFootageTags::BodycamAcquiredEvent(), HorrorObjectiveMilestoneSources::Bodycam, TEXT("Trailer.Beat.BodycamAcquired"), TEXT("随身摄像机已上线，保持举起。"), TEXT("已取得随身摄像机") },
		{ HorrorFoundFootageTags::FirstNoteCollectedEvent(), HorrorObjectiveMilestoneSources::FirstNote, TEXT("Trailer.Beat.FirstNote"), TEXT("笔记已记录，寻找第一个异常点。"), TEXT("已记录第一份笔记") },
		{ HorrorFoundFootageTags::FirstAnomalyRecordedEvent(), HorrorObjectiveMilestoneSources::FirstAnomaly, TEXT("Trailer.Beat.FirstAnomaly"), TEXT("异常已捕获，前往档案终端。"), TEXT("已记录第一个异常") },
		{ HorrorFoundFootageTags::ArchiveReviewedEvent(), HorrorObjectiveMilestoneSources::Archive, TEXT("Trailer.Beat.ArchiveReviewed"), TEXT("档案审查完成，出口正在解锁。"), TEXT("档案已审查") },
		{ HorrorFoundFootageTags::ExitUnlockedEvent(), HorrorObjectiveMilestoneSources::Exit, TEXT("Trailer.Beat.ExitUnlocked"), TEXT("出口已解锁，立刻离开。"), TEXT("出口已解锁") },
		{ HorrorDay1Tags::Day1CompletedEvent(), HorrorObjectiveMilestoneSources::Day1Complete, TEXT("Trailer.Beat.Day1Complete"), TEXT("第 1 天完成。"), TEXT("第 1 天完成") }
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
