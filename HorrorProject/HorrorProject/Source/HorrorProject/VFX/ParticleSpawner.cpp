// Copyright Epic Games, Inc. All Rights Reserved.

#include "ParticleSpawner.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "TimerManager.h"

UParticleSpawner::UParticleSpawner()
{
	// Performance optimization: Use timer-based cleanup instead of tick
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.TickGroup = TG_PostUpdateWork;

	// Memory optimization: Pre-allocate particle systems map
	ParticleSystems.Reserve(8);
}

void UParticleSpawner::BeginPlay()
{
	Super::BeginPlay();

	// Performance optimization: Reserve capacity for active effects
	ActiveEffects.Reserve(MaxActiveParticles / 10);

	// Use timer for periodic cleanup instead of tick
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(CleanupTimerHandle, this, &UParticleSpawner::CleanupFinishedEffects, 1.0f, true);
	}
}

void UParticleSpawner::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(CleanupTimerHandle);
	}

	Super::EndPlay(EndPlayReason);
}

void UParticleSpawner::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	// Performance optimization: Tick disabled, using timer-based cleanup
}

UNiagaraComponent* UParticleSpawner::SpawnEffect(EParticleEffectType EffectType, const FParticleSpawnSettings& Settings)
{
	if (!CanSpawnNewEffect())
	{
		return nullptr;
	}

	UNiagaraSystem* ParticleSystem = ResolveParticleSystem(EffectType);
	if (!ParticleSystem)
	{
		return nullptr;
	}

	UNiagaraComponent* NewEffect = Settings.bAttachToParent
		? SpawnAttachedToOwner(ParticleSystem, Settings)
		: SpawnAtWorldLocation(ParticleSystem, Settings);

	TrackSpawnedEffect(NewEffect);

	return NewEffect;
}

UNiagaraComponent* UParticleSpawner::SpawnEffectAtLocation(EParticleEffectType EffectType, FVector Location, FRotator Rotation)
{
	FParticleSpawnSettings Settings;
	Settings.Location = Location;
	Settings.Rotation = Rotation;
	Settings.bAutoDestroy = true;

	return SpawnEffect(EffectType, Settings);
}

UNiagaraComponent* UParticleSpawner::SpawnAttachedEffect(EParticleEffectType EffectType, USceneComponent* AttachToComponent, FName SocketName)
{
	if (!CanSpawnNewEffect() || !AttachToComponent)
	{
		return nullptr;
	}

	TObjectPtr<UNiagaraSystem>* ParticleSystemPtr = ParticleSystems.Find(EffectType);
	if (!ParticleSystemPtr || !*ParticleSystemPtr)
	{
		return nullptr;
	}

	UNiagaraComponent* NewEffect = UNiagaraFunctionLibrary::SpawnSystemAttached(
		ParticleSystemPtr->Get(),
		AttachToComponent,
		SocketName,
		FVector::ZeroVector,
		FRotator::ZeroRotator,
		EAttachLocation::KeepRelativeOffset,
		true
	);

	if (NewEffect)
	{
		ActiveEffects.Add(NewEffect);
		ApplyLODSettings(NewEffect);
	}

	return NewEffect;
}

void UParticleSpawner::StopEffect(UNiagaraComponent* Effect, bool bImmediate)
{
	if (Effect)
	{
		if (bImmediate)
		{
			Effect->DeactivateImmediate();
		}
		else
		{
			Effect->Deactivate();
		}
	}
}

void UParticleSpawner::StopAllEffects(bool bImmediate)
{
	for (UNiagaraComponent* Effect : ActiveEffects)
	{
		if (Effect)
		{
			StopEffect(Effect, bImmediate);
		}
	}

	if (bImmediate)
	{
		ActiveEffects.Empty();
	}
}

void UParticleSpawner::SetEffectParameter(UNiagaraComponent* Effect, FName ParameterName, float Value)
{
	if (Effect)
	{
		Effect->SetFloatParameter(ParameterName, Value);
	}
}

void UParticleSpawner::SetEffectColor(UNiagaraComponent* Effect, FLinearColor Color)
{
	if (Effect)
	{
		Effect->SetColorParameter(FName("Color"), Color);
	}
}

void UParticleSpawner::UpdateParticleBudget(int32 MaxParticles)
{
	MaxActiveParticles = FMath::Max(0, MaxParticles);

	// Stop excess effects if over budget
	while (GetActiveParticleCount() > MaxActiveParticles && ActiveEffects.Num() > 0)
	{
		UNiagaraComponent* OldestEffect = ActiveEffects.IsEmpty() ? nullptr : ActiveEffects.GetData()[0];
		if (OldestEffect)
		{
			StopEffect(OldestEffect, true);
		}
		ActiveEffects.RemoveAt(0);
	}
}

int32 UParticleSpawner::GetActiveParticleCount() const
{
	// Performance optimization: Use simple count instead of expensive iteration
	return ActiveEffects.Num();
}

void UParticleSpawner::CleanupFinishedEffects()
{
	ActiveEffects.RemoveAll([](UNiagaraComponent* Effect)
	{
		return !Effect || !Effect->IsActive();
	});
}

void UParticleSpawner::ApplyLODSettings(UNiagaraComponent* Effect)
{
	if (!Effect)
	{
		return;
	}

	if (bEnableParticleLOD)
	{
		// Apply LOD distance
		Effect->SetMaxSimTime(10.0f);
	}

	if (bEnableParticleCulling)
	{
		Effect->SetAllowScalability(true);
	}
	else
	{
		Effect->SetAllowScalability(false);
	}
}

bool UParticleSpawner::CanSpawnNewEffect() const
{
	return GetActiveParticleCount() < MaxActiveParticles;
}

UNiagaraSystem* UParticleSpawner::ResolveParticleSystem(EParticleEffectType EffectType) const
{
	const TObjectPtr<UNiagaraSystem>* ParticleSystemPtr = ParticleSystems.Find(EffectType);
	return ParticleSystemPtr && *ParticleSystemPtr ? ParticleSystemPtr->Get() : nullptr;
}

UNiagaraComponent* UParticleSpawner::SpawnAttachedToOwner(UNiagaraSystem* ParticleSystem, const FParticleSpawnSettings& Settings)
{
	AActor* Owner = GetOwner();
	USceneComponent* RootComponent = Owner ? Owner->GetRootComponent() : nullptr;
	if (!RootComponent)
	{
		return nullptr;
	}

	UNiagaraComponent* NewEffect = UNiagaraFunctionLibrary::SpawnSystemAttached(
		ParticleSystem,
		RootComponent,
		NAME_None,
		Settings.Location,
		Settings.Rotation,
		EAttachLocation::KeepRelativeOffset,
		Settings.bAutoDestroy
	);

	if (NewEffect)
	{
		NewEffect->SetRelativeScale3D(Settings.Scale);
	}

	return NewEffect;
}

UNiagaraComponent* UParticleSpawner::SpawnAtWorldLocation(UNiagaraSystem* ParticleSystem, const FParticleSpawnSettings& Settings)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Warning, TEXT("ParticleSpawner: World is null in SpawnEffect"));
		return nullptr;
	}

	return UNiagaraFunctionLibrary::SpawnSystemAtLocation(
		World,
		ParticleSystem,
		Settings.Location,
		Settings.Rotation,
		Settings.Scale,
		Settings.bAutoDestroy
	);
}

void UParticleSpawner::TrackSpawnedEffect(UNiagaraComponent* Effect)
{
	if (!Effect)
	{
		return;
	}

	ActiveEffects.Add(Effect);
	ApplyLODSettings(Effect);
}
