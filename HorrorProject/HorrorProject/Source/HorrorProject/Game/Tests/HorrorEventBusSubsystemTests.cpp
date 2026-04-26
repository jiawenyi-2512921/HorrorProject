#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR && HORRORPROJECT_ENABLE_LEGACY_AUTOMATION_TESTS

// Copyright Epic Games, Inc. All Rights Reserved.

#if WITH_DEV_AUTOMATION_TESTS

#include "Game/HorrorEventBusSubsystem.h"

#include "Engine/World.h"
#include "Game/HorrorFoundFootageContract.h"
#include "Game/HorrorGameModeBase.h"
#include "GameFramework/WorldSettings.h"
#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"

namespace
{
	bool CreateEventBusTestWorld(FAutomationTestBase& Test, FTestWorldWrapper& TestWorld, UWorld*& OutWorld, UHorrorEventBusSubsystem*& OutEventBus)
	{
		OutWorld = nullptr;
		OutEventBus = nullptr;

		Test.TestTrue(TEXT("Transient game world should be created for EventBus coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
		OutWorld = TestWorld.GetTestWorld();
		if (!OutWorld)
		{
			return false;
		}

		OutEventBus = OutWorld->GetSubsystem<UHorrorEventBusSubsystem>();
		Test.TestNotNull(TEXT("Transient world should expose the horror event bus subsystem."), OutEventBus);
		if (!OutEventBus)
		{
			TestWorld.DestroyTestWorld(false);
			return false;
		}

		OutEventBus->ResetForTests();
		return true;
	}

	AHorrorGameModeBase* CreateEventBusGameModeTestWorld(FAutomationTestBase& Test, FTestWorldWrapper& TestWorld, UWorld*& OutWorld, UHorrorEventBusSubsystem*& OutEventBus)
	{
		if (!CreateEventBusTestWorld(Test, TestWorld, OutWorld, OutEventBus))
		{
			return nullptr;
		}

		OutWorld->GetWorldSettings()->DefaultGameMode = AHorrorGameModeBase::StaticClass();
		Test.TestTrue(TEXT("Transient world should create the found-footage game mode."), OutWorld->SetGameMode(FURL()));

		AHorrorGameModeBase* GameMode = OutWorld->GetAuthGameMode<AHorrorGameModeBase>();
		Test.TestNotNull(TEXT("Transient world should expose the found-footage game mode."), GameMode);
		if (!GameMode)
		{
			TestWorld.DestroyTestWorld(false);
		}

		return GameMode;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorEventBusDirectPublishTest,
	"HorrorProject.Game.EventBus.DirectPublishBroadcastsAndStoresHistory",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorEventBusDirectPublishTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	UWorld* World = nullptr;
	UHorrorEventBusSubsystem* EventBus = nullptr;
	if (!CreateEventBusTestWorld(*this, TestWorld, World, EventBus))
	{
		return false;
	}

	int32 BroadcastCount = 0;
	FHorrorEventMessage LastMessage;
	FDelegateHandle DelegateHandle = EventBus->GetOnEventPublishedNative().AddLambda(
		[&BroadcastCount, &LastMessage](const FHorrorEventMessage& Message)
		{
			++BroadcastCount;
			LastMessage = Message;
		});

	TestTrue(
		TEXT("Valid direct publish should succeed."),
		EventBus->Publish(
			HorrorFoundFootageTags::BodycamAcquiredEvent(),
			TEXT("BodycamPickup"),
			HorrorFoundFootageTags::BodycamAcquiredState(),
			World));

	const TArray<FHorrorEventMessage>& History = EventBus->GetHistory();
	TestEqual(TEXT("Direct publish should broadcast exactly once."), BroadcastCount, 1);
	TestEqual(TEXT("Direct publish should store exactly one history entry."), History.Num(), 1);
	if (History.Num() == 1)
	{
		TestEqual(TEXT("History should store the published event tag."), History[0].EventTag, HorrorFoundFootageTags::BodycamAcquiredEvent());
		TestEqual(TEXT("History should store the associated state tag."), History[0].StateTag, HorrorFoundFootageTags::BodycamAcquiredState());
		TestEqual(TEXT("History should store the source id."), History[0].SourceId, FName(TEXT("BodycamPickup")));
		TestNull(TEXT("History should not retain the transient source object."), History[0].SourceObject.Get());
		TestTrue(TEXT("History should store a non-negative world timestamp."), History[0].WorldSeconds >= 0.0f);
		TestEqual(TEXT("Unknown metadata should leave trailer beat empty."), History[0].TrailerBeatId, NAME_None);
		TestTrue(TEXT("Unknown metadata should leave objective hint empty."), History[0].ObjectiveHint.IsEmpty());
	}
	TestEqual(TEXT("Native delegate should receive the published event tag."), LastMessage.EventTag, HorrorFoundFootageTags::BodycamAcquiredEvent());

	EventBus->GetOnEventPublishedNative().Remove(DelegateHandle);
	EventBus->ResetForTests();
	DelegateHandle = EventBus->GetOnEventPublishedNative().AddLambda(
		[&BroadcastCount, &LastMessage](const FHorrorEventMessage& Message)
		{
			++BroadcastCount;
			LastMessage = Message;
		});
	FHorrorObjectiveMessageMetadata BodycamMetadata;
	BodycamMetadata.TrailerBeatId = TEXT("Beat.BodycamAcquire");
	BodycamMetadata.ObjectiveHint = FText::FromString(TEXT("Recover the bodycam."));
	BodycamMetadata.DebugLabel = FText::FromString(TEXT("Bodycam Pickup"));
	EventBus->RegisterObjectiveMetadata(TEXT("BodycamPickup"), BodycamMetadata);
	FHorrorObjectiveMessageMetadata StoredMetadata;
	TestTrue(TEXT("EventBus should retain registered objective metadata by source id."), EventBus->GetObjectiveMetadataForTests(TEXT("BodycamPickup"), StoredMetadata));
	TestEqual(TEXT("Stored metadata should preserve trailer beat id."), StoredMetadata.TrailerBeatId, FName(TEXT("Beat.BodycamAcquire")));
	BroadcastCount = 0;
	TestTrue(
		TEXT("Metadata direct publish should succeed."),
		EventBus->Publish(
			HorrorFoundFootageTags::BodycamAcquiredEvent(),
			TEXT("BodycamPickup"),
			HorrorFoundFootageTags::BodycamAcquiredState(),
			World));
	const TArray<FHorrorEventMessage>& MetadataHistory = EventBus->GetHistory();
	TestEqual(TEXT("Metadata direct publish should broadcast exactly once."), BroadcastCount, 1);
	TestEqual(TEXT("Metadata direct publish should store exactly one history entry."), MetadataHistory.Num(), 1);
	if (MetadataHistory.Num() == 1)
	{
		TestEqual(TEXT("Metadata history should carry trailer beat id."), MetadataHistory[0].TrailerBeatId, FName(TEXT("Beat.BodycamAcquire")));
		TestEqual(TEXT("Metadata history should carry objective hint."), MetadataHistory[0].ObjectiveHint.ToString(), FString(TEXT("Recover the bodycam.")));
		TestEqual(TEXT("Metadata history should carry debug label."), MetadataHistory[0].DebugLabel.ToString(), FString(TEXT("Bodycam Pickup")));
	}
	TestEqual(TEXT("Metadata native delegate should carry trailer beat id."), LastMessage.TrailerBeatId, FName(TEXT("Beat.BodycamAcquire")));

	EventBus->ResetForTests();
	FHorrorObjectiveMessageMetadata SourceFallbackMetadata;
	SourceFallbackMetadata.TrailerBeatId = TEXT("Beat.SharedFallback");
	SourceFallbackMetadata.ObjectiveHint = FText::FromString(TEXT("Fallback hint."));
	SourceFallbackMetadata.DebugLabel = FText::FromString(TEXT("Fallback Label"));
	EventBus->RegisterObjectiveMetadata(TEXT("SharedObjective"), SourceFallbackMetadata);

	FHorrorObjectiveMessageMetadata EventOverrideMetadata;
	EventOverrideMetadata.TrailerBeatId = TEXT("Beat.SharedOverride");
	EventOverrideMetadata.ObjectiveHint = FText::FromString(TEXT("Override hint."));
	EventOverrideMetadata.DebugLabel = FText::FromString(TEXT("Override Label"));
	EventBus->RegisterObjectiveMetadata(HorrorFoundFootageTags::ArchiveReviewedEvent(), TEXT("SharedObjective"), EventOverrideMetadata);

	TestTrue(TEXT("Matching event-specific metadata publish should succeed."), EventBus->Publish(HorrorFoundFootageTags::ArchiveReviewedEvent(), TEXT("SharedObjective"), HorrorFoundFootageTags::ArchiveReviewedState(), World));
	TestTrue(TEXT("Non-matching event metadata publish should succeed."), EventBus->Publish(HorrorFoundFootageTags::BodycamAcquiredEvent(), TEXT("SharedObjective"), HorrorFoundFootageTags::BodycamAcquiredState(), World));
	const TArray<FHorrorEventMessage>& PrecedenceHistory = EventBus->GetHistory();
	TestEqual(TEXT("Metadata precedence publishes should produce two history entries."), PrecedenceHistory.Num(), 2);
	if (PrecedenceHistory.Num() == 2)
	{
		TestEqual(TEXT("Event-specific metadata should override source fallback for matching event."), PrecedenceHistory[0].TrailerBeatId, FName(TEXT("Beat.SharedOverride")));
		TestEqual(TEXT("Event-specific metadata should preserve matching objective hint."), PrecedenceHistory[0].ObjectiveHint.ToString(), FString(TEXT("Override hint.")));
		TestEqual(TEXT("Source metadata should remain fallback for non-matching event."), PrecedenceHistory[1].TrailerBeatId, FName(TEXT("Beat.SharedFallback")));
		TestEqual(TEXT("Source metadata fallback should preserve objective hint."), PrecedenceHistory[1].ObjectiveHint.ToString(), FString(TEXT("Fallback hint.")));
	}

	EventBus->UnregisterObjectiveMetadata(HorrorFoundFootageTags::ArchiveReviewedEvent(), TEXT("SharedObjective"));
	FHorrorObjectiveMessageMetadata RemovedOverrideMetadata;
	TestFalse(TEXT("Unregistered event-specific metadata should no longer be queryable."), EventBus->GetObjectiveMetadataForTests(HorrorFoundFootageTags::ArchiveReviewedEvent(), TEXT("SharedObjective"), RemovedOverrideMetadata));
	EventBus->UnregisterObjectiveMetadata(FGameplayTag(), TEXT("SharedObjective"));
	EventBus->UnregisterObjectiveMetadata(HorrorFoundFootageTags::ArchiveReviewedEvent(), NAME_None);
	TestTrue(TEXT("Invalid unregister calls should not remove source fallback metadata."), EventBus->GetObjectiveMetadataForTests(TEXT("SharedObjective"), RemovedOverrideMetadata));
	TestTrue(TEXT("Post-unregister archive publish should succeed."), EventBus->Publish(HorrorFoundFootageTags::ArchiveReviewedEvent(), TEXT("SharedObjective"), HorrorFoundFootageTags::ArchiveReviewedState(), World));
	const TArray<FHorrorEventMessage>& PostUnregisterHistory = EventBus->GetHistory();
	TestEqual(TEXT("Post-unregister publish should append one history entry."), PostUnregisterHistory.Num(), 3);
	if (PostUnregisterHistory.Num() == 3)
	{
		TestEqual(TEXT("Unregistered override should restore source fallback trailer beat."), PostUnregisterHistory[2].TrailerBeatId, FName(TEXT("Beat.SharedFallback")));
		TestEqual(TEXT("Unregistered override should restore source fallback hint."), PostUnregisterHistory[2].ObjectiveHint.ToString(), FString(TEXT("Fallback hint.")));
		TestEqual(TEXT("Unregistered override should restore source fallback label."), PostUnregisterHistory[2].DebugLabel.ToString(), FString(TEXT("Fallback Label")));
	}

	EventBus->GetOnEventPublishedNative().Remove(DelegateHandle);
	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorEventBusMetadataGuardrailsTest,
	"HorrorProject.Game.EventBus.MetadataRegistrationGuardrails",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorEventBusMetadataGuardrailsTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	UWorld* World = nullptr;
	UHorrorEventBusSubsystem* EventBus = nullptr;
	if (!CreateEventBusTestWorld(*this, TestWorld, World, EventBus))
	{
		return false;
	}

	FHorrorObjectiveMessageMetadata StoredMetadata;
	FHorrorObjectiveMessageMetadata EmptyMetadata;
	EventBus->RegisterObjectiveMetadata(NAME_None, EmptyMetadata);
	EventBus->RegisterObjectiveMetadata(TEXT("EmptySource"), EmptyMetadata);
	TestFalse(TEXT("NAME_None source metadata should not be registered."), EventBus->GetObjectiveMetadataForTests(NAME_None, StoredMetadata));
	TestFalse(TEXT("Empty source metadata should not be registered."), EventBus->GetObjectiveMetadataForTests(TEXT("EmptySource"), StoredMetadata));

	FHorrorObjectiveMessageMetadata SourceFallbackMetadata;
	SourceFallbackMetadata.TrailerBeatId = TEXT("Beat.GuardrailFallback");
	SourceFallbackMetadata.ObjectiveHint = FText::FromString(TEXT("Fallback survives invalid overrides."));
	SourceFallbackMetadata.DebugLabel = FText::FromString(TEXT("Guardrail Fallback"));
	EventBus->RegisterObjectiveMetadata(TEXT("GuardrailSource"), SourceFallbackMetadata);
	TestTrue(TEXT("Valid source metadata should be registered."), EventBus->GetObjectiveMetadataForTests(TEXT("GuardrailSource"), StoredMetadata));
	EventBus->RegisterObjectiveMetadata(TEXT("GuardrailSource"), EmptyMetadata);
	TestFalse(TEXT("Empty source metadata should clear stale registered metadata."), EventBus->GetObjectiveMetadataForTests(TEXT("GuardrailSource"), StoredMetadata));
	EventBus->RegisterObjectiveMetadata(TEXT("GuardrailSource"), SourceFallbackMetadata);

	FHorrorObjectiveMessageMetadata InvalidOverrideMetadata;
	InvalidOverrideMetadata.TrailerBeatId = TEXT("Beat.InvalidOverride");
	InvalidOverrideMetadata.ObjectiveHint = FText::FromString(TEXT("Invalid override."));
	InvalidOverrideMetadata.DebugLabel = FText::FromString(TEXT("Invalid Override"));
	EventBus->RegisterObjectiveMetadata(FGameplayTag(), TEXT("GuardrailSource"), InvalidOverrideMetadata);
	EventBus->RegisterObjectiveMetadata(HorrorFoundFootageTags::ArchiveReviewedEvent(), NAME_None, InvalidOverrideMetadata);
	EventBus->RegisterObjectiveMetadata(HorrorFoundFootageTags::ArchiveReviewedEvent(), TEXT("GuardrailSource"), InvalidOverrideMetadata);
	TestTrue(TEXT("Valid event-specific metadata should be registered."), EventBus->GetObjectiveMetadataForTests(HorrorFoundFootageTags::ArchiveReviewedEvent(), TEXT("GuardrailSource"), StoredMetadata));
	EventBus->RegisterObjectiveMetadata(HorrorFoundFootageTags::ArchiveReviewedEvent(), TEXT("GuardrailSource"), EmptyMetadata);
	TestFalse(TEXT("Empty event-specific metadata should clear stale override metadata."), EventBus->GetObjectiveMetadataForTests(HorrorFoundFootageTags::ArchiveReviewedEvent(), TEXT("GuardrailSource"), StoredMetadata));

	TestTrue(TEXT("Guardrail publish should succeed."), EventBus->Publish(HorrorFoundFootageTags::ArchiveReviewedEvent(), TEXT("GuardrailSource"), HorrorFoundFootageTags::ArchiveReviewedState(), World));
	const TArray<FHorrorEventMessage>& History = EventBus->GetHistory();
	TestEqual(TEXT("Guardrail publish should store one history entry."), History.Num(), 1);
	if (History.Num() == 1)
	{
		TestEqual(TEXT("Invalid event-specific metadata should not mask source fallback trailer beat."), History[0].TrailerBeatId, FName(TEXT("Beat.GuardrailFallback")));
		TestEqual(TEXT("Invalid event-specific metadata should not mask source fallback hint."), History[0].ObjectiveHint.ToString(), FString(TEXT("Fallback survives invalid overrides.")));
		TestEqual(TEXT("Invalid event-specific metadata should not mask source fallback label."), History[0].DebugLabel.ToString(), FString(TEXT("Guardrail Fallback")));
	}

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorEventBusInvalidPublishTest,
	"HorrorProject.Game.EventBus.InvalidTagPublishFailsWithoutHistory",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorEventBusInvalidPublishTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	UWorld* World = nullptr;
	UHorrorEventBusSubsystem* EventBus = nullptr;
	if (!CreateEventBusTestWorld(*this, TestWorld, World, EventBus))
	{
		return false;
	}

	int32 BroadcastCount = 0;
	const FDelegateHandle DelegateHandle = EventBus->GetOnEventPublishedNative().AddLambda(
		[&BroadcastCount](const FHorrorEventMessage& Message)
		{
			++BroadcastCount;
		});

	TestFalse(
		TEXT("Invalid event tags should not publish."),
		EventBus->Publish(FGameplayTag(), TEXT("Invalid"), HorrorFoundFootageTags::BodycamAcquiredState(), World));
	TestEqual(TEXT("Invalid publishes should not broadcast."), BroadcastCount, 0);
	TestEqual(TEXT("Invalid publishes should not enter history."), EventBus->GetHistory().Num(), 0);

	EventBus->GetOnEventPublishedNative().Remove(DelegateHandle);
	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorEventBusHistoryCapacityTest,
	"HorrorProject.Game.EventBus.HistoryCapacityDropsOldestEntries",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorEventBusHistoryCapacityTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	UWorld* World = nullptr;
	UHorrorEventBusSubsystem* EventBus = nullptr;
	if (!CreateEventBusTestWorld(*this, TestWorld, World, EventBus))
	{
		return false;
	}

	EventBus->SetHistoryCapacityForTests(2);
	TestTrue(TEXT("First capacity publish should succeed."), EventBus->Publish(HorrorFoundFootageTags::BodycamAcquiredEvent(), TEXT("First"), HorrorFoundFootageTags::BodycamAcquiredState(), World));
	TestTrue(TEXT("Second capacity publish should succeed."), EventBus->Publish(HorrorFoundFootageTags::FirstNoteCollectedEvent(), TEXT("Second"), HorrorFoundFootageTags::FirstNoteCollectedState(), World));
	TestTrue(TEXT("Third capacity publish should succeed."), EventBus->Publish(HorrorFoundFootageTags::ArchiveReviewedEvent(), TEXT("Third"), HorrorFoundFootageTags::ArchiveReviewedState(), World));

	const TArray<FHorrorEventMessage>& History = EventBus->GetHistory();
	TestEqual(TEXT("EventBus history should retain only the configured capacity."), History.Num(), 2);
	if (History.Num() == 2)
	{
		TestEqual(TEXT("History capacity should drop the oldest event first."), History[0].SourceId, FName(TEXT("Second")));
		TestEqual(TEXT("History capacity should retain the newest event."), History[1].SourceId, FName(TEXT("Third")));
	}

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorEventBusGameModeObjectivePublishTest,
	"HorrorProject.Game.EventBus.GameModeObjectiveEventsPublishOnce",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorEventBusGameModeObjectivePublishTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	UWorld* World = nullptr;
	UHorrorEventBusSubsystem* EventBus = nullptr;
	AHorrorGameModeBase* GameMode = CreateEventBusGameModeTestWorld(*this, TestWorld, World, EventBus);
	if (!GameMode || !EventBus)
	{
		return false;
	}

	TestTrue(TEXT("Bodycam objective should record through GameMode."), GameMode->TryAcquireBodycam(TEXT("BodycamPickup"), true));
	TestEqual(TEXT("Bodycam objective should publish one event."), EventBus->GetHistory().Num(), 1);
	TestFalse(TEXT("Duplicate bodycam objective should be rejected."), GameMode->TryAcquireBodycam(TEXT("BodycamPickup"), true));
	TestEqual(TEXT("Duplicate bodycam objective should not publish again."), EventBus->GetHistory().Num(), 1);
	if (EventBus->GetHistory().Num() >= 1)
	{
		TestEqual(TEXT("Bodycam publish should carry the objective event."), EventBus->GetHistory()[0].EventTag, HorrorFoundFootageTags::BodycamAcquiredEvent());
		TestEqual(TEXT("Bodycam publish should carry the objective state."), EventBus->GetHistory()[0].StateTag, HorrorFoundFootageTags::BodycamAcquiredState());
	}

	TestTrue(TEXT("First note objective should record through GameMode."), GameMode->TryCollectFirstNote(TEXT("Note01")));
	TestEqual(TEXT("First note objective should publish one new event."), EventBus->GetHistory().Num(), 2);
	TestFalse(TEXT("Duplicate first note objective should be rejected."), GameMode->TryCollectFirstNote(TEXT("Note01")));
	TestEqual(TEXT("Duplicate first note objective should not publish again."), EventBus->GetHistory().Num(), 2);

	TestTrue(TEXT("First anomaly candidate should register through GameMode."), GameMode->BeginFirstAnomalyCandidate(TEXT("Anomaly01")));
	TestFalse(TEXT("Non-recording first anomaly should be rejected."), GameMode->TryRecordFirstAnomaly(false));
	TestEqual(TEXT("Rejected first anomaly recording should not publish."), EventBus->GetHistory().Num(), 2);
	TestTrue(TEXT("Recording first anomaly should record through GameMode."), GameMode->TryRecordFirstAnomaly(true));
	TestEqual(TEXT("Recording first anomaly should publish one new event."), EventBus->GetHistory().Num(), 3);
	TestFalse(TEXT("Duplicate first anomaly recording should be rejected."), GameMode->TryRecordFirstAnomaly(true));
	TestEqual(TEXT("Duplicate first anomaly recording should not publish again."), EventBus->GetHistory().Num(), 3);

	TestTrue(TEXT("Archive review should record through GameMode."), GameMode->TryReviewArchive(TEXT("ArchiveTerminal")));
	TestEqual(TEXT("Archive review should publish its event and automatic exit unlock."), EventBus->GetHistory().Num(), 5);
	TestFalse(TEXT("Duplicate archive review should be rejected."), GameMode->TryReviewArchive(TEXT("ArchiveTerminal")));
	TestEqual(TEXT("Duplicate archive review should not publish again."), EventBus->GetHistory().Num(), 5);

	const TArray<FHorrorEventMessage>& History = EventBus->GetHistory();
	if (History.Num() >= 5)
	{
		TestEqual(TEXT("Final explicit objective publish should be archive reviewed."), History[3].EventTag, HorrorFoundFootageTags::ArchiveReviewedEvent());
		TestEqual(TEXT("Full found-footage sequence should publish Event.Exit.Unlocked."), History[4].EventTag, HorrorFoundFootageTags::ExitUnlockedEvent());
		TestEqual(TEXT("Exit unlock publish should carry the exit-unlocked state."), History[4].StateTag, HorrorFoundFootageTags::ExitUnlockedState());
		TestEqual(TEXT("Exit unlock publish should preserve the contract source id."), History[4].SourceId, FName(TEXT("FoundFootageContract")));
	}

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

#endif

#endif // WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR
