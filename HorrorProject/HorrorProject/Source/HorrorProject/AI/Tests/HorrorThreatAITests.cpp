// Copyright Epic Games, Inc. All Rights Reserved.

#if WITH_DEV_AUTOMATION_TESTS

#include "AI/HorrorThreatAIController.h"
#include "AI/HorrorThreatCharacter.h"

#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorThreatAIStateTest,
	"HorrorProject.AI.Threat.StateAndDetection",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FHorrorThreatAIStateTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for threat AI coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AHorrorThreatCharacter* Threat = World->SpawnActor<AHorrorThreatCharacter>(FVector::ZeroVector, FRotator::ZeroRotator, SpawnParameters);
	AHorrorThreatCharacter* NearTarget = World->SpawnActor<AHorrorThreatCharacter>(FVector(100.0f, 0.0f, 0.0f), FRotator::ZeroRotator, SpawnParameters);
	AHorrorThreatCharacter* FarTarget = World->SpawnActor<AHorrorThreatCharacter>(FVector(1000.0f, 0.0f, 0.0f), FRotator::ZeroRotator, SpawnParameters);
	TestNotNull(TEXT("Threat character should spawn."), Threat);
	TestNotNull(TEXT("Near target should spawn."), NearTarget);
	TestNotNull(TEXT("Far target should spawn."), FarTarget);
	if (!Threat || !NearTarget || !FarTarget)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	Threat->DetectionRadius = 250.0f;
	UHorrorThreatDelegateProbe* DelegateProbe = NewObject<UHorrorThreatDelegateProbe>();
	Threat->OnThreatActiveChanged.AddDynamic(DelegateProbe, &UHorrorThreatDelegateProbe::HandleThreatActiveChanged);
	Threat->OnDetectedTargetChanged.AddDynamic(DelegateProbe, &UHorrorThreatDelegateProbe::HandleDetectedTargetChanged);
	TestEqual(TEXT("Threat should expose its default id."), Threat->ThreatId, FName(TEXT("Threat.Default")));
	TestFalse(TEXT("Threat should start inactive."), Threat->IsThreatActive());
	TestFalse(TEXT("Inactive threats should not detect nearby actors."), Threat->CanDetectActor(NearTarget));
	TestTrue(TEXT("Activating an inactive threat should succeed."), Threat->ActivateThreat());
	TestEqual(TEXT("Activation should broadcast once."), DelegateProbe->ActiveValues.Num(), 1);
	TestTrue(TEXT("Activation broadcast should carry active=true."), DelegateProbe->ActiveValues.Last());
	TestFalse(TEXT("Duplicate activation should be ignored."), Threat->ActivateThreat());
	TestEqual(TEXT("Duplicate activation should not rebroadcast."), DelegateProbe->ActiveValues.Num(), 1);
	TestTrue(TEXT("Active threats should detect actors inside radius."), Threat->CanDetectActor(NearTarget));
	TestFalse(TEXT("Active threats should not detect themselves."), Threat->CanDetectActor(Threat));
	TestFalse(TEXT("Self-target updates should be ignored."), Threat->UpdateDetectedTarget(Threat));
	TestEqual(TEXT("Rejected self-target should not broadcast target changes when target is already empty."), DelegateProbe->TargetValues.Num(), 0);
	TestTrue(TEXT("Detected targets inside radius should be stored."), Threat->UpdateDetectedTarget(NearTarget));
	TestEqual(TEXT("New detected target should broadcast once."), DelegateProbe->TargetValues.Num(), 1);
	TestTrue(TEXT("Detected target broadcast should carry the target actor."), DelegateProbe->TargetValues.Last().Get() == NearTarget);
	TestTrue(TEXT("Stored detected target should be queryable."), Threat->GetDetectedTarget() == NearTarget);
	TestFalse(TEXT("Duplicate detected target updates should be ignored."), Threat->UpdateDetectedTarget(NearTarget));
	TestEqual(TEXT("Duplicate detected target should not rebroadcast."), DelegateProbe->TargetValues.Num(), 1);
	TestFalse(TEXT("Active threats should reject actors outside radius."), Threat->CanDetectActor(FarTarget));
	TestTrue(TEXT("Undetectable targets should clear the stored target."), Threat->UpdateDetectedTarget(FarTarget));
	TestEqual(TEXT("Clearing detected target should broadcast null."), DelegateProbe->TargetValues.Num(), 2);
	TestNull(TEXT("Cleared target broadcast should carry null."), DelegateProbe->TargetValues.Last().Get());
	TestNull(TEXT("Rejected detection should clear the detected target."), Threat->GetDetectedTarget());
	TestTrue(TEXT("Detected targets can be stored again."), Threat->UpdateDetectedTarget(NearTarget));
	TestTrue(TEXT("Detected target actor should be destroyed for stale target coverage."), NearTarget->Destroy());
	TestNull(TEXT("Destroyed detected targets should not be queryable."), Threat->GetDetectedTarget());
	TestFalse(TEXT("Destroyed detected targets should be rejected by detection."), Threat->CanDetectActor(NearTarget));
	TestFalse(TEXT("Out-of-radius targets should remain rejected after stale target cleanup."), Threat->UpdateDetectedTarget(FarTarget));
	AHorrorThreatCharacter* ReplacementTarget = World->SpawnActor<AHorrorThreatCharacter>(FVector(100.0f, 50.0f, 0.0f), FRotator::ZeroRotator, SpawnParameters);
	TestNotNull(TEXT("Replacement target should spawn."), ReplacementTarget);
	TestTrue(TEXT("Detected targets can be stored after stale target cleanup."), Threat->UpdateDetectedTarget(ReplacementTarget));
	TestTrue(TEXT("Deactivating an active threat should succeed."), Threat->DeactivateThreat());
	TestEqual(TEXT("Deactivation should broadcast active=false."), DelegateProbe->ActiveValues, TArray<bool>({ true, false }));
	TestNull(TEXT("Deactivation should broadcast target clear."), DelegateProbe->TargetValues.Last().Get());
	TestNull(TEXT("Deactivation should clear the detected target."), Threat->GetDetectedTarget());
	TestFalse(TEXT("Inactive threats should stop detecting actors."), Threat->CanDetectActor(NearTarget));

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorThreatAIControllerPossessionTest,
	"HorrorProject.AI.Threat.ControllerPossession",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FHorrorThreatAIControllerPossessionTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for threat controller coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	AHorrorThreatCharacter* Threat = World->SpawnActor<AHorrorThreatCharacter>(FVector::ZeroVector, FRotator::ZeroRotator);
	AHorrorThreatAIController* Controller = World->SpawnActor<AHorrorThreatAIController>();
	TestNotNull(TEXT("Threat character should spawn for possession."), Threat);
	TestNotNull(TEXT("Threat controller should spawn."), Controller);
	if (!Threat || !Controller)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	Controller->Possess(Threat);
	TestEqual(TEXT("Threat controller should expose its possessed threat."), Controller->GetControlledThreat(), Threat);
	Controller->UnPossess();
	TestNull(TEXT("Threat controller should clear controlled threat after unpossess."), Controller->GetControlledThreat());

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

#endif
