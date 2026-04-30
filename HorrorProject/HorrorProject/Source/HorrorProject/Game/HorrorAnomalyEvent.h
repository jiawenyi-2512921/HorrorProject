// Copyright HorrorProject. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "HorrorAnomalyEvent.generated.h"

class USceneComponent;
class UBoxComponent;
class UParticleSystemComponent;
class USoundBase;
class UCameraShakeBase;
class UMaterialInstanceDynamic;
class AHorrorEventBusSubsystem;

UENUM(BlueprintType)
enum class EAnomalyType : uint8
{
	VisualGlitch UMETA(DisplayName="视觉故障"),
	SpatialDistortion UMETA(DisplayName="空间扭曲"),
	TemporalShift UMETA(DisplayName="时间偏移"),
	EntityManifestation UMETA(DisplayName="实体显现"),
	EnvironmentalCollapse UMETA(DisplayName="环境崩塌"),
	SoundAnomaly UMETA(DisplayName="声音异常")
};

UENUM(BlueprintType)
enum class EAnomalyState : uint8
{
	Dormant UMETA(DisplayName="休眠"),
	Arming UMETA(DisplayName="预备"),
	Active UMETA(DisplayName="激活"),
	Escalating UMETA(DisplayName="升级"),
	Resolving UMETA(DisplayName="解除中"),
	Resolved UMETA(DisplayName="已解除")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FAnomalyStateChangedSignature, EAnomalyState, NewState, EAnomalyType, AnomalyType);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAnomalyPlayerEnteredSignature, AActor*, PlayerActor);

UCLASS(BlueprintType, Blueprintable, ClassGroup=(Horror))
class HORRORPROJECT_API AHorrorAnomalyEvent : public AActor
{
	GENERATED_BODY()

public:
	AHorrorAnomalyEvent();

	UFUNCTION(BlueprintCallable, Category="Horror|Anomaly")
	void ArmAnomaly();

	UFUNCTION(BlueprintCallable, Category="Horror|Anomaly")
	void TriggerAnomaly(AActor* InstigatorActor = nullptr);

	UFUNCTION(BlueprintCallable, Category="Horror|Anomaly")
	void EscalateAnomaly();

	UFUNCTION(BlueprintCallable, Category="Horror|Anomaly")
	void ResolveAnomaly();

	UFUNCTION(BlueprintCallable, Category="Horror|Anomaly")
	void ResetAnomaly();

	UFUNCTION(BlueprintPure, Category="Horror|Anomaly")
	EAnomalyState GetAnomalyState() const { return AnomalyState; }

	UFUNCTION(BlueprintPure, Category="Horror|Anomaly")
	EAnomalyType GetAnomalyType() const { return AnomalyType; }

	UFUNCTION(BlueprintPure, Category="Horror|Anomaly")
	bool IsPlayerInZone() const;

	UFUNCTION(BlueprintPure, Category="Horror|Anomaly")
	float GetAnomalyIntensity() const { return CurrentIntensity; }

	UPROPERTY(BlueprintAssignable, Category="Horror|Anomaly")
	FAnomalyStateChangedSignature OnAnomalyStateChanged;

	UPROPERTY(BlueprintAssignable, Category="Horror|Anomaly")
	FAnomalyPlayerEnteredSignature OnPlayerEnteredAnomalyZone;

	// Configuration
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Anomaly")
	EAnomalyType AnomalyType = EAnomalyType::VisualGlitch;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Anomaly", meta=(ClampMin="0.0", Units="s"))
	float ArmDuration = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Anomaly", meta=(ClampMin="0.0", Units="s"))
	float ActiveDuration = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Anomaly", meta=(ClampMin="0.0", Units="s"))
	float EscalationDuration = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Anomaly", meta=(ClampMin="0.0", ClampMax="1.0"))
	float MaxIntensity = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Anomaly")
	bool bAutoTriggerOnPlayerEnter = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Anomaly")
	bool bRequiresRecording = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Anomaly")
	bool bSingleUse = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Anomaly")
	FName AnomalyId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Anomaly")
	FGameplayTag AnomalyEventTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Anomaly|Effects")
	TObjectPtr<USoundBase> AnomalySound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Anomaly|Effects")
	TObjectPtr<USoundBase> EscalationSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Anomaly|Effects")
	TSubclassOf<UCameraShakeBase> AnomalyCameraShake;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Anomaly|Effects", meta=(ClampMin="0.0", ClampMax="1.0"))
	float CameraShakeScale = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Anomaly|Effects")
	float FearAmount = 20.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Anomaly|Effects")
	float SanityDrain = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Anomaly|EventBus")
	bool bPublishToEventBus = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Anomaly|EventBus")
	FName EventBusSourceId = TEXT("AnomalyEvent");

protected:
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintImplementableEvent, Category="Horror|Anomaly")
	void BP_OnAnomalyTriggered(AActor* InstigatorActor);

	UFUNCTION(BlueprintImplementableEvent, Category="Horror|Anomaly")
	void BP_OnAnomalyEscalated();

	UFUNCTION(BlueprintImplementableEvent, Category="Horror|Anomaly")
	void BP_OnAnomalyResolved();

	UFUNCTION(BlueprintImplementableEvent, Category="Horror|Anomaly")
	void BP_OnAnomalyReset();

private:
	UFUNCTION()
	void OnTriggerOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	void UpdateAnomalyEffects(float DeltaTime);
	void ApplyPlayerFeedback(AActor* PlayerActor);
	void PlayAnomalySound(USoundBase* Sound);
	void TriggerCameraShake();
	void PublishAnomalyEvent(FName EventName);
	void TransitionState(EAnomalyState NewState);

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category="Horror|Anomaly", meta=(AllowPrivateAccess="true"))
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category="Horror|Anomaly", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UBoxComponent> TriggerZone;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category="Horror|Anomaly", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UParticleSystemComponent> AnomalyParticles;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Horror|Anomaly", meta=(AllowPrivateAccess="true"))
	EAnomalyState AnomalyState = EAnomalyState::Dormant;

	float StateTimer = 0.0f;
	float CurrentIntensity = 0.0f;
	bool bHasTriggered = false;

	FTimerHandle AnomalyTimerHandle;
};
