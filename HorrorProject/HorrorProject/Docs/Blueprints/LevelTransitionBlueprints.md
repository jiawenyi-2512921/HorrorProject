# 关卡传送系统蓝图实现指南

## 概述

本文档提供关卡传送系统的蓝图实现指南，包括传送触发器、传送管理器和与现有目标系统的集成。

## 核心蓝图类

### 1. BP_LevelTransitionTrigger

传送触发器蓝图，继承自ALevelTransitionTrigger C++类。

#### 创建步骤

1. 在Content Browser中右键 → Blueprint Class
2. 选择父类：LevelTransitionTrigger
3. 命名为：BP_LevelTransitionTrigger
4. 保存位置：Content/Blueprints/LevelTransition/

#### 组件配置

```
BP_LevelTransitionTrigger
├── TriggerVolume (BoxComponent) - 继承自父类
│   └── Collision Preset: OverlapAllDynamic
├── EditorIcon (BillboardComponent) - 继承自父类
└── TransitionEffect (ParticleSystemComponent) - 可选视觉效果
```

#### 属性配置

在Details面板中配置：

```
Transition | Config
├── Trigger Type: OnOverlap / OnObjectiveComplete / Manual
├── Target Level Name: "Level_02_Hospital"
├── Target Spawn Point Tag: "HospitalEntrance"
├── Required Objective Node Ids: ["FindKey", "UnlockDoor"]
├── Required Tags: GameplayTag容器
├── Only Trigger Once: true
└── Transition Delay: 1.0

Transition | Animation
├── Transition Animation Type: "FadeToBlack"
└── Transition Delay: 1.0
```

#### 蓝图事件实现

##### Event OnBeforeTransition

在传送前执行的逻辑（保存数据、播放音效等）。

```
Event OnBeforeTransition
  ├── Instigator Actor (输入)
  │
  ├── [可选] Play Sound 2D
  │   └── Sound: TransitionSound
  │
  ├── [可选] Save Game State
  │   └── Get Game Instance → Save Player State For Transition
  │
  └── [可选] Show Transition UI
      └── Create Widget → Add to Viewport
```

##### Event OnTransitionStarted

传送开始时的逻辑（播放过渡动画）。

```
Event OnTransitionStarted
  ├── Instigator Actor (输入)
  │
  ├── Get Player Controller
  │   └── Cast to PlayerController
  │
  ├── Disable Input
  │   └── Disable Input (Player Controller)
  │
  ├── Play Transition Animation
  │   ├── Branch (Transition Animation Type)
  │   ├── "FadeToBlack" → Play Fade Animation
  │   ├── "WhiteFlash" → Play Flash Animation
  │   └── "StaticNoise" → Play Noise Animation
  │
  └── [自动] 延迟后执行关卡切换（C++层处理）
```

#### 示例蓝图图表

##### 区域触发类型示例

```
[TriggerVolume] OnComponentBeginOverlap
  ↓
[Cast to Character] Other Actor
  ↓ (Success)
[Branch] Can Trigger Transition?
  ↓ (True)
[Trigger Transition] Self
  ↓
[Play Sound 2D] TransitionSound
```

##### 目标完成触发类型示例

```
[Event BeginPlay]
  ↓
[Get World Subsystem] HorrorObjectiveManager
  ↓
[Bind Event to OnNodeStateChanged]
  ↓
[Custom Event: OnObjectiveNodeChanged]
  ├── Node (输入)
  ├── New State (输入)
  │
  ├── [Branch] New State == Completed?
  │   ↓ (True)
  ├── [Branch] Node ID in Required Objective Node Ids?
  │   ↓ (True)
  ├── [Branch] All Required Objectives Complete?
  │   ↓ (True)
  └── [Trigger Transition] Self
```

### 2. BP_LevelTransitionManager

传送管理器蓝图，继承自ULevelTransitionManager C++类。

#### 创建步骤

1. 在Content Browser中右键 → Blueprint Class
2. 选择父类：LevelTransitionManager
3. 命名为：BP_LevelTransitionManager
4. 保存位置：Content/Blueprints/LevelTransition/

#### 功能函数

##### Execute Transition with Animation

执行带动画的传送。

```
Function: ExecuteTransitionWithAnimation
  ├── 输入
  │   ├── Target Level Name (Name)
  │   ├── Spawn Point Tag (Name)
  │   ├── Instigator Actor (Actor)
  │   └── Animation Type (Name)
  │
  ├── Get Game Instance
  │   └── Cast to HorrorGameInstance
  │
  ├── Make LevelTransitionData
  │   ├── Target Level Name
  │   ├── Spawn Point Tag
  │   ├── Use Spawn Point: true
  │   └── Transition Animation Type
  │
  ├── Save Player State For Transition
  │   └── Get Player Controller
  │
  ├── Broadcast OnTransitionStarted
  │
  ├── Delay (Transition Duration)
  │
  └── Open Level
      └── Level Name: Target Level Name
```

##### Find Spawn Point By Tag

查找指定标签的出生点。

```
Function: FindSpawnPointByTag
  ├── 输入: Spawn Point Tag (Name)
  ├── 输出: Player Start (Actor)
  │
  ├── Get All Actors of Class
  │   └── Class: PlayerStart
  │
  ├── ForEach Loop
  │   ├── Get Actor with Tag
  │   └── Branch (Tag Matches?)
  │       ↓ (True)
  │       └── Return Player Start
  │
  └── Return None (未找到)
```

### 3. BP_LevelSpawnPoint

关卡出生点蓝图，继承自PlayerStart。

#### 创建步骤

1. 在Content Browser中右键 → Blueprint Class
2. 选择父类：PlayerStart
3. 命名为：BP_LevelSpawnPoint
4. 保存位置：Content/Blueprints/LevelTransition/

#### 组件配置

```
BP_LevelSpawnPoint
├── CapsuleComponent (继承)
├── ArrowComponent (继承)
├── SpawnEffect (ParticleSystemComponent) - 可选
└── DebugText (TextRenderComponent) - 编辑器可视化
```

#### 属性配置

```
Spawn Point Config
├── Spawn Point Tag: "MainEntrance"
├── Spawn Point Description: "医院主入口"
├── Camera Fade In: true
├── Fade Duration: 1.0
└── Spawn Effect: ParticleSystem引用
```

#### 蓝图实现

```
Event OnPlayerSpawned (自定义事件)
  ├── Player Controller (输入)
  │
  ├── [可选] Play Spawn Effect
  │   └── Spawn Emitter at Location
  │
  ├── [可选] Camera Fade From Black
  │   ├── Get Player Camera Manager
  │   └── Start Camera Fade (From: 1.0, To: 0.0)
  │
  └── [可选] Restore Player State
      └── Get Game Instance → Restore Player State After Transition
```

## 与HorrorObjectiveNode集成

### 在目标节点中触发传送

#### 方法1：直接在目标节点蓝图中触发

```
[BP_HorrorObjectiveNode] Event OnNodeCompleted
  ├── Instigator Actor (输入)
  │
  ├── [Branch] Should Trigger Level Transition?
  │   ↓ (True)
  ├── Get World Subsystem
  │   └── Class: LevelTransitionManager
  │
  └── Execute Transition
      ├── Target Level Name: "Level_02_Hospital"
      ├── Spawn Point Tag: "HospitalEntrance"
      └── Instigator Actor
```

#### 方法2：使用传送触发器监听目标完成

在关卡中放置BP_LevelTransitionTrigger：

1. 设置Trigger Type为OnObjectiveComplete
2. 在Required Objective Node Ids中添加目标ID
3. 触发器会自动监听目标完成事件

### 条件传送示例

```
Function: CheckTransitionConditions
  ├── 输出: Can Transition (Bool)
  │
  ├── Get World Subsystem
  │   └── Class: HorrorObjectiveManager
  │
  ├── ForEach (Required Objective Node Ids)
  │   ├── Find Node By Id
  │   └── Branch (Is Completed?)
  │       ↓ (False)
  │       └── Return False
  │
  ├── [可选] Check Inventory Items
  │   └── Has Required Items?
  │
  └── Return True
```

## 传送点配置工作流

### 1. 在源关卡中放置传送触发器

```
步骤：
1. 拖拽 BP_LevelTransitionTrigger 到关卡
2. 调整 TriggerVolume 大小和位置
3. 配置属性：
   - Target Level Name: "Level_02_Hospital"
   - Target Spawn Point Tag: "MainEntrance"
   - Trigger Type: OnOverlap
4. 测试触发区域
```

### 2. 在目标关卡中放置出生点

```
步骤：
1. 拖拽 BP_LevelSpawnPoint 到关卡
2. 调整位置和朝向（使用Arrow Component）
3. 在Tags中添加标签："MainEntrance"
4. 配置Spawn Point Tag属性
5. 测试生成位置
```

### 3. 配置关卡流程

在Project Settings → Game → Level Transition中配置：

