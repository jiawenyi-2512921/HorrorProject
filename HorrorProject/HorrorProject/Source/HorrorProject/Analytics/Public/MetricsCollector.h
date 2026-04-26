// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "MetricsCollector.generated.h"

USTRUCT(BlueprintType)
struct FMetricData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Analytics")
	FString MetricName;

	UPROPERTY(BlueprintReadWrite, Category = "Analytics")
	float Value = 0.0f;

	UPROPERTY(BlueprintReadWrite, Category = "Analytics")
	FDateTime Timestamp;

	UPROPERTY(BlueprintReadWrite, Category = "Analytics")
	TMap<FString, FString> Tags;
};

USTRUCT(BlueprintType)
struct FMetricStats
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Analytics")
	float Min = 0.0f;

	UPROPERTY(BlueprintReadWrite, Category = "Analytics")
	float Max = 0.0f;

	UPROPERTY(BlueprintReadWrite, Category = "Analytics")
	float Average = 0.0f;

	UPROPERTY(BlueprintReadWrite, Category = "Analytics")
	float Total = 0.0f;

	UPROPERTY(BlueprintReadWrite, Category = "Analytics")
	int32 Count = 0;
};

/**
 * Metrics Collector - Collects and aggregates performance metrics
 */
UCLASS()
class HORRORPROJECT_API UMetricsCollector : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Analytics")
	void RecordMetric(const FString& MetricName, float Value, const TMap<FString, FString>& Tags = TMap<FString, FString>());

	UFUNCTION(BlueprintCallable, Category = "Analytics")
	void RecordTimingMetric(const FString& MetricName, float DurationMs);

	UFUNCTION(BlueprintCallable, Category = "Analytics")
	void RecordCounterMetric(const FString& MetricName, int32 Count = 1);

	UFUNCTION(BlueprintCallable, Category = "Analytics")
	FMetricStats GetMetricStats(const FString& MetricName) const;

	UFUNCTION(BlueprintCallable, Category = "Analytics")
	TArray<FString> GetAllMetricNames() const;

	void ClearAllMetrics();

	UFUNCTION(BlueprintCallable, Category = "Analytics")
	void ExportMetrics(const FString& OutputPath);

protected:
	UPROPERTY()
	TMap<FString, TArray<FMetricData>> MetricsByName;

	UPROPERTY()
	TMap<FString, FMetricStats> CachedStats;

	static constexpr int32 MaxMetricsPerName = 1000;

private:
	void UpdateStats(const FString& MetricName);
	void PruneOldMetrics(const FString& MetricName);
};
