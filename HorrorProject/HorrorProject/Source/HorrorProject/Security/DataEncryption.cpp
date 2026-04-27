// Copyright Epic Games, Inc. All Rights Reserved.

#include "DataEncryption.h"
#include "HAL/PlatformMisc.h"
#include "Misc/AES.h"
#include "Misc/App.h"
#include "Misc/Base64.h"
#include "Misc/Guid.h"

namespace
{
	constexpr uint8 MinPKCS7Padding = 1;
	constexpr uint8 MaxPKCS7Padding = FAES::AESBlockSize;

	void AppendStringBytes(const FString& Value, TArray<uint8>& OutBytes)
	{
		FTCHARToUTF8 Converter(*Value);
		OutBytes.Append(reinterpret_cast<const uint8*>(Converter.Get()), Converter.Length());
	}
}

bool UDataEncryption::EncryptData(const TArray<uint8>& PlainData, TArray<uint8>& OutEncryptedData, const FString& Key)
{
	if (PlainData.Num() == 0 || Key.IsEmpty())
	{
		return false;
	}

	TArray<uint8> KeyBytes;
	DeriveKeyFromPassword(Key, KeyBytes);

	if (KeyBytes.Num() != FAES::FAESKey::KeySize)
	{
		return false;
	}

	OutEncryptedData = PlainData;

	const int32 PaddingSize = FAES::AESBlockSize - (OutEncryptedData.Num() % FAES::AESBlockSize);
	const uint8 PaddingByte = static_cast<uint8>(PaddingSize);
	for (int32 PaddingIndex = 0; PaddingIndex < PaddingSize; ++PaddingIndex)
	{
		OutEncryptedData.Add(PaddingByte);
	}

	FAES::EncryptData(
		OutEncryptedData.GetData(),
		static_cast<uint64>(OutEncryptedData.Num()),
		KeyBytes.GetData(),
		static_cast<uint32>(KeyBytes.Num()));

	return true;
}

bool UDataEncryption::DecryptData(const TArray<uint8>& EncryptedData, TArray<uint8>& OutPlainData, const FString& Key)
{
	if (EncryptedData.Num() == 0 || Key.IsEmpty() || EncryptedData.Num() % FAES::AESBlockSize != 0)
	{
		return false;
	}

	TArray<uint8> KeyBytes;
	DeriveKeyFromPassword(Key, KeyBytes);
	if (KeyBytes.Num() != FAES::FAESKey::KeySize)
	{
		return false;
	}

	OutPlainData = EncryptedData;
	FAES::DecryptData(
		OutPlainData.GetData(),
		static_cast<uint64>(OutPlainData.Num()),
		KeyBytes.GetData(),
		static_cast<uint32>(KeyBytes.Num()));

	const uint8 PaddingByte = OutPlainData.Last();
	if (PaddingByte < MinPKCS7Padding || PaddingByte > MaxPKCS7Padding || PaddingByte > OutPlainData.Num())
	{
		OutPlainData.Reset();
		return false;
	}

	for (int32 Index = OutPlainData.Num() - PaddingByte; Index < OutPlainData.Num(); ++Index)
	{
		if (!OutPlainData.IsValidIndex(Index) || OutPlainData[Index] != PaddingByte)
		{
			OutPlainData.Reset();
			return false;
		}
	}

	OutPlainData.SetNum(OutPlainData.Num() - PaddingByte);
	return true;
}

FString UDataEncryption::GenerateSHA256Hash(const TArray<uint8>& Data)
{
	if (Data.Num() == 0)
	{
		return FString();
	}

	FSHA256Signature Signature;
	if (!FPlatformMisc::GetSHA256Signature(Data.GetData(), static_cast<uint32>(Data.Num()), Signature))
	{
		return FString();
	}

	return Signature.ToString();
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

	while (Key.Len() < KeyLength)
	{
		Key += FGuid::NewGuid().ToString(EGuidFormats::Digits);
	}

	Key.LeftInline(KeyLength);
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
	const FString KeyMaterial = FString::Printf(
		TEXT("%s:%s:%s"),
		FApp::GetProjectName(),
		*FPlatformMisc::GetLoginId(),
		*FPlatformMisc::GetDeviceId());

	TArray<uint8> KeyMaterialBytes;
	AppendStringBytes(KeyMaterial, KeyMaterialBytes);
	return GenerateSHA256Hash(KeyMaterialBytes);
}

void UDataEncryption::DeriveKeyFromPassword(const FString& Password, TArray<uint8>& OutKey)
{
	OutKey.Reset();

	TArray<uint8> PasswordBytes;
	AppendStringBytes(Password, PasswordBytes);
	if (PasswordBytes.Num() == 0)
	{
		return;
	}

	FSHA256Signature Signature;
	if (FPlatformMisc::GetSHA256Signature(
		PasswordBytes.GetData(),
		static_cast<uint32>(PasswordBytes.Num()),
		Signature))
	{
		OutKey.Append(Signature.Signature, UE_ARRAY_COUNT(Signature.Signature));
	}
}
