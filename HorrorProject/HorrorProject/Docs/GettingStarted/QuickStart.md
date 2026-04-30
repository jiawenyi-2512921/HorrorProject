# HorrorProject 快速开始指南

## 🎯 目标
- **5分钟** - 创建第一个恐怖房间
- **10分钟** - 添加AI敌人
- **15分钟** - 完成音频配置
- **30分钟** - 完成可玩原型

---

## ⚡ 5分钟：创建第一个房间

### 步骤1：新建关卡（1分钟）
1. 打开UE编辑器
2. `File → New Level → Empty Level`
3. 保存为 `Content/Maps/MyFirstHorrorRoom`

### 步骤2：添加基础照明（1分钟）
1. 拖入 `Directional Light`（模拟月光）
   - Intensity: `0.5`
   - Color: 淡蓝色 `(0.7, 0.8, 1.0)`
2. 拖入 `Sky Light`
   - Intensity: `0.3`
3. 拖入 `Exponential Height Fog`
   - Fog Density: `0.02`

### 步骤3：搭建房间（3分钟）
使用 **Industrial_Architecture** 资产包：

```
Content/SD_Art/Industrial_Architecture/Meshes/
```

快速房间模板：
- 地板：`SM_Floor_4x4m` × 4块
- 墙壁：`SM_Wall_4x3m` × 4面
- 天花板：`SM_Ceiling_4x4m` × 4块
- 门：`SM_Door_Single`

**快捷键提示**：
- `Alt + 拖动` = 复制
- `End` = 对齐到地面
- `Ctrl + L` = 锁定选中物体

### 步骤4：添加玩家出生点
1. 搜索 `Player Start`
2. 拖入场景
3. 按 `Alt + P` 测试游戏

✅ **检查点**：你应该能在黑暗房间中生成并移动

---

## 🤖 10分钟：添加AI敌人

### 步骤1：放置石头魔像（2分钟）
1. 导航到：
```
Content/StoneGolem/Blueprints/
```
2. 拖入 `BP_StoneGolem` 到房间角落
3. 设置初始状态：
   - AI Behavior: `Patrol`
   - Detection Range: `1500`

### 步骤2：配置AI巡逻路径（3分钟）
1. 搜索 `Nav Mesh Bounds Volume`
2. 拖入场景，缩放覆盖整个房间
3. 按 `P` 键查看导航网格（绿色区域）

创建巡逻点：
1. 搜索 `Target Point`
2. 在房间中放置3-4个巡逻点
3. 选中 `BP_StoneGolem`
4. Details面板 → `Patrol Points` → 添加巡逻点引用

### 步骤3：测试AI行为（2分钟）
1. 按 `Alt + P` 开始游戏
2. 观察魔像巡逻
3. 靠近测试检测范围

### 步骤4：调整AI参数（3分钟）
在 `BP_StoneGolem` Details面板：
```
Movement Speed: 300 (巡逻) / 600 (追逐)
Detection Range: 1500
Hearing Range: 2000
Attack Range: 200
```

✅ **检查点**：魔像应该在巡逻点间移动，发现你时追逐

---

## 🔊 15分钟：完成音频配置

### 步骤1：添加环境音效（5分钟）

#### 创建音频区域
1. 搜索 `HorrorAudioZoneActor`
2. 拖入房间中心
3. 配置参数：
```
Zone Type: Interior
Ambient Sound: SoundsOfHorror/Atmosphere/Amb_Industrial_Loop
Volume: 0.5
Fade Distance: 500
```

#### 添加点音源
在房间角落放置：
- 水滴声：`SoundsOfHorror/Atmosphere/Amb_WaterDrip`
- 机械噪音：`SoundsOfHorror/Atmosphere/Amb_Machinery_Hum`

### 步骤2：配置紧张音乐（5分钟）

#### 创建音乐触发器
1. 搜索 `Box Trigger`
2. 放置在房间入口
3. 添加蓝图逻辑：

```blueprint
Event BeginOverlap
  → Get Audio Subsystem
  → Play Music Track
     Track: SoundsOfHorror/Tension/Mus_Tension_Low
     Fade In: 2.0s
```

#### AI检测音乐
在 `BP_StoneGolem` 事件图表：
```blueprint
On Player Detected
  → Get Audio Subsystem
  → Play Music Track
     Track: SoundsOfHorror/Tension/Mus_Chase
     Fade In: 0.5s
```

### 步骤3：添加脚步声（5分钟）
1. 打开 `BP_HorrorPlayerCharacter`
2. 找到 `FootstepAnimNotify`
3. 确认已配置：
```
Surface Type: Concrete → Sound: Footstep_Concrete
Surface Type: Metal → Sound: Footstep_Metal
Surface Type: Water → Sound: Footstep_Water
```

