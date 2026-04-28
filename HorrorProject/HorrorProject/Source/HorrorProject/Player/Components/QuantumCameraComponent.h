// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "QuantumCameraComponent.generated.h"

UENUM(BlueprintType)
enum class EQuantumCameraMode : uint8
{
	Disabled UMETA(DisplayName="已禁用"),
	Viewfinder UMETA(DisplayName="取景"),
	Recording UMETA(DisplayName="录制"),
	Photo UMETA(DisplayName="拍照"),
	Rewind UMETA(DisplayName="倒带")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FQuantumCameraModeChangedSignature, EQuantumCameraMode, NewMode);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FQuantumCameraAcquiredChangedSignature, bool, bNewAcquired);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FQuantumCameraEnabledChangedSignature, bool, bNewEnabled);

UCLASS(MinimalAPI, Hidden, NotBlueprintable, NotPlaceable)
class UQuantumCameraRuntimeDelegateProbe : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION()
	void HandleCameraAcquiredChanged(bool bNewAcquired);

	UFUNCTION()
	void HandleCameraEnabledChanged(bool bNewEnabled);

	UFUNCTION()
	void HandleCameraModeChanged(EQuantumCameraMode NewMode);

	int32 AcquiredBroadcastCount = 0;
	int32 EnabledBroadcastCount = 0;
	int32 ModeBroadcastCount = 0;
	TArray<bool> AcquiredValues;
	TArray<bool> EnabledValues;
	TArray<EQuantumCameraMode> ModeValues;
};

/**
 * Adds Quantum Camera Component behavior to its owning actor in the Player module.
 */
UCLASS(ClassGroup=(Horror), BlueprintType, Blueprintable, meta=(BlueprintSpawnableComponent))
class HORRORPROJECT_API UQuantumCameraComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UQuantumCameraComponent();

	UFUNCTION(BlueprintCallable, Category="QuantumCamera|Runtime")
	bool SetCameraAcquired(bool bNewAcquired);

	UFUNCTION(BlueprintPure, Category="QuantumCamera|Runtime")
	bool IsCameraAcquired() const;

	UFUNCTION(BlueprintCallable, Category="QuantumCamera|Runtime")
	bool SetCameraEnabled(bool bNewEnabled);

	UFUNCTION(BlueprintPure, Category="QuantumCamera|Runtime")
	bool IsCameraEnabled() const;

	UFUNCTION(BlueprintCallable, Category="QuantumCamera|Modes")
	bool SetCameraMode(EQuantumCameraMode NewMode);

	UFUNCTION(BlueprintPure, Category="QuantumCamera|Modes")
	EQuantumCameraMode GetCameraMode() const;

	UFUNCTION(BlueprintPure, Category="QuantumCamera|Modes")
	bool IsCameraMode(EQuantumCameraMode QueryMode) const;

	UFUNCTION(BlueprintCallable, Category="QuantumCamera|Recording")
	bool StartRecording();

	UFUNCTION(BlueprintCallable, Category="QuantumCamera|Recording")
	bool StopRecording();

	UFUNCTION(BlueprintPure, Category="QuantumCamera|Battery")
	class UCameraBatteryComponent* GetBatteryComponent() const;

	UFUNCTION(BlueprintCallable, Category="QuantumCamera|Photo")
	bool TakePhoto();

	UFUNCTION(BlueprintCallable, Category="QuantumCamera|Rewind")
	bool StartRewind();

	UFUNCTION(BlueprintCallable, Category="QuantumCamera|Rewind")
	bool StopRewind();

	UPROPERTY(BlueprintAssignable, Category="QuantumCamera|Modes")
	FQuantumCameraModeChangedSignature OnCameraModeChanged;

	UPROPERTY(BlueprintAssignable, Category="QuantumCamera|Runtime")
	FQuantumCameraAcquiredChangedSignature OnCameraAcquiredChanged;

	UPROPERTY(BlueprintAssignable, Category="QuantumCamera|Runtime")
	FQuantumCameraEnabledChangedSignature OnCameraEnabledChanged;

protected:
	virtual void BeginPlay() override;

private:
	bool CanUseCamera() const;

	UFUNCTION()
	void OnBatteryDepleted();

	void FindOrCreateBatteryComponent();

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="QuantumCamera|Runtime", meta=(AllowPrivateAccess="true"))
	bool bCameraAcquired = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="QuantumCamera|Runtime", meta=(AllowPrivateAccess="true"))
	bool bCameraEnabled = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="QuantumCamera|Modes", meta=(AllowPrivateAccess="true"))
	EQuantumCameraMode CameraMode = EQuantumCameraMode::Disabled;

	UPROPERTY(Transient)
	TObjectPtr<UCameraBatteryComponent> CachedBatteryComponent = nullptr;
};
