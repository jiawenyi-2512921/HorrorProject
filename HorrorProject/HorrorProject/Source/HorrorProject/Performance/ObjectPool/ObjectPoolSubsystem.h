// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
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

/**
 * Object Pool Subsystem
 * Manages pooled objects to reduce allocation/deallocation overhead
 */
UCLASS()
class HORRORPROJECT_API UObjectPoolSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;

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
	int32 DefaultMaxPoolSize = 64;

	UPROPERTY(EditDefaultsOnly, Category = "Performance|ObjectPool")
	float DefaultCleanupInterval = 30.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Performance|ObjectPool")
	float ObjectTimeoutSeconds = 60.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Performance|ObjectPool")
	int32 MaxAudioComponents = 32;

	UPROPERTY(EditDefaultsOnly, Category = "Performance|ObjectPool")
	int32 MaxNiagaraComponents = 48;

private:
	UPROPERTY(Transient)
	TMap<UClass*, TArray<FPooledObjectEntry>> ObjectPools;

	UPROPERTY(Transient)
	TMap<UClass*, int32> MaxPoolSizes;

	float LastCleanupTime = 0.0f;
	float CleanupInterval = 30.0f;

	void CleanupPools();
	void CleanupPool(UClass* ObjectClass);
	FPooledObjectEntry* FindAvailableEntry(UClass* ObjectClass);
	void ResetPooledObject(UObject* Object);
};

template<typename T>
T* UObjectPoolSubsystem::AcquireObject(UObject* Outer)
{
	UClass* ObjectClass = T::StaticClass();

	// Try to find available object in pool
	FPooledObjectEntry* Entry = FindAvailableEntry(ObjectClass);

	if (Entry && Entry->Object)
	{
		Entry->bInUse = true;
		Entry->LastUsedTime = GetWorld()->GetTimeSeconds();
		Entry->UseCount++;
		ResetPooledObject(Entry->Object);
		return Cast<T>(Entry->Object);
	}

	// Create new object if pool is empty or below max size
	TArray<FPooledObjectEntry>& Pool = ObjectPools.FindOrAdd(ObjectClass);
	int32 MaxSize = MaxPoolSizes.Contains(ObjectClass) ? MaxPoolSizes[ObjectClass] : DefaultMaxPoolSize;

	if (Pool.Num() < MaxSize)
	{
		T* NewObject = NewObject<T>(Outer ? Outer : this);

		FPooledObjectEntry NewEntry;
		NewEntry.Object = NewObject;
		NewEntry.bInUse = true;
		NewEntry.LastUsedTime = GetWorld()->GetTimeSeconds();
		NewEntry.UseCount = 1;

		Pool.Add(NewEntry);
		return NewObject;
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
	TArray<FPooledObjectEntry>* Pool = ObjectPools.Find(ObjectClass);

	if (!Pool)
	{
		return;
	}

	for (FPooledObjectEntry& Entry : *Pool)
	{
		if (Entry.Object == Object)
		{
			Entry.bInUse = false;
			Entry.LastUsedTime = GetWorld()->GetTimeSeconds();
			ResetPooledObject(Object);
			break;
		}
	}
}
