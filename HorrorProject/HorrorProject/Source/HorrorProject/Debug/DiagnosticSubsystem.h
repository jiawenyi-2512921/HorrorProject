// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "DiagnosticSubsystem.generated.h"

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
	EDiagnosticSeverity Severity;

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

	UFUNCTION(BlueprintCallable, Category = "Diagnostics")
	void SetAutoDiagnostics(bool bEnabled, float Interval = 60.0f);

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
	float MinAcceptableFPS = 30.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Diagnostics")
	float MaxMemoryUsageMB = 4096.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Diagnostics")
	int32 MaxActorCount = 10000;

	UPROPERTY(EditDefaultsOnly, Category = "Diagnostics")
	int32 MaxDrawCalls = 5000;
};
