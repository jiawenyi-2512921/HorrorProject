// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "GameplayTagContainer.h"
#include "Sound/SoundAttenuation.h"
#include "TimerManager.h"
#include "UObject/SoftObjectPath.h"
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
	Ambient UMETA(DisplayName="环境"),
	Anomaly UMETA(DisplayName="异常"),
	Site UMETA(DisplayName="站点"),
	Interaction UMETA(DisplayName="交互"),
	Escape UMETA(DisplayName="逃离"),
	Music UMETA(DisplayName="音乐")
};

UENUM(BlueprintType)
enum class EHorrorDay1AudioStage : uint8
{
	Exploration UMETA(DisplayName="探索"),
	Objective UMETA(DisplayName="目标"),
	Anomaly UMETA(DisplayName="异常"),
	Chase UMETA(DisplayName="追逐"),
	Resolved UMETA(DisplayName="已解决"),
	Escape UMETA(DisplayName="逃离"),
	Complete UMETA(DisplayName="完成"),
	Failure UMETA(DisplayName="失败")
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
	EHorrorAudioCategory Category = EHorrorAudioCategory::Site;

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

	bool PlayEventSound(const struct FHorrorEventMessage& Message);

	UFUNCTION(BlueprintCallable, Category="Horror|Audio")
	bool EnterAudioZone(FName ZoneId);

	UFUNCTION(BlueprintCallable, Category="Horror|Audio")
	bool ExitAudioZone(FName ZoneId);

	UFUNCTION(BlueprintCallable, Category="Horror|Audio")
	bool RegisterDefaultHorrorAmbience();

	UFUNCTION(BlueprintCallable, Category="Horror|Audio")
	bool StartDefaultHorrorAmbience();

	UFUNCTION(BlueprintCallable, Category="Horror|Audio")
	void StopAllAmbient(float FadeOutDuration = 1.0f);

	UFUNCTION(BlueprintCallable, Category="Horror|Audio")
	void SetCategoryVolume(EHorrorAudioCategory Category, float Volume);

	UFUNCTION(BlueprintPure, Category="Horror|Audio")
	float GetCategoryVolume(EHorrorAudioCategory Category) const;

	UFUNCTION(BlueprintPure, Category="Horror|Audio")
	FName GetCurrentZoneId() const { return CurrentZoneId; }

	UFUNCTION(BlueprintCallable, Category="Horror|Audio|Day1")
	bool HandleDay1Event(FGameplayTag EventTag, FName SourceId);

	UFUNCTION(BlueprintCallable, Category="Horror|Audio|Day1")
	bool HandleDay1EventName(FName EventName, FName SourceId);

	UFUNCTION(BlueprintCallable, Category="Horror|Audio|Day1")
	void SetDay1AudioStage(EHorrorDay1AudioStage NewStage, FGameplayTag EventTag, FName SourceId);

	UFUNCTION(BlueprintPure, Category="Horror|Audio|Day1")
	EHorrorDay1AudioStage GetDay1AudioStage() const { return CurrentDay1AudioStage; }

	UFUNCTION(BlueprintPure, Category="Horror|Audio|Day1")
	FGameplayTag GetLastDay1AudioEventTag() const { return LastDay1AudioEventTag; }

	UFUNCTION(BlueprintPure, Category="Horror|Audio|Day1")
	FName GetLastDay1AudioSourceId() const { return LastDay1AudioSourceId; }

	UFUNCTION(BlueprintPure, Category="Horror|Audio|Day1")
	float GetDay1AudioStageVolumeMultiplier() const;

	UFUNCTION(BlueprintPure, Category="Horror|Audio|Day1")
	EHorrorAudioCategory GetDay1AudioStageCategory() const;

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

#if WITH_DEV_AUTOMATION_TESTS
	bool HasEventMappingForTests(FGameplayTag EventTag) const;
	EHorrorAudioCategory GetEventMappingCategoryForTests(FGameplayTag EventTag) const;
	int32 GetEventMappingCountForTests() const;
	bool HasDefaultHorrorAmbienceForTests() const;
	FString GetDefaultHorrorAmbienceSoundPathForTests() const;
	FName GetDefaultHorrorAmbienceZoneIdForTests() const { return DefaultHorrorAmbienceZoneId; }
	EHorrorAudioCategory ResolveAdvancedInteractionAudioCategoryForTests(const struct FHorrorEventMessage& Message) const;
	float ResolveAdvancedInteractionAudioVolumeForTests(const struct FHorrorEventMessage& Message) const;
	FGameplayTag GetLastResolvedEventAudioTagForTests() const { return LastResolvedEventAudioTagForTests; }
	EHorrorAudioCategory GetLastResolvedEventAudioCategoryForTests() const { return LastResolvedEventAudioCategoryForTests; }
	float GetLastResolvedEventAudioVolumeForTests() const { return LastResolvedEventAudioVolumeForTests; }
	bool WasLastResolvedEventAudioSuppressedForTests() const { return bLastResolvedEventAudioSuppressedForTests; }
#endif

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

