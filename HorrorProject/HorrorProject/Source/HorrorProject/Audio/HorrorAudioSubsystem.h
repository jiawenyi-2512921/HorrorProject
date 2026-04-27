// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "GameplayTagContainer.h"
#include "Sound/SoundAttenuation.h"
#include "HorrorAudioSubsystem.generated.h"

class USoundBase;
class UAudioComponent;
class USoundConcurrency;

namespace HorrorAudioDefaults
{
	inline constexpr float EventAttenuationRadiusCm = 2000.0f;
	inline constexpr int32 DefaultPriority = 50;
	inline constexpr int32 MaxPooledComponents = 32;
	inline constexpr int32 MaxConcurrentSounds = 16;
	inline constexpr float PoolCleanupIdleSeconds = 30.0f;
}

UENUM(BlueprintType)
enum class EHorrorAudioCategory : uint8
{
	Ambient UMETA(DisplayName="Ambient"),
	Anomaly UMETA(DisplayName="Anomaly"),
	Site UMETA(DisplayName="Site"),
	Interaction UMETA(DisplayName="Interaction"),
	Escape UMETA(DisplayName="Escape"),
	Music UMETA(DisplayName="Music")
};

USTRUCT(BlueprintType)
struct HORRORPROJECT_API FHorrorAudioZoneConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Audio")
	FName ZoneId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Audio")
	TObjectPtr<USoundBase> AmbientSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Audio", meta=(ClampMin="0.0", ClampMax="1.0"))
	float AmbientVolume = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Audio")
	bool bLoopAmbient = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Audio", meta=(ClampMin="0.0"))
	float FadeInDuration = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Audio", meta=(ClampMin="0.0"))
	float FadeOutDuration = 1.5f;
};

USTRUCT(BlueprintType)
struct HORRORPROJECT_API FHorrorAudioEventMapping
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Audio")
	FGameplayTag EventTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Audio")
	TObjectPtr<USoundBase> Sound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Audio", meta=(ClampMin="0.0", ClampMax="1.0"))
	float VolumeMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Audio")
	bool bAttachToSource = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Audio")
	bool bUse3DAttenuation = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Audio", meta=(ClampMin="0.0"))
	float AttenuationRadius = HorrorAudioDefaults::EventAttenuationRadiusCm;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Audio")
	TObjectPtr<USoundAttenuation> AttenuationSettings;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Audio")
	int32 Priority = HorrorAudioDefaults::DefaultPriority;
};

USTRUCT(BlueprintType)
struct HORRORPROJECT_API FHorrorAudioPoolEntry
{
	GENERATED_BODY()

	UPROPERTY()
	TObjectPtr<USoundBase> Sound;

	UPROPERTY()
	TObjectPtr<UAudioComponent> Component;

	UPROPERTY()
	float LastUsedTime = 0.0f;

	UPROPERTY()
	bool bInUse = false;
};

USTRUCT(BlueprintType)
struct HORRORPROJECT_API FHorrorAudioQueueEntry
{
	GENERATED_BODY()

	UPROPERTY()
	TObjectPtr<USoundBase> Sound;

	UPROPERTY()
	FVector Location = FVector::ZeroVector;

	UPROPERTY()
	float VolumeMultiplier = 1.0f;

	UPROPERTY()
	int32 Priority = HorrorAudioDefaults::DefaultPriority;

	UPROPERTY()
	float QueueTime = 0.0f;

	UPROPERTY()
	bool bIs3D = true;
};

/**
 * Coordinates Horror Audio Subsystem services for the Audio module.
 */
