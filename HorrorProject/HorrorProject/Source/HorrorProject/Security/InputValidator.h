// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "InputValidator.generated.h"

UCLASS()
class HORRORPROJECT_API UInputValidator : public UObject
{
	GENERATED_BODY()

public:
	// String validation
	static bool ValidateString(const FString& Input, int32 MaxLength = 256, bool bAllowSpecialChars = false);
	static FString SanitizeString(const FString& Input, int32 MaxLength = 256);

	// Numeric validation
	static bool ValidateInt32(int32 Value, int32 MinValue, int32 MaxValue);
	static bool ValidateFloat(float Value, float MinValue, float MaxValue);

	// Session name validation
	static bool ValidateSessionName(const FString& SessionName);
	static FString SanitizeSessionName(const FString& SessionName);

	// Save slot validation
	static bool ValidateSaveSlotIndex(int32 SlotIndex);
	static bool ValidateSaveSlotName(const FString& SlotName);

	// Network validation
	static bool ValidateNetworkTransform(const FVector& Location, const FRotator& Rotation);
	static bool ValidateNetworkTimestamp(float Timestamp, float CurrentTime);

	// File path validation
	static bool ValidateFilePath(const FString& FilePath);
	static FString SanitizeFilePath(const FString& FilePath);

private:
	static constexpr int32 MaxSessionNameLength = 64;
	static constexpr int32 MaxSaveSlotNameLength = 32;
	static constexpr int32 MaxFilePathLength = 260;
	static constexpr float MaxNetworkLatency = 5.0f;
	static constexpr float MaxWorldBounds = 1000000.0f;
};
