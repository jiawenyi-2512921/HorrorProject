// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Game/HorrorObjectiveNode.h"
#include "HorrorObjectiveManager.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnObjectiveNodeStateChanged, AHorrorObjectiveNode*, Node, EHorrorObjectiveNodeState, NewState);

UCLASS()
class HORRORPROJECT_API UHorrorObjectiveManager : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category="Horror|Objectives")
	bool RegisterNode(AHorrorObjectiveNode* Node);

	UFUNCTION(BlueprintCallable, Category="Horror|Objectives")
	bool UnregisterNode(AHorrorObjectiveNode* Node);

	UFUNCTION(BlueprintCallable, Category="Horror|Objectives")
	bool ActivateNode(FName NodeId);

	UFUNCTION(BlueprintCallable, Category="Horror|Objectives")
	bool CompleteNode(FName NodeId, AActor* InstigatorActor);

	UFUNCTION(BlueprintPure, Category="Horror|Objectives")
	AHorrorObjectiveNode* FindNodeById(FName NodeId) const;

	UFUNCTION(BlueprintPure, Category="Horror|Objectives")
	TArray<AHorrorObjectiveNode*> GetActiveNodes() const;

	UFUNCTION(BlueprintPure, Category="Horror|Objectives")
	TArray<AHorrorObjectiveNode*> GetNodesByType(EHorrorObjectiveNodeType NodeType) const;

	UFUNCTION(BlueprintPure, Category="Horror|Objectives")
	bool ArePrerequisitesMet(const TArray<FName>& PrerequisiteNodeIds) const;

	UFUNCTION(BlueprintCallable, Category="Horror|Objectives")
	void ActivateNodesWithMetPrerequisites();

	UPROPERTY(BlueprintAssignable, Category="Horror|Objectives")
	FOnObjectiveNodeStateChanged OnNodeStateChanged;

	const TArray<AHorrorObjectiveNode*>& GetAllNodesForTests() const { return RegisteredNodes; }

private:
	UPROPERTY(Transient)
	TArray<TObjectPtr<AHorrorObjectiveNode>> RegisteredNodes;

	UPROPERTY(Transient)
	TMap<FName, TObjectPtr<AHorrorObjectiveNode>> NodesByIdMap;
};
