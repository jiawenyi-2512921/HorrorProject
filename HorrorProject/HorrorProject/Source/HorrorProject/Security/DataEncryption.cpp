// Copyright Epic Games, Inc. All Rights Reserved.

#include "DataEncryption.h"
#include "Misc/SecureHash.h"
#include "Misc/Base64.h"
#include "HAL/PlatformFileManager.h"

bool UDataEncryption::EncryptData(const TArray<uint8>& PlainData, TArray<uint8>& OutEncryptedData, const FString& Key)
{
	if (PlainData.Num() == 0 || Key.IsEmpty())
	{
		return false;
	}

	TArray<uint8> KeyBytes;
	DeriveKeyFromPassword(Key, KeyBytes);

	OutEncryptedData.SetNum(PlainData.Num());

	for (int32 i = 0; i < PlainData.Num(); ++i)
	{
		OutEncryptedData[i] = PlainData[i] ^ KeyBytes[i % KeyBytes.Num()];
	}

	return true;
}

bool UDataEncryption::DecryptData(const TArray<uint8>& EncryptedData, TArray<uint8>& OutPlainData, const FString& Key)
{
	return EncryptData(EncryptedData, OutPlainData, Key);
}

FString UDataEncryption::GenerateSHA256Hash(const TArray<uint8>& Data)
{
	if (Data.Num() == 0)
	{
		return FString();
	}

	FSHAHash Hash;
	FSHA1::HashBuffer(Data.GetData(), Data.Num(), Hash.Hash);

	return Hash.ToString();
}

bool UDataEncryption::VerifyDataIntegrity(const TArray<uint8>& Data, const FString& ExpectedHash)
{
	FString ActualHash = GenerateSHA256Hash(Data);
	return ActualHash.Equals(ExpectedHash, ESearchCase::CaseSensitive);
}

FString UDataEncryption::GenerateSecureKey(int32 KeyLength)
{
	FString Key;
	Key.Reserve(KeyLength);

	for (int32 i = 0; i < KeyLength; ++i)
	{
		int32 RandomValue = FMath::RandRange(0, 61);
		TCHAR Char;

		if (RandomValue < 10)
		{
			Char = '0' + RandomValue;
		}
		else if (RandomValue < 36)
		{
			Char = 'A' + (RandomValue - 10);
		}
		else
		{
			Char = 'a' + (RandomValue - 36);
		}

		Key.AppendChar(Char);
	}

	return Key;
}

bool UDataEncryption::EncryptSaveData(const TArray<uint8>& SaveData, TArray<uint8>& OutEncrypted)
{
	FString Key = GetEncryptionKey();
	return EncryptData(SaveData, OutEncrypted, Key);
}

bool UDataEncryption::DecryptSaveData(const TArray<uint8>& EncryptedData, TArray<uint8>& OutDecrypted)
{
	FString Key = GetEncryptionKey();
	return DecryptData(EncryptedData, OutDecrypted, Key);
}

FString UDataEncryption::GetEncryptionKey()
{
	static FString CachedKey;

	if (CachedKey.IsEmpty())
	{
		CachedKey = TEXT("HorrorProject_SecureKey_2026_v1");
	}

	return CachedKey;
}

void UDataEncryption::DeriveKeyFromPassword(const FString& Password, TArray<uint8>& OutKey)
{
	OutKey.Reset();

	FTCHARToUTF8 Converter(*Password);
	const uint8* UTF8Data = (const uint8*)Converter.Get();
	int32 UTF8Length = Converter.Length();

	OutKey.Append(UTF8Data, UTF8Length);

	while (OutKey.Num() < 32)
	{
		OutKey.Append(UTF8Data, FMath::Min(UTF8Length, 32 - OutKey.Num()));
	}

	if (OutKey.Num() > 32)
	{
		OutKey.SetNum(32);
	}
}
