// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FlashlightComponent.generated.h"

class USpotLightComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FFlashlightToggledSignature, bool, bIsOn);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FFlashlightBatteryChangedSignature, float, BatteryPercent, float, BatterySeconds);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FFlashlightBatteryDepletedSignature);

UCLASS(ClassGroup=(Horror), BlueprintType, Blueprintable, meta=(BlueprintSpawnableComponent))
class HORRORPROJECT_API UFlashlightComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UFlashlightComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, Category="Flashlight")
	bool ToggleFlashlight();

	UFUNCTION(BlueprintCallable, Category="Flashlight")
	bool SetFlashlightEnabled(bool bEnabled);

	UFUNCTION(BlueprintPure, Category="Flashlight")
	bool IsFlashlightOn() const { return bFlashlightOn; }

	UFUNCTION(BlueprintCallable, Category="Flashlight|Battery")
	bool AddBatteryCharge(float Seconds);

	UFUNCTION(BlueprintPure, Category="Flashlight|Battery")
	float GetBatteryPercent() const;

	UFUNCTION(BlueprintPure, Category="Flashlight|Battery")
	float GetBatterySeconds() const { return BatterySeconds; }

	UFUNCTION(BlueprintPure, Category="Flashlight|Battery")
	float GetMaxBatterySeconds() const { return MaxBatterySeconds; }

	UFUNCTION(BlueprintPure, Category="Flashlight|Battery")
	bool IsBatteryDepleted() const { return BatterySeconds <= 0.0f; }

	UFUNCTION(BlueprintCallable, Category="Flashlight|Light")
	bool BindSpotLight(USpotLightComponent* SpotLight);

	UFUNCTION(BlueprintPure, Category="Flashlight|Light")
	USpotLightComponent* GetBoundSpotLight() const { return BoundSpotLight; }

	UPROPERTY(BlueprintAssignable, Category="Flashlight")
	FFlashlightToggledSignature OnFlashlightToggled;

	UPROPERTY(BlueprintAssignable, Category="Flashlight|Battery")
	FFlashlightBatteryChangedSignature OnBatteryChanged;

	UPROPERTY(BlueprintAssignable, Category="Flashlight|Battery")
	FFlashlightBatteryDepletedSignature OnBatteryDepleted;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Flashlight|Battery", meta=(ClampMin="0.0", Units="s"))
	float MaxBatterySeconds = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Flashlight|Battery", meta=(ClampMin="0.0", Units="s"))
	float StartingBatterySeconds = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Flashlight|Battery", meta=(ClampMin="0.0", ClampMax="100.0", Units="s"))
	float BatteryDrainRate = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Flashlight|Battery")
	bool bAutoDepleteBattery = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Flashlight|Battery")
	bool bAutoDisableOnDepletion = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Flashlight|Light", meta=(ClampMin="0.0"))
	float FlickerThresholdPercent = 0.15f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Flashlight|Light", meta=(ClampMin="0.0", ClampMax="1.0"))
	float FlickerIntensityMin = 0.3f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Flashlight|Light", meta=(ClampMin="0.0", ClampMax="10.0"))
	float FlickerFrequency = 8.0f;

private:
	void UpdateLightState();
	void UpdateBatteryDrain(float DeltaTime);
	void UpdateFlickerEffect(float DeltaTime);
	bool TryAutoBindSpotLight();

	UPROPERTY(Transient)
	TObjectPtr<USpotLightComponent> BoundSpotLight;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, SaveGame, Category="Flashlight", meta=(AllowPrivateAccess="true"))
	bool bFlashlightOn = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, SaveGame, Category="Flashlight|Battery", meta=(AllowPrivateAccess="true"))
	float BatterySeconds = 300.0f;

	float FlickerTime = 0.0f;
	float BaseIntensity = 0.0f;
	bool bBatteryDepletedBroadcast = false;
};
