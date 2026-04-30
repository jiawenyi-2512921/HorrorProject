# 关卡传送系统快速设置指南

## 5分钟快速开始

本指南将帮助你在5分钟内设置第一个关卡传送点并测试传送流程。

## 前置要求

- Unreal Engine 5.6
- HorrorProject已正确配置
- 至少有两个关卡（源关卡和目标关卡）

## 步骤1：创建C++类（2分钟）

### 1.1 添加GameInstance类

在Visual Studio中创建`HorrorGameInstance`类：

**HorrorGameInstance.h**
```cpp
#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "HorrorGameInstance.generated.h"

USTRUCT(BlueprintType)
struct FLevelTransitionData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    FName TargetLevelName;

    UPROPERTY(BlueprintReadWrite)
    FName SpawnPointTag;

    UPROPERTY(BlueprintReadWrite)
    bool bUseSpawnPoint = true;
};

UCLASS()
class HORRORPROJECT_API UHorrorGameInstance : public UGameInstance
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category="Horror|LevelTransition")
    void BeginLevelTransition(const FLevelTransitionData& TransitionData);

    UFUNCTION(BlueprintPure, Category="Horror|LevelTransition")
    bool HasPendingTransition() const { return bHasPendingTransition; }

    UFUNCTION(BlueprintPure, Category="Horror|LevelTransition")
    FLevelTransitionData GetPendingTransitionData() const { return PendingTransitionData; }

protected:
    UPROPERTY()
    FLevelTransitionData PendingTransitionData;

    UPROPERTY()
    bool bHasPendingTransition = false;
};
```

**HorrorGameInstance.cpp**
```cpp
#include "HorrorGameInstance.h"
#include "Kismet/GameplayStatics.h"

void UHorrorGameInstance::BeginLevelTransition(const FLevelTransitionData& TransitionData)
{
    PendingTransitionData = TransitionData;
    bHasPendingTransition = true;

    // 打开目标关卡
    UGameplayStatics::OpenLevel(this, TransitionData.TargetLevelName);
}
```

### 1.2 编译项目

```bash
# 在项目根目录
"C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe" HorrorProject.sln /t:Build /p:Configuration=Development
```

或在Visual Studio中按`Ctrl+Shift+B`编译。

### 1.3 配置GameInstance

1. 打开Unreal Editor
2. Edit → Project Settings
3. 搜索"Game Instance"
4. 设置Game Instance Class为`HorrorGameInstance`
5. 重启编辑器

## 步骤2：创建蓝图类（1分钟）

### 2.1 创建传送触发器蓝图

1. Content Browser → 右键 → Blueprint Class
2. 选择父类：Actor
3. 命名：`BP_SimpleLevelTransition`
4. 保存位置：`Content/Blueprints/LevelTransition/`

### 2.2 添加组件

在`BP_SimpleLevelTransition`中：

1. 添加`Box Collision`组件
   - 命名：TriggerBox
   - 设置大小：X=200, Y=200, Z=200
   - Collision Preset：OverlapAllDynamic

2. 添加`Billboard`组件（可选，用于编辑器可视化）

### 2.3 添加变量

在Variables面板添加：

| 变量名 | 类型 | 默认值 | 说明 |
|--------|------|--------|------|
| TargetLevelName | Name | Level_02_Hospital | 目标关卡名称 |
| SpawnPointTag | Name | MainEntrance | 目标出生点标签 |
| TransitionDelay | Float | 1.0 | 传送延迟（秒） |

## 步骤3：实现传送逻辑（1分钟）

### 3.1 蓝图事件图表

在`BP_SimpleLevelTransition`的Event Graph中：

```
[Event BeginPlay]
  ↓
[TriggerBox] Bind Event to OnComponentBeginOverlap
```

```
[OnComponentBeginOverlap]
  ↓
[Cast to Character] Other Actor
  ↓ (Success)
[Delay] TransitionDelay
  ↓
[Get Game Instance]
  ↓
[Cast to HorrorGameInstance]
  ↓
[Make LevelTransitionData]
  ├── Target Level Name: TargetLevelName
  ├── Spawn Point Tag: SpawnPointTag
  └── Use Spawn Point: true
  ↓
[Begin Level Transition]
```

### 3.2 简化版蓝图（复制粘贴）

如果你熟悉蓝图，可以直接实现这个简化版本：

1. Event BeginPlay → Bind OnComponentBeginOverlap
2. OnComponentBeginOverlap → Cast to Character → Delay → Open Level (TargetLevelName)

## 步骤4：配置关卡（1分钟）

### 4.1 在源关卡中放置传送触发器

