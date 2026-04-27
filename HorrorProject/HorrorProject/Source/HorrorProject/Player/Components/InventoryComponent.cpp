// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/Components/InventoryComponent.h"

namespace
{
	constexpr int32 ExpectedEvidenceCollectionCapacity = 32;
}

void UInventoryEvidenceDelegateProbe::HandleEvidenceCollected(FName EvidenceId, int32 TotalEvidenceCount)
{
	++BroadcastCount;
	LastEvidenceId = EvidenceId;
	LastTotalEvidenceCount = TotalEvidenceCount;
	EvidenceIds.Add(EvidenceId);
	TotalEvidenceCounts.Add(TotalEvidenceCount);
}

UInventoryComponent::UInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UInventoryComponent::InitializeComponent()
{
	Super::InitializeComponent();

	// Memory optimization: Pre-allocate capacity for typical evidence collection
	CollectedEvidenceIds.Reserve(ExpectedEvidenceCollectionCapacity);
	EvidenceMetadataById.Reserve(ExpectedEvidenceCollectionCapacity);
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
	CollectedMetadata.Reserve(CollectedEvidenceIds.Num());
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
