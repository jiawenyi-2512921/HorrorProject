#include "StatisticsSubsystem.h"
#include "StatisticsSaveGame.h"
#include "Kismet/GameplayStatics.h"

namespace HorrorStatisticsSave
{
	const FString SlotName(TEXT("HorrorProject_Statistics"));
	constexpr int32 UserIndex = 0;
	constexpr int32 SaveVersion = 1;
	constexpr float SecondsPerHour = 3600.0f;
	constexpr float CentimetersPerMeter = 100.0f;
}

void UStatisticsSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	SessionStartTime = 0.0f;
	TotalPlayTime = 0.0f;
	bSessionActive = false;

	InitializeDefaultStatistics();
	LoadStatistics();

	UE_LOG(LogTemp, Log, TEXT("StatisticsSubsystem initialized"));
}

void UStatisticsSubsystem::Deinitialize()
{
	if (bSessionActive)
	{
		EndSession();
	}

	SaveStatistics();
	Super::Deinitialize();
}

void UStatisticsSubsystem::InitializeDefaultStatistics()
{
	PlayerStats = FPlayerStatistics();

	// Initialize custom statistics
	CustomStatistics.Add(FName("RoomsExplored"), 0.0f);
	CustomStatistics.Add(FName("EvidenceCollected"), 0.0f);
	CustomStatistics.Add(FName("DocumentsRead"), 0.0f);
	CustomStatistics.Add(FName("GhostEncounters"), 0.0f);
	CustomStatistics.Add(FName("Deaths"), 0.0f);
	CustomStatistics.Add(FName("SecretsFound"), 0.0f);
	CustomStatistics.Add(FName("DoorsOpened"), 0.0f);
	CustomStatistics.Add(FName("ItemsUsed"), 0.0f);
	CustomStatistics.Add(FName("DistanceTraveled"), 0.0f);
	CustomStatistics.Add(FName("TimeInDarkness"), 0.0f);
	CustomStatistics.Add(FName("SanityLost"), 0.0f);
	CustomStatistics.Add(FName("JumpScares"), 0.0f);
}

void UStatisticsSubsystem::IncrementStatistic(FName StatName, float Amount)
{
	float CurrentValue = GetStatistic(StatName);
	SetStatistic(StatName, CurrentValue + Amount);
}

void UStatisticsSubsystem::SetStatistic(FName StatName, float Value)
{
	// Update player stats struct
	if (StatName == FName("RoomsExplored"))
	{
		PlayerStats.RoomsExplored = FMath::RoundToInt(Value);
	}
	else if (StatName == FName("EvidenceCollected"))
	{
		PlayerStats.EvidenceCollected = FMath::RoundToInt(Value);
	}
	else if (StatName == FName("DocumentsRead"))
	{
		PlayerStats.DocumentsRead = FMath::RoundToInt(Value);
	}
	else if (StatName == FName("GhostEncounters"))
	{
		PlayerStats.GhostEncounters = FMath::RoundToInt(Value);
	}
	else if (StatName == FName("Deaths"))
	{
		PlayerStats.Deaths = FMath::RoundToInt(Value);
	}
	else if (StatName == FName("SecretsFound"))
	{
		PlayerStats.SecretsFound = FMath::RoundToInt(Value);
	}
	else if (StatName == FName("DistanceTraveled"))
	{
		PlayerStats.DistanceTraveled = Value;
	}
	else if (StatName == FName("TimeInDarkness"))
	{
		PlayerStats.TimeInDarkness = Value;
	}

	// Update custom statistics
	CustomStatistics.Add(StatName, Value);

	// Broadcast event
	OnStatisticUpdated.Broadcast(StatName, Value);
}

float UStatisticsSubsystem::GetStatistic(FName StatName) const
{
	const float* Value = CustomStatistics.Find(StatName);
	return Value ? *Value : 0.0f;
}

FPlayerStatistics UStatisticsSubsystem::GetPlayerStatistics() const
{
	FPlayerStatistics Stats = PlayerStats;
	Stats.TotalPlayTime = TotalPlayTime;
	return Stats;
}

void UStatisticsSubsystem::ResetStatistics()
{
	InitializeDefaultStatistics();
	TotalPlayTime = 0.0f;
	SaveStatistics();

	UE_LOG(LogTemp, Log, TEXT("Statistics reset"));
}

void UStatisticsSubsystem::ResetStatistic(FName StatName)
{
	SetStatistic(StatName, 0.0f);
}

void UStatisticsSubsystem::StartSession()
{
	if (bSessionActive)
	{
		return;
	}

	SessionStartTime = UGameplayStatics::GetTimeSeconds(GetWorld());
	bSessionActive = true;

	UE_LOG(LogTemp, Log, TEXT("Statistics session started"));
}

void UStatisticsSubsystem::EndSession()
{
	if (!bSessionActive)
	{
		return;
	}

	UpdatePlayTime();
	bSessionActive = false;

	SaveStatistics();

	UE_LOG(LogTemp, Log, TEXT("Statistics session ended. Total play time: %.2f seconds"), TotalPlayTime);
}

float UStatisticsSubsystem::GetCurrentSessionDuration() const
{
	if (!bSessionActive)
	{
		return 0.0f;
	}

	float CurrentTime = UGameplayStatics::GetTimeSeconds(GetWorld());
	return CurrentTime - SessionStartTime;
}

float UStatisticsSubsystem::GetTotalPlayTime() const
{
	float PlayTime = TotalPlayTime;

	if (bSessionActive)
	{
		PlayTime += GetCurrentSessionDuration();
	}

	return PlayTime;
}

