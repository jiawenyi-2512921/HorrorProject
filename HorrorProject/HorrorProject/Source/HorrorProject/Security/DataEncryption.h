// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "DataEncryption.generated.h"

UCLASS()
class HORRORPROJECT_API UDataEncryption : public UObject
{
	GENERATED_BODY()

public:
	// AES-256 encryption for save data
	static bool EncryptData(const TArray<uint8>& PlainData, TArray<uint8>& OutEncryptedData, const FString& Key);
	static bool DecryptData(const TArray<uint8>& EncryptedData, TArray<uint8>& OutPlainData, const FString& Key);

	// Hash generation for integrity checks
	static FString GenerateSHA256Hash(const TArray<uint8>& Data);
	static bool VerifyDataIntegrity(const TArray<uint8>& Data, const FString& ExpectedHash);

	// Secure key generation
	static FString GenerateSecureKey(int32 KeyLength = 32);

	// Save data encryption helpers
	static bool EncryptSaveData(const TArray<uint8>& SaveData, TArray<uint8>& OutEncrypted);
	static bool DecryptSaveData(const TArray<uint8>& EncryptedData, TArray<uint8>& OutDecrypted);

private:
	static FString GetEncryptionKey();
	static void DeriveKeyFromPassword(const FString& Password, TArray<uint8>& OutKey);
};
