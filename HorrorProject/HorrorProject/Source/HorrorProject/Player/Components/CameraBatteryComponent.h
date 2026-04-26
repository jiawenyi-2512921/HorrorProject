// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "CameraBatteryComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FBatteryPercentageChangedSignature, float, NewPercentage);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FBatteryDepletedSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FBatteryLowWarningSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FBatteryChargingChangedSignature);

UCLASS(ClassGroup=(Horror), BlueprintType, Blueprintable, meta=(BlueprintSpawnableComponent))
class HORRORPROJECT_API UCameraBatteryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCameraBatteryComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, Category="CameraBattery|Runtime")
	void StartRecordingDrain();

	UFUNCTION(BlueprintCallable, Category="CameraBattery|Runtime")
	void StopRecordingDrain();

	UFUNCTION(BlueprintCallable, Category="CameraBattery|Runtime")
	void StartFlashlightDrain();

	UFUNCTION(BlueprintCallable, Category="CameraBattery|Runtime")
	void StopFlashlightDrain();

	UFUNCTION(BlueprintCallable, Category="CameraBattery|Runtime")
	void StartCharging();

	UFUNCTION(BlueprintCallable, Category="CameraBattery|Runtime")
	void StopCharging();

	UFUNCTION(BlueprintCallable, Category="CameraBattery|Runtime")
	void SetBatteryPercentage(float NewPercentage);

	UFUNCTION(BlueprintPure, Category="CameraBattery|Runtime")
	float GetBatteryPercentage() const { return BatteryPercentage; }

	UFUNCTION(BlueprintPure, Category="CameraBattery|Runtime")
	bool IsBatteryDepleted() const { return BatteryPercentage <= 0.0f; }

	UFUNCTION(BlueprintPure, Category="CameraBattery|Runtime")
	bool IsBatteryLow() const { return BatteryPercentage <= LowBatteryThreshold; }

	UFUNCTION(BlueprintPure, Category="CameraBattery|Runtime")
	bool IsCharging() const { return bIsCharging; }

	UFUNCTION(BlueprintPure, Category="CameraBattery|Runtime")
	bool IsRecordingDraining() const { return bIsRecordingDraining; }

	UFUNCTION(BlueprintPure, Category="CameraBattery|Runtime")
	bool IsFlashlightDraining() const { return bIsFlashlightDraining; }

	UPROPERTY(BlueprintAssignable, Category="CameraBattery|Events")
	FBatteryPercentageChangedSignature OnBatteryPercentageChanged;

	UPROPERTY(BlueprintAssignable, Category="CameraBattery|Events")
	FBatteryDepletedSignature OnBatteryDepleted;

	UPROPERTY(BlueprintAssignable, Category="CameraBattery|Events")
	FBatteryLowWarningSignature OnBatteryLowWarning;

	UPROPERTY(BlueprintAssignable, Category="CameraBattery|Events")
	FBatteryChargingChangedSignature OnBatteryChargingChanged;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CameraBattery|Config", meta=(ClampMin="0.0", ClampMax="100.0"))
	float InitialBatteryPercentage = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CameraBattery|Config", meta=(ClampMin="0.0"))
	float RecordingDrainRate = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CameraBattery|Config", meta=(ClampMin="0.0"))
	float FlashlightDrainRate = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CameraBattery|Config", meta=(ClampMin="0.0"))
	float ChargeRate = 15.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CameraBattery|Config", meta=(ClampMin="0.0", ClampMax="100.0"))
	float LowBatteryThreshold = 20.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CameraBattery|Events")
	FGameplayTag BatteryDepletedEventTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CameraBattery|Events")
	FGameplayTag BatteryLowEventTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CameraBattery|Events")
	FGameplayTag BatteryChargingEventTag;

private:
	void UpdateBattery(float DeltaTime);
	void BroadcastBatteryChange();
	void PublishEventBusEvent(FGameplayTag EventTag);

	UPROPERTY(VisibleInstanceOnly, Category="CameraBattery|Runtime")
	float BatteryPercentage = 100.0f;

	UPROPERTY(VisibleInstanceOnly, Category="CameraBattery|Runtime")
	bool bIsRecordingDraining = false;

	UPROPERTY(VisibleInstanceOnly, Category="CameraBattery|Runtime")
	bool bIsFlashlightDraining = false;

	UPROPERTY(VisibleInstanceOnly, Category="CameraBattery|Runtime")
	bool bIsCharging = false;

	bool bHasTriggeredLowWarning = false;
	bool bHasTriggeredDepletion = false;
};
