// Copyright Epic Games, Inc. All Rights Reserved.

#include "HorrorUI.h"
#include "Engine/World.h"
#include "Game/HorrorEventBusSubsystem.h"
#include "Game/HorrorGameModeBase.h"
#include "Player/HorrorPlayerCharacter.h"
#include "Player/Components/VHSEffectComponent.h"
#include "HorrorCharacter.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Player/Components/InventoryComponent.h"
#include "Player/Components/NoteRecorderComponent.h"

#endif

FHorrorArchiveSnapshot UHorrorUI::BuildArchiveSnapshot(const UInventoryComponent* InventoryComponent, const UNoteRecorderComponent* NoteRecorderComponent)
{
	FHorrorArchiveSnapshot Snapshot;

	if (InventoryComponent)
	{
		Snapshot.CollectedEvidenceIds = InventoryComponent->GetCollectedEvidenceIds();
		Snapshot.TotalEvidenceCount = InventoryComponent->GetCollectedEvidenceCount();
		Snapshot.CollectedEvidenceMetadata = InventoryComponent->GetCollectedEvidenceMetadata();
	}

	if (NoteRecorderComponent)
	{
		Snapshot.RecordedNoteIds = NoteRecorderComponent->GetRecordedNoteIds();
		Snapshot.TotalRecordedNotes = NoteRecorderComponent->GetRecordedNoteCount();
		Snapshot.RecordedNoteMetadata = NoteRecorderComponent->GetRecordedNoteMetadata();
	}

	return Snapshot;
}

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorUIArchiveSnapshotIncludesMetadataTest,
	"HorrorProject.UI.Archive.SnapshotIncludesMetadata",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FHorrorUIArchiveSnapshotIncludesMetadataTest::RunTest(const FString& Parameters)
{
	UInventoryComponent* InventoryComponent = NewObject<UInventoryComponent>();
	UNoteRecorderComponent* NoteRecorderComponent = NewObject<UNoteRecorderComponent>();
	const FName EvidenceId(TEXT("Evidence.Bodycam"));
	const FName NoteId(TEXT("Note.Intro"));

	FHorrorEvidenceMetadata EvidenceMetadata;
	EvidenceMetadata.EvidenceId = EvidenceId;
	EvidenceMetadata.DisplayName = FText::FromString(TEXT("Bodycam"));
	InventoryComponent->RegisterEvidenceMetadata(EvidenceMetadata);
	InventoryComponent->AddCollectedEvidenceId(EvidenceId);
	InventoryComponent->AddCollectedEvidenceId(TEXT("Evidence.Unregistered"));

	FHorrorNoteMetadata NoteMetadata;
	NoteMetadata.NoteId = NoteId;
	NoteMetadata.Title = FText::FromString(TEXT("Intro"));
	NoteRecorderComponent->RegisterNoteMetadata(NoteMetadata);
	NoteRecorderComponent->AddRecordedNoteId(NoteId);
	NoteRecorderComponent->AddRecordedNoteId(TEXT("Note.Unregistered"));

	const FHorrorArchiveSnapshot Snapshot = UHorrorUI::BuildArchiveSnapshot(InventoryComponent, NoteRecorderComponent);
	TestEqual(TEXT("Archive snapshot should preserve evidence ID count."), Snapshot.CollectedEvidenceIds.Num(), 2);
	TestEqual(TEXT("Archive snapshot should preserve evidence total count."), Snapshot.TotalEvidenceCount, 2);
	TestEqual(TEXT("Archive snapshot should expose registered evidence metadata only."), Snapshot.CollectedEvidenceMetadata.Num(), 1);
	if (Snapshot.CollectedEvidenceMetadata.Num() > 0)
	{
		TestEqual(TEXT("Archive snapshot should preserve evidence metadata display name."), Snapshot.CollectedEvidenceMetadata[0].DisplayName.ToString(), FString(TEXT("Bodycam")));
	}
	TestEqual(TEXT("Archive snapshot should preserve note ID count."), Snapshot.RecordedNoteIds.Num(), 2);
	TestEqual(TEXT("Archive snapshot should preserve note total count."), Snapshot.TotalRecordedNotes, 2);
	TestEqual(TEXT("Archive snapshot should expose registered note metadata only."), Snapshot.RecordedNoteMetadata.Num(), 1);
	if (Snapshot.RecordedNoteMetadata.Num() > 0)
	{
		TestEqual(TEXT("Archive snapshot should preserve note metadata title."), Snapshot.RecordedNoteMetadata[0].Title.ToString(), FString(TEXT("Intro")));
	}

	return true;
}

