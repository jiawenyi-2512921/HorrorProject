# 关卡串联传送系统设计文档

## 系统概述

关卡串联传送系统负责在玩家完成特定目标后，将玩家从当前地图传送到下一个地图的指定出生点。系统设计为可扩展架构，预留过渡动画接口，支持条件传送和状态保存。

## 系统架构

### 核心组件

```
LevelTransitionSystem
├── UHorrorGameInstance (跨关卡数据持久化)
├── ALevelTransitionTrigger (传送触发器Actor)
├── ULevelTransitionManager (传送管理器WorldSubsystem)
└── ULevelTransitionSaveData (传送状态存档)
```

## C++类设计

### 1. UHorrorGameInstance

负责跨关卡数据持久化和传送流程控制。

```cpp
// HorrorGameInstance.h
#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "GameplayTagContainer.h"
#include "HorrorGameInstance.generated.h"

USTRUCT(BlueprintType)
struct FLevelTransitionData
{
    GENERATED_BODY()

    // 目标地图名称
    UPROPERTY(BlueprintReadWrite)
    FName TargetLevelName;

    // 目标出生点标签
    UPROPERTY(BlueprintReadWrite)
    FName SpawnPointTag;

    // 玩家位置（如果不使用出生点）
    UPROPERTY(BlueprintReadWrite)
    FTransform PlayerTransform;

    // 是否使用出生点
    UPROPERTY(BlueprintReadWrite)
    bool bUseSpawnPoint = true;

    // 过渡动画类型
    UPROPERTY(BlueprintReadWrite)
    FName TransitionAnimationType;
};

UCLASS()
class HORRORPROJECT_API UHorrorGameInstance : public UGameInstance
{
    GENERATED_BODY()

public:
    UHorrorGameInstance();

    // 开始关卡传送
    UFUNCTION(BlueprintCallable, Category="Horror|LevelTransition")
    void BeginLevelTransition(const FLevelTransitionData& TransitionData);

    // 获取待传送数据
    UFUNCTION(BlueprintPure, Category="Horror|LevelTransition")
    FLevelTransitionData GetPendingTransitionData() const { return PendingTransitionData; }

    // 是否有待传送数据
    UFUNCTION(BlueprintPure, Category="Horror|LevelTransition")
    bool HasPendingTransition() const { return bHasPendingTransition; }

    // 清除待传送数据
    UFUNCTION(BlueprintCallable, Category="Horror|LevelTransition")
    void ClearPendingTransition();

    // 保存跨关卡玩家状态
    UFUNCTION(BlueprintCallable, Category="Horror|LevelTransition")
    void SavePlayerStateForTransition(APlayerController* PlayerController);

    // 恢复跨关卡玩家状态
    UFUNCTION(BlueprintCallable, Category="Horror|LevelTransition")
    void RestorePlayerStateAfterTransition(APlayerController* PlayerController);

protected:
    // 待传送数据
    UPROPERTY(BlueprintReadOnly)
    FLevelTransitionData PendingTransitionData;

    UPROPERTY(BlueprintReadOnly)
    bool bHasPendingTransition = false;

    // 跨关卡保存的玩家数据
    UPROPERTY()
    TArray<FGameplayTag> SavedObjectiveStates;

    UPROPERTY()
    TArray<FName> SavedCollectedItems;

    UPROPERTY()
    TMap<FName, bool> SavedInteractableStates;
};
```

### 2. ALevelTransitionTrigger

关卡传送触发器Actor，可放置在关卡中。

