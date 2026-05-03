#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR

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

	int32 CountHistoryEventsByTag(const UHorrorEventBusSubsystem& EventBus, FGameplayTag EventTag)
	{
		int32 MatchCount = 0;
		for (const FHorrorEventMessage& Message : EventBus.GetHistory())
		{
			if (Message.EventTag == EventTag)
			{
				++MatchCount;
			}
		}
		return MatchCount;
	}

	const FHorrorEventMessage* FindHistoryEventByTag(const UHorrorEventBusSubsystem& EventBus, FGameplayTag EventTag)
	{
		return EventBus.GetHistory().FindByPredicate(
			[EventTag](const FHorrorEventMessage& Message)
			{
				return Message.EventTag == EventTag;
			});
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
	BodycamMetadata.FeedbackSeverity = EHorrorObjectiveFeedbackSeverity::Success;
	BodycamMetadata.bRetryable = false;
	BodycamMetadata.DisplaySeconds = 4.25f;
	EventBus->RegisterObjectiveMetadata(TEXT("BodycamPickup"), BodycamMetadata);
	FHorrorObjectiveMessageMetadata StoredMetadata;
	TestTrue(TEXT("EventBus should retain registered objective metadata by source id."), EventBus->GetObjectiveMetadataForTests(TEXT("BodycamPickup"), StoredMetadata));
	TestEqual(TEXT("Stored metadata should preserve trailer beat id."), StoredMetadata.TrailerBeatId, FName(TEXT("Beat.BodycamAcquire")));
	TestEqual(TEXT("Stored metadata should preserve feedback severity."), StoredMetadata.FeedbackSeverity, EHorrorObjectiveFeedbackSeverity::Success);
	TestTrue(TEXT("Stored metadata should preserve display duration."), FMath::IsNearlyEqual(StoredMetadata.DisplaySeconds, 4.25f));
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
		TestEqual(TEXT("Metadata history should carry feedback severity."), MetadataHistory[0].FeedbackSeverity, EHorrorObjectiveFeedbackSeverity::Success);
		TestFalse(TEXT("Metadata history should carry retryable flag."), MetadataHistory[0].bRetryable);
		TestTrue(TEXT("Metadata history should carry display duration."), FMath::IsNearlyEqual(MetadataHistory[0].DisplaySeconds, 4.25f));
	}
	TestEqual(TEXT("Metadata native delegate should carry trailer beat id."), LastMessage.TrailerBeatId, FName(TEXT("Beat.BodycamAcquire")));
	TestEqual(TEXT("Metadata native delegate should carry feedback severity."), LastMessage.FeedbackSeverity, EHorrorObjectiveFeedbackSeverity::Success);

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
	FHorrorEventBusHistoryCapacityPreservesChronologicalViewTest,
	"HorrorProject.Game.EventBus.HistoryCapacityPreservesChronologicalView",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorEventBusHistoryCapacityPreservesChronologicalViewTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	UWorld* World = nullptr;
	UHorrorEventBusSubsystem* EventBus = nullptr;
	if (!CreateEventBusTestWorld(*this, TestWorld, World, EventBus))
	{
		return false;
	}

	EventBus->SetHistoryCapacityForTests(3);
	TestTrue(TEXT("Capacity publish 1 should succeed."), EventBus->Publish(HorrorFoundFootageTags::BodycamAcquiredEvent(), TEXT("One"), HorrorFoundFootageTags::BodycamAcquiredState(), World));
	TestTrue(TEXT("Capacity publish 2 should succeed."), EventBus->Publish(HorrorFoundFootageTags::FirstNoteCollectedEvent(), TEXT("Two"), HorrorFoundFootageTags::FirstNoteCollectedState(), World));
	TestTrue(TEXT("Capacity publish 3 should succeed."), EventBus->Publish(HorrorFoundFootageTags::FirstAnomalyRecordedEvent(), TEXT("Three"), HorrorFoundFootageTags::FirstAnomalyRecordedState(), World));
	TestTrue(TEXT("Capacity publish 4 should succeed."), EventBus->Publish(HorrorFoundFootageTags::ArchiveReviewedEvent(), TEXT("Four"), HorrorFoundFootageTags::ArchiveReviewedState(), World));
	TestTrue(TEXT("Capacity publish 5 should succeed."), EventBus->Publish(HorrorFoundFootageTags::ExitUnlockedEvent(), TEXT("Five"), HorrorFoundFootageTags::ExitUnlockedState(), World));

	const TArray<FHorrorEventMessage>& History = EventBus->GetHistory();
	TestEqual(TEXT("Chronological history view should keep configured capacity."), History.Num(), 3);
	if (History.Num() == 3)
	{
		TestEqual(TEXT("Chronological view should expose the oldest retained message first."), History[0].SourceId, FName(TEXT("Three")));
		TestEqual(TEXT("Chronological view should expose the middle retained message second."), History[1].SourceId, FName(TEXT("Four")));
		TestEqual(TEXT("Chronological view should expose the newest retained message last."), History[2].SourceId, FName(TEXT("Five")));
	}

	const TArray<FHorrorEventMessage>& CachedHistory = EventBus->GetHistory();
	TestEqual(TEXT("Repeated chronological history reads should be stable."), CachedHistory.Num(), History.Num());
	if (CachedHistory.Num() == 3)
	{
		TestEqual(TEXT("Repeated chronological view should keep the newest retained message last."), CachedHistory[2].SourceId, FName(TEXT("Five")));
	}

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorEventBusEventSpecificMetadataIsolationTest,
	"HorrorProject.Game.EventBus.EventSpecificMetadataUsesStructuredKeys",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorEventBusEventSpecificMetadataIsolationTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	UWorld* World = nullptr;
	UHorrorEventBusSubsystem* EventBus = nullptr;
	if (!CreateEventBusTestWorld(*this, TestWorld, World, EventBus))
	{
		return false;
	}

	FHorrorObjectiveMessageMetadata ArchiveMetadata;
	ArchiveMetadata.TrailerBeatId = TEXT("Beat.Archive.Only");
	ArchiveMetadata.ObjectiveHint = FText::FromString(TEXT("只用于档案复查事件。"));
	ArchiveMetadata.FeedbackSeverity = EHorrorObjectiveFeedbackSeverity::Success;
	EventBus->RegisterObjectiveMetadata(HorrorFoundFootageTags::ArchiveReviewedEvent(), TEXT("SharedTerminal"), ArchiveMetadata);

	FHorrorObjectiveMessageMetadata ExitMetadata;
	ExitMetadata.TrailerBeatId = TEXT("Beat.Exit.Only");
	ExitMetadata.ObjectiveHint = FText::FromString(TEXT("只用于出口解锁事件。"));
	ExitMetadata.FeedbackSeverity = EHorrorObjectiveFeedbackSeverity::Critical;
	EventBus->RegisterObjectiveMetadata(HorrorFoundFootageTags::ExitUnlockedEvent(), TEXT("SharedTerminal"), ExitMetadata);

	FHorrorObjectiveMessageMetadata StoredArchiveMetadata;
	FHorrorObjectiveMessageMetadata StoredExitMetadata;
	TestTrue(TEXT("Archive-specific metadata should be retrievable by exact event/source pair."), EventBus->GetObjectiveMetadataForTests(HorrorFoundFootageTags::ArchiveReviewedEvent(), TEXT("SharedTerminal"), StoredArchiveMetadata));
	TestTrue(TEXT("Exit-specific metadata should be retrievable by exact event/source pair."), EventBus->GetObjectiveMetadataForTests(HorrorFoundFootageTags::ExitUnlockedEvent(), TEXT("SharedTerminal"), StoredExitMetadata));
	TestEqual(TEXT("Archive metadata should not be replaced by another event for the same source."), StoredArchiveMetadata.TrailerBeatId, FName(TEXT("Beat.Archive.Only")));
	TestEqual(TEXT("Exit metadata should stay isolated from archive metadata."), StoredExitMetadata.TrailerBeatId, FName(TEXT("Beat.Exit.Only")));

	TestTrue(TEXT("Archive publish should succeed."), EventBus->Publish(HorrorFoundFootageTags::ArchiveReviewedEvent(), TEXT("SharedTerminal"), HorrorFoundFootageTags::ArchiveReviewedState(), World));
	TestTrue(TEXT("Exit publish should succeed."), EventBus->Publish(HorrorFoundFootageTags::ExitUnlockedEvent(), TEXT("SharedTerminal"), HorrorFoundFootageTags::ExitUnlockedState(), World));

	const TArray<FHorrorEventMessage>& History = EventBus->GetHistory();
	TestEqual(TEXT("Isolated metadata publishes should produce two history entries."), History.Num(), 2);
	if (History.Num() == 2)
	{
		TestEqual(TEXT("Archive event should carry only archive metadata."), History[0].TrailerBeatId, FName(TEXT("Beat.Archive.Only")));
		TestEqual(TEXT("Archive event should preserve archive severity."), History[0].FeedbackSeverity, EHorrorObjectiveFeedbackSeverity::Success);
		TestEqual(TEXT("Exit event should carry only exit metadata."), History[1].TrailerBeatId, FName(TEXT("Beat.Exit.Only")));
		TestEqual(TEXT("Exit event should preserve exit severity."), History[1].FeedbackSeverity, EHorrorObjectiveFeedbackSeverity::Critical);
	}

	EventBus->UnregisterObjectiveMetadata(HorrorFoundFootageTags::ArchiveReviewedEvent(), TEXT("SharedTerminal"));
	TestFalse(TEXT("Removing archive metadata should not leave an archive override behind."), EventBus->GetObjectiveMetadataForTests(HorrorFoundFootageTags::ArchiveReviewedEvent(), TEXT("SharedTerminal"), StoredArchiveMetadata));
	TestTrue(TEXT("Removing archive metadata should not remove exit metadata for the same source."), EventBus->GetObjectiveMetadataForTests(HorrorFoundFootageTags::ExitUnlockedEvent(), TEXT("SharedTerminal"), StoredExitMetadata));

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
	TestEqual(TEXT("Bodycam objective event should publish once."), CountHistoryEventsByTag(*EventBus, HorrorFoundFootageTags::BodycamAcquiredEvent()), 1);
	const int32 HistoryCountAfterBodycam = EventBus->GetHistory().Num();
	TestFalse(TEXT("Duplicate bodycam objective should be rejected."), GameMode->TryAcquireBodycam(TEXT("BodycamPickup"), true));
	TestEqual(TEXT("Duplicate bodycam objective should not publish new events."), EventBus->GetHistory().Num(), HistoryCountAfterBodycam);
	if (const FHorrorEventMessage* BodycamMessage = FindHistoryEventByTag(*EventBus, HorrorFoundFootageTags::BodycamAcquiredEvent()))
	{
		TestEqual(TEXT("Bodycam publish should carry the objective state."), BodycamMessage->StateTag, HorrorFoundFootageTags::BodycamAcquiredState());
	}

	TestTrue(TEXT("First note objective should record through GameMode."), GameMode->TryCollectFirstNote(TEXT("Note01")));
	TestEqual(TEXT("First note objective event should publish once."), CountHistoryEventsByTag(*EventBus, HorrorFoundFootageTags::FirstNoteCollectedEvent()), 1);
	const int32 HistoryCountAfterFirstNote = EventBus->GetHistory().Num();
	TestFalse(TEXT("Duplicate first note objective should be rejected."), GameMode->TryCollectFirstNote(TEXT("Note01")));
	TestEqual(TEXT("Duplicate first note objective should not publish new events."), EventBus->GetHistory().Num(), HistoryCountAfterFirstNote);

	TestTrue(TEXT("First anomaly candidate should register through GameMode."), GameMode->BeginFirstAnomalyCandidate(TEXT("Anomaly01")));
	TestFalse(TEXT("Non-recording first anomaly should be rejected."), GameMode->TryRecordFirstAnomaly(false));
	TestEqual(TEXT("Rejected first anomaly recording should not publish new events."), EventBus->GetHistory().Num(), HistoryCountAfterFirstNote);
	TestTrue(TEXT("Recording first anomaly should record through GameMode."), GameMode->TryRecordFirstAnomaly(true));
	TestEqual(TEXT("First anomaly objective event should publish once."), CountHistoryEventsByTag(*EventBus, HorrorFoundFootageTags::FirstAnomalyRecordedEvent()), 1);
	const int32 HistoryCountAfterFirstAnomaly = EventBus->GetHistory().Num();
	TestFalse(TEXT("Duplicate first anomaly recording should be rejected."), GameMode->TryRecordFirstAnomaly(true));
	TestEqual(TEXT("Duplicate first anomaly recording should not publish new events."), EventBus->GetHistory().Num(), HistoryCountAfterFirstAnomaly);

	TestTrue(TEXT("Archive review should record through GameMode."), GameMode->TryReviewArchive(TEXT("ArchiveTerminal")));
	TestEqual(TEXT("Archive review objective event should publish once."), CountHistoryEventsByTag(*EventBus, HorrorFoundFootageTags::ArchiveReviewedEvent()), 1);
	TestEqual(TEXT("Full found-footage sequence should publish one exit unlock objective event."), CountHistoryEventsByTag(*EventBus, HorrorFoundFootageTags::ExitUnlockedEvent()), 1);
	const int32 HistoryCountAfterArchive = EventBus->GetHistory().Num();
	TestFalse(TEXT("Duplicate archive review should be rejected."), GameMode->TryReviewArchive(TEXT("ArchiveTerminal")));
	TestEqual(TEXT("Duplicate archive review should not publish new events."), EventBus->GetHistory().Num(), HistoryCountAfterArchive);

	if (const FHorrorEventMessage* ArchiveMessage = FindHistoryEventByTag(*EventBus, HorrorFoundFootageTags::ArchiveReviewedEvent()))
	{
		TestEqual(TEXT("Archive publish should carry the archive-reviewed state."), ArchiveMessage->StateTag, HorrorFoundFootageTags::ArchiveReviewedState());
	}
	if (const FHorrorEventMessage* ExitMessage = FindHistoryEventByTag(*EventBus, HorrorFoundFootageTags::ExitUnlockedEvent()))
	{
		TestEqual(TEXT("Exit unlock publish should carry the exit-unlocked state."), ExitMessage->StateTag, HorrorFoundFootageTags::ExitUnlockedState());
		TestEqual(TEXT("Exit unlock publish should preserve the contract source id."), ExitMessage->SourceId, FName(TEXT("FoundFootageContract")));
	}

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

#endif

#endif // WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR
