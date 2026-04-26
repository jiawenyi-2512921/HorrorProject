// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HorrorGolemBehaviorComponent.generated.h"

class AHorrorThreatCharacter;
class AActor;

UENUM(BlueprintType)
enum class EGolemEncounterState : uint8
{
	Dormant UMETA(DisplayName="Dormant"),
	DistantSighting UMETA(DisplayName="Distant Sighting (30m+)"),
	CloseStalking UMETA(DisplayName="Close Stalking (10-15m)"),
	ChaseTriggered UMETA(DisplayName="Chase Triggered (20m, 70% speed)"),
	FullChase UMETA(DisplayName="Full Chase (10-25m, 100% speed)"),
	FinalImpact UMETA(DisplayName="Final Impact (5m, attack)")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FGolemStateChangedSignature, EGolemEncounterState, OldState, EGolemEncounterState, NewState);

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
	float DistantSightingMinDistance = 3000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Golem|Phase1", meta=(ClampMin="0.0", Units="cm/s"))
	float DistantSightingMoveSpeed = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Golem|Phase1")
	bool bDistantSightingStationary = true;

	// Phase 2: Close Stalking (10-15m)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Golem|Phase2", meta=(ClampMin="0.0", Units="cm"))
	float CloseStalking_MinDistance = 1000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Golem|Phase2", meta=(ClampMin="0.0", Units="cm"))
	float CloseStalking_MaxDistance = 1500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Golem|Phase2", meta=(ClampMin="0.0", Units="cm/s"))
	float CloseStalking_PatrolSpeed = 150.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Golem|Phase2", meta=(ClampMin="0.0", Units="s"))
	float CloseStalking_PatrolPauseTime = 2.0f;

	// Phase 3: Chase Triggered (20m start, 70% speed)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Golem|Phase3", meta=(ClampMin="0.0", Units="cm"))
	float ChaseTriggered_StartDistance = 2000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Golem|Phase3", meta=(ClampMin="0.0", ClampMax="1.0"))
	float ChaseTriggered_SpeedMultiplier = 0.7f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Golem|Phase3", meta=(ClampMin="0.0", Units="cm/s"))
	float ChaseTriggered_BaseSpeed = 400.0f;

	// Phase 4: Full Chase (10-25m, 100% speed + destruction)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Golem|Phase4", meta=(ClampMin="0.0", Units="cm"))
	float FullChase_MinDistance = 1000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Golem|Phase4", meta=(ClampMin="0.0", Units="cm"))
	float FullChase_MaxDistance = 2500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Golem|Phase4", meta=(ClampMin="0.0", Units="cm/s"))
	float FullChase_Speed = 600.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Golem|Phase4")
	bool bFullChase_EnableDestruction = true;

	// Phase 5: Final Impact (5m, attack)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Golem|Phase5", meta=(ClampMin="0.0", Units="cm"))
	float FinalImpact_TriggerDistance = 500.0f;

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

	float StateTimer = 0.0f;
	int32 PatrolWaypointIndex = 0;
	FVector LastPatrolLocation = FVector::ZeroVector;
	bool bPatrolPaused = false;
	float PatrolPauseTimer = 0.0f;

	void UpdateStateMachine(float DeltaTime);
	void TransitionToState(EGolemEncounterState NewState);

	void UpdateDistantSighting(float DeltaTime);
	void UpdateCloseStalking(float DeltaTime);
	void UpdateChaseTriggered(float DeltaTime);
	void UpdateFullChase(float DeltaTime);
	void UpdateFinalImpact(float DeltaTime);

	void MoveTowardsTarget(float Speed, float DeltaTime);
	void PatrolAroundTarget(float DeltaTime);
	void CheckEnvironmentDestruction();

	void DrawDebugState();
};
