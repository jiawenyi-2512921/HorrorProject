// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "ParticleSpawner.generated.h"

UENUM(BlueprintType)
enum class EParticleEffectType : uint8
{
	UnderwaterDebris UMETA(DisplayName = "水下碎屑"),
	DustParticles UMETA(DisplayName = "尘埃粒子"),
	BloodSplatter UMETA(DisplayName = "血迹飞溅"),
	WaterDrips UMETA(DisplayName = "滴水"),
	SteamVents UMETA(DisplayName = "蒸汽喷口"),
	EmergencyLights UMETA(DisplayName = "应急灯光"),
	BubbleTrail UMETA(DisplayName = "气泡轨迹"),
	SparkShower UMETA(DisplayName = "火花喷溅")
};

USTRUCT(BlueprintType)
struct FParticleSpawnSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Location = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FRotator Rotation = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Scale = FVector::OneVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Lifetime = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAutoDestroy = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAttachToParent = false;
};

/**
 * Defines Particle Spawner behavior for the VFX module.
 */
UCLASS(ClassGroup=(VFX), meta=(BlueprintSpawnableComponent))
class HORRORPROJECT_API UParticleSpawner : public UActorComponent
{
	GENERATED_BODY()

public:
	static constexpr float DefaultParticleLODDistance = 2000.0f;

	UParticleSpawner();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	// Spawn particle effect
	UFUNCTION(BlueprintCallable, Category = "VFX|Particles")
	UNiagaraComponent* SpawnEffect(EParticleEffectType EffectType, const FParticleSpawnSettings& Settings);

	// Spawn effect at location
	UFUNCTION(BlueprintCallable, Category = "VFX|Particles")
	UNiagaraComponent* SpawnEffectAtLocation(EParticleEffectType EffectType, FVector Location, FRotator Rotation = FRotator::ZeroRotator);

	// Spawn attached effect
	UFUNCTION(BlueprintCallable, Category = "VFX|Particles")
	UNiagaraComponent* SpawnAttachedEffect(EParticleEffectType EffectType, USceneComponent* AttachToComponent, FName SocketName = NAME_None);

	// Stop effect
	UFUNCTION(BlueprintCallable, Category = "VFX|Particles")
	void StopEffect(UNiagaraComponent* Effect, bool bImmediate = false);

	// Stop all effects
	UFUNCTION(BlueprintCallable, Category = "VFX|Particles")
	void StopAllEffects(bool bImmediate = false);

	// Set effect parameter
	UFUNCTION(BlueprintCallable, Category = "VFX|Particles")
	void SetEffectParameter(UNiagaraComponent* Effect, FName ParameterName, float Value);

	// Set effect color
	UFUNCTION(BlueprintCallable, Category = "VFX|Particles")
	void SetEffectColor(UNiagaraComponent* Effect, FLinearColor Color);

	// Update particle budget
	UFUNCTION(BlueprintCallable, Category = "VFX|Particles")
	void UpdateParticleBudget(int32 MaxParticles);

	// Get active particle count
	UFUNCTION(BlueprintPure, Category = "VFX|Particles")
	int32 GetActiveParticleCount() const;

protected:
	// Niagara system references
	UPROPERTY(EditAnywhere, Category = "VFX|Particles")
	TMap<EParticleEffectType, TObjectPtr<UNiagaraSystem>> ParticleSystems;

	// Active effects
	UPROPERTY()
	TArray<TObjectPtr<UNiagaraComponent>> ActiveEffects;

	// Performance settings
	UPROPERTY(EditAnywhere, Category = "VFX|Performance")
	int32 MaxActiveParticles = 1000;

	UPROPERTY(EditAnywhere, Category = "VFX|Performance")
	float ParticleLODDistance = DefaultParticleLODDistance;

	UPROPERTY(EditAnywhere, Category = "VFX|Performance")
	bool bEnableParticleLOD = true;

	UPROPERTY(EditAnywhere, Category = "VFX|Performance")
	bool bEnableParticleCulling = true;

private:
	UNiagaraSystem* ResolveParticleSystem(EParticleEffectType EffectType) const;
	UNiagaraComponent* SpawnAttachedToOwner(UNiagaraSystem* ParticleSystem, const FParticleSpawnSettings& Settings);
	UNiagaraComponent* SpawnAtWorldLocation(UNiagaraSystem* ParticleSystem, const FParticleSpawnSettings& Settings);
	void TrackSpawnedEffect(UNiagaraComponent* Effect);
	void CleanupFinishedEffects();
	void ApplyLODSettings(UNiagaraComponent* Effect);
	bool CanSpawnNewEffect() const;

	int32 CurrentParticleCount = 0;
	FTimerHandle CleanupTimerHandle;
};
