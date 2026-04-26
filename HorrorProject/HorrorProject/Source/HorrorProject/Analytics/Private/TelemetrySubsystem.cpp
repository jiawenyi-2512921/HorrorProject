// Copyright Epic Games, Inc. All Rights Reserved.

#include "TelemetrySubsystem.h"
#include "PerformanceTelemetry.h"
#include "GameplayTelemetry.h"
#include "ErrorTelemetry.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"

void UTelemetrySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Create telemetry components
	PerformanceTelemetry = NewObject<UPerformanceTelemetry>(this);
	GameplayTelemetry = NewObject<UGameplayTelemetry>(this);
	ErrorTelemetry = NewObject<UErrorTelemetry>(this);

	StartTelemetryCollection();

	UE_LOG(LogTemp, Log, TEXT("TelemetrySubsystem initialized"));
}

void UTelemetrySubsystem::Deinitialize()
{
	StopTelemetryCollection();

	// Generate final report
	FTelemetryReport FinalReport = GenerateReport();
	SaveTelemetryReport(FinalReport);

	Super::Deinitialize();
}

void UTelemetrySubsystem::Tick(float DeltaTime)
{
	if (!bTelemetryEnabled)
	{
		return;
	}

	// Update telemetry components
	if (PerformanceTelemetry)
	{
		PerformanceTelemetry->Update(DeltaTime);
	}

	if (GameplayTelemetry)
	{
		GameplayTelemetry->Update(DeltaTime);
	}
}

void UTelemetrySubsystem::StartTelemetryCollection()
{
	bTelemetryEnabled = true;

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			CollectionTimerHandle,
			this,
			&UTelemetrySubsystem::CollectTelemetry,
			CollectionInterval,
			true
		);
	}

	UE_LOG(LogTemp, Log, TEXT("Telemetry collection started"));
}

void UTelemetrySubsystem::StopTelemetryCollection()
{
	bTelemetryEnabled = false;

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(CollectionTimerHandle);
	}

	UE_LOG(LogTemp, Log, TEXT("Telemetry collection stopped"));
}

FTelemetryReport UTelemetrySubsystem::GenerateReport()
{
	FTelemetryReport Report;
	Report.ReportId = FGuid::NewGuid().ToString();
	Report.GeneratedAt = FDateTime::UtcNow();

	// Collect performance metrics
	if (PerformanceTelemetry)
	{
		Report.PerformanceMetrics.Add(TEXT("avg_fps"), FString::Printf(TEXT("%.2f"), PerformanceTelemetry->GetAverageFPS()));
		Report.PerformanceMetrics.Add(TEXT("min_fps"), FString::Printf(TEXT("%.2f"), PerformanceTelemetry->GetMinFPS()));
		Report.PerformanceMetrics.Add(TEXT("max_fps"), FString::Printf(TEXT("%.2f"), PerformanceTelemetry->GetMaxFPS()));
		Report.PerformanceMetrics.Add(TEXT("avg_frame_time"), FString::Printf(TEXT("%.2f"), PerformanceTelemetry->GetAverageFrameTime()));
		Report.PerformanceMetrics.Add(TEXT("memory_usage_mb"), FString::Printf(TEXT("%.2f"), PerformanceTelemetry->GetMemoryUsageMB()));
	}

	// Collect gameplay metrics
	if (GameplayTelemetry)
	{
		Report.GameplayMetrics.Add(TEXT("total_playtime"), FString::Printf(TEXT("%.2f"), GameplayTelemetry->GetTotalPlaytime()));
		Report.GameplayMetrics.Add(TEXT("levels_completed"), FString::Printf(TEXT("%d"), GameplayTelemetry->GetLevelsCompleted()));
		Report.GameplayMetrics.Add(TEXT("deaths"), FString::Printf(TEXT("%d"), GameplayTelemetry->GetDeathCount()));
		Report.GameplayMetrics.Add(TEXT("achievements_unlocked"), FString::Printf(TEXT("%d"), GameplayTelemetry->GetAchievementsUnlocked()));
	}

	// Collect errors
	if (ErrorTelemetry)
	{
		Report.Errors = ErrorTelemetry->GetRecentErrors();
	}

	ReportHistory.Add(Report);

	return Report;
}

