// Copyright Epic Games, Inc. All Rights Reserved.

#include "InputValidator.h"
#include "Misc/Char.h"

namespace
{
	constexpr int32 MaxSaveSlotIndex = 99;
}

bool UInputValidator::ValidateString(const FString& Input, int32 MaxLength, bool bAllowSpecialChars)
{
	if (Input.IsEmpty() || Input.Len() > MaxLength)
	{
		return false;
	}

	for (const TCHAR& Char : Input)
	{
		if (!FChar::IsAlnum(Char) && !FChar::IsWhitespace(Char))
		{
			if (!bAllowSpecialChars || (Char != '_' && Char != '-'))
			{
				return false;
			}
		}
	}

	return true;
}

FString UInputValidator::SanitizeString(const FString& Input, int32 MaxLength)
{
	FString Sanitized;
	Sanitized.Reserve(FMath::Min(Input.Len(), MaxLength));

	const TCHAR* InputCursor = *Input;
	for (int32 i = 0; i < Input.Len() && Sanitized.Len() < MaxLength; ++i, ++InputCursor)
	{
		const TCHAR Char = *InputCursor;
		if (FChar::IsAlnum(Char) || FChar::IsWhitespace(Char) || Char == '_' || Char == '-')
		{
			Sanitized.AppendChar(Char);
		}
	}

	return Sanitized;
}

bool UInputValidator::ValidateInt32(int32 Value, int32 MinValue, int32 MaxValue)
{
	return Value >= MinValue && Value <= MaxValue;
}

bool UInputValidator::ValidateFloat(float Value, float MinValue, float MaxValue)
{
	return !FMath::IsNaN(Value) && !FMath::IsFinite(Value) == false && Value >= MinValue && Value <= MaxValue;
}

bool UInputValidator::ValidateSessionName(const FString& SessionName)
{
	return ValidateString(SessionName, MaxSessionNameLength, true);
}

FString UInputValidator::SanitizeSessionName(const FString& SessionName)
{
	return SanitizeString(SessionName, MaxSessionNameLength);
}

bool UInputValidator::ValidateSaveSlotIndex(int32 SlotIndex)
{
	return ValidateInt32(SlotIndex, 0, MaxSaveSlotIndex);
}

bool UInputValidator::ValidateSaveSlotName(const FString& SlotName)
{
	return ValidateString(SlotName, MaxSaveSlotNameLength, true);
}

bool UInputValidator::ValidateNetworkTransform(const FVector& Location, const FRotator& Rotation)
{
	if (FMath::IsNaN(Location.X) || FMath::IsNaN(Location.Y) || FMath::IsNaN(Location.Z))
	{
		return false;
	}

	if (FMath::Abs(Location.X) > MaxWorldBounds ||
		FMath::Abs(Location.Y) > MaxWorldBounds ||
		FMath::Abs(Location.Z) > MaxWorldBounds)
	{
		return false;
	}

	if (FMath::IsNaN(Rotation.Pitch) || FMath::IsNaN(Rotation.Yaw) || FMath::IsNaN(Rotation.Roll))
	{
		return false;
	}

	return true;
}

bool UInputValidator::ValidateNetworkTimestamp(float Timestamp, float CurrentTime)
{
	if (FMath::IsNaN(Timestamp) || Timestamp < 0.0f)
	{
		return false;
	}

	float Latency = FMath::Abs(CurrentTime - Timestamp);
	return Latency <= MaxNetworkLatency;
}

bool UInputValidator::ValidateFilePath(const FString& FilePath)
{
	if (FilePath.IsEmpty() || FilePath.Len() > MaxFilePathLength)
	{
		return false;
	}

	if (FilePath.Contains(TEXT("..")) || FilePath.Contains(TEXT("//")) || FilePath.Contains(TEXT("\\\\")))
	{
		return false;
	}

	return true;
}

FString UInputValidator::SanitizeFilePath(const FString& FilePath)
{
	FString Sanitized = FilePath;
	Sanitized.ReplaceInline(TEXT(".."), TEXT(""));
	Sanitized.ReplaceInline(TEXT("//"), TEXT("/"));
	Sanitized.ReplaceInline(TEXT("\\\\"), TEXT("\\"));

	if (Sanitized.Len() > MaxFilePathLength)
	{
		Sanitized = Sanitized.Left(MaxFilePathLength);
	}

	return Sanitized;
}