```
Level Flow Configuration
├── Level 01 (ForestPath)
│   ├── Objectives: ["FindCabin", "InvestigateNoise"]
│   └── Transitions:
│       └── To Level_02_Hospital (SpawnPoint: "MainEntrance")
│
├── Level 02 (Hospital)
│   ├── Objectives: ["FindMedicalRecords", "EscapeHospital"]
│   └── Transitions:
│       ├── To Level_03_Basement (SpawnPoint: "BasementStairs")
│       └── To Level_01_ForestPath (SpawnPoint: "CabinExit") - 可选返回
│
└── Level 03 (Basement)
    └── Objectives: ["DefeatBoss", "FindExit"]
```

## 过渡动画蓝图实现

### BP_TransitionAnimationWidget

UMG Widget蓝图，用于显示过渡动画。

#### 创建步骤

1. 创建Widget Blueprint
2. 命名为：WBP_TransitionAnimation
3. 保存位置：Content/UI/Transition/

#### Widget结构

```
Canvas Panel
└── Overlay
    ├── Image (FadeImage)
    │   ├── Color: Black
    │   ├── Opacity: 0.0
    │   └── Render Opacity: Animated
    │
    ├── Image (NoiseImage) - 可选
    │   └── Material: M_StaticNoise
    │
    └── Text (LoadingText)
        └── Text: "Loading..."
```

#### 动画配置

##### FadeIn Animation

```
Animation: FadeIn
├── Track: FadeImage.RenderOpacity
│   ├── Time 0.0s: 0.0
│   └── Time 1.0s: 1.0
└── Easing: Ease In Out
```

##### FadeOut Animation

```
Animation: FadeOut
├── Track: FadeImage.RenderOpacity
│   ├── Time 0.0s: 1.0
│   └── Time 1.0s: 0.0
└── Easing: Ease In Out
```

#### 蓝图函数

##### Play Transition In

```
Function: PlayTransitionIn
  ├── 输入: Animation Type (Name)
  │
  ├── Switch on Name (Animation Type)
  │   ├── "FadeToBlack" → Play Animation (FadeIn)
  │   ├── "WhiteFlash" → Play Flash Animation
  │   └── "StaticNoise" → Play Noise Animation
  │
  └── Bind Event to OnAnimationFinished
```

##### Play Transition Out

```
Function: PlayTransitionOut
  ├── 输入: Animation Type (Name)
  │
  ├── Switch on Name (Animation Type)
  │   ├── "FadeToBlack" → Play Animation (FadeOut)
  │   └── Default → Play Animation (FadeOut)
  │
  └── Delay → Remove from Parent
```

### 在GameMode中集成过渡动画

```
[BP_HorrorGameMode] Event BeginPlay
  ↓
Create Widget (WBP_TransitionAnimation)
  ↓
Add to Viewport
  ↓
Set Z-Order: 999 (最顶层)
  ↓
Store as Variable: TransitionWidget
```

```
Function: StartLevelTransition
  ├── Get Transition Widget
  │
  ├── Play Transition In
  │   └── Animation Type: "FadeToBlack"
  │
  ├── Delay (Animation Duration)
  │
  └── Execute Transition
      └── Level Transition Manager
```

## 测试和调试

### 测试传送触发器

#### 测试蓝图：BP_TransitionTester

```
Event BeginPlay
  ↓
[Branch] Is Editor?
  ↓ (True)
[Delay] 2.0 seconds
  ↓
[Print String] "Testing Transition Trigger..."
  ↓
[Get All Actors of Class] LevelTransitionTrigger
  ↓
[ForEach Loop]
  ├── [Print String] Trigger Name + Config
  └── [Draw Debug Box] Trigger Volume
```

### 调试可视化

#### 在编辑器中显示传送信息

```
[BP_LevelTransitionTrigger] Event Tick (仅编辑器)
  ├── [Branch] Is Editor?
  │   ↓ (True)
  ├── Draw Debug Box
  │   ├── Location: TriggerVolume Location
  │   ├── Extent: TriggerVolume Extent
  │   └── Color: Green (可触发) / Red (不可触发)
  │
  └── Draw Debug String
      ├── Text: "→ " + Target Level Name
      └── Location: Above Trigger
```

### 快速测试命令

在Level Blueprint中添加测试快捷键：

```
Event Key (F9)
  ↓
[Print String] "Force Triggering Transition..."
  ↓
[Get All Actors of Class] LevelTransitionTrigger
  ↓
[Get] Array Element 0
  ↓
[Trigger Transition]
  └── Instigator: Get Player Character
```

## 性能优化

### 1. 触发器优化

