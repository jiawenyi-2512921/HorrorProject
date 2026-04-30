// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HorrorCampaignBossActor.generated.h"

class UPointLightComponent;
class UNiagaraComponent;
class USceneComponent;
class USkeletalMeshComponent;
class UTextRenderComponent;

UCLASS(BlueprintType, Blueprintable, ClassGroup=(Horror))
class HORRORPROJECT_API AHorrorCampaignBossActor : public AActor
{
	GENERATED_BODY()

public:
	AHorrorCampaignBossActor();

	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable, Category="Horror|Campaign")
	void ConfigureBoss(FName InChapterId, FText InBossName, int32 InRequiredWeakPointCount);

	UFUNCTION(BlueprintCallable, Category="Horror|Campaign")
	void SetBossDefeated(bool bInDefeated);

	UFUNCTION(BlueprintCallable, Category="Horror|Campaign")
	void SetBossAwake(bool bInAwake);

	UFUNCTION(BlueprintCallable, Category="Horror|Campaign")
	void ConfigureChasePressure(float InMoveSpeed, float InEngageRadius, float InAttackRadius, float InFearPressureRadius, float InActorScale);

	UFUNCTION(BlueprintCallable, Category="Horror|Campaign")
	bool RegisterWeakPointResolved();

	UFUNCTION(BlueprintPure, Category="Horror|Campaign")
	bool IsBossDefeated() const { return bDefeated; }

	UFUNCTION(BlueprintPure, Category="Horror|Campaign")
	bool IsBossAwake() const { return bAwake && !bDefeated; }

	UFUNCTION(BlueprintPure, Category="Horror|Campaign")
	int32 GetRequiredWeakPointCount() const { return RequiredWeakPointCount; }

	UFUNCTION(BlueprintPure, Category="Horror|Campaign")
	int32 GetResolvedWeakPointCount() const { return ResolvedWeakPointCount; }

	UFUNCTION(BlueprintCallable, Category="Horror|Campaign")
	bool ApplyBossPressureToActor(AActor* TargetActor, float DeltaTime);

	UFUNCTION(BlueprintCallable, Category="Horror|Campaign")
	bool MoveBossTowardActor(AActor* TargetActor, float DeltaTime);

	UFUNCTION(BlueprintPure, Category="Horror|Campaign")
	bool CanBossAttackActor(AActor* TargetActor) const;

	UFUNCTION(BlueprintCallable, Category="Horror|Campaign")
	bool TryTriggerBossAttack(AActor* TargetActor);

	UFUNCTION(BlueprintPure, Category="Horror|Campaign")
	float CalculateFearPressureAmount(float DistanceCm, float DeltaTime) const;

protected:
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintImplementableEvent, Category="Horror|Campaign")
	void BP_OnBossAttack(AActor* TargetActor);

	UFUNCTION(BlueprintImplementableEvent, Category="Horror|Campaign")
	void BP_OnBossAwake();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Horror|Campaign")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Horror|Campaign")
	TObjectPtr<USkeletalMeshComponent> BossMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Horror|Campaign")
	TObjectPtr<UTextRenderComponent> BossLabel;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Horror|Campaign")
	TObjectPtr<UPointLightComponent> BossLight;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Horror|Campaign")
	TObjectPtr<UNiagaraComponent> BossVFX;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Horror|Campaign")
	FName ChapterId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Horror|Campaign")
	FText BossName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Horror|Campaign")
	int32 RequiredWeakPointCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Horror|Campaign|Pressure", meta=(ClampMin="0.0", Units="cm"))
	float FearPressureRadius = 2400.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Horror|Campaign|Pressure", meta=(ClampMin="0.0"))
	float FearPressurePerSecond = 9.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Horror|Campaign|Pressure", meta=(ClampMin="0.0"))
	float WeakPointFearPressureBonus = 0.35f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Horror|Campaign|Pressure")
	bool bEnableFearPressure = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Horror|Campaign|Movement")
	bool bEnableBossMovement = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Horror|Campaign|Movement", meta=(ClampMin="0.0", Units="cm"))
	float BossEngageRadius = 3200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Horror|Campaign|Movement", meta=(ClampMin="0.0", Units="cm/s"))
	float BossMoveSpeed = 260.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Horror|Campaign|Attack", meta=(ClampMin="0.0", Units="cm"))
	float BossAttackRadius = 260.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Horror|Campaign|Attack", meta=(ClampMin="0.0", Units="s"))
	float BossAttackCooldownSeconds = 2.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Horror|Campaign|Attack")
	bool bBossAttackRequestsPlayerFailure = true;

private:
	void ApplyBossVisuals();
	void ApplyBossPressureToPlayerPawns(float DeltaTime);
	void UpdateBossMovementAndAttacks(float DeltaTime);
	float GetWeakPointPressureMultiplier() const;

	UPROPERTY(Transient)
	bool bDefeated = false;

	UPROPERTY(Transient)
	bool bAwake = false;

	UPROPERTY(Transient)
	int32 ResolvedWeakPointCount = 0;

	UPROPERTY(Transient)
	float LastBossAttackWorldSeconds = -100000.0f;
};
