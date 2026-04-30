// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

struct HORRORPROJECT_API FHorrorMapChainEntry
{
	FString MapPackageName;
	FText DisplayName;
};

class HORRORPROJECT_API FHorrorMapChain
{
public:
	static const TArray<FHorrorMapChainEntry>& GetEntries();
	static FString NormalizeMapPackageName(FString MapPackageName);
	static int32 FindMapIndex(const FString& MapPackageName);
	static bool IsMapInChain(const FString& MapPackageName);
	static bool IsFinalMap(const FString& MapPackageName);
	static bool TryGetNextMapPackage(const FString& CurrentMapPackageName, FString& OutNextMapPackage);
	static FString BuildTravelOptionsForConfiguredGameMode();
};
