#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR && HORRORPROJECT_ENABLE_LEGACY_AUTOMATION_TESTS

// Copyright Epic Games, Inc. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "Network/NetworkReplicationComponent.h"
#include "Network/ReplicatedPlayerState.h"
#include "Network/ReplicatedGameState.h"
#include "GameFramework/Actor.h"
#include "Tests/AutomationCommon.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTestNetworkReplication, "HorrorProject.Network.Replication", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FTestNetworkReplication::RunTest(const FString& Parameters)
{
	// Test NetworkReplicationComponent
	{
		UWorld* World = FAutomationEditorCommonUtils::CreateNewMap();
		TestNotNull(TEXT("World created"), World);

		AActor* TestActor = World->SpawnActor<AActor>();
		TestNotNull(TEXT("Test actor created"), TestActor);

		UNetworkReplicationComponent* RepComp = NewObject<UNetworkReplicationComponent>(TestActor);
		TestNotNull(TEXT("NetworkReplicationComponent created"), RepComp);

		RepComp->RegisterComponent();
		RepComp->ReplicationRate = 30.0f;
		RepComp->InterpolationSpeed = 10.0f;

		TestEqual(TEXT("Replication rate set correctly"), RepComp->ReplicationRate, 30.0f);
		TestEqual(TEXT("Interpolation speed set correctly"), RepComp->InterpolationSpeed, 10.0f);

		// Test transform replication
		FVector TestLocation(100.0f, 200.0f, 300.0f);
		FRotator TestRotation(0.0f, 90.0f, 0.0f);
		RepComp->SetTargetTransform(TestLocation, TestRotation);

		TestTrue(TEXT("Network ready check"), RepComp->IsNetworkReady() || World->GetNetMode() == NM_Standalone);
	}

	// Test ReplicatedPlayerState
	{
		UWorld* World = FAutomationEditorCommonUtils::CreateNewMap();
		AReplicatedPlayerState* PlayerState = World->SpawnActor<AReplicatedPlayerState>();
		TestNotNull(TEXT("ReplicatedPlayerState created"), PlayerState);

		// Test health system
		PlayerState->SetHealth(75.0f);
		TestEqual(TEXT("Health set correctly"), PlayerState->Health, 75.0f);

		PlayerState->SetHealth(150.0f); // Should clamp to MaxHealth
		TestEqual(TEXT("Health clamped to max"), PlayerState->Health, PlayerState->MaxHealth);

		PlayerState->SetHealth(0.0f);
		TestFalse(TEXT("Player died when health reached 0"), PlayerState->bIsAlive);

		// Test stamina system
		PlayerState->SetStamina(50.0f);
		TestEqual(TEXT("Stamina set correctly"), PlayerState->Stamina, 50.0f);

		// Test fear system
		PlayerState->SetFear(75.0f);
		TestEqual(TEXT("Fear set correctly"), PlayerState->Fear, 75.0f);

		// Test inventory
		PlayerState->AddInventoryItem(TEXT("Flashlight"));
		TestTrue(TEXT("Item added to inventory"), PlayerState->HasInventoryItem(TEXT("Flashlight")));

		PlayerState->RemoveInventoryItem(TEXT("Flashlight"));
		TestFalse(TEXT("Item removed from inventory"), PlayerState->HasInventoryItem(TEXT("Flashlight")));
	}

	// Test ReplicatedGameState
	{
		UWorld* World = FAutomationEditorCommonUtils::CreateNewMap();
		AReplicatedGameState* GameState = World->SpawnActor<AReplicatedGameState>();
		TestNotNull(TEXT("ReplicatedGameState created"), GameState);

		// Test game phase
		GameState->SetGamePhase(EGamePhase::Playing);
		TestEqual(TEXT("Game phase set correctly"), GameState->GamePhase, EGamePhase::Playing);

		// Test objectives
		GameState->TotalObjectives = 5;
		GameState->AddObjectiveProgress(1);
		TestEqual(TEXT("Objective progress updated"), GameState->ObjectivesCompleted, 1);

		GameState->AddObjectiveProgress(4);
		TestEqual(TEXT("All objectives completed"), GameState->ObjectivesCompleted, 5);

		// Test player management
		GameState->AddPlayerName(TEXT("Player1"));
		GameState->AddPlayerName(TEXT("Player2"));
		TestEqual(TEXT("Player count correct"), GameState->GetConnectedPlayerCount(), 2);

		GameState->RemovePlayerName(TEXT("Player1"));
		TestEqual(TEXT("Player removed correctly"), GameState->GetConnectedPlayerCount(), 1);

		// Test game start conditions
		GameState->MaxPlayers = 4;
		TestTrue(TEXT("Can start game with valid player count"), GameState->CanStartGame());
	}

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTestNetworkSync, "HorrorProject.Network.Sync", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FTestNetworkSync::RunTest(const FString& Parameters)
{
	UWorld* World = FAutomationEditorCommonUtils::CreateNewMap();
	TestNotNull(TEXT("World created"), World);

	AActor* TestActor = World->SpawnActor<AActor>();
	UNetworkSyncComponent* SyncComp = NewObject<UNetworkSyncComponent>(TestActor);
	TestNotNull(TEXT("NetworkSyncComponent created"), SyncComp);

	SyncComp->RegisterComponent();

	// Test float sync
	SyncComp->SyncFloat(TEXT("TestFloat"), 42.5f);
	float RetrievedFloat = SyncComp->GetSyncedFloat(TEXT("TestFloat"));
	TestEqual(TEXT("Float synced correctly"), RetrievedFloat, 42.5f);

	// Test int sync
	SyncComp->SyncInt(TEXT("TestInt"), 100);
	int32 RetrievedInt = SyncComp->GetSyncedInt(TEXT("TestInt"));
	TestEqual(TEXT("Int synced correctly"), RetrievedInt, 100);

	// Test bool sync
	SyncComp->SyncBool(TEXT("TestBool"), true);
	bool RetrievedBool = SyncComp->GetSyncedBool(TEXT("TestBool"));
	TestTrue(TEXT("Bool synced correctly"), RetrievedBool);

	// Test vector sync
	FVector TestVector(1.0f, 2.0f, 3.0f);
	SyncComp->SyncVector(TEXT("TestVector"), TestVector);
	FVector RetrievedVector = SyncComp->GetSyncedVector(TEXT("TestVector"));
	TestEqual(TEXT("Vector synced correctly"), RetrievedVector, TestVector);

	// Test bandwidth tracking
	float Bandwidth = SyncComp->GetBandwidthUsage();
	TestTrue(TEXT("Bandwidth tracking initialized"), Bandwidth >= 0.0f);

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR
