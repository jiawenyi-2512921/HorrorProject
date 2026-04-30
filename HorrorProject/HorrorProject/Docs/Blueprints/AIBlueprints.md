# AI蓝图指南

## 概述

本文档详细说明如何在HorrorProject中创建和配置AI蓝图，包括石头魔像行为树、巡逻追逐逻辑，以及与HorrorEncounterDirector的集成。

## 目录

- [AI架构概览](#ai架构概览)
- [石头魔像蓝图](#石头魔像蓝图)
- [行为树和黑板](#行为树和黑板)
- [巡逻和追逐逻辑](#巡逻和追逐逻辑)
- [与EncounterDirector集成](#与encounterdirector集成)
- [AI调试和优化](#ai调试和优化)

---

## AI架构概览

### 核心组件

HorrorProject的AI系统由以下部分组成：

```
AI系统架构：
├─ AHorrorThreatCharacter (威胁角色)
│   ├─ UHorrorGolemBehaviorComponent (魔像行为组件)
│   └─ AHorrorThreatAIController (AI控制器)
├─ AHorrorEncounterDirector (遭遇导演)
└─ Behavior Tree + Blackboard (行为树系统)
```

### 魔像遭遇阶段

石头魔像有5个遭遇阶段：

| 阶段 | 名称 | 距离 | 行为 | 速度 |
|------|------|------|------|------|
| 1 | Dormant | - | 休眠 | 0 |
| 2 | DistantSighting | 30m+ | 远距离静止观察 | 0.5 m/s |
| 3 | CloseStalking | 10-15m | 近距离跟踪巡逻 | 1.5 m/s |
| 4 | ChaseTriggered | 20m | 追逐触发 | 4.0 m/s (70%) |
| 5 | FullChase | 10-25m | 全速追逐 | 6.0 m/s (100%) |
| 6 | FinalImpact | <5m | 最终冲击 | 攻击 |

---

## 石头魔像蓝图

### 创建魔像蓝图

#### 步骤1：创建蓝图类

1. 内容浏览器右键 → **Blueprint Class**
2. 父类选择 `HorrorThreatCharacter`
3. 命名：`BP_GolemThreat`
4. 保存到 `Content/Blueprints/AI/Threats/`

#### 步骤2：配置基础属性

在 **Class Defaults** 中：

```
Horror | Threat:
├─ Threat Id: "Threat.Golem.Main"
├─ Detection Radius: 800.0 (检测半径，cm)
└─ Threat Active: false (初始状态)

Character Movement:
├─ Max Walk Speed: 600.0 (最大速度，对应FullChase)
├─ Max Acceleration: 2048.0
├─ Braking Deceleration Walking: 2048.0
└─ Can Crouch: false
```

#### 步骤3：配置魔像行为组件

`UHorrorGolemBehaviorComponent` 自动添加到角色，配置各阶段参数：

```
Horror | Golem | Phase1 (远距离目击):
├─ Distant Sighting Min Distance: 3000.0 (30m)
├─ Distant Sighting Move Speed: 50.0 (0.5 m/s)
└─ Distant Sighting Stationary: true (静止观察)

Horror | Golem | Phase2 (近距离跟踪):
├─ Close Stalking Min Distance: 1000.0 (10m)
├─ Close Stalking Max Distance: 1500.0 (15m)
├─ Close Stalking Patrol Speed: 150.0 (1.5 m/s)
└─ Close Stalking Patrol Pause Time: 2.0 (巡逻暂停秒数)

Horror | Golem | Phase3 (追逐触发):
├─ Chase Triggered Start Distance: 2000.0 (20m)
├─ Chase Triggered Speed Multiplier: 0.7 (70%速度)
└─ Chase Triggered Base Speed: 400.0 (4.0 m/s)

Horror | Golem | Phase4 (全速追逐):
├─ Full Chase Min Distance: 1000.0 (10m)
├─ Full Chase Max Distance: 2500.0 (25m)
├─ Full Chase Speed: 600.0 (6.0 m/s)
├─ Chase Lost Target Distance: 4000.0 (失去目标距离)
├─ Chase Lost Target Grace Time: 2.0 (宽限时间)
└─ Full Chase Enable Destruction: true (启用环境破坏)

Horror | Golem | Phase5 (最终冲击):
├─ Final Impact Trigger Distance: 500.0 (5m)
└─ Final Impact Trigger Cutscene: true (触发过场动画)

Horror | Golem | Debug:
└─ Draw Debug Info: true (开发版本启用)
```

#### 步骤4：添加视觉组件

```
BP_GolemThreat (Root)
├─ Mesh (SkeletalMeshComponent)
│   ├─ Skeletal Mesh: SK_Golem
│   ├─ Anim Class: ABP_Golem
│   └─ Collision Preset: Pawn
├─ CapsuleComponent (继承)
│   ├─ Capsule Half Height: 96.0
│   └─ Capsule Radius: 42.0
└─ ParticleSystem (可选，魔像光环)
    ├─ Template: PS_GolemAura
    └─ Auto Activate: false
```

#### 步骤5：实现蓝图事件

##### 状态变化事件

```
Event BP_OnStateChanged (from GolemBehaviorComponent)
├─ Input: Old State, New State
├─ Switch on New State
│   ├─ Dormant:
│   │   └─ Stop All Effects
│   ├─ DistantSighting:
│   │   ├─ Play Animation: Anim_Golem_Idle
│   │   └─ Play Sound: SFX_Golem_Spotted
│   ├─ CloseStalking:
│   │   ├─ Play Animation: Anim_Golem_Walk
│   │   └─ Activate Stalking Effects
│   ├─ ChaseTriggered:
│   │   ├─ Play Animation: Anim_Golem_Run
│   │   ├─ Play Sound: SFX_Golem_Chase_Start
│   │   └─ Trigger Camera Shake
│   ├─ FullChase:
│   │   ├─ Play Animation: Anim_Golem_Sprint
│   │   ├─ Play Sound: SFX_Golem_Roar
│   │   └─ Enable Destruction VFX
│   └─ FinalImpact:
│       ├─ Play Animation: Anim_Golem_Attack
│       └─ Trigger Death Sequence
```

![BP_OnStateChanged事件图表截图占位符]

##### 追逐更新事件

```
Event BP_OnChaseUpdate
├─ Input: Delta Time, Current Speed
├─ Update Animation Speed
│   └─ Set Float Parameter (AnimBP): "MoveSpeed" = Current Speed
├─ Play Footstep Effects
│   └─ Spawn Decal at Location (每步)
└─ Update Audio
    └─ Set Sound Parameter: "Intensity" = Speed / Max Speed
```

##### 环境破坏事件

```
Event BP_OnEnvironmentDestruction
├─ Input: Destruction Location
├─ Spawn Destruction Effect
│   ├─ Spawn Emitter at Location: PS_WallBreak
│   └─ Play Sound at Location: SFX_Wall_Crumble
├─ Apply Radial Damage
│   ├─ Damage: 50.0
│   ├─ Radius: 300.0
│   └─ Damage Type: DT_Environmental
└─ Spawn Debris
    └─ Spawn Actor: BP_Debris_Chunks
```

##### 最终冲击事件

```
Event BP_OnFinalImpactTriggered
├─ Get Player Controller
│   └─ Play Camera Shake: CS_FinalImpact
├─ Play Cutscene
│   └─ Play Level Sequence: LS_GolemKill
├─ Disable Player Input
└─ Trigger Game Over
    └─ Delay 3.0
        └─ Show Death Screen
```

### 魔像动画蓝图

创建 `ABP_Golem` 动画蓝图：

#### 状态机

```
AnimGraph:
└─ State Machine: Golem Locomotion
    ├─ State: Idle
    │   └─ Animation: Anim_Golem_Idle
    ├─ State: Walk
    │   └─ Blend Space: BS_Golem_Walk
    ├─ State: Run
    │   └─ Blend Space: BS_Golem_Run
    └─ State: Attack
        └─ Animation: Anim_Golem_Attack

Transitions:
├─ Idle → Walk: Speed > 10
├─ Walk → Run: Speed > 300
├─ Run → Attack: Is Attacking
└─ Attack → Idle: Animation Complete
```

#### 动画变量

```
Variables:
├─ Move Speed (float) - 从角色获取
├─ Is Attacking (bool) - 攻击状态
├─ Threat State (Enum) - 当前威胁阶段
└─ Target Direction (Vector) - 目标方向
```

---

## 行为树和黑板

### 创建行为树

虽然魔像主要使用 `UHorrorGolemBehaviorComponent`，但可以结合行为树实现更复杂的AI。

#### 步骤1：创建黑板

1. 内容浏览器右键 → **Artificial Intelligence** → **Blackboard**
2. 命名：`BB_GolemThreat`
3. 保存到 `Content/Blueprints/AI/Blackboard/`

#### 步骤2：配置黑板键

```
Blackboard Keys:
├─ TargetActor (Object, Actor) - 目标玩家
├─ CurrentState (Enum, EGolemEncounterState) - 当前状态
├─ PatrolLocation (Vector) - 巡逻位置
├─ LastKnownLocation (Vector) - 最后已知位置
├─ DistanceToTarget (Float) - 到目标距离
├─ CanSeeTarget (Bool) - 是否看到目标
└─ IsChasing (Bool) - 是否追逐中
```

#### 步骤3：创建行为树

1. 内容浏览器右键 → **Artificial Intelligence** → **Behavior Tree**
2. 命名：`BT_GolemThreat`
3. 设置黑板：`BB_GolemThreat`

#### 步骤4：构建行为树结构

```
Root
└─ Selector: Main Behavior
    ├─ Sequence: Handle Final Impact
    │   ├─ Decorator: Blackboard Based Condition
    │   │   └─ Current State == FinalImpact
    │   ├─ Task: Play Attack Animation
    │   └─ Task: Trigger Death Sequence
    │
    ├─ Sequence: Full Chase
    │   ├─ Decorator: Blackboard Based Condition
    │   │   └─ Current State == FullChase
    │   ├─ Task: Move To Target
    │   └─ Task: Check Destruction
    │
    ├─ Sequence: Close Stalking
    │   ├─ Decorator: Blackboard Based Condition
    │   │   └─ Current State == CloseStalking
    │   ├─ Task: Patrol Around Target
    │   └─ Task: Random Pause
    │
    └─ Sequence: Distant Sighting
        ├─ Decorator: Blackboard Based Condition
        │   └─ Current State == DistantSighting
        └─ Task: Face Target
```

### 自定义行为树任务

#### 创建巡逻任务

1. 创建蓝图类，父类选择 `BTTask_BlueprintBase`
2. 命名：`BTTask_PatrolAroundTarget`

```
Event Receive Execute AI
├─ Input: Owner Controller, Controlled Pawn
├─ Get Blackboard
│   └─ Get Value as Actor: "TargetActor"
├─ Calculate Patrol Point
│   ├─ Get Random Point in Navigable Radius
│   │   ├─ Origin: Target Location
│   │   ├─ Radius: 1200.0
│   │   └─ Nav Agent: Controlled Pawn
│   └─ Set Blackboard Value: "PatrolLocation"
├─ AI Move To
│   ├─ Pawn: Controlled Pawn
│   ├─ Destination: Patrol Location
│   └─ Acceptance Radius: 100.0
└─ Finish Execute (Success)
```

#### 创建检测任务

```
BTTask_UpdateTargetDetection
├─ Event Receive Execute AI
│   ├─ Get Controlled Pawn
│   ├─ Get Target Actor (from Blackboard)
│   ├─ Line Trace to Target
│   │   ├─ Start: Pawn Eye Location
│   │   ├─ End: Target Location
│   │   └─ Trace Channel: Visibility
│   ├─ Set Blackboard Value: "CanSeeTarget" = !Blocked
│   ├─ Calculate Distance
│   │   └─ Set Blackboard Value: "DistanceToTarget"
│   └─ Finish Execute (Success)
```

### AI控制器配置

创建 `BP_GolemAIController`：

1. 父类选择 `HorrorThreatAIController`
2. 配置行为树

```
Class Defaults:
├─ Behavior Tree Asset: BT_GolemThreat
└─ Auto Possess AI: Placed in World or Spawned

AI Perception:
├─ Sight Config:
│   ├─ Sight Radius: 3000.0
│   ├─ Lose Sight Radius: 3500.0
│   ├─ Peripheral Vision Angle: 90.0
│   └─ Detection by Affiliation: Detect Neutrals
└─ Hearing Config:
    ├─ Hearing Range: 2000.0
    └─ Detection by Affiliation: Detect Neutrals
```

---

## 巡逻和追逐逻辑

### 巡逻系统

#### 巡逻点设置

在关卡中放置巡逻点：

1. 创建 `BP_PatrolPoint` Actor
2. 放置多个巡逻点
3. 设置巡逻顺序

```
BP_PatrolPoint:
├─ Variable: Patrol Index (int)
├─ Variable: Wait Time (float) = 2.0
├─ Variable: Next Patrol Point (BP_PatrolPoint Reference)
└─ Component: Billboard (编辑器可见)
```

#### 巡逻逻辑实现

在魔像蓝图中：

```
Function: Start Patrol
├─ Get All Patrol Points
│   └─ Get All Actors of Class: BP_PatrolPoint
├─ Sort by Patrol Index
├─ Set Current Patrol Index: 0
└─ Move to Next Patrol Point

Function: Move to Next Patrol Point
├─ Get Current Patrol Point
├─ AI Move To
│   ├─ Destination: Patrol Point Location
│   ├─ Acceptance Radius: 100.0
│   └─ On Success:
│       ├─ Wait (Patrol Point.Wait Time)
│       ├─ Increment Patrol Index
│       └─ Move to Next Patrol Point (循环)
```

### 追逐系统

#### 追逐触发条件

```
Function: Check Chase Trigger
├─ Get Distance to Target
├─ Get Line of Sight to Target
├─ Branch (Distance < Chase Trigger Distance AND Has LOS)
│   └─ True:
│       ├─ Get Golem Behavior Component
│       │   └─ Force State Transition: ChaseTriggered
│       └─ Start Chase
```

#### 追逐逻辑

```
Function: Update Chase
├─ Get Target Actor
├─ Branch (Target Valid)
│   ├─ True:
│   │   ├─ Get Distance to Target
│   │   ├─ Update Chase Speed (based on distance)
│   │   ├─ AI Move To
│   │   │   ├─ Destination: Target Location
│   │   │   ├─ Acceptance Radius: 50.0
│   │   │   └─ Move Speed: Current Chase Speed
│   │   └─ Check Final Impact Distance
│   └─ False:
│       └─ Lost Target Logic
```

#### 失去目标处理

```
Function: Handle Lost Target
├─ Increment Lost Target Timer
├─ Branch (Timer > Grace Time)
│   ├─ True:
│   │   ├─ Stop Chase
│   │   ├─ Return to Last Known Location
│   │   └─ Transition to Stalking State
│   └─ False:
│       └─ Continue Chase to Last Known Location
```

### 导航和寻路

#### 使用导航系统

```
Function: Move To Location with Nav
├─ Get Nav System
├─ Find Path to Location Synchronously
│   ├─ Start: Self Location
│   ├─ End: Target Location
│   └─ Nav Agent: Self
├─ Branch (Path Valid)
│   ├─ True:
│   │   └─ AI Move To
│   │       ├─ Destination: Target Location
│   │       └─ Use Pathfinding: true
│   └─ False:
│       └─ Move Directly (ignore obstacles)
```

#### 动态障碍物处理

```
Event On Move Completed
├─ Branch (Result == Failed)
│   └─ True:
│       ├─ Check for Destructible Obstacle
│       ├─ Branch (Can Destroy)
│       │   ├─ True:
│       │   │   ├─ Destroy Obstacle
│       │   │   └─ Retry Move
│       │   └─ False:
│       │       └─ Find Alternative Path
```

---

## 与EncounterDirector集成

### 创建遭遇导演蓝图

#### 步骤1：创建蓝图

1. 父类选择 `HorrorEncounterDirector`
2. 命名：`BP_EncounterDirector_Golem`
3. 保存到 `Content/Blueprints/Encounters/`

#### 步骤2：配置遭遇属性

```
Horror | Encounter:
├─ Default Encounter Id: "Encounter.Golem.Hallway01"
├─ Threat Class: BP_GolemThreat
├─ Threat Relative Transform:
│   ├─ Location: (500, 0, 0) (相对导演位置)
│   └─ Rotation: (0, 180, 0) (面向玩家)
├─ Reveal Radius: 1200.0 (触发半径)
└─ Gate Route During Reveal: true (遭遇期间封锁路线)

Horror | Encounter | Audio:
├─ Prime Sound: SFX_Encounter_Prime
├─ Reveal Sound: SFX_Encounter_Reveal
└─ Resolve Sound: SFX_Encounter_Resolve

Horror | Encounter | Camera:
├─ Reveal Camera Shake: CS_EncounterReveal
└─ Reveal Camera Shake Scale: 1.0

Horror | Encounter | Timing:
└─ Reveal Delay Seconds: 0.5 (延迟触发)

Horror | Encounter | EventBus:
├─ Publish to Event Bus: true
└─ Event Bus Source Id: "EncounterDirector.Golem"
```

#### 步骤3：放置到关卡

1. 将 `BP_EncounterDirector_Golem` 拖入关卡
2. 调整位置（魔像生成位置）
3. 配置触发条件

### 遭遇流程

#### 阶段1：准备（Prime）

```
Function: Setup Encounter
├─ Get Encounter Director
│   └─ Prime Encounter
│       └─ Encounter Id: "Encounter.Golem.Hallway01"
├─ 设置触发器
└─ 等待玩家进入
```

#### 阶段2：触发（Reveal）

```
Event On Player Enter Trigger
├─ Get Encounter Director
│   └─ Trigger Reveal
│       └─ Player Actor: Triggering Player
├─ 播放揭示音效和相机抖动
└─ 生成威胁Actor
```

在导演蓝图中实现：

```
Event BP_OnEncounterRevealed
├─ Input: Revealed Encounter Id, Player Actor, Revealed Threat
├─ Play Reveal Sequence
│   ├─ Play Sound: Reveal Sound
│   ├─ Trigger Camera Shake
│   └─ Spawn Particle Effect
├─ Activate Threat
│   └─ Revealed Threat → Activate Threat
├─ Activate Golem Behavior
│   └─ Activate Golem Behavior
│       └─ Target Actor: Player Actor
└─ Gate Route (if enabled)
    └─ Close Doors / Spawn Barriers
```

#### 阶段3：遭遇进行

魔像行为组件自动管理状态转换：

```
Golem Behavior Component (自动):
├─ 监控玩家距离
├─ 状态转换逻辑
├─ 移动和追逐
└─ 触发最终冲击
```

#### 阶段4：解决（Resolve）

```
Function: Resolve Encounter
├─ 条件检查:
│   ├─ 玩家逃脱成功
│   ├─ 玩家到达安全区
│   └─ 或触发特殊事件
├─ Get Encounter Director
│   └─ Resolve Encounter
├─ Deactivate Golem Behavior
└─ Ungate Route
```

在导演蓝图中：

```
Event BP_OnEncounterResolved
├─ Input: Resolved Encounter Id
├─ Deactivate Threat
│   └─ Get Threat Actor → Deactivate Threat
├─ Play Resolve Effects
│   └─ Play Sound: Resolve Sound
├─ Open Route
│   └─ Open Doors / Remove Barriers
└─ Publish Event
    └─ Event Tag: Gameplay.Encounter.Resolved
```

### 遭遇事件监听

在其他系统中监听遭遇事件：

```
Event BeginPlay (关卡蓝图)
└─ Get Encounter Director
    └─ Bind Event to On Encounter Phase Changed
        └─ Custom Event: Handle Encounter Phase Changed
            ├─ Switch on New Phase
            │   ├─ Primed: 准备UI提示
            │   ├─ Revealed: 更新任务目标
            │   ├─ Resolved: 奖励玩家
            │   └─ Dormant: 重置状态
```

### 多遭遇管理

对于多个遭遇导演：

```
BP_EncounterManager (关卡蓝图或GameMode)
├─ Variable: Encounter Directors (Array)
├─ Variable: Active Encounter (Reference)
│
Function: Register Encounter Director
├─ Add to Array: Encounter Directors
└─ Bind Events

Function: Activate Next Encounter
├─ Get Next Director from Array
├─ Prime Encounter
└─ Set as Active Encounter

Event On Encounter Resolved
├─ Clear Active Encounter
├─ Check for Next Encounter
└─ Activate Next Encounter (if exists)
```

---

## AI调试和优化

### 调试工具

#### 启用AI调试显示

在魔像蓝图中：

```
Class Defaults:
└─ Horror | Golem | Debug:
    └─ Draw Debug Info: true
```

这会显示：
- 当前状态（颜色编码）
- 到目标距离
- 距离阈值圆圈
- 移动路径

#### 使用Gameplay Debugger

在编辑器中按 `'` (单引号) 键打开Gameplay Debugger：

```
Gameplay Debugger Categories:
├─ NavMesh (导航网格)
├─ AI (AI状态)
├─ Perception (感知系统)
└─ Behavior Tree (行为树)
```

#### 自定义调试命令

创建控制台命令：

```
Function: Debug Command - Show AI State
├─ Get All Actors of Class: BP_GolemThreat
├─ For Each Actor:
│   ├─ Get Golem Behavior Component
│   ├─ Get Current State
│   └─ Print String: "Golem {Name}: {State}"
```

注册命令：

```
Event BeginPlay
└─ Register Console Command
    ├─ Command: "ShowAIState"
    └─ Callback: Debug Command - Show AI State
```

### 性能优化

#### 减少Tick频率

```
Event BeginPlay
└─ Set Component Tick Interval
    ├─ Component: Golem Behavior Component
    └─ Tick Interval: 0.1 (每0.1秒Tick一次)
```

#### 距离检查优化

```
Function: Should Update AI (优化)
├─ Get Distance to Player (Squared)
├─ Branch (Distance Squared > Max Distance Squared)
│   ├─ True:
│   │   └─ Set Tick Enabled: false (太远，停止Tick)
│   └─ False:
│       └─ Set Tick Enabled: true
```

#### 使用LOD系统

```
Function: Update AI LOD
├─ Get Distance to Player
├─ Switch on Distance Range
│   ├─ < 1000: Full AI (所有功能)
│   ├─ 1000-3000: Medium AI (简化行为)
│   └─ > 3000: Low AI (仅基础更新)
```

#### 异步寻路

```
Function: Find Path Async
├─ AI Move To (Async)
│   ├─ Destination: Target Location
│   ├─ Acceptance Radius: 100.0
│   └─ On Success / Fail:
│       └─ Handle Move Result
```

### 测试清单

- [ ] 所有遭遇阶段正常转换
- [ ] 距离阈值准确
- [ ] 导航和寻路无卡顿
- [ ] 音效和动画同步
- [ ] 相机抖动效果合理
- [ ] 性能在目标范围内（60 FPS）
- [ ] 多次遭遇可重置
- [ ] 保存/加载状态正确
- [ ] 调试信息清晰
- [ ] 边界情况处理（玩家死亡、传送等）

---

## 高级技巧

### 动态难度调整

```
Function: Adjust AI Difficulty
├─ Get Player Performance Metrics
│   ├─ Death Count
│   ├─ Time in Level
│   └─ Fear Level
├─ Calculate Difficulty Multiplier
└─ Apply to AI Parameters
    ├─ Chase Speed *= Multiplier
    ├─ Detection Radius *= Multiplier
    └─ Reaction Time /= Multiplier
```

### AI协作

多个威胁协同工作：

```
Function: Coordinate AI Threats
├─ Get All Active Threats
├─ Assign Roles:
│   ├─ Threat 1: Direct Chase
│   ├─ Threat 2: Flank Left
│   └─ Threat 3: Block Escape
└─ Synchronize Attacks
```

### 预测玩家移动

```
Function: Predict Player Location
├─ Get Player Velocity
├─ Calculate Future Position
│   └─ Future Pos = Current Pos + (Velocity * Prediction Time)
└─ Move to Predicted Position
```

---

## 相关文档

- [BlueprintArchitecture.md](BlueprintArchitecture.md) - 蓝图架构基础
- [InteractionBlueprints.md](InteractionBlueprints.md) - AI与交互系统集成
- [../AI/BehaviorTrees/BehaviorTreeSetup.md](../AI/BehaviorTrees/BehaviorTreeSetup.md) - 行为树详细指南
- [../Architecture/SystemOverview.md](../Architecture/SystemOverview.md) - AI系统架构概览

---

**版本**: 1.0
**最后更新**: 2026-04-28
**维护者**: HorrorProject Team
