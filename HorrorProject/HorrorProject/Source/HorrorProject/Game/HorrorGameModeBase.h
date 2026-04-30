// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Game/HorrorAnomalyDirector.h"
#include "Game/HorrorCampaign.h"
#include "Game/HorrorFoundFootageContract.h"
#include "HorrorGameModeBase.generated.h"

class ADeepWaterStationRouteKit;
class AHorrorCampaignBossActor;
class AHorrorCampaignObjectiveActor;
class AHorrorEncounterDirector;
class AHorrorMapChainExit;
class APlayerController;
class AHorrorPlayerCharacter;
class UHorrorSaveSubsystem;

USTRUCT(BlueprintType)
struct HORRORPROJECT_API FHorrorCampaignAtmosphereTuning
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Atmosphere", meta=(ClampMin="0.0", ClampMax="1.0"))
	float LightScale = 0.28f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Atmosphere")
	float AutoExposureBias = -1.15f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Atmosphere", meta=(ClampMin="0.0", ClampMax="1.0"))
	float VignetteIntensity = 0.58f;
};

USTRUCT(BlueprintType)
struct HORRORPROJECT_API FHorrorCampaignAmbushThreatTuning
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Ambush", meta=(ClampMin="0.1"))
	float ActorScale = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Ambush", meta=(ClampMin="0.0", Units="cm/s"))
	float MoveSpeed = 180.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Ambush", meta=(ClampMin="0.0", Units="cm"))
	float EngageRadius = 3800.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Ambush", meta=(ClampMin="0.0", Units="cm"))
	float AttackRadius = 120.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Ambush", meta=(ClampMin="0.0", Units="cm"))
	float FearPressureRadius = 1500.0f;
};

/**
 * Defines Horror Game Mode Base behavior for the Game module.
 */
UCLASS()
class HORRORPROJECT_API AHorrorGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public:
	AHorrorGameModeBase();

	UFUNCTION(BlueprintCallable, Category="Horror|Objectives")
	bool TryAcquireBodycam(FName SourceId, bool bEnableBodycam);

	UFUNCTION(BlueprintCallable, Category="Horror|Objectives")
	bool TryCollectFirstNote(FName SourceId);

	UFUNCTION(BlueprintCallable, Category="Horror|Objectives")
	bool BeginFirstAnomalyCandidate(FName SourceId);

	UFUNCTION(BlueprintCallable, Category="Horror|Objectives")
	bool TryRecordFirstAnomaly(bool bIsRecording);

	UFUNCTION(BlueprintCallable, Category="Horror|Objectives")
	bool TryReviewArchive(FName SourceId);

	UFUNCTION(BlueprintPure, Category="Horror|Objectives")
	bool HasBodycamAcquired() const;

	UFUNCTION(BlueprintPure, Category="Horror|Objectives")
	bool HasCollectedFirstNote() const;

	UFUNCTION(BlueprintPure, Category="Horror|Objectives")
	bool HasRecordedFirstAnomaly() const;

	UFUNCTION(BlueprintPure, Category="Horror|Objectives")
	bool HasPendingFirstAnomalyCandidate() const;

	UFUNCTION(BlueprintPure, Category="Horror|Objectives")
	FName GetPendingFirstAnomalySourceId() const;

	UFUNCTION(BlueprintPure, Category="Horror|Objectives")
	bool HasReviewedArchive() const;

	UFUNCTION(BlueprintPure, Category="Horror|Objectives")
	bool HasFoundFootageRecordedEvent(FGameplayTag EventTag) const;

	UFUNCTION(BlueprintPure, Category="Horror|Objectives")
	bool HasFoundFootageCompletedState(FGameplayTag StateTag) const;

	UFUNCTION(BlueprintPure, Category="Horror|Objectives")
	bool CanCollectFirstNote() const;

	UFUNCTION(BlueprintPure, Category="Horror|Objectives")
	bool CanBeginFirstAnomalyCandidate() const;

	UFUNCTION(BlueprintPure, Category="Horror|Objectives")
	bool CanRecordFirstAnomaly(bool bIsRecording) const;

	UFUNCTION(BlueprintPure, Category="Horror|Objectives")
	bool CanReviewArchive() const;

	UFUNCTION(BlueprintPure, Category="Horror|Objectives")
	bool IsExitUnlocked() const;

	UFUNCTION(BlueprintCallable, Category="Horror|Objectives")
	bool TryCompleteDay1(FName SourceId);

	UFUNCTION(BlueprintPure, Category="Horror|Objectives")
	bool IsDay1Complete() const { return bDay1Complete; }

	void ImportDay1CompleteState(bool bInDay1Complete);

	UFUNCTION(BlueprintPure, Category="Horror|Objectives")
	FHorrorFoundFootageProgressSnapshot BuildFoundFootageProgressSnapshot() const;

	UFUNCTION(BlueprintPure, Category="Horror|Objectives")
	FHorrorObjectiveTrackerSnapshot BuildObjectiveTrackerSnapshot() const;

	const FHorrorFoundFootageContract& GetFoundFootageContract() const;
	FHorrorFoundFootageSaveState ExportFoundFootageSaveState() const;
	void ImportFoundFootageSaveState(const FHorrorFoundFootageSaveState& SaveState);
	void ImportPendingFirstAnomalyCandidate(FName SourceId);

	UFUNCTION(BlueprintCallable, Category="Horror|Objectives")
	void SyncFoundFootageRuntimeStateToPlayer();

	UFUNCTION(BlueprintPure, Category="Horror|Encounter")
	AHorrorEncounterDirector* GetRuntimeEncounterDirector() const { return RuntimeEncounterDirector.Get(); }

	UFUNCTION(BlueprintPure, Category="Horror|Route")
	ADeepWaterStationRouteKit* GetRuntimeRouteKit() const { return RuntimeRouteKit.Get(); }

	UFUNCTION(BlueprintPure, Category="Horror|Map Chain")
	AHorrorMapChainExit* GetRuntimeMapChainExit() const { return RuntimeMapChainExit.Get(); }

	UFUNCTION(BlueprintPure, Category="Horror|Campaign")
	bool CanCompleteCampaignObjective(FName ChapterId, FName ObjectiveId) const;

	UFUNCTION(BlueprintCallable, Category="Horror|Campaign")
	bool TryCompleteCampaignObjective(FName ChapterId, FName ObjectiveId, AActor* InstigatorActor);

	UFUNCTION(BlueprintPure, Category="Horror|Campaign")
	bool IsCurrentCampaignChapterComplete() const;

	UFUNCTION(BlueprintPure, Category="Horror|Campaign")
	FName GetCurrentCampaignChapterId() const { return CampaignProgress.GetActiveChapterId(); }

	UFUNCTION(BlueprintPure, Category="Horror|Campaign")
	int32 GetCampaignCompletedObjectiveCount() const { return CampaignProgress.GetCompletedObjectiveCount(); }

	UFUNCTION(BlueprintPure, Category="Horror|Campaign")
	int32 GetCampaignRequiredObjectiveCount() const { return CampaignProgress.GetRequiredObjectiveCount(); }

	UFUNCTION(BlueprintPure, Category="Horror|Campaign")
	bool IsCampaignBossDefeated() const { return CampaignProgress.IsBossDefeated(); }

	UFUNCTION(BlueprintPure, Category="Horror|Campaign")
	bool TryGetCurrentCampaignObjectiveWorldLocation(FVector& OutLocation) const;

	UFUNCTION(BlueprintPure, Category="Horror|Campaign")
	FText GetCurrentCampaignObjectivePromptText() const;

	UFUNCTION(BlueprintPure, Category="Horror|Campaign")
	FText GetCurrentCampaignObjectiveActionText() const;

	UFUNCTION(BlueprintCallable, Category="Horror|Campaign")
	int32 ApplyCampaignHorrorAtmosphere();

	UFUNCTION(BlueprintCallable, Category="Horror|Campaign")
	bool StartCampaignAmbushThreat(FName SourceId, AActor* ThreatAnchor);

	UFUNCTION(BlueprintCallable, Category="Horror|Campaign")
	void StopCampaignAmbushThreat(FName SourceId);

	static bool ClearImportedMapCameraFade(APlayerController* PlayerController);

	FHorrorCampaignAtmosphereTuning ResolveCampaignAtmosphereTuningForMapForTests(const FString& MapPackageName) const;
	FHorrorCampaignAmbushThreatTuning ResolveCampaignAmbushThreatTuningForMapForTests(const FString& MapPackageName) const;
	int32 SanitizeImportedMapVisualObstructionsForTests(const FString& MapPackageName);

	const FHorrorCampaignChapterDefinition* GetCurrentCampaignChapterDefinition() const;
	const TArray<AHorrorCampaignObjectiveActor*>& GetRuntimeCampaignObjectivesForTests() const { return RuntimeCampaignObjectiveActorViews; }

	void ResetCampaignProgressForChapterForTests(FName ChapterId);

	UFUNCTION(BlueprintCallable, Category="Horror|Save")
	bool SaveDay1Checkpoint(FName CheckpointId);

	UFUNCTION(BlueprintCallable, Category="Horror|Save")
	bool LoadDay1Checkpoint();

	UFUNCTION(BlueprintCallable, Category="Horror|Player Failure")
	bool RequestPlayerDeath(FName DeathCause);

	UFUNCTION(BlueprintCallable, Category="Horror|Player Failure")
	bool RequestPlayerFailure(FName FailureCause, FText FeedbackText);

	UFUNCTION(BlueprintPure, Category="Horror|Player Failure")
	FName GetLastPlayerFailureCause() const { return LastPlayerFailureCause; }

	UFUNCTION(BlueprintPure, Category="Horror|Player Failure")
	bool DidLastPlayerFailureRecoverFromCheckpoint() const { return bLastPlayerFailureRecoveredFromCheckpoint; }

	UFUNCTION(BlueprintPure, Category="Horror|Player Failure")
	int32 GetPlayerFailureCount() const { return PlayerFailureCount; }

	UFUNCTION(BlueprintPure, Category="Horror|Objectives")
	bool IsLeadPlayerRecording() const;

	UFUNCTION(BlueprintPure, Category="Horror|Bootstrap")
	bool ShouldAutoGrantBodycamOnPlayerBeginPlay() const { return bAutoGrantBodycamOnPlayerBeginPlay; }

	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
	virtual void BeginPlay() override;
	virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;
	virtual APawn* SpawnDefaultPawnAtTransform_Implementation(AController* NewPlayer, const FTransform& SpawnTransform) override;

private:
	static constexpr float DefaultRuntimeRouteKitHeight = 80.0f;

	bool RecordFoundFootageEvent(FGameplayTag EventTag, FName SourceId);
	AHorrorPlayerCharacter* ResolveLeadPlayerCharacter() const;
	ADeepWaterStationRouteKit* EnsureRouteKit();
	FTransform ResolveRuntimeRouteKitSpawnTransform() const;
	AHorrorEncounterDirector* EnsureEncounterDirector();
	AHorrorMapChainExit* EnsureMapChainExit();
	bool EnsureCampaignRuntime();
	void RegisterDefaultObjectiveMetadata();
	void HandleObjectiveStateChange(FGameplayTag StateTag);
	void HandleEncounterObjectiveStateChange(FGameplayTag StateTag);
	void HandleAutosaveObjectiveStateChange(FGameplayTag StateTag);
	void RestoreEncounterStateFromFoundFootageCheckpoint();
	void TryAutosaveOnMilestone(FName CheckpointId);
	FTransform ResolveMapChainExitSpawnTransform() const;
	void SanitizeImportedMapChainRuntime();
	void StopImportedMapShowcaseSequences();
	void RestoreLeadPlayerViewAndInput();
	bool ShouldAutoSpawnLegacyRouteKitInCurrentMap() const;
	FTransform ResolveCampaignRuntimeAnchorTransform() const;
	FTransform ResolveCampaignObjectiveTransform(const FHorrorCampaignObjectiveDefinition& Objective) const;
	FTransform ResolveCampaignBossTransform() const;
	void ClearCampaignRuntimeActors();
	void SpawnCampaignObjectives(const FHorrorCampaignChapterDefinition& Chapter);
	void SpawnCampaignBossIfNeeded(const FHorrorCampaignChapterDefinition& Chapter);
	void RefreshCampaignObjectiveActors();
	FTransform ResolveCampaignAmbushThreatTransform(const AActor* ThreatAnchor) const;
	FHorrorCampaignAtmosphereTuning ResolveCampaignAtmosphereTuningForMap(const FString& MapPackageName) const;
	FHorrorCampaignAmbushThreatTuning ResolveCampaignAmbushThreatTuningForMap(const FString& MapPackageName) const;
	bool ShouldSanitizeImportedMapVisualObstructions(const FString& MapPackageName) const;
	int32 SanitizeImportedMapVisualObstructions(const FString& MapPackageName);
	void ShowCampaignMessage(FText Message, FLinearColor Color, float DurationSeconds) const;
	void ShowCampaignChapterIntro(const FHorrorCampaignChapterDefinition& Chapter) const;
	void ShowCurrentCampaignObjectiveHint() const;
	bool ShouldUseCampaignFailureRecovery(FName FailureCause) const;
	FTransform ResolveCampaignFailureRecoveryTransform() const;
	bool TryRecoverFromCampaignFailure(FName FailureCause);
	void QueueCampaignAutoTravelIfNeeded();
	void ExecuteCampaignAutoTravel();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Bootstrap", meta=(AllowPrivateAccess="true"))
	bool bAutoGrantBodycamOnPlayerBeginPlay = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Route", meta=(AllowPrivateAccess="true"))
	bool bAutoSpawnRouteKitOnBeginPlay = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Route", meta=(AllowPrivateAccess="true"))
	FTransform RuntimeRouteKitTransform = FTransform(FRotator::ZeroRotator, FVector(0.0f, 0.0f, DefaultRuntimeRouteKitHeight));

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Route", meta=(AllowPrivateAccess="true"))
	bool bAnchorRuntimeRouteKitToPlayerStart = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Spawn", meta=(AllowPrivateAccess="true"))
	FName PreferredPlayerStartTag = TEXT("DeepWaterStationBase");

	UPROPERTY(Transient)
	TObjectPtr<ADeepWaterStationRouteKit> RuntimeRouteKit;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Encounter", meta=(AllowPrivateAccess="true"))
	bool bAutoSpawnEncounterDirectorOnBeginPlay = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Encounter", meta=(AllowPrivateAccess="true"))
	FTransform RuntimeEncounterDirectorTransform = FTransform::Identity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Encounter", meta=(AllowPrivateAccess="true"))
	TSubclassOf<AHorrorEncounterDirector> RuntimeEncounterDirectorClass;

	UPROPERTY(Transient)
	TObjectPtr<AHorrorEncounterDirector> RuntimeEncounterDirector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Map Chain", meta=(AllowPrivateAccess="true"))
	bool bAutoSpawnMapChainExitOnBeginPlay = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Map Chain", meta=(AllowPrivateAccess="true"))
	TSubclassOf<AHorrorMapChainExit> RuntimeMapChainExitClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Map Chain", meta=(AllowPrivateAccess="true"))
	FVector RuntimeMapChainExitOffset = FVector(380.0f, 160.0f, 110.0f);

	UPROPERTY(Transient)
	TObjectPtr<AHorrorMapChainExit> RuntimeMapChainExit;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Map Chain", meta=(AllowPrivateAccess="true"))
	bool bSanitizeImportedMapShowcaseRuntimeOnBeginPlay = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign", meta=(AllowPrivateAccess="true"))
	bool bAutoSpawnCampaignRuntimeOnBeginPlay = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign", meta=(AllowPrivateAccess="true"))
	TSubclassOf<AHorrorCampaignObjectiveActor> RuntimeCampaignObjectiveClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign", meta=(AllowPrivateAccess="true"))
	TSubclassOf<AHorrorCampaignBossActor> RuntimeCampaignBossClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign", meta=(AllowPrivateAccess="true"))
	FVector RuntimeCampaignBossOffset = FVector(1450.0f, 0.0f, 100.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign", meta=(AllowPrivateAccess="true"))
	bool bAutoTravelToNextCampaignMapOnChapterComplete = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign", meta=(AllowPrivateAccess="true"))
	bool bAutoApplyCampaignHorrorAtmosphere = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Audio", meta=(AllowPrivateAccess="true"))
	bool bAutoStartDefaultHorrorAmbience = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign", meta=(AllowPrivateAccess="true", ClampMin="0.0", ClampMax="1.0"))
	float CampaignHorrorAtmosphereLightScale = 0.28f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign", meta=(AllowPrivateAccess="true"))
	float CampaignHorrorAtmosphereAutoExposureBias = -1.15f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign", meta=(AllowPrivateAccess="true", ClampMin="0.0", ClampMax="1.0"))
	float CampaignHorrorAtmosphereVignetteIntensity = 0.58f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign", meta=(AllowPrivateAccess="true", ClampMin="0.0", ClampMax="1.0"))
	float ScrapopolisAtmosphereLightScale = 0.62f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign", meta=(AllowPrivateAccess="true"))
	float ScrapopolisAtmosphereAutoExposureBias = -0.35f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign", meta=(AllowPrivateAccess="true", ClampMin="0.0", ClampMax="1.0"))
	float ScrapopolisAtmosphereVignetteIntensity = 0.34f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign", meta=(AllowPrivateAccess="true", ClampMin="0.0", Units="s"))
	float CampaignAutoTravelDelaySeconds = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Ambush", meta=(AllowPrivateAccess="true"))
	bool bSpawnCampaignAmbushThreats = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Ambush", meta=(AllowPrivateAccess="true"))
	FVector RuntimeCampaignAmbushThreatOffset = FVector(-2400.0f, 850.0f, 100.0f);

	UPROPERTY(Transient)
	TArray<TObjectPtr<AHorrorCampaignObjectiveActor>> RuntimeCampaignObjectiveActors;

	UPROPERTY(Transient)
	TObjectPtr<AHorrorCampaignBossActor> RuntimeCampaignBoss;

	UPROPERTY(Transient)
	TObjectPtr<AHorrorCampaignBossActor> RuntimeCampaignAmbushThreat;

	TArray<AHorrorCampaignObjectiveActor*> RuntimeCampaignObjectiveActorViews;
	FHorrorCampaignProgress CampaignProgress;
	FTimerHandle CampaignAutoTravelTimerHandle;
	bool bCampaignAutoTravelQueued = false;
	FName ActiveCampaignAmbushSourceId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Save", meta=(AllowPrivateAccess="true"))
	bool bAutosaveOnObjectiveMilestone = true;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Horror|Player Failure", meta=(AllowPrivateAccess="true"))
	FName LastPlayerFailureCause;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Horror|Player Failure", meta=(AllowPrivateAccess="true"))
	bool bLastPlayerFailureRecoveredFromCheckpoint = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Horror|Player Failure", meta=(AllowPrivateAccess="true"))
	int32 PlayerFailureCount = 0;

	UPROPERTY(Transient)
	bool bDay1Complete = false;

	FHorrorFoundFootageContract FoundFootageContract;
	FHorrorAnomalyDirector AnomalyDirector;
};
