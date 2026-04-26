// Copyright Epic Games, Inc. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "Security/InputValidator.h"
#include "Security/DataEncryption.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FInputValidatorStringTest, "HorrorProject.Security.InputValidator.String", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FInputValidatorStringTest::RunTest(const FString& Parameters)
{
	// Valid strings
	TestTrue("Valid alphanumeric string", UInputValidator::ValidateString(TEXT("ValidName123"), 256, false));
	TestTrue("Valid string with spaces", UInputValidator::ValidateString(TEXT("Valid Name 123"), 256, false));
	TestTrue("Valid string with special chars", UInputValidator::ValidateString(TEXT("Valid_Name-123"), 256, true));

	// Invalid strings
	TestFalse("Empty string", UInputValidator::ValidateString(TEXT(""), 256, false));
	TestFalse("String too long", UInputValidator::ValidateString(FString::ChrN(300, 'A'), 256, false));
	TestFalse("String with special chars when not allowed", UInputValidator::ValidateString(TEXT("Invalid@Name"), 256, false));

	// Sanitization
	FString Sanitized = UInputValidator::SanitizeString(TEXT("Test<>Name&123"), 256);
	TestTrue("Sanitized string removes special chars", !Sanitized.Contains(TEXT("<")) && !Sanitized.Contains(TEXT(">")));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FInputValidatorNumericTest, "HorrorProject.Security.InputValidator.Numeric", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FInputValidatorNumericTest::RunTest(const FString& Parameters)
{
	// Valid integers
	TestTrue("Valid int32 in range", UInputValidator::ValidateInt32(50, 0, 100));
	TestTrue("Valid int32 at min", UInputValidator::ValidateInt32(0, 0, 100));
	TestTrue("Valid int32 at max", UInputValidator::ValidateInt32(100, 0, 100));

	// Invalid integers
	TestFalse("Int32 below min", UInputValidator::ValidateInt32(-1, 0, 100));
	TestFalse("Int32 above max", UInputValidator::ValidateInt32(101, 0, 100));

	// Valid floats
	TestTrue("Valid float in range", UInputValidator::ValidateFloat(50.5f, 0.0f, 100.0f));
	TestFalse("NaN float", UInputValidator::ValidateFloat(NAN, 0.0f, 100.0f));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FInputValidatorSessionTest, "HorrorProject.Security.InputValidator.Session", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FInputValidatorSessionTest::RunTest(const FString& Parameters)
{
	// Valid session names
	TestTrue("Valid session name", UInputValidator::ValidateSessionName(TEXT("MyGameSession")));
	TestTrue("Valid session with numbers", UInputValidator::ValidateSessionName(TEXT("Session_123")));

	// Invalid session names
	TestFalse("Empty session name", UInputValidator::ValidateSessionName(TEXT("")));
	TestFalse("Session name too long", UInputValidator::ValidateSessionName(FString::ChrN(100, 'A')));

	// Sanitization
	FString Sanitized = UInputValidator::SanitizeSessionName(TEXT("Session<script>alert()</script>"));
	TestTrue("Sanitized session name", !Sanitized.Contains(TEXT("<")) && !Sanitized.Contains(TEXT(">")));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FInputValidatorNetworkTest, "HorrorProject.Security.InputValidator.Network", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FInputValidatorNetworkTest::RunTest(const FString& Parameters)
{
	// Valid transforms
	FVector ValidLocation(100.0f, 200.0f, 300.0f);
	FRotator ValidRotation(0.0f, 90.0f, 0.0f);
	TestTrue("Valid transform", UInputValidator::ValidateNetworkTransform(ValidLocation, ValidRotation));

	// Invalid transforms - NaN
	FVector NaNLocation(NAN, 200.0f, 300.0f);
	TestFalse("NaN location", UInputValidator::ValidateNetworkTransform(NaNLocation, ValidRotation));

	// Invalid transforms - out of bounds
	FVector OutOfBoundsLocation(2000000.0f, 200.0f, 300.0f);
	TestFalse("Out of bounds location", UInputValidator::ValidateNetworkTransform(OutOfBoundsLocation, ValidRotation));

	// Valid timestamp
	TestTrue("Valid timestamp", UInputValidator::ValidateNetworkTimestamp(100.0f, 101.0f));

	// Invalid timestamp
	TestFalse("Invalid timestamp - too old", UInputValidator::ValidateNetworkTimestamp(100.0f, 110.0f));
	TestFalse("NaN timestamp", UInputValidator::ValidateNetworkTimestamp(NAN, 100.0f));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDataEncryptionTest, "HorrorProject.Security.DataEncryption.Basic", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FDataEncryptionTest::RunTest(const FString& Parameters)
{
	// Test encryption/decryption
	TArray<uint8> PlainData = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
	TArray<uint8> EncryptedData;
	TArray<uint8> DecryptedData;

	FString Key = TEXT("TestKey123");

	TestTrue("Encryption succeeds", UDataEncryption::EncryptData(PlainData, EncryptedData, Key));
	TestTrue("Encrypted data differs from plain", EncryptedData != PlainData);

	TestTrue("Decryption succeeds", UDataEncryption::DecryptData(EncryptedData, DecryptedData, Key));
	TestTrue("Decrypted data matches original", DecryptedData == PlainData);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDataEncryptionHashTest, "HorrorProject.Security.DataEncryption.Hash", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FDataEncryptionHashTest::RunTest(const FString& Parameters)
{
	TArray<uint8> TestData = {1, 2, 3, 4, 5};
	FString Hash1 = UDataEncryption::GenerateSHA256Hash(TestData);

	TestTrue("Hash is not empty", !Hash1.IsEmpty());

	// Same data should produce same hash
	FString Hash2 = UDataEncryption::GenerateSHA256Hash(TestData);
	TestTrue("Same data produces same hash", Hash1.Equals(Hash2));

	// Different data should produce different hash
	TArray<uint8> DifferentData = {1, 2, 3, 4, 6};
	FString Hash3 = UDataEncryption::GenerateSHA256Hash(DifferentData);
	TestFalse("Different data produces different hash", Hash1.Equals(Hash3));

	// Verify integrity
	TestTrue("Integrity verification succeeds", UDataEncryption::VerifyDataIntegrity(TestData, Hash1));
	TestFalse("Integrity verification fails for wrong hash", UDataEncryption::VerifyDataIntegrity(TestData, Hash3));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDataEncryptionKeyGenerationTest, "HorrorProject.Security.DataEncryption.KeyGeneration", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FDataEncryptionKeyGenerationTest::RunTest(const FString& Parameters)
{
	FString Key1 = UDataEncryption::GenerateSecureKey(32);
	TestTrue("Key has correct length", Key1.Len() == 32);

	FString Key2 = UDataEncryption::GenerateSecureKey(32);
	TestFalse("Different keys are generated", Key1.Equals(Key2));

	// Test different lengths
	FString ShortKey = UDataEncryption::GenerateSecureKey(16);
	TestTrue("Short key has correct length", ShortKey.Len() == 16);

	FString LongKey = UDataEncryption::GenerateSecureKey(64);
	TestTrue("Long key has correct length", LongKey.Len() == 64);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FInputValidatorFilePathTest, "HorrorProject.Security.InputValidator.FilePath", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FInputValidatorFilePathTest::RunTest(const FString& Parameters)
{
	// Valid paths
	TestTrue("Valid file path", UInputValidator::ValidateFilePath(TEXT("SaveGames/Slot1.sav")));
	TestTrue("Valid absolute path", UInputValidator::ValidateFilePath(TEXT("C:/Games/HorrorProject/Save.sav")));

	// Invalid paths - directory traversal
	TestFalse("Path with ..", UInputValidator::ValidateFilePath(TEXT("../../../etc/passwd")));
	TestFalse("Path with double slashes", UInputValidator::ValidateFilePath(TEXT("SaveGames//Slot1.sav")));

	// Sanitization
	FString Sanitized = UInputValidator::SanitizeFilePath(TEXT("SaveGames/../Config/Settings.ini"));
	TestTrue("Sanitized path removes ..", !Sanitized.Contains(TEXT("..")));

	return true;
}
