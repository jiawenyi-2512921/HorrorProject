// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "TelemetrySubsystem.generated.h"

class UPerformanceTelemetry;
class UGameplayTelemetry;
class UErrorTelemetry;

USTRUCT(BlueprintType)
struct FTelemetryReport
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Telemetry")
	FString ReportId;

	UPROPERTY(BlueprintReadWrite, Category = "Telemetry")
	FDateTime GeneratedAt;

	UPROPERTY(BlueprintReadWrite, Category = "Telemetry")
	TMap<FString, FString> PerformanceMetrics;

	UPROPERTY(BlueprintReadWrite, Category = "Telemetry")
	TMap<FString, FString> GameplayMetrics;

	UPROPERTY(BlueprintReadWrite, Category = "Telemetry")
	TArray<FString> Errors;
};

/**
 * Telemetry Subsystem - Comprehensive game telemetry system
 */
UCLASS()
class HORRORPROJECT_API UTelemetrySubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual void Tick(float DeltaTime);

	UFUNCTION(BlueprintCallable, Category = "Telemetry")
	void StartTelemetryCollection();

	UFUNCTION(BlueprintCallable, Category = "Telemetry")
	void StopTelemetryCollection();

	UFUNCTION(BlueprintCallable, Category = "Telemetry")
	FTelemetryReport GenerateReport();

	UFUNCTION(BlueprintCallable, Category = "Telemetry")
	void ExportTelemetryData(const FString& OutputPath);

	UFUNCTION(BlueprintCallable, Category = "Telemetry")
	bool IsTelemetryEnabled() const { return bTelemetryEnabled; }

	// Component Access
	UFUNCTION(BlueprintCallable, Category = "Telemetry")
	UPerformanceTelemetry* GetPerformanceTelemetry() const { return PerformanceTelemetry; }

	UFUNCTION(BlueprintCallable, Category = "Telemetry")
	UGameplayTelemetry* GetGameplayTelemetry() const { return GameplayTelemetry; }

	UFUNCTION(BlueprintCallable, Category = "Telemetry")
	UErrorTelemetry* GetErrorTelemetry() const { return ErrorTelemetry; }

protected:
	UPROPERTY()
	TObjectPtr<UPerformanceTelemetry> PerformanceTelemetry;

	UPROPERTY()
	TObjectPtr<UGameplayTelemetry> GameplayTelemetry;

	UPROPERTY()
	TObjectPtr<UErrorTelemetry> ErrorTelemetry;

	UPROPERTY()
	bool bTelemetryEnabled = true;

	UPROPERTY()
	TArray<FTelemetryReport> ReportHistory;

private:
	void CollectTelemetry();
	void SaveTelemetryReport(const FTelemetryReport& Report);

	FTimerHandle CollectionTimerHandle;
	static constexpr float CollectionInterval = 60.0f;
};