✅ **检查点**：你应该听到环境音、紧张音乐和脚步声

---

## 🎮 30分钟：完成可玩原型

### 步骤1：添加交互物品（5分钟）

#### 放置证据物品
1. 搜索 `BP_EvidenceActor`
2. 拖入3个到房间不同位置
3. 配置每个证据：
```
Evidence ID: "evidence_note_01"
Display Name: "血迹笔记"
Description: "一张沾满血迹的纸条..."
```

#### 添加门锁机制
1. 在门上添加 `BP_InteractableDoor`
2. 设置：
```
Requires Key: true
Key ID: "rusty_key"
Locked Message: "需要生锈的钥匙"
```

3. 放置钥匙：
```
BP_EvidenceActor
Evidence ID: "rusty_key"
Display Name: "生锈的钥匙"
```

### 步骤2：配置手电筒（5分钟）
1. 打开 `BP_HorrorPlayerCharacter`
2. 找到 `FlashlightComponent`
3. 调整参数：
```
Initial Battery: 100%
Drain Rate: 5% per minute
Light Intensity: 5000
Light Radius: 2000
```

测试：
- 按 `F` 开关手电筒
- 观察电池消耗

### 步骤3：添加恐惧系统（5分钟）
1. 在 `BP_HorrorPlayerCharacter` 中启用 `FearComponent`
2. 配置触发器：

在房间黑暗区域放置 `Box Trigger`：
```blueprint
Event BeginOverlap
  → Get Fear Component
  → Increase Fear
     Amount: 10
     Duration: 5.0s
```

在魔像附近：
```blueprint
On Player Detected (BP_StoneGolem)
  → Get Fear Component
  → Increase Fear
     Amount: 30
     Duration: 10.0s
```

### 步骤4：添加VHS视觉效果（5分钟）
1. 打开 `BP_HorrorPlayerCharacter`
2. 添加 `VHSEffectComponent`
3. 配置：
```
Noise Intensity: 0.3
Scanline Speed: 2.0
Chromatic Aberration: 0.5
Vignette Intensity: 0.4
```

链接到恐惧值：
```blueprint
Fear Component → On Fear Changed
  → VHS Effect Component → Set Intensity
     Intensity = Fear Value / 100
```

### 步骤5：设置目标系统（5分钟）
1. 搜索 `BP_ObjectiveNode`
2. 创建目标链：

**目标1：探索房间**
```
Objective ID: "explore_room"
Description: "探索这个废弃的房间"
Auto Complete: On Enter Zone
```

**目标2：收集证据**
```
Objective ID: "collect_evidence"
Description: "收集3个证据"
Required Count: 3
```

**目标3：找到钥匙**
```
Objective ID: "find_key"
Description: "找到出口的钥匙"
```

**目标4：逃离**
```
Objective ID: "escape"
Description: "打开门逃离"
```

### 步骤6：最终测试（5分钟）
完整游玩流程：
1. ✅ 生成在黑暗房间
2. ✅ 打开手电筒探索
3. ✅ 听到环境音和脚步声
4. ✅ 发现巡逻的魔像
5. ✅ 收集证据和钥匙
6. ✅ 被追逐时恐惧值上升
7. ✅ 打开门逃离

---

## 🚀 下一步

### 进阶教程
- [CheatSheet.md](CheatSheet.md) - 快捷键和常用操作
- [VideoTutorialScript.md](VideoTutorialScript.md) - 详细视频教程脚本
- [FAQ.md](../FAQ.md) - 常见问题解答

### 深入学习
- [LevelDesignGuide.md](../LevelDesignGuide.md) - 完整关卡设计指南
- [AI系统文档](../AI/) - AI行为树和感知系统
- [音频系统文档](../Audio/) - 高级音频配置

### Python自动化
- 使用 `CodexCreateVerticalSliceMap.py` 自动生成关卡
- 使用 `CodexAuditVerticalSliceMap.py` 检查关卡质量

---

## 💡 快速技巧

### 性能优化
- 使用 `stat fps` 查看帧率
- 使用 `stat unit` 查看性能瓶颈
- 限制同时活跃的AI数量（推荐≤3）

### 调试技巧
- `~` 打开控制台
- `ghost` - 穿墙模式
- `slomo 0.5` - 慢动作（调试AI）
- `show collision` - 显示碰撞体

### 快速迭代
1. 使用 `Alt + P` 快速测试（不保存）
2. 使用 `Simulate` 模式观察AI行为
3. 使用 `Blueprint Debugger` 调试逻辑

---

**预计完成时间**：30分钟
**难度**：初学者
**前置要求**：UE5基础操作

**下一步**：查看 [CheatSheet.md](CheatSheet.md) 学习更多快捷操作
