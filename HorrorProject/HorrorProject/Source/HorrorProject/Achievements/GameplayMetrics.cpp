#include "GameplayMetrics.h"

FGameplayMetrics UGameplayMetrics::CreateMetrics()
{
	FGameplayMetrics Metrics;
	Metrics.SessionStartTime = FDateTime::Now();
	return Metrics;
}

void UGameplayMetrics::UpdateFPSMetrics(FGameplayMetrics& Metrics, float CurrentFPS)
{
	if (Metrics.MinFPS == 0.0f || CurrentFPS < Metrics.MinFPS)
	{
		Metrics.MinFPS = CurrentFPS;
	}

	if (CurrentFPS > Metrics.MaxFPS)
	{
		Metrics.MaxFPS = CurrentFPS;
	}

	// Update average (simple moving average)
	if (Metrics.AverageFPS == 0.0f)
	{
		Metrics.AverageFPS = CurrentFPS;
	}
	else
	{
		Metrics.AverageFPS = (Metrics.AverageFPS * 0.95f) + (CurrentFPS * 0.05f);
	}
}

void UGameplayMetrics::UpdateSanityMetrics(FGameplayMetrics& Metrics, float CurrentSanity)
{
	if (CurrentSanity < Metrics.MinSanity)
	{
		Metrics.MinSanity = CurrentSanity;
	}

	// Update average
	if (Metrics.AverageSanity == 100.0f)
	{
		Metrics.AverageSanity = CurrentSanity;
	}
	else
	{
		Metrics.AverageSanity = (Metrics.AverageSanity * 0.99f) + (CurrentSanity * 0.01f);
	}
}

void UGameplayMetrics::RecordJumpScare(FGameplayMetrics& Metrics, float Intensity)
{
	Metrics.JumpScareCount++;
	Metrics.TotalScareIntensity += Intensity;

	// Simulate heart rate increase
	Metrics.AverageHeartRate += Intensity * 10.0f;
}

void UGameplayMetrics::RecordInteraction(FGameplayMetrics& Metrics, bool bSuccess)
{
	Metrics.TotalInteractions++;

	if (!bSuccess)
	{
		Metrics.FailedInteractions++;
	}
}

FString UGameplayMetrics::GenerateMetricsReport(const FGameplayMetrics& Metrics)
{
	FString Report;

	Report += TEXT("=== GAMEPLAY METRICS REPORT ===\n\n");

	// Performance
	Report += TEXT("--- PERFORMANCE ---\n");
	Report += FString::Printf(TEXT("Average FPS: %.2f\n"), Metrics.AverageFPS);
	Report += FString::Printf(TEXT("Min FPS: %.2f\n"), Metrics.MinFPS);
	Report += FString::Printf(TEXT("Max FPS: %.2f\n"), Metrics.MaxFPS);

	// Gameplay
	Report += TEXT("\n--- GAMEPLAY ---\n");
	Report += FString::Printf(TEXT("Average Sanity: %.2f%%\n"), Metrics.AverageSanity);
	Report += FString::Printf(TEXT("Min Sanity: %.2f%%\n"), Metrics.MinSanity);
	Report += FString::Printf(TEXT("Total Interactions: %d\n"), Metrics.TotalInteractions);
	Report += FString::Printf(TEXT("Failed Interactions: %d\n"), Metrics.FailedInteractions);

	float SuccessRate = Metrics.TotalInteractions > 0
		? (float)(Metrics.TotalInteractions - Metrics.FailedInteractions) / Metrics.TotalInteractions * 100.0f
		: 0.0f;
	Report += FString::Printf(TEXT("Interaction Success Rate: %.2f%%\n"), SuccessRate);

	// Horror
	Report += TEXT("\n--- HORROR EXPERIENCE ---\n");
	Report += FString::Printf(TEXT("Jump Scares: %d\n"), Metrics.JumpScareCount);
	Report += FString::Printf(TEXT("Total Scare Intensity: %.2f\n"), Metrics.TotalScareIntensity);

	float AvgScareIntensity = Metrics.JumpScareCount > 0
		? Metrics.TotalScareIntensity / Metrics.JumpScareCount
		: 0.0f;
	Report += FString::Printf(TEXT("Average Scare Intensity: %.2f\n"), AvgScareIntensity);

	// Session
	Report += TEXT("\n--- SESSION ---\n");
	Report += FString::Printf(TEXT("Session Duration: %.2f minutes\n"), Metrics.SessionDuration / 60.0f);
	Report += FString::Printf(TEXT("Saves: %d\n"), Metrics.SaveCount);
	Report += FString::Printf(TEXT("Loads: %d\n"), Metrics.LoadCount);

	Report += TEXT("\n================================\n");

	return Report;
}