UCLASS()
class HORRORPROJECT_API UHorrorAudioSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintCallable, Category="Horror|Audio")
	UAudioComponent* PlaySoundAtLocation(USoundBase* Sound, FVector Location, float VolumeMultiplier = 1.0f, float PitchMultiplier = 1.0f);

	UFUNCTION(BlueprintCallable, Category="Horror|Audio")
	UAudioComponent* PlaySoundAttached(USoundBase* Sound, USceneComponent* AttachComponent, FName AttachPointName = NAME_None, float VolumeMultiplier = 1.0f);

	UFUNCTION(BlueprintCallable, Category="Horror|Audio")
	UAudioComponent* PlaySound2D(USoundBase* Sound, float VolumeMultiplier = 1.0f);

	UFUNCTION(BlueprintCallable, Category="Horror|Audio")
	bool PlayEventSound(FGameplayTag EventTag, UObject* SourceObject = nullptr);

	UFUNCTION(BlueprintCallable, Category="Horror|Audio")
	bool EnterAudioZone(FName ZoneId);

	UFUNCTION(BlueprintCallable, Category="Horror|Audio")
	bool ExitAudioZone(FName ZoneId);

	UFUNCTION(BlueprintCallable, Category="Horror|Audio")
	void StopAllAmbient(float FadeOutDuration = 1.0f);

	UFUNCTION(BlueprintCallable, Category="Horror|Audio")
	void SetCategoryVolume(EHorrorAudioCategory Category, float Volume);

	UFUNCTION(BlueprintPure, Category="Horror|Audio")
	float GetCategoryVolume(EHorrorAudioCategory Category) const;

	UFUNCTION(BlueprintPure, Category="Horror|Audio")
	FName GetCurrentZoneId() const { return CurrentZoneId; }

	UFUNCTION(BlueprintCallable, Category="Horror|Audio")
	void RegisterEventMapping(const FHorrorAudioEventMapping& Mapping);

	UFUNCTION(BlueprintCallable, Category="Horror|Audio")
	void RegisterZoneConfig(const FHorrorAudioZoneConfig& Config);

	UFUNCTION(BlueprintCallable, Category="Horror|Audio")
	void UnregisterEventMapping(FGameplayTag EventTag);

	UFUNCTION(BlueprintCallable, Category="Horror|Audio")
	void UnregisterZoneConfig(FName ZoneId);

	UFUNCTION(BlueprintCallable, Category="Horror|Audio")
	UAudioComponent* PlaySoundWithPriority(USoundBase* Sound, FVector Location, int32 Priority, float VolumeMultiplier = 1.0f);

	UFUNCTION(BlueprintCallable, Category="Horror|Audio")
	void QueueSound(USoundBase* Sound, FVector Location, int32 Priority, float VolumeMultiplier = 1.0f);

	UFUNCTION(BlueprintCallable, Category="Horror|Audio")
	void SetOcclusionEnabled(bool bEnabled);

	UFUNCTION(BlueprintCallable, Category="Horror|Audio")
	void UpdateOcclusion(float DeltaTime);

	UFUNCTION(BlueprintPure, Category="Horror|Audio")
	int32 GetActiveAudioComponentCount() const;

	UFUNCTION(BlueprintCallable, Category="Horror|Audio")
	void PreloadSound(USoundBase* Sound);

	UFUNCTION(BlueprintCallable, Category="Horror|Audio")
	void UnloadSound(USoundBase* Sound);

protected:
	UPROPERTY(EditDefaultsOnly, Category="Horror|Audio")
	TMap<EHorrorAudioCategory, float> CategoryVolumes;

	UPROPERTY(Transient)
	TMap<FGameplayTag, FHorrorAudioEventMapping> EventMappings;

	UPROPERTY(Transient)
	TMap<FName, FHorrorAudioZoneConfig> ZoneConfigs;

	UPROPERTY(Transient)
	TObjectPtr<UAudioComponent> CurrentAmbientComponent;

	UPROPERTY(Transient)
	FName CurrentZoneId = NAME_None;

	UPROPERTY(EditDefaultsOnly, Category="Horror|Audio|Pool")
	int32 MaxPooledComponents = HorrorAudioDefaults::MaxPooledComponents;

	UPROPERTY(EditDefaultsOnly, Category="Horror|Audio|Pool")
	float PoolCleanupInterval = 10.0f;

	UPROPERTY(EditDefaultsOnly, Category="Horror|Audio|Queue")
	int32 MaxConcurrentSounds = HorrorAudioDefaults::MaxConcurrentSounds;

	UPROPERTY(EditDefaultsOnly, Category="Horror|Audio|Occlusion")
	bool bEnableOcclusion = true;

	UPROPERTY(EditDefaultsOnly, Category="Horror|Audio|Occlusion")
	float OcclusionUpdateRate = 0.1f;

	UPROPERTY(EditDefaultsOnly, Category="Horror|Audio|Occlusion")
	float OcclusionVolumeMultiplier = 0.3f;

	UPROPERTY(EditDefaultsOnly, Category="Horror|Audio|Occlusion", meta=(ClampMin="0.0"))
	float OcclusionInterpSpeed = 8.0f;

private:
	UPROPERTY(Transient)
	TArray<FHorrorAudioPoolEntry> AudioPool;

	UPROPERTY(Transient)
	TArray<FHorrorAudioQueueEntry> AudioQueue;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UAudioComponent>> ActiveComponents;

	UPROPERTY(Transient)
	TSet<TObjectPtr<USoundBase>> PreloadedSounds;

	UPROPERTY(Transient)
	TMap<TObjectPtr<UAudioComponent>, float> ComponentBaseVolumes;

	float LastPoolCleanupTime = 0.0f;
	float LastOcclusionUpdateTime = 0.0f;

	void OnEventPublished(const struct FHorrorEventMessage& Message);
	void InitializeDefaultVolumes();
	void ProcessAudioQueue();
	void CleanupAudioPool();
	UAudioComponent* GetPooledComponent(USoundBase* Sound);
	void ReturnComponentToPool(UAudioComponent* Component);
	bool CanPlaySound() const;
	void UpdateActiveSounds(float DeltaTime);
};
