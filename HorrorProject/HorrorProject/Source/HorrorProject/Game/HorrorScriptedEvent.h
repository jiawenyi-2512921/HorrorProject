// Copyright HorrorProject. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "HorrorScriptedEvent.generated.h"

class USceneComponent;
class UBoxComponent;
class USoundBase;
class UCameraShakeBase;
class UParticleSystem;
class AHorrorEncounterDirector;
class AHorrorAnomalyEvent;

UENUM(BlueprintType)
enum class EScriptedEventType : uint8
{
	JumpScare UMETA(DisplayName="惊吓事件"),
	EnvironmentalEvent UMETA(DisplayName="环境事件"),
	SoundEvent UMETA(DisplayName="声音事件"),
	VisualEvent UMETA(DisplayName="视觉事件"),
	ChaseSequence UMETA(DisplayName="追逐段落"),
	TensionBuildup UMETA(DisplayName="紧张铺垫")
};

UENUM(BlueprintType)
enum class EScriptedEventState : uint8
{
	Idle UMETA(DisplayName="待机"),
	Primed UMETA(DisplayName="已预备"),
	Executing UMETA(DisplayName="执行中"),
	Completed UMETA(DisplayName="已完成"),
	Failed UMETA(DisplayName="已失败")
};

USTRUCT(BlueprintType)
struct FScriptedEventStep
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="ScriptedEvent")
	FName StepId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="ScriptedEvent", meta=(ClampMin="0.0", Units="s"))
	float DelayBeforeStep = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="ScriptedEvent", meta=(ClampMin="0.0", Units="s"))
	float StepDuration = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="ScriptedEvent")
	TObjectPtr<USoundBase> Sound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="ScriptedEvent")
	TSubclassOf<UCameraShakeBase> CameraShake;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="ScriptedEvent", meta=(ClampMin="0.0", ClampMax="5.0"))
	float CameraShakeScale = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="ScriptedEvent")
	float FearAmount = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="ScriptedEvent")
	float SanityDrain = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="ScriptedEvent")
	bool bTriggerLightFlicker = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="ScriptedEvent")
	bool bTriggerAudioDistortion = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="ScriptedEvent")
	FGameplayTag EventToPublish;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="ScriptedEvent")
	FText SubtitleText;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FScriptedEventStateChangedSignature, EScriptedEventState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FScriptedEventStepStartedSignature, int32, StepIndex, const FScriptedEventStep&, Step);

UCLASS(BlueprintType, Blueprintable, ClassGroup=(Horror))
class HORRORPROJECT_API AHorrorScriptedEvent : public AActor
{
	GENERATED_BODY()

public:
	AHorrorScriptedEvent();

	UFUNCTION(BlueprintCallable, Category="Horror|ScriptedEvent")
	void PrimeEvent();

	UFUNCTION(BlueprintCallable, Category="Horror|ScriptedEvent")
	void ExecuteEvent(AActor* InstigatorActor = nullptr);

	UFUNCTION(BlueprintCallable, Category="Horror|ScriptedEvent")
	void CancelEvent();

	UFUNCTION(BlueprintCallable, Category="Horror|ScriptedEvent")
	void ResetEvent();

	UFUNCTION(BlueprintPure, Category="Horror|ScriptedEvent")
	EScriptedEventState GetEventState() const { return EventState; }

	UFUNCTION(BlueprintPure, Category="Horror|ScriptedEvent")
	int32 GetCurrentStepIndex() const { return CurrentStepIndex; }

	UFUNCTION(BlueprintPure, Category="Horror|ScriptedEvent")
	bool IsExecuting() const { return EventState == EScriptedEventState::Executing; }

	UPROPERTY(BlueprintAssignable, Category="Horror|ScriptedEvent")
	FScriptedEventStateChangedSignature OnEventStateChanged;

	UPROPERTY(BlueprintAssignable, Category="Horror|ScriptedEvent")
	FScriptedEventStepStartedSignature OnStepStarted;

	// Configuration
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|ScriptedEvent")
	FName EventId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|ScriptedEvent")
	EScriptedEventType EventType = EScriptedEventType::JumpScare;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|ScriptedEvent")
	bool bAutoTriggerOnPlayerEnter = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|ScriptedEvent")
	bool bSingleUse = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|ScriptedEvent")
	bool bRequiresPriming = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|ScriptedEvent")
	FGameplayTag RequiredStateTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|ScriptedEvent", meta=(ClampMin="0.0", Units="s"))
	float ReTriggerCooldown = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Horror|ScriptedEvent")
	TArray<FScriptedEventStep> EventSteps;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|ScriptedEvent")
	TObjectPtr<AHorrorEncounterDirector> LinkedEncounter;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|ScriptedEvent")
	TObjectPtr<AHorrorAnomalyEvent> LinkedAnomaly;

	// Audio
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|ScriptedEvent|Audio")
	TObjectPtr<USoundBase> AmbientSoundOverride;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|ScriptedEvent|Audio", meta=(ClampMin="0.0", ClampMax="1.0"))
	float AmbientVolumeOverride = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|ScriptedEvent|EventBus")
	bool bPublishToEventBus = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|ScriptedEvent|EventBus")
	FName EventBusSourceId = TEXT("ScriptedEvent");

protected:
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintImplementableEvent, Category="Horror|ScriptedEvent")
	void BP_OnEventExecuted(AActor* InstigatorActor);

	UFUNCTION(BlueprintImplementableEvent, Category="Horror|ScriptedEvent")
	void BP_OnStepExecuted(int32 StepIndex, const FScriptedEventStep& Step);

	UFUNCTION(BlueprintImplementableEvent, Category="Horror|ScriptedEvent")
	void BP_OnEventCompleted();

private:
	UFUNCTION()
	void OnTriggerOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	void ExecuteStep(int32 StepIndex);
	void ApplyStepEffects(const FScriptedEventStep& Step, AActor* PlayerActor);
	void PlayStepSound(USoundBase* Sound);
	void TriggerStepCameraShake(TSubclassOf<UCameraShakeBase> ShakeClass, float Scale);
	void PublishStepEvent(FGameplayTag EventTag);
	void TransitionState(EScriptedEventState NewState);

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category="Horror|ScriptedEvent", meta=(AllowPrivateAccess="true"))
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category="Horror|ScriptedEvent", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UBoxComponent> TriggerZone;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Horror|ScriptedEvent", meta=(AllowPrivateAccess="true"))
	EScriptedEventState EventState = EScriptedEventState::Idle;

	int32 CurrentStepIndex = -1;
	bool bHasTriggered = false;
	float LastTriggerTime = -1000.0f;

	FTimerHandle StepTimerHandle;
	FTimerHandle DelayTimerHandle;
};
