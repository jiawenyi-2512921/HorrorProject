# 交互蓝图指南

## 概述

本文档详细说明如何在HorrorProject中创建和配置交互蓝图，包括门、证据收集、触发器和事件系统。

## 目录

- [BaseInteractable基础](#baseinteractable基础)
- [门蓝图实现](#门蓝图实现)
- [证据收集蓝图](#证据收集蓝图)
- [触发器和事件系统](#触发器和事件系统)
- [高级交互模式](#高级交互模式)

---

## BaseInteractable基础

### 核心概念

所有交互物体继承自 `ABaseInteractable`，提供统一的交互接口：

```cpp
// 核心接口
- CanInteract()  // 是否可交互
- Interact()     // 执行交互
- OnInteract()   // 交互回调（子类重写）
```

### 创建基础交互物体

#### 步骤1：创建蓝图类

1. 内容浏览器右键 → **Blueprint Class**
2. 父类选择 `BaseInteractable`
3. 命名：`BP_CustomInteractable`
4. 保存到 `Content/Blueprints/Interaction/`

#### 步骤2：配置基础属性

在 **Class Defaults** 中设置：

```
Interaction:
├─ Interactable Id: "CustomInteractable_01" (唯一标识)
├─ Interaction Prompt: "按 E 交互" (提示文本)
├─ Can Interact Multiple Times: false (是否可重复交互)
└─ Require Line of Sight: true (是否需要视线)

Interaction | Audio:
└─ Interaction Sound: SFX_Interact_Generic

Interaction | Events:
├─ Interaction Event Tag: Gameplay.Interaction.Custom
└─ Interaction State Tag: State.Interacted
```

#### 步骤3：添加视觉组件

在 **Components** 面板：

```
BP_CustomInteractable (Root)
├─ StaticMesh (可交互的网格)
│   ├─ Static Mesh: SM_InteractableObject
│   └─ Collision Preset: BlockAllDynamic
└─ ParticleSystem (可选，高亮效果)
    ├─ Template: PS_InteractHighlight
    └─ Auto Activate: false
```

#### 步骤4：实现交互逻辑

重写 `OnInteract` 事件：

```
Event OnInteract
├─ [InstigatorActor] (交互发起者)
├─ [Hit] (命中结果)
│
├─ Print String: "玩家交互了物体"
├─ Play Sound at Location (Interaction Sound)
├─ Spawn Emitter at Location (PS_InteractEffect)
└─ Destroy Actor (如果是一次性交互)
```

![OnInteract事件图表截图占位符]

### 交互条件检查

自定义 `CanInteract` 逻辑：

```
Function: Can Interact (Override)
├─ Input: Instigator Actor, Hit
├─ Output: Return Value (bool)
│
├─ Parent: Can Interact (调用父类检查)
├─ Branch (Parent Result)
│   ├─ True:
│   │   ├─ 自定义条件检查
│   │   │   ├─ Has Required Item?
│   │   │   ├─ Is Quest Active?
│   │   │   └─ Is Time of Day Correct?
│   │   └─ Return (All Conditions Met)
│   └─ False:
│       └─ Return false
```

---

## 门蓝图实现

### 使用ADoorInteractable

`ADoorInteractable` 提供完整的门功能：

- 开关动画
- 锁定/解锁
- 密码保护
- 自动关闭
- 音效和事件

### 创建门蓝图

#### 步骤1：创建蓝图

1. 父类选择 `DoorInteractable`
2. 命名：`BP_DoorInteractable`
3. 保存到 `Content/Blueprints/Interaction/Doors/`

#### 步骤2：配置门组件

自动创建的组件结构：

```
BP_DoorInteractable
├─ DoorRoot (SceneComponent)
├─ DoorFrame (StaticMeshComponent)
│   └─ Static Mesh: SM_DoorFrame
├─ DoorMesh (StaticMeshComponent)
│   ├─ Static Mesh: SM_Door
│   └─ Pivot: 门轴位置 (0, 0, 0)
└─ InteractionVolume (BoxComponent)
    ├─ Box Extent: (100, 100, 100)
    └─ Collision Preset: OverlapAllDynamic
```

**重要：门轴设置**

确保 `DoorMesh` 的轴心点在门的铰链位置：

1. 在3D建模软件中设置轴心
2. 或在蓝图中调整 `DoorMesh` 的相对位置

#### 步骤3：配置门属性

```
Door | Animation:
├─ Open Angle: 90.0 (开门角度)
├─ Open Rotation Offset: (0, 90, 0) (旋转偏移)
├─ Open Speed: 2.0 (开门速度)
└─ Open Curve: Curve_DoorOpen (可选，平滑曲线)

Door | Audio:
├─ Open Sound: SFX_Door_Open
├─ Close Sound: SFX_Door_Close
└─ Locked Sound: SFX_Door_Locked

Door | Behavior:
├─ Auto Close: true (自动关闭)
├─ Auto Close Delay: 3.0 (延迟秒数)
└─ Start Locked: false (初始锁定)
```

#### 步骤4：密码门配置

对于需要密码的门：

```
Door | Password:
├─ Requires Password: true
├─ Required Password: "1234"
└─ Password Hint: "办公室白板上的数字"
```

在关卡蓝图或门蓝图中配置密码：

```
Event BeginPlay
└─ Configure Password
    ├─ Target: Self
    ├─ In Password: "1234"
    └─ In Password Hint: "办公室白板上的数字"
```

#### 步骤5：密码输入UI集成

当玩家尝试打开密码门时，门会触发密码输入请求：

```cpp
// C++层自动处理
void ADoorInteractable::RequestPasswordEntry(AActor* InstigatorActor)
{
    // 发布事件到EventBus
    // UI监听此事件并显示密码输入界面
}
```

在UI蓝图中监听：

```
Event Construct (WBP_PasswordInput)
└─ Get Event Bus
    └─ Subscribe to Event
        ├─ Event Tag: UI.PasswordInput.Request
        └─ Callback: Show Password Input
```

提交密码：

```
Button: Submit Password
└─ On Clicked
    └─ Get Door Reference
        └─ Submit Password
            ├─ Instigator Actor: Player
            ├─ Submitted Password: Input Text
            └─ Branch (Result)
                ├─ True: Close UI, Door Opens
                └─ False: Show Error Message
```

### 门状态管理

监听门状态变化：

```
Event BeginPlay
└─ Bind Event to On Door State Changed (自定义委托)
    └─ Custom Event: Handle Door State Changed
        ├─ Switch on Door State
        │   ├─ Closed: 更新UI提示
        │   ├─ Opening: 播放音效
        │   ├─ Open: 允许通过
        │   ├─ Closing: 警告玩家
        │   └─ Locked: 显示锁定图标
```

### 门事件集成

门自动发布事件到EventBus：

```
事件标签：
- Gameplay.Interaction.Door.Opened
- Gameplay.Interaction.Door.Closed
- Gameplay.Interaction.Door.Locked
- Gameplay.Interaction.Door.Unlocked
```

在其他系统中监听：

```
Get Event Bus
└─ Subscribe to Event
    ├─ Event Tag: Gameplay.Interaction.Door.Opened
    └─ Callback: On Door Opened
        └─ 触发任务进度、解锁新区域等
```

---

## 证据收集蓝图

### 证据类型

HorrorProject支持多种证据类型：

1. **文档证据** (`ADocumentInteractable`)
2. **可检查物体** (`AExaminableInteractable`)
3. **录音机** (`ARecorderInteractable`)
4. **拾取物品** (`APickupInteractable`)

### 创建文档证据

#### 步骤1：创建蓝图

1. 父类选择 `DocumentInteractable`
2. 命名：`BP_Document_CaseFile01`
3. 保存到 `Content/Blueprints/Evidence/Documents/`

#### 步骤2：配置文档内容

```
Document:
├─ Document Id: "Doc_CaseFile_01"
├─ Document Title: "案件档案 #47"
├─ Document Content: "详细的案件描述文本..."
├─ Document Image: T_Document_CaseFile01 (可选)
└─ Document Category: Evidence.Document.CaseFile

Evidence:
├─ Evidence Points: 10 (收集奖励分数)
└─ Is Critical Evidence: true (是否关键证据)
```

#### 步骤3：添加3D模型

```
Components:
├─ StaticMesh (文档外观)
│   ├─ Static Mesh: SM_Document_Paper
│   └─ Material: M_Document_Aged
└─ SpotLight (可选，高亮)
    ├─ Intensity: 1000
    └─ Attenuation Radius: 200
```

#### 步骤4：实现收集逻辑

```
Event OnInteract (Override)
├─ Parent: OnInteract
├─ Get Evidence Collection Component (from Player)
│   └─ Add Evidence
│       ├─ Evidence Id: Document Id
│       ├─ Evidence Type: Document
│       └─ Evidence Data: Document Content
├─ Play Collection Effect
│   ├─ Spawn Emitter: PS_EvidenceCollected
│   └─ Play Sound: SFX_Evidence_Collected
├─ Update UI
│   └─ Show Evidence Notification
└─ Destroy Actor (延迟0.5秒)
```

### 创建可检查物体

用于玩家可以近距离观察的物体：

#### 步骤1：创建蓝图

1. 父类选择 `ExaminableInteractable`
2. 命名：`BP_Examinable_Statue`

#### 步骤2：配置检查模式

```
Examinable:
├─ Examination Mode: Rotate (旋转查看)
├─ Allow Zoom: true
├─ Min Zoom Distance: 50.0
├─ Max Zoom Distance: 200.0
├─ Rotation Speed: 50.0
└─ Examination Duration: 0.0 (无限制)

Examination | Camera:
├─ Examination Camera Distance: 100.0
├─ Examination Camera FOV: 70.0
└─ Smooth Camera Transition: true
```

#### 步骤3：添加检查提示

```
Examinable | UI:
├─ Examination Title: "古老的雕像"
├─ Examination Description: "雕像表面有奇怪的符文..."
└─ Examination Hints: ["旋转查看背面", "注意底座"]
```

#### 步骤4：实现检查事件

```
Event On Examination Started
├─ Disable Player Movement
├─ Set Camera to Examination Mode
└─ Show Examination UI

Event On Examination Ended
├─ Restore Player Movement
├─ Restore Camera
└─ Hide Examination UI
```

### 录音机证据

#### 创建录音机蓝图

1. 父类选择 `RecorderInteractable`
2. 命名：`BP_Recorder_Testimony01`

#### 配置录音内容

```
Recorder:
├─ Recording Id: "Rec_Testimony_01"
├─ Audio Clip: SFX_Recording_Testimony01
├─ Transcript: "录音文字稿..."
├─ Recording Duration: 45.0 (秒)
└─ Auto Play on Interact: true

Recorder | Playback:
├─ Can Pause: true
├─ Can Rewind: true
└─ Show Playback UI: true
```

---

## 触发器和事件系统

### 创建触发器

#### 基础触发器

使用虚幻引擎的 `TriggerBox` 或 `TriggerVolume`：

1. 放置 `TriggerBox` 到关卡
2. 调整大小和位置
3. 在蓝图中添加逻辑

```
Event ActorBeginOverlap
├─ Cast to HorrorPlayerCharacter (Other Actor)
├─ Branch (Cast Success)
│   └─ True:
│       ├─ 触发事件逻辑
│       ├─ Publish Event to EventBus
│       │   └─ Event Tag: Gameplay.Trigger.AreaEntered
│       └─ Destroy Actor (一次性触发器)
```

#### 高级触发器蓝图

创建自定义触发器类：

```
BP_HorrorTrigger (继承自Actor)
├─ Components:
│   ├─ BoxComponent (触发体积)
│   └─ BillboardComponent (编辑器可见性)
│
├─ Properties:
│   ├─ Trigger Id: "Trigger_01"
│   ├─ Trigger Type: Enum (Enter, Exit, Stay)
│   ├─ Can Trigger Multiple Times: false
│   ├─ Cooldown Time: 0.0
│   └─ Required Actor Tag: "Player"
│
└─ Events:
    ├─ On Trigger Activated (委托)
    └─ On Trigger Deactivated (委托)
```

实现逻辑：

```
Event ActorBeginOverlap
├─ Check Can Trigger
│   ├─ Has Required Tag?
│   ├─ Not in Cooldown?
│   └─ Not Already Triggered? (if single use)
├─ Branch (Can Trigger)
│   └─ True:
│       ├─ Execute Trigger Logic
│       ├─ Broadcast On Trigger Activated
│       ├─ Publish to EventBus
│       └─ Start Cooldown Timer
```

### 事件总线集成

#### 发布事件

在任何蓝图中发布事件：

```
Function: Publish Interaction Event
├─ Get Game Instance
│   └─ Get Subsystem (HorrorEventBusSubsystem)
│       └─ Publish Event
│           ├─ Event Tag: Gameplay.Interaction.Custom
│           ├─ Source Id: Self.Interactable Id
│           ├─ Target Id: Instigator.Name
│           └─ Payload: (可选数据)
```

#### 订阅事件

在需要响应事件的Actor中：

```
Event BeginPlay
└─ Get Event Bus
    └─ Subscribe to Event
        ├─ Event Tag: Gameplay.Interaction.Door.Opened
        ├─ Callback: Custom Event "On Door Opened"
        └─ Filter Source Id: "Door_MainEntrance" (可选)

Custom Event: On Door Opened
├─ Input: Event Data (struct)
├─ 处理事件逻辑
│   ├─ 更新任务状态
│   ├─ 触发过场动画
│   └─ 生成敌人
```

#### 事件标签规范

使用GameplayTags组织事件：

```
Gameplay.Interaction.Door.Opened
Gameplay.Interaction.Door.Closed
Gameplay.Interaction.Evidence.Collected
Gameplay.Interaction.Puzzle.Solved
Gameplay.Encounter.Threat.Revealed
Gameplay.Encounter.Threat.Defeated
Gameplay.Player.Fear.Increased
Gameplay.Player.Item.Acquired
```

在 `Config/DefaultGameplayTags.ini` 中定义。

---

## 高级交互模式

### 多阶段交互

实现需要多次交互的物体：

```
BP_MultiStageInteractable
├─ Variable: Current Stage (int) = 0
├─ Variable: Max Stages (int) = 3
│
Event OnInteract (Override)
├─ Switch on Current Stage
│   ├─ Case 0: 第一阶段逻辑
│   ├─ Case 1: 第二阶段逻辑
│   ├─ Case 2: 第三阶段逻辑
│   └─ Default: 完成
├─ Increment Current Stage
└─ Update Interaction Prompt
```

### 条件交互

基于游戏状态的交互：

```
Function: Can Interact (Override)
├─ Parent: Can Interact
├─ Branch (Parent Result)
│   └─ True:
│       ├─ Get Game State
│       ├─ Check Conditions:
│       │   ├─ Has Required Item?
│       │   ├─ Quest Stage Correct?
│       │   ├─ Time of Day Valid?
│       │   └─ Player Fear Level OK?
│       └─ Return (All Conditions Met)
```

### 组合交互

多个物体协同工作：

```
BP_PuzzleSwitch (4个开关)
├─ Variable: Switch Id (int)
├─ Variable: Is Activated (bool)
│
Event OnInteract
├─ Toggle Is Activated
├─ Update Visual State
├─ Get All Puzzle Switches
├─ Check All Activated
│   └─ Branch (All Active)
│       └─ True:
│           ├─ Publish Event: Puzzle.Solved
│           └─ Trigger Reward
```

### 交互链

一个交互触发另一个：

```
BP_InteractionChain
├─ Variable: Next Interactable (Actor Reference)
│
Event OnInteract
├─ Execute This Interaction
├─ Branch (Next Interactable Valid)
│   └─ True:
│       ├─ Enable Next Interactable
│       └─ Show Hint to Player
```

### 时间限制交互

需要在限定时间内完成：

```
BP_TimedInteractable
├─ Variable: Time Limit (float) = 10.0
├─ Variable: Timer Handle (Timer Handle)
│
Event OnInteract
├─ Start Interaction
├─ Set Timer by Function Name
│   ├─ Function: "OnTimerExpired"
│   ├─ Time: Time Limit
│   └─ Looping: false
└─ Show Timer UI

Function: OnTimerExpired
├─ Cancel Interaction
├─ Play Failure Effect
└─ Reset Interactable
```

---

## 调试和测试

### 调试工具

#### 显示交互范围

```
Event Tick (仅编辑器)
└─ Draw Debug Sphere
    ├─ Center: Actor Location
    ├─ Radius: Interaction Range
    ├─ Color: Green (可交互) / Red (不可交互)
    └─ Duration: 0.0 (持续显示)
```

#### 打印交互状态

```
Event OnInteract
├─ Print String
│   ├─ In String: "Interacted: {Interactable Id}"
│   ├─ Text Color: Yellow
│   └─ Duration: 2.0
```

### 测试清单

- [ ] 交互提示正确显示
- [ ] 交互范围合理
- [ ] 音效正常播放
- [ ] 视觉反馈清晰
- [ ] 多次交互正常（如果允许）
- [ ] 保存/加载状态正确
- [ ] 事件正确发布
- [ ] 性能无问题（Tick优化）

---

## 性能优化

### 减少Tick使用

```
// 不要在Tick中检查交互
❌ Event Tick
    └─ Check Can Interact

// 使用事件驱动
✅ Event ActorBeginOverlap (InteractionVolume)
    └─ Enable Interaction Prompt
```

### 异步加载资产

```
Function: Load Document Content
├─ Async Load Asset
│   ├─ Asset Reference: Document Texture (Soft Reference)
│   └─ On Loaded:
│       └─ Display Document
```

### 对象池

对于频繁生成的交互物体：

```
Function: Spawn Interactable
├─ Get from Pool
├─ Branch (Pool Has Available)
│   ├─ True: Reuse Pooled Actor
│   └─ False: Spawn New Actor
```

---

## 相关文档

- [BlueprintArchitecture.md](BlueprintArchitecture.md) - 蓝图架构基础
- [AIBlueprints.md](AIBlueprints.md) - AI交互集成
- [../Blueprint/Interaction_Blueprint_Guide.md](../Blueprint/Interaction_Blueprint_Guide.md) - 交互系统API
- [../Examples/README.md](../Examples/README.md) - 交互示例入口

---

**版本**: 1.0
**最后更新**: 2026-04-28
**维护者**: HorrorProject Team
