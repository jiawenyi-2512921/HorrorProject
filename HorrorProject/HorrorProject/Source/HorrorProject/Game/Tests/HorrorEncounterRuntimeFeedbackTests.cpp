// Copyright Epic Games, Inc. All Rights Reserved.

#include "Game/HorrorEncounterDirector.h"

#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR

#include "AI/HorrorGolemBehaviorComponent.h"
#include "AI/HorrorThreatCharacter.h"
#include "Audio/HorrorAudioSubsystem.h"
#include "Game/HorrorEventBusSubsystem.h"
#include "GameplayTagContainer.h"
#include "Misc/AutomationTest.h"
#include "Player/HorrorPlayerCharacter.h"
#include "Player/Components/FearComponent.h"
#include "Player/Components/NoiseGeneratorComponent.h"
#include "Tests/AutomationCommon.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorEncounterRevealAppliesRuntimeFeedbackTest,
	"HorrorProject.Game.Encounter.RuntimeFeedback.RevealAppliesPressure",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorEncounterRevealAppliesRuntimeFeedbackTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for encounter feedback coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	AHorrorEncounterDirector* Director = World->SpawnActor<AHorrorEncounterDirector>(FVector::ZeroVector, FRotator::ZeroRotator);
	AHorrorPlayerCharacter* PlayerCharacter = World->SpawnActor<AHorrorPlayerCharacter>(FVector(500.0f, 0.0f, 0.0f), FRotator::ZeroRotator);
	TestNotNull(TEXT("Encounter feedback test should spawn a director."), Director);
	TestNotNull(TEXT("Encounter feedback test should spawn a player character."), PlayerCharacter);
	if (!Director || !PlayerCharacter)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	PlayerCharacter->DispatchBeginPlay();
	Director->ThreatActor = nullptr;
	Director->ThreatClass = AHorrorThreatCharacter::StaticClass();
	Director->RevealRadius = 0.0f;

	UFearComponent* Fear = PlayerCharacter->GetFearComponent();
	UNoiseGeneratorComponent* NoiseGenerator = PlayerCharacter->GetNoiseGeneratorComponent();
	TestNotNull(TEXT("Player should expose a fear component."), Fear);
	TestNotNull(TEXT("Player should expose a noise generator component."), NoiseGenerator);
	if (!Fear || !NoiseGenerator)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	TestTrue(TEXT("Encounter should prime before runtime reveal feedback."), Director->PrimeEncounter(TEXT("Encounter.GolemReveal01")));
	TestTrue(TEXT("Encounter reveal should succeed for the player."), Director->TriggerReveal(PlayerCharacter));
	TestEqual(TEXT("Encounter should advance to revealed."), Director->GetEncounterPhase(), EHorrorEncounterPhase::Revealed);
	TestTrue(TEXT("Reveal should add player fear pressure."), Fear->GetFearValue() > 0.0f);
	TestTrue(TEXT("Reveal should emit a player noise event for AI/audio hooks."), NoiseGenerator->GetCurrentNoiseLevel() > 0.0f);

	AHorrorThreatCharacter* Threat = Director->GetThreatActor();
	TestNotNull(TEXT("Reveal should spawn the default threat."), Threat);
	if (!Threat)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	UHorrorGolemBehaviorComponent* GolemBehavior = Threat->GetGolemBehavior();
	TestNotNull(TEXT("Default threat should carry a native golem behavior component."), GolemBehavior);
	if (!GolemBehavior)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	TestTrue(TEXT("Reveal should activate golem behavior."), GolemBehavior->IsBehaviorActive());
	TestEqual(TEXT("Golem behavior should target the revealed player."), GolemBehavior->GetTargetActor(), Cast<AActor>(PlayerCharacter));

	const float FearAfterReveal = Fear->GetFearValue();
	TestTrue(TEXT("Revealed encounter should resolve."), Director->ResolveEncounter());
	TestFalse(TEXT("Resolve should deactivate golem behavior."), GolemBehavior->IsBehaviorActive());
	TestTrue(TEXT("Resolve should relieve some player fear pressure."), Fear->GetFearValue() < FearAfterReveal);

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorEncounterPublishesAudioStageEventsTest,
	"HorrorProject.Game.Encounter.PublishesAudioStageEvents",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorEncounterPublishesAudioStageEventsTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for encounter audio event coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	UHorrorEventBusSubsystem* EventBus = World->GetSubsystem<UHorrorEventBusSubsystem>();
	TestNotNull(TEXT("Encounter audio event test should expose the event bus."), EventBus);
	if (!EventBus)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	UHorrorAudioSubsystem* AudioSubsystem = World->GetSubsystem<UHorrorAudioSubsystem>();
	TestNotNull(TEXT("Encounter audio event test should expose the audio subsystem."), AudioSubsystem);
	if (!AudioSubsystem)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	const FDelegateHandle AudioEventHandle = EventBus->GetOnEventPublishedNative().AddLambda(
		[AudioSubsystem](const FHorrorEventMessage& Message)
		{
			AudioSubsystem->HandleDay1Event(Message.EventTag, Message.SourceId);
		});

	AHorrorEncounterDirector* Director = World->SpawnActor<AHorrorEncounterDirector>(FVector::ZeroVector, FRotator::ZeroRotator);
	AActor* PlayerActor = World->SpawnActor<AActor>(FVector(100.0f, 0.0f, 0.0f), FRotator::ZeroRotator);
	TestNotNull(TEXT("Encounter audio event test should spawn a director."), Director);
	TestNotNull(TEXT("Encounter audio event test should spawn a player stand-in."), PlayerActor);
	if (!Director || !PlayerActor)
	{
		EventBus->GetOnEventPublishedNative().Remove(AudioEventHandle);
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	const FName AuthoredEncounterId(TEXT("Encounter.AudioStageEvents.Custom"));
	const FGameplayTag PrimeEventTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Encounter.Primed")), false);
	const FGameplayTag RevealEventTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Encounter.Revealed")), false);
	const FGameplayTag ResolveEventTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Encounter.Resolved")), false);
	TestTrue(TEXT("Encounter prime event tag should be registered for audio subscription."), PrimeEventTag.IsValid());
	TestTrue(TEXT("Encounter reveal event tag should be registered for audio subscription."), RevealEventTag.IsValid());
	TestTrue(TEXT("Encounter resolve event tag should be registered for audio subscription."), ResolveEventTag.IsValid());

	Director->ThreatActor = nullptr;
	Director->ThreatClass = nullptr;
	Director->RevealRadius = 0.0f;

	TestTrue(TEXT("Encounter should prime with an authored id that is not itself a registered gameplay tag."), Director->PrimeEncounter(AuthoredEncounterId));
	TestEqual(TEXT("Prime should publish an audio-subscribable encounter event."), AudioSubsystem->GetLastDay1AudioEventTag(), PrimeEventTag);
	TestEqual(TEXT("Prime should move Day1 audio to anomaly tension."), AudioSubsystem->GetDay1AudioStage(), EHorrorDay1AudioStage::Anomaly);

	TestTrue(TEXT("Encounter should reveal after prime."), Director->TriggerReveal(PlayerActor));
	TestEqual(TEXT("Reveal should publish an audio-subscribable encounter event."), AudioSubsystem->GetLastDay1AudioEventTag(), RevealEventTag);
	TestEqual(TEXT("Reveal should move Day1 audio to chase tension."), AudioSubsystem->GetDay1AudioStage(), EHorrorDay1AudioStage::Chase);

	TestTrue(TEXT("Encounter should resolve after reveal."), Director->ResolveEncounter());
	TestEqual(TEXT("Resolve should publish an audio-subscribable encounter event."), AudioSubsystem->GetLastDay1AudioEventTag(), ResolveEventTag);
	TestEqual(TEXT("Resolve should move Day1 audio to resolved ambience."), AudioSubsystem->GetDay1AudioStage(), EHorrorDay1AudioStage::Resolved);

	const TArray<FHorrorEventMessage>& History = EventBus->GetHistory();
	TestEqual(TEXT("Prime, reveal, and resolve should each publish one gameplay event."), History.Num(), 3);
	if (History.Num() == 3)
	{
		TestEqual(TEXT("First encounter event should be prime."), History[0].EventTag, PrimeEventTag);
		TestEqual(TEXT("Second encounter event should be reveal."), History[1].EventTag, RevealEventTag);
		TestEqual(TEXT("Third encounter event should be resolve."), History[2].EventTag, ResolveEventTag);
	}

	EventBus->GetOnEventPublishedNative().Remove(AudioEventHandle);
	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

#endif
