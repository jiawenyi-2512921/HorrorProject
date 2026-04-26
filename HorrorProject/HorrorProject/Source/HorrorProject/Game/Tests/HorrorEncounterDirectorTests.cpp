#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR && HORRORPROJECT_ENABLE_LEGACY_AUTOMATION_TESTS

// Copyright Epic Games, Inc. All Rights Reserved.

#if WITH_DEV_AUTOMATION_TESTS

#include "Game/HorrorEncounterDirector.h"

#include "AI/HorrorThreatCharacter.h"
#include "Engine/World.h"
#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorEncounterDirectorLifecycleTest,
	"HorrorProject.Game.Encounter.Director.Lifecycle",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorEncounterDirectorLifecycleTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for encounter director coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	AHorrorEncounterDirector* Director = World->SpawnActor<AHorrorEncounterDirector>();
	TestNotNull(TEXT("Encounter director should spawn in a transient world."), Director);
	if (!Director)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	AHorrorThreatCharacter* Threat = World->SpawnActor<AHorrorThreatCharacter>(FVector(100.0f, 0.0f, 0.0f), FRotator::ZeroRotator);
	TestNotNull(TEXT("Encounter director test should spawn a threat actor."), Threat);
	if (!Threat)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	AActor* PlayerActor = World->SpawnActor<AActor>(FVector(200.0f, 0.0f, 0.0f), FRotator::ZeroRotator);
	TestNotNull(TEXT("Encounter director test should spawn a player stand-in."), PlayerActor);
	if (!PlayerActor)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	Director->ThreatActor = Threat;
	Director->RevealRadius = 500.0f;
	UHorrorEncounterPhaseDelegateProbe* DelegateProbe = NewObject<UHorrorEncounterPhaseDelegateProbe>();
	Director->OnEncounterPhaseChanged.AddDynamic(DelegateProbe, &UHorrorEncounterPhaseDelegateProbe::HandleEncounterPhaseChanged);
	TestEqual(TEXT("Manual threat actor should be reused by spawn bridge."), Director->SpawnThreatActor(), Threat);
	TestEqual(TEXT("Manual threat actor should be queryable from the director."), Director->GetThreatActor(), Threat);

	TestEqual(TEXT("Encounter director should start dormant."), Director->GetEncounterPhase(), EHorrorEncounterPhase::Dormant);
	TestFalse(TEXT("Dormant encounter should not gate the route."), Director->IsRouteGated());
	TestFalse(TEXT("Dormant encounter should not reveal."), Director->TriggerReveal(PlayerActor));
	TestEqual(TEXT("Failed dormant reveal should not broadcast phase changes."), DelegateProbe->PhaseValues.Num(), 0);

	TestTrue(TEXT("Encounter director should prime with a default id."), Director->PrimeEncounter(NAME_None));
	TestEqual(TEXT("Prime should broadcast one phase change."), DelegateProbe->PhaseValues.Num(), 1);
	TestEqual(TEXT("Prime broadcast should carry primed phase."), DelegateProbe->PhaseValues.Last(), EHorrorEncounterPhase::Primed);
	TestEqual(TEXT("Prime broadcast should carry default encounter id."), DelegateProbe->EncounterIds.Last(), Director->DefaultEncounterId);
	TestEqual(TEXT("Encounter director should use its default id when primed with none."), Director->GetEncounterId(), Director->DefaultEncounterId);
	TestEqual(TEXT("Primed encounter should expose the primed phase."), Director->GetEncounterPhase(), EHorrorEncounterPhase::Primed);
	TestFalse(TEXT("Primed encounter should not gate the route before reveal."), Director->IsRouteGated());
	TestFalse(TEXT("Already-primed encounter should reject duplicate priming."), Director->PrimeEncounter(TEXT("Encounter.Other")));
	TestEqual(TEXT("Duplicate priming should not broadcast phase changes."), DelegateProbe->PhaseValues.Num(), 1);
	TestTrue(TEXT("Player inside reveal radius should be eligible for reveal."), Director->CanTriggerReveal(PlayerActor));

	TestTrue(TEXT("Primed encounter should reveal for an eligible player."), Director->TriggerReveal(PlayerActor));
	TestEqual(TEXT("Reveal should broadcast a second phase change."), DelegateProbe->PhaseValues.Num(), 2);
	TestEqual(TEXT("Reveal broadcast should carry revealed phase."), DelegateProbe->PhaseValues.Last(), EHorrorEncounterPhase::Revealed);
	TestEqual(TEXT("Revealed encounter should expose the revealed phase."), Director->GetEncounterPhase(), EHorrorEncounterPhase::Revealed);
	TestTrue(TEXT("Revealed encounter should gate the route."), Director->IsRouteGated());
	TestEqual(TEXT("Revealed encounter should remember the reveal target."), Director->GetLastRevealTarget(), PlayerActor);
	TestTrue(TEXT("Revealed encounter should activate the threat."), Threat->IsThreatActive());
	TestEqual(TEXT("Revealed encounter should point the threat at the player."), Threat->GetDetectedTarget(), PlayerActor);
	TestFalse(TEXT("Revealed encounter should reject duplicate reveals."), Director->TriggerReveal(PlayerActor));

	TestTrue(TEXT("Revealed encounter should resolve."), Director->ResolveEncounter());
	TestEqual(TEXT("Resolve should broadcast a third phase change."), DelegateProbe->PhaseValues.Num(), 3);
	TestEqual(TEXT("Resolve broadcast should carry resolved phase."), DelegateProbe->PhaseValues.Last(), EHorrorEncounterPhase::Resolved);
	TestEqual(TEXT("Resolved encounter should expose the resolved phase."), Director->GetEncounterPhase(), EHorrorEncounterPhase::Resolved);
	TestFalse(TEXT("Resolved encounter should stop gating the route."), Director->IsRouteGated());
	TestFalse(TEXT("Resolved encounter should deactivate the threat."), Threat->IsThreatActive());
	TestFalse(TEXT("Resolved encounter should reject duplicate resolve."), Director->ResolveEncounter());

	TestTrue(TEXT("Resolved encounter should reset to dormant."), Director->ResetEncounter());
	TestEqual(TEXT("Reset should broadcast a fourth phase change."), DelegateProbe->PhaseValues.Num(), 4);
	TestEqual(TEXT("Reset broadcast should carry dormant phase."), DelegateProbe->PhaseValues.Last(), EHorrorEncounterPhase::Dormant);
	TestEqual(TEXT("Reset broadcast should carry the reset encounter id."), DelegateProbe->EncounterIds.Last(), Director->DefaultEncounterId);
	TestEqual(TEXT("Lifecycle broadcasts should preserve phase order."), DelegateProbe->PhaseValues, TArray<EHorrorEncounterPhase>({ EHorrorEncounterPhase::Primed, EHorrorEncounterPhase::Revealed, EHorrorEncounterPhase::Resolved, EHorrorEncounterPhase::Dormant }));
	TestEqual(TEXT("Reset encounter should be dormant."), Director->GetEncounterPhase(), EHorrorEncounterPhase::Dormant);
	TestEqual(TEXT("Reset encounter should clear the active id."), Director->GetEncounterId(), NAME_None);
	TestNull(TEXT("Reset encounter should clear the reveal target."), Director->GetLastRevealTarget());
	TestFalse(TEXT("Fully reset encounter should reject duplicate reset."), Director->ResetEncounter());
	TestEqual(TEXT("Duplicate reset should not broadcast phase changes."), DelegateProbe->PhaseValues.Num(), 4);

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorEncounterDirectorSpawnsAuthoredThreatTest,
	"HorrorProject.Game.Encounter.Director.SpawnsAuthoredThreat",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorEncounterDirectorSpawnsAuthoredThreatTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for authored threat coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	AHorrorEncounterDirector* Director = World->SpawnActor<AHorrorEncounterDirector>();
	AHorrorThreatCharacter* PlayerActor = World->SpawnActor<AHorrorThreatCharacter>(FVector(1075.0f, 0.0f, 0.0f), FRotator::ZeroRotator);
	TestNotNull(TEXT("Encounter director should spawn for authored threat coverage."), Director);
	TestNotNull(TEXT("Player stand-in should spawn for authored threat coverage."), PlayerActor);
	if (!Director || !PlayerActor)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}


	Director->SetActorLocation(FVector(1000.0f, 0.0f, 0.0f));
	Director->ThreatActor = nullptr;
	Director->ThreatClass = AHorrorThreatCharacter::StaticClass();
	Director->ThreatRelativeTransform = FTransform(FRotator::ZeroRotator, FVector(50.0f, 25.0f, 0.0f));
	Director->RevealRadius = 500.0f;

	TestTrue(TEXT("Encounter director should prime before spawning an authored reveal threat."), Director->PrimeEncounter(TEXT("Encounter.AuthoredThreat")));
	TestNull(TEXT("Authored threat should not be preassigned before reveal."), Director->GetThreatActor());
	TestTrue(TEXT("Primed encounter should reveal and spawn the authored threat."), Director->TriggerReveal(PlayerActor));

	AHorrorThreatCharacter* SpawnedThreat = Director->GetThreatActor();
	TestNotNull(TEXT("Reveal should create an authored threat actor."), SpawnedThreat);
	if (!SpawnedThreat)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	TestEqual(TEXT("Authored threat should spawn relative to the encounter director."), SpawnedThreat->GetActorLocation(), FVector(1050.0f, 25.0f, 0.0f));
	TestTrue(TEXT("Authored threat should be owned by the encounter director."), SpawnedThreat->GetOwner() == Director);
	TestTrue(TEXT("Authored threat should activate on reveal."), SpawnedThreat->IsThreatActive());
	TestTrue(TEXT("Authored threat should detect the reveal target."), SpawnedThreat->GetDetectedTarget() == PlayerActor);
	TestEqual(TEXT("Repeated spawn calls should return the authored threat."), Director->SpawnThreatActor(), SpawnedThreat);
	TestTrue(TEXT("Resolved authored encounter should deactivate the spawned threat."), Director->ResolveEncounter());
	TestFalse(TEXT("Resolved authored threat should no longer be active."), SpawnedThreat->IsThreatActive());

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorEncounterDirectorSpawnsAuthoredThreatWithRotatedDirectorTest,
	"HorrorProject.Game.Encounter.Director.SpawnsAuthoredThreatWithRotatedDirector",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorEncounterDirectorSpawnsAuthoredThreatWithRotatedDirectorTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for rotated authored threat coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	AHorrorEncounterDirector* Director = World->SpawnActor<AHorrorEncounterDirector>();
	AActor* PlayerActor = World->SpawnActor<AActor>(FVector(1000.0f, 100.0f, 0.0f), FRotator::ZeroRotator);
	TestNotNull(TEXT("Rotated encounter director should spawn."), Director);
	TestNotNull(TEXT("Rotated encounter player stand-in should spawn."), PlayerActor);
	if (!Director || !PlayerActor)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	Director->SetActorTransform(FTransform(FRotator(0.0f, 90.0f, 0.0f), FVector(1000.0f, 0.0f, 0.0f)));
	Director->ThreatActor = nullptr;
	Director->ThreatClass = AHorrorThreatCharacter::StaticClass();
	Director->ThreatRelativeTransform = FTransform(FRotator::ZeroRotator, FVector(100.0f, 0.0f, 0.0f));
	Director->RevealRadius = 0.0f;

	TestTrue(TEXT("Rotated encounter should prime before reveal."), Director->PrimeEncounter(TEXT("Encounter.RotatedThreat")));
	TestTrue(TEXT("Rotated encounter should reveal and spawn the authored threat."), Director->TriggerReveal(PlayerActor));

	AHorrorThreatCharacter* SpawnedThreat = Director->GetThreatActor();
	TestNotNull(TEXT("Rotated reveal should create an authored threat actor."), SpawnedThreat);
	if (!SpawnedThreat)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	TestEqual(TEXT("Authored threat should respect the director rotation when applying the relative offset."), SpawnedThreat->GetActorLocation(), FVector(1000.0f, 100.0f, 0.0f));

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorEncounterDirectorThreatlessRevealTest,
	"HorrorProject.Game.Encounter.Director.ThreatlessReveal",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorEncounterDirectorThreatlessRevealTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for threatless reveal coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	AHorrorEncounterDirector* Director = World->SpawnActor<AHorrorEncounterDirector>(FVector::ZeroVector, FRotator::ZeroRotator);
	AActor* PlayerActor = World->SpawnActor<AActor>(FVector(200.0f, 0.0f, 0.0f), FRotator::ZeroRotator);
	TestNotNull(TEXT("Threatless reveal director should spawn."), Director);
	TestNotNull(TEXT("Threatless reveal player stand-in should spawn."), PlayerActor);
	if (!Director || !PlayerActor)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	Director->ThreatActor = nullptr;
	Director->ThreatClass = nullptr;
	Director->RevealRadius = 500.0f;

	TestTrue(TEXT("Threatless encounter should prime."), Director->PrimeEncounter(TEXT("Encounter.Threatless")));
	TestTrue(TEXT("Threatless encounter should still reveal when the player is eligible."), Director->TriggerReveal(PlayerActor));
	TestEqual(TEXT("Threatless reveal should advance to revealed phase."), Director->GetEncounterPhase(), EHorrorEncounterPhase::Revealed);
	TestTrue(TEXT("Threatless revealed encounter should still gate the route."), Director->IsRouteGated());
	TestNull(TEXT("Threatless reveal should not create a threat actor."), Director->GetThreatActor());
	TestEqual(TEXT("Threatless reveal should still remember the reveal target."), Director->GetLastRevealTarget(), PlayerActor);
	TestTrue(TEXT("Threatless revealed encounter should resolve."), Director->ResolveEncounter());
	TestFalse(TEXT("Resolved threatless encounter should stop gating the route."), Director->IsRouteGated());

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorEncounterDirectorRadiusSemanticsTest,
	"HorrorProject.Game.Encounter.Director.RadiusSemantics",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorEncounterDirectorRadiusSemanticsTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for encounter radius coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	AHorrorEncounterDirector* Director = World->SpawnActor<AHorrorEncounterDirector>(FVector(1000.0f, 0.0f, 0.0f), FRotator::ZeroRotator);
	AHorrorThreatCharacter* Threat = World->SpawnActor<AHorrorThreatCharacter>(FVector(1000.0f, 0.0f, 0.0f), FRotator::ZeroRotator);
	AActor* FarPlayerActor = World->SpawnActor<AActor>(FVector(2000.0f, 0.0f, 0.0f), FRotator::ZeroRotator);
	TestNotNull(TEXT("Radius semantics director should spawn."), Director);
	TestNotNull(TEXT("Radius semantics threat should spawn."), Threat);
	TestNotNull(TEXT("Radius semantics player stand-in should spawn."), FarPlayerActor);
	if (!Director || !Threat || !FarPlayerActor)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	Director->ThreatActor = Threat;
	Director->RevealRadius = 500.0f;
	TestTrue(TEXT("Radius-limited encounter should prime."), Director->PrimeEncounter(TEXT("Encounter.RadiusLimited")));
	TestFalse(TEXT("Positive reveal radius should reject an out-of-range target."), Director->CanTriggerReveal(FarPlayerActor));
	TestFalse(TEXT("Out-of-range reveal should fail."), Director->TriggerReveal(FarPlayerActor));
	TestEqual(TEXT("Rejected out-of-range reveal should remain primed."), Director->GetEncounterPhase(), EHorrorEncounterPhase::Primed);

	Director->ResetEncounter();
	Director->RevealRadius = 0.0f;
	TestTrue(TEXT("Zero-radius encounter should prime."), Director->PrimeEncounter(TEXT("Encounter.UnlimitedRadius")));
	TestTrue(TEXT("Zero reveal radius should allow an otherwise distant target."), Director->CanTriggerReveal(FarPlayerActor));
	TestTrue(TEXT("Zero reveal radius should reveal an otherwise distant target."), Director->TriggerReveal(FarPlayerActor));
	TestEqual(TEXT("Zero-radius reveal should advance to revealed phase."), Director->GetEncounterPhase(), EHorrorEncounterPhase::Revealed);

	Director->ResetEncounter();
	Director->RevealRadius = -1.0f;
	TestTrue(TEXT("Negative-radius encounter should prime."), Director->PrimeEncounter(TEXT("Encounter.NegativeRadius")));
	TestTrue(TEXT("Negative reveal radius should allow an otherwise distant target."), Director->CanTriggerReveal(FarPlayerActor));

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorEncounterDirectorThreatDetectionDivergenceTest,
	"HorrorProject.Game.Encounter.Director.ThreatDetectionDivergence",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorEncounterDirectorThreatDetectionDivergenceTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for threat detection divergence coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	AHorrorEncounterDirector* Director = World->SpawnActor<AHorrorEncounterDirector>(FVector::ZeroVector, FRotator::ZeroRotator);
	AHorrorThreatCharacter* Threat = World->SpawnActor<AHorrorThreatCharacter>(FVector::ZeroVector, FRotator::ZeroRotator);
	AActor* PlayerActor = World->SpawnActor<AActor>(FVector(100.0f, 0.0f, 0.0f), FRotator::ZeroRotator);
	TestNotNull(TEXT("Detection divergence director should spawn."), Director);
	TestNotNull(TEXT("Detection divergence threat should spawn."), Threat);
	TestNotNull(TEXT("Detection divergence player stand-in should spawn."), PlayerActor);
	if (!Director || !Threat || !PlayerActor)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	Director->ThreatActor = Threat;
	Director->RevealRadius = 500.0f;
	Threat->DetectionRadius = 0.0f;

	TestTrue(TEXT("Detection divergence encounter should prime."), Director->PrimeEncounter(TEXT("Encounter.DetectionDivergence")));
	TestTrue(TEXT("Encounter reveal radius should allow the nearby player."), Director->TriggerReveal(PlayerActor));
	TestEqual(TEXT("Reveal should still advance when threat detection rejects the target."), Director->GetEncounterPhase(), EHorrorEncounterPhase::Revealed);
	TestTrue(TEXT("Threat should still activate during reveal."), Threat->IsThreatActive());
	TestNull(TEXT("Zero threat detection radius should reject and clear the reveal target."), Threat->GetDetectedTarget());
	TestTrue(TEXT("Resolving divergence encounter should deactivate threat."), Director->ResolveEncounter());
	TestFalse(TEXT("Resolved divergence threat should be inactive."), Threat->IsThreatActive());

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorEncounterDirectorResetReusesThreatTest,
	"HorrorProject.Game.Encounter.Director.ResetReusesThreat",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorEncounterDirectorResetReusesThreatTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for reset threat reuse coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	AHorrorEncounterDirector* Director = World->SpawnActor<AHorrorEncounterDirector>(FVector::ZeroVector, FRotator::ZeroRotator);
	AHorrorThreatCharacter* FirstPlayerActor = World->SpawnActor<AHorrorThreatCharacter>(FVector(100.0f, 0.0f, 0.0f), FRotator::ZeroRotator);
	AHorrorThreatCharacter* SecondPlayerActor = World->SpawnActor<AHorrorThreatCharacter>(FVector(150.0f, 0.0f, 0.0f), FRotator::ZeroRotator);
	TestNotNull(TEXT("Reset reuse director should spawn."), Director);
	TestNotNull(TEXT("Reset reuse first player stand-in should spawn."), FirstPlayerActor);
	TestNotNull(TEXT("Reset reuse second player stand-in should spawn."), SecondPlayerActor);
	if (!Director || !FirstPlayerActor || !SecondPlayerActor)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	Director->ThreatActor = nullptr;
	Director->ThreatClass = AHorrorThreatCharacter::StaticClass();
	Director->RevealRadius = 500.0f;

	TestTrue(TEXT("First reusable encounter should prime."), Director->PrimeEncounter(TEXT("Encounter.ReuseA")));
	TestTrue(TEXT("First reusable encounter should reveal."), Director->TriggerReveal(FirstPlayerActor));
	AHorrorThreatCharacter* SpawnedThreat = Director->GetThreatActor();
	TestNotNull(TEXT("First reveal should spawn a reusable threat."), SpawnedThreat);
	if (!SpawnedThreat)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}
	TestTrue(TEXT("First reveal should activate the reusable threat."), SpawnedThreat->IsThreatActive());
	TestTrue(TEXT("First reveal should assign the first target."), SpawnedThreat->GetDetectedTarget() == FirstPlayerActor);

	TestTrue(TEXT("Reset should deactivate the reusable threat."), Director->ResetEncounter());
	TestEqual(TEXT("Reset should keep the spawned threat assigned for reuse."), Director->GetThreatActor(), SpawnedThreat);
	TestFalse(TEXT("Reset reusable threat should be inactive."), SpawnedThreat->IsThreatActive());
	TestNull(TEXT("Reset reusable threat should clear its detected target."), SpawnedThreat->GetDetectedTarget());

	TestTrue(TEXT("Second reusable encounter should prime."), Director->PrimeEncounter(TEXT("Encounter.ReuseB")));
	TestTrue(TEXT("Second reusable encounter should reveal."), Director->TriggerReveal(SecondPlayerActor));
	TestEqual(TEXT("Second reveal should reuse the same threat actor."), Director->GetThreatActor(), SpawnedThreat);
	TestTrue(TEXT("Second reveal should reactivate the reusable threat."), SpawnedThreat->IsThreatActive());
	TestTrue(TEXT("Second reveal should assign the second target."), SpawnedThreat->GetDetectedTarget() == SecondPlayerActor);

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorEncounterDirectorRevealCanAvoidRouteGateTest,
	"HorrorProject.Game.Encounter.Director.RevealCanAvoidRouteGate",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorEncounterDirectorRevealCanAvoidRouteGateTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for non-gating reveal coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	AHorrorEncounterDirector* Director = World->SpawnActor<AHorrorEncounterDirector>(FVector::ZeroVector, FRotator::ZeroRotator);
	AHorrorThreatCharacter* Threat = World->SpawnActor<AHorrorThreatCharacter>(FVector::ZeroVector, FRotator::ZeroRotator);
	AActor* PlayerActor = World->SpawnActor<AActor>(FVector(100.0f, 0.0f, 0.0f), FRotator::ZeroRotator);
	TestNotNull(TEXT("Non-gating reveal director should spawn."), Director);
	TestNotNull(TEXT("Non-gating reveal threat should spawn."), Threat);
	TestNotNull(TEXT("Non-gating reveal player stand-in should spawn."), PlayerActor);
	if (!Director || !Threat || !PlayerActor)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	Director->ThreatActor = Threat;
	Director->RevealRadius = 500.0f;
	Director->bGateRouteDuringReveal = false;

	TestTrue(TEXT("Non-gating encounter should prime."), Director->PrimeEncounter(TEXT("Encounter.NonGatingReveal")));
	TestTrue(TEXT("Non-gating encounter should reveal."), Director->TriggerReveal(PlayerActor));
	TestEqual(TEXT("Non-gating reveal should still advance to revealed phase."), Director->GetEncounterPhase(), EHorrorEncounterPhase::Revealed);
	TestFalse(TEXT("Non-gating reveal should not gate the route."), Director->IsRouteGated());
	TestTrue(TEXT("Non-gating reveal should still activate the threat."), Threat->IsThreatActive());
	TestTrue(TEXT("Non-gating revealed encounter should still resolve."), Director->ResolveEncounter());
	TestFalse(TEXT("Resolved non-gating threat should be inactive."), Threat->IsThreatActive());

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorEncounterDirectorResetFromPrimedClearsStateTest,
	"HorrorProject.Game.Encounter.Director.ResetFromPrimedClearsState",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorEncounterDirectorResetFromPrimedClearsStateTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for primed reset coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	AHorrorEncounterDirector* Director = World->SpawnActor<AHorrorEncounterDirector>();
	TestNotNull(TEXT("Primed reset director should spawn."), Director);
	if (!Director)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	TestTrue(TEXT("Encounter should prime before primed reset."), Director->PrimeEncounter(TEXT("Encounter.PrimedReset")));
	TestTrue(TEXT("Primed encounter should reset without reveal."), Director->ResetEncounter());
	TestEqual(TEXT("Primed reset should return to dormant."), Director->GetEncounterPhase(), EHorrorEncounterPhase::Dormant);
	TestEqual(TEXT("Primed reset should clear active id."), Director->GetEncounterId(), NAME_None);
	TestNull(TEXT("Primed reset should leave reveal target clear."), Director->GetLastRevealTarget());
	TestFalse(TEXT("Primed reset should leave route ungated."), Director->IsRouteGated());
	TestTrue(TEXT("Primed reset should allow the encounter to be primed again."), Director->PrimeEncounter(TEXT("Encounter.AfterPrimedReset")));

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorEncounterDirectorResetFromRevealedClearsGateAndThreatTest,
	"HorrorProject.Game.Encounter.Director.ResetFromRevealedClearsGateAndThreat",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorEncounterDirectorResetFromRevealedClearsGateAndThreatTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for revealed reset coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	AHorrorEncounterDirector* Director = World->SpawnActor<AHorrorEncounterDirector>(FVector::ZeroVector, FRotator::ZeroRotator);
	AHorrorThreatCharacter* Threat = World->SpawnActor<AHorrorThreatCharacter>(FVector::ZeroVector, FRotator::ZeroRotator);
	AActor* PlayerActor = World->SpawnActor<AActor>(FVector(100.0f, 0.0f, 0.0f), FRotator::ZeroRotator);
	TestNotNull(TEXT("Revealed reset director should spawn."), Director);
	TestNotNull(TEXT("Revealed reset threat should spawn."), Threat);
	TestNotNull(TEXT("Revealed reset player stand-in should spawn."), PlayerActor);
	if (!Director || !Threat || !PlayerActor)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	Director->ThreatActor = Threat;
	Director->RevealRadius = 500.0f;
	TestTrue(TEXT("Encounter should prime before revealed reset."), Director->PrimeEncounter(TEXT("Encounter.RevealedReset")));
	TestTrue(TEXT("Encounter should reveal before revealed reset."), Director->TriggerReveal(PlayerActor));
	TestTrue(TEXT("Revealed encounter should gate before reset."), Director->IsRouteGated());
	TestTrue(TEXT("Revealed threat should be active before reset."), Threat->IsThreatActive());

	TestTrue(TEXT("Revealed encounter should reset."), Director->ResetEncounter());
	TestEqual(TEXT("Revealed reset should return to dormant."), Director->GetEncounterPhase(), EHorrorEncounterPhase::Dormant);
	TestFalse(TEXT("Revealed reset should ungate the route."), Director->IsRouteGated());
	TestNull(TEXT("Revealed reset should clear reveal target."), Director->GetLastRevealTarget());
	TestEqual(TEXT("Revealed reset should clear encounter id."), Director->GetEncounterId(), NAME_None);
	TestEqual(TEXT("Revealed reset should preserve assigned threat actor."), Director->GetThreatActor(), Threat);
	TestFalse(TEXT("Revealed reset should deactivate threat."), Threat->IsThreatActive());
	TestNull(TEXT("Revealed reset should clear threat detected target."), Threat->GetDetectedTarget());

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorEncounterDirectorResolveFailurePreservesStateTest,
	"HorrorProject.Game.Encounter.Director.ResolveFailurePreservesState",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorEncounterDirectorResolveFailurePreservesStateTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for resolve failure coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	AHorrorEncounterDirector* Director = World->SpawnActor<AHorrorEncounterDirector>();
	AHorrorThreatCharacter* Threat = World->SpawnActor<AHorrorThreatCharacter>();
	TestNotNull(TEXT("Resolve failure director should spawn."), Director);
	TestNotNull(TEXT("Resolve failure threat should spawn."), Threat);
	if (!Director || !Threat)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	Director->ThreatActor = Threat;
	TestFalse(TEXT("Dormant resolve should fail."), Director->ResolveEncounter());
	TestEqual(TEXT("Dormant failed resolve should preserve dormant phase."), Director->GetEncounterPhase(), EHorrorEncounterPhase::Dormant);
	TestEqual(TEXT("Dormant failed resolve should preserve empty id."), Director->GetEncounterId(), NAME_None);

	TestTrue(TEXT("Encounter should prime before primed resolve failure."), Director->PrimeEncounter(TEXT("Encounter.ResolveFailure")));
	TestTrue(TEXT("Manual threat activation should succeed before failed resolve."), Threat->ActivateThreat());
	TestFalse(TEXT("Primed resolve should fail."), Director->ResolveEncounter());
	TestEqual(TEXT("Primed failed resolve should preserve primed phase."), Director->GetEncounterPhase(), EHorrorEncounterPhase::Primed);
	TestEqual(TEXT("Primed failed resolve should preserve active id."), Director->GetEncounterId(), FName(TEXT("Encounter.ResolveFailure")));
	TestFalse(TEXT("Primed failed resolve should not gate route."), Director->IsRouteGated());
	TestTrue(TEXT("Primed failed resolve should not deactivate threat unexpectedly."), Threat->IsThreatActive());

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorEncounterDirectorFailedRevealDoesNotMutateStateTest,
	"HorrorProject.Game.Encounter.Director.FailedRevealDoesNotMutateState",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorEncounterDirectorFailedRevealDoesNotMutateStateTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for failed reveal coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	AHorrorEncounterDirector* Director = World->SpawnActor<AHorrorEncounterDirector>(FVector::ZeroVector, FRotator::ZeroRotator);
	AHorrorThreatCharacter* FarPlayerActor = World->SpawnActor<AHorrorThreatCharacter>(FVector(10000.0f, 0.0f, 0.0f), FRotator::ZeroRotator);
	TestNotNull(TEXT("Failed reveal director should spawn."), Director);
	TestNotNull(TEXT("Failed reveal far player stand-in should spawn."), FarPlayerActor);
	if (!Director || !FarPlayerActor)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	Director->ThreatActor = nullptr;
	Director->ThreatClass = AHorrorThreatCharacter::StaticClass();
	Director->RevealRadius = 100.0f;
	TestTrue(TEXT("Encounter should prime before failed reveal."), Director->PrimeEncounter(TEXT("Encounter.FailedReveal")));
	TestFalse(TEXT("Null reveal should fail."), Director->TriggerReveal(nullptr));
	TestFalse(TEXT("Out-of-range player should not be eligible for reveal."), Director->CanTriggerReveal(FarPlayerActor));
	TestFalse(TEXT("Out-of-range reveal should fail."), Director->TriggerReveal(FarPlayerActor));
	TestEqual(TEXT("Failed reveals should preserve primed phase."), Director->GetEncounterPhase(), EHorrorEncounterPhase::Primed);
	TestEqual(TEXT("Failed reveals should preserve encounter id."), Director->GetEncounterId(), FName(TEXT("Encounter.FailedReveal")));
	TestNull(TEXT("Failed reveals should not assign reveal target."), Director->GetLastRevealTarget());
	TestNull(TEXT("Failed reveals should not spawn a threat."), Director->GetThreatActor());
	TestFalse(TEXT("Failed reveals should not gate route."), Director->IsRouteGated());

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

#endif

#endif // WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR
