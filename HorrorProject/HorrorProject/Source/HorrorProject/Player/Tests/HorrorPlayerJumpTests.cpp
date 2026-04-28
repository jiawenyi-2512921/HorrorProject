// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/HorrorPlayerCharacter.h"

#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR

#include "GameFramework/CharacterMovementComponent.h"
#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"

namespace
{
	constexpr float ExpectedMinimumJumpZVelocity = 520.0f;
	constexpr float ExpectedMinimumStepHeight = 55.0f;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorPlayerCharacterEnablesJumpingTest,
	"HorrorProject.Player.Character.EnablesJumping",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorPlayerCharacterEnablesJumpingTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for jump defaults coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	AHorrorPlayerCharacter* PlayerCharacter = World->SpawnActor<AHorrorPlayerCharacter>();
	TestNotNull(TEXT("Jump defaults test should spawn a horror player character."), PlayerCharacter);
	if (!PlayerCharacter)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	PlayerCharacter->DispatchBeginPlay();

	UCharacterMovementComponent* MovementComponent = PlayerCharacter->GetCharacterMovement();
	TestNotNull(TEXT("Jump defaults test should expose character movement."), MovementComponent);
	if (!MovementComponent)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	TestTrue(TEXT("Player nav agent should allow jumping."), MovementComponent->NavAgentProps.bCanJump);
	TestTrue(TEXT("Player jump velocity should clear DeepWaterStation traversal gaps."), MovementComponent->JumpZVelocity >= ExpectedMinimumJumpZVelocity);
	TestTrue(TEXT("Player step height should tolerate small imported-map lips."), MovementComponent->MaxStepHeight >= ExpectedMinimumStepHeight);

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

#endif
