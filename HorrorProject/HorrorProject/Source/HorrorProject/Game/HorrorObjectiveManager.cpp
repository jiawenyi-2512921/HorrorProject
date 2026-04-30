// Copyright HorrorProject. All Rights Reserved.

#include "Game/HorrorObjectiveManager.h"
#include "HorrorProject/HorrorProject.h"

bool UHorrorObjectiveManager::RegisterNode(AHorrorObjectiveNode* Node)
{
	if (!Node || Node->NodeId.IsNone())
	{
		UE_LOG(LogHorrorProject, Warning, TEXT("RegisterNode: Invalid node or empty NodeId"));
		return false;
	}

	if (NodesByIdMap.Contains(Node->NodeId))
	{
		UE_LOG(LogHorrorProject, Warning, TEXT("RegisterNode: Node '%s' already registered"), *Node->NodeId.ToString());
		return false;
	}

	RegisteredNodes.Add(Node);
	NodesByIdMap.Add(Node->NodeId, Node);
	return true;
}

bool UHorrorObjectiveManager::UnregisterNode(AHorrorObjectiveNode* Node)
{
	if (!Node)
	{
		UE_LOG(LogHorrorProject, Warning, TEXT("UnregisterNode: null Node"));
		return false;
	}

	if (!NodesByIdMap.Contains(Node->NodeId))
	{
		UE_LOG(LogHorrorProject, Warning, TEXT("UnregisterNode: Node '%s' not found"), *Node->NodeId.ToString());
		return false;
	}

	NodesByIdMap.Remove(Node->NodeId);
	return RegisteredNodes.Remove(Node) > 0;
}

bool UHorrorObjectiveManager::ActivateNode(FName NodeId)
{
	AHorrorObjectiveNode* Node = FindNodeById(NodeId);
	if (!Node)
	{
		UE_LOG(LogHorrorProject, Warning, TEXT("ActivateNode: Node '%s' not found"), *NodeId.ToString());
		return false;
	}

	if (Node->ActivateNode())
	{
		OnNodeStateChanged.Broadcast(Node, Node->NodeState);
		return true;
	}

	return false;
}

bool UHorrorObjectiveManager::CompleteNode(FName NodeId, AActor* InstigatorActor)
{
	AHorrorObjectiveNode* Node = FindNodeById(NodeId);
	if (!Node)
	{
		UE_LOG(LogHorrorProject, Warning, TEXT("CompleteNode: Node '%s' not found"), *NodeId.ToString());
		return false;
	}

	if (Node->CompleteNode(InstigatorActor))
	{
		OnNodeStateChanged.Broadcast(Node, Node->NodeState);
		ActivateNodesWithMetPrerequisites();
		return true;
	}

	return false;
}

AHorrorObjectiveNode* UHorrorObjectiveManager::FindNodeById(FName NodeId) const
{
	if (const TObjectPtr<AHorrorObjectiveNode>* FoundNode = NodesByIdMap.Find(NodeId))
	{
		return *FoundNode;
	}
	return nullptr;
}

TArray<AHorrorObjectiveNode*> UHorrorObjectiveManager::GetActiveNodes() const
{
	TArray<AHorrorObjectiveNode*> ActiveNodes;
	for (AHorrorObjectiveNode* Node : RegisteredNodes)
	{
		if (Node && Node->IsActive())
		{
			ActiveNodes.Add(Node);
		}
	}
	return ActiveNodes;
}

TArray<AHorrorObjectiveNode*> UHorrorObjectiveManager::GetNodesByType(EHorrorObjectiveNodeType NodeType) const
{
	TArray<AHorrorObjectiveNode*> FilteredNodes;
	for (AHorrorObjectiveNode* Node : RegisteredNodes)
	{
		if (Node && Node->NodeType == NodeType)
		{
			FilteredNodes.Add(Node);
		}
	}
	return FilteredNodes;
}

bool UHorrorObjectiveManager::ArePrerequisitesMet(const TArray<FName>& PrerequisiteNodeIds) const
{
	for (const FName& PrereqId : PrerequisiteNodeIds)
	{
		AHorrorObjectiveNode* PrereqNode = FindNodeById(PrereqId);
		if (!PrereqNode || !PrereqNode->IsCompleted())
		{
			return false;
		}
	}
	return true;
}

void UHorrorObjectiveManager::ActivateNodesWithMetPrerequisites()
{
	for (AHorrorObjectiveNode* Node : RegisteredNodes)
	{
		if (Node && Node->NodeState == EHorrorObjectiveNodeState::Inactive)
		{
			if (ArePrerequisitesMet(Node->PrerequisiteNodeIds))
			{
				ActivateNode(Node->NodeId);
			}
		}
	}
}
