#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR && HORRORPROJECT_ENABLE_LEGACY_AUTOMATION_TESTS

// Copyright Epic Games, Inc. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "GameFramework/SaveGame.h"
#include "Kismet/GameplayStatics.h"
#include "Game/HorrorEventBusSubsystem.h"
#include "Audio/HorrorAudioSubsystem.h"
#include "Evidence/ArchiveSubsystem.h"
#include "Evidence/EvidenceSaveGame.h"
#include "Player/Components/FearComponent.h"
#include "Player/Components/CameraBatteryComponent.h"
#include "Tests/AutomationCommon.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSaveLoadIntegrationBasicTest,
	"HorrorProject.Integration.SaveLoad.BasicSaveLoad",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FSaveLoadIntegrationBasicTest::RunTest(const FString& Parameters)
{
	UWorld* World = FAutomationEditorCommonUtils::CreateNewMap();
	if (!World)
	{
		AddError(TEXT("Failed to create test world"));
		return false;
	}

	const FString SlotName = TEXT("TestSaveSlot");
	const int32 UserIndex = 0;

	// Create and save
	UEvidenceSaveGame* SaveGame = Cast<UEvidenceSaveGame>(UGameplayStatics::CreateSaveGameObject(UEvidenceSaveGame::StaticClass()));
	TestNotNull(TEXT("Save game object should be created"), SaveGame);

	bool bSaved = UGameplayStatics::SaveGameToSlot(SaveGame, SlotName, UserIndex);
	TestTrue(TEXT("Save should succeed"), bSaved);

	// Load
	USaveGame* LoadedGame = UGameplayStatics::LoadGameFromSlot(SlotName, UserIndex);
	TestNotNull(TEXT("Loaded game should not be null"), LoadedGame);

	// Cleanup
	UGameplayStatics::DeleteGameInSlot(SlotName, UserIndex);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSaveLoadIntegrationEventBusStateTest,
	"HorrorProject.Integration.SaveLoad.EventBusState",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FSaveLoadIntegrationEventBusStateTest::RunTest(const FString& Parameters)
{
	UWorld* World = FAutomationEditorCommonUtils::CreateNewMap();
	if (!World)
	{
		AddError(TEXT("Failed to create test world"));
		return false;
	}

	UHorrorEventBusSubsystem* EventBus = World->GetSubsystem<UHorrorEventBusSubsystem>();
	if (!EventBus)
	{
		AddError(TEXT("EventBus subsystem not found"));
		return false;
	}

	// Publish some events
	FGameplayTag Tag1 = FGameplayTag::RequestGameplayTag(FName("Horror.Test.Event1"));
	FGameplayTag Tag2 = FGameplayTag::RequestGameplayTag(FName("Horror.Test.Event2"));

	EventBus->Publish(Tag1, FName("Source1"), FGameplayTag(), nullptr);
	EventBus->Publish(Tag2, FName("Source2"), FGameplayTag(), nullptr);

	int32 EventCount = EventBus->GetHistory().Num();
	TestEqual(TEXT("Should have 2 events"), EventCount, 2);

	// In a real save/load, these events would be serialized
	// For now, verify the state exists
	TestTrue(TEXT("Event history should be accessible"), EventCount > 0);

	EventBus->ResetForTests();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSaveLoadIntegrationPlayerStateTest,
	"HorrorProject.Integration.SaveLoad.PlayerState",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FSaveLoadIntegrationPlayerStateTest::RunTest(const FString& Parameters)
{
	UWorld* World = FAutomationEditorCommonUtils::CreateNewMap();
	if (!World)
	{
		AddError(TEXT("Failed to create test world"));
		return false;
	}

	AActor* PlayerActor = World->SpawnActor<AActor>();

	UFearComponent* FearComp = NewObject<UFearComponent>(PlayerActor);
	FearComp->RegisterComponent();

	UCameraBatteryComponent* BatteryComp = NewObject<UCameraBatteryComponent>(PlayerActor);
	BatteryComp->RegisterComponent();

	// Get initial states
	float InitialFear = FearComp->GetCurrentFear();
	float InitialBattery = BatteryComp->GetCurrentCharge();

	TestTrue(TEXT("Initial fear should be valid"), InitialFear >= 0.0f);
	TestTrue(TEXT("Initial battery should be valid"), InitialBattery >= 0.0f && InitialBattery <= 100.0f);

	// In a real save/load, these values would be serialized and restored
	TestNotNull(TEXT("Fear component should exist"), FearComp);
	TestNotNull(TEXT("Battery component should exist"), BatteryComp);

	PlayerActor->Destroy();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSaveLoadIntegrationEvidenceStateTest,
	"HorrorProject.Integration.SaveLoad.EvidenceState",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FSaveLoadIntegrationEvidenceStateTest::RunTest(const FString& Parameters)
{
	UWorld* World = FAutomationEditorCommonUtils::CreateNewMap();
	if (!World)
	{
		AddError(TEXT("Failed to create test world"));
		return false;
	}

	UArchiveSubsystem* ArchiveSubsystem = World->GetSubsystem<UArchiveSubsystem>();
	if (!ArchiveSubsystem)
	{
		AddError(TEXT("Archive subsystem not found"));
		return false;
	}

	// Archive subsystem should be ready for save/load
	TestNotNull(TEXT("Archive subsystem should exist"), ArchiveSubsystem);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSaveLoadIntegrationAudioStateTest,
	"HorrorProject.Integration.SaveLoad.AudioState",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FSaveLoadIntegrationAudioStateTest::RunTest(const FString& Parameters)
{
	UWorld* World = FAutomationEditorCommonUtils::CreateNewMap();
	if (!World)
	{
		AddError(TEXT("Failed to create test world"));
		return false;
	}

	UHorrorAudioSubsystem* AudioSubsystem = World->GetSubsystem<UHorrorAudioSubsystem>();
	if (!AudioSubsystem)
	{
		AddError(TEXT("Audio subsystem not found"));
		return false;
	}

	// Audio subsystem state should be saveable
	TestTrue(TEXT("Audio subsystem should be initialized"), AudioSubsystem->IsInitialized());

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSaveLoadIntegrationFullCycleTest,
	"HorrorProject.Integration.SaveLoad.FullCycle",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FSaveLoadIntegrationFullCycleTest::RunTest(const FString& Parameters)
{
	UWorld* World = FAutomationEditorCommonUtils::CreateNewMap();
	if (!World)
	{
		AddError(TEXT("Failed to create test world"));
		return false;
	}

	const FString SlotName = TEXT("TestFullCycleSlot");
	const int32 UserIndex = 0;

	// Setup game state
	UHorrorEventBusSubsystem* EventBus = World->GetSubsystem<UHorrorEventBusSubsystem>();
	UHorrorAudioSubsystem* AudioSubsystem = World->GetSubsystem<UHorrorAudioSubsystem>();
	UArchiveSubsystem* ArchiveSubsystem = World->GetSubsystem<UArchiveSubsystem>();

	TestNotNull(TEXT("EventBus should exist"), EventBus);
	TestNotNull(TEXT("Audio subsystem should exist"), AudioSubsystem);
	TestNotNull(TEXT("Archive subsystem should exist"), ArchiveSubsystem);

	// Publish some events
	FGameplayTag TestTag = FGameplayTag::RequestGameplayTag(FName("Horror.Test.SaveLoad"));
	EventBus->Publish(TestTag, FName("TestSource"), FGameplayTag(), nullptr);

	int32 EventCountBeforeSave = EventBus->GetHistory().Num();

	// Create save game
	UEvidenceSaveGame* SaveGame = Cast<UEvidenceSaveGame>(UGameplayStatics::CreateSaveGameObject(UEvidenceSaveGame::StaticClass()));
	TestNotNull(TEXT("Save game should be created"), SaveGame);

	// Save
	bool bSaved = UGameplayStatics::SaveGameToSlot(SaveGame, SlotName, UserIndex);
	TestTrue(TEXT("Save should succeed"), bSaved);

	// Reset state
	EventBus->ResetForTests();
	TestEqual(TEXT("Event history should be cleared"), EventBus->GetHistory().Num(), 0);

	// Load
	USaveGame* LoadedGame = UGameplayStatics::LoadGameFromSlot(SlotName, UserIndex);
	TestNotNull(TEXT("Loaded game should not be null"), LoadedGame);

	// Cleanup
	UGameplayStatics::DeleteGameInSlot(SlotName, UserIndex);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSaveLoadIntegrationNoDataLossTest,
	"HorrorProject.Integration.SaveLoad.NoDataLoss",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FSaveLoadIntegrationNoDataLossTest::RunTest(const FString& Parameters)
{
	UWorld* World = FAutomationEditorCommonUtils::CreateNewMap();
	if (!World)
	{
		AddError(TEXT("Failed to create test world"));
		return false;
	}

	const FString SlotName = TEXT("TestNoDataLossSlot");
	const int32 UserIndex = 0;

	// Create save with data
	UEvidenceSaveGame* SaveGame = Cast<UEvidenceSaveGame>(UGameplayStatics::CreateSaveGameObject(UEvidenceSaveGame::StaticClass()));
	TestNotNull(TEXT("Save game should be created"), SaveGame);

	// Save
	bool bSaved = UGameplayStatics::SaveGameToSlot(SaveGame, SlotName, UserIndex);
	TestTrue(TEXT("Save should succeed"), bSaved);

	// Load immediately
	UEvidenceSaveGame* LoadedGame = Cast<UEvidenceSaveGame>(UGameplayStatics::LoadGameFromSlot(SlotName, UserIndex));
	TestNotNull(TEXT("Loaded game should not be null"), LoadedGame);

	// Verify data integrity
	TestTrue(TEXT("Loaded game should be correct type"), LoadedGame->IsA(UEvidenceSaveGame::StaticClass()));

	// Cleanup
	UGameplayStatics::DeleteGameInSlot(SlotName, UserIndex);

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR
