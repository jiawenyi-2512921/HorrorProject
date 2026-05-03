// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/HorrorPlayerController.h"

#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR

#include "Game/HorrorEventBusSubsystem.h"
#include "Game/DeepWaterStationRouteKit.h"
#include "Game/HorrorCampaignObjectiveActor.h"
#include "Game/HorrorFoundFootageContract.h"
#include "Game/HorrorGameModeBase.h"
#include "Game/HorrorEncounterDirector.h"
#include "Components/BoxComponent.h"
#include "Components/SpotLightComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/WorldSettings.h"
#include "InputCoreTypes.h"
#include "Interaction/DoorInteractable.h"
#include "Misc/AutomationTest.h"
#include "Player/HorrorPlayerCharacter.h"
#include "Player/Components/FlashlightComponent.h"
#include "Player/Components/InteractionComponent.h"
#include "Player/Components/NoteRecorderComponent.h"
#include "Tests/AutomationCommon.h"
#include "UI/Day1SliceHUD.h"

namespace
{
	UFlashlightComponent* AttachRegisteredFlashlightComponent(AActor& Owner)
	{
		UFlashlightComponent* Flashlight = NewObject<UFlashlightComponent>(&Owner, TEXT("TestFlashlight"));
		if (Flashlight)
		{
			Flashlight->RegisterComponent();
		}
		return Flashlight;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorPlayerControllerNativeInputContextsTest,
	"HorrorProject.Player.Controller.NativeInputContexts",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorPlayerControllerNativeInputContextsTest::RunTest(const FString& Parameters)
{
	const AHorrorPlayerController* NativeDefaults = GetDefault<AHorrorPlayerController>();
	TestNotNull(TEXT("Native player controller defaults should be available."), NativeDefaults);
	if (!NativeDefaults)
	{
		return false;
	}

	TestTrue(
		TEXT("Native Day1 controller should load the gameplay input mapping without relying on the old Blueprint controller."),
		NativeDefaults->GetDefaultMappingContextCountForTests() > 0);
	TestTrue(
		TEXT("Native Day1 controller should load mouse look input without relying on the old Blueprint controller."),
		NativeDefaults->GetMobileExcludedMappingContextCountForTests() > 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorPlayerControllerDay1PauseInputTest,
	"HorrorProject.Player.Controller.Day1PauseInput",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorPlayerControllerDay1PauseInputTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for Day1 pause input coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	AHorrorPlayerController* PlayerController = World->SpawnActor<AHorrorPlayerController>();
	TestNotNull(TEXT("Day1 pause input test should spawn a horror player controller."), PlayerController);
	ADay1SliceHUD* HUD = World->SpawnActor<ADay1SliceHUD>();
	TestNotNull(TEXT("Day1 pause input test should attach the native Day1 HUD."), HUD);
	ADoorInteractable* Door = World->SpawnActor<ADoorInteractable>();
	TestNotNull(TEXT("Day1 pause input test should spawn a password door."), Door);
	if (!PlayerController || !HUD || !Door)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}
	PlayerController->MyHUD = HUD;

	TestFalse(TEXT("Pause menu should start closed."), PlayerController->IsDay1PauseMenuOpenForTests());
	TestTrue(TEXT("Escape should open the Day1 pause menu."), PlayerController->HandleInputKeyForTests(EKeys::Escape));
	TestTrue(TEXT("Pause menu should be open after Escape."), PlayerController->IsDay1PauseMenuOpenForTests());
	TestEqual(TEXT("Pause menu should start on Resume."), PlayerController->GetDay1PauseMenuSelectionForTests(), EDay1PauseMenuSelection::Resume);

	TestTrue(TEXT("Down should move pause selection."), PlayerController->HandleInputKeyForTests(EKeys::Down));
	TestEqual(TEXT("Pause menu should move to mouse sensitivity."), PlayerController->GetDay1PauseMenuSelectionForTests(), EDay1PauseMenuSelection::MouseSensitivity);

	TestTrue(TEXT("Escape should close the Day1 pause menu."), PlayerController->HandleInputKeyForTests(EKeys::Escape));
	TestFalse(TEXT("Pause menu should close after Escape while paused."), PlayerController->IsDay1PauseMenuOpenForTests());

	UHorrorEventBusSubsystem* EventBus = World->GetSubsystem<UHorrorEventBusSubsystem>();
	TestNotNull(TEXT("Day1 pause input test should expose the event bus."), EventBus);
	if (EventBus)
	{
		TestTrue(TEXT("J should open the notes journal before completion."), PlayerController->HandleInputKeyForTests(EKeys::J));
		TestTrue(TEXT("Notes journal should be open before completion lock."), PlayerController->IsDay1NotesJournalOpenForTests());

		Door->ConfigurePassword(TEXT("2468"), FText::FromString(TEXT("完成锁定测试密码。")));
		PlayerController->BeginDoorPasswordEntry(Door);
		TestTrue(TEXT("Password entry should be active before completion lock."), PlayerController->IsAwaitingDoorPassword());
		TestTrue(TEXT("HUD should show password entry before completion lock."), HUD->IsPasswordPromptVisibleForTests());

		TestFalse(TEXT("Day1 completion input lock should start disabled."), PlayerController->IsDay1CompletionInputLockedForTests());
		PlayerController->BindObjectiveEventBusForTests();
		EventBus->Publish(HorrorDay1Tags::Day1CompletedEvent(), TEXT("Exit.Gate"), HorrorDay1Tags::Day1CompletedState(), PlayerController);
		TestTrue(TEXT("Day1 completion event should lock movement/look input."), PlayerController->IsDay1CompletionInputLockedForTests());
		TestFalse(TEXT("Day1 completion should close the notes journal."), PlayerController->IsDay1NotesJournalOpenForTests());
		TestFalse(TEXT("Day1 completion should cancel pending password entry."), PlayerController->IsAwaitingDoorPassword());
		TestFalse(TEXT("Day1 completion should clear the password HUD prompt."), HUD->IsPasswordPromptVisibleForTests());

		TestTrue(TEXT("Escape after Day1 completion should be consumed."), PlayerController->HandleInputKeyForTests(EKeys::Escape));
		TestFalse(TEXT("Escape after Day1 completion should not reopen pause."), PlayerController->IsDay1PauseMenuOpenForTests());
		TestTrue(TEXT("J after Day1 completion should be consumed."), PlayerController->HandleInputKeyForTests(EKeys::J));
		TestFalse(TEXT("J after Day1 completion should not reopen notes journal."), PlayerController->IsDay1NotesJournalOpenForTests());
		PlayerController->BeginDoorPasswordEntry(Door);
		TestFalse(TEXT("Completion lock should keep password entry inactive after new attempts."), PlayerController->IsAwaitingDoorPassword());
	}

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorPlayerControllerFallbackFlashlightInputTest,
	"HorrorProject.Player.Controller.FallbackFlashlightInput",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorPlayerControllerFallbackFlashlightInputTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for flashlight input coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	AHorrorPlayerController* PlayerController = World->SpawnActor<AHorrorPlayerController>();
	AHorrorPlayerCharacter* PlayerCharacter = World->SpawnActor<AHorrorPlayerCharacter>();
	TestNotNull(TEXT("Flashlight input test should spawn a horror player controller."), PlayerController);
	TestNotNull(TEXT("Flashlight input test should spawn a horror player character."), PlayerCharacter);
	if (!PlayerController || !PlayerCharacter)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	PlayerController->PlayerState = World->SpawnActor<APlayerState>();
	World->AddController(PlayerController);
	PlayerController->Possess(PlayerCharacter);

	UFlashlightComponent* Flashlight = PlayerCharacter->GetFlashlightComponent();
	TestNotNull(TEXT("Player character should expose a flashlight component."), Flashlight);
	if (!Flashlight)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	TestFalse(TEXT("Flashlight should start off."), Flashlight->IsFlashlightOn());
	TestTrue(TEXT("F should be consumed by the fallback flashlight input path."), PlayerController->HandleInputKeyForTests(EKeys::F));
	TestTrue(TEXT("F should turn the flashlight on even if a map misses the Enhanced Input binding."), Flashlight->IsFlashlightOn());
	TestTrue(TEXT("Pressing F again should be consumed."), PlayerController->HandleInputKeyForTests(EKeys::F));
	TestFalse(TEXT("Pressing F again should turn the flashlight off."), Flashlight->IsFlashlightOn());

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorPlayerControllerFallbackFlashlightCreatesRuntimeRigForImportedPawnTest,
	"HorrorProject.Player.Controller.FallbackFlashlightCreatesRuntimeRigForImportedPawn",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorPlayerControllerFallbackFlashlightCreatesRuntimeRigForImportedPawnTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for runtime flashlight rig coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	AHorrorPlayerController* PlayerController = World->SpawnActor<AHorrorPlayerController>();
	APawn* ImportedPawn = World->SpawnActor<APawn>();
	TestNotNull(TEXT("Runtime rig test should spawn a horror player controller."), PlayerController);
	TestNotNull(TEXT("Runtime rig test should spawn an imported-style pawn."), ImportedPawn);
	if (!PlayerController || !ImportedPawn)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	TestNull(TEXT("Bare imported pawn should start without a flashlight component."), ImportedPawn->FindComponentByClass<UFlashlightComponent>());
	TestNull(TEXT("Bare imported pawn should start without a spotlight component."), ImportedPawn->FindComponentByClass<USpotLightComponent>());

	PlayerController->PlayerState = World->SpawnActor<APlayerState>();
	World->AddController(PlayerController);
	PlayerController->Possess(ImportedPawn);

	UFlashlightComponent* RuntimeFlashlight = ImportedPawn->FindComponentByClass<UFlashlightComponent>();
	TestNotNull(TEXT("Possessing an imported pawn should install a runtime flashlight component."), RuntimeFlashlight);
	if (!RuntimeFlashlight)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	USpotLightComponent* RuntimeSpotLight = RuntimeFlashlight->GetBoundSpotLight();
	TestNotNull(TEXT("Runtime flashlight should bind to a visible spotlight beam."), RuntimeSpotLight);
	if (!RuntimeSpotLight)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	TestFalse(TEXT("Runtime flashlight should start off."), RuntimeFlashlight->IsFlashlightOn());
	TestFalse(TEXT("Runtime spotlight should start hidden."), RuntimeSpotLight->IsVisible());
	TestTrue(TEXT("F should be consumed by the runtime flashlight rig."), PlayerController->HandleInputKeyForTests(EKeys::F));
	TestTrue(TEXT("F should turn on the runtime flashlight."), RuntimeFlashlight->IsFlashlightOn());
	TestTrue(TEXT("F should make the runtime spotlight visible."), RuntimeSpotLight->IsVisible());

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorPlayerControllerDefaultFlashlightIsReadableTest,
	"HorrorProject.Player.Controller.DefaultFlashlightIsReadable",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorPlayerControllerDefaultFlashlightIsReadableTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for flashlight readability coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	AHorrorPlayerCharacter* PlayerCharacter = World->SpawnActor<AHorrorPlayerCharacter>();
	TestNotNull(TEXT("Flashlight readability test should spawn the horror player character."), PlayerCharacter);
	if (!PlayerCharacter)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	const USpotLightComponent* SpotLight = PlayerCharacter->GetSpotLight();
	TestNotNull(TEXT("The native player character should expose the flashlight spotlight."), SpotLight);
	if (!SpotLight)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	TestTrue(
		TEXT("Default flashlight intensity should stay readable while preserving a darker horror tone."),
		SpotLight->Intensity >= 900.0f && SpotLight->Intensity <= 1000.0f);
	TestTrue(
		TEXT("Default flashlight radius should remain playable without washing out whole rooms."),
		SpotLight->AttenuationRadius >= 1100.0f && SpotLight->AttenuationRadius <= 1200.0f);

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorPlayerControllerFallbackFlashlightFindsPawnComponentTest,
	"HorrorProject.Player.Controller.FallbackFlashlightFindsPawnComponent",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorPlayerControllerFallbackFlashlightFindsPawnComponentTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for generic pawn flashlight coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	AHorrorPlayerController* PlayerController = World->SpawnActor<AHorrorPlayerController>();
	APawn* ImportedPawn = World->SpawnActor<APawn>();
	TestNotNull(TEXT("Generic pawn flashlight test should spawn a horror player controller."), PlayerController);
	TestNotNull(TEXT("Generic pawn flashlight test should spawn an imported-style pawn."), ImportedPawn);
	if (!PlayerController || !ImportedPawn)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	UFlashlightComponent* Flashlight = AttachRegisteredFlashlightComponent(*ImportedPawn);
	TestNotNull(TEXT("Imported-style pawn should receive a flashlight component."), Flashlight);
	if (!Flashlight)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	PlayerController->PlayerState = World->SpawnActor<APlayerState>();
	World->AddController(PlayerController);
	PlayerController->Possess(ImportedPawn);

	TestFalse(TEXT("Generic pawn flashlight should start off."), Flashlight->IsFlashlightOn());
	TestTrue(TEXT("F should be consumed when the possessed pawn has a flashlight component."), PlayerController->HandleInputKeyForTests(EKeys::F));
	TestTrue(TEXT("F should turn on a flashlight component even when the pawn is not AHorrorPlayerCharacter."), Flashlight->IsFlashlightOn());
	TestTrue(TEXT("F should be consumed for the second generic pawn toggle."), PlayerController->HandleInputKeyForTests(EKeys::F));
	TestFalse(TEXT("F should turn the generic pawn flashlight off again."), Flashlight->IsFlashlightOn());

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorPlayerControllerFallbackFlashlightFindsPawnSpotLightTest,
	"HorrorProject.Player.Controller.FallbackFlashlightFindsPawnSpotLight",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorPlayerControllerFallbackFlashlightFindsPawnSpotLightTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for imported spotlight coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	AHorrorPlayerController* PlayerController = World->SpawnActor<AHorrorPlayerController>();
	APawn* ImportedPawn = World->SpawnActor<APawn>();
	TestNotNull(TEXT("Imported spotlight test should spawn a horror player controller."), PlayerController);
	TestNotNull(TEXT("Imported spotlight test should spawn an imported-style pawn."), ImportedPawn);
	if (!PlayerController || !ImportedPawn)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	USpotLightComponent* SpotLight = NewObject<USpotLightComponent>(ImportedPawn, TEXT("TestSpotLight"));
	TestNotNull(TEXT("Imported-style pawn should receive a spotlight component."), SpotLight);
	if (!SpotLight)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	SpotLight->RegisterComponent();
	SpotLight->SetVisibility(false);

	PlayerController->PlayerState = World->SpawnActor<APlayerState>();
	World->AddController(PlayerController);
	PlayerController->Possess(ImportedPawn);

	TestFalse(TEXT("Imported-style spotlight should start hidden."), SpotLight->IsVisible());
	TestTrue(TEXT("F should be consumed when the possessed pawn has only a spotlight fallback."), PlayerController->HandleInputKeyForTests(EKeys::F));
	TestTrue(TEXT("F should reveal an imported pawn spotlight when no flashlight component exists."), SpotLight->IsVisible());
	TestTrue(TEXT("F should be consumed for the second imported spotlight toggle."), PlayerController->HandleInputKeyForTests(EKeys::F));
	TestFalse(TEXT("F should hide the imported pawn spotlight again."), SpotLight->IsVisible());

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorPlayerControllerDay1ObjectiveNavigationTest,
	"HorrorProject.Player.Controller.Day1ObjectiveNavigation",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorPlayerControllerDay1ObjectiveNavigationTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for Day1 objective navigation coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	World->GetWorldSettings()->DefaultGameMode = AHorrorGameModeBase::StaticClass();
	TestTrue(TEXT("Transient world should create the Day1 navigation game mode."), World->SetGameMode(FURL()));
	AHorrorGameModeBase* GameMode = World->GetAuthGameMode<AHorrorGameModeBase>();
	TestNotNull(TEXT("Day1 navigation test should expose the game mode."), GameMode);
	if (!GameMode)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	AHorrorPlayerController* PlayerController = World->SpawnActor<AHorrorPlayerController>();
	ADay1SliceHUD* HUD = World->SpawnActor<ADay1SliceHUD>();
	AHorrorPlayerCharacter* PlayerCharacter = World->SpawnActor<AHorrorPlayerCharacter>(FVector(0.0f, 0.0f, 80.0f), FRotator::ZeroRotator);
	TestNotNull(TEXT("Day1 navigation test should spawn a horror player controller."), PlayerController);
	TestNotNull(TEXT("Day1 navigation test should attach the native Day1 HUD."), HUD);
	TestNotNull(TEXT("Day1 navigation test should spawn a horror player character."), PlayerCharacter);
	if (!PlayerController || !HUD || !PlayerCharacter)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	PlayerController->PlayerState = World->SpawnActor<APlayerState>();
	World->AddController(PlayerController);
	PlayerController->MyHUD = HUD;
	PlayerController->Possess(PlayerCharacter);
	GameMode->DispatchBeginPlay();
	PlayerController->RefreshDay1HUDStateForTests();

	TestFalse(TEXT("Objective navigation should be visible when the route kit has a next target."), HUD->GetObjectiveNavigationForTests().IsEmpty());
	TestTrue(
		TEXT("Objective navigation should include an approximate distance to the next target."),
		HUD->GetObjectiveNavigationForTests().ToString().Contains(TEXT("2 米")));
	TestTrue(
		TEXT("Objective navigation should include a facing-relative direction."),
		HUD->GetObjectiveNavigationForTests().ToString().Contains(TEXT("前方")));

	TestTrue(TEXT("Bodycam acquisition should advance the navigation target."), GameMode->TryAcquireBodycam(TEXT("Evidence.Bodycam"), true));
	PlayerController->RefreshDay1HUDStateForTests();
	TestTrue(
		TEXT("Objective navigation should update after the first objective completes."),
		HUD->GetObjectiveNavigationForTests().ToString().Contains(TEXT("6 米")));

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorPlayerControllerNavigationDoesNotPrioritizeCampaignBeforeDay1CompleteTest,
	"HorrorProject.Player.Controller.NavigationDoesNotPrioritizeCampaignBeforeDay1Complete",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorPlayerControllerNavigationDoesNotPrioritizeCampaignBeforeDay1CompleteTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for Day1 campaign navigation boundary coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	World->GetWorldSettings()->DefaultGameMode = AHorrorGameModeBase::StaticClass();
	TestTrue(TEXT("Transient world should create the navigation boundary game mode."), World->SetGameMode(FURL()));
	AHorrorGameModeBase* GameMode = World->GetAuthGameMode<AHorrorGameModeBase>();
	AHorrorPlayerController* PlayerController = World->SpawnActor<AHorrorPlayerController>();
	TestNotNull(TEXT("Navigation boundary test should expose the game mode."), GameMode);
	TestNotNull(TEXT("Navigation boundary test should spawn a horror player controller."), PlayerController);
	if (!GameMode || !PlayerController)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	FString ErrorMessage;
	GameMode->InitGame(TEXT("/Game/DeepWaterStation/Maps/DemoMap_VerticalSlice_Day1"), TEXT(""), ErrorMessage);
	GameMode->ResetCampaignProgressForChapterForTests(TEXT("Chapter.DeepWaterStationFinale"));
	GameMode->ImportDay1CompleteState(false);

	TestFalse(TEXT("Day1 map should not expose campaign HUD before found-footage completion."), GameMode->ShouldExposeCampaignObjectivesToHUD());
	TestFalse(TEXT("Player navigation should keep using Day1 RouteKit before found-footage completion."), PlayerController->ShouldPrioritizeCampaignNavigationForTests(GameMode));

	GameMode->ImportDay1CompleteState(true);
	TestTrue(TEXT("Day1 completion should expose the campaign chapter HUD."), GameMode->ShouldExposeCampaignObjectivesToHUD());
	TestTrue(TEXT("Player navigation should prioritize campaign objectives after Day1 completion."), PlayerController->ShouldPrioritizeCampaignNavigationForTests(GameMode));

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorPlayerControllerCyclesCampaignNavigationFocusInputTest,
	"HorrorProject.Player.Controller.CyclesCampaignNavigationFocusInput",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorPlayerControllerCyclesCampaignNavigationFocusInputTest::RunTest(const FString& Parameters)
{
	const FHorrorCampaignChapterDefinition* ForestChapter = FHorrorCampaign::FindChapterById(TEXT("Chapter.ForestOfSpikes"));
	TestNotNull(TEXT("Forest chapter should exist for controller navigation focus input coverage."), ForestChapter);
	if (!ForestChapter)
	{
		return false;
	}

	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for controller navigation focus input coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	World->GetWorldSettings()->DefaultGameMode = AHorrorGameModeBase::StaticClass();
	TestTrue(TEXT("Transient world should create the campaign navigation focus game mode."), World->SetGameMode(FURL()));
	AHorrorGameModeBase* GameMode = World->GetAuthGameMode<AHorrorGameModeBase>();
	AHorrorPlayerController* PlayerController = World->SpawnActor<AHorrorPlayerController>();
	ADay1SliceHUD* HUD = World->SpawnActor<ADay1SliceHUD>();
	AHorrorPlayerCharacter* PlayerCharacter = World->SpawnActor<AHorrorPlayerCharacter>(FVector::ZeroVector, FRotator::ZeroRotator);
	TestNotNull(TEXT("Navigation focus input test should expose the game mode."), GameMode);
	TestNotNull(TEXT("Navigation focus input test should spawn a player controller."), PlayerController);
	TestNotNull(TEXT("Navigation focus input test should attach the native Day1 HUD."), HUD);
	TestNotNull(TEXT("Navigation focus input test should spawn a player character."), PlayerCharacter);
	if (!GameMode || !PlayerController || !HUD || !PlayerCharacter)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	PlayerController->PlayerState = World->SpawnActor<APlayerState>();
	World->AddController(PlayerController);
	PlayerController->MyHUD = HUD;
	PlayerController->Possess(PlayerCharacter);
	GameMode->ResetCampaignProgressForChapterForTests(ForestChapter->ChapterId);

	PlayerController->RefreshDay1HUDStateForTests();
	TestTrue(TEXT("Initial campaign navigation should be visible."), HUD->IsObjectiveNavigationVisibleForTests());
	TestTrue(TEXT("Initial campaign navigation should point at the story objective."), HUD->GetObjectiveNavigationLabelForTests().ToString().Contains(TEXT("根系")));
	TestTrue(TEXT("Right bracket should cycle campaign navigation focus forward."), PlayerController->HandleInputKeyForTests(EKeys::RightBracket));
	TestEqual(TEXT("Forward input should focus the current story objective first."), GameMode->GetCampaignNavigationFocusObjectiveId(), FName(TEXT("Forest.ReadRootGlyph")));
	TestTrue(TEXT("Second right bracket should cycle to the parallel spike beacon."), PlayerController->HandleInputKeyForTests(EKeys::RightBracket));
	TestEqual(TEXT("Forward input should focus the parallel spike beacon."), GameMode->GetCampaignNavigationFocusObjectiveId(), FName(TEXT("Forest.AlignSpikeBeacon")));
	TestTrue(TEXT("HUD should immediately show the focused parallel objective."), HUD->GetObjectiveNavigationLabelForTests().ToString().Contains(TEXT("信标")));
	TestTrue(TEXT("HUD should explain manual navigation focus after input."), HUD->GetObjectiveNavigationDeviceStatusForTests().ToString().Contains(TEXT("手动导航")));
	TestTrue(TEXT("Left bracket should cycle campaign navigation focus backward."), PlayerController->HandleInputKeyForTests(EKeys::LeftBracket));
	TestEqual(TEXT("Backward input should return to the root glyph focus."), GameMode->GetCampaignNavigationFocusObjectiveId(), FName(TEXT("Forest.ReadRootGlyph")));
	TestTrue(TEXT("HUD should update immediately after backward focus input."), HUD->GetObjectiveNavigationLabelForTests().ToString().Contains(TEXT("根系")));

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorPlayerControllerDay1ArchiveNavigationRepairsIncompleteRouteKitTest,
	"HorrorProject.Player.Controller.Day1ArchiveNavigationRepairsIncompleteRouteKit",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorPlayerControllerDay1ArchiveNavigationRepairsIncompleteRouteKitTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for archive navigation repair coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	World->GetWorldSettings()->DefaultGameMode = AHorrorGameModeBase::StaticClass();
	TestTrue(TEXT("Transient world should create the archive navigation game mode."), World->SetGameMode(FURL()));
	AHorrorGameModeBase* GameMode = World->GetAuthGameMode<AHorrorGameModeBase>();
	TestNotNull(TEXT("Archive navigation repair test should expose the game mode."), GameMode);
	if (!GameMode)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	ADeepWaterStationRouteKit* LegacyRouteKit = World->SpawnActor<ADeepWaterStationRouteKit>();
	TestNotNull(TEXT("Archive navigation repair test should spawn a legacy route kit."), LegacyRouteKit);
	if (!LegacyRouteKit)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	LegacyRouteKit->ConfigureDefaultFirstLoopObjectiveNodes();
	LegacyRouteKit->ObjectiveNodes.RemoveAll(
		[](const FDeepWaterStationObjectiveNode& Node)
		{
			return Node.Objective == EFoundFootageInteractableObjective::ArchiveReview;
		});

	AHorrorPlayerController* PlayerController = World->SpawnActor<AHorrorPlayerController>();
	ADay1SliceHUD* HUD = World->SpawnActor<ADay1SliceHUD>();
	AHorrorPlayerCharacter* PlayerCharacter = World->SpawnActor<AHorrorPlayerCharacter>(FVector(0.0f, 0.0f, 80.0f), FRotator::ZeroRotator);
	TestNotNull(TEXT("Archive navigation repair test should spawn a horror player controller."), PlayerController);
	TestNotNull(TEXT("Archive navigation repair test should attach the native Day1 HUD."), HUD);
	TestNotNull(TEXT("Archive navigation repair test should spawn a horror player character."), PlayerCharacter);
	if (!PlayerController || !HUD || !PlayerCharacter)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	PlayerController->PlayerState = World->SpawnActor<APlayerState>();
	World->AddController(PlayerController);
	PlayerController->MyHUD = HUD;
	PlayerController->Possess(PlayerCharacter);
	TestSame<AController*>(TEXT("Opened advanced panel test player should be possessed by the spawned horror controller."), PlayerCharacter->GetController(), PlayerController);
	GameMode->DispatchBeginPlay();

	TestTrue(TEXT("Bodycam acquisition should unlock the first note."), GameMode->TryAcquireBodycam(TEXT("Evidence.Bodycam"), true));
	TestTrue(TEXT("First note collection should unlock anomaly capture."), GameMode->TryCollectFirstNote(TEXT("Note.Intro")));
	TestTrue(TEXT("First anomaly candidate should be registered."), GameMode->BeginFirstAnomalyCandidate(TEXT("Evidence.Anomaly01")));
	TestTrue(TEXT("First anomaly recording should move the tracker to archive review."), GameMode->TryRecordFirstAnomaly(true));

	PlayerController->RefreshDay1HUDStateForTests();
	const FString NavigationText = HUD->GetObjectiveNavigationForTests().ToString();
	TestFalse(TEXT("Archive review navigation should remain visible even when an old route kit omitted the terminal node."), NavigationText.IsEmpty());
	TestTrue(TEXT("Archive review navigation should name the archive terminal."), NavigationText.Contains(TEXT("档案终端")));
	TestTrue(TEXT("Archive review navigation should include the terminal distance."), NavigationText.Contains(TEXT("17 米")));

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorPlayerControllerRestampClearsModalHUDStateTest,
	"HorrorProject.Player.Controller.RestampClearsModalHUDState",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorPlayerControllerRestampClearsModalHUDStateTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for checkpoint restamp UI coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	AHorrorPlayerController* PlayerController = World->SpawnActor<AHorrorPlayerController>();
	TestNotNull(TEXT("Checkpoint restamp test should spawn a horror player controller."), PlayerController);
	ADay1SliceHUD* HUD = World->SpawnActor<ADay1SliceHUD>();
	TestNotNull(TEXT("Checkpoint restamp test should attach a Day1 HUD."), HUD);
	ADoorInteractable* Door = World->SpawnActor<ADoorInteractable>();
	TestNotNull(TEXT("Checkpoint restamp test should spawn a password door."), Door);
	if (!PlayerController || !HUD || !Door)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}
	PlayerController->MyHUD = HUD;

	Door->ConfigurePassword(TEXT("1234"), FText::FromString(TEXT("备忘录密码。")));
	PlayerController->BeginDoorPasswordEntry(Door);
	TestTrue(TEXT("Password entry should be active before checkpoint restamp."), PlayerController->IsAwaitingDoorPassword());
	TestTrue(TEXT("Digit input should populate the password buffer before checkpoint restamp."), PlayerController->HandleInputKeyForTests(EKeys::One));
	TestEqual(TEXT("Password buffer should contain typed digits before checkpoint restamp."), PlayerController->GetDoorPasswordBufferForTests(), FString(TEXT("1")));

	PlayerController->RestampCheckpointLoadedUIState();
	TestFalse(TEXT("Checkpoint restamp should cancel pending password entry."), PlayerController->IsAwaitingDoorPassword());
	TestTrue(TEXT("Checkpoint restamp should clear pending password digits."), PlayerController->GetDoorPasswordBufferForTests().IsEmpty());

	TestTrue(TEXT("Escape should open the Day1 pause menu before checkpoint restamp."), PlayerController->HandleInputKeyForTests(EKeys::Escape));
	TestTrue(TEXT("Pause menu should be open before checkpoint restamp."), PlayerController->IsDay1PauseMenuOpenForTests());

	PlayerController->RestampCheckpointLoadedUIState();
	TestFalse(TEXT("Checkpoint restamp should close the Day1 pause menu."), PlayerController->IsDay1PauseMenuOpenForTests());

	if (UHorrorEventBusSubsystem* EventBus = World->GetSubsystem<UHorrorEventBusSubsystem>())
	{
		PlayerController->BindObjectiveEventBusForTests();
		EventBus->Publish(HorrorDay1Tags::Day1CompletedEvent(), TEXT("Exit.Gate"), HorrorDay1Tags::Day1CompletedState(), PlayerController);
		TestTrue(TEXT("Completion event should show the completion overlay before restamp."), HUD->IsDay1CompletionOverlayVisibleForTests());
		TestTrue(TEXT("Completion event should lock input before restamp."), PlayerController->IsDay1CompletionInputLockedForTests());

		PlayerController->RestampCheckpointLoadedUIState();
		TestFalse(TEXT("Non-complete checkpoint restamp should clear completion input lock."), PlayerController->IsDay1CompletionInputLockedForTests());
		TestFalse(TEXT("Non-complete checkpoint restamp should clear the completion overlay."), HUD->IsDay1CompletionOverlayVisibleForTests());
	}

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorPlayerControllerDay1NotesJournalInputTest,
	"HorrorProject.Player.Controller.Day1NotesJournalInput",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorPlayerControllerDay1NotesJournalInputTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for Day1 journal input coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	AHorrorPlayerController* PlayerController = World->SpawnActor<AHorrorPlayerController>();
	ADay1SliceHUD* HUD = World->SpawnActor<ADay1SliceHUD>();
	AHorrorPlayerCharacter* PlayerCharacter = World->SpawnActor<AHorrorPlayerCharacter>();
	TestNotNull(TEXT("Day1 journal input test should spawn a horror player controller."), PlayerController);
	TestNotNull(TEXT("Day1 journal input test should attach the native Day1 HUD."), HUD);
	TestNotNull(TEXT("Day1 journal input test should spawn a horror player character."), PlayerCharacter);
	if (!PlayerController || !HUD || !PlayerCharacter)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	PlayerController->MyHUD = HUD;
	PlayerController->Possess(PlayerCharacter);

	TestFalse(TEXT("Journal should start closed."), PlayerController->IsDay1NotesJournalOpenForTests());
	TestTrue(TEXT("J should open the Day1 notes journal."), PlayerController->HandleInputKeyForTests(EKeys::J));
	TestTrue(TEXT("Controller should track open notes journal state."), PlayerController->IsDay1NotesJournalOpenForTests());
	TestTrue(TEXT("HUD should show an empty journal when no notes are recorded."), HUD->IsNotesJournalVisibleForTests());
	TestTrue(TEXT("HUD should expose empty journal state with no recorded notes."), HUD->IsNotesJournalEmptyForTests());

	TestTrue(TEXT("J should close the Day1 notes journal."), PlayerController->HandleInputKeyForTests(EKeys::J));
	TestFalse(TEXT("Controller should track closed notes journal state."), PlayerController->IsDay1NotesJournalOpenForTests());
	TestFalse(TEXT("HUD should hide the journal after toggling closed."), HUD->IsNotesJournalVisibleForTests());

	UNoteRecorderComponent* NoteRecorder = PlayerCharacter->GetNoteRecorderComponent();
	TestNotNull(TEXT("Player character should expose a note recorder component."), NoteRecorder);
	if (!NoteRecorder)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	FHorrorNoteMetadata StationMemo;
	StationMemo.NoteId = TEXT("Note.StationMemo");
	StationMemo.Title = FText::FromString(TEXT("站内备忘录"));
	StationMemo.Body = FText::FromString(TEXT("上次下潜后门禁密码已更改。"));
	NoteRecorder->RegisterNoteMetadata(StationMemo);
	TestTrue(TEXT("Runtime note recorder should accept station memo id."), NoteRecorder->AddRecordedNoteId(StationMemo.NoteId));

	TestTrue(TEXT("Tab should open the Day1 notes journal."), PlayerController->HandleInputKeyForTests(EKeys::Tab));
	TestTrue(TEXT("Controller should track open notes journal after Tab."), PlayerController->IsDay1NotesJournalOpenForTests());
	TestEqual(TEXT("HUD should receive recorded note entries from the pawn note recorder."), HUD->GetNotesJournalEntryCountForTests(), 1);
	TestEqual(TEXT("HUD should receive recorded note title fallback from metadata."), HUD->GetNotesJournalTitleForTests(0).ToString(), FString(TEXT("站内备忘录")));
	TestEqual(TEXT("HUD should receive recorded note body fallback from metadata."), HUD->GetNotesJournalBodyForTests(0).ToString(), FString(TEXT("上次下潜后门禁密码已更改。")));

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorPlayerControllerWrongDoorPasswordShowsHUDReasonTest,
	"HorrorProject.Player.Controller.WrongDoorPasswordShowsHUDReason",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorPlayerControllerWrongDoorPasswordShowsHUDReasonTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for wrong password HUD feedback coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	AHorrorPlayerController* PlayerController = World->SpawnActor<AHorrorPlayerController>();
	ADay1SliceHUD* HUD = World->SpawnActor<ADay1SliceHUD>();
	ADoorInteractable* Door = World->SpawnActor<ADoorInteractable>();
	TestNotNull(TEXT("Wrong password test should spawn a horror player controller."), PlayerController);
	TestNotNull(TEXT("Wrong password test should attach the native Day1 HUD."), HUD);
	TestNotNull(TEXT("Wrong password test should spawn a password door."), Door);
	if (!PlayerController || !HUD || !Door)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	PlayerController->MyHUD = HUD;
	Door->ConfigurePassword(TEXT("9999"), FText::FromString(TEXT("最终档案密码。")));
	PlayerController->BeginDoorPasswordEntry(Door);
	TestTrue(TEXT("Password entry should be active before wrong code input."), PlayerController->IsAwaitingDoorPassword());

	TestTrue(TEXT("Journal key should be consumed while password entry is active."), PlayerController->HandleInputKeyForTests(EKeys::J));
	TestFalse(TEXT("Password entry should block the notes journal from opening underneath."), PlayerController->IsDay1NotesJournalOpenForTests());

	TestTrue(TEXT("Wrong password test should accept first digit input."), PlayerController->HandleInputKeyForTests(EKeys::One));
	TestTrue(TEXT("Wrong password test should accept second digit input."), PlayerController->HandleInputKeyForTests(EKeys::Two));
	TestTrue(TEXT("Wrong password test should accept third digit input."), PlayerController->HandleInputKeyForTests(EKeys::Three));
	TestTrue(TEXT("Wrong password test should accept fourth digit input."), PlayerController->HandleInputKeyForTests(EKeys::Four));
	TestTrue(TEXT("Enter should submit the wrong password."), PlayerController->HandleInputKeyForTests(EKeys::Enter));

	TestTrue(TEXT("Wrong password should keep password entry active."), PlayerController->IsAwaitingDoorPassword());
	TestTrue(TEXT("Wrong password should keep the password prompt visible."), HUD->IsPasswordPromptVisibleForTests());
	TestTrue(TEXT("Wrong password should show a transient HUD reason."), HUD->IsTransientMessageVisibleForTests());
	TestEqual(TEXT("Wrong password should explain the rejection."), HUD->GetTransientMessageForTests().ToString(), FString(TEXT("密码错误。")));

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorPlayerControllerDay1NoteRecordedShowsReviewPromptTest,
	"HorrorProject.Player.Controller.Day1NoteRecordedShowsReviewPrompt",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorPlayerControllerDay1NoteRecordedShowsReviewPromptTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for note-recorded HUD feedback coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	AHorrorPlayerController* PlayerController = World->SpawnActor<AHorrorPlayerController>();
	ADay1SliceHUD* HUD = World->SpawnActor<ADay1SliceHUD>();
	AHorrorPlayerCharacter* PlayerCharacter = World->SpawnActor<AHorrorPlayerCharacter>();
	TestNotNull(TEXT("Note-recorded HUD feedback test should spawn a horror player controller."), PlayerController);
	TestNotNull(TEXT("Note-recorded HUD feedback test should attach the native Day1 HUD."), HUD);
	TestNotNull(TEXT("Note-recorded HUD feedback test should spawn a horror player character."), PlayerCharacter);
	if (!PlayerController || !HUD || !PlayerCharacter)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	PlayerController->MyHUD = HUD;
	PlayerController->Possess(PlayerCharacter);

	UNoteRecorderComponent* NoteRecorder = PlayerCharacter->GetNoteRecorderComponent();
	TestNotNull(TEXT("Note-recorded HUD feedback test should expose the note recorder."), NoteRecorder);
	if (!NoteRecorder)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	TestTrue(TEXT("Possessing a horror player should bind the controller to note-recorded events."), PlayerController->IsBoundToNoteRecorderForTests(NoteRecorder));
	TestTrue(TEXT("Possessing a horror player should register the controller note-recorded delegate."), PlayerController->HasNoteRecordedDelegateForTests(NoteRecorder));
	TestTrue(TEXT("Adding a new recorded note should trigger controller HUD feedback."), NoteRecorder->AddRecordedNoteId(TEXT("Note.ObjectiveHint.Evidence.Anomaly01")));
	TestEqual(TEXT("Recorded note delegate should invoke controller feedback handler."), PlayerController->GetNoteRecordedFeedbackCountForTests(), 1);
	TestTrue(TEXT("Recorded note feedback should be visible on the native HUD."), HUD->IsTransientMessageVisibleForTests());
	TestTrue(TEXT("Recorded note feedback should tell the player how to review notes in Chinese."), HUD->GetTransientMessageForTests().ToString().Contains(TEXT("笔记键/物品栏键")));
	TestFalse(TEXT("Recorded note feedback should not expose raw English key names."), HUD->GetTransientMessageForTests().ToString().Contains(TEXT("Tab")));

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorPlayerControllerDoorPasswordHintMentionsNotesTest,
	"HorrorProject.Player.Controller.DoorPasswordHintMentionsNotes",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorPlayerControllerDoorPasswordHintMentionsNotesTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for password note hint coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	AHorrorPlayerController* PlayerController = World->SpawnActor<AHorrorPlayerController>();
	ADay1SliceHUD* HUD = World->SpawnActor<ADay1SliceHUD>();
	AHorrorPlayerCharacter* PlayerCharacter = World->SpawnActor<AHorrorPlayerCharacter>();
	ADoorInteractable* Door = World->SpawnActor<ADoorInteractable>();
	TestNotNull(TEXT("Password note hint test should spawn a horror player controller."), PlayerController);
	TestNotNull(TEXT("Password note hint test should attach the native Day1 HUD."), HUD);
	TestNotNull(TEXT("Password note hint test should spawn a horror player character."), PlayerCharacter);
	TestNotNull(TEXT("Password note hint test should spawn a password door."), Door);
	if (!PlayerController || !HUD || !PlayerCharacter || !Door)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	PlayerController->MyHUD = HUD;
	PlayerController->Possess(PlayerCharacter);

	UNoteRecorderComponent* NoteRecorder = PlayerCharacter->GetNoteRecorderComponent();
	TestNotNull(TEXT("Password note hint test should expose the note recorder."), NoteRecorder);
	if (!NoteRecorder)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}
	TestTrue(TEXT("Password note hint test should record a clue note."), NoteRecorder->AddRecordedNoteId(TEXT("Note.Intro")));

	Door->ConfigurePassword(TEXT("0417"), FText::FromString(TEXT("最终档案密码。")));
	PlayerController->BeginDoorPasswordEntry(Door);

	const FString Hint = HUD->GetPasswordHintForTests().ToString();
	TestTrue(TEXT("Password prompt should keep the door-specific hint."), Hint.Contains(TEXT("最终档案密码。")));
	TestTrue(TEXT("Password prompt should point players back to recorded notes in Chinese."), Hint.Contains(TEXT("笔记键/物品栏键")));
	TestFalse(TEXT("Password prompt should not expose raw English key names."), Hint.Contains(TEXT("Tab")));

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorPlayerControllerDay1HUDFeedbackEventsTest,
	"HorrorProject.Player.Controller.Day1HUDFeedbackEvents",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorPlayerControllerDay1HUDFeedbackEventsTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for Day1 HUD feedback event coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	AHorrorPlayerController* PlayerController = World->SpawnActor<AHorrorPlayerController>();
	TestNotNull(TEXT("HUD feedback event test should spawn a horror player controller."), PlayerController);
	if (!PlayerController)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	ADay1SliceHUD* HUD = World->SpawnActor<ADay1SliceHUD>();
	PlayerController->MyHUD = HUD;
	TestNotNull(TEXT("HUD feedback event test should attach the native Day1 HUD."), HUD);
	if (!HUD)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	UHorrorEventBusSubsystem* EventBus = World->GetSubsystem<UHorrorEventBusSubsystem>();
	TestNotNull(TEXT("HUD feedback event test should expose the event bus."), EventBus);
	if (!EventBus)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	PlayerController->BindObjectiveEventBusForTests();
	EventBus->Publish(
		HorrorFoundFootageTags::BodycamAcquiredEvent(),
		TEXT("Bodycam"),
		HorrorFoundFootageTags::BodycamAcquiredState(),
		PlayerController);
	TestFalse(TEXT("Objective milestone events should not claim a checkpoint was saved before the save event."), HUD->IsAutosaveIndicatorVisibleForTests());

	EventBus->Publish(
		HorrorSaveTags::CheckpointSavedEvent(),
		TEXT("Checkpoint.Day1.BodycamAcquired"),
		HorrorSaveTags::CheckpointSavedState(),
		PlayerController);
	TestTrue(TEXT("Checkpoint saved events should show an autosave HUD indicator."), HUD->IsAutosaveIndicatorVisibleForTests());

	const FGameplayTag ObjectiveFailedTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Event.Campaign.ObjectiveFailed")), false);
	FHorrorObjectiveMessageMetadata FailureMetadata;
	FailureMetadata.DebugLabel = FText::FromString(TEXT("目标失败"));
	FailureMetadata.ObjectiveHint = FText::FromString(TEXT("接线稳定度耗尽，重新互动可重试。"));
	EventBus->RegisterObjectiveMetadata(ObjectiveFailedTag, TEXT("Test.FailedObjective"), FailureMetadata);
	EventBus->Publish(
		ObjectiveFailedTag,
		TEXT("Test.FailedObjective"),
		ObjectiveFailedTag,
		PlayerController);
	TestEqual(TEXT("Objective failure toast should be visible before lower-priority feedback."), HUD->GetObjectiveToastTitleForTests().ToString(), FString(TEXT("目标失败")));

	EventBus->Publish(
		HorrorSaveTags::CheckpointSavedEvent(),
		TEXT("Checkpoint.Day1.AfterFailure"),
		HorrorSaveTags::CheckpointSavedState(),
		PlayerController);
	TestTrue(TEXT("Autosave indicator should still show while a failure toast is active."), HUD->IsAutosaveIndicatorVisibleForTests());
	TestEqual(TEXT("Autosave feedback should not overwrite a higher-priority objective failure toast."), HUD->GetObjectiveToastTitleForTests().ToString(), FString(TEXT("目标失败")));
	EventBus->UnregisterObjectiveMetadata(ObjectiveFailedTag, TEXT("Test.FailedObjective"));

	const FGameplayTag ObjectiveCompletedTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Event.Campaign.ObjectiveCompleted")), false);
	FHorrorObjectiveMessageMetadata CompletionMetadata;
	CompletionMetadata.DebugLabel = FText::FromString(TEXT("目标完成"));
	CompletionMetadata.ObjectiveHint = FText::FromString(TEXT("下一目标已解锁。"));
	EventBus->RegisterObjectiveMetadata(ObjectiveCompletedTag, TEXT("Test.CompletedObjective"), CompletionMetadata);
	EventBus->Publish(
		ObjectiveCompletedTag,
		TEXT("Test.CompletedObjective"),
		ObjectiveCompletedTag,
		PlayerController);
	TestEqual(TEXT("Lower-priority completion toast should queue behind active failure toast."), HUD->GetObjectiveToastTitleForTests().ToString(), FString(TEXT("目标失败")));
	TestEqual(TEXT("Lower-priority completion toast should be retained in the objective notification queue."), PlayerController->GetQueuedObjectiveNotificationCountForTests(), 1);
	TestTrue(TEXT("Queued completion feedback should be visible after the active failure toast expires."), PlayerController->UpdateDay1RuntimeStateForTests(5.5f));
	TestEqual(TEXT("Queued completion feedback should become the next objective toast."), HUD->GetObjectiveToastTitleForTests().ToString(), FString(TEXT("目标完成")));
	TestEqual(TEXT("Queued completion feedback should preserve its hint."), HUD->GetObjectiveToastHintForTests().ToString(), FString(TEXT("下一目标已解锁。")));
	EventBus->UnregisterObjectiveMetadata(ObjectiveCompletedTag, TEXT("Test.CompletedObjective"));

	EventBus->Publish(
		HorrorDay1Tags::PlayerFailureEvent(),
		TEXT("Death.Test"),
		HorrorDay1Tags::CheckpointRestoredState(),
		PlayerController);
	TestTrue(TEXT("Checkpoint restored failure events should show transient HUD feedback."), HUD->IsTransientMessageVisibleForTests());
	TestEqual(TEXT("Checkpoint restored feedback should be explicit."), HUD->GetTransientMessageForTests().ToString(), FString(TEXT("已恢复到上一个检查点。")));

	EventBus->Publish(
		HorrorDay1Tags::PlayerFailureEvent(),
		TEXT("Death.Test"),
		HorrorDay1Tags::CheckpointMissingState(),
		PlayerController);
	TestTrue(TEXT("Missing-checkpoint failure events should show transient HUD feedback."), HUD->IsTransientMessageVisibleForTests());
	TestEqual(TEXT("Missing-checkpoint feedback should be explicit."), HUD->GetTransientMessageForTests().ToString(), FString(TEXT("没有可用检查点。")));

	const FGameplayTag CircuitFailureTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Event.Campaign.Advanced.Circuit.Failure")), false);
	const FGameplayTag CircuitSuccessTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Event.Campaign.Advanced.Circuit.Success")), false);
	EventBus->Publish(CircuitFailureTag, TEXT("Test.Circuit"), CircuitFailureTag, PlayerController);
	TestTrue(TEXT("Advanced circuit failure should show explicit transient device feedback."), HUD->IsTransientMessageVisibleForTests());
	TestTrue(TEXT("Advanced circuit failure feedback should mention wiring failure."), HUD->GetTransientMessageForTests().ToString().Contains(TEXT("接线")));
	TestFalse(TEXT("Advanced circuit failure should not fall back to a generic objective toast."), HUD->GetCurrentObjectiveForTests().ToString().Contains(TEXT("目标已更新")));

	EventBus->Publish(CircuitSuccessTag, TEXT("Test.Circuit"), CircuitSuccessTag, PlayerController);
	TestTrue(TEXT("Advanced circuit success should show explicit transient device feedback."), HUD->IsTransientMessageVisibleForTests());
	TestTrue(TEXT("Advanced circuit success feedback should mention a successful wire arc."), HUD->GetTransientMessageForTests().ToString().Contains(TEXT("电弧")));

	const FGameplayTag SpectralScanFailureTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Event.Campaign.Advanced.SpectralScan.Failure")), false);
	const FGameplayTag SpectralScanSuccessTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Event.Campaign.Advanced.SpectralScan.Success")), false);
	EventBus->Publish(SpectralScanFailureTag, TEXT("Test.Scan"), SpectralScanFailureTag, PlayerController);
	TestTrue(TEXT("Advanced spectral scan failure should show scan-specific transient feedback."), HUD->GetTransientMessageForTests().ToString().Contains(TEXT("扫描")));
	TestFalse(TEXT("Advanced spectral scan failure should not mention wiring."), HUD->GetTransientMessageForTests().ToString().Contains(TEXT("接线")));
	TestFalse(TEXT("Advanced spectral scan failure should not mention gears."), HUD->GetTransientMessageForTests().ToString().Contains(TEXT("齿轮")));

	EventBus->Publish(SpectralScanSuccessTag, TEXT("Test.Scan"), SpectralScanSuccessTag, PlayerController);
	TestTrue(TEXT("Advanced spectral scan success should show scan-specific transient feedback."), HUD->GetTransientMessageForTests().ToString().Contains(TEXT("波段")) || HUD->GetTransientMessageForTests().ToString().Contains(TEXT("频谱")));
	TestFalse(TEXT("Advanced spectral scan success should not mention wiring."), HUD->GetTransientMessageForTests().ToString().Contains(TEXT("接线")));

	const FGameplayTag SignalTuningFailureTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Event.Campaign.Advanced.SignalTuning.Failure")), false);
	const FGameplayTag SignalTuningSuccessTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Event.Campaign.Advanced.SignalTuning.Success")), false);
	EventBus->Publish(SignalTuningFailureTag, TEXT("Test.Tuning"), SignalTuningFailureTag, PlayerController);
	TestTrue(TEXT("Advanced signal tuning failure should show tuning-specific transient feedback."), HUD->GetTransientMessageForTests().ToString().Contains(TEXT("调谐")));
	TestFalse(TEXT("Advanced signal tuning failure should not mention wiring."), HUD->GetTransientMessageForTests().ToString().Contains(TEXT("接线")));
	TestFalse(TEXT("Advanced signal tuning failure should not mention gears."), HUD->GetTransientMessageForTests().ToString().Contains(TEXT("齿轮")));

	EventBus->Publish(SignalTuningSuccessTag, TEXT("Test.Tuning"), SignalTuningSuccessTag, PlayerController);
	TestTrue(TEXT("Advanced signal tuning success should show frequency-specific transient feedback."), HUD->GetTransientMessageForTests().ToString().Contains(TEXT("频率")) || HUD->GetTransientMessageForTests().ToString().Contains(TEXT("回放")));
	TestFalse(TEXT("Advanced signal tuning success should not mention wiring."), HUD->GetTransientMessageForTests().ToString().Contains(TEXT("接线")));

	EventBus->Publish(HorrorDay1Tags::Day1CompletedEvent(), TEXT("Exit.Gate"), HorrorDay1Tags::Day1CompletedState(), PlayerController);
	TestTrue(TEXT("Day1 completion should show the cut-to-black completion overlay."), HUD->IsDay1CompletionOverlayVisibleForTests());
	TestEqual(TEXT("Day1 completion should use an explicit completion title."), HUD->GetDay1CompletionTitleForTests().ToString(), FString(TEXT("第 1 天完成")));

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorPlayerControllerRoutesAdvancedInteractionDeviceKeysTest,
	"HorrorProject.Player.Controller.RoutesAdvancedInteractionDeviceKeys",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorPlayerControllerRoutesAdvancedInteractionDeviceKeysTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for advanced interaction input coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	AHorrorPlayerController* PlayerController = World->SpawnActor<AHorrorPlayerController>();
	AHorrorPlayerCharacter* PlayerCharacter = World->SpawnActor<AHorrorPlayerCharacter>();
	ADay1SliceHUD* HUD = World->SpawnActor<ADay1SliceHUD>();
	AHorrorCampaignObjectiveActor* ObjectiveActor = World->SpawnActor<AHorrorCampaignObjectiveActor>();
	TestNotNull(TEXT("Advanced input test should spawn a player controller."), PlayerController);
	TestNotNull(TEXT("Advanced input test should spawn a player character."), PlayerCharacter);
	TestNotNull(TEXT("Advanced input test should spawn the Day1 HUD."), HUD);
	TestNotNull(TEXT("Advanced input test should spawn a campaign objective actor."), ObjectiveActor);
	if (!PlayerController || !PlayerCharacter || !HUD || !ObjectiveActor)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	PlayerController->PlayerState = World->SpawnActor<APlayerState>();
	World->AddController(PlayerController);
	PlayerController->MyHUD = HUD;
	PlayerController->Possess(PlayerCharacter);

	FHorrorCampaignObjectiveDefinition Objective;
	Objective.ObjectiveId = TEXT("Test.ControllerCircuit");
	Objective.ObjectiveType = EHorrorCampaignObjectiveType::RestorePower;
	Objective.InteractionMode = EHorrorCampaignInteractionMode::CircuitWiring;
	Objective.PromptText = FText::FromString(TEXT("接入控制器测试电路"));
	Objective.CompletionText = FText::FromString(TEXT("控制器测试电路恢复。"));
	ObjectiveActor->ConfigureObjective(TEXT("Chapter.Test"), Objective);
	const FHitResult EmptyHit;
	TestTrue(TEXT("Opening the circuit task should activate advanced interaction."), ObjectiveActor->Interact_Implementation(PlayerCharacter, EmptyHit));
	PlayerController->SetActiveAdvancedInteractionObjectiveForTests(ObjectiveActor);

	ObjectiveActor->Tick(0.65f);
	const FName FirstExpectedInput = ObjectiveActor->GetExpectedAdvancedInputId();
	TestEqual(TEXT("The first circuit target should be the blue terminal."), FirstExpectedInput, FName(TEXT("蓝色端子")));
	TestTrue(TEXT("A should be consumed by the advanced interaction panel."), PlayerController->HandleInputKeyForTests(EKeys::A));
	TestTrue(TEXT("A should advance circuit progress when the timing window is open."), ObjectiveActor->GetAdvancedInteractionProgressFraction() > 0.0f);

	ObjectiveActor->Tick(0.65f);
	TestTrue(TEXT("A should be consumed even when it is the wrong animated input."), PlayerController->HandleInputKeyForTests(EKeys::A));
	TestTrue(TEXT("Wrong animated input should create red-spark feedback."), ObjectiveActor->GetAdvancedInteractionFeedbackText().ToString().Contains(TEXT("红色火花")));

	PlayerController->RefreshDay1HUDStateForTests();
	TestTrue(TEXT("HUD should show the animated advanced interaction panel while the task is active."), HUD->IsAdvancedInteractionPanelVisibleForTests());
	TestEqual(TEXT("HUD should receive the circuit interaction mode."), HUD->GetAdvancedInteractionModeForTests(), EHorrorCampaignInteractionMode::CircuitWiring);

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorPlayerControllerRequiresExplicitAdvancedInteractionSelectionTest,
	"HorrorProject.Player.Controller.RequiresExplicitAdvancedInteractionSelection",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorPlayerControllerRoutesSignalTuningKeysAsSemanticCommandsTest,
	"HorrorProject.Player.Controller.RoutesSignalTuningKeysAsSemanticCommands",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorPlayerControllerRoutesSpectralScanKeysAsSemanticCommandsTest,
	"HorrorProject.Player.Controller.RoutesSpectralScanKeysAsSemanticCommands",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorPlayerControllerRoutesSpectralScanKeysAsSemanticCommandsTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for spectral scan command routing."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	AHorrorPlayerController* PlayerController = World->SpawnActor<AHorrorPlayerController>();
	AHorrorPlayerCharacter* PlayerCharacter = World->SpawnActor<AHorrorPlayerCharacter>();
	ADay1SliceHUD* HUD = World->SpawnActor<ADay1SliceHUD>();
	AHorrorCampaignObjectiveActor* ObjectiveActor = World->SpawnActor<AHorrorCampaignObjectiveActor>();
	TestNotNull(TEXT("Spectral command test should spawn a player controller."), PlayerController);
	TestNotNull(TEXT("Spectral command test should spawn a player character."), PlayerCharacter);
	TestNotNull(TEXT("Spectral command test should spawn the Day1 HUD."), HUD);
	TestNotNull(TEXT("Spectral command test should spawn a campaign objective actor."), ObjectiveActor);
	if (!PlayerController || !PlayerCharacter || !HUD || !ObjectiveActor)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	PlayerController->PlayerState = World->SpawnActor<APlayerState>();
	World->AddController(PlayerController);
	PlayerController->MyHUD = HUD;
	PlayerController->Possess(PlayerCharacter);

	FHorrorCampaignObjectiveDefinition Objective;
	Objective.ObjectiveId = TEXT("Test.ControllerSpectralScan");
	Objective.ObjectiveType = EHorrorCampaignObjectiveType::ScanAnomaly;
	Objective.InteractionMode = EHorrorCampaignInteractionMode::SpectralScan;
	Objective.PromptText = FText::FromString(TEXT("扫描控制器测试黑盒"));
	Objective.CompletionText = FText::FromString(TEXT("控制器测试频谱归档。"));
	Objective.AdvancedInteractionTuning.RequiredStepCount = 3;
	Objective.AdvancedInteractionTuning.SuccessProgress = 0.34f;
	ObjectiveActor->ConfigureObjective(TEXT("Chapter.Test"), Objective);

	const FHitResult EmptyHit;
	TestTrue(TEXT("Opening the spectral scan task should activate advanced interaction."), ObjectiveActor->Interact_Implementation(PlayerCharacter, EmptyHit));
	PlayerController->SetActiveAdvancedInteractionObjectiveForTests(ObjectiveActor);
	ObjectiveActor->Tick(0.9f);

	FHorrorAdvancedInteractionHUDState ScanState = ObjectiveActor->BuildAdvancedInteractionHUDState();
	TestTrue(TEXT("Spectral scan should start offset so the controller cannot solve it by pressing S immediately."), !FMath::IsNearlyEqual(ScanState.SpectralFilterFocusFraction, ScanState.SpectralTargetFocusFraction, 0.02f));
	TestTrue(TEXT("S should be consumed but rejected while the spectral filter is off-peak."), PlayerController->HandleInputKeyForTests(EKeys::S));
	TestEqual(TEXT("Off-peak S should not advance spectral progress."), ObjectiveActor->GetAdvancedInteractionProgressFraction(), 0.0f);
	TestTrue(TEXT("Off-peak S feedback should mention spectral filtering."), ObjectiveActor->GetAdvancedInteractionFeedbackText().ToString().Contains(TEXT("滤波")) || ObjectiveActor->GetAdvancedInteractionFeedbackText().ToString().Contains(TEXT("扫频")));

	for (int32 AttemptIndex = 0; AttemptIndex < 5; ++AttemptIndex)
	{
		ScanState = ObjectiveActor->BuildAdvancedInteractionHUDState();
		if (FMath::Abs(ScanState.SpectralFilterFocusFraction - ScanState.SpectralTargetFocusFraction) <= 0.16f)
		{
			break;
		}

		const FKey CorrectiveKey = ScanState.SpectralFilterFocusFraction < ScanState.SpectralTargetFocusFraction ? EKeys::D : EKeys::A;
		TestTrue(TEXT("Corrective spectral key should be consumed as active sweep input."), PlayerController->HandleInputKeyForTests(CorrectiveKey));
		TestTrue(TEXT("Corrective spectral key should route to the sweep command path."), ObjectiveActor->GetAdvancedInteractionFeedbackText().ToString().Contains(TEXT("扫频")));
		TestEqual(TEXT("Active spectral sweep should not advance objective progress."), ObjectiveActor->GetAdvancedInteractionProgressFraction(), 0.0f);
	}
	ScanState = ObjectiveActor->BuildAdvancedInteractionHUDState();
	TestTrue(TEXT("Corrective controller input should align the spectral filter before center lock."), FMath::Abs(ScanState.SpectralFilterFocusFraction - ScanState.SpectralTargetFocusFraction) <= 0.16f);

	ObjectiveActor->Tick(0.65f);
	TestTrue(TEXT("S should lock the spectral peak once the filter is aligned."), PlayerController->HandleInputKeyForTests(EKeys::S));
	TestTrue(TEXT("Center lock should advance spectral scan progress."), ObjectiveActor->GetAdvancedInteractionProgressFraction() > 0.0f);
	TestTrue(TEXT("Center lock feedback should mention spectral locking."), ObjectiveActor->GetAdvancedInteractionFeedbackText().ToString().Contains(TEXT("锁定")));

	PlayerController->RefreshDay1HUDStateForTests();
	TestTrue(TEXT("HUD should keep the spectral scan advanced panel visible after semantic commands."), HUD->IsAdvancedInteractionPanelVisibleForTests());
	TestEqual(TEXT("HUD should receive the spectral scan interaction mode."), HUD->GetAdvancedInteractionModeForTests(), EHorrorCampaignInteractionMode::SpectralScan);

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

bool FHorrorPlayerControllerRoutesSignalTuningKeysAsSemanticCommandsTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for signal tuning command routing."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	AHorrorPlayerController* PlayerController = World->SpawnActor<AHorrorPlayerController>();
	AHorrorPlayerCharacter* PlayerCharacter = World->SpawnActor<AHorrorPlayerCharacter>();
	ADay1SliceHUD* HUD = World->SpawnActor<ADay1SliceHUD>();
	AHorrorCampaignObjectiveActor* ObjectiveActor = World->SpawnActor<AHorrorCampaignObjectiveActor>();
	TestNotNull(TEXT("Signal command test should spawn a player controller."), PlayerController);
	TestNotNull(TEXT("Signal command test should spawn a player character."), PlayerCharacter);
	TestNotNull(TEXT("Signal command test should spawn the Day1 HUD."), HUD);
	TestNotNull(TEXT("Signal command test should spawn a campaign objective actor."), ObjectiveActor);
	if (!PlayerController || !PlayerCharacter || !HUD || !ObjectiveActor)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	PlayerController->PlayerState = World->SpawnActor<APlayerState>();
	World->AddController(PlayerController);
	PlayerController->MyHUD = HUD;
	PlayerController->Possess(PlayerCharacter);

	FHorrorCampaignObjectiveDefinition Objective;
	Objective.ObjectiveId = TEXT("Test.ControllerSignalTuning");
	Objective.ObjectiveType = EHorrorCampaignObjectiveType::AcquireSignal;
	Objective.InteractionMode = EHorrorCampaignInteractionMode::SignalTuning;
	Objective.PromptText = FText::FromString(TEXT("调谐控制器测试黑盒"));
	Objective.CompletionText = FText::FromString(TEXT("控制器测试黑盒同步。"));
	Objective.AdvancedInteractionTuning.RequiredStepCount = 3;
	Objective.AdvancedInteractionTuning.SuccessProgress = 0.34f;
	ObjectiveActor->ConfigureObjective(TEXT("Chapter.Test"), Objective);

	const FHitResult EmptyHit;
	TestTrue(TEXT("Opening the signal tuning task should activate advanced interaction."), ObjectiveActor->Interact_Implementation(PlayerCharacter, EmptyHit));
	PlayerController->SetActiveAdvancedInteractionObjectiveForTests(ObjectiveActor);
	ObjectiveActor->Tick(0.65f);

	FHorrorAdvancedInteractionHUDState SignalState = ObjectiveActor->BuildAdvancedInteractionHUDState();
	TestTrue(TEXT("Signal tuning should start offset so the controller cannot solve it by pressing S immediately."), !FMath::IsNearlyEqual(SignalState.SignalBalanceFraction, SignalState.SignalTargetBalanceFraction, 0.02f));
	TestTrue(TEXT("S should be consumed but rejected while the signal is off-center."), PlayerController->HandleInputKeyForTests(EKeys::S));
	TestEqual(TEXT("Off-center S should not advance signal progress."), ObjectiveActor->GetAdvancedInteractionProgressFraction(), 0.0f);
	TestTrue(TEXT("Off-center S feedback should mention signal balance."), ObjectiveActor->GetAdvancedInteractionFeedbackText().ToString().Contains(TEXT("声像")));

	const FKey CorrectiveKey = SignalState.SignalBalanceFraction < SignalState.SignalTargetBalanceFraction ? EKeys::D : EKeys::A;
	for (int32 AttemptIndex = 0; AttemptIndex < 8; ++AttemptIndex)
	{
		SignalState = ObjectiveActor->BuildAdvancedInteractionHUDState();
		if (FMath::Abs(SignalState.SignalBalanceFraction - SignalState.SignalTargetBalanceFraction) <= 0.12f)
		{
			break;
		}

		TestTrue(TEXT("Corrective key should be consumed as continuous tuning inside the signal panel."), PlayerController->HandleInputKeyForTests(CorrectiveKey));
		TestTrue(TEXT("Corrective key should route to the continuous command path, not the old discrete select path."), ObjectiveActor->GetAdvancedInteractionFeedbackText().ToString().Contains(TEXT("连续微调")));
		TestEqual(TEXT("Continuous tuning should not advance signal progress."), ObjectiveActor->GetAdvancedInteractionProgressFraction(), 0.0f);
	}
	SignalState = ObjectiveActor->BuildAdvancedInteractionHUDState();
	TestTrue(TEXT("Corrective controller input should align the signal before center confirmation."), FMath::Abs(SignalState.SignalBalanceFraction - SignalState.SignalTargetBalanceFraction) <= 0.12f);

	ObjectiveActor->Tick(0.65f);
	TestTrue(TEXT("S should confirm the center frequency once the signal is aligned."), PlayerController->HandleInputKeyForTests(EKeys::S));
	TestTrue(TEXT("Center confirmation should advance signal tuning progress."), ObjectiveActor->GetAdvancedInteractionProgressFraction() > 0.0f);
	TestTrue(TEXT("Center confirmation feedback should mention frequency alignment."), ObjectiveActor->GetAdvancedInteractionFeedbackText().ToString().Contains(TEXT("频率")));

	PlayerController->RefreshDay1HUDStateForTests();
	TestTrue(TEXT("HUD should keep the signal tuning advanced panel visible after semantic commands."), HUD->IsAdvancedInteractionPanelVisibleForTests());
	TestEqual(TEXT("HUD should receive the signal tuning interaction mode."), HUD->GetAdvancedInteractionModeForTests(), EHorrorCampaignInteractionMode::SignalTuning);

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

bool FHorrorPlayerControllerRequiresExplicitAdvancedInteractionSelectionTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for explicit advanced interaction selection coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	AHorrorPlayerController* PlayerController = World->SpawnActor<AHorrorPlayerController>();
	AHorrorPlayerCharacter* PlayerCharacter = World->SpawnActor<AHorrorPlayerCharacter>();
	ADay1SliceHUD* HUD = World->SpawnActor<ADay1SliceHUD>();
	AHorrorCampaignObjectiveActor* ObjectiveActor = World->SpawnActor<AHorrorCampaignObjectiveActor>();
	TestNotNull(TEXT("Explicit selection test should spawn a player controller."), PlayerController);
	TestNotNull(TEXT("Explicit selection test should spawn a player character."), PlayerCharacter);
	TestNotNull(TEXT("Explicit selection test should spawn the Day1 HUD."), HUD);
	TestNotNull(TEXT("Explicit selection test should spawn a campaign objective actor."), ObjectiveActor);
	if (!PlayerController || !PlayerCharacter || !HUD || !ObjectiveActor)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	PlayerController->PlayerState = World->SpawnActor<APlayerState>();
	World->AddController(PlayerController);
	PlayerController->MyHUD = HUD;
	PlayerController->Possess(PlayerCharacter);

	FHorrorCampaignObjectiveDefinition Objective;
	Objective.ObjectiveId = TEXT("Test.ExplicitCircuitSelection");
	Objective.ObjectiveType = EHorrorCampaignObjectiveType::RestorePower;
	Objective.InteractionMode = EHorrorCampaignInteractionMode::CircuitWiring;
	Objective.PromptText = FText::FromString(TEXT("验证显式接线选择"));
	Objective.CompletionText = FText::FromString(TEXT("显式接线验证完成。"));
	ObjectiveActor->ConfigureObjective(TEXT("Chapter.Test"), Objective);

	const FHitResult EmptyHit;
	TestTrue(TEXT("Opening the circuit task should activate advanced interaction."), ObjectiveActor->Interact_Implementation(PlayerCharacter, EmptyHit));
	PlayerController->SetActiveAdvancedInteractionObjectiveForTests(ObjectiveActor);
	ObjectiveActor->Tick(0.65f);

	TestTrue(TEXT("E should be consumed by the advanced panel without auto-selecting the correct terminal."), PlayerController->HandleInputKeyForTests(EKeys::E));
	TestEqual(TEXT("E should not advance circuit progress because the player must choose a terminal."), ObjectiveActor->GetAdvancedInteractionProgressFraction(), 0.0f);
	TestTrue(TEXT("E should show an explicit selection prompt instead of success feedback."), ObjectiveActor->GetAdvancedInteractionFeedbackText().ToString().Contains(TEXT("A/S/D")));

	ObjectiveActor->Tick(0.65f);
	TestTrue(TEXT("Space should be consumed by the advanced panel without auto-selecting the correct terminal."), PlayerController->HandleInputKeyForTests(EKeys::SpaceBar));
	TestEqual(TEXT("Space should not advance circuit progress because the player must choose a terminal."), ObjectiveActor->GetAdvancedInteractionProgressFraction(), 0.0f);

	ObjectiveActor->Tick(0.65f);
	TestTrue(TEXT("A should still submit the explicitly selected blue terminal."), PlayerController->HandleInputKeyForTests(EKeys::A));
	TestTrue(TEXT("Explicit device-key selection should advance circuit progress in the timing window."), ObjectiveActor->GetAdvancedInteractionProgressFraction() > 0.0f);

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorPlayerControllerKeepsAdvancedInteractionPanelAfterWorldInteractionTest,
	"HorrorProject.Player.Controller.KeepsAdvancedInteractionPanelAfterWorldInteraction",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorPlayerControllerKeepsAdvancedInteractionPanelVisibleForRetryableFailureTest,
	"HorrorProject.Player.Controller.KeepsAdvancedInteractionPanelVisibleForRetryableFailure",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorPlayerControllerKeepsAdvancedInteractionPanelAfterWorldInteractionTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for opened advanced panel coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	AHorrorPlayerController* PlayerController = World->SpawnActor<AHorrorPlayerController>();
	AHorrorPlayerCharacter* PlayerCharacter = World->SpawnActor<AHorrorPlayerCharacter>();
	ADay1SliceHUD* HUD = World->SpawnActor<ADay1SliceHUD>();
	AHorrorCampaignObjectiveActor* ObjectiveActor = World->SpawnActor<AHorrorCampaignObjectiveActor>();
	TestNotNull(TEXT("Opened advanced panel test should spawn a player controller."), PlayerController);
	TestNotNull(TEXT("Opened advanced panel test should spawn a player character."), PlayerCharacter);
	TestNotNull(TEXT("Opened advanced panel test should spawn the Day1 HUD."), HUD);
	TestNotNull(TEXT("Opened advanced panel test should spawn a campaign objective actor."), ObjectiveActor);
	if (!PlayerController || !PlayerCharacter || !HUD || !ObjectiveActor)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	PlayerController->PlayerState = World->SpawnActor<APlayerState>();
	World->AddController(PlayerController);
	PlayerController->MyHUD = HUD;
	PlayerController->Possess(PlayerCharacter);

	FHorrorCampaignObjectiveDefinition Objective;
	Objective.ObjectiveId = TEXT("Test.PersistentCircuitPanel");
	Objective.ObjectiveType = EHorrorCampaignObjectiveType::RestorePower;
	Objective.InteractionMode = EHorrorCampaignInteractionMode::CircuitWiring;
	Objective.PromptText = FText::FromString(TEXT("接入持久窗口测试电路"));
	Objective.CompletionText = FText::FromString(TEXT("持久窗口测试电路恢复。"));
	ObjectiveActor->ConfigureObjective(TEXT("Chapter.Test"), Objective);

	FHitResult ObjectiveHit;
	ObjectiveHit.HitObjectHandle = FActorInstanceHandle(ObjectiveActor);
	ObjectiveHit.Component = ObjectiveActor->GetInteractionBoundsForTests();
	ObjectiveHit.ImpactPoint = ObjectiveActor->GetActorLocation();
	UInteractionComponent* Interaction = PlayerCharacter->GetInteractionComponent();
	TestNotNull(TEXT("Opened advanced panel test player should expose interaction."), Interaction);
	if (!Interaction)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	TestTrue(TEXT("World interaction should open the circuit task."), Interaction->TryInteractWithHit(ObjectiveHit));
	TestTrue(TEXT("World interaction should activate the advanced objective before controller caching."), ObjectiveActor->IsAdvancedInteractionActive());
	AHorrorCampaignObjectiveActor* CachedObjective = PlayerController->GetActiveAdvancedInteractionObjectiveForTests();
	TestNotNull(TEXT("Opening an advanced world task should cache an active objective on the player controller."), CachedObjective);
	TestTrue(TEXT("Opening an advanced world task should cache the same objective on the player controller."), CachedObjective == ObjectiveActor);

	PlayerController->RefreshDay1HUDStateForTests();
	TestTrue(TEXT("HUD should keep the advanced panel visible after the opening interaction."), HUD->IsAdvancedInteractionPanelVisibleForTests());
	TestTrue(TEXT("Opening an advanced panel should clear the ordinary focused interaction prompt."), HUD->GetInteractionPromptForTests().IsEmpty());
	TestFalse(TEXT("Opening an advanced panel should hide the ordinary focused interaction card."), HUD->IsInteractionContextVisibleForTests());

	ObjectiveActor->Tick(0.65f);
	TestTrue(TEXT("A input should route to the cached advanced panel without needing to refocus the actor."), PlayerController->HandleInputKeyForTests(EKeys::A));
	TestTrue(TEXT("Cached advanced panel input should advance the task."), ObjectiveActor->GetAdvancedInteractionProgressFraction() > 0.0f);

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

bool FHorrorPlayerControllerKeepsAdvancedInteractionPanelVisibleForRetryableFailureTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for retryable advanced panel coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	AHorrorPlayerController* PlayerController = World->SpawnActor<AHorrorPlayerController>();
	AHorrorPlayerCharacter* PlayerCharacter = World->SpawnActor<AHorrorPlayerCharacter>();
	ADay1SliceHUD* HUD = World->SpawnActor<ADay1SliceHUD>();
	AHorrorCampaignObjectiveActor* ObjectiveActor = World->SpawnActor<AHorrorCampaignObjectiveActor>();
	TestNotNull(TEXT("Retryable advanced panel test should spawn a player controller."), PlayerController);
	TestNotNull(TEXT("Retryable advanced panel test should spawn a player character."), PlayerCharacter);
	TestNotNull(TEXT("Retryable advanced panel test should spawn the Day1 HUD."), HUD);
	TestNotNull(TEXT("Retryable advanced panel test should spawn a campaign objective actor."), ObjectiveActor);
	if (!PlayerController || !PlayerCharacter || !HUD || !ObjectiveActor)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	PlayerController->PlayerState = World->SpawnActor<APlayerState>();
	World->AddController(PlayerController);
	PlayerController->MyHUD = HUD;
	PlayerController->Possess(PlayerCharacter);

	FHorrorCampaignObjectiveDefinition Objective;
	Objective.ObjectiveId = TEXT("Test.ControllerRetryableSignalTuning");
	Objective.ObjectiveType = EHorrorCampaignObjectiveType::AcquireSignal;
	Objective.InteractionMode = EHorrorCampaignInteractionMode::SignalTuning;
	Objective.PromptText = FText::FromString(TEXT("调谐失败保留窗口测试"));
	Objective.CompletionText = FText::FromString(TEXT("调谐失败窗口测试完成。"));
	Objective.AdvancedInteractionTuning.RequiredStepCount = 3;
	Objective.AdvancedInteractionTuning.SuccessProgress = 0.34f;
	Objective.AdvancedInteractionTuning.FailureStabilityDamage = 1.0f;
	ObjectiveActor->ConfigureObjective(TEXT("Chapter.Test"), Objective);

	const FHitResult EmptyHit;
	TestTrue(TEXT("Opening the signal tuning task should activate advanced interaction."), ObjectiveActor->Interact_Implementation(PlayerCharacter, EmptyHit));
	PlayerController->SetActiveAdvancedInteractionObjectiveForTests(ObjectiveActor);
	ObjectiveActor->Tick(0.9f);

	TestTrue(TEXT("Left tuning should move the signal away from center."), ObjectiveActor->SubmitAdvancedInteractionInput(TEXT("左声道"), PlayerCharacter));
	TestTrue(TEXT("Confirming an offset signal should overload the task into a retryable failure."), ObjectiveActor->SubmitAdvancedInteractionInput(TEXT("中心频率"), PlayerCharacter));
	TestFalse(TEXT("Retryable failure should close the live advanced interaction loop."), ObjectiveActor->IsAdvancedInteractionActive());
	TestEqual(TEXT("Signal tuning retryable failure should surface in runtime state."), ObjectiveActor->BuildObjectiveRuntimeState().Status, EHorrorCampaignObjectiveRuntimeStatus::FailedRetryable);

	PlayerController->RefreshDay1HUDStateForTests();
	TestTrue(TEXT("HUD should keep the advanced panel visible for retryable failures."), HUD->IsAdvancedInteractionPanelVisibleForTests());
	TestEqual(TEXT("HUD should preserve the failed signal tuning mode."), HUD->GetAdvancedInteractionModeForTests(), EHorrorCampaignInteractionMode::SignalTuning);
	TestEqual(TEXT("HUD should mark retryable failures as overloaded."), HUD->GetAdvancedInteractionFeedbackStateForTests(), EHorrorAdvancedInteractionFeedbackState::Overloaded);
	TestTrue(TEXT("HUD should explain the failed device in Chinese."), HUD->GetAdvancedInteractionDeviceStatusForTests().ToString().Contains(TEXT("过载")));
	TestTrue(TEXT("HUD should tell the player how to restart."), HUD->GetAdvancedInteractionNextActionForTests().ToString().Contains(TEXT("重新互动")));
	TestTrue(TEXT("HUD should keep recovery guidance visible after failure."), HUD->GetAdvancedInteractionFailureRecoveryForTests().ToString().Contains(TEXT("重试")) || HUD->GetAdvancedInteractionFailureRecoveryForTests().ToString().Contains(TEXT("重新")));

	TestTrue(TEXT("Pressing E on a retryable advanced failure should reopen the same task."), PlayerController->HandleInputKeyForTests(EKeys::E));
	TestTrue(TEXT("Retry should reactivate the signal tuning loop."), ObjectiveActor->IsAdvancedInteractionActive());
	TestTrue(TEXT("Retry should restore the advanced panel."), HUD->IsAdvancedInteractionPanelVisibleForTests());
	TestEqual(TEXT("Retry should restore full signal stability."), ObjectiveActor->BuildAdvancedInteractionHUDState().StabilityFraction, 1.0f);

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorPlayerControllerDay1HUDRouteKitDangerTest,
	"HorrorProject.Player.Controller.Day1HUDRouteKitDanger",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorPlayerControllerDay1HUDRouteKitDangerTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for RouteKit danger HUD coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	World->GetWorldSettings()->DefaultGameMode = AHorrorGameModeBase::StaticClass();
	TestTrue(TEXT("Transient world should create the Day1 RouteKit danger game mode."), World->SetGameMode(FURL()));
	AHorrorGameModeBase* GameMode = World->GetAuthGameMode<AHorrorGameModeBase>();
	TestNotNull(TEXT("RouteKit danger test should expose the game mode."), GameMode);
	if (!GameMode)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	AHorrorPlayerController* PlayerController = World->SpawnActor<AHorrorPlayerController>();
	ADay1SliceHUD* HUD = World->SpawnActor<ADay1SliceHUD>();
	AHorrorPlayerCharacter* PlayerCharacter = World->SpawnActor<AHorrorPlayerCharacter>(FVector(1000.0f, 300.0f, 80.0f), FRotator::ZeroRotator);
	TestNotNull(TEXT("RouteKit danger test should spawn a horror player controller."), PlayerController);
	TestNotNull(TEXT("RouteKit danger test should attach the native Day1 HUD."), HUD);
	TestNotNull(TEXT("RouteKit danger test should spawn a horror player character."), PlayerCharacter);
	if (!PlayerController || !HUD || !PlayerCharacter)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	PlayerController->PlayerState = World->SpawnActor<APlayerState>();
	World->AddController(PlayerController);
	PlayerController->MyHUD = HUD;
	PlayerController->Possess(PlayerCharacter);

	GameMode->DispatchBeginPlay();
	ADeepWaterStationRouteKit* RouteKit = GameMode->GetRuntimeRouteKit();
	TestNotNull(TEXT("RouteKit danger test should bootstrap the runtime route kit."), RouteKit);
	AHorrorEncounterDirector* EncounterDirector = RouteKit ? RouteKit->GetSpawnedEncounterDirector() : nullptr;
	TestNotNull(TEXT("RouteKit danger test should bootstrap the placed route encounter director."), EncounterDirector);
	if (!RouteKit || !EncounterDirector)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	TestFalse(TEXT("HUD danger should start hidden before the route encounter reveal."), HUD->IsDangerStatusVisibleForTests());
	TestTrue(TEXT("RouteKit should reveal its encounter when the player reaches the reveal point."), RouteKit->TriggerEncounterReveal(PlayerCharacter));
	TestTrue(TEXT("RouteKit should report the route gated while the encounter is revealed."), RouteKit->IsRouteGatedByEncounter());

	PlayerController->RefreshDay1HUDStateForTests();
	TestTrue(TEXT("HUD danger should reflect the RouteKit encounter gate, not only the GameMode fallback director."), HUD->IsDangerStatusVisibleForTests());

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

#endif
