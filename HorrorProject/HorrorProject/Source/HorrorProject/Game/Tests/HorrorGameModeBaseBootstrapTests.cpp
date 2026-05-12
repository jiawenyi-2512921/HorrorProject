// Copyright Epic Games, Inc. All Rights Reserved.

#include "Game/HorrorGameModeBase.h"

#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR

#include "EnhancedInputSubsystems.h"
#include "Engine/Engine.h"
#include "EngineUtils.h"
#include "Engine/GameInstance.h"
#include "Engine/LocalPlayer.h"
#include "Game/DeepWaterStationRouteKit.h"
#include "GameFramework/HUD.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerStart.h"
#include "InputMappingContext.h"
#include "Misc/AutomationTest.h"
#include "Player/HorrorPlayerCharacter.h"
#include "Player/HorrorPlayerController.h"
#include "Tests/AutomationCommon.h"
#include "UI/Day1SliceHUD.h"

namespace
{
	bool DestroyTestWorld(FTestWorldWrapper& TestWorld)
	{
		return TestWorld.DestroyTestWorld(false);
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorGameModeBaseForcesDay1NativeHUDTest,
	"HorrorProject.Game.GameModeBase.ForcesDay1NativeHUD",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorGameModeBaseForcesDay1NativeHUDTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for native Day1 HUD coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	AHorrorGameModeBase* GameMode = World->SpawnActor<AHorrorGameModeBase>();
	TestNotNull(TEXT("Native Day1 HUD test should spawn GameMode."), GameMode);
	if (!GameMode)
	{
		DestroyTestWorld(TestWorld);
		return false;
	}

	GameMode->HUDClass = AHUD::StaticClass();
	GameMode->PlayerControllerClass = APlayerController::StaticClass();
	FString ErrorMessage;
	GameMode->InitGame(TEXT("DemoMap_VerticalSlice_Day1"), TEXT(""), ErrorMessage);

	TestTrue(
		TEXT("GameMode should force the Day1 native Chinese HUD even when a Blueprint default overrides HUDClass."),
		GameMode->HUDClass.Get() == ADay1SliceHUD::StaticClass());
	TestTrue(
		TEXT("GameMode should force the native Day1 controller so the old English prototype widget cannot be spawned."),
		GameMode->PlayerControllerClass.Get() == AHorrorPlayerController::StaticClass());

	TestTrue(TEXT("Transient world should be destroyed cleanly."), DestroyTestWorld(TestWorld));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorGameModeBaseForcesNativePlayerPawnTest,
	"HorrorProject.Game.GameModeBase.ForcesNativePlayerPawn",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorGameModeBaseForcesNativePlayerPawnTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for native player pawn coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	AHorrorGameModeBase* GameMode = World->SpawnActor<AHorrorGameModeBase>();
	TestNotNull(TEXT("Native player pawn test should spawn GameMode."), GameMode);
	if (!GameMode)
	{
		DestroyTestWorld(TestWorld);
		return false;
	}

	GameMode->DefaultPawnClass = APawn::StaticClass();
	FString ErrorMessage;
	GameMode->InitGame(TEXT("Level_Scrapopolis_Demo"), TEXT(""), ErrorMessage);

	TestTrue(
		TEXT("GameMode should force the native horror player pawn even when an imported map or Blueprint default overrides DefaultPawnClass."),
		GameMode->DefaultPawnClass.Get() == AHorrorPlayerCharacter::StaticClass());

	TestTrue(TEXT("Transient world should be destroyed cleanly."), DestroyTestWorld(TestWorld));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorGameModeBaseChoosesPreferredPlayerStartTest,
	"HorrorProject.Game.GameModeBase.ChoosesPreferredPlayerStart",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorGameModeBaseChoosesPreferredPlayerStartTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for preferred PlayerStart coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	APlayerStart* UntaggedPlayerStart = World->SpawnActor<APlayerStart>(FVector::ZeroVector, FRotator::ZeroRotator);
	TestNotNull(TEXT("Preferred PlayerStart test should create an untagged start."), UntaggedPlayerStart);

	APlayerStart* TaggedPlayerStart = World->SpawnActor<APlayerStart>(FVector(500.0f, 0.0f, 0.0f), FRotator(0.0f, 90.0f, 0.0f));
	TestNotNull(TEXT("Preferred PlayerStart test should create a tagged start."), TaggedPlayerStart);
	if (!UntaggedPlayerStart || !TaggedPlayerStart)
	{
		DestroyTestWorld(TestWorld);
		return false;
	}

	TaggedPlayerStart->PlayerStartTag = TEXT("DeepWaterStationBase");

	AHorrorGameModeBase* GameMode = World->SpawnActor<AHorrorGameModeBase>();
	TestNotNull(TEXT("Preferred PlayerStart test should spawn GameMode."), GameMode);
	if (!GameMode)
	{
		DestroyTestWorld(TestWorld);
		return false;
	}

	TestTrue(
		TEXT("GameMode should choose the tagged DeepWaterStation PlayerStart when available."),
		GameMode->ChoosePlayerStart(nullptr) == TaggedPlayerStart);

	TestTrue(TEXT("Transient world should be destroyed cleanly."), DestroyTestWorld(TestWorld));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorGameModeBaseFallsBackToPlacedPlayerStartTest,
	"HorrorProject.Game.GameModeBase.FallsBackToPlacedPlayerStart",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorGameModeBaseFallsBackToPlacedPlayerStartTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for fallback PlayerStart coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	APlayerStart* PlayerStart = World->SpawnActor<APlayerStart>(FVector(250.0f, 0.0f, 0.0f), FRotator::ZeroRotator);
	TestNotNull(TEXT("Fallback PlayerStart test should create a placed start."), PlayerStart);
	if (!PlayerStart)
	{
		DestroyTestWorld(TestWorld);
		return false;
	}

	AHorrorGameModeBase* GameMode = World->SpawnActor<AHorrorGameModeBase>();
	TestNotNull(TEXT("Fallback PlayerStart test should spawn GameMode."), GameMode);
	if (!GameMode)
	{
		DestroyTestWorld(TestWorld);
		return false;
	}

	TestTrue(
		TEXT("GameMode should use a placed PlayerStart instead of falling back to WorldSettings."),
		GameMode->ChoosePlayerStart(nullptr) == PlayerStart);

	TestTrue(TEXT("Transient world should be destroyed cleanly."), DestroyTestWorld(TestWorld));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorGameModeBaseAnchorsRouteKitToPlayerStartTest,
	"HorrorProject.Game.GameModeBase.AnchorsRouteKitToPlayerStart",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorGameModeBaseAnchorsRouteKitToPlayerStartTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for route kit bootstrap coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	const FVector PlayerStartLocation(1000.0f, 2000.0f, 300.0f);
	const FRotator PlayerStartRotation(0.0f, -90.0f, 0.0f);
	APlayerStart* PlayerStart = World->SpawnActor<APlayerStart>(PlayerStartLocation, PlayerStartRotation);
	TestNotNull(TEXT("Route kit anchor test should create a PlayerStart."), PlayerStart);
	if (!PlayerStart)
	{
		DestroyTestWorld(TestWorld);
		return false;
	}

	AHorrorGameModeBase* GameMode = World->SpawnActor<AHorrorGameModeBase>();
	TestNotNull(TEXT("Route kit anchor test should spawn GameMode."), GameMode);
	if (!GameMode)
	{
		DestroyTestWorld(TestWorld);
		return false;
	}

	GameMode->DispatchBeginPlay();

	ADeepWaterStationRouteKit* RouteKit = GameMode->GetRuntimeRouteKit();
	TestNotNull(TEXT("GameMode should spawn a runtime route kit."), RouteKit);
	if (!RouteKit)
	{
		DestroyTestWorld(TestWorld);
		return false;
	}

	TestTrue(
		TEXT("Runtime route kit should spawn at the PlayerStart location by default."),
		RouteKit->GetActorLocation().Equals(PlayerStartLocation, KINDA_SMALL_NUMBER));
	TestTrue(
		TEXT("Runtime route kit should face the PlayerStart yaw by default."),
		FMath::IsNearlyEqual(FRotator::NormalizeAxis(RouteKit->GetActorRotation().Yaw), PlayerStartRotation.Yaw, 0.1f));

	TestTrue(TEXT("Transient world should be destroyed cleanly."), DestroyTestWorld(TestWorld));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorGameModeBaseUsesConfiguredRouteKitTransformWithoutPlayerStartTest,
	"HorrorProject.Game.GameModeBase.UsesConfiguredRouteKitTransformWithoutPlayerStart",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorGameModeBaseRestoresMouseLookMappingAfterImportedMapCleanupTest,
	"HorrorProject.Game.GameModeBase.RestoresMouseLookMappingAfterImportedMapCleanup",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorGameModeBaseUsesConfiguredRouteKitTransformWithoutPlayerStartTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for route kit fallback coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	AHorrorGameModeBase* GameMode = World->SpawnActor<AHorrorGameModeBase>();
	TestNotNull(TEXT("Route kit fallback test should spawn GameMode."), GameMode);
	if (!GameMode)
	{
		DestroyTestWorld(TestWorld);
		return false;
	}

	GameMode->DispatchBeginPlay();

	ADeepWaterStationRouteKit* RouteKit = GameMode->GetRuntimeRouteKit();
	TestNotNull(TEXT("GameMode should spawn a runtime route kit without a PlayerStart."), RouteKit);
	if (!RouteKit)
	{
		DestroyTestWorld(TestWorld);
		return false;
	}

	TestTrue(
		TEXT("Runtime route kit should keep the configured fallback transform when no PlayerStart exists."),
		RouteKit->GetActorLocation().Equals(FVector(0.0f, 0.0f, 80.0f), KINDA_SMALL_NUMBER));

	TestTrue(TEXT("Transient world should be destroyed cleanly."), DestroyTestWorld(TestWorld));
	return true;
}

bool FHorrorGameModeBaseRestoresMouseLookMappingAfterImportedMapCleanupTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for imported-map input restore coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	UGameInstance* GameInstance = World->GetGameInstance();
	TestNotNull(TEXT("Input restore test should expose a game instance."), GameInstance);
	if (!GameInstance)
	{
		DestroyTestWorld(TestWorld);
		return false;
	}