#endif

void UHorrorUI::ReleaseObservedDelegates()
{
	if (ObservedCharacter.IsValid())
	{
		ObservedCharacter->OnSprintMeterUpdated.RemoveDynamic(this, &UHorrorUI::OnSprintMeterUpdated);
		ObservedCharacter->OnSprintStateChanged.RemoveDynamic(this, &UHorrorUI::OnSprintStateChanged);
	}

	if (ObservedInventoryComponent.IsValid())
	{
		ObservedInventoryComponent->OnEvidenceCollected.RemoveDynamic(this, &UHorrorUI::OnEvidenceCollected);
	}

	if (ObservedNoteRecorderComponent.IsValid())
	{
		ObservedNoteRecorderComponent->OnNoteRecorded.RemoveDynamic(this, &UHorrorUI::OnNoteRecorded);
	}

	if (ObservedVHSEffectComponent.IsValid())
	{
		ObservedVHSEffectComponent->OnVHSFeedbackChanged.RemoveDynamic(this, &UHorrorUI::OnVHSFeedbackChanged);
	}

	if (ObservedEventBus.IsValid())
	{
		ObservedEventBus->OnEventPublished.RemoveDynamic(this, &UHorrorUI::OnObjectiveEventPublished);
	}

	ObservedCharacter.Reset();
	ObservedInventoryComponent.Reset();
	ObservedNoteRecorderComponent.Reset();
	ObservedVHSEffectComponent.Reset();
	ObservedEventBus.Reset();
}

void UHorrorUI::SetupCharacter(AHorrorCharacter* HorrorCharacter)
{
	ReleaseObservedDelegates();

	if (!HorrorCharacter)
	{
		RefreshArchiveSnapshot();
		RefreshVHSFeedbackState();
		return;
	}

	ObservedCharacter = HorrorCharacter;
	HorrorCharacter->OnSprintMeterUpdated.AddDynamic(this, &UHorrorUI::OnSprintMeterUpdated);
	HorrorCharacter->OnSprintStateChanged.AddDynamic(this, &UHorrorUI::OnSprintStateChanged);

	if (const AHorrorPlayerCharacter* PlayerCharacter = Cast<AHorrorPlayerCharacter>(HorrorCharacter))
	{
		ObservedInventoryComponent = PlayerCharacter->GetInventoryComponent();
		ObservedNoteRecorderComponent = PlayerCharacter->GetNoteRecorderComponent();
		ObservedVHSEffectComponent = PlayerCharacter->GetVHSEffectComponent();

		if (ObservedInventoryComponent.IsValid())
		{
			ObservedInventoryComponent->OnEvidenceCollected.AddDynamic(this, &UHorrorUI::OnEvidenceCollected);
		}

		if (ObservedNoteRecorderComponent.IsValid())
		{
			ObservedNoteRecorderComponent->OnNoteRecorded.AddDynamic(this, &UHorrorUI::OnNoteRecorded);
		}

		if (ObservedVHSEffectComponent.IsValid())
		{
			ObservedVHSEffectComponent->OnVHSFeedbackChanged.AddDynamic(this, &UHorrorUI::OnVHSFeedbackChanged);
		}
	}

	RefreshArchiveSnapshot();
	RefreshVHSFeedbackState();
	RefreshObjectiveProgressSnapshot();
	BindObjectiveEvents();
}