void UTelemetrySubsystem::ExportTelemetryData(const FString& OutputPath)
{
	TSharedPtr<FJsonObject> RootObject = MakeShareable(new FJsonObject);

	TArray<TSharedPtr<FJsonValue>> ReportsArray;
	for (const FTelemetryReport& Report : ReportHistory)
	{
		TSharedPtr<FJsonObject> ReportObj = MakeShareable(new FJsonObject);
		ReportObj->SetStringField(TEXT("report_id"), Report.ReportId);
		ReportObj->SetStringField(TEXT("generated_at"), Report.GeneratedAt.ToIso8601());

		// Performance metrics
		TSharedPtr<FJsonObject> PerfObj = MakeShareable(new FJsonObject);
		for (const auto& Metric : Report.PerformanceMetrics)
		{
			PerfObj->SetStringField(Metric.Key, Metric.Value);
		}
		ReportObj->SetObjectField(TEXT("performance_metrics"), PerfObj);

		// Gameplay metrics
		TSharedPtr<FJsonObject> GameplayObj = MakeShareable(new FJsonObject);
		for (const auto& Metric : Report.GameplayMetrics)
		{
			GameplayObj->SetStringField(Metric.Key, Metric.Value);
		}
		ReportObj->SetObjectField(TEXT("gameplay_metrics"), GameplayObj);

		// Errors
		TArray<TSharedPtr<FJsonValue>> ErrorsArray;
		for (const FString& Error : Report.Errors)
		{
			ErrorsArray.Add(MakeShareable(new FJsonValueString(Error)));
		}
		ReportObj->SetArrayField(TEXT("errors"), ErrorsArray);

		ReportsArray.Add(MakeShareable(new FJsonValueObject(ReportObj)));
	}
	RootObject->SetArrayField(TEXT("reports"), ReportsArray);

	FString JsonString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonString);
	FJsonSerializer::Serialize(RootObject.ToSharedRef(), Writer);

	FFileHelper::SaveStringToFile(JsonString, *OutputPath);

	UE_LOG(LogTemp, Log, TEXT("Telemetry data exported to: %s"), *OutputPath);
}

void UTelemetrySubsystem::CollectTelemetry()
{
	FTelemetryReport Report = GenerateReport();
	SaveTelemetryReport(Report);

	UE_LOG(LogTemp, Verbose, TEXT("Telemetry collected: %s"), *Report.ReportId);
}

void UTelemetrySubsystem::SaveTelemetryReport(const FTelemetryReport& Report)
{
	FString ReportPath = FPaths::ProjectSavedDir() / TEXT("Analytics/Telemetry/") / FString::Printf(TEXT("report_%s.json"), *Report.ReportId);

	TSharedPtr<FJsonObject> ReportObj = MakeShareable(new FJsonObject);
	ReportObj->SetStringField(TEXT("report_id"), Report.ReportId);
	ReportObj->SetStringField(TEXT("generated_at"), Report.GeneratedAt.ToIso8601());

	TSharedPtr<FJsonObject> PerfObj = MakeShareable(new FJsonObject);
	for (const auto& Metric : Report.PerformanceMetrics)
	{
		PerfObj->SetStringField(Metric.Key, Metric.Value);
	}
	ReportObj->SetObjectField(TEXT("performance_metrics"), PerfObj);

	TSharedPtr<FJsonObject> GameplayObj = MakeShareable(new FJsonObject);
	for (const auto& Metric : Report.GameplayMetrics)
	{
		GameplayObj->SetStringField(Metric.Key, Metric.Value);
	}
	ReportObj->SetObjectField(TEXT("gameplay_metrics"), GameplayObj);

	TArray<TSharedPtr<FJsonValue>> ErrorsArray;
	for (const FString& Error : Report.Errors)
	{
		ErrorsArray.Add(MakeShareable(new FJsonValueString(Error)));
	}
	ReportObj->SetArrayField(TEXT("errors"), ErrorsArray);

	FString JsonString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonString);
	FJsonSerializer::Serialize(ReportObj.ToSharedRef(), Writer);

	FFileHelper::SaveStringToFile(JsonString, *ReportPath);
}