1. 打开源关卡（如`Level_01_ForestPath`）
2. 从Content Browser拖拽`BP_SimpleLevelTransition`到关卡
3. 在Details面板配置：
   - Target Level Name: `Level_02_Hospital`
   - Spawn Point Tag: `MainEntrance`
   - Transition Delay: `1.0`
4. 调整触发器位置（放在玩家需要传送的位置）

### 4.2 在目标关卡中放置出生点

1. 打开目标关卡（如`Level_02_Hospital`）
2. 从Place Actors面板拖拽`Player Start`到关卡
3. 在Details面板的Tags中添加：`MainEntrance`
4. 调整出生点位置和朝向（使用Arrow Component）

## 步骤5：测试传送（30秒）

### 5.1 运行测试

1. 在源关卡中点击Play（Alt+P）
2. 移动角色进入传送触发器区域
3. 等待1秒延迟
4. 观察是否成功传送到目标关卡

### 5.2 验证检查清单

- [ ] 成功传送到目标关卡
- [ ] 玩家出现在正确的出生点位置
- [ ] 玩家朝向正确
- [ ] 没有报错信息

## 常见问题快速解决

### 问题1：传送后玩家位置不对

**原因**：出生点标签不匹配

**解决**：
1. 检查触发器的`SpawnPointTag`
2. 检查目标关卡PlayerStart的Tags
3. 确保两者完全一致（区分大小写）

### 问题2：传送不触发

**原因**：碰撞设置错误

**解决**：
1. 选中TriggerBox组件
2. Details → Collision → Collision Presets → OverlapAllDynamic
3. 确保Generate Overlap Events已勾选

### 问题3：找不到目标关卡

**原因**：关卡名称错误或未添加到构建

**解决**：
1. 检查关卡名称拼写（不包含.umap后缀）
2. Edit → Project Settings → Packaging → List of maps to include
3. 添加目标关卡到列表

### 问题4：传送后黑屏

**原因**：目标关卡没有PlayerStart

**解决**：
1. 在目标关卡中添加PlayerStart
2. 或者在GameMode中设置默认出生点

## 进阶功能（可选）

### 添加淡入淡出效果

在传送前添加：

```
[Get Player Controller]
  ↓
[Get Player Camera Manager]
  ↓
[Start Camera Fade]
  ├── From Alpha: 0.0
  ├── To Alpha: 1.0
  ├── Duration: 1.0
  └── Color: Black
```

### 添加传送音效

在Delay节点前添加：

```
[Play Sound 2D]
  └── Sound: YourTransitionSound
```

### 添加条件传送（需要完成目标）

在Cast to Character后添加：

```
[Get World Subsystem]
  └── Class: HorrorObjectiveManager
  ↓
[Are Prerequisites Met]
  └── Prerequisite Node Ids: ["FindKey", "UnlockDoor"]
  ↓
[Branch]
  ├── True → Continue Transition
  └── False → Print String "需要先完成目标"
```

## 下一步

现在你已经有了一个基本的传送系统！接下来可以：

1. **添加过渡动画**：参考`Docs/Blueprints/LevelTransitionBlueprints.md`
2. **集成目标系统**：让传送在完成特定目标后自动触发
3. **保存玩家状态**：在传送时保存物品栏和进度
4. **批量配置**：使用`Scripts/LevelDesign/LevelTransitionSetup.py`工具

## 完整示例项目结构

```
Content/
├── Blueprints/
│   └── LevelTransition/
│       └── BP_SimpleLevelTransition
├── Maps/
│   ├── Level_01_ForestPath
│   └── Level_02_Hospital
└── Sounds/
    └── TransitionSound

Config/
└── LevelFlow.json

Scripts/
└── LevelDesign/
    └── LevelTransitionSetup.py
```

## 测试检查清单

完成以下测试以确保系统正常工作：

- [ ] 基本传送功能正常
- [ ] 出生点位置正确
- [ ] 可以在编辑器中测试
- [ ] 可以在打包后的游戏中测试
- [ ] 传送延迟正常工作
- [ ] 没有内存泄漏或崩溃

## 性能提示

- 传送触发器使用简单碰撞体（Box/Sphere）
- 避免在Tick中检查传送条件
- 使用事件驱动而非轮询
- 传送前卸载不需要的资源

## 获取帮助

如果遇到问题：

1. 查看Output Log（Window → Developer Tools → Output Log）
2. 检查蓝图编译错误
3. 参考完整文档：`Docs/Systems/LevelTransitionSystem.md`
4. 使用调试工具：Print String节点显示变量值

## 总结

你现在已经完成了：

✅ 创建了GameInstance类来管理跨关卡数据
✅ 创建了传送触发器蓝图
✅ 配置了源关卡和目标关卡
✅ 测试了传送功能

总用时：约5分钟

下一步可以根据项目需求添加更多高级功能！
