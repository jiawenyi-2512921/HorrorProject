#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR && HORRORPROJECT_ENABLE_LEGACY_AUTOMATION_TESTS

// Copyright Epic Games, Inc. All Rights Reserved.

#include "Game/HorrorObjectiveNode.h"
#include "Game/HorrorNavigationNode.h"
#include "Game/HorrorInteractionNode.h"
#include "Game/HorrorEncounterNode.h"
#include "Game/HorrorObjectiveManager.h"
#include "Components/BoxComponent.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHorrorObjectiveNodeBasicTest, "HorrorProject.Game.ObjectiveNode.Basic", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorObjectiveNodeBasicTest::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	FWorldContext& WorldContext = GEngine->CreateNewWorldContext(EWorldType::Game);
	WorldContext.SetCurrentWorld(World);

	AHorrorObjectiveNode* Node = World->SpawnActor<AHorrorObjectiveNode>();
	Node->NodeId = TEXT("TestNode");
	Node->NodeType = EHorrorObjectiveNodeType::Navigation;

	TestEqual("Initial state is Inactive", Node->NodeState, EHorrorObjectiveNodeState::Inactive);
	TestFalse("Node is not active initially", Node->IsActive());
	TestFalse("Node is not completed initially", Node->IsCompleted());

	TestTrue("Can activate node", Node->CanActivate());
	TestTrue("Activate node succeeds", Node->ActivateNode());
	TestEqual("State is Active after activation", Node->NodeState, EHorrorObjectiveNodeState::Active);
	TestTrue("Node is active", Node->IsActive());

	AActor* Instigator = World->SpawnActor<AActor>();
	TestTrue("Can complete node", Node->CanComplete(Instigator));
	TestTrue("Complete node succeeds", Node->CompleteNode(Instigator));
	TestEqual("State is Completed", Node->NodeState, EHorrorObjectiveNodeState::Completed);
	TestTrue("Node is completed", Node->IsCompleted());

	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(false);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHorrorNavigationNodeTest, "HorrorProject.Game.ObjectiveNode.Navigation", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorNavigationNodeTest::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	FWorldContext& WorldContext = GEngine->CreateNewWorldContext(EWorldType::Game);
	WorldContext.SetCurrentWorld(World);

	AHorrorNavigationNode* NavNode = World->SpawnActor<AHorrorNavigationNode>();
	NavNode->NodeId = TEXT("NavNode01");
	NavNode->ActivationRadius = 500.0f;
	NavNode->bAutoCompleteOnOverlap = true;

	TestEqual("Node type is Navigation", NavNode->NodeType, EHorrorObjectiveNodeType::Navigation);
	TestTrue("Activate navigation node", NavNode->ActivateNode());

	TestNotNull("Trigger volume exists", NavNode->TriggerVolume);
	FVector Extent = NavNode->TriggerVolume->GetScaledBoxExtent();
	TestEqual("Trigger volume extent matches radius", Extent.X, 500.0f, 0.1f);

	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(false);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHorrorInteractionNodeTest, "HorrorProject.Game.ObjectiveNode.Interaction", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorInteractionNodeTest::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	FWorldContext& WorldContext = GEngine->CreateNewWorldContext(EWorldType::Game);
	WorldContext.SetCurrentWorld(World);

	AHorrorInteractionNode* InteractNode = World->SpawnActor<AHorrorInteractionNode>();
	InteractNode->NodeId = TEXT("InteractNode01");
	InteractNode->InteractionPrompt = FText::FromString(TEXT("Press E to interact"));

	TestEqual("Node type is Interaction", InteractNode->NodeType, EHorrorObjectiveNodeType::Interaction);

	AActor* Instigator = World->SpawnActor<AActor>();
	FHitResult Hit;

	TestFalse("Cannot interact when inactive", InteractNode->CanInteract_Implementation(Instigator, Hit));

	InteractNode->ActivateNode();
	TestTrue("Can interact when active", InteractNode->CanInteract_Implementation(Instigator, Hit));

	TestTrue("Interact succeeds", InteractNode->Interact_Implementation(Instigator, Hit));
	TestTrue("Node is completed after interaction", InteractNode->IsCompleted());

	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(false);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHorrorEncounterNodeTest, "HorrorProject.Game.ObjectiveNode.Encounter", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorEncounterNodeTest::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	FWorldContext& WorldContext = GEngine->CreateNewWorldContext(EWorldType::Game);
	WorldContext.SetCurrentWorld(World);

	AHorrorEncounterNode* EncounterNode = World->SpawnActor<AHorrorEncounterNode>();
	EncounterNode->NodeId = TEXT("EncounterNode01");
	EncounterNode->TriggerRadius = 1000.0f;
	EncounterNode->bAutoCompleteOnResolve = true;

	TestEqual("Node type is Encounter", EncounterNode->NodeType, EHorrorObjectiveNodeType::Encounter);
	TestFalse("Encounter is not active initially", EncounterNode->IsEncounterActive());

	EncounterNode->ActivateNode();
	TestTrue("Node is active", EncounterNode->IsActive());

	AActor* Instigator = World->SpawnActor<AActor>();
	TestTrue("Trigger encounter succeeds", EncounterNode->TriggerEncounter(Instigator));
	TestTrue("Encounter is active", EncounterNode->IsEncounterActive());

	TestTrue("Resolve encounter succeeds", EncounterNode->ResolveEncounter());
	TestFalse("Encounter is not active after resolve", EncounterNode->IsEncounterActive());
	TestTrue("Node is completed after resolve", EncounterNode->IsCompleted());

	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(false);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHorrorObjectiveManagerTest, "HorrorProject.Game.ObjectiveManager.Basic", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorObjectiveManagerTest::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	FWorldContext& WorldContext = GEngine->CreateNewWorldContext(EWorldType::Game);
	WorldContext.SetCurrentWorld(World);

	UHorrorObjectiveManager* Manager = World->GetSubsystem<UHorrorObjectiveManager>();
	TestNotNull("Manager exists", Manager);

	AHorrorObjectiveNode* Node1 = World->SpawnActor<AHorrorObjectiveNode>();
	Node1->NodeId = TEXT("Node01");
	Node1->NodeType = EHorrorObjectiveNodeType::Navigation;

	AHorrorObjectiveNode* Node2 = World->SpawnActor<AHorrorObjectiveNode>();
	Node2->NodeId = TEXT("Node02");
	Node2->NodeType = EHorrorObjectiveNodeType::Interaction;
	Node2->PrerequisiteNodeIds.Add(TEXT("Node01"));

	TestTrue("Register Node1", Manager->RegisterNode(Node1));
	TestTrue("Register Node2", Manager->RegisterNode(Node2));
	TestEqual("Two nodes registered", Manager->GetAllNodesForTests().Num(), 2);

	AHorrorObjectiveNode* FoundNode = Manager->FindNodeById(TEXT("Node01"));
	TestNotNull("Find Node1 by ID", FoundNode);
	TestEqual("Found node matches", FoundNode, Node1);

	TestTrue("Activate Node1", Manager->ActivateNode(TEXT("Node01")));
	TestEqual("One active node", Manager->GetActiveNodes().Num(), 1);

	TestFalse("Node2 prerequisites not met", Manager->ArePrerequisitesMet(Node2->PrerequisiteNodeIds));

	AActor* Instigator = World->SpawnActor<AActor>();
	TestTrue("Complete Node1", Manager->CompleteNode(TEXT("Node01"), Instigator));
	TestTrue("Node2 prerequisites met", Manager->ArePrerequisitesMet(Node2->PrerequisiteNodeIds));

	Manager->ActivateNodesWithMetPrerequisites();
	TestTrue("Node2 is now active", Node2->IsActive());

	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(false);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHorrorObjectiveManagerNodeTypesTest, "HorrorProject.Game.ObjectiveManager.NodeTypes", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorObjectiveManagerNodeTypesTest::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	FWorldContext& WorldContext = GEngine->CreateNewWorldContext(EWorldType::Game);
	WorldContext.SetCurrentWorld(World);

	UHorrorObjectiveManager* Manager = World->GetSubsystem<UHorrorObjectiveManager>();

	AHorrorNavigationNode* NavNode = World->SpawnActor<AHorrorNavigationNode>();
	NavNode->NodeId = TEXT("Nav01");

	AHorrorInteractionNode* InteractNode = World->SpawnActor<AHorrorInteractionNode>();
	InteractNode->NodeId = TEXT("Interact01");

	AHorrorEncounterNode* EncounterNode = World->SpawnActor<AHorrorEncounterNode>();
	EncounterNode->NodeId = TEXT("Encounter01");

	Manager->RegisterNode(NavNode);
	Manager->RegisterNode(InteractNode);
	Manager->RegisterNode(EncounterNode);

	TArray<AHorrorObjectiveNode*> NavNodes = Manager->GetNodesByType(EHorrorObjectiveNodeType::Navigation);
	TestEqual("One navigation node", NavNodes.Num(), 1);

	TArray<AHorrorObjectiveNode*> InteractNodes = Manager->GetNodesByType(EHorrorObjectiveNodeType::Interaction);
	TestEqual("One interaction node", InteractNodes.Num(), 1);

	TArray<AHorrorObjectiveNode*> EncounterNodes = Manager->GetNodesByType(EHorrorObjectiveNodeType::Encounter);
	TestEqual("One encounter node", EncounterNodes.Num(), 1);

	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(false);
	return true;
}

#endif

#endif // WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR
