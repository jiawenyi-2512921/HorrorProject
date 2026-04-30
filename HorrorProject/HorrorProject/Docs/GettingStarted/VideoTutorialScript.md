# HorrorProject 视频教程脚本

## 📹 教程1：从零开始创建恐怖房间（10分钟）

### 开场（30秒）
```
[画面：UE5启动画面]
大家好，欢迎来到HorrorProject教程系列。
今天我们将在10分钟内从零开始创建一个完整的恐怖房间。

你将学会：
✓ 搭建基础场景
✓ 配置照明和氛围
✓ 添加音效
✓ 放置交互物品

让我们开始吧！
```

### 第1步：创建新关卡（1分钟）
```
[画面：UE编辑器主界面]
首先，点击 File → New Level → Empty Level
这会给我们一个完全空白的画布。

[操作：创建关卡]
保存为 Content/Maps/Tutorial_HorrorRoom

[提示框显示]
💡 快捷键：Ctrl + S 快速保存
```

### 第2步：添加基础照明（1分钟30秒）
```
[画面：空白关卡]
恐怖游戏的关键是光影对比。
我们需要三种光源：

1. Directional Light - 模拟月光
[操作：从Place Actors拖入]
   设置 Intensity: 0.5
   颜色调整为淡蓝色 (0.7, 0.8, 1.0)
   这会营造夜晚的感觉

2. Sky Light - 环境光
[操作：拖入Sky Light]
   Intensity: 0.3
   保持低强度，制造压抑感

3. Exponential Height Fog - 体积雾
[操作：拖入雾效]
   Fog Density: 0.02
   这会增加神秘感和深度

[画面：对比 - 添加光照前后]
看到区别了吗？现在有了基础氛围。
```

### 第3步：搭建房间结构（3分钟）
```
[画面：Content Browser]
导航到 Content/SD_Art/Industrial_Architecture/Meshes/

我们将使用工业风格资产包。
这是一个模块化系统，可以快速搭建场景。

[操作：拖入地板]
首先是地板：SM_Floor_4x4m
按住 Alt 键拖动可以快速复制
[演示：创建 2x2 的地板网格]

[操作：添加墙壁]
接下来是墙壁：SM_Wall_4x3m
[演示：围绕地板放置4面墙]

💡 快捷键提示：
- Alt + 拖动 = 复制
- End 键 = 对齐到地面
- W/E/R = 移动/旋转/缩放工具

[操作：添加天花板]
天花板：SM_Ceiling_4x4m
[演示：覆盖顶部]

[操作：添加门]
最后添加门：SM_Door_Single
[演示：在墙上开洞并放置门]

[画面：完成的房间结构]
基础结构完成！现在我们有了一个封闭的工业风房间。
```

### 第4步：添加细节和道具（2分钟）
```
[画面：房间内部]
现在让我们添加一些细节，让房间更有故事感。

[操作：添加管道]
从 IndustrialPipesM 包中添加管道
[演示：沿墙壁放置管道]

[操作：添加箱子和杂物]
从 Scrapopolis 包中添加废弃物品
[演示：随机放置箱子、桶等]

[操作：添加血迹贴花]
搜索 "Decal"
[演示：在地板和墙上添加血迹贴花]

💡 环境叙事技巧：
- 不要摆放得太整齐
- 制造"有人匆忙离开"的感觉
- 使用光影引导玩家视线
```

### 第5步：配置音频（1分钟30秒）
```
[画面：搜索 HorrorAudioZoneActor]
现在添加音效，让房间"活"起来。

[操作：拖入音频区域]
放置在房间中心
配置：
- Zone Type: Interior
- Ambient Sound: Amb_Industrial_Loop
- Volume: 0.5

[操作：添加点音源]
在角落添加水滴声：Amb_WaterDrip
在管道旁添加机械声：Amb_Machinery_Hum

[演示：播放预览]
听到了吗？现在有了环境音效。
```

### 第6步：添加玩家出生点和测试（30秒）
```
[操作：搜索 Player Start]
拖入房间中央

[按下 Alt + P]
让我们测试一下！

[画面：第一人称视角游玩]
完美！我们现在可以：
✓ 在房间中移动
✓ 听到环境音
✓ 看到氛围光照

[按 Esc 退出]
```

