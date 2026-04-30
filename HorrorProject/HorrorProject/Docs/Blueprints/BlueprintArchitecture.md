# 蓝图架构指南

## 概述

本文档详细说明如何在HorrorProject中使用蓝图继承C++类，配置玩家角色，以及实现组件组合模式。

## 目录

- [C++类继承蓝图](#c类继承蓝图)
- [BP_HorrorPlayerCharacter配置](#bp_horrorplayercharacter配置)
- [组件组合模式](#组件组合模式)
- [最佳实践](#最佳实践)

---

## C++类继承蓝图

### 基本原则

HorrorProject采用"C++为基础，蓝图为扩展"的架构模式：

- **C++层**：定义核心逻辑、性能关键代码、网络复制
- **蓝图层**：配置参数、视觉效果、关卡特定逻辑、快速迭代

### 创建蓝图子类

#### 步骤1：选择父类

在内容浏览器中：

1. 右键 → **Blueprint Class**
2. 选择 **All Classes** 展开完整类列表
3. 搜索目标C++类（例如：`HorrorPlayerCharacter`）
4. 选择并创建

![创建蓝图类截图占位符]

#### 步骤2：命名规范

遵循项目命名约定：

```
BP_<ClassName>
例如：
- BP_HorrorPlayerCharacter
- BP_DoorInteractable
- BP_GolemThreat
```

#### 步骤3：验证继承

打开蓝图后，在 **Class Settings** 中验证：

- **Parent Class**：显示正确的C++类
- **Interfaces**：自动继承C++接口（如IInteractableInterface）

### 可继承的核心类

| C++类 | 用途 | 蓝图命名示例 |
|-------|------|--------------|
| `AHorrorPlayerCharacter` | 玩家角色 | `BP_HorrorPlayerCharacter` |
| `ABaseInteractable` | 交互物体基类 | `BP_CustomInteractable` |
| `ADoorInteractable` | 门 | `BP_DoorInteractable` |
| `AHorrorThreatCharacter` | 威胁敌人 | `BP_GolemThreat` |
| `AHorrorEncounterDirector` | 遭遇导演 | `BP_EncounterDirector_Level01` |

---

## BP_HorrorPlayerCharacter配置

### 创建玩家角色蓝图

#### 基础设置

1. 创建蓝图类，父类选择 `HorrorPlayerCharacter`
2. 命名为 `BP_HorrorPlayerCharacter`
3. 保存到 `Content/Blueprints/Player/`

#### 组件配置

`AHorrorPlayerCharacter` 自动创建以下组件（C++层）：

```cpp
// 自动创建的组件
- InteractionComponent      // 交互系统
- InventoryComponent         // 物品栏
- VHSEffectComponent         // VHS视觉效果
- NoteRecorderComponent      // 录音笔记
- QuantumCameraComponent     // 量子相机
- FlashlightComponent        // 手电筒
- FearComponent              // 恐惧系统
- NoiseGeneratorComponent    // 噪音生成
- EvidenceCollectionComponent // 证据收集
```

**在蓝图中配置这些组件：**

##### 1. InteractionComponent

```
Details面板：
├─ Interaction Range: 200.0 (交互距离，单位cm)
├─ Trace Channel: ECC_Visibility
└─ Debug Draw: false (发布版本关闭)
```

##### 2. VHSEffectComponent

```
Details面板：
├─ VHS Effect Intensity: 0.5 (效果强度)
├─ Chromatic Aberration: 0.3
├─ Noise Intensity: 0.2
├─ Scan Lines: true
└─ Vignette Intensity: 0.4
```

##### 3. QuantumCameraComponent

```
Details面板：
├─ Camera Mode: Normal (初始模式)
├─ Rewind Duration: 5.0 (回溯时长，秒)
├─ Photo Cooldown: 1.0 (拍照冷却)
└─ Max Photos: 24 (最大照片数)
```

##### 4. FlashlightComponent

```
Details面板：
├─ Light Intensity: 5000.0
├─ Attenuation Radius: 1000.0
├─ Cone Angle: 30.0
├─ Battery Drain Rate: 0.1 (每秒消耗)
└─ Initial Battery: 100.0
```

##### 5. FearComponent

```
Details面板：
├─ Max Fear: 100.0
├─ Fear Decay Rate: 5.0 (每秒恢复)
├─ Fear Threshold High: 75.0 (高恐惧阈值)
└─ Fear Threshold Critical: 90.0 (临界阈值)
```

#### 输入配置

在 **Class Defaults** 中配置输入动作：

```
Input Actions:
├─ Interact Action: IA_Interact
├─ Record Action: IA_Record
├─ Take Photo Action: IA_TakePhoto
├─ Rewind Action: IA_Rewind
├─ Open Archive Action: IA_OpenArchive
└─ Toggle Flashlight Action: IA_ToggleFlashlight
```

**输入动作资产路径：**

```
/Game/Input/Actions/IA_Interact
/Game/Input/Actions/IA_Record
/Game/Input/Actions/IA_TakePhoto
/Game/Input/Actions/IA_Rewind
/Game/Input/Actions/IA_OpenArchive
/Game/Input/Actions/IA_ToggleFlashlight
```

#### 蓝图事件实现

##### BP_OpenArchive事件

在事件图表中实现打开档案UI：

```
Event BP_OpenArchive
├─ Create Widget (Class: WBP_ArchiveMenu)
├─ Add to Viewport
├─ Set Input Mode UI Only
└─ Set Show Mouse Cursor: true
```

![BP_OpenArchive事件图表截图占位符]

##### 自定义交互反馈

重写 `DoInteract` 函数添加自定义逻辑：

```
Function: DoInteract (Override)
├─ Parent: DoInteract (调用父类)
├─ Play Camera Shake (BP_CameraShake_Interact)
└─ Play Sound 2D (SFX_Interact_Generic)
```

#### 相机设置

配置第一人称相机：

```
Camera Component (继承自父类):
├─ Field of View: 90.0
├─ Location: (0, 0, 64) (眼睛高度)
└─ Post Process Settings:
    ├─ Motion Blur Amount: 0.3
    └─ Bloom Intensity: 0.5
```

#### 移动配置

```
Character Movement Component:
├─ Max Walk Speed: 400.0
├─ Max Walk Speed Crouched: 200.0
├─ Jump Z Velocity: 420.0
├─ Air Control: 0.2
└─ Can Crouch: true
```

---

## 组件组合模式

### 设计理念

HorrorProject使用组件化架构，遵循"组合优于继承"原则：

```
Actor (容器)
├─ Component A (功能模块1)
├─ Component B (功能模块2)
└─ Component C (功能模块3)
```

### 组件通信模式

#### 1. 直接引用（推荐用于同一Actor内）

```cpp
// C++层获取组件
UInteractionComponent* Interaction = GetInteractionComponent();
if (Interaction)
{
    Interaction->TryInteract();
}
```

蓝图中：

```
Get Interaction Component
└─ Try Interact
```

#### 2. 事件委托（推荐用于跨Actor通信）

C++定义委托：

```cpp
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInteractionComplete, AActor*, InteractedActor);

UPROPERTY(BlueprintAssignable)
FOnInteractionComplete OnInteractionComplete;
```

蓝图中绑定：

```
Event BeginPlay
└─ Bind Event to On Interaction Complete
    └─ Custom Event: Handle Interaction Complete
```

#### 3. 事件总线（推荐用于全局事件）

使用 `UHorrorEventBusSubsystem`：

```
Get Game Instance
└─ Get Subsystem (HorrorEventBusSubsystem)
    └─ Publish Event
        ├─ Event Tag: Gameplay.Interaction.DoorOpened
        └─ Source Id: Door_01
```

### 创建自定义组件

#### 步骤1：C++组件基类

```cpp
UCLASS(BlueprintType, Blueprintable, ClassGroup=(Horror), meta=(BlueprintSpawnableComponent))
class HORRORPROJECT_API UCustomComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category="Custom")
    void DoSomething();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Custom")
    float SomeParameter = 1.0f;
};
```

#### 步骤2：蓝图中添加组件

1. 打开目标Actor蓝图
2. **Components** 面板 → **Add Component**
3. 搜索 `CustomComponent`
4. 配置参数

#### 步骤3：蓝图中访问组件

```
Get Component by Class (CustomComponent)
└─ Do Something
```

### 组件生命周期

理解组件初始化顺序：

```
1. Constructor (C++)
2. PostInitializeComponents (C++)
3. BeginPlay (C++)
4. Event BeginPlay (Blueprint)
5. Tick / Event Tick
```

**最佳实践：**

- 在 `PostInitializeComponents` 中初始化组件间依赖
- 在 `BeginPlay` 中执行游戏逻辑初始化
- 避免在构造函数中访问其他组件

---

## 最佳实践

### 1. 性能优化

#### 减少Tick开销

```
Event BeginPlay
└─ Set Actor Tick Enabled: false (不需要Tick的Actor)
```

对于组件：

```
Component Details:
└─ Component Tick:
    ├─ Start with Tick Enabled: false
    └─ Tick Interval: 0.1 (降低Tick频率)
```

#### 使用对象池

对于频繁生成的Actor（如粒子、音效）：

```
Function: Spawn Pooled Actor
├─ Get from Pool
├─ If Valid: Reuse
└─ Else: Spawn New
```

### 2. 蓝图组织

#### 函数库分类

创建蓝图函数库：

```
Content/Blueprints/Libraries/
├─ BPL_InteractionHelpers
├─ BPL_AudioHelpers
└─ BPL_VFXHelpers
```

#### 宏使用

对于重复逻辑，创建宏：

```
Macro: Play Interaction Feedback
├─ Input: Interaction Type (Enum)
├─ Play Sound
├─ Play Camera Shake
└─ Spawn Particle
```

### 3. 调试技巧

#### 启用调试绘制

```
Class Defaults:
└─ Debug:
    ├─ Draw Debug Info: true (开发版本)
    └─ Debug Color: (R=1, G=0, B=0)
```

#### 打印调试信息

```
Print String
├─ In String: "Current State: {State}"
├─ Text Color: Yellow
└─ Duration: 2.0
```

#### 使用断点

在蓝图节点上右键 → **Add Breakpoint**

### 4. 版本控制

#### 蓝图差异

- 避免不必要的蓝图重新编译
- 提交前清理未使用的节点
- 使用 **Blueprint Diff** 工具审查变更

#### 资产引用

- 使用软引用（Soft Object Reference）避免加载整个依赖链
- 异步加载大型资产

```
Async Load Asset
├─ Asset Reference: TSoftObjectPtr<UTexture2D>
└─ On Loaded:
    └─ Use Asset
```

### 5. 文档化

#### 添加注释

在蓝图节点上右键 → **Add Comment**

```
Comment: "Phase 1: 检查交互条件"
├─ Can Interact?
├─ Is In Range?
└─ Has Line of Sight?
```

#### 函数描述

在函数详情中填写：

```
Function Details:
├─ Description: "尝试与目标物体交互，返回是否成功"
├─ Category: "Interaction"
└─ Keywords: "interact, use, activate"
```

---

## 常见问题

### Q: 蓝图中看不到C++类？

**A:** 确保：
1. C++类标记为 `Blueprintable`
2. 项目已编译成功
3. 编辑器已重启

### Q: 组件在蓝图中无法访问？

**A:** 检查C++中的访问修饰符：
```cpp
UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
TObjectPtr<UMyComponent> MyComponent;
```

### Q: 如何在蓝图中调用C++函数？

**A:** C++函数需要标记为 `BlueprintCallable`：
```cpp
UFUNCTION(BlueprintCallable, Category="MyCategory")
void MyFunction();
```

### Q: 蓝图性能问题？

**A:** 优化建议：
1. 将性能关键代码移至C++
2. 减少Tick使用，改用Timer
3. 使用Nativize Blueprint（打包时）
4. 避免复杂的数组操作

---

## 相关文档

- [InteractionBlueprints.md](InteractionBlueprints.md) - 交互蓝图详细指南
- [AIBlueprints.md](AIBlueprints.md) - AI蓝图实现
- [VFXBlueprints.md](VFXBlueprints.md) - 视觉效果配置
- [../Architecture/ComponentDiagram.md](../Architecture/ComponentDiagram.md) - 组件架构技术文档

---

**版本**: 1.0
**最后更新**: 2026-04-28
**维护者**: HorrorProject Team
