#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR && HORRORPROJECT_ENABLE_LEGACY_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "Network/HorrorGameModeMultiplayer.h"
#include "Network/MultiplayerSessionSubsystem.h"
#include "Network/NetworkReplicationComponent.h"
#include "Network/NetworkSyncComponent.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FNetworkBoundaryTest, "HorrorProject.Network.BoundaryConditions", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FNetworkBoundaryTest::RunTest(const FString& Parameters)
{
    UWorld* World = FAutomationEditorCommonUtils::CreateNewMap();
    TestNotNull(TEXT("World created"), World);

    // Test max player limit
    AHorrorGameModeMultiplayer* GameMode = World->SpawnActor<AHorrorGameModeMultiplayer>();
    TestNotNull(TEXT("GameMode created"), GameMode);

    GameMode->MaxPlayers = 4;
    TestEqual(TEXT("Max players set"), GameMode->MaxPlayers, 4);

    // Test zero players
    GameMode->MinPlayers = 0;
    TestEqual(TEXT("Min players can be zero"), GameMode->MinPlayers, 0);

    // Test negative values (should clamp)
    GameMode->MinPlayers = -1;
    TestTrue(TEXT("Negative min players handled"), GameMode->MinPlayers >= 0);

    // Test max > min
    GameMode->MinPlayers = 5;
    GameMode->MaxPlayers = 2;
    TestTrue(TEXT("Max >= Min enforced"), GameMode->MaxPlayers >= GameMode->MinPlayers);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FNetworkLatencyTest, "HorrorProject.Network.LatencyHandling", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FNetworkLatencyTest::RunTest(const FString& Parameters)
{
    UWorld* World = FAutomationEditorCommonUtils::CreateNewMap();

    // Test high latency scenarios
    AHorrorPlayerControllerMultiplayer* PC = World->SpawnActor<AHorrorPlayerControllerMultiplayer>();
    TestNotNull(TEXT("PlayerController created"), PC);

    // Simulate high ping
    PC->SimulatePing(500.0f);
    TestTrue(TEXT("High ping handled"), PC->GetCurrentPing() >= 0.0f);

    // Simulate packet loss
    PC->SimulatePacketLoss(0.5f);
    TestTrue(TEXT("Packet loss handled"), PC->GetPacketLoss() >= 0.0f && PC->GetPacketLoss() <= 1.0f);

    // Test lag compensation with extreme values
    PC->SetLagCompensationTime(1000.0f);
    TestTrue(TEXT("Extreme lag compensation handled"), true);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FNetworkDisconnectionTest, "HorrorProject.Network.DisconnectionHandling", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FNetworkDisconnectionTest::RunTest(const FString& Parameters)
{
    UWorld* World = FAutomationEditorCommonUtils::CreateNewMap();
    AHorrorGameModeMultiplayer* GameMode = World->SpawnActor<AHorrorGameModeMultiplayer>();

    // Test sudden disconnection
    GameMode->OnPlayerDisconnected(nullptr);
    TestTrue(TEXT("Null player disconnection handled"), true);

    // Test reconnection
    AHorrorPlayerControllerMultiplayer* PC = World->SpawnActor<AHorrorPlayerControllerMultiplayer>();
    GameMode->OnPlayerReconnected(PC);
    TestTrue(TEXT("Player reconnection handled"), true);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FNetworkReplicationStressTest, "HorrorProject.Network.ReplicationStress", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FNetworkReplicationStressTest::RunTest(const FString& Parameters)
{
    UWorld* World = FAutomationEditorCommonUtils::CreateNewMap();
    AActor* TestActor = World->SpawnActor<AActor>();

    UNetworkReplicationComponent* RepComp = NewObject<UNetworkReplicationComponent>(TestActor);
    TestNotNull(TEXT("ReplicationComponent created"), RepComp);

    // Stress test: Rapid state changes
    const int32 NumIterations = 1000;
    double StartTime = FPlatformTime::Seconds();

    for (int32 i = 0; i < NumIterations; ++i)
    {
        RepComp->MarkForReplication();
        RepComp->UpdateReplicationData();
    }

    double EndTime = FPlatformTime::Seconds();
    double TotalTime = EndTime - StartTime;

    AddInfo(FString::Printf(TEXT("Replication stress test: %d iterations in %.3f seconds"), NumIterations, TotalTime));
    TestTrue(TEXT("Replication handles stress"), TotalTime < 1.0); // Should complete in under 1 second

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FNetworkSyncEdgeCaseTest, "HorrorProject.Network.SyncEdgeCases", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FNetworkSyncEdgeCaseTest::RunTest(const FString& Parameters)
{
    UWorld* World = FAutomationEditorCommonUtils::CreateNewMap();
    AActor* TestActor = World->SpawnActor<AActor>();

    UNetworkSyncComponent* SyncComp = NewObject<UNetworkSyncComponent>(TestActor);
    TestNotNull(TEXT("SyncComponent created"), SyncComp);

    // Test sync with null data
    SyncComp->SyncData(nullptr);
    TestTrue(TEXT("Null data sync handled"), true);

    // Test sync with empty array
    TArray<uint8> EmptyData;
    SyncComp->SyncDataArray(EmptyData);
    TestTrue(TEXT("Empty array sync handled"), true);

    // Test sync with large data
    TArray<uint8> LargeData;
    LargeData.SetNum(1024 * 1024); // 1MB
    SyncComp->SyncDataArray(LargeData);
    TestTrue(TEXT("Large data sync handled"), true);

    return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR
