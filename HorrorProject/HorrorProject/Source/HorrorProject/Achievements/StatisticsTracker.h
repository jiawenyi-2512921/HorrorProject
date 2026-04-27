#pragma once

#include "CoreMinimal.h"
#include "Achievements/GameplayMetrics.h"
#include "Components/ActorComponent.h"
#include "StatisticsTracker.generated.h"

/**
 * Defines Statistics Tracker behavior for the Achievements module.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class HORRORPROJECT_API UStatisticsTracker : public UActorComponent
{
	GENERATED_BODY()

public:
	UStatisticsTracker();

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// Tracking Methods
	UFUNCTION(BlueprintCallable, Category = "Statistics Tracker")
	void TrackMovement(float Distance);

	UFUNCTION(BlueprintCallable, Category = "Statistics Tracker")
	void TrackDoorOpened();

	UFUNCTION(BlueprintCallable, Category = "Statistics Tracker")
	void TrackItemUsed(FName ItemID);

	UFUNCTION(BlueprintCallable, Category = "Statistics Tracker")
	void TrackSanityChange(float OldSanity, float NewSanity);

	UFUNCTION(BlueprintCallable, Category = "Statistics Tracker")
	void TrackJumpScare(float Intensity);

	UFUNCTION(BlueprintCallable, Category = "Statistics Tracker")
	void TrackSaveGame();

	UFUNCTION(BlueprintCallable, Category = "Statistics Tracker")
	void TrackLoadGame();

	UFUNCTION(BlueprintCallable, Category = "Statistics Tracker")
	void TrackFPS(float CurrentFPS);

	UFUNCTION(BlueprintCallable, Category = "Statistics Tracker")
	void TrackInteraction(bool bSuccess);

	UFUNCTION(BlueprintCallable, Category = "Statistics Tracker")
	void UpdateDarknessTime(float DeltaTime, bool bInDarkness);

	UFUNCTION(BlueprintCallable, Category = "Statistics Tracker")
	FString GenerateSessionReport() const;

private:
	UPROPERTY()
	class UStatisticsSubsystem* StatisticsSubsystem;

	UPROPERTY()
	struct FHorrorGameplayMetrics CurrentMetrics;

	float LastPositionUpdateTime;
	FVector LastPosition;
};
