// Copyright HorrorProject. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "GameplayTagContainer.h"
#include "Game/HorrorEventBusSubsystem.h"
#include "HorrorDirectorSubsystem.generated.h"

class AHorrorEncounterDirector;
class AActor;
class USoundBase;

UENUM(BlueprintType)
enum class ETensionLevel : uint8
{
	Silent UMETA(DisplayName="平静"),
	Uneasy UMETA(DisplayName="不安"),
	Tense UMETA(DisplayName="紧绷"),
	Dread UMETA(DisplayName="恐惧"),
	Peak UMETA(DisplayName="峰值")
};

UENUM(BlueprintType)
enum class EHorrorDirectorState : uint8
{
	Idle UMETA(DisplayName="待机"),
	BuildingTension UMETA(DisplayName="积累紧张"),
	Sustaining UMETA(DisplayName="维持紧张"),
	Releasing UMETA(DisplayName="释放紧张"),
	CoolingDown UMETA(DisplayName="冷却")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FHorrorDirectorTensionChangedSignature, ETensionLevel, NewLevel, float, TensionValue);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FHorrorDirectorEventSignature, EHorrorDirectorState, NewState, float, TensionValue);

USTRUCT(BlueprintType)
struct FHorrorDirectorTensionCurve
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Director", meta=(ClampMin="0.0", ClampMax="1.0"))
	float SilentMax = 0.15f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Director", meta=(ClampMin="0.0", ClampMax="1.0"))
	float UneasyMax = 0.35f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Director", meta=(ClampMin="0.0", ClampMax="1.0"))
	float TenseMax = 0.6f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Director", meta=(ClampMin="0.0", ClampMax="1.0"))
	float DreadMax = 0.85f;
};

USTRUCT(BlueprintType)
struct FHorrorDirectorEncounterWeight
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Director")
	FName EncounterId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Director")
	FGameplayTag RequiredStateTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Director", meta=(ClampMin="0.0"))
	float BaseWeight = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Director", meta=(ClampMin="0.0"))
	int32 MaxTriggerCount = 1;

	UPROPERTY(Transient, BlueprintReadOnly, Category="Director")
	int32 TimesTriggered = 0;
};

UCLASS()
class HORRORPROJECT_API UHorrorDirectorSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;

	UFUNCTION(BlueprintCallable, Category="Horror|Director")
	void SetDirectorEnabled(bool bEnabled);

	UFUNCTION(BlueprintCallable, Category="Horror|Director")
	void AddTension(float Amount, FName SourceId = NAME_None);

	UFUNCTION(BlueprintCallable, Category="Horror|Director")
	void ReduceTension(float Amount);

	UFUNCTION(BlueprintCallable, Category="Horror|Director")
	void SetTension(float NewTension);

	UFUNCTION(BlueprintCallable, Category="Horror|Director")
	void RegisterEncounter(AHorrorEncounterDirector* Encounter);

	UFUNCTION(BlueprintCallable, Category="Horror|Director")
	void UnregisterEncounter(AHorrorEncounterDirector* Encounter);

	UFUNCTION(BlueprintCallable, Category="Horror|Director")
	void OnPlayerHealthChanged(float HealthPercent);

	UFUNCTION(BlueprintCallable, Category="Horror|Director")
	void OnPlayerSanityChanged(float SanityPercent);

	UFUNCTION(BlueprintCallable, Category="Horror|Director")
	void OnPlayerNoiseGenerated(float NoiseLevel);

	UFUNCTION(BlueprintCallable, Category="Horror|Director")
	void OnEncounterResolved(FName EncounterId);

	UFUNCTION(BlueprintPure, Category="Horror|Director")
	float GetTension() const { return TensionValue; }

	UFUNCTION(BlueprintPure, Category="Horror|Director")
	ETensionLevel GetTensionLevel() const { return CurrentTensionLevel; }

	UFUNCTION(BlueprintPure, Category="Horror|Director")
	EHorrorDirectorState GetDirectorState() const { return DirectorState; }

	UFUNCTION(BlueprintPure, Category="Horror|Director")
	bool IsDirectorEnabled() const { return bDirectorEnabled; }

	UFUNCTION(BlueprintCallable, Category="Horror|Director")
	void ForceTriggerEncounter(FName EncounterId);

	UPROPERTY(BlueprintAssignable, Category="Horror|Director")
	FHorrorDirectorTensionChangedSignature OnTensionChanged;

	UPROPERTY(BlueprintAssignable, Category="Horror|Director")
	FHorrorDirectorEventSignature OnDirectorStateChanged;

	// Configuration
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Director|Tension")
	FHorrorDirectorTensionCurve TensionCurve;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Director|Tension", meta=(ClampMin="0.0", ClampMax="1.0"))
	float TensionBuildRate = 0.05f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Director|Tension", meta=(ClampMin="0.0", ClampMax="1.0"))
	float TensionDecayRate = 0.02f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Director|Tension", meta=(ClampMin="0.0", Units="s"))
	float CooldownDuration = 15.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Director|Tension", meta=(ClampMin="0.0", Units="s"))
	float SustainDuration = 8.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Director|Encounters")
	TArray<FHorrorDirectorEncounterWeight> EncounterWeights;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Director|Encounters", meta=(ClampMin="0.0", ClampMax="1.0"))
	float EncounterTriggerThreshold = 0.6f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Director|Encounters", meta=(ClampMin="0.0", Units="s"))
	float EncounterCooldown = 30.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Director|Ambient")
	TObjectPtr<USoundBase> AmbientTensionSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Director|Ambient", meta=(ClampMin="0.0", ClampMax="1.0"))
	float MaxAmbientVolume = 0.7f;

private:
	void UpdateTension(float DeltaTime);
	void UpdateDirectorState(float DeltaTime);
	void CheckEncounterTrigger();
	void TriggerBestEncounter();
	AHorrorEncounterDirector* SelectEncounterByWeight() const;
	void UpdateAmbientAudio();
	void PublishDirectorEvent(FName EventName);
	void HandleEventBusEvent(const struct FHorrorEventMessage& Message);

	UPROPERTY(Transient)
	TArray<TWeakObjectPtr<AHorrorEncounterDirector>> RegisteredEncounters;

	UPROPERTY(Transient)
	TWeakObjectPtr<AHorrorEncounterDirector> ActiveEncounter;

	float TensionValue = 0.0f;
	ETensionLevel CurrentTensionLevel = ETensionLevel::Silent;
	EHorrorDirectorState DirectorState = EHorrorDirectorState::Idle;
	bool bDirectorEnabled = true;
	float StateTimer = 0.0f;
	float EncounterCooldownTimer = 0.0f;
	float PlayerHealthPercent = 1.0f;
	float PlayerSanityPercent = 1.0f;
	float LastNoiseLevel = 0.0f;

	FTimerHandle UpdateTimerHandle;
};