```
[BP_LevelTransitionTrigger] Event BeginPlay
  ↓
[Branch] Trigger Type == OnOverlap?
  ↓ (True)
  └── Enable Trigger Volume
  ↓ (False)
  └── Disable Trigger Volume (节省碰撞检测)
```

### 2. 延迟加载

```
Function: PreloadTargetLevel
  ├── Load Asset Async
  │   └── Asset Path: Target Level Path
  │
  └── Bind Event to OnAssetLoaded
      └── [Print String] "Level Preloaded"
```

### 3. 资源卸载

```
Event OnBeforeTransition
  ↓
[Flush Level Streaming]
  ↓
[Garbage Collect]
  ↓
[Minimize Memory Usage]
```

## 常见蓝图模式

### 模式1：多目标完成后传送

```
[BP_MultiObjectiveTransitionTrigger]

Variables:
├── Required Objectives (Array<Name>)
└── Completed Objectives (Array<Name>)

Event OnObjectiveCompleted
  ├── Add to Completed Objectives
  │
  ├── [Branch] All Objectives Complete?
  │   ↓ (True)
  └── Trigger Transition
```

### 模式2：带确认对话框的传送

```
Event OnTriggerVolumeOverlap
  ↓
[Create Widget] WBP_TransitionConfirmDialog
  ├── Message: "前往医院？"
  ├── On Confirm → Trigger Transition
  └── On Cancel → Close Dialog
```

### 模式3：单向传送门

```
[BP_OneWayPortal]

Event OnTransitionCompleted
  ↓
[Destroy Actor] Self (传送后销毁触发器)
```

### 模式4：条件传送（需要钥匙）

```
Function: CanTriggerTransition
  ├── Get Player Character
  │
  ├── Get Component (InventoryComponent)
  │
  ├── Has Item?
  │   └── Item Name: "HospitalKey"
  │
  └── Return (Has Item)
```

## 最佳实践

### 1. 命名规范

```
传送触发器：
- Trigger_Level01_To_Level02_MainExit
- Trigger_Hospital_To_Basement_Stairs

出生点：
- Spawn_Hospital_MainEntrance
- Spawn_Basement_StairsBottom
```

### 2. 组织结构

```
Content/Blueprints/LevelTransition/
├── Core/
│   ├── BP_LevelTransitionTrigger
│   ├── BP_LevelTransitionManager
│   └── BP_LevelSpawnPoint
├── Animations/
│   ├── WBP_TransitionAnimation
│   └── Materials/
│       └── M_TransitionEffect
└── Levels/
    ├── Level01/
    │   └── BP_Level01_Transitions
    └── Level02/
        └── BP_Level02_Transitions
```

### 3. 文档注释

在蓝图中添加注释节点：

```
Comment: "传送到医院主入口"
Comment: "需要完成'找到钥匙'目标"
Comment: "播放1秒淡出动画"
```

### 4. 错误处理

```
Function: ExecuteTransition
  ↓
[Validate] Target Level Exists?
  ↓ (False)
  └── [Print String] "Error: Level not found!"
  └── Return

[Validate] Spawn Point Exists?
  ↓ (False)
  └── [Print String] "Warning: Using default spawn"
  └── Use Default Player Start
```

## 示例场景配置

### 场景：森林小屋到医院

#### Level_01_ForestPath配置

```
1. 放置 Trigger_ForestPath_To_Hospital
   - 位置：小屋门口
   - Target Level: "Level_02_Hospital"
   - Spawn Point Tag: "HospitalMainEntrance"
   - Required Objectives: ["InvestigateCabin"]

2. 配置目标节点
   - BP_ObjectiveNode_InvestigateCabin
   - 完成后自动触发传送检查
```

#### Level_02_Hospital配置

```
1. 放置 Spawn_Hospital_MainEntrance
   - 位置：医院大门内
   - Tag: "HospitalMainEntrance"
   - 朝向：面向医院内部

2. 配置欢迎触发器（可选）
   - 播放环境音效
   - 显示关卡标题
```

## 故障排除

### 问题1：传送后玩家掉落

```
解决方案：
1. 检查Spawn Point的Z轴高度
2. 在Spawn Point下方添加碰撞体
3. 在传送后延迟0.1秒再启用物理
```

### 问题2：传送触发器不响应

```
调试步骤：
1. 检查TriggerVolume的Collision设置
2. 验证Trigger Type配置
3. 使用Draw Debug Box可视化触发区域
4. 检查Required Objectives是否正确
```

### 问题3：过渡动画不播放

```
调试步骤：
1. 验证Widget是否正确添加到Viewport
2. 检查Animation是否绑定
3. 确认Transition Delay设置
4. 查看Output Log中的错误信息
```