### 结尾（30秒）
```
[画面：完成的房间俯视图]
恭喜！你已经创建了第一个恐怖房间。

在下一个教程中，我们将：
- 添加AI敌人
- 配置巡逻路径
- 实现追逐机制

记得订阅频道，我们下期见！

[字幕：相关文档]
📄 QuickStart.md - 完整文字教程
📄 CheatSheet.md - 快捷键速查表
```

---

## 📹 教程2：添加AI敌人和巡逻系统（15分钟）

### 开场（30秒）
```
[画面：上一教程完成的房间]
欢迎回来！上次我们创建了一个恐怖房间。
今天我们将添加真正的威胁 - AI敌人。

你将学会：
✓ 放置石头魔像
✓ 配置导航网格
✓ 设置巡逻路径
✓ 调整AI行为参数

让我们开始！
```

### 第1步：了解AI系统（2分钟）
```
[画面：架构图]
HorrorProject的AI系统包含三个核心部分：

1. AI Controller - 大脑
   负责决策和行为切换

2. Behavior Tree - 行为树
   定义AI的行为逻辑

3. Perception System - 感知系统
   视觉、听觉、伤害感知

[画面：状态机图]
AI有三种状态：
- Patrol（巡逻）- 默认状态
- Chase（追逐）- 发现玩家
- Attack（攻击）- 接近玩家

现在让我们实际操作。
```

### 第2步：放置石头魔像（2分钟）
```
[画面：Content Browser]
导航到 Content/StoneGolem/Blueprints/

[操作：拖入 BP_StoneGolem]
将魔像放置在房间角落

[画面：Details面板]
让我们看看可配置的参数：

Movement（移动）：
- Patrol Speed: 300 - 巡逻速度
- Chase Speed: 600 - 追逐速度

Detection（检测）：
- Sight Range: 1500 - 视觉范围
- Hearing Range: 2000 - 听觉范围
- Peripheral Vision: 90° - 周边视野

Behavior（行为）：
- Initial State: Patrol - 初始状态
- Attack Range: 200 - 攻击范围
- Attack Damage: 25 - 攻击伤害

[提示框]
💡 先使用默认值，稍后我们会调整
```

### 第3步：配置导航网格（3分钟）
```
[画面：视口]
AI需要导航网格才能移动。

[操作：搜索 Nav Mesh Bounds Volume]
拖入场景

[操作：缩放体积]
按 R 键切换到缩放工具
拉伸覆盖整个房间
确保包含所有可行走区域

[按 P 键]
这会显示导航网格
[画面：绿色网格覆盖地板]

绿色区域 = AI可以行走
红色区域 = 被阻挡

[演示：调整障碍物]
如果有红色区域，可能需要：
- 移动障碍物
- 调整碰撞设置
- 扩大导航体积

[操作：重新构建导航]
Build → Build Paths

[画面：完整的绿色导航网格]
完美！现在AI可以在整个房间移动。
```

### 第4步：创建巡逻路径（3分钟）
```
[画面：俯视图]
现在设置巡逻路径。

[操作：搜索 Target Point]
这些是巡逻点标记

[演示：放置巡逻点]
在房间中放置4个巡逻点：
1. 房间左上角
2. 房间右上角
3. 房间右下角
4. 房间左下角

[提示框]
💡 巡逻点技巧：
- 形成闭环路径
- 避免直线路径（太机械）
- 在关键位置放置（门口、拐角）

[操作：选中 BP_StoneGolem]
在 Details 面板找到 Patrol Points 数组

[演示：添加巡逻点引用]
点击 + 号添加元素
从场景中选择每个 Target Point
按顺序添加

[画面：显示巡逻路径连线]
现在魔像会按这个路径巡逻。
```

### 第5步：测试和调整AI行为（4分钟）
```
[按 Alt + P 开始测试]

[画面：第一人称视角]
让我们观察AI行为：

[演示：魔像巡逻]
看，魔像正在巡逻点之间移动
移动速度平稳，转向自然

[演示：靠近魔像]
当我们进入检测范围...
[画面：魔像转向玩家]
它发现我们了！

[演示：追逐场景]
魔像开始追逐
速度明显加快
始终面向玩家

[演示：逃离]
当我们跑远...
[画面：魔像返回巡逻]
它会返回最后的巡逻点

[按 Esc 退出]

现在让我们调整参数，让AI更有挑战性。

[画面：BP_StoneGolem Details]
调整这些参数：

Detection Range: 1500 → 2000
（更容易发现玩家）

Chase Speed: 600 → 700
（追得更快）

Peripheral Vision: 90° → 120°
（视野更广）

[再次测试]
[画面：对比前后差异]
现在AI更具威胁性了！
```

