// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "DiagnosticSubsystem.generated.h"

namespace HorrorDiagnosticsDefaults
{
	inline constexpr float AutoDiagnosticsIntervalSeconds = 60.0f;
	inline constexpr float MinAcceptableFPS = 30.0f;
	inline constexpr float MaxMemoryUsageMB = 4096.0f;
	inline constexpr int32 MaxActorCount = 10000;
	inline constexpr int32 MaxDrawCalls = 5000;
}

UENUM(BlueprintType)
enum class EDiagnosticSeverity : uint8
{
	Info,
	Warning,
	Error,
	Critical
};

USTRUCT(BlueprintType)
struct FDiagnosticReport
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FString Category;

	UPROPERTY(BlueprintReadOnly)
	FString Message;

	UPROPERTY(BlueprintReadOnly)
	EDiagnosticSeverity Severity = EDiagnosticSeverity::Info;

	UPROPERTY(BlueprintReadOnly)
	FDateTime Timestamp;

	UPROPERTY(BlueprintReadOnly)
	TMap<FString, FString> AdditionalData;
};

/**
 * Centralized diagnostic subsystem for monitoring game health
 */
UCLASS()
class HORRORPROJECT_API UDiagnosticSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintCallable, Category = "Diagnostics")
	void RunFullDiagnostics();

	UFUNCTION(BlueprintCallable, Category = "Diagnostics")
	void RunPerformanceDiagnostics();

	UFUNCTION(BlueprintCallable, Category = "Diagnostics")
	void RunMemoryDiagnostics();

	UFUNCTION(BlueprintCallable, Category = "Diagnostics")
	void RunNetworkDiagnostics();

	UFUNCTION(BlueprintCallable, Category = "Diagnostics")
	void RunGameplayDiagnostics();

	UFUNCTION(BlueprintCallable, Category = "Diagnostics")
	TArray<FDiagnosticReport> GetDiagnosticReports() const { return DiagnosticReports; }

	UFUNCTION(BlueprintCallable, Category = "Diagnostics")
	void ClearDiagnosticReports();

	UFUNCTION(BlueprintCallable, Category = "Diagnostics")
	void ExportDiagnosticsToFile(const FString& FilePath);

	UFUNCTION(BlueprintCallable, Category = "Diagnostics", meta=(CPP_Default_Interval="60.0"))
	void SetAutoDiagnostics(bool bEnabled, float Interval);

	void AddDiagnosticReport(const FString& Category, const FString& Message, EDiagnosticSeverity Severity);

protected:
	void PerformAutoDiagnostics();
	void CheckFrameRate();
	void CheckMemoryUsage();
	void CheckActorCount();
	void CheckDrawCalls();

	UPROPERTY()
	TArray<FDiagnosticReport> DiagnosticReports;

	UPROPERTY()
	bool bAutoDiagnosticsEnabled;

	UPROPERTY()
	float AutoDiagnosticsInterval;

	FTimerHandle AutoDiagnosticsTimer;

	// Thresholds
	UPROPERTY(EditDefaultsOnly, Category = "Diagnostics")
	float MinAcceptableFPS = HorrorDiagnosticsDefaults::MinAcceptableFPS;

	UPROPERTY(EditDefaultsOnly, Category = "Diagnostics")
	float MaxMemoryUsageMB = HorrorDiagnosticsDefaults::MaxMemoryUsageMB;

	UPROPERTY(EditDefaultsOnly, Category = "Diagnostics")
	int32 MaxActorCount = HorrorDiagnosticsDefaults::MaxActorCount;

	UPROPERTY(EditDefaultsOnly, Category = "Diagnostics")
	int32 MaxDrawCalls = HorrorDiagnosticsDefaults::MaxDrawCalls;
};
