// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HorrorGolemBehaviorComponent.generated.h"

class AHorrorThreatCharacter;
class AActor;

namespace HorrorGolemDefaults
{
	inline constexpr float DistantSightingMinDistanceCm = 3000.0f;
	inline constexpr float DistantSightingMoveSpeedCmPerSecond = 50.0f;
	inline constexpr float CloseStalkingMinDistanceCm = 1000.0f;
	inline constexpr float CloseStalkingMaxDistanceCm = 1500.0f;
	inline constexpr float CloseStalkingPatrolSpeedCmPerSecond = 150.0f;
	inline constexpr float CloseStalkingPatrolPauseSeconds = 2.0f;
	inline constexpr float ChaseTriggeredStartDistanceCm = 2000.0f;
	inline constexpr float ChaseTriggeredSpeedMultiplier = 0.7f;
	inline constexpr float ChaseTriggeredBaseSpeedCmPerSecond = 400.0f;
	inline constexpr float FullChaseMinDistanceCm = 1000.0f;
	inline constexpr float FullChaseMaxDistanceCm = 2500.0f;
	inline constexpr float FullChaseSpeedCmPerSecond = 600.0f;
	inline constexpr float FinalImpactTriggerDistanceCm = 500.0f;
}

UENUM(BlueprintType)
enum class EGolemEncounterState : uint8
{
	Dormant UMETA(DisplayName="休眠"),
	DistantSighting UMETA(DisplayName="远距离目击（30 米以上）"),
	CloseStalking UMETA(DisplayName="近距离跟踪（10-15 米）"),
	ChaseTriggered UMETA(DisplayName="追逐触发（20 米，70% 速度）"),
	FullChase UMETA(DisplayName="全速追逐（10-25 米，100% 速度）"),
	FinalImpact UMETA(DisplayName="最终冲击（5 米，攻击）")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FGolemStateChangedSignature, EGolemEncounterState, OldState, EGolemEncounterState, NewState);

