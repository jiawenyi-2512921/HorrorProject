# 深水站 - 关卡设计指南

## 📋 目录
1. [恐怖游戏关卡设计原则](#原则)
2. [你的可用系统](#系统)
3. [关卡设计工作流程](#工作流程)
4. [具体操作步骤](#步骤)
5. [Python辅助工具](#工具)

---

## 🎯 恐怖游戏关卡设计原则

### 核心理念：控制节奏
恐怖游戏的关键是**紧张-放松-高潮**的循环：

```
安全区 → 紧张区 → 危险区 → 逃脱 → 安全区
  ↓         ↓         ↓        ↓        ↓
 探索     音效暗示   遭遇敌人   追逐    喘息
```

### 三大支柱

#### 1. 环境叙事
- **视觉线索**：血迹、损坏的设备、散落的物品
- **音频线索**：远处的脚步声、机械噪音、水滴声
- **光影对比**：明暗交替，制造不确定感

#### 2. 玩家引导
- **灯光引导**：用光源指引方向
- **声音引导**：用音效吸引注意
- **空间设计**：窄通道→开阔空间→窄通道

#### 3. 威胁管理
- **隐藏威胁**：让玩家感觉"有东西在那里"
- **假威胁**：制造虚惊（风吹动的门、掉落的物品）
- **真威胁**：实际的敌人遭遇

---

## 🛠️ 你的可用系统

### C++组件（已实现）

#### 玩家系统
- `FlashlightComponent` - 手电筒（电池系统）
- `InteractionComponent` - 交互检测
- `FearComponent` - 恐惧值系统
- `QuantumCameraComponent` - 量子相机（观察改变现实？）
- `CameraRecordingComponent` - 录制功能
- `VHSEffectComponent` - VHS视觉效果
- `InventoryComponent` - 物品栏

#### AI系统
- `HorrorThreatAIController` - 敌人AI控制器
- `HorrorGolemBehaviorComponent` - 石头魔像行为
- `HorrorEncounterDirector` - 遭遇导演（动态生成威胁）

#### 环境系统
- `HorrorAudioZoneActor` - 音频区域
- `HorrorAudioZoneTrigger` - 音频触发器
- `BaseInteractable` - 可交互物体基类
- `EvidenceActor` - 证据物品
- `HorrorObjectiveNode` - 目标节点

#### 视觉效果
- `PostProcessController` - 后处理控制
- `VHSVisualEffectComponent` - VHS视觉效果
- `NoiseOverlayComponent` - 噪点叠加
- `ScanlineComponent` - 扫描线效果

### 资产包

#### 环境资产
1. **DeepWaterStation** - 主场景（工业/水下站）
2. **Industrial_Architecture** - 工业建筑模块
3. **Fantastic_Dungeon_Pack** - 地牢模块化资产
4. **Scrapopolis** - 废弃城市
5. **ForestOfSpikes** - 森林环境
6. **IndustrialPipesM** - 工业管道

#### 音频资产（SoundsOfHorror）
- **Atmosphere** - 环境氛围音
- **Tension** - 紧张音乐
- **BuildUps** - 渐强音效
- **Jumpscares** - 跳杀音效
- **Impacts** - 撞击声
- **Clues** - 线索提示音
- **Rumbles** - 隆隆声

#### 敌人
- **Stone_Golem** - 石头魔像（带动画）

#### 特效
- **Bodycam_VHS_Effect** - 摄像机效果包

---

## 🔄 关卡设计工作流程

### 阶段1：规划（纸面设计）
```
1. 画出关卡平面图
2. 标记关键区域：
   - 起点/终点
   - 安全区
   - 危险区
   - 目标位置
   - 敌人巡逻路径
3. 设计玩家路线
4. 规划节奏点
```

### 阶段2：白盒搭建（Blockout）
```
使用 LevelPrototyping 资产快速搭建：
1. 用简单几何体搭建空间布局
2. 测试玩家移动流畅度
3. 调整房间大小和连接
4. 放置临时灯光
```

### 阶段3：系统集成
```
1. 放置交互物体
2. 配置音频区域
3. 设置AI巡逻路径
4. 添加目标节点
5. 配置遭遇导演
```

### 阶段4：美术替换
```
1. 用真实资产替换白盒
2. 添加细节装饰
3. 优化光照
4. 添加粒子效果
```

### 阶段5：打磨
```
1. 音效细化
2. 后处理调整
3. 性能优化
4. 测试迭代
```

---

## 📝 具体操作步骤

### 步骤1：打开现有地图
```
1. 启动UE编辑器
2. 打开：Content/DeepWaterStation/Maps/DemoMap_VerticalSlice_Day1
3. 查看当前布局
```

### 步骤2：创建第一个房间

#### 使用白盒资产
```
1. 在Content Browser搜索：LevelPrototyping
2. 拖拽这些Mesh到场景：
   - SM_ChamferedCube (墙壁)
   - SM_Cube (地板/天花板)
3. 使用 W/E/R 键切换移动/旋转/缩放
4. 按住 Alt + 拖拽 = 复制物体
```

#### 快捷键
- `W` - 移动工具
- `E` - 旋转工具
- `R` - 缩放工具
- `F` - 聚焦选中物体
- `Ctrl+D` - 复制
- `End` - 物体吸附到地面

### 步骤3：添加灯光

#### 点光源（Point Light）
```
1. 在Place Actors面板搜索：Point Light
2. 拖入场景
3. 调整属性：
   - Intensity: 1000-5000（根据房间大小）
   - Attenuation Radius: 500-2000
   - Light Color: 偏冷色调（恐怖氛围）
```

#### 聚光灯（Spot Light）
```
用于：
- 手电筒效果
- 走廊照明
- 重点照明
```

### 步骤4：添加音频区域

#### 使用你的HorrorAudioZoneActor
```
1. 在Content Browser搜索：BP_HorrorAudioZone（如果有）
   或者在C++类中找到 HorrorAudioZoneActor
2. 拖入场景
3. 调整Box Component大小覆盖区域
4. 在Details面板配置：
   - 选择音效：Content/SoundsOfHorror/Atmosphere/
   - 设置音量、淡入淡出时间
```

### 步骤5：放置交互物体

#### 证据物品
```
1. 创建蓝图继承自 EvidenceActor
2. 设置Static Mesh（文档、照片等）
3. 配置交互提示文本
4. 放置在关卡中
```

#### 门
```
1. 使用 LevelPrototyping/Interactable/Door
2. 或创建蓝图继承 BaseInteractable
3. 添加开门动画
```

### 步骤6：配置AI敌人

#### 放置石头魔像
```
1. 找到 Stone_Golem 蓝图
2. 拖入场景
3. 添加 HorrorThreatAIController
4. 创建Nav Mesh Bounds Volume（导航网格）：
   - 在Place Actors搜索：Nav Mesh Bounds Volume
   - 调整大小覆盖可行走区域
   - 按 P 键显示绿色导航网格
```

#### 设置巡逻路径
```
1. 放置 Target Point 作为巡逻点
2. 在AI蓝图中引用这些点
3. 或使用 Spline Component 创建路径
```

### 步骤7：配置玩家起点
```
1. 找到 Player Start
2. 移动到起始位置
3. 调整旋转方向（红色箭头指向玩家初始朝向）
```

### 步骤8：测试
```
1. 点击工具栏的 Play 按钮（或按 Alt+P）
2. 测试：
   - 移动是否流畅
   - 灯光是否合适
   - 音效是否触发
   - AI是否正常巡逻
3. 按 Esc 退出测试
```

---

## 🐍 Python辅助工具

### 我可以为你创建的脚本

#### 1. 批量放置灯光
```python
# 自动在走廊每隔X米放置一盏灯
# 自动调整灯光强度和颜色
```

#### 2. 音频区域生成器
```python
# 根据房间大小自动创建音频区域
# 批量配置音效
```

#### 3. 导航网格检查器
```python
# 检查哪些区域没有导航网格
# 自动生成Nav Mesh Bounds
```

#### 4. 资产替换工具
```python
# 批量替换白盒为真实资产
# 保持位置和旋转
```

---

## 🎨 恐怖氛围营造技巧

### 灯光技巧
1. **对比度**：明暗对比强烈
2. **色温**：冷色调（蓝、绿）更恐怖
3. **闪烁**：偶尔的灯光闪烁
4. **阴影**：长阴影制造不确定感

### 音效技巧
1. **分层**：环境音 + 随机音效 + 音乐
2. **方向性**：3D音效引导注意力
3. **静默**：突然的安静比噪音更可怕
4. **假象**：远处的脚步声（但没有敌人）

### 空间设计
1. **狭窄通道**：增加压迫感
2. **开阔空间**：暴露感、脆弱感
3. **多路径**：让玩家做选择（增加焦虑）
4. **死胡同**：制造陷阱感

---

## 🚦 第一个关卡检查清单

### 必须有的元素
- [ ] 玩家起点（Player Start）
- [ ] 至少一个目标（证据/出口）
- [ ] 导航网格（Nav Mesh）
- [ ] 基础灯光
- [ ] 环境音效
- [ ] 至少一个交互物体
- [ ] 测试通过（能从头走到尾）

### 推荐添加
- [ ] 至少一个敌人
- [ ] 音频区域（3-5个）
- [ ] 假威胁（1-2个）
- [ ] 手电筒必需区域（黑暗区）
- [ ] 证据收集点（2-3个）
- [ ] VHS效果配置

---

## 📞 需要帮助时

### 我可以帮你：
1. **写Python脚本**自动化重复工作
2. **创建蓝图逻辑**（我提供代码/逻辑，你在UE中实现）
3. **分析问题**（截图给我，我帮你诊断）
4. **优化性能**（检查资产使用）
5. **设计关卡流程**（提供详细方案）

### 你需要做的：
1. 在UE编辑器中实际操作
2. 测试效果
3. 给我反馈
4. 提供截图/日志（如果遇到问题）

---

## 🎯 下一步行动

### 立即开始（5分钟）
1. 打开 DemoMap_VerticalSlice_Day1
2. 截图给我看当前状态
3. 告诉我你想做什么类型的场景

### 我会帮你：
1. 分析现有布局
2. 提供具体改进建议
3. 创建Python工具辅助你
4. 一步步指导你完成第一个房间

**准备好了吗？打开地图，截图给我，我们开始！** 🚀