	ULocalPlayer* LocalPlayer = GEngine ? NewObject<ULocalPlayer>(GEngine, GEngine->LocalPlayerClass) : nullptr;
	TestNotNull(TEXT("Input restore test should create a local player."), LocalPlayer);
	if (!LocalPlayer)
	{
		DestroyTestWorld(TestWorld);
		return false;
	}
	const int32 LocalPlayerIndex = GameInstance->AddLocalPlayer(LocalPlayer, FPlatformUserId::CreateFromInternalId(0));
	TestTrue(
		TEXT("Input restore test should register the local player with the game instance."),
		LocalPlayerIndex != INDEX_NONE);

	AHorrorGameModeBase* GameMode = World->SpawnActor<AHorrorGameModeBase>();
	AHorrorPlayerController* PlayerController = World->SpawnActor<AHorrorPlayerController>();
	AHorrorPlayerCharacter* PlayerCharacter = World->SpawnActor<AHorrorPlayerCharacter>();
	TestNotNull(TEXT("Input restore test should spawn the horror game mode."), GameMode);
	TestNotNull(TEXT("Input restore test should spawn the horror player controller."), PlayerController);
	TestNotNull(TEXT("Input restore test should spawn the horror player character."), PlayerCharacter);
	if (!GameMode || !PlayerController || !PlayerCharacter)
	{
		DestroyTestWorld(TestWorld);
		return false;
	}

