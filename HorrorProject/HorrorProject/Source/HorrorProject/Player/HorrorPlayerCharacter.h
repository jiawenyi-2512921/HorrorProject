// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Variant_Horror/HorrorCharacter.h"
#include "Player/Components/QuantumCameraComponent.h"
#include "HorrorPlayerCharacter.generated.h"

class UEnhancedInputComponent;
class UInputAction;
class UInputComponent;
class UInteractionComponent;
class UInventoryComponent;
class UNoteRecorderComponent;
class UQuantumCameraComponent;
class UVHSEffectComponent;
class UFlashlightComponent;
class UFearComponent;
class UNoiseGeneratorComponent;
class UEvidenceCollectionComponent;
class USanityComponent;
class UEnvironmentalStoryComponent;
class UCameraRecordingComponent;
class UCameraPhotoComponent;

/**
 * Implements actor-level Horror Player Character behavior for the Player module.
 */
UCLASS()
class HORRORPROJECT_API AHorrorPlayerCharacter : public AHorrorCharacter
{
	GENERATED_BODY()

public:
	AHorrorPlayerCharacter();

	UInteractionComponent* GetInteractionComponent() const { return Interaction; }
	UInventoryComponent* GetInventoryComponent() const { return Inventory; }
	UVHSEffectComponent* GetVHSEffectComponent() const { return VHSEffect; }
	UNoteRecorderComponent* GetNoteRecorderComponent() const { return NoteRecorder; }
	UQuantumCameraComponent* GetQuantumCameraComponent() const { return QuantumCamera; }
	UCameraRecordingComponent* GetCameraRecordingComponent() const { return CameraRecording; }
	UCameraPhotoComponent* GetCameraPhotoComponent() const { return CameraPhoto; }
	UFlashlightComponent* GetFlashlightComponent() const { return Flashlight; }
	UFearComponent* GetFearComponent() const { return Fear; }
	UNoiseGeneratorComponent* GetNoiseGeneratorComponent() const { return NoiseGenerator; }
	UEvidenceCollectionComponent* GetEvidenceCollectionComponent() const { return EvidenceCollection; }
	USanityComponent* GetSanityComponent() const { return Sanity; }
	UEnvironmentalStoryComponent* GetEnvironmentalStoryComponent() const { return EnvironmentalStory; }

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input")
	TObjectPtr<UInputAction> InteractAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input")
	TObjectPtr<UInputAction> RecordAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input")
	TObjectPtr<UInputAction> TakePhotoAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input")
	TObjectPtr<UInputAction> RewindAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input")
	TObjectPtr<UInputAction> OpenArchiveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input")
	TObjectPtr<UInputAction> ToggleFlashlightAction;

	virtual void BeginPlay() override;
	virtual void PostInitializeComponents() override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	UFUNCTION(BlueprintCallable, Category="Input")
	void DoInteract();

	UFUNCTION(BlueprintCallable, Category="Input")
	void DoStartRecord();

	UFUNCTION(BlueprintCallable, Category="Input")
	void DoStopRecord();

	UFUNCTION(BlueprintCallable, Category="Input")
	void DoTakePhoto();

	UFUNCTION(BlueprintCallable, Category="Input")
	void DoStartRewind();

	UFUNCTION(BlueprintCallable, Category="Input")
	void DoStopRewind();

	UFUNCTION(BlueprintCallable, Category="Input")
	void DoOpenArchive();

	UFUNCTION(BlueprintCallable, Category="Input")
	void DoToggleFlashlight();

#if WITH_DEV_AUTOMATION_TESTS
public:
	void StartRecordForTests() { DoStartRecord(); }
	void StopRecordForTests() { DoStopRecord(); }
	void TakePhotoForTests() { DoTakePhoto(); }
	void StartRewindForTests() { DoStartRewind(); }
	void StopRewindForTests() { DoStopRewind(); }
protected:
#endif

	UFUNCTION(BlueprintImplementableEvent, Category="Input", meta=(DisplayName="打开档案"))
	void BP_OpenArchive();

private:
	using FInputActionMember = TObjectPtr<UInputAction> AHorrorPlayerCharacter::*;

	UInputAction* ResolveInteractAction();
	UInputAction* ResolveInputAction(FInputActionMember ActionMember, const TCHAR* AssetPath, const TCHAR* DebugName);
	void BindQuantumCameraDelegates();
	void RebindInteractStarted(UEnhancedInputComponent* EnhancedInputComponent, const UInputAction* Action);

	UFUNCTION()
	void HandleQuantumCameraModeChanged(EQuantumCameraMode NewMode);

	UFUNCTION()
	void HandleQuantumCameraAcquiredChanged(bool bNewAcquired);

	UFUNCTION()
	void HandleQuantumCameraEnabledChanged(bool bNewEnabled);

	uint32 InteractStartedBindingHandle = 0;
	uint32 RecordStartedBindingHandle = 0;
	uint32 RecordCompletedBindingHandle = 0;
	uint32 RecordCanceledBindingHandle = 0;
	uint32 TakePhotoStartedBindingHandle = 0;
	uint32 RewindStartedBindingHandle = 0;
	uint32 RewindCompletedBindingHandle = 0;
	uint32 RewindCanceledBindingHandle = 0;
	uint32 OpenArchiveStartedBindingHandle = 0;
	uint32 ToggleFlashlightStartedBindingHandle = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UInteractionComponent> Interaction;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UInventoryComponent> Inventory;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UVHSEffectComponent> VHSEffect;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UNoteRecorderComponent> NoteRecorder;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UQuantumCameraComponent> QuantumCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UCameraRecordingComponent> CameraRecording;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UCameraPhotoComponent> CameraPhoto;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UFlashlightComponent> Flashlight;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UFearComponent> Fear;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UNoiseGeneratorComponent> NoiseGenerator;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UEvidenceCollectionComponent> EvidenceCollection;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	TObjectPtr<USanityComponent> Sanity;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UEnvironmentalStoryComponent> EnvironmentalStory;
};
