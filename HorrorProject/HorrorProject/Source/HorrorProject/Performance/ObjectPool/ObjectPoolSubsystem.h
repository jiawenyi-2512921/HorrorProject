// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Subsystems/WorldSubsystem.h"
#include "ObjectPoolSubsystem.generated.h"

class UAudioComponent;
class UNiagaraComponent;

/**
 * Generic object pool entry
 */
USTRUCT()
struct FPooledObjectEntry
{
	GENERATED_BODY()

	UPROPERTY()
	TObjectPtr<UObject> Object;

	UPROPERTY()
	float LastUsedTime = 0.0f;

	UPROPERTY()
	bool bInUse = false;

	UPROPERTY()
	int32 UseCount = 0;
};

USTRUCT()
struct FPooledObjectEntryArray
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<FPooledObjectEntry> Entries;
};

/**
 * Object Pool Subsystem
 * Manages pooled objects to reduce allocation/deallocation overhead
 */
UCLASS()
class HORRORPROJECT_API UObjectPoolSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	static constexpr int32 DefaultConfiguredMaxPoolSize = 64;
	static constexpr float DefaultConfiguredCleanupInterval = 30.0f;
	static constexpr float DefaultObjectTimeoutSeconds = 60.0f;
	static constexpr int32 DefaultMaxAudioComponents = 32;
	static constexpr int32 DefaultMaxNiagaraComponents = 48;

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// Audio component pooling
	UFUNCTION(BlueprintCallable, Category = "Performance|ObjectPool")
	UAudioComponent* AcquireAudioComponent(AActor* Owner);

	UFUNCTION(BlueprintCallable, Category = "Performance|ObjectPool")
	void ReleaseAudioComponent(UAudioComponent* Component);

	// Niagara component pooling
	UFUNCTION(BlueprintCallable, Category = "Performance|ObjectPool")
	UNiagaraComponent* AcquireNiagaraComponent(AActor* Owner);

	UFUNCTION(BlueprintCallable, Category = "Performance|ObjectPool")
	void ReleaseNiagaraComponent(UNiagaraComponent* Component);

	// Generic object pooling
	template<typename T>
	T* AcquireObject(UObject* Outer);

	template<typename T>
	void ReleaseObject(T* Object);

	// Pool statistics
	UFUNCTION(BlueprintPure, Category = "Performance|ObjectPool")
	int32 GetPoolSize(UClass* ObjectClass) const;

	UFUNCTION(BlueprintPure, Category = "Performance|ObjectPool")
	int32 GetActiveObjectCount(UClass* ObjectClass) const;

	UFUNCTION(BlueprintCallable, Category = "Performance|ObjectPool")
	void ClearPool(UClass* ObjectClass);

	UFUNCTION(BlueprintCallable, Category = "Performance|ObjectPool")
	void ClearAllPools();

	// Configuration
	UFUNCTION(BlueprintCallable, Category = "Performance|ObjectPool")
	void SetMaxPoolSize(UClass* ObjectClass, int32 MaxSize);

	UFUNCTION(BlueprintCallable, Category = "Performance|ObjectPool")
	void SetPoolCleanupInterval(float Interval);

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Performance|ObjectPool")
	int32 DefaultMaxPoolSize = DefaultConfiguredMaxPoolSize;

	UPROPERTY(EditDefaultsOnly, Category = "Performance|ObjectPool")
	float DefaultCleanupInterval = DefaultConfiguredCleanupInterval;

	UPROPERTY(EditDefaultsOnly, Category = "Performance|ObjectPool")
	float ObjectTimeoutSeconds = DefaultObjectTimeoutSeconds;

	UPROPERTY(EditDefaultsOnly, Category = "Performance|ObjectPool")
	int32 MaxAudioComponents = DefaultMaxAudioComponents;

	UPROPERTY(EditDefaultsOnly, Category = "Performance|ObjectPool")
	int32 MaxNiagaraComponents = DefaultMaxNiagaraComponents;

private:
	UPROPERTY(Transient)
	TMap<UClass*, FPooledObjectEntryArray> ObjectPools;

	UPROPERTY(Transient)
	TMap<UClass*, int32> MaxPoolSizes;

	float LastCleanupTime = 0.0f;
	float CleanupInterval = DefaultConfiguredCleanupInterval;
	FTimerHandle CleanupTimerHandle;

	void CleanupPools();
	void CleanupPool(UClass* ObjectClass);
	FPooledObjectEntry* FindAvailableEntry(UClass* ObjectClass);
	void ResetPooledObject(UObject* Object);
};

template<typename T>
T* UObjectPoolSubsystem::AcquireObject(UObject* Outer)
{
	UClass* ObjectClass = T::StaticClass();
	UWorld* World = GetWorld();
	const float CurrentTime = World ? World->GetTimeSeconds() : 0.0f;

	// Try to find available object in pool
	FPooledObjectEntry* Entry = FindAvailableEntry(ObjectClass);

	if (Entry && Entry->Object)
	{
		Entry->bInUse = true;
		Entry->LastUsedTime = CurrentTime;
		Entry->UseCount++;
		ResetPooledObject(Entry->Object);
		return Cast<T>(Entry->Object);
	}

	// Create new object if pool is empty or below max size
	FPooledObjectEntryArray& Pool = ObjectPools.FindOrAdd(ObjectClass);
	const int32* MaxSizeOverride = MaxPoolSizes.Find(ObjectClass);
	int32 MaxSize = MaxSizeOverride ? *MaxSizeOverride : DefaultMaxPoolSize;

	if (Pool.Entries.Num() < MaxSize)
	{
		T* CreatedObject = ::NewObject<T>(Outer ? Outer : this);

		FPooledObjectEntry NewEntry;
		NewEntry.Object = CreatedObject;
		NewEntry.bInUse = true;
		NewEntry.LastUsedTime = CurrentTime;
		NewEntry.UseCount = 1;

		Pool.Entries.Add(NewEntry);
		return CreatedObject;
	}

	return nullptr;
}

template<typename T>
void UObjectPoolSubsystem::ReleaseObject(T* Object)
{
	if (!Object)
	{
		return;
	}

	UClass* ObjectClass = Object->GetClass();
	FPooledObjectEntryArray* Pool = ObjectPools.Find(ObjectClass);

	if (!Pool)
	{
		return;
	}

	for (FPooledObjectEntry& Entry : Pool->Entries)
	{
		if (Entry.Object == Object)
		{
			Entry.bInUse = false;
			UWorld* World = GetWorld();
			Entry.LastUsedTime = World ? World->GetTimeSeconds() : 0.0f;
			ResetPooledObject(Object);
			break;
		}
	}
}