void UHorrorUI::RestampCheckpointLoadedState()
{
	RefreshArchiveSnapshot();
	RefreshVHSFeedbackState();
	RefreshObjectiveProgressSnapshot();
}

void UHorrorUI::OnSprintMeterUpdated(float Percent)
{
	BP_SprintMeterUpdated(Percent);
}

void UHorrorUI::OnSprintStateChanged(bool bSprinting)
{
	BP_SprintStateChanged(bSprinting);
}

void UHorrorUI::OnEvidenceCollected(FName EvidenceId, int32 TotalEvidenceCount)
{
	BP_EvidenceCollected(EvidenceId, TotalEvidenceCount);
	RefreshArchiveSnapshot();
}

void UHorrorUI::OnNoteRecorded(FName NoteId, int32 TotalRecordedNotes)
{
	BP_NoteRecorded(NoteId, TotalRecordedNotes);
	RefreshArchiveSnapshot();
}

void UHorrorUI::OnVHSFeedbackChanged(bool bIsActive, EQuantumCameraMode CameraMode)
{
	BP_VHSFeedbackChanged(bIsActive, CameraMode);
}

void UHorrorUI::BindObjectiveEvents()
{
	if (ObservedEventBus.IsValid())
	{
		ObservedEventBus->OnEventPublished.RemoveDynamic(this, &UHorrorUI::OnObjectiveEventPublished);
		ObservedEventBus.Reset();
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	UHorrorEventBusSubsystem* EventBus = World->GetSubsystem<UHorrorEventBusSubsystem>();
	if (!EventBus)
	{
		return;
	}

	ObservedEventBus = EventBus;
	EventBus->OnEventPublished.AddDynamic(this, &UHorrorUI::OnObjectiveEventPublished);
}

void UHorrorUI::OnObjectiveEventPublished(const FHorrorEventMessage& Message)
{
	BP_ObjectiveEventPublished(Message);
	RefreshObjectiveProgressSnapshot();
	BP_ObjectiveToastRequested(Message.EventTag, Message.SourceId, Message.StateTag);
	BP_ObjectiveToastRequestedWithMetadata(Message.EventTag, Message.SourceId, Message.StateTag, Message.TrailerBeatId, Message.ObjectiveHint, Message.DebugLabel);
}

void UHorrorUI::RefreshVHSFeedbackState()
{
	if (!ObservedVHSEffectComponent.IsValid())
	{
		BP_VHSFeedbackChanged(false, EQuantumCameraMode::Disabled);
		return;
	}

	BP_VHSFeedbackChanged(
		ObservedVHSEffectComponent->IsFeedbackActive(),
		ObservedVHSEffectComponent->GetFeedbackCameraMode());
}

void UHorrorUI::RefreshArchiveSnapshot()
{
	const FHorrorArchiveSnapshot Snapshot = BuildArchiveSnapshot(ObservedInventoryComponent.Get(), ObservedNoteRecorderComponent.Get());

	BP_ArchiveSnapshotUpdated(
		Snapshot.CollectedEvidenceIds,
		Snapshot.TotalEvidenceCount,
		Snapshot.RecordedNoteIds,
		Snapshot.TotalRecordedNotes);
	BP_ArchiveMetadataSnapshotUpdated(Snapshot);
}

void UHorrorUI::RefreshObjectiveProgressSnapshot()
{
	const UWorld* World = GetWorld();
	const AHorrorGameModeBase* GameMode = World ? World->GetAuthGameMode<AHorrorGameModeBase>() : nullptr;
	BP_ObjectiveProgressSnapshotUpdated(GameMode ? GameMode->BuildFoundFootageProgressSnapshot() : FHorrorFoundFootageProgressSnapshot());
}

void UHorrorUI::NativeDestruct()
{
	ReleaseObservedDelegates();
	Super::NativeDestruct();
}
