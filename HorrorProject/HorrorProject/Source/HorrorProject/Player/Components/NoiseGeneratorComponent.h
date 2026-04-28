// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NoiseGeneratorComponent.generated.h"

UENUM(BlueprintType)
enum class ENoiseType : uint8
{
	Footstep UMETA(DisplayName="脚步"),
	Sprint UMETA(DisplayName="奔跑"),
	Jump UMETA(DisplayName="跳跃"),
	Land UMETA(DisplayName="落地"),
	Interaction UMETA(DisplayName="互动"),
	Custom UMETA(DisplayName="自定义")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FNoiseGeneratedSignature, ENoiseType, NoiseType, float, Loudness, FVector, Location);

/**
 * Adds Noise Generator Component behavior to its owning actor in the Player module.
 */
UCLASS(ClassGroup=(Horror), BlueprintType, Blueprintable, meta=(BlueprintSpawnableComponent))
class HORRORPROJECT_API UNoiseGeneratorComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UNoiseGeneratorComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, Category="Noise")
	bool GenerateNoise(ENoiseType NoiseType, float LoudnessMultiplier = 1.0f, FVector Location = FVector::ZeroVector);

	UFUNCTION(BlueprintCallable, Category="Noise")
	bool GenerateNoiseAtLocation(float Loudness, FVector Location, FName Tag = NAME_None);

	UFUNCTION(BlueprintPure, Category="Noise")
	float GetCurrentNoiseLevel() const { return CurrentNoiseLevel; }

	UFUNCTION(BlueprintPure, Category="Noise")
	bool IsSprinting() const { return bIsSprinting; }

	UFUNCTION(BlueprintCallable, Category="Noise")
	void SetSprinting(bool bSprinting);

	UFUNCTION(BlueprintPure, Category="Noise")
	float GetFootstepLoudness() const;

	UPROPERTY(BlueprintAssignable, Category="Noise")
	FNoiseGeneratedSignature OnNoiseGenerated;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Noise|Footsteps", meta=(ClampMin="0.0", ClampMax="1.0"))
	float FootstepBaseLoudness = 0.3f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Noise|Footsteps", meta=(ClampMin="0.0", ClampMax="10.0"))
	float SprintLoudnessMultiplier = 2.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Noise|Footsteps", meta=(ClampMin="0.0", Units="s"))
	float FootstepInterval = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Noise|Footsteps", meta=(ClampMin="0.0", Units="s"))
	float SprintFootstepInterval = 0.3f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Noise|Actions", meta=(ClampMin="0.0", ClampMax="1.0"))
	float JumpLoudness = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Noise|Actions", meta=(ClampMin="0.0", ClampMax="1.0"))
	float LandLoudness = 0.6f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Noise|Actions", meta=(ClampMin="0.0", ClampMax="1.0"))
	float InteractionLoudness = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Noise|Detection", meta=(ClampMin="0.0", Units="cm"))
	float NoiseDetectionRadius = 1000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Noise|Detection")
	bool bUseAINoiseSystem = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Noise|Detection")
	bool bAutoGenerateFootsteps = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Noise|Detection", meta=(ClampMin="0.0", ClampMax="1.0"))
	float NoiseDecayRate = 2.0f;

private:
	void UpdateFootstepGeneration(float DeltaTime);
	void UpdateNoiseDecay(float DeltaTime);
	void MakeAINoise(float Loudness, const FVector& Location);
	bool IsMoving() const;
	FVector GetOwnerLocation() const;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Noise", meta=(AllowPrivateAccess="true"))
	float CurrentNoiseLevel = 0.0f;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Noise", meta=(AllowPrivateAccess="true"))
	bool bIsSprinting = false;

	float FootstepTimer = 0.0f;
	FVector LastLocation = FVector::ZeroVector;
};