void UStatisticsSubsystem::UpdatePlayTime()
{
	if (bSessionActive)
	{
		TotalPlayTime += GetCurrentSessionDuration();
		SessionStartTime = UGameplayStatics::GetTimeSeconds(GetWorld());
	}
}

void UStatisticsSubsystem::SaveStatistics()
{
	if (bSessionActive)
	{
		UpdatePlayTime();
	}

	UHorrorStatisticsSaveGame* SaveGame = Cast<UHorrorStatisticsSaveGame>(
		UGameplayStatics::CreateSaveGameObject(UHorrorStatisticsSaveGame::StaticClass()));
	if (!SaveGame)
	{
		UE_LOG(LogTemp, Warning, TEXT("Unable to create statistics save game object"));
		return;
	}

	SaveGame->SaveVersion = HorrorStatisticsSave::SaveVersion;
	SaveGame->SavedAtUtc = FDateTime::UtcNow();
	SaveGame->PlayerStats = PlayerStats;
	SaveGame->PlayerStats.TotalPlayTime = TotalPlayTime;
	SaveGame->CustomStatistics = CustomStatistics;
	SaveGame->TotalPlayTime = TotalPlayTime;

	if (!UGameplayStatics::SaveGameToSlot(SaveGame, HorrorStatisticsSave::SlotName, HorrorStatisticsSave::UserIndex))
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to save statistics to slot '%s'"), *HorrorStatisticsSave::SlotName);
	}
}

void UStatisticsSubsystem::LoadStatistics()
{
	if (!UGameplayStatics::DoesSaveGameExist(HorrorStatisticsSave::SlotName, HorrorStatisticsSave::UserIndex))
	{
		UE_LOG(LogTemp, Verbose, TEXT("No statistics save exists in slot '%s'"), *HorrorStatisticsSave::SlotName);
		return;
	}

	UHorrorStatisticsSaveGame* SaveGame = Cast<UHorrorStatisticsSaveGame>(
		UGameplayStatics::LoadGameFromSlot(HorrorStatisticsSave::SlotName, HorrorStatisticsSave::UserIndex));
	if (!SaveGame)
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to load statistics from slot '%s'"), *HorrorStatisticsSave::SlotName);
		return;
	}

	if (SaveGame->SaveVersion != HorrorStatisticsSave::SaveVersion)
	{
		UE_LOG(LogTemp, Warning, TEXT("Ignoring unsupported statistics save version %d"), SaveGame->SaveVersion);
		return;
	}

	PlayerStats = SaveGame->PlayerStats;
	TotalPlayTime = FMath::Max(0.0f, SaveGame->TotalPlayTime);
	PlayerStats.TotalPlayTime = TotalPlayTime;

	TMap<FName, float> Defaults = CustomStatistics;
	CustomStatistics = SaveGame->CustomStatistics;
	for (const TPair<FName, float>& Pair : Defaults)
	{
		if (!CustomStatistics.Contains(Pair.Key))
		{
			CustomStatistics.Add(Pair.Key, Pair.Value);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("Loaded %d custom statistics"), CustomStatistics.Num());
}

FString UStatisticsSubsystem::GenerateStatisticsReport() const
{
	FString Report;

	Report += TEXT("=== PLAYER STATISTICS REPORT ===\n\n");

	// Play Time
	const float Hours = TotalPlayTime / HorrorStatisticsSave::SecondsPerHour;
	Report += FString::Printf(TEXT("Total Play Time: %.2f hours\n"), Hours);

	// Exploration
	Report += TEXT("\n--- EXPLORATION ---\n");
	Report += FString::Printf(TEXT("Rooms Explored: %d\n"), PlayerStats.RoomsExplored);
	Report += FString::Printf(TEXT("Distance Traveled: %.2f meters\n"), PlayerStats.DistanceTraveled / HorrorStatisticsSave::CentimetersPerMeter);
	Report += FString::Printf(TEXT("Secrets Found: %d\n"), PlayerStats.SecretsFound);

	// Collection
	Report += TEXT("\n--- COLLECTION ---\n");
	Report += FString::Printf(TEXT("Evidence Collected: %d\n"), PlayerStats.EvidenceCollected);
	Report += FString::Printf(TEXT("Documents Read: %d\n"), PlayerStats.DocumentsRead);

	// Survival
	Report += TEXT("\n--- SURVIVAL ---\n");
	Report += FString::Printf(TEXT("Ghost Encounters: %d\n"), PlayerStats.GhostEncounters);
	Report += FString::Printf(TEXT("Deaths: %d\n"), PlayerStats.Deaths);
	Report += FString::Printf(TEXT("Time in Darkness: %.2f seconds\n"), PlayerStats.TimeInDarkness);

	// Custom Statistics
	Report += TEXT("\n--- OTHER STATISTICS ---\n");
	Report += FString::Printf(TEXT("Doors Opened: %.0f\n"), GetStatistic(FName("DoorsOpened")));
	Report += FString::Printf(TEXT("Items Used: %.0f\n"), GetStatistic(FName("ItemsUsed")));
	Report += FString::Printf(TEXT("Sanity Lost: %.0f\n"), GetStatistic(FName("SanityLost")));
	Report += FString::Printf(TEXT("Jump Scares: %.0f\n"), GetStatistic(FName("JumpScares")));

	Report += TEXT("\n================================\n");

	return Report;
}
