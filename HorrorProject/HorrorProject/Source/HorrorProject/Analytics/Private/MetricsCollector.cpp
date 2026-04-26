// Copyright Epic Games, Inc. All Rights Reserved.

#include "MetricsCollector.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"

void UMetricsCollector::RecordMetric(const FString& MetricName, float Value, const TMap<FString, FString>& Tags)
{
	FMetricData Metric;
	Metric.MetricName = MetricName;
	Metric.Value = Value;
	Metric.Timestamp = FDateTime::UtcNow();
	Metric.Tags = Tags;

	TArray<FMetricData>& Metrics = MetricsByName.FindOrAdd(MetricName);
	Metrics.Add(Metric);

	PruneOldMetrics(MetricName);
	UpdateStats(MetricName);
}

void UMetricsCollector::RecordMetric(const FString& MetricName, float Value)
{
	RecordMetric(MetricName, Value, TMap<FString, FString>());
}

void UMetricsCollector::RecordTimingMetric(const FString& MetricName, float DurationMs)
{
	TMap<FString, FString> Tags;
	Tags.Add(TEXT("type"), TEXT("timing"));
	RecordMetric(MetricName, DurationMs, Tags);
}

void UMetricsCollector::RecordCounterMetric(const FString& MetricName, int32 Count)
{
	TMap<FString, FString> Tags;
	Tags.Add(TEXT("type"), TEXT("counter"));
	RecordMetric(MetricName, static_cast<float>(Count), Tags);
}

FMetricStats UMetricsCollector::GetMetricStats(const FString& MetricName) const
{
	const FMetricStats* Stats = CachedStats.Find(MetricName);
	return Stats ? *Stats : FMetricStats();
}

TArray<FString> UMetricsCollector::GetAllMetricNames() const
{
	TArray<FString> Names;
	MetricsByName.GetKeys(Names);
	return Names;
}

void UMetricsCollector::ClearAllMetrics()
{
	MetricsByName.Empty();
	CachedStats.Empty();
}

void UMetricsCollector::ExportMetrics(const FString& OutputPath)
{
	TSharedPtr<FJsonObject> RootObject = MakeShareable(new FJsonObject);

	TArray<TSharedPtr<FJsonValue>> MetricsArray;
	for (const auto& Pair : MetricsByName)
	{
		for (const FMetricData& Metric : Pair.Value)
		{
			TSharedPtr<FJsonObject> MetricObj = MakeShareable(new FJsonObject);
			MetricObj->SetStringField(TEXT("metric_name"), Metric.MetricName);
			MetricObj->SetNumberField(TEXT("value"), Metric.Value);
			MetricObj->SetStringField(TEXT("timestamp"), Metric.Timestamp.ToIso8601());

			TSharedPtr<FJsonObject> TagsObj = MakeShareable(new FJsonObject);
			for (const auto& Tag : Metric.Tags)
			{
				TagsObj->SetStringField(Tag.Key, Tag.Value);
			}
			MetricObj->SetObjectField(TEXT("tags"), TagsObj);

			MetricsArray.Add(MakeShareable(new FJsonValueObject(MetricObj)));
		}
	}
	RootObject->SetArrayField(TEXT("metrics"), MetricsArray);

	// Add stats
	TSharedPtr<FJsonObject> StatsObj = MakeShareable(new FJsonObject);
	for (const auto& Pair : CachedStats)
	{
		TSharedPtr<FJsonObject> StatObj = MakeShareable(new FJsonObject);
		StatObj->SetNumberField(TEXT("min"), Pair.Value.Min);
		StatObj->SetNumberField(TEXT("max"), Pair.Value.Max);
		StatObj->SetNumberField(TEXT("average"), Pair.Value.Average);
		StatObj->SetNumberField(TEXT("total"), Pair.Value.Total);
		StatObj->SetNumberField(TEXT("count"), Pair.Value.Count);
		StatsObj->SetObjectField(Pair.Key, StatObj);
	}
	RootObject->SetObjectField(TEXT("stats"), StatsObj);

	FString JsonString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonString);
	FJsonSerializer::Serialize(RootObject.ToSharedRef(), Writer);

	FFileHelper::SaveStringToFile(JsonString, *OutputPath);
}

void UMetricsCollector::UpdateStats(const FString& MetricName)
{
	const TArray<FMetricData>* Metrics = MetricsByName.Find(MetricName);
	if (!Metrics || Metrics->Num() == 0)
	{
		return;
	}

	FMetricStats Stats;
	Stats.Min = TNumericLimits<float>::Max();
	Stats.Max = TNumericLimits<float>::Lowest();
	Stats.Total = 0.0f;
	Stats.Count = Metrics->Num();

	for (const FMetricData& Metric : *Metrics)
	{
		Stats.Min = FMath::Min(Stats.Min, Metric.Value);
		Stats.Max = FMath::Max(Stats.Max, Metric.Value);
		Stats.Total += Metric.Value;
	}

	Stats.Average = Stats.Total / Stats.Count;

	CachedStats.Add(MetricName, Stats);
}

void UMetricsCollector::PruneOldMetrics(const FString& MetricName)
{
	TArray<FMetricData>* Metrics = MetricsByName.Find(MetricName);
	if (!Metrics)
	{
		return;
	}

	if (Metrics->Num() > MaxMetricsPerName)
	{
		Metrics->RemoveAt(0, Metrics->Num() - MaxMetricsPerName);
	}
}