	World->AddController(PlayerController);
	PlayerController->SetPlayer(LocalPlayer);
	PlayerController->Possess(PlayerCharacter);

	UEnhancedInputLocalPlayerSubsystem* InputSubsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
	TestNotNull(TEXT("Local player should expose Enhanced Input subsystem."), InputSubsystem);
	UInputMappingContext* MouseLookContext = LoadObject<UInputMappingContext>(nullptr, TEXT("/Game/Input/IMC_MouseLook.IMC_MouseLook"));
	TestNotNull(TEXT("Mouse-look mapping context should load for runtime restore."), MouseLookContext);
	if (!InputSubsystem || !MouseLookContext)
	{
		DestroyTestWorld(TestWorld);
		return false;
	}

	InputSubsystem->ClearAllMappings();
	TestFalse(TEXT("Test setup should remove mouse-look mapping before restore."), InputSubsystem->HasMappingContext(MouseLookContext));
	PlayerController->SetIgnoreLookInput(true);
	PlayerController->bShowMouseCursor = true;
	PlayerController->bEnableClickEvents = true;
	PlayerController->bEnableMouseOverEvents = true;

	GameMode->RestoreLeadPlayerViewAndInputForTests();

	TestTrue(TEXT("Imported-map cleanup should reapply the mouse-look mapping so mouse deltas rotate the camera."), InputSubsystem->HasMappingContext(MouseLookContext));
	TestFalse(TEXT("Imported-map cleanup should restore look input."), PlayerController->IsLookInputIgnored());
	TestFalse(TEXT("Imported-map cleanup should hide the menu cursor."), PlayerController->bShowMouseCursor);
	TestFalse(TEXT("Imported-map cleanup should disable menu click handling."), PlayerController->bEnableClickEvents);
	TestFalse(TEXT("Imported-map cleanup should disable menu hover handling."), PlayerController->bEnableMouseOverEvents);

	TestTrue(TEXT("Transient world should be destroyed cleanly."), DestroyTestWorld(TestWorld));
	return true;
}

#endif
