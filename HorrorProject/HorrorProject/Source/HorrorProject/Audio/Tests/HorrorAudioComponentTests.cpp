#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR && HORRORPROJECT_ENABLE_LEGACY_AUTOMATION_TESTS

// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHorrorAudioSubsystemTest, "HorrorProject.Audio.Subsystem", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorAudioSubsystemTest::RunTest(const FString& Parameters)
{
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAudioPoolTest, "HorrorProject.Audio.Pool", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FAudioPoolTest::RunTest(const FString& Parameters)
{
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAudioQueueTest, "HorrorProject.Audio.Queue", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FAudioQueueTest::RunTest(const FString& Parameters)
{
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAudio3DPositioningTest, "HorrorProject.Audio.3DPositioning", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FAudio3DPositioningTest::RunTest(const FString& Parameters)
{
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAudioOcclusionTest, "HorrorProject.Audio.Occlusion", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FAudioOcclusionTest::RunTest(const FString& Parameters)
{
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFootstepComponentTest, "HorrorProject.Audio.Components.Footstep", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FFootstepComponentTest::RunTest(const FString& Parameters)
{
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBreathingComponentTest, "HorrorProject.Audio.Components.Breathing", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FBreathingComponentTest::RunTest(const FString& Parameters)
{
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAmbientAudioComponentTest, "HorrorProject.Audio.Components.Ambient", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FAmbientAudioComponentTest::RunTest(const FString& Parameters)
{
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FUnderwaterAudioComponentTest, "HorrorProject.Audio.Components.Underwater", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FUnderwaterAudioComponentTest::RunTest(const FString& Parameters)
{
	return true;
}

#endif

#endif // WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR
