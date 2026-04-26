// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#if WITH_DEV_AUTOMATION_TESTS

// 替换 if (!Foo) { TestWorld.DestroyTestWorld(false); return false; } 早退梯。
// 失败时通过 AddError 输出原因并清理世界，避免吞掉后续断言。
//
// 用法：
//   HORROR_TEST_REQUIRE(GameMode != nullptr, "Game mode must spawn");
//   HORROR_TEST_REQUIRE_WORLD(World, TestWorld, "Test world must be created");
#define HORROR_TEST_REQUIRE(Condition, Message) \
	do { if (!(Condition)) { AddError(FString::Printf(TEXT("Required precondition failed: %s"), TEXT(Message))); return false; } } while(0)

#define HORROR_TEST_REQUIRE_WORLD(World, TestWorld, Message) \
	do { if (!(World)) { AddError(FString::Printf(TEXT("Required world precondition failed: %s"), TEXT(Message))); (TestWorld).DestroyTestWorld(false); return false; } } while(0)

#endif
