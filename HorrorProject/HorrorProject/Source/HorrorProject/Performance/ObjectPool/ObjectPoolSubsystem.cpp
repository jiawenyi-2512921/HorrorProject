// Copyright Epic Games, Inc. All Rights Reserved.

#include "ObjectPoolSubsystem.h"
#include "Components/AudioComponent.h"
#include "NiagaraComponent.h"
#include "Engine/World.h"

void UObjectPoolSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	CleanupInterval = DefaultCleanupInterval;
	LastCleanupTime = 0.0f;

	// Pre-allocate pools for common types
	ObjectPools.Reserve(8);
	MaxPoolSizes.Reserve(8);

	// Set default max sizes
	MaxPoolSizes.Add(UAudioComponent::StaticClass(), MaxAudioComponents);
	MaxPoolSizes.Add(UNiagaraComponent::StaticClass(), MaxNiagaraComponents);
}

void UObjectPoolSubsystem::Deinitialize()
{
	ClearAllPools();
	Super::Deinitialize();
}

void UObjectPoolSubsystem::Tick(float DeltaTime)
{
	float CurrentTime = GetWorld()->GetTimeSeconds();

	if (CurrentTime - LastCleanupTime >= CleanupInterval)
	{
		CleanupPools();
		LastCleanupTime = CurrentTime;
	}
}

TStatId UObjectPoolSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UObjectPoolSubsystem, STATGROUP_Tickables);
}

UAudioComponent* UObjectPoolSubsystem::AcquireAudioComponent(AActor* Owner)
{
	return AcquireObject<UAudioComponent>(Owner);
}

void UObjectPoolSubsystem::ReleaseAudioComponent(UAudioComponent* Component)
{
	if (Component)
	{
		Component->Stop();
		Component->SetSound(nullptr);
		ReleaseObject(Component);
	}
}

UNiagaraComponent* UObjectPoolSubsystem::AcquireNiagaraComponent(AActor* Owner)
{
	return AcquireObject<UNiagaraComponent>(Owner);
}

void UObjectPoolSubsystem::ReleaseNiagaraComponent(UNiagaraComponent* Component)
{
	if (Component)
	{
		Component->Deactivate();
		Component->SetAsset(nullptr);
		ReleaseObject(Component);
	}
}

int32 UObjectPoolSubsystem::GetPoolSize(UClass* ObjectClass) const
{
	const TArray<FPooledObjectEntry>* Pool = ObjectPools.Find(ObjectClass);
	return Pool ? Pool->Num() : 0;
}

int32 UObjectPoolSubsystem::GetActiveObjectCount(UClass* ObjectClass) const
{
	const TArray<FPooledObjectEntry>* Pool = ObjectPools.Find(ObjectClass);
	if (!Pool)
	{
		return 0;
	}

	int32 Count = 0;
	for (const FPooledObjectEntry& Entry : *Pool)
	{
		if (Entry.bInUse)
		{
			Count++;
		}
	}
	return Count;
}

void UObjectPoolSubsystem::ClearPool(UClass* ObjectClass)
{
	TArray<FPooledObjectEntry>* Pool = ObjectPools.Find(ObjectClass);
	if (Pool)
	{
		Pool->Empty();
	}
}

void UObjectPoolSubsystem::ClearAllPools()
{
	ObjectPools.Empty();
}

void UObjectPoolSubsystem::SetMaxPoolSize(UClass* ObjectClass, int32 MaxSize)
{
	MaxPoolSizes.Add(ObjectClass, MaxSize);
}

void UObjectPoolSubsystem::SetPoolCleanupInterval(float Interval)
{
	CleanupInterval = FMath::Max(1.0f, Interval);
}

void UObjectPoolSubsystem::CleanupPools()
{
	float CurrentTime = GetWorld()->GetTimeSeconds();

	for (auto& PoolPair : ObjectPools)
	{
		TArray<FPooledObjectEntry>& Pool = PoolPair.Value;

		// Remove unused objects that have timed out
		Pool.RemoveAll([CurrentTime, this](const FPooledObjectEntry& Entry)
		{
			return !Entry.bInUse && (CurrentTime - Entry.LastUsedTime) > ObjectTimeoutSeconds;
		});
	}
}

void UObjectPoolSubsystem::CleanupPool(UClass* ObjectClass)
{
	TArray<FPooledObjectEntry>* Pool = ObjectPools.Find(ObjectClass);
	if (!Pool)
	{
		return;
	}

	float CurrentTime = GetWorld()->GetTimeSeconds();

	Pool->RemoveAll([CurrentTime, this](const FPooledObjectEntry& Entry)
	{
		return !Entry.bInUse && (CurrentTime - Entry.LastUsedTime) > ObjectTimeoutSeconds;
	});
}

FPooledObjectEntry* UObjectPoolSubsystem::FindAvailableEntry(UClass* ObjectClass)
{
	TArray<FPooledObjectEntry>* Pool = ObjectPools.Find(ObjectClass);
	if (!Pool)
	{
		return nullptr;
	}

	for (FPooledObjectEntry& Entry : *Pool)
	{
		if (!Entry.bInUse && Entry.Object)
		{
			return &Entry;
		}
	}

	return nullptr;
}

void UObjectPoolSubsystem::ResetPooledObject(UObject* Object)
{
	if (!Object)
	{
		return;
	}

	// Reset common component types
	if (UAudioComponent* AudioComp = Cast<UAudioComponent>(Object))
	{
		AudioComp->Stop();
		AudioComp->SetSound(nullptr);
		AudioComp->SetVolumeMultiplier(1.0f);
		AudioComp->SetPitchMultiplier(1.0f);
	}
	else if (UNiagaraComponent* NiagaraComp = Cast<UNiagaraComponent>(Object))
	{
		NiagaraComp->Deactivate();
		NiagaraComp->SetAsset(nullptr);
	}
}
