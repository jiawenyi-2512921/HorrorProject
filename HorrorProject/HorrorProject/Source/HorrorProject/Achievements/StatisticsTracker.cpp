#include "StatisticsTracker.h"
#include "StatisticsSubsystem.h"
#include "GameplayMetrics.h"
#include "Engine/GameInstance.h"
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

	UWorld* World = GetWorld();
	if (World)
	{
		if (UGameInstance* GameInstance = World->GetGameInstance())
		{
			StatisticsSubsystem = GameInstance->GetSubsystem<UStatisticsSubsystem>();
		}
	}

	if (StatisticsSubsystem)
	{
		StatisticsSubsystem->StartSession();
	}

	CurrentMetrics = UHorrorGameplayMetricsLibrary::CreateMetrics();
	if (AActor* Owner = GetOwner())
	{
		LastPosition = Owner->GetActorLocation();
	}
	LastPositionUpdateTime = World ? UGameplayStatics::GetTimeSeconds(World) : 0.0f;
}

void UStatisticsTracker::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	AActor* Owner = GetOwner();
	UWorld* World = GetWorld();
	if (!Owner || !World)
	{
		return;
	}

	// Track distance traveled
	FVector CurrentPosition = Owner->GetActorLocation();
	float Distance = FVector::Dist(LastPosition, CurrentPosition);

	if (Distance > 1.0f) // Minimum threshold
	{
		TrackMovement(Distance);
		LastPosition = CurrentPosition;
	}

	// Update session duration
	CurrentMetrics.SessionDuration = UGameplayStatics::GetTimeSeconds(World) - LastPositionUpdateTime;
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

	UHorrorGameplayMetricsLibrary::UpdateSanityMetrics(CurrentMetrics, NewSanity);
}

void UStatisticsTracker::TrackJumpScare(float Intensity)
{
	if (StatisticsSubsystem)
	{
		StatisticsSubsystem->IncrementStatistic(FName("JumpScares"));
	}

	UHorrorGameplayMetricsLibrary::RecordJumpScare(CurrentMetrics, Intensity);
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
	UHorrorGameplayMetricsLibrary::UpdateFPSMetrics(CurrentMetrics, CurrentFPS);
}

void UStatisticsTracker::TrackInteraction(bool bSuccess)
{
	UHorrorGameplayMetricsLibrary::RecordInteraction(CurrentMetrics, bSuccess);
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
	Report += UHorrorGameplayMetricsLibrary::GenerateMetricsReport(CurrentMetrics);

	return Report;
}
