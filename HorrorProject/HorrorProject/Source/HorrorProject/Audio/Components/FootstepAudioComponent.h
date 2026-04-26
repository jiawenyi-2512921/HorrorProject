// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FootstepAudioComponent.generated.h"

UENUM(BlueprintType)
enum class EFootstepSurfaceType : uint8
{
	Concrete UMETA(DisplayName="Concrete"),
	Wood UMETA(DisplayName="Wood"),
	Metal UMETA(DisplayName="Metal"),
	Grass UMETA(DisplayName="Grass"),
	Water UMETA(DisplayName="Water"),
	Gravel UMETA(DisplayName="Gravel"),
	Carpet UMETA(DisplayName="Carpet"),
	Mud UMETA(DisplayName="Mud")
};

USTRUCT(BlueprintType)
struct HORRORPROJECT_API FFootstepSoundSet
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Footstep")
	TArray<TObjectPtr<USoundBase>> WalkSounds;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Footstep")
	TArray<TObjectPtr<USoundBase>> RunSounds;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Footstep")
	TArray<TObjectPtr<USoundBase>> CrouchSounds;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Footstep")
	TArray<TObjectPtr<USoundBase>> JumpSounds;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Footstep")
	TArray<TObjectPtr<USoundBase>> LandSounds;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Footstep", meta=(ClampMin="0.0", ClampMax="1.0"))
	float BaseVolume = 0.7f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Footstep", meta=(ClampMin="0.0", ClampMax="2.0"))
	float PitchVariation = 0.1f;
};

UCLASS(ClassGroup=(Audio), meta=(BlueprintSpawnableComponent))
class HORRORPROJECT_API UFootstepAudioComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UFootstepAudioComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, Category="Footstep Audio")
	void PlayFootstep(bool bIsLeftFoot);

	UFUNCTION(BlueprintCallable, Category="Footstep Audio")
	void PlayJumpSound();

	UFUNCTION(BlueprintCallable, Category="Footstep Audio")
	void PlayLandSound(float ImpactIntensity);

	UFUNCTION(BlueprintCallable, Category="Footstep Audio")
	void SetMovementSpeed(float Speed);

	UFUNCTION(BlueprintCallable, Category="Footstep Audio")
	void SetCrouching(bool bIsCrouching);

	UFUNCTION(BlueprintCallable, Category="Footstep Audio")
	void RegisterSurfaceType(EFootstepSurfaceType SurfaceType, const FFootstepSoundSet& SoundSet);

	UFUNCTION(BlueprintCallable, Category="Footstep Audio")
	void SetCurrentSurface(EFootstepSurfaceType SurfaceType);

	UFUNCTION(BlueprintCallable, Category="Footstep Audio")
	EFootstepSurfaceType DetectSurfaceType();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Footstep Audio")
	TMap<EFootstepSurfaceType, FFootstepSoundSet> SurfaceSounds;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Footstep Audio")
	EFootstepSurfaceType CurrentSurface = EFootstepSurfaceType::Concrete;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Footstep Audio")
	float WalkSpeedThreshold = 200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Footstep Audio")
	float RunSpeedThreshold = 400.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Footstep Audio")
	float FootstepInterval = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Footstep Audio")
	float RunFootstepInterval = 0.3f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Footstep Audio")
	float CrouchFootstepInterval = 0.7f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Footstep Audio")
	bool bAutoPlayFootsteps = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Footstep Audio")
	float TraceDistance = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Footstep Audio")
	float VolumeMultiplierWalk = 0.7f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Footstep Audio")
	float VolumeMultiplierRun = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Footstep Audio")
	float VolumeMultiplierCrouch = 0.4f;

private:
	float CurrentSpeed = 0.0f;
	bool bIsCrouching = false;
	float TimeSinceLastFootstep = 0.0f;
	bool bIsLeftFoot = true;

	void PlaySound(const TArray<TObjectPtr<USoundBase>>& Sounds, float VolumeMultiplier);
	float GetCurrentFootstepInterval() const;
	float GetCurrentVolumeMultiplier() const;
};