/**
 * Adds Horror Golem Behavior Component behavior to its owning actor in the AI module.
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup=(Horror), meta=(BlueprintSpawnableComponent))
class HORRORPROJECT_API UHorrorGolemBehaviorComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UHorrorGolemBehaviorComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, Category="Horror|Golem")
	void ActivateBehavior(AActor* InTargetActor);

	UFUNCTION(BlueprintCallable, Category="Horror|Golem")
	void DeactivateBehavior();

	UFUNCTION(BlueprintPure, Category="Horror|Golem")
	bool IsBehaviorActive() const { return bBehaviorActive; }

	UFUNCTION(BlueprintPure, Category="Horror|Golem")
	EGolemEncounterState GetCurrentState() const { return CurrentState; }

	UFUNCTION(BlueprintPure, Category="Horror|Golem")
	AActor* GetTargetActor() const { return TargetActor.Get(); }

	UFUNCTION(BlueprintPure, Category="Horror|Golem")
	float GetDistanceToTarget() const;

	UFUNCTION(BlueprintCallable, Category="Horror|Golem")
	void ForceStateTransition(EGolemEncounterState NewState);

	UPROPERTY(BlueprintAssignable, Category="Horror|Golem")
	FGolemStateChangedSignature OnStateChanged;

	// Phase 1: Distant Sighting (30m+)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Golem|Phase1", meta=(ClampMin="0.0", Units="cm"))
	float DistantSightingMinDistance = HorrorGolemDefaults::DistantSightingMinDistanceCm;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Golem|Phase1", meta=(ClampMin="0.0", Units="cm/s"))
	float DistantSightingMoveSpeed = HorrorGolemDefaults::DistantSightingMoveSpeedCmPerSecond;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Golem|Phase1")
	bool bDistantSightingStationary = true;

	// Phase 2: Close Stalking (10-15m)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Golem|Phase2", meta=(ClampMin="0.0", Units="cm"))
	float CloseStalking_MinDistance = HorrorGolemDefaults::CloseStalkingMinDistanceCm;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Golem|Phase2", meta=(ClampMin="0.0", Units="cm"))
	float CloseStalking_MaxDistance = HorrorGolemDefaults::CloseStalkingMaxDistanceCm;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Golem|Phase2", meta=(ClampMin="0.0", Units="cm/s"))
	float CloseStalking_PatrolSpeed = HorrorGolemDefaults::CloseStalkingPatrolSpeedCmPerSecond;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Golem|Phase2", meta=(ClampMin="0.0", Units="s"))
	float CloseStalking_PatrolPauseTime = HorrorGolemDefaults::CloseStalkingPatrolPauseSeconds;

	// Phase 3: Chase Triggered (20m start, 70% speed)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Golem|Phase3", meta=(ClampMin="0.0", Units="cm"))
	float ChaseTriggered_StartDistance = HorrorGolemDefaults::ChaseTriggeredStartDistanceCm;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Golem|Phase3", meta=(ClampMin="0.0", ClampMax="1.0"))
	float ChaseTriggered_SpeedMultiplier = HorrorGolemDefaults::ChaseTriggeredSpeedMultiplier;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Golem|Phase3", meta=(ClampMin="0.0", Units="cm/s"))
	float ChaseTriggered_BaseSpeed = HorrorGolemDefaults::ChaseTriggeredBaseSpeedCmPerSecond;

	// Phase 4: Full Chase (10-25m, 100% speed + destruction)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Golem|Phase4", meta=(ClampMin="0.0", Units="cm"))
	float FullChase_MinDistance = HorrorGolemDefaults::FullChaseMinDistanceCm;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Golem|Phase4", meta=(ClampMin="0.0", Units="cm"))
	float FullChase_MaxDistance = HorrorGolemDefaults::FullChaseMaxDistanceCm;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Golem|Phase4", meta=(ClampMin="0.0", Units="cm/s"))
	float FullChase_Speed = HorrorGolemDefaults::FullChaseSpeedCmPerSecond;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Golem|Phase4", meta=(ClampMin="0.0", Units="cm"))
	float ChaseLostTargetDistance = 4000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Golem|Phase4", meta=(ClampMin="0.0", Units="s"))
	float ChaseLostTargetGraceTime = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Golem|Phase4")
	bool bFullChase_EnableDestruction = true;

	// Phase 5: Final Impact (5m, attack)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Golem|Phase5", meta=(ClampMin="0.0", Units="cm"))
	float FinalImpact_TriggerDistance = HorrorGolemDefaults::FinalImpactTriggerDistanceCm;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Golem|Phase5")
	bool bFinalImpact_TriggerCutscene = true;

	// Debug
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Golem|Debug")
	bool bDrawDebugInfo = false;

protected:
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintImplementableEvent, Category="Horror|Golem")
	void BP_OnStateChanged(EGolemEncounterState OldState, EGolemEncounterState NewState);

	UFUNCTION(BlueprintImplementableEvent, Category="Horror|Golem")
	void BP_OnDistantSightingUpdate(float DeltaTime);

	UFUNCTION(BlueprintImplementableEvent, Category="Horror|Golem")
	void BP_OnCloseStalking_PatrolUpdate(float DeltaTime);

	UFUNCTION(BlueprintImplementableEvent, Category="Horror|Golem")
	void BP_OnChaseUpdate(float DeltaTime, float CurrentSpeed);

	UFUNCTION(BlueprintImplementableEvent, Category="Horror|Golem")
	void BP_OnFinalImpactTriggered();

	UFUNCTION(BlueprintImplementableEvent, Category="Horror|Golem")
	void BP_OnEnvironmentDestruction(const FVector& DestructionLocation);

private:
	UPROPERTY()
	TWeakObjectPtr<AActor> TargetActor;

	UPROPERTY()
	TWeakObjectPtr<AHorrorThreatCharacter> OwnerThreat;

	EGolemEncounterState CurrentState = EGolemEncounterState::Dormant;
	bool bBehaviorActive = false;
	bool bFinalImpactFailureRequested = false;

	float StateTimer = 0.0f;
	float LostTargetTimer = 0.0f;
	int32 PatrolWaypointIndex = 0;
	FVector LastPatrolLocation = FVector::ZeroVector;
	bool bPatrolPaused = false;
	float PatrolPauseTimer = 0.0f;

	void UpdateStateMachine(float DeltaTime);
	void UpdateDistantSightingState(float DistanceToTarget, float DeltaTime);
	void UpdateCloseStalkingState(float DistanceToTarget, float DeltaTime);
	void UpdateChaseTriggeredState(float DistanceToTarget, float DeltaTime);
	void UpdateFullChaseState(float DistanceToTarget, float DeltaTime);
	void TransitionToState(EGolemEncounterState NewState);

	void UpdateDistantSighting(float DeltaTime);
	void UpdateCloseStalking(float DeltaTime);
	void UpdateChaseTriggered(float DeltaTime);
	void UpdateFullChase(float DeltaTime);
	void UpdateFinalImpact(float DeltaTime);

	bool HandleLostTarget(float DistanceToTarget, float DeltaTime);
	bool HandleInvalidTarget();
	bool MoveToNavigableLocation(const FVector& Destination, float Speed, float AcceptanceRadius, float DeltaTime);
	bool MoveDirectlyTowardLocation(const FVector& Destination, float Speed, float DeltaTime);
	void MoveTowardsTarget(float Speed, float DeltaTime);
	void PatrolAroundTarget(float DeltaTime);
	void StopNavigationMove();
	void CheckEnvironmentDestruction();
	void TriggerFinalImpactFailure();

	void DrawDebugState();
	FColor GetDebugStateColor() const;
	void DrawDebugStateMarker(const UWorld* World, const FVector& GolemLocation, FColor StateColor) const;
	void DrawDebugTargetInfo(const UWorld* World, const FVector& GolemLocation, FColor StateColor) const;
	void DrawDebugDistanceThresholds(const UWorld* World, const FVector& GolemLocation) const;
};
