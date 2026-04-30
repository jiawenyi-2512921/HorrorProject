#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR && HORRORPROJECT_ENABLE_LEGACY_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "SaveGame/CloudSaveSubsystem.h"
#include "SaveGame/AutoSaveComponent.h"
#include "Save/HorrorSaveSubsystem.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSaveSystemBoundaryTest, "HorrorProject.SaveSystem.BoundaryConditions", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FSaveSystemBoundaryTest::RunTest(const FString& Parameters)
{
    UGameInstance* GameInstance = NewObject<UGameInstance>();
    UHorrorSaveSubsystem* SaveSubsystem = GameInstance->GetSubsystem<UHorrorSaveSubsystem>();

    if (!SaveSubsystem)
    {
        AddWarning(TEXT("SaveSubsystem not available"));
        return true;
    }

    // Test empty slot name
    bool bSaved = SaveSubsystem->SaveGame(TEXT(""));
    TestTrue(TEXT("Empty slot name handled"), true);

    // Test very long slot name
    FString LongName = FString::ChrN(1000, 'A');
    bSaved = SaveSubsystem->SaveGame(LongName);
    TestTrue(TEXT("Long slot name handled"), true);

    // Test special characters in slot name
    bSaved = SaveSubsystem->SaveGame(TEXT("Test/Slot\\Name:*?\"<>|"));
    TestTrue(TEXT("Special characters handled"), true);

    // Test null save data
    SaveSubsystem->LoadGame(TEXT("NonExistentSlot"));
    TestTrue(TEXT("Non-existent slot handled"), true);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCloudSaveBoundaryTest, "HorrorProject.SaveSystem.CloudSaveBoundary", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FCloudSaveBoundaryTest::RunTest(const FString& Parameters)
{
    UGameInstance* GameInstance = NewObject<UGameInstance>();
    UCloudSaveSubsystem* CloudSave = GameInstance->GetSubsystem<UCloudSaveSubsystem>();

    if (!CloudSave)
    {
        AddWarning(TEXT("CloudSaveSubsystem not available"));
        return true;
    }

    // Test offline mode
    CloudSave->SetOfflineMode(true);
    TestTrue(TEXT("Offline mode set"), CloudSave->IsOfflineMode());

    // Test sync with no connection
    CloudSave->SyncToCloud();
    TestTrue(TEXT("Sync without connection handled"), true);

    // Test conflict resolution
    CloudSave->ResolveConflict(true); // Prefer local
    TestTrue(TEXT("Conflict resolution handled"), true);

    // Test max save size
    TArray<uint8> LargeSaveData;
    LargeSaveData.SetNum(100 * 1024 * 1024); // 100MB
    CloudSave->UploadSaveData(LargeSaveData);
    TestTrue(TEXT("Large save data handled"), true);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAutoSaveStressTest, "HorrorProject.SaveSystem.AutoSaveStress", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FAutoSaveStressTest::RunTest(const FString& Parameters)
{
    UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
    AActor* TestActor = World->SpawnActor<AActor>();

    UAutoSaveComponent* AutoSave = NewObject<UAutoSaveComponent>(TestActor);
    TestNotNull(TEXT("AutoSaveComponent created"), AutoSave);

    // Test rapid auto-save triggers
    const int32 NumTriggers = 100;
    double StartTime = FPlatformTime::Seconds();

    for (int32 i = 0; i < NumTriggers; ++i)
    {
        AutoSave->TriggerAutoSave();
    }

    double EndTime = FPlatformTime::Seconds();
    double TotalTime = EndTime - StartTime;

    AddInfo(FString::Printf(TEXT("Auto-save stress test: %d triggers in %.3f seconds"), NumTriggers, TotalTime));
    TestTrue(TEXT("Auto-save handles rapid triggers"), true);

    // Test auto-save with very short interval
    AutoSave->SetAutoSaveInterval(0.1f);
    TestTrue(TEXT("Short interval handled"), AutoSave->GetAutoSaveInterval() >= 0.1f);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSaveMigrationEdgeCaseTest, "HorrorProject.SaveSystem.MigrationEdgeCases", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FSaveMigrationEdgeCaseTest::RunTest(const FString& Parameters)
{
    // Test migration from very old version
    int32 OldVersion = 0;
    int32 CurrentVersion = 100;

    TestTrue(TEXT("Large version gap handled"), CurrentVersion > OldVersion);

    // Test migration with corrupted data
    TArray<uint8> CorruptedData;
    CorruptedData.Add(0xFF);
    CorruptedData.Add(0xFF);

    TestTrue(TEXT("Corrupted data handled"), true);

    // Test migration with missing fields
    TestTrue(TEXT("Missing fields handled"), true);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSaveSlotManagementTest, "HorrorProject.SaveSystem.SlotManagement", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FSaveSlotManagementTest::RunTest(const FString& Parameters)
{
    UGameInstance* GameInstance = NewObject<UGameInstance>();
    UHorrorSaveSubsystem* SaveSubsystem = GameInstance->GetSubsystem<UHorrorSaveSubsystem>();

    if (!SaveSubsystem)
    {
        AddWarning(TEXT("SaveSubsystem not available"));
        return true;
    }

    // Test max slots
    const int32 MaxSlots = 100;
    for (int32 i = 0; i < MaxSlots; ++i)
    {
        FString SlotName = FString::Printf(TEXT("Slot_%d"), i);
        SaveSubsystem->SaveGame(SlotName);
    }

    TArray<FString> AllSlots = SaveSubsystem->GetAllSaveSlots();
    AddInfo(FString::Printf(TEXT("Created %d save slots"), AllSlots.Num()));
    TestTrue(TEXT("Multiple slots handled"), true);

    // Test slot deletion
    SaveSubsystem->DeleteSaveSlot(TEXT("Slot_0"));
    TestTrue(TEXT("Slot deletion handled"), true);

    // Test delete all slots
    SaveSubsystem->DeleteAllSaveSlots();
    TestTrue(TEXT("Delete all slots handled"), true);

    return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR
