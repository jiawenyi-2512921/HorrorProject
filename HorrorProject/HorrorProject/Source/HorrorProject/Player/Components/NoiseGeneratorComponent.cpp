// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/Components/NoiseGeneratorComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Perception/AISense_Hearing.h"

UNoiseGeneratorComponent::UNoiseGeneratorComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
}

void UNoiseGeneratorComponent::BeginPlay()
{
	Super::BeginPlay();
	CurrentNoiseLevel = 0.0f;
	FootstepTimer = 0.0f;
	LastLocation = GetOwnerLocation();
}

void UNoiseGeneratorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bAutoGenerateFootsteps)
	{
		UpdateFootstepGeneration(DeltaTime);
	}

	UpdateNoiseDecay(DeltaTime);
}

bool UNoiseGeneratorComponent::GenerateNoise(ENoiseType NoiseType, float LoudnessMultiplier, FVector Location)
{
	float BaseLoudness = 0.0f;

	switch (NoiseType)
	{
	case ENoiseType::Footstep:
		BaseLoudness = GetFootstepLoudness();
		break;
	case ENoiseType::Sprint:
		BaseLoudness = FootstepBaseLoudness * SprintLoudnessMultiplier;
		break;
	case ENoiseType::Jump:
		BaseLoudness = JumpLoudness;
		break;
	case ENoiseType::Land:
		BaseLoudness = LandLoudness;
		break;
	case ENoiseType::Interaction:
		BaseLoudness = InteractionLoudness;
		break;
	case ENoiseType::Custom:
		BaseLoudness = 1.0f;
		break;
	}

	const float FinalLoudness = BaseLoudness * LoudnessMultiplier;
	const FVector NoiseLocation = Location.IsZero() ? GetOwnerLocation() : Location;

	CurrentNoiseLevel = FMath::Max(CurrentNoiseLevel, FinalLoudness);

	if (bUseAINoiseSystem)
	{
		MakeAINoise(FinalLoudness, NoiseLocation);
	}

	OnNoiseGenerated.Broadcast(NoiseType, FinalLoudness, NoiseLocation);

	return true;
}

bool UNoiseGeneratorComponent::GenerateNoiseAtLocation(float Loudness, FVector Location, FName Tag)
{
	if (Loudness <= 0.0f)
	{
		return false;
	}

	CurrentNoiseLevel = FMath::Max(CurrentNoiseLevel, Loudness);

	if (bUseAINoiseSystem)
	{
		MakeAINoise(Loudness, Location);
	}

	OnNoiseGenerated.Broadcast(ENoiseType::Custom, Loudness, Location);

	return true;
}

void UNoiseGeneratorComponent::SetSprinting(bool bSprinting)
{
	if (bIsSprinting != bSprinting)
	{
		bIsSprinting = bSprinting;
		FootstepTimer = 0.0f;
	}
}

float UNoiseGeneratorComponent::GetFootstepLoudness() const
{
	const float BaseLoudness = FootstepBaseLoudness;
	return bIsSprinting ? BaseLoudness * SprintLoudnessMultiplier : BaseLoudness;
}

void UNoiseGeneratorComponent::UpdateFootstepGeneration(float DeltaTime)
{
	if (!IsMoving())
	{
		FootstepTimer = 0.0f;
		return;
	}

	FootstepTimer += DeltaTime;

	const float CurrentInterval = bIsSprinting ? SprintFootstepInterval : FootstepInterval;
	if (FootstepTimer >= CurrentInterval)
	{
		FootstepTimer = 0.0f;
		GenerateNoise(ENoiseType::Footstep);
	}
}

void UNoiseGeneratorComponent::UpdateNoiseDecay(float DeltaTime)
{
	if (CurrentNoiseLevel > 0.0f)
	{
		CurrentNoiseLevel = FMath::Max(0.0f, CurrentNoiseLevel - (NoiseDecayRate * DeltaTime));
	}
}

void UNoiseGeneratorComponent::MakeAINoise(float Loudness, const FVector& Location)
{
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	const float Range = Loudness * NoiseDetectionRadius;
	UAISense_Hearing::ReportNoiseEvent(
		GetWorld(),
		Location,
		Loudness,
		Owner,
		Range,
		NAME_None
	);
}

bool UNoiseGeneratorComponent::IsMoving() const
{
	const ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
	if (!OwnerCharacter || !OwnerCharacter->GetCharacterMovement())
	{
		return false;
	}

	return !OwnerCharacter->GetCharacterMovement()->Velocity.IsNearlyZero();
}

FVector UNoiseGeneratorComponent::GetOwnerLocation() const
{
	const AActor* Owner = GetOwner();
	return Owner ? Owner->GetActorLocation() : FVector::ZeroVector;
}
