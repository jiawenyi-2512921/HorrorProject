// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Game/HorrorAdvancedInteractionTypes.h"
#include "GameplayTagContainer.h"
#include "Subsystems/WorldSubsystem.h"
#include "HorrorEventBusSubsystem.generated.h"

struct FHorrorEventSourceMetadataKey
{
	FHorrorEventSourceMetadataKey() = default;
	FHorrorEventSourceMetadataKey(FGameplayTag InEventTag, FName InSourceId)
		: EventTag(InEventTag)
		, SourceId(InSourceId)
	{
	}

	FGameplayTag EventTag;
	FName SourceId = NAME_None;

	friend bool operator==(const FHorrorEventSourceMetadataKey& Left, const FHorrorEventSourceMetadataKey& Right)
	{
		return Left.EventTag == Right.EventTag && Left.SourceId == Right.SourceId;
	}
};

FORCEINLINE uint32 GetTypeHash(const FHorrorEventSourceMetadataKey& Key)
{
	return HashCombine(GetTypeHash(Key.EventTag), GetTypeHash(Key.SourceId));
}

UENUM(BlueprintType)
enum class EHorrorObjectiveFeedbackSeverity : uint8
{
	Info,
	Success,
	Warning,
	Failure,
	Critical
};

USTRUCT(BlueprintType)
struct HORRORPROJECT_API FHorrorObjectiveMessageMetadata
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category="Horror|Events")
	FName TrailerBeatId = NAME_None;

	UPROPERTY(BlueprintReadOnly, Category="Horror|Events")
	FText ObjectiveHint;

	UPROPERTY(BlueprintReadOnly, Category="Horror|Events")
	FText DebugLabel;

	UPROPERTY(BlueprintReadOnly, Category="Horror|Events")
	EHorrorObjectiveFeedbackSeverity FeedbackSeverity = EHorrorObjectiveFeedbackSeverity::Info;

	UPROPERTY(BlueprintReadOnly, Category="Horror|Events")
	bool bRetryable = false;

	UPROPERTY(BlueprintReadOnly, Category="Horror|Events")
	float DisplaySeconds = 5.0f;

	UPROPERTY(BlueprintReadOnly, Category="Horror|Events")
	FName FailureCause = NAME_None;

	UPROPERTY(BlueprintReadOnly, Category="Horror|Events")
	FName RecoveryAction = NAME_None;

	UPROPERTY(BlueprintReadOnly, Category="Horror|Events")
	EHorrorAdvancedInteractionOutcomeKind AdvancedOutcomeKind = EHorrorAdvancedInteractionOutcomeKind::Ignored;

	UPROPERTY(BlueprintReadOnly, Category="Horror|Events")
	FName AdvancedFaultId = NAME_None;

	UPROPERTY(BlueprintReadOnly, Category="Horror|Events")
	int32 AttemptIndex = 0;
};

USTRUCT(BlueprintType)
struct HORRORPROJECT_API FHorrorEventMessage
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category="Horror|Events")
	FGameplayTag EventTag;

	UPROPERTY(BlueprintReadOnly, Category="Horror|Events")
	FGameplayTag StateTag;

	UPROPERTY(BlueprintReadOnly, Category="Horror|Events")
	FName SourceId;

	UPROPERTY(BlueprintReadOnly, Category="Horror|Events")
	TObjectPtr<UObject> SourceObject = nullptr;

	UPROPERTY(BlueprintReadOnly, Category="Horror|Events")
	float WorldSeconds = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category="Horror|Events")
	FName TrailerBeatId = NAME_None;

	UPROPERTY(BlueprintReadOnly, Category="Horror|Events")
	FText ObjectiveHint;

	UPROPERTY(BlueprintReadOnly, Category="Horror|Events")
	FText DebugLabel;

	UPROPERTY(BlueprintReadOnly, Category="Horror|Events")
	EHorrorObjectiveFeedbackSeverity FeedbackSeverity = EHorrorObjectiveFeedbackSeverity::Info;

	UPROPERTY(BlueprintReadOnly, Category="Horror|Events")
	bool bRetryable = false;

	UPROPERTY(BlueprintReadOnly, Category="Horror|Events")
	float DisplaySeconds = 5.0f;

	UPROPERTY(BlueprintReadOnly, Category="Horror|Events")
	FName FailureCause = NAME_None;

	UPROPERTY(BlueprintReadOnly, Category="Horror|Events")
	FName RecoveryAction = NAME_None;

	UPROPERTY(BlueprintReadOnly, Category="Horror|Events")
	EHorrorAdvancedInteractionOutcomeKind AdvancedOutcomeKind = EHorrorAdvancedInteractionOutcomeKind::Ignored;

	UPROPERTY(BlueprintReadOnly, Category="Horror|Events")
	FName AdvancedFaultId = NAME_None;

	UPROPERTY(BlueprintReadOnly, Category="Horror|Events")
	int32 AttemptIndex = 0;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FHorrorEventPublishedDynamicDelegate, const FHorrorEventMessage&, Message);
DECLARE_MULTICAST_DELEGATE_OneParam(FHorrorEventPublishedNativeDelegate, const FHorrorEventMessage&);

/**
 * Coordinates Horror Event Bus Subsystem services for the Game module.
 */
UCLASS()
class HORRORPROJECT_API UHorrorEventBusSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, Category="Horror|Events")
	FHorrorEventPublishedDynamicDelegate OnEventPublished;

	UFUNCTION(BlueprintCallable, Category="Horror|Events")
	bool Publish(FGameplayTag EventTag, FName SourceId, FGameplayTag StateTag, UObject* SourceObject);

	FHorrorEventPublishedNativeDelegate& GetOnEventPublishedNative();
	const TArray<FHorrorEventMessage>& GetHistory() const;
	void RegisterObjectiveMetadata(FName SourceId, const FHorrorObjectiveMessageMetadata& Metadata);
	void RegisterObjectiveMetadata(FGameplayTag EventTag, FName SourceId, const FHorrorObjectiveMessageMetadata& Metadata);
	void UnregisterObjectiveMetadata(FName SourceId);
	void UnregisterObjectiveMetadata(FGameplayTag EventTag, FName SourceId);
	bool GetObjectiveMetadataForTests(FName SourceId, FHorrorObjectiveMessageMetadata& OutMetadata) const;
	bool GetObjectiveMetadataForTests(FGameplayTag EventTag, FName SourceId, FHorrorObjectiveMessageMetadata& OutMetadata) const;
	void ResetForTests();
#if WITH_DEV_AUTOMATION_TESTS
	void SetHistoryCapacityForTests(int32 NewHistoryCapacity);
#endif

private:
	static constexpr int32 DefaultHistoryCapacity = 128;

	UPROPERTY(EditDefaultsOnly, Category="Horror|Events", meta=(ClampMin="1"))
	int32 HistoryCapacity = DefaultHistoryCapacity;

	UPROPERTY(Transient)
	TArray<FHorrorEventMessage> HistoryRing;

	mutable TArray<FHorrorEventMessage> OrderedHistoryView;
	mutable bool bOrderedHistoryViewDirty = true;
	int32 HistoryStartIndex = 0;
	int32 HistoryCount = 0;

	UPROPERTY(Transient)
	TMap<FName, FHorrorObjectiveMessageMetadata> ObjectiveMetadataBySourceId;

	TMap<FHorrorEventSourceMetadataKey, FHorrorObjectiveMessageMetadata> ObjectiveMetadataByEventAndSourceId;

	FHorrorEventPublishedNativeDelegate OnEventPublishedNative;

	void AppendHistoryMessage(FHorrorEventMessage&& Message);
	void EnsureHistoryRingCapacity();
	void RebuildOrderedHistoryView() const;

};
