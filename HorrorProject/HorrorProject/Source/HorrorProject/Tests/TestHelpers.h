// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"

/**
 * Test helper utilities for HorrorProject tests
 */
namespace HorrorTestHelpers
{
	/**
	 * Creates a transient game world for testing
	 */
	inline UWorld* CreateTestWorld()
	{
		return UWorld::CreateWorld(EWorldType::Game, false);
	}

	/**
	 * Destroys a test world
	 */
	inline void DestroyTestWorld(UWorld* World)
	{
		if (World)
		{
			World->DestroyWorld(false);
		}
	}

	/**
	 * Spawns a test actor in the world
	 */
	template<typename T>
	inline T* SpawnTestActor(UWorld* World, FVector Location = FVector::ZeroVector, FRotator Rotation = FRotator::ZeroRotator)
	{
		if (!World)
		{
			return nullptr;
		}

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		return World->SpawnActor<T>(Location, Rotation, SpawnParams);
	}

	/**
	 * Creates a component and registers it
	 */
	template<typename T>
	inline T* CreateAndRegisterComponent(AActor* Owner)
	{
		if (!Owner)
		{
			return nullptr;
		}

		T* Component = NewObject<T>(Owner);
		if (Component)
		{
			Component->RegisterComponent();
		}
		return Component;
	}

	/**
	 * Simulates time passing by ticking a component
	 */
	inline void TickComponent(UActorComponent* Component, float DeltaTime)
	{
		if (Component)
		{
			Component->TickComponent(DeltaTime, ELevelTick::LEVELTICK_All, nullptr);
		}
	}

	/**
	 * Checks if two floats are nearly equal with tolerance
	 */
	inline bool IsNearlyEqual(float A, float B, float Tolerance = 0.01f)
	{
		return FMath::IsNearlyEqual(A, B, Tolerance);
	}

	/**
	 * Checks if a value is within a range
	 */
	inline bool IsInRange(float Value, float Min, float Max)
	{
		return Value >= Min && Value <= Max;
	}

	/**
	 * Creates a test gameplay tag
	 */
	inline FGameplayTag CreateTestTag(const FString& TagName)
	{
		return FGameplayTag::RequestGameplayTag(FName(*TagName));
	}

	/**
	 * Simulates multiple ticks
	 */
	inline void SimulateTicks(UActorComponent* Component, int32 NumTicks, float DeltaTime = 0.016f)
	{
		for (int32 i = 0; i < NumTicks; ++i)
		{
			TickComponent(Component, DeltaTime);
		}
	}

	/**
	 * Waits for a condition to be true (simulated via ticks)
	 */
	template<typename Predicate>
	inline bool WaitForCondition(UActorComponent* Component, Predicate Pred, int32 MaxTicks = 100, float DeltaTime = 0.016f)
	{
		for (int32 i = 0; i < MaxTicks; ++i)
		{
			if (Pred())
			{
				return true;
			}
			TickComponent(Component, DeltaTime);
		}
		return false;
	}

	/**
	 * Test world wrapper with RAII cleanup
	 */
	class FTestWorldScope
	{
	public:
		FTestWorldScope()
		{
			World = CreateTestWorld();
		}

		~FTestWorldScope()
		{
			DestroyTestWorld(World);
		}

		UWorld* GetWorld() const { return World; }
		bool IsValid() const { return World != nullptr; }

		template<typename T>
		T* SpawnActor(FVector Location = FVector::ZeroVector, FRotator Rotation = FRotator::ZeroRotator)
		{
			return SpawnTestActor<T>(World, Location, Rotation);
		}

	private:
		UWorld* World = nullptr;
	};
}
