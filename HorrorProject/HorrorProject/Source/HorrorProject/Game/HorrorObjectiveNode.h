// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "HorrorObjectiveNode.generated.h"

class UBoxComponent;
class UHorrorEventBusSubsystem;

UENUM(BlueprintType)
enum class EHorrorObjectiveNodeType : uint8
{
	Navigation UMETA(DisplayName="Navigation Node"),
	Interaction UMETA(DisplayName="Interaction Node"),
	Encounter UMETA(DisplayName="Encounter Node")
};

UENUM(BlueprintType)
enum class EHorrorObjectiveNodeState : uint8
{
	Inactive UMETA(DisplayName="Inactive"),
	Active UMETA(DisplayName="Active"),
	Completed UMETA(DisplayName="Completed"),
	Failed UMETA(DisplayName="Failed")
};

/**
 * Implements actor-level Horror Objective Node behavior for the Game module.
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup=(Horror))
class HORRORPROJECT_API AHorrorObjectiveNode : public AActor
{
	GENERATED_BODY()

public:
	AHorrorObjectiveNode();

	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category="Horror|Objectives")
	bool ActivateNode();

	UFUNCTION(BlueprintCallable, Category="Horror|Objectives")
	bool CompleteNode(AActor* InstigatorActor);

	UFUNCTION(BlueprintCallable, Category="Horror|Objectives")
	bool FailNode();

	UFUNCTION(BlueprintPure, Category="Horror|Objectives")
	bool IsActive() const { return NodeState == EHorrorObjectiveNodeState::Active; }

	UFUNCTION(BlueprintPure, Category="Horror|Objectives")
	bool IsCompleted() const { return NodeState == EHorrorObjectiveNodeState::Completed; }

	UFUNCTION(BlueprintPure, Category="Horror|Objectives")
	bool CanActivate() const;

	UFUNCTION(BlueprintPure, Category="Horror|Objectives")
	bool CanComplete(AActor* InstigatorActor) const;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Objectives")
	EHorrorObjectiveNodeType NodeType = EHorrorObjectiveNodeType::Navigation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Objectives")
	FName NodeId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Objectives")
	FText NodeTitle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Objectives")
	FText NodeDescription;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Objectives")
	FGameplayTag EventTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Objectives")
	FGameplayTag StateTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Objectives")
	TArray<FName> PrerequisiteNodeIds;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Objectives")
	bool bAutoActivateOnBeginPlay = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Objectives")
	bool bPublishEventOnActivate = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Objectives")
	bool bPublishEventOnComplete = true;

	UPROPERTY(BlueprintReadOnly, Category="Horror|Objectives")
	EHorrorObjectiveNodeState NodeState = EHorrorObjectiveNodeState::Inactive;

protected:
	UFUNCTION(BlueprintNativeEvent, Category="Horror|Objectives")
	void OnNodeActivated();

	UFUNCTION(BlueprintNativeEvent, Category="Horror|Objectives")
	void OnNodeCompleted(AActor* InstigatorActor);

	UFUNCTION(BlueprintNativeEvent, Category="Horror|Objectives")
	void OnNodeFailed();

	virtual void OnNodeActivated_Implementation();
	virtual void OnNodeCompleted_Implementation(AActor* InstigatorActor);
	virtual void OnNodeFailed_Implementation();

	UHorrorEventBusSubsystem* GetEventBus() const;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Horror|Objectives")
	TObjectPtr<UBoxComponent> TriggerVolume;
};