	UPROPERTY(BlueprintReadOnly, Transient, Category="Horror|Audio|Day1", meta=(AllowPrivateAccess="true"))
	EHorrorDay1AudioStage CurrentDay1AudioStage = EHorrorDay1AudioStage::Exploration;

	UPROPERTY(BlueprintReadOnly, Transient, Category="Horror|Audio|Day1", meta=(AllowPrivateAccess="true"))
	FGameplayTag LastDay1AudioEventTag;

	UPROPERTY(BlueprintReadOnly, Transient, Category="Horror|Audio|Day1", meta=(AllowPrivateAccess="true"))
	FName LastDay1AudioSourceId = NAME_None;

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

	UPROPERTY(EditDefaultsOnly, Category="Horror|Audio|Default Ambience")
	bool bEnableDefaultHorrorAmbience = true;

	UPROPERTY(EditDefaultsOnly, Category="Horror|Audio|Default Ambience")
	FName DefaultHorrorAmbienceZoneId = TEXT("Campaign.DefaultHorrorAmbience");

	UPROPERTY(EditDefaultsOnly, Category="Horror|Audio|Default Ambience")
	FSoftObjectPath DefaultHorrorAmbienceSoundPath = FSoftObjectPath(TEXT("/Game/Horror/Audio/MainTitles.MainTitles"));

	UPROPERTY(EditDefaultsOnly, Category="Horror|Audio|Default Ambience", meta=(ClampMin="0.0", ClampMax="1.0"))
	float DefaultHorrorAmbienceVolume = 0.9f;

	UPROPERTY(EditDefaultsOnly, Category="Horror|Audio|Default Ambience", meta=(ClampMin="0.0", Units="s"))
	float DefaultHorrorAmbienceFadeInSeconds = 3.0f;

	UPROPERTY(EditDefaultsOnly, Category="Horror|Audio|Default Ambience", meta=(ClampMin="0.0", Units="s"))
	float DefaultHorrorAmbienceFadeOutSeconds = 1.5f;

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

	FTimerHandle ActiveSoundUpdateTimerHandle;

	float LastPoolCleanupTime = 0.0f;
	float LastOcclusionUpdateTime = 0.0f;
	float ActiveSoundUpdateInterval = 0.1f;

#if WITH_DEV_AUTOMATION_TESTS
	FGameplayTag LastResolvedEventAudioTagForTests;
	EHorrorAudioCategory LastResolvedEventAudioCategoryForTests = EHorrorAudioCategory::Site;
	float LastResolvedEventAudioVolumeForTests = 0.0f;
	bool bLastResolvedEventAudioSuppressedForTests = false;
#endif

	void OnEventPublished(const struct FHorrorEventMessage& Message);
	void InitializeDefaultVolumes();
	void RegisterDefaultDay1AudioMappings();
	USoundBase* ResolveDefaultHorrorAmbienceSound() const;
	bool TryResolveDay1StageFromEvent(FGameplayTag EventTag, FName EventName, EHorrorDay1AudioStage& OutStage) const;
	EHorrorAudioCategory ResolveAdvancedInteractionAudioCategory(const struct FHorrorEventMessage& Message) const;
	float ResolveAdvancedInteractionAudioVolume(const struct FHorrorEventMessage& Message) const;
	bool PlayMappedEventSound(const FHorrorAudioEventMapping& Mapping, UObject* SourceObject, EHorrorAudioCategory EffectiveCategory, float EffectiveVolumeMultiplier);
#if WITH_DEV_AUTOMATION_TESTS
	void RecordResolvedEventAudioForTests(FGameplayTag EventTag, EHorrorAudioCategory Category, float VolumeMultiplier, bool bSuppressed);
#endif
	void ProcessAudioQueue();
	void CleanupAudioPool();
	UAudioComponent* GetPooledComponent(USoundBase* Sound);
	void ReturnComponentToPool(UAudioComponent* Component);
	bool CanPlaySound() const;
	void TickActiveAudioSystems();
	void UpdateActiveSounds(float DeltaTime);
};
