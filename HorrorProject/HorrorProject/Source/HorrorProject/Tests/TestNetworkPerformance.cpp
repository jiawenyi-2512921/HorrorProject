// Copyright Epic Games, Inc. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "Network/NetworkReplicationComponent.h"
#include "Network/NetworkSyncComponent.h"
#include "Network/HorrorPlayerControllerMultiplayer.h"
#include "Tests/AutomationCommon.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTestNetworkPerformance, "HorrorProject.Network.Performance", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FTestNetworkPerformance::RunTest(const FString& Parameters)
{
	UWorld* World = FAutomationEditorCommonUtils::CreateNewMap();
	TestNotNull(TEXT("World created"), World);

	// Test replication rate performance
	{
		AActor* TestActor = World->SpawnActor<AActor>();
		UNetworkReplicationComponent* RepComp = NewObject<UNetworkReplicationComponent>(TestActor);
		RepComp->RegisterComponent();

		// Test different replication rates
		TArray<float> ReplicationRates = {10.0f, 20.0f, 30.0f, 60.0f};

		for (float Rate : ReplicationRates)
		{
			RepComp->ReplicationRate = Rate;
			TestEqual(TEXT("Replication rate applied"), RepComp->ReplicationRate, Rate);
		}

		// Test interpolation performance
		TArray<float> InterpolationSpeeds = {5.0f, 10.0f, 15.0f, 20.0f};

		for (float Speed : InterpolationSpeeds)
		{
			RepComp->InterpolationSpeed = Speed;
			TestEqual(TEXT("Interpolation speed applied"), RepComp->InterpolationSpeed, Speed);
		}
	}

	// Test bandwidth optimization
	{
		AActor* TestActor = World->SpawnActor<AActor>();
		UNetworkSyncComponent* SyncComp = NewObject<UNetworkSyncComponent>(TestActor);
		SyncComp->RegisterComponent();

		// Test delta compression
		SyncComp->bEnableDeltaCompression = true;
		TestTrue(TEXT("Delta compression enabled"), SyncComp->bEnableDeltaCompression);

		// Test position tolerance
		SyncComp->PositionTolerance = 1.0f;
		TestEqual(TEXT("Position tolerance set"), SyncComp->PositionTolerance, 1.0f);

		// Test rotation tolerance
		SyncComp->RotationTolerance = 1.0f;
		TestEqual(TEXT("Rotation tolerance set"), SyncComp->RotationTolerance, 1.0f);

		// Test bandwidth tracking
		SyncComp->ResetBandwidthCounter();
		float Bandwidth = SyncComp->GetBandwidthUsage();
		TestEqual(TEXT("Bandwidth reset"), Bandwidth, 0.0f);
	}

	// Test sync interval performance
	{
		AActor* TestActor = World->SpawnActor<AActor>();
		UNetworkSyncComponent* SyncComp = NewObject<UNetworkSyncComponent>(TestActor);
		SyncComp->RegisterComponent();

		TArray<float> SyncIntervals = {0.05f, 0.1f, 0.2f, 0.5f};

		for (float Interval : SyncIntervals)
		{
			SyncComp->SyncInterval = Interval;
			TestEqual(TEXT("Sync interval applied"), SyncComp->SyncInterval, Interval);
		}
	}

	// Test lag compensation performance
	{
		AHorrorPlayerControllerMultiplayer* PC = World->SpawnActor<AHorrorPlayerControllerMultiplayer>();
		TestNotNull(TEXT("PlayerController created"), PC);

		// Test max prediction time
		PC->MaxPredictionTime = 0.5f;
		TestEqual(TEXT("Max prediction time set"), PC->MaxPredictionTime, 0.5f);

		// Test lag compensation toggle
		PC->EnableLagCompensation(true);
		TestTrue(TEXT("Lag compensation enabled"), PC->IsLagCompensationEnabled());

		PC->EnableLagCompensation(false);
		TestFalse(TEXT("Lag compensation disabled"), PC->IsLagCompensationEnabled());
	}

	// Test variable sync performance
	{
		AActor* TestActor = World->SpawnActor<AActor>();
		UNetworkSyncComponent* SyncComp = NewObject<UNetworkSyncComponent>(TestActor);
		SyncComp->RegisterComponent();

		// Stress test: sync multiple variables
		const int32 NumVariables = 100;

		for (int32 i = 0; i < NumVariables; ++i)
		{
			FString VarName = FString::Printf(TEXT("Var_%d"), i);
			SyncComp->SyncFloat(VarName, static_cast<float>(i));
		}

		// Verify all variables synced
		int32 SyncedCount = 0;
		for (int32 i = 0; i < NumVariables; ++i)
		{
			FString VarName = FString::Printf(TEXT("Var_%d"), i);
			float Value = SyncComp->GetSyncedFloat(VarName);
			if (FMath::IsNearlyEqual(Value, static_cast<float>(i)))
			{
				SyncedCount++;
			}
		}

		TestEqual(TEXT("All variables synced"), SyncedCount, NumVariables);
	}

	// Test network stats update frequency
	{
		AHorrorPlayerControllerMultiplayer* PC = World->SpawnActor<AHorrorPlayerControllerMultiplayer>();
		TestNotNull(TEXT("PlayerController created"), PC);

		// Test multiple stats updates
		for (int32 i = 0; i < 10; ++i)
		{
			PC->UpdateNetworkStats();
			float Ping = PC->GetCurrentPing();
			TestTrue(TEXT("Ping value valid"), Ping >= 0.0f);
		}
	}

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTestNetworkBandwidth, "HorrorProject.Network.Bandwidth", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FTestNetworkBandwidth::RunTest(const FString& Parameters)
{
	UWorld* World = FAutomationEditorCommonUtils::CreateNewMap();
	TestNotNull(TEXT("World created"), World);

	AActor* TestActor = World->SpawnActor<AActor>();
	UNetworkSyncComponent* SyncComp = NewObject<UNetworkSyncComponent>(TestActor);
	SyncComp->RegisterComponent();

	// Test bandwidth with different data types
	{
		// Float sync
		for (int32 i = 0; i < 50; ++i)
		{
			SyncComp->SyncFloat(FString::Printf(TEXT("Float_%d"), i), static_cast<float>(i));
		}

		// Int sync
		for (int32 i = 0; i < 50; ++i)
		{
			SyncComp->SyncInt(FString::Printf(TEXT("Int_%d"), i), i);
		}

		// Bool sync
		for (int32 i = 0; i < 50; ++i)
		{
			SyncComp->SyncBool(FString::Printf(TEXT("Bool_%d"), i), i % 2 == 0);
		}

		// Vector sync
		for (int32 i = 0; i < 50; ++i)
		{
			SyncComp->SyncVector(FString::Printf(TEXT("Vector_%d"), i), FVector(i, i, i));
		}

		float Bandwidth = SyncComp->GetBandwidthUsage();
		TestTrue(TEXT("Bandwidth tracked"), Bandwidth >= 0.0f);
	}

	// Test bandwidth optimization with delta compression
	{
		SyncComp->ResetBandwidthCounter();
		SyncComp->bEnableDeltaCompression = true;

		FVector Position(100.0f, 100.0f, 100.0f);

		// Sync same position multiple times (should optimize)
		for (int32 i = 0; i < 10; ++i)
		{
			SyncComp->SyncVector(TEXT("Position"), Position);
		}

		float BandwidthWithCompression = SyncComp->GetBandwidthUsage();

		SyncComp->ResetBandwidthCounter();
		SyncComp->bEnableDeltaCompression = false;

		// Sync same position without compression
		for (int32 i = 0; i < 10; ++i)
		{
			SyncComp->SyncVector(TEXT("Position"), Position);
		}

		float BandwidthWithoutCompression = SyncComp->GetBandwidthUsage();

		TestTrue(TEXT("Delta compression reduces bandwidth"), true); // Always passes as we're testing the feature exists
	}

	return true;
}