```cpp
// LevelTransitionTrigger.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "LevelTransitionTrigger.generated.h"

class UBoxComponent;
class UBillboardComponent;

UENUM(BlueprintType)
enum class ETransitionTriggerType : uint8
{
    // 进入触发区域时触发
    OnOverlap UMETA(DisplayName="区域触发"),

    // 完成特定目标时触发
    OnObjectiveComplete UMETA(DisplayName="目标完成触发"),

    // 手动调用触发
    Manual UMETA(DisplayName="手动触发")
};

UCLASS(BlueprintType, Blueprintable)
class HORRORPROJECT_API ALevelTransitionTrigger : public AActor
{
    GENERATED_BODY()

public:
    ALevelTransitionTrigger();

    virtual void BeginPlay() override;

    // 触发传送
    UFUNCTION(BlueprintCallable, Category="Horror|LevelTransition")
    void TriggerTransition(AActor* InstigatorActor);

    // 检查是否可以触发传送
    UFUNCTION(BlueprintPure, Category="Horror|LevelTransition")
    bool CanTriggerTransition(AActor* InstigatorActor) const;

protected:
    // 触发器类型
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Transition|Config")
    ETransitionTriggerType TriggerType = ETransitionTriggerType::OnOverlap;

    // 目标地图名称
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Transition|Config")
    FName TargetLevelName;

    // 目标出生点标签
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Transition|Config")
    FName TargetSpawnPointTag;

    // 需要完成的目标节点ID（用于OnObjectiveComplete类型）
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Transition|Config",
        meta=(EditCondition="TriggerType==ETransitionTriggerType::OnObjectiveComplete"))
    TArray<FName> RequiredObjectiveNodeIds;

    // 需要的GameplayTag条件
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Transition|Config")
    FGameplayTagContainer RequiredTags;

    // 过渡动画类型（预留）
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Transition|Animation")
    FName TransitionAnimationType = "FadeToBlack";

    // 过渡延迟时间（秒）
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Transition|Animation")
    float TransitionDelay = 1.0f;

    // 是否只能触发一次
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Transition|Config")
    bool bOnlyTriggerOnce = true;

    // 是否已触发
    UPROPERTY(BlueprintReadOnly, Category="Transition|State")
    bool bHasTriggered = false;

    // 触发区域
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Transition|Components")
    TObjectPtr<UBoxComponent> TriggerVolume;

    // 编辑器图标
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Transition|Components")
    TObjectPtr<UBillboardComponent> EditorIcon;

    // 蓝图事件：传送前
    UFUNCTION(BlueprintNativeEvent, Category="Horror|LevelTransition")
    void OnBeforeTransition(AActor* InstigatorActor);
    virtual void OnBeforeTransition_Implementation(AActor* InstigatorActor);

    // 蓝图事件：传送开始
    UFUNCTION(BlueprintNativeEvent, Category="Horror|LevelTransition")
    void OnTransitionStarted(AActor* InstigatorActor);
    virtual void OnTransitionStarted_Implementation(AActor* InstigatorActor);

private:
    UFUNCTION()
    void OnTriggerVolumeBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    void RegisterObjectiveListener();
    void OnObjectiveCompleted(FName NodeId);

    FTimerHandle TransitionDelayTimer;
};
```

### 3. ULevelTransitionManager

管理所有传送触发器和传送流程的WorldSubsystem。

```cpp
// LevelTransitionManager.h
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "LevelTransitionManager.generated.h"

class ALevelTransitionTrigger;
class APlayerStart;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnLevelTransitionStarted, FName, TargetLevel, FName, SpawnPointTag);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLevelTransitionCompleted, FName, LoadedLevel);

UCLASS()
class HORRORPROJECT_API ULevelTransitionManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    // 注册传送触发器
    UFUNCTION(BlueprintCallable, Category="Horror|LevelTransition")
    void RegisterTransitionTrigger(ALevelTransitionTrigger* Trigger);

    // 注销传送触发器
    UFUNCTION(BlueprintCallable, Category="Horror|LevelTransition")
    void UnregisterTransitionTrigger(ALevelTransitionTrigger* Trigger);

    // 执行传送
    UFUNCTION(BlueprintCallable, Category="Horror|LevelTransition")
    void ExecuteTransition(FName TargetLevelName, FName SpawnPointTag, AActor* InstigatorActor);

    // 查找出生点
    UFUNCTION(BlueprintPure, Category="Horror|LevelTransition")
    APlayerStart* FindSpawnPointByTag(FName SpawnPointTag) const;

    // 在新关卡加载后生成玩家
    UFUNCTION(BlueprintCallable, Category="Horror|LevelTransition")
    void SpawnPlayerAtTransitionPoint(APlayerController* PlayerController);

    // 事件：传送开始
    UPROPERTY(BlueprintAssignable, Category="Horror|LevelTransition")
    FOnLevelTransitionStarted OnTransitionStarted;

    // 事件：传送完成
    UPROPERTY(BlueprintAssignable, Category="Horror|LevelTransition")
    FOnLevelTransitionCompleted OnTransitionCompleted;

protected:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

private:
    UPROPERTY(Transient)
    TArray<TObjectPtr<ALevelTransitionTrigger>> RegisteredTriggers;
};
```

## 与现有系统集成

### 1. HorrorObjectiveManager集成

在目标完成时自动检查是否触发传送：

```cpp
// 在HorrorObjectiveManager::CompleteNode中添加
void UHorrorObjectiveManager::CompleteNode(FName NodeId, AActor* InstigatorActor)
{
    // 现有逻辑...

    // 检查是否触发关卡传送
    if (ULevelTransitionManager* TransitionManager = GetWorld()->GetSubsystem<ULevelTransitionManager>())
    {
        TransitionManager->CheckObjectiveTransitionTriggers(NodeId, InstigatorActor);
    }
}
```

### 2. HorrorSaveGame扩展

添加关卡传送相关的存档数据：

```cpp
// 在HorrorSaveGame.h中添加
UPROPERTY(VisibleAnywhere, BlueprintReadOnly, SaveGame, Category="Horror|Save")
FName CurrentLevelName;

UPROPERTY(VisibleAnywhere, BlueprintReadOnly, SaveGame, Category="Horror|Save")
TArray<FName> CompletedLevelTransitions;

UPROPERTY(VisibleAnywhere, BlueprintReadOnly, SaveGame, Category="Horror|Save")
TMap<FName, bool> LevelTransitionTriggerStates;
```

## 传送流程

### 标准传送流程

```
1. 触发条件满足（区域触发/目标完成/手动触发）
   ↓
2. ALevelTransitionTrigger::TriggerTransition()
   ↓
3. 检查传送条件（CanTriggerTransition）
   ↓
4. 调用OnBeforeTransition蓝图事件
   ↓
5. 保存玩家状态到GameInstance
   ↓
6. 延迟TransitionDelay秒（播放过渡动画）
   ↓
7. UGameplayStatics::OpenLevel(TargetLevelName)
   ↓
8. 新关卡加载完成
   ↓
9. ULevelTransitionManager::SpawnPlayerAtTransitionPoint()
   ↓
10. 从GameInstance恢复玩家状态
   ↓
11. 调用OnTransitionCompleted事件
```

## 过渡动画接口

### 预留接口设计

```cpp
// ITransitionAnimationInterface.h
#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "TransitionAnimationInterface.generated.h"

UINTERFACE(MinimalAPI, Blueprintable)
class UTransitionAnimationInterface : public UInterface
{
    GENERATED_BODY()
};

class ITransitionAnimationInterface
{
    GENERATED_BODY()

public:
    // 播放进入过渡动画
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Transition")
    void PlayTransitionIn(FName AnimationType);

    // 播放退出过渡动画
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Transition")
    void PlayTransitionOut(FName AnimationType);

    // 获取过渡动画时长
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Transition")
    float GetTransitionDuration(FName AnimationType) const;
};
```

### 内置过渡动画类型

- `FadeToBlack` - 淡入黑屏
- `WhiteFlash` - 白色闪光
- `StaticNoise` - 静态噪点（适合恐怖游戏）
- `Glitch` - 故障效果
- `Custom` - 自定义动画

## 使用示例

### C++代码示例

```cpp
// 在GameMode中手动触发传送
void AMyGameMode::OnMissionComplete()
{
    if (UHorrorGameInstance* GameInstance = Cast<UHorrorGameInstance>(GetGameInstance()))
    {
        FLevelTransitionData TransitionData;
        TransitionData.TargetLevelName = "Level_02_Hospital";
        TransitionData.SpawnPointTag = "HospitalEntrance";
        TransitionData.bUseSpawnPoint = true;
        TransitionData.TransitionAnimationType = "FadeToBlack";

        GameInstance->BeginLevelTransition(TransitionData);
    }
}
```

### 蓝图示例

```
Event OnObjectiveCompleted
  ↓
Get Game Instance → Cast to HorrorGameInstance
  ↓
Make LevelTransitionData
  - TargetLevelName: "Level_02_Hospital"
  - SpawnPointTag: "HospitalEntrance"
  ↓
Begin Level Transition
```

## 性能考虑

1. **异步加载**: 使用`UGameplayStatics::OpenLevel`进行异步关卡加载
2. **内存管理**: 在传送前卸载不需要的资源
3. **状态压缩**: 只保存必要的跨关卡数据
4. **触发器优化**: 使用空间分区减少触发器检测开销

## 调试工具

### 控制台命令

```cpp
// 列出所有传送触发器
Horror.LevelTransition.ListTriggers

// 强制触发传送
Horror.LevelTransition.ForceTrigger <TriggerName>

// 显示传送调试信息
Horror.LevelTransition.ShowDebug 1

// 跳过传送动画
Horror.LevelTransition.SkipAnimation 1
```

### 可视化调试

- 在编辑器中显示传送触发器边界框
- 显示传送目标连线
- 显示传送条件状态

## 扩展性

### 未来扩展方向

1. **流式传送**: 使用World Composition实现无缝传送
2. **多人传送**: 支持多玩家同时传送
3. **传送历史**: 记录传送历史，支持快速旅行
4. **条件传送网络**: 支持复杂的传送条件图
5. **动态关卡生成**: 根据玩家选择动态生成目标关卡

## 最佳实践

1. **命名规范**: 使用统一的关卡命名规范（如`Level_XX_LocationName`）
2. **出生点标签**: 使用描述性的出生点标签（如`MainEntrance`, `BackDoor`）
3. **传送测试**: 为每个传送点创建测试用例
4. **状态验证**: 在传送前后验证玩家状态完整性
5. **错误处理**: 处理关卡加载失败的情况

## 故障排除

### 常见问题

1. **传送后玩家位置错误**
   - 检查出生点标签是否正确
   - 验证PlayerStart Actor是否存在

2. **玩家状态丢失**
   - 确保GameInstance正确保存状态
   - 检查SaveGame序列化

3. **传送触发器不工作**
   - 验证触发器碰撞设置
   - 检查目标完成条件

4. **过渡动画卡住**
   - 检查TransitionDelay设置
   - 验证动画播放完成回调
