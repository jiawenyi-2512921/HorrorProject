#include "StatisticsTracker.h"
#include "StatisticsSubsystem.h"
#include "GameplayMetrics.h"
#include "Kismet/GameplayStatics.h"

UStatisticsTracker::UStatisticsTracker()
{
	PrimaryComponentTick.bCanEverTick = true;
	LastPositionUpdateTime = 0.0f;
	LastPosition = FVector::ZeroVector;
}

void UStatisticsTracker::BeginPlay()
{
	Super::BeginPlay();

	StatisticsSubsystem = GetWorld()->GetGameInstance()->GetSubsystem<UStatisticsSubsystem>();

	if (StatisticsSubsystem)
	{
		StatisticsSubsystem->StartSession();
	}

	CurrentMetrics = UGameplayMetrics::CreateMetrics();
	LastPosition = GetOwner()->GetActorLocation();
	LastPositionUpdateTime = UGameplayStatics::GetTimeSeconds(GetWorld());
}

void UStatisticsTracker::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Track distance traveled
	FVector CurrentPosition = GetOwner()->GetActorLocation();
	float Distance = FVector::Dist(LastPosition, CurrentPosition);

	if (Distance > 1.0f) // Minimum threshold
	{
		TrackMovement(Distance);
		LastPosition = CurrentPosition;
	}

	// Update session duration
	CurrentMetrics.SessionDuration = UGameplayStatics::GetTimeSeconds(GetWorld()) - LastPositionUpdateTime;
}

void UStatisticsTracker::TrackMovement(float Distance)
{
	if (StatisticsSubsystem)
	{
		StatisticsSubsystem->IncrementStatistic(FName("DistanceTraveled"), Distance);
	}
}

void UStatisticsTracker::TrackDoorOpened()
{
	if (StatisticsSubsystem)
	{
		StatisticsSubsystem->IncrementStatistic(FName("DoorsOpened"));
	}
}

void UStatisticsTracker::TrackItemUsed(FName ItemID)
{
	if (StatisticsSubsystem)
	{
		StatisticsSubsystem->IncrementStatistic(FName("ItemsUsed"));
	}
}

void UStatisticsTracker::TrackSanityChange(float OldSanity, float NewSanity)
{
	if (StatisticsSubsystem)
	{
		float SanityLost = FMath::Max(0.0f, OldSanity - NewSanity);
		StatisticsSubsystem->IncrementStatistic(FName("SanityLost"), SanityLost);
	}

	UGameplayMetrics::UpdateSanityMetrics(CurrentMetrics, NewSanity);
}

void UStatisticsTracker::TrackJumpScare(float Intensity)
{
	if (StatisticsSubsystem)
	{
		StatisticsSubsystem->IncrementStatistic(FName("JumpScares"));
	}

	UGameplayMetrics::RecordJumpScare(CurrentMetrics, Intensity);
}

void UStatisticsTracker::TrackSaveGame()
{
	CurrentMetrics.SaveCount++;
}

void UStatisticsTracker::TrackLoadGame()
{
	CurrentMetrics.LoadCount++;
}

void UStatisticsTracker::TrackFPS(float CurrentFPS)
{
	UGameplayMetrics::UpdateFPSMetrics(CurrentMetrics, CurrentFPS);
}

void UStatisticsTracker::TrackInteraction(bool bSuccess)
{
	UGameplayMetrics::RecordInteraction(CurrentMetrics, bSuccess);
}

void UStatisticsTracker::UpdateDarknessTime(float DeltaTime, bool bInDarkness)
{
	if (bInDarkness && StatisticsSubsystem)
	{
		StatisticsSubsystem->IncrementStatistic(FName("TimeInDarkness"), DeltaTime);
	}
}

FString UStatisticsTracker::GenerateSessionReport() const
{
	FString Report;

	Report += TEXT("=== SESSION REPORT ===\n\n");

	// Statistics Report
	if (StatisticsSubsystem)
	{
		Report += StatisticsSubsystem->GenerateStatisticsReport();
	}

	Report += TEXT("\n");

	// Metrics Report
	Report += UGameplayMetrics::GenerateMetricsReport(CurrentMetrics);

	return Report;
}
