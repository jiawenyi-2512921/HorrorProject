// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/Components/InventoryComponent.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"

void UInventoryEvidenceDelegateProbe::HandleEvidenceCollected(FName EvidenceId, int32 TotalEvidenceCount)
{
	++BroadcastCount;
	LastEvidenceId = EvidenceId;
	LastTotalEvidenceCount = TotalEvidenceCount;
	EvidenceIds.Add(EvidenceId);
	TotalEvidenceCounts.Add(TotalEvidenceCount);
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FInventoryComponentStoresUniqueEvidenceIdsTest,
	"HorrorProject.Player.Inventory.StoresUniqueEvidenceIds",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FInventoryComponentStoresUniqueEvidenceIdsTest::RunTest(const FString& Parameters)
{
	UInventoryComponent* InventoryComponent = NewObject<UInventoryComponent>();
	UInventoryEvidenceDelegateProbe* DelegateProbe = NewObject<UInventoryEvidenceDelegateProbe>();
	InventoryComponent->OnEvidenceCollected.AddDynamic(DelegateProbe, &UInventoryEvidenceDelegateProbe::HandleEvidenceCollected);
	const FName FirstEvidenceId(TEXT("Evidence.Bodycam"));
	const FName SecondEvidenceId(TEXT("Evidence.Note01"));

	TestFalse(TEXT("Empty evidence IDs should be rejected."), InventoryComponent->AddCollectedEvidenceId(NAME_None));
	TestTrue(TEXT("First evidence ID should be accepted."), InventoryComponent->AddCollectedEvidenceId(FirstEvidenceId));
	TestFalse(TEXT("Duplicate evidence IDs should be ignored."), InventoryComponent->AddCollectedEvidenceId(FirstEvidenceId));
	TestTrue(TEXT("Second evidence ID should be accepted."), InventoryComponent->AddCollectedEvidenceId(SecondEvidenceId));
	TestEqual(TEXT("Runtime evidence collection should broadcast unique additions only."), DelegateProbe->BroadcastCount, 2);
	TestEqual(TEXT("Evidence delegate should publish last collected id."), DelegateProbe->LastEvidenceId, SecondEvidenceId);
	TestEqual(TEXT("Evidence delegate should publish last total count."), DelegateProbe->LastTotalEvidenceCount, 2);
	TestEqual(TEXT("Evidence delegate should preserve id payload order."), DelegateProbe->EvidenceIds, TArray<FName>({ FirstEvidenceId, SecondEvidenceId }));
	TestEqual(TEXT("Evidence delegate should preserve total payload order."), DelegateProbe->TotalEvidenceCounts, TArray<int32>({ 1, 2 }));
	TestTrue(TEXT("Known evidence IDs should be queryable."), InventoryComponent->HasCollectedEvidenceId(FirstEvidenceId));
	TestFalse(TEXT("Unknown evidence IDs should not be queryable."), InventoryComponent->HasCollectedEvidenceId(TEXT("Evidence.Unknown")));
	TestEqual(TEXT("Stored evidence count should match unique IDs."), InventoryComponent->GetCollectedEvidenceCount(), 2);

	const TArray<FName>& EvidenceIds = InventoryComponent->GetCollectedEvidenceIds();
	TestEqual(TEXT("Collected evidence IDs should preserve acquisition order."), EvidenceIds.Num(), 2);
	TestEqual(TEXT("First collected evidence should remain first."), EvidenceIds[0], FirstEvidenceId);
	TestEqual(TEXT("Second collected evidence should remain second."), EvidenceIds[1], SecondEvidenceId);

	FHorrorEvidenceMetadata BodycamMetadata;
	BodycamMetadata.EvidenceId = FirstEvidenceId;
	BodycamMetadata.DisplayName = FText::FromString(TEXT("Bodycam"));
	BodycamMetadata.Description = FText::FromString(TEXT("Recovered recording rig."));
	InventoryComponent->RegisterEvidenceMetadata(BodycamMetadata);

	FHorrorEvidenceMetadata InvalidMetadata;
	InvalidMetadata.EvidenceId = NAME_None;
	InvalidMetadata.DisplayName = FText::FromString(TEXT("Invalid"));
	InventoryComponent->RegisterEvidenceMetadata(InvalidMetadata);

	FHorrorEvidenceMetadata ResolvedMetadata;
	TestFalse(TEXT("NAME_None evidence metadata should be ignored."), InventoryComponent->GetEvidenceMetadata(NAME_None, ResolvedMetadata));
	TestTrue(TEXT("Registered evidence metadata should be queryable."), InventoryComponent->GetEvidenceMetadata(FirstEvidenceId, ResolvedMetadata));
	TestEqual(TEXT("Evidence metadata should preserve display name."), ResolvedMetadata.DisplayName.ToString(), FString(TEXT("Bodycam")));

	FHorrorEvidenceMetadata UpdatedBodycamMetadata;
	UpdatedBodycamMetadata.EvidenceId = FirstEvidenceId;
	UpdatedBodycamMetadata.DisplayName = FText::FromString(TEXT("Recovered Bodycam"));
	UpdatedBodycamMetadata.Description = FText::FromString(TEXT("Updated recording rig."));
	InventoryComponent->RegisterEvidenceMetadata(UpdatedBodycamMetadata);
	TestTrue(TEXT("Updated evidence metadata should remain queryable."), InventoryComponent->GetEvidenceMetadata(FirstEvidenceId, ResolvedMetadata));
	TestEqual(TEXT("Evidence metadata should overwrite same-id entries deterministically."), ResolvedMetadata.DisplayName.ToString(), FString(TEXT("Recovered Bodycam")));

	FHorrorEvidenceMetadata NoteMetadata;
	NoteMetadata.EvidenceId = SecondEvidenceId;
	NoteMetadata.DisplayName = FText::FromString(TEXT("Archive Note"));
	InventoryComponent->RegisterEvidenceMetadata(NoteMetadata);

	const TArray<FHorrorEvidenceMetadata> CollectedMetadata = InventoryComponent->GetCollectedEvidenceMetadata();
	TestEqual(TEXT("Collected evidence metadata should include registered collected evidence only."), CollectedMetadata.Num(), 2);
	if (CollectedMetadata.Num() == 2)
	{
		TestEqual(TEXT("Collected evidence metadata should preserve collected order for first id."), CollectedMetadata[0].EvidenceId, FirstEvidenceId);
		TestEqual(TEXT("Collected evidence metadata should preserve collected order for second id."), CollectedMetadata[1].EvidenceId, SecondEvidenceId);
		TestEqual(TEXT("Collected evidence metadata should include overwritten first metadata."), CollectedMetadata[0].DisplayName.ToString(), FString(TEXT("Recovered Bodycam")));
	}

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FInventoryComponentImportDoesNotBroadcastTest,
	"HorrorProject.Player.Inventory.ImportDoesNotBroadcast",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FInventoryComponentImportDoesNotBroadcastTest::RunTest(const FString& Parameters)
{
	UInventoryComponent* InventoryComponent = NewObject<UInventoryComponent>();
	UInventoryEvidenceDelegateProbe* DelegateProbe = NewObject<UInventoryEvidenceDelegateProbe>();
	InventoryComponent->OnEvidenceCollected.AddDynamic(DelegateProbe, &UInventoryEvidenceDelegateProbe::HandleEvidenceCollected);
	TestNotNull(TEXT("Inventory import test should create an inventory component."), InventoryComponent);
	if (!InventoryComponent)
	{
		return false;
	}

	InventoryComponent->ImportCollectedEvidenceIds({ NAME_None, TEXT("Evidence.Bodycam"), TEXT("Evidence.Bodycam"), TEXT("Evidence.Anomaly01") });

	TestEqual(TEXT("Evidence import should sanitize and de-dupe ids."), InventoryComponent->ExportCollectedEvidenceIds().Num(), 2);
	TestTrue(TEXT("Evidence import should restore the bodycam id."), InventoryComponent->HasCollectedEvidenceId(TEXT("Evidence.Bodycam")));
	TestTrue(TEXT("Evidence import should restore the anomaly id."), InventoryComponent->HasCollectedEvidenceId(TEXT("Evidence.Anomaly01")));
	TestEqual(TEXT("Evidence import should not broadcast collection delegates."), DelegateProbe->BroadcastCount, 0);

	TestTrue(TEXT("Runtime evidence collection should still broadcast after import."), InventoryComponent->AddCollectedEvidenceId(TEXT("Evidence.Archive")));
	TestEqual(TEXT("Runtime evidence collection should broadcast once."), DelegateProbe->BroadcastCount, 1);
	TestEqual(TEXT("Runtime evidence collection should broadcast restored runtime id."), DelegateProbe->LastEvidenceId, FName(TEXT("Evidence.Archive")));
	TestEqual(TEXT("Runtime evidence collection should broadcast total after imported ids."), DelegateProbe->LastTotalEvidenceCount, 3);

	return true;
}

#endif

UInventoryComponent::UInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

bool UInventoryComponent::AddCollectedEvidenceId(FName EvidenceId)
{
	if (EvidenceId.IsNone() || CollectedEvidenceIds.Contains(EvidenceId))
	{
		return false;
	}

	CollectedEvidenceIds.Add(EvidenceId);
	OnEvidenceCollected.Broadcast(EvidenceId, CollectedEvidenceIds.Num());
	return true;
}

bool UInventoryComponent::HasCollectedEvidenceId(FName EvidenceId) const
{
	return !EvidenceId.IsNone() && CollectedEvidenceIds.Contains(EvidenceId);
}

const TArray<FName>& UInventoryComponent::GetCollectedEvidenceIds() const
{
	return CollectedEvidenceIds;
}

const TArray<FName>& UInventoryComponent::ExportCollectedEvidenceIds() const
{
	return CollectedEvidenceIds;
}

void UInventoryComponent::ImportCollectedEvidenceIds(const TArray<FName>& EvidenceIds)
{
	CollectedEvidenceIds.Reset();

	for (const FName EvidenceId : EvidenceIds)
	{
		if (EvidenceId.IsNone() || CollectedEvidenceIds.Contains(EvidenceId))
		{
			continue;
		}

		CollectedEvidenceIds.Add(EvidenceId);
	}
}

void UInventoryComponent::RegisterEvidenceMetadata(const FHorrorEvidenceMetadata& Metadata)
{
	if (Metadata.EvidenceId.IsNone())
	{
		return;
	}

	EvidenceMetadataById.Add(Metadata.EvidenceId, Metadata);
}

bool UInventoryComponent::GetEvidenceMetadata(FName EvidenceId, FHorrorEvidenceMetadata& OutMetadata) const
{
	if (const FHorrorEvidenceMetadata* Metadata = EvidenceMetadataById.Find(EvidenceId))
	{
		OutMetadata = *Metadata;
		return true;
	}

	return false;
}

TArray<FHorrorEvidenceMetadata> UInventoryComponent::GetCollectedEvidenceMetadata() const
{
	TArray<FHorrorEvidenceMetadata> CollectedMetadata;
	for (const FName EvidenceId : CollectedEvidenceIds)
	{
		if (const FHorrorEvidenceMetadata* Metadata = EvidenceMetadataById.Find(EvidenceId))
		{
			CollectedMetadata.Add(*Metadata);
		}
	}
	return CollectedMetadata;
}

int32 UInventoryComponent::GetCollectedEvidenceCount() const
{
	return CollectedEvidenceIds.Num();
}
