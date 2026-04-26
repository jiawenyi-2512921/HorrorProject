// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Subsystems/WorldSubsystem.h"
#include "HorrorEventBusSubsystem.generated.h"

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
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FHorrorEventPublishedDynamicDelegate, const FHorrorEventMessage&, Message);
DECLARE_MULTICAST_DELEGATE_OneParam(FHorrorEventPublishedNativeDelegate, const FHorrorEventMessage&);

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
	UPROPERTY(EditDefaultsOnly, Category="Horror|Events", meta=(ClampMin="1"))
	int32 HistoryCapacity = 128;

	UPROPERTY(Transient)
	TArray<FHorrorEventMessage> History;

	UPROPERTY(Transient)
	TMap<FName, FHorrorObjectiveMessageMetadata> ObjectiveMetadataBySourceId;

	UPROPERTY(Transient)
	TMap<FName, FHorrorObjectiveMessageMetadata> ObjectiveMetadataByEventAndSourceId;

	FHorrorEventPublishedNativeDelegate OnEventPublishedNative;
};