### 第6步：添加AI音效（2分钟）
```
[画面：BP_StoneGolem 事件图表]
让我们添加音效反馈。

[操作：找到 On Player Detected 事件]
这个事件在发现玩家时触发

[演示：添加节点]
On Player Detected
  → Get Audio Subsystem
  → Play Music Track
     Track: Mus_Chase
     Fade In: 0.5s

[操作：找到 On Lost Player 事件]
On Lost Player
  → Get Audio Subsystem
  → Stop Music Track
     Fade Out: 2.0s

[测试]
[画面：被发现时音乐响起]
完美！现在有音频反馈了。
```

### 结尾（30秒）
```
[画面：完整的AI系统运行]
太棒了！现在你有了：
✓ 会巡逻的AI敌人
✓ 智能的追逐系统
✓ 音效反馈

下一个教程：
- 添加多个AI
- 配置AI协作
- 实现隐藏和躲避机制

感谢观看，下期见！
```

---

## 📹 教程3：完整游戏循环 - 从进入到逃离（20分钟）

### 开场（1分钟）
```
[画面：完整游戏演示]
欢迎来到最终教程！
今天我们将把所有元素整合成一个完整的游戏循环。

玩家体验流程：
1. 进入黑暗房间
2. 打开手电筒探索
3. 收集证据和钥匙
4. 躲避巡逻的敌人
5. 打开门逃离

让我们开始制作！
```

### 第1步：配置玩家系统（3分钟）
```
[画面：BP_HorrorPlayerCharacter]
打开玩家角色蓝图

[操作：配置手电筒]
找到 FlashlightComponent
设置：
- Initial Battery: 100%
- Drain Rate: 5% per minute
- Light Intensity: 5000
- Light Radius: 2000
- Toggle Key: F

[演示：测试手电筒]
[画面：按F开关手电筒]
[画面：电池UI显示]

[操作：配置恐惧系统]
找到 FearComponent
设置：
- Max Fear: 100
- Fear Decay Rate: 5 per second
- Fear Effects Threshold: 50

[演示：恐惧效果]
[画面：恐惧值上升时的视觉效果]
- 屏幕边缘暗化
- 心跳声加快
- 视野轻微晃动
```

### 第2步：创建目标系统（4分钟）
```
[画面：Content Browser]
搜索 BP_ObjectiveNode

我们需要创建4个目标：

[操作：目标1 - 探索]
Objective ID: "explore_room"
Description: "探索废弃的房间"
Type: Location Based
Trigger: Box Trigger（覆盖房间中心）

[演示：放置触发器]
[画面：进入触发器时目标完成]

[操作：目标2 - 收集证据]
Objective ID: "collect_evidence"
Description: "收集3个证据"
Type: Collection
Required Count: 3

[演示：放置3个 BP_EvidenceActor]
Evidence 1: "血迹笔记"
Evidence 2: "损坏的录音机"
Evidence 3: "神秘照片"

[操作：目标3 - 找到钥匙]
Objective ID: "find_key"
Description: "找到出口的钥匙"
Type: Item Collection

[演示：放置钥匙]
BP_EvidenceActor
Evidence ID: "rusty_key"
Display Name: "生锈的钥匙"

[操作：目标4 - 逃离]
Objective ID: "escape"
Description: "打开门逃离"
Type: Interaction

[演示：配置门]
BP_InteractableDoor
Requires Key: true
Key ID: "rusty_key"
```

### 第3步：设计恐怖遭遇（5分钟）
```
[画面：关卡俯视图]
现在设计恐怖时刻。

[操作：遭遇1 - 初次惊吓]
在玩家进入房间后触发

[演示：创建触发器]
Box Trigger → Event BeginOverlap
  → Play Sound: SFX_Jumpscare_01
  → Spawn Particle Effect
  → Camera Shake

[测试]
[画面：玩家进入，突然的音效和视觉效果]

[操作：遭遇2 - 魔像苏醒]
当玩家收集第一个证据时

[演示：蓝图逻辑]
On Evidence Collected
  → If (Evidence Count == 1)
    → BP_StoneGolem → Activate
    → Play Music: Mus_Tension_Medium
    → Show Message: "你感觉到有什么东西醒了..."

[操作：遭遇3 - 灯光闪烁]
随机触发，增加紧张感

[演示：创建 BP_LightFlicker]
Timer: Random (30-60 seconds)
  → Flicker All Lights
  → Play Sound: SFX_ElectricalBuzz
  → Increase Fear: 10

[操作：遭遇4 - 最终追逐]
当玩家拿到钥匙时

[演示：蓝图逻辑]
On Key Collected
  → BP_StoneGolem → Set Chase Speed: 800
  → Play Music: Mus_Chase
  → Show Message: "快跑！"
```

