// Copyright Epic Games, Inc. All Rights Reserved.

#include "Game/HorrorGameModeBase.h"

#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR

#include "EngineUtils.h"
#include "Game/DeepWaterStationRouteKit.h"
#include "GameFramework/HUD.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerStart.h"
#include "Misc/AutomationTest.h"
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

#endif
