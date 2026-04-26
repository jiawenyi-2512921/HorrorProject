// Copyright Epic Games, Inc. All Rights Reserved.

#include "ParticleSpawner.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"

UParticleSpawner::UParticleSpawner()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PostUpdateWork;
}

void UParticleSpawner::BeginPlay()
{
	Super::BeginPlay();
}

void UParticleSpawner::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	CleanupFinishedEffects();
}

UNiagaraComponent* UParticleSpawner::SpawnEffect(EParticleEffectType EffectType, const FParticleSpawnSettings& Settings)
{
	if (!CanSpawnNewEffect())
	{
		return nullptr;
	}

	if (!ParticleSystems.Contains(EffectType) || !ParticleSystems[EffectType])
	{
		return nullptr;
	}

	UNiagaraComponent* NewEffect = nullptr;

	if (Settings.bAttachToParent && GetOwner())
	{
		NewEffect = UNiagaraFunctionLibrary::SpawnSystemAttached(
			ParticleSystems[EffectType],
			GetOwner()->GetRootComponent(),
			NAME_None,
			Settings.Location,
			Settings.Rotation,
			Settings.Scale,
			EAttachLocation::KeepRelativeOffset,
			Settings.bAutoDestroy
		);
	}
	else
	{
		NewEffect = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(),
			ParticleSystems[EffectType],
			Settings.Location,
			Settings.Rotation,
			Settings.Scale,
			Settings.bAutoDestroy
		);
	}

	if (NewEffect)
	{
		ActiveEffects.Add(NewEffect);
		ApplyLODSettings(NewEffect);
	}

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

	if (!ParticleSystems.Contains(EffectType) || !ParticleSystems[EffectType])
	{
		return nullptr;
	}

	UNiagaraComponent* NewEffect = UNiagaraFunctionLibrary::SpawnSystemAttached(
		ParticleSystems[EffectType],
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
		if (UNiagaraComponent* OldestEffect = ActiveEffects[0])
		{
			StopEffect(OldestEffect, true);
		}
		ActiveEffects.RemoveAt(0);
	}
}

int32 UParticleSpawner::GetActiveParticleCount() const
{
	int32 TotalCount = 0;

	for (const UNiagaraComponent* Effect : ActiveEffects)
	{
		if (Effect && Effect->IsActive())
		{
			// Estimate particle count (actual count requires system-specific queries)
			TotalCount += 100; // Placeholder
		}
	}

	return TotalCount;
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
		// Enable culling
		Effect->SetCullProxy(nullptr);
	}
}

bool UParticleSpawner::CanSpawnNewEffect() const
{
	return GetActiveParticleCount() < MaxActiveParticles;
}
