# 视觉效果蓝图指南

## 概述

本文档详细说明如何在HorrorProject中配置和使用视觉效果蓝图，包括VHS效果、后处理体积、相机抖动和粒子效果。

## 目录

- [VHS效果配置](#vhs效果配置)
- [后处理体积设置](#后处理体积设置)
- [相机抖动和效果](#相机抖动和效果)
- [粒子系统](#粒子系统)
- [材质效果](#材质效果)
- [性能优化](#性能优化)

---

## VHS效果配置

### VHSEffectComponent

`UVHSEffectComponent` 提供复古VHS录像带视觉效果，自动添加到 `AHorrorPlayerCharacter`。

### 配置VHS效果

#### 在玩家角色蓝图中

打开 `BP_HorrorPlayerCharacter`，选择 `VHSEffect` 组件：

```
VHS Effect Component:
├─ VHS Effect Intensity: 0.5 (总体强度 0-1)
├─ Chromatic Aberration: 0.3 (色差强度)
├─ Noise Intensity: 0.2 (噪点强度)
├─ Scan Lines Enabled: true (扫描线)
├─ Scan Line Intensity: 0.15
├─ Scan Line Count: 240 (扫描线数量)
├─ Vignette Intensity: 0.4 (暗角强度)
├─ Vignette Smoothness: 0.5
├─ Distortion Amount: 0.1 (画面扭曲)
├─ Flicker Enabled: true (闪烁效果)
├─ Flicker Frequency: 0.5 (闪烁频率)
└─ Color Grading:
    ├─ Saturation: 0.8 (饱和度降低)
    ├─ Contrast: 1.1 (对比度提高)
    └─ Color Tint: (R=1.0, G=0.95, B=0.9) (暖色调)
```

### 动态调整VHS效果

#### 基于恐惧值调整

在玩家角色蓝图中：

```
Event Tick
├─ Get Fear Component
│   └─ Get Current Fear (0-100)
├─ Calculate VHS Intensity
│   └─ Intensity = Fear / 100.0
├─ Get VHS Effect Component
│   └─ Set VHS Effect Intensity
│       └─ New Intensity: Calculated Intensity
```

![VHS强度随恐惧值变化截图占位符]

#### 基于事件触发

```
Custom Event: Trigger VHS Glitch
├─ Get VHS Effect Component
├─ Store Original Intensity
├─ Set VHS Effect Intensity: 1.0 (最大)
├─ Set Distortion Amount: 0.5
├─ Delay 0.2
└─ Restore Original Intensity
```

### VHS效果材质

创建后处理材质 `M_VHSEffect`：

#### 材质设置

1. 创建 **Material**，命名 `M_VHSEffect`
2. 设置 **Material Domain**: Post Process
3. 设置 **Blendable Location**: After Tonemapping

#### 材质节点结构

```
Material Graph:
├─ Scene Texture: PostProcessInput0 (原始画面)
├─ Chromatic Aberration:
│   ├─ UV Offset (R通道): +0.002
│   ├─ UV Offset (G通道): 0.0
│   └─ UV Offset (B通道): -0.002
├─ Scan Lines:
│   ├─ UV.Y * Scan Line Count
│   ├─ Frac
│   ├─ Step (0.5)
│   └─ Lerp with Original (Intensity)
├─ Noise:
│   ├─ Noise Texture (Tiling)
│   ├─ Time-based UV Offset
│   └─ Multiply by Noise Intensity
├─ Vignette:
│   ├─ Distance from Center
│   ├─ Power (Smoothness)
│   └─ Multiply with Color
└─ Final Output:
    └─ Emissive Color
```

![VHS材质节点图截图占位符]

#### 材质参数

```
Material Parameters (Scalar):
├─ VHSIntensity (0.5)
├─ ChromaticAberration (0.003)
├─ NoiseIntensity (0.2)
├─ ScanLineIntensity (0.15)
├─ ScanLineCount (240)
├─ VignetteIntensity (0.4)
├─ VignetteSmoothness (0.5)
└─ DistortionAmount (0.1)

Material Parameters (Vector):
└─ ColorTint (1.0, 0.95, 0.9, 1.0)
```

### 创建VHS材质实例

1. 右键 `M_VHSEffect` → **Create Material Instance**
2. 命名：`MI_VHSEffect_Default`
3. 在蓝图中应用：

```
Event BeginPlay
├─ Get Player Camera Manager
│   └─ Add or Update Blendable
│       ├─ Material: MI_VHSEffect_Default
│       └─ Weight: 1.0
```

---

## 后处理体积设置

### 创建后处理体积

#### 全局后处理

1. 在关卡中放置 **Post Process Volume**
2. 勾选 **Infinite Extent (Unbound)** (全局生效)
3. 配置效果

```
Post Process Volume Settings:
├─ Priority: 1.0 (优先级)
├─ Blend Weight: 1.0
└─ Infinite Extent: true
```

#### 局部后处理

用于特定区域的效果：

```
Post Process Volume (局部):
├─ Infinite Extent: false
├─ Brush Shape: Box
├─ Brush Extent: (500, 500, 300)
└─ Blend Radius: 100.0 (过渡半径)
```

### 恐怖氛围后处理

#### 基础设置

```
Lens:
├─ Bloom:
│   ├─ Intensity: 0.5
│   ├─ Threshold: 1.0
│   └─ Size Scale: 4.0
├─ Lens Flares:
│   ├─ Intensity: 0.3
│   └─ Tint: (R=1.0, G=0.9, B=0.8)
├─ Vignette:
│   ├─ Intensity: 0.6 (强暗角)
│   └─ Smoothness: 0.3
└─ Chromatic Aberration:
    └─ Intensity: 0.2

Color Grading:
├─ Saturation: 0.7 (降低饱和度)
├─ Contrast: 1.2 (提高对比度)
├─ Gamma: 0.9 (稍微变暗)
├─ Gain: 1.0
└─ Shadows:
    ├─ Saturation: 0.5 (阴影去色)
    └─ Tint: (R=0.8, G=0.85, B=1.0) (冷色调)

Film:
├─ Grain Intensity: 0.3 (胶片颗粒)
├─ Grain Jitter: 0.5
└─ Toe: 0.5 (暗部细节)

Motion Blur:
├─ Amount: 0.3
└─ Max: 0.5
```

### 动态后处理

#### 基于玩家状态

创建 `BP_DynamicPostProcess` Actor：

```
BP_DynamicPostProcess:
├─ Component: Post Process Component
├─ Variable: Target Vignette Intensity (float)
├─ Variable: Current Vignette Intensity (float)
│
Event Tick:
├─ Get Player Character
│   └─ Get Fear Component
│       └─ Get Current Fear
├─ Calculate Target Vignette
│   └─ Target = 0.4 + (Fear / 100.0) * 0.4
├─ Lerp Current to Target
│   └─ Current = Lerp(Current, Target, DeltaTime * 2.0)
└─ Update Post Process Settings
    └─ Set Vignette Intensity: Current
```

#### 基于区域

不同区域不同效果：

```
BP_AreaPostProcess:
├─ Enum: Area Type (Safe, Tense, Danger, Horror)
├─ Map: Area Type → Post Process Settings
│
Function: Apply Area Settings
├─ Switch on Area Type
│   ├─ Safe:
│   │   ├─ Saturation: 1.0
│   │   ├─ Vignette: 0.2
│   │   └─ Grain: 0.1
│   ├─ Tense:
│   │   ├─ Saturation: 0.8
│   │   ├─ Vignette: 0.4
│   │   └─ Grain: 0.2
│   ├─ Danger:
│   │   ├─ Saturation: 0.6
│   │   ├─ Vignette: 0.6
│   │   └─ Grain: 0.3
│   └─ Horror:
│       ├─ Saturation: 0.4
│       ├─ Vignette: 0.8
│       └─ Grain: 0.5
```

---

## 相机抖动和效果

### 创建相机抖动

#### 基础相机抖动

1. 创建蓝图类，父类选择 `CameraShakeBase`
2. 命名：`BP_CameraShake_Footstep`
3. 保存到 `Content/Blueprints/Camera/Shakes/`

#### 配置抖动参数

使用 `MatineeCameraShake` 类型：

```
BP_CameraShake_Footstep:
├─ Oscillation Duration: 0.2 (持续时间)
├─ Blend In Time: 0.05
├─ Blend Out Time: 0.1
│
├─ Rotation Oscillation:
│   ├─ Pitch:
│   │   ├─ Amplitude: 0.5 (幅度)
│   │   └─ Frequency: 10.0 (频率)
│   ├─ Yaw:
│   │   ├─ Amplitude: 0.3
│   │   └─ Frequency: 8.0
│   └─ Roll:
│       ├─ Amplitude: 0.2
│       └─ Frequency: 12.0
│
└─ Location Oscillation:
    ├─ X (前后):
    │   ├─ Amplitude: 0.5
    │   └─ Frequency: 10.0
    ├─ Y (左右):
    │   ├─ Amplitude: 0.3
    │   └─ Frequency: 8.0
    └─ Z (上下):
        ├─ Amplitude: 1.0
        └─ Frequency: 10.0
```

### 预设相机抖动

#### 脚步抖动

```
BP_CameraShake_Walk:
├─ Duration: 0.15
├─ Location Z Amplitude: 0.5
└─ Rotation Pitch Amplitude: 0.3

BP_CameraShake_Run:
├─ Duration: 0.12
├─ Location Z Amplitude: 1.0
└─ Rotation Pitch Amplitude: 0.6
```

#### 交互抖动

```
BP_CameraShake_Interact:
├─ Duration: 0.3
├─ Location Y Amplitude: 1.0
└─ Rotation Yaw Amplitude: 0.5
```

#### 遭遇抖动

```
BP_CameraShake_EncounterReveal:
├─ Duration: 1.5
├─ Location Amplitude: (2.0, 2.0, 3.0)
├─ Rotation Amplitude: (1.5, 1.0, 0.5)
└─ FOV Oscillation:
    ├─ Amplitude: 5.0
    └─ Frequency: 3.0

BP_CameraShake_GolemFootstep:
├─ Duration: 0.5
├─ Location Z Amplitude: 5.0 (强烈震动)
├─ Rotation Pitch Amplitude: 2.0
└─ Attenuation:
    ├─ Inner Radius: 500.0
    └─ Outer Radius: 2000.0 (距离衰减)
```

### 触发相机抖动

#### 在蓝图中触发

```
Function: Play Camera Shake
├─ Get Player Camera Manager
│   └─ Start Camera Shake
│       ├─ Shake Class: BP_CameraShake_Footstep
│       ├─ Scale: 1.0 (强度缩放)
│       ├─ Play Space: Camera Local
│       ├─ User Play Space Rot: (0, 0, 0)
│       └─ Source: Self (可选，用于距离衰减)
```

#### 基于距离的抖动

```
Function: Play Distance-Based Shake
├─ Get Player Character
├─ Calculate Distance to Player
├─ Calculate Shake Scale
│   └─ Scale = 1.0 - (Distance / Max Distance)
├─ Branch (Scale > 0)
│   └─ True:
│       └─ Start Camera Shake
│           ├─ Shake Class: BP_CameraShake_GolemFootstep
│           └─ Scale: Calculated Scale
```

### 相机效果组件

创建自定义相机效果组件：

```
BP_CameraEffectComponent (ActorComponent):
├─ Variable: Shake Intensity Multiplier (float) = 1.0
├─ Variable: Enabled (bool) = true
│
Function: Play Footstep Shake
├─ Branch (Enabled)
│   └─ True:
│       └─ Start Camera Shake
│           ├─ Shake Class: BP_CameraShake_Footstep
│           └─ Scale: Shake Intensity Multiplier

Function: Play Impact Shake
├─ Input: Impact Strength (float)
├─ Select Shake Class based on Strength
│   ├─ < 0.3: BP_CameraShake_Light
│   ├─ 0.3-0.7: BP_CameraShake_Medium
│   └─ > 0.7: BP_CameraShake_Heavy
└─ Start Camera Shake
```

---

## 粒子系统

### 交互粒子效果

#### 证据收集效果

创建 `PS_EvidenceCollected`：

```
Particle System:
├─ Emitter: Sparkles
│   ├─ Spawn Rate: 50
│   ├─ Lifetime: 1.0
│   ├─ Initial Size: (5, 10)
│   ├─ Initial Velocity: Cone (Angle=30)
│   ├─ Color Over Life: White → Transparent
│   └─ Material: M_Sparkle
│
└─ Emitter: Glow
    ├─ Spawn: Burst (Count=1)
    ├─ Lifetime: 0.5
    ├─ Initial Size: 50 → 100 (Size Over Life)
    ├─ Color: Gold
    └─ Material: M_Glow_Soft
```

#### 门开启效果

```
PS_DoorOpen:
├─ Emitter: Dust
│   ├─ Spawn Rate: 20
│   ├─ Lifetime: 2.0
│   ├─ Initial Velocity: Random in Cone
│   ├─ Drag: 0.5
│   └─ Material: M_Dust
│
└─ Emitter: Light Rays
    ├─ Spawn: Burst (Count=5)
    ├─ Lifetime: 1.0
    ├─ Initial Size: (10, 200)
    └─ Material: M_LightRay
```

### 恐怖氛围粒子

#### 环境雾气

```
PS_AmbientFog:
├─ Emitter: Fog Volumes
│   ├─ Spawn Rate: 2
│   ├─ Lifetime: 10.0
│   ├─ Initial Size: (200, 300)
│   ├─ Initial Velocity: (10, 0, 5)
│   ├─ Color: (0.8, 0.85, 0.9, 0.3)
│   └─ Material: M_Fog_Volumetric
│
└─ Local Vector Field (可选)
    └─ 控制雾气流动方向
```

#### 灰尘粒子

```
PS_DustMotes:
├─ Emitter: Dust Particles
│   ├─ Spawn Rate: 10
│   ├─ Lifetime: 5.0
│   ├─ Initial Size: (1, 3)
│   ├─ Initial Velocity: (0, 0, 5) + Random
│   ├─ Orbit: Enabled (飘动效果)
│   └─ Material: M_DustMote
```

### 魔像粒子效果

#### 魔像光环

```
PS_GolemAura:
├─ Emitter: Energy Particles
│   ├─ Spawn Rate: 30
│   ├─ Lifetime: 2.0
│   ├─ Initial Location: Sphere Surface
│   ├─ Initial Velocity: Outward from Center
│   ├─ Color: Dark Red → Black
│   └─ Material: M_EnergyParticle
│
└─ Emitter: Distortion
    ├─ Spawn Rate: 5
    ├─ Lifetime: 1.0
    ├─ Initial Size: 100
    └─ Material: M_HeatDistortion
```

#### 脚步冲击

```
PS_GolemFootstep:
├─ Emitter: Ground Impact
│   ├─ Spawn: Burst (Count=20)
│   ├─ Lifetime: 0.5
│   ├─ Initial Velocity: Radial Outward
│   ├─ Size Over Life: 5 → 20
│   └─ Material: M_GroundDebris
│
├─ Emitter: Shockwave
│   ├─ Spawn: Burst (Count=1)
│   ├─ Lifetime: 0.3
│   ├─ Size Over Life: 0 → 200
│   └─ Material: M_Shockwave
│
└─ Emitter: Dust Cloud
    ├─ Spawn: Burst (Count=10)
    ├─ Lifetime: 1.0
    └─ Material: M_Dust
```

### 在蓝图中生成粒子

```
Function: Spawn Particle Effect
├─ Spawn Emitter at Location
│   ├─ Emitter Template: PS_EvidenceCollected
│   ├─ Location: Target Location
│   ├─ Rotation: (0, 0, 0)
│   ├─ Scale: (1, 1, 1)
│   └─ Auto Destroy: true
└─ Return: Particle System Component
```

#### 附加到Actor

```
Function: Attach Particle to Actor
├─ Spawn Emitter Attached
│   ├─ Emitter Template: PS_GolemAura
│   ├─ Attach to Component: Mesh
│   ├─ Attach Point Name: "spine_03"
│   ├─ Location: (0, 0, 0)
│   └─ Attach Type: Snap to Target
└─ Store Reference for later control
```

---

## 材质效果

### 发光材质

创建交互物体高亮材质：

```
M_InteractHighlight:
├─ Base Color: Object Color
├─ Emissive:
│   ├─ Fresnel (Exponent=3.0)
│   ├─ Multiply by Highlight Color
│   └─ Multiply by Intensity Parameter
└─ Opacity: 0.8 (半透明)
```

#### 动态高亮

```
Material Function: Pulsing Glow
├─ Time * Pulse Speed
├─ Sin
├─ Remap (0-1)
└─ Multiply with Emissive Color
```

### 溶解效果

用于物体消失：

```
M_Dissolve:
├─ Base Color: Object Texture
├─ Opacity Mask:
│   ├─ Noise Texture
│   ├─ Add Dissolve Progress Parameter
│   └─ Step (Threshold)
├─ Emissive:
│   ├─ Edge Detection (Opacity Mask边缘)
│   └─ Multiply by Glow Color
└─ Material Domain: Surface
```

在蓝图中使用：

```
Function: Dissolve Actor
├─ Create Dynamic Material Instance
│   └─ Parent: M_Dissolve
├─ Timeline: Dissolve Progress (0 → 1, Duration=2.0)
│   └─ Update:
│       └─ Set Scalar Parameter: "DissolveProgress"
└─ On Finished:
    └─ Destroy Actor
```

### 扫描线材质

```
M_ScanLines:
├─ UV.Y * Line Count
├─ Frac
├─ Step (0.5)
├─ Lerp with Original Color
└─ Output to Emissive (Post Process)
```

---

## 性能优化

### 粒子系统优化

#### LOD设置

```
Particle System LOD:
├─ LOD 0 (近距离):
│   ├─ Spawn Rate: 50
│   └─ Max Particles: 500
├─ LOD 1 (中距离):
│   ├─ Spawn Rate: 25
│   └─ Max Particles: 250
└─ LOD 2 (远距离):
    ├─ Spawn Rate: 10
    └─ Max Particles: 100
```

#### 距离剔除

```
Particle System Settings:
└─ Bounds:
    ├─ Fixed Bounds: true
    └─ Bounds: (200, 200, 200)
└─ LOD:
    └─ LOD Distance Check Time: 0.25 (每0.25秒检查一次)
```

### 后处理优化

#### 条件启用

```
Function: Update Post Process
├─ Get Distance to Player
├─ Branch (Distance < Activation Distance)
│   ├─ True: Enable Post Process
│   └─ False: Disable Post Process
```

#### 降低更新频率

```
Event BeginPlay
└─ Set Timer by Function Name
    ├─ Function: "Update Post Process"
    ├─ Time: 0.1 (每0.1秒更新)
    └─ Looping: true
```

### 材质优化

#### 使用材质实例

```
// 运行时修改参数
❌ 创建新材质
✅ 使用Material Instance Dynamic

Function: Update Material Parameter
├─ Get Material Instance Dynamic
│   └─ Set Scalar Parameter Value
│       ├─ Parameter Name: "Intensity"
│       └─ Value: New Intensity
```

#### 简化材质复杂度

- 减少纹理采样次数
- 避免复杂数学运算
- 使用预计算的查找表（LUT）
- 合并相似材质

### 性能分析

#### 使用Profiler

```
控制台命令:
├─ stat fps (显示帧率)
├─ stat unit (显示各系统耗时)
├─ stat particles (粒子统计)
└─ profilegpu (GPU性能分析)
```

#### 优化目标

```
性能目标:
├─ 帧率: 60 FPS (PC), 30 FPS (Console)
├─ 粒子总数: < 5000
├─ 后处理材质: < 5个同时激活
└─ 相机抖动: < 3个同时播放
```

---

## 测试清单

- [ ] VHS效果在不同光照下正常显示
- [ ] 后处理体积过渡平滑
- [ ] 相机抖动强度合理，不引起晕眩
- [ ] 粒子效果在各LOD级别正常
- [ ] 材质效果无闪烁或错误
- [ ] 性能达标（60 FPS）
- [ ] 多个效果叠加无冲突
- [ ] 保存/加载后效果保持
- [ ] 不同分辨率下效果一致
- [ ] 色盲模式兼容

---

## 相关文档

- [BlueprintArchitecture.md](BlueprintArchitecture.md) - 蓝图架构基础
- [../VFX/Niagara/NiagaraBestPractices.md](../VFX/Niagara/NiagaraBestPractices.md) - 粒子系统详细指南
- [../Materials/MaterialLibrary.md](../Materials/MaterialLibrary.md) - 材质系统指南
- [../Performance/RenderingOptimization.md](../Performance/RenderingOptimization.md) - 性能优化指南

---

**版本**: 1.0
**最后更新**: 2026-04-28
**维护者**: HorrorProject Team