### 第4步：配置音频层次（3分钟）
```
[画面：音频系统架构]
恐怖游戏需要多层音频：

Layer 1: 环境音（持续）
[操作：HorrorAudioZoneActor]
- Amb_Industrial_Loop
- Volume: 0.3
- Always Playing

Layer 2: 紧张音乐（动态）
[演示：蓝图逻辑]
根据情况切换：
- 安全：Mus_Tension_Low
- 警觉：Mus_Tension_Medium
- 追逐：Mus_Chase

Layer 3: 点音源（空间）
[操作：放置音源]
- 水滴声（角落）
- 机械声（管道）
- 风声（通风口）

Layer 4: 事件音效（触发）
- 脚步声
- 门声
- 跳杀音效

[演示：所有层次同时播放]
[画面：音频可视化]
```

### 第5步：优化和润色（3分钟）
```
[画面：性能统计]
按 ` 键打开控制台
输入：stat fps

[检查性能]
目标：60 FPS

[操作：优化检查清单]
✓ 光照：使用光照贴图
✓ 阴影：限制动态阴影数量
✓ AI：最多3个同时活跃
✓ 音频：使用衰减设置
✓ 粒子：限制发射数量

[操作：添加润色]
1. 后处理体积
   - Vignette: 0.4
   - Film Grain: 0.3
   - Chromatic Aberration: 0.5

2. 相机抖动
   - 行走：轻微抖动
   - 奔跑：明显抖动
   - 恐惧：随机抖动

3. VHS效果
   - 噪点
   - 扫描线
   - 色差
```

### 第6步：完整测试（1分钟）
```
[画面：完整游玩流程]
让我们完整体验一遍：

[0:00] 生成在黑暗房间
[0:05] 打开手电筒
[0:10] 探索，触发初次惊吓
[0:30] 收集第一个证据
[0:35] 魔像苏醒，音乐变化
[1:00] 躲避魔像，收集剩余证据
[1:30] 找到钥匙
[1:35] 最终追逐开始
[2:00] 打开门逃离

[画面：胜利画面]
完美！
```

### 结尾（1分钟）
```
[画面：完成的项目]
恭喜！你已经创建了一个完整的恐怖游戏原型。

你学会了：
✓ 关卡设计
✓ AI系统
✓ 音频配置
✓ 目标系统
✓ 玩家机制
✓ 恐怖氛围营造

下一步：
- 添加更多房间
- 创建不同类型的敌人
- 实现存档系统
- 添加多人模式

相关资源：
📄 完整文档：Docs/
🐍 Python工具：Scripts/
💬 社区支持：Discord

感谢观看整个系列！
记得分享你的作品！

[字幕：制作人员名单]
```

---

## 📝 录制注意事项

### 技术要求
- **分辨率**：1920x1080 @ 60fps
- **音频**：清晰的麦克风，无背景噪音
- **录制软件**：OBS Studio / Camtasia
- **编辑软件**：DaVinci Resolve / Premiere Pro

### 录制技巧
1. **准备脚本**：提前写好每句话
2. **分段录制**：每个步骤单独录制
3. **多次尝试**：选择最好的版本
4. **添加字幕**：方便非母语观众
5. **使用标注**：箭头、高亮、文字提示

### 后期制作
- 添加背景音乐（低音量）
- 添加转场效果
- 添加片头片尾
- 添加时间戳（YouTube章节）
- 添加相关链接（描述栏）

### 发布清单
- [ ] 标题优化（SEO）
- [ ] 缩略图设计
- [ ] 描述包含关键词
- [ ] 添加标签
- [ ] 添加到播放列表
- [ ] 社交媒体宣传

---

**预计制作时间**：
- 录制：6小时
- 编辑：8小时
- 总计：14小时（3个视频）

**目标观众**：UE初学者，恐怖游戏开发者

**相关文档**：
- [QuickStart.md](QuickStart.md)
- [CheatSheet.md](CheatSheet.md)
- [FAQ.md](../FAQ.md)
