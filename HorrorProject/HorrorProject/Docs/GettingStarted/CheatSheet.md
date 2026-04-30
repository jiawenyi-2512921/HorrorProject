# HorrorProject 速查表

## 🎮 UE编辑器快捷键

### 视口导航
| 快捷键 | 功能 |
|--------|------|
| `鼠标右键 + WASD` | 飞行模式导航 |
| `鼠标中键拖动` | 平移视图 |
| `Alt + 鼠标左键` | 围绕物体旋转 |
| `Alt + 鼠标右键` | 缩放视图 |
| `F` | 聚焦选中物体 |
| `G` | 切换游戏模式预览 |

### 物体操作
| 快捷键 | 功能 |
|--------|------|
| `W` | 移动工具 |
| `E` | 旋转工具 |
| `R` | 缩放工具 |
| `Space` | 循环切换工具 |
| `Ctrl + D` | 复制选中物体 |
| `Alt + 拖动` | 快速复制 |
| `End` | 对齐到地面 |
| `Ctrl + End` | 对齐到表面 |
| `Shift + E` | 对齐到网格 |

### 选择与编辑
| 快捷键 | 功能 |
|--------|------|
| `Ctrl + A` | 全选 |
| `Ctrl + Shift + A` | 取消选择 |
| `H` | 隐藏选中 |
| `Ctrl + H` | 取消隐藏全部 |
| `Ctrl + L` | 锁定选中 |
| `Ctrl + P` | 解锁全部 |
| `Ctrl + G` | 组合物体 |
| `Ctrl + Shift + G` | 取消组合 |

### 游戏测试
| 快捷键 | 功能 |
|--------|------|
| `Alt + P` | 在当前视口测试 |
| `Ctrl + P` | 在独立窗口测试 |
| `Esc` | 停止游戏 |
| `F8` | 弹出玩家控制器（调试） |
| `Pause` | 暂停游戏 |
| `` ` `` | 打开控制台 |

### 蓝图编辑
| 快捷键 | 功能 |
|--------|------|
| `Ctrl + S` | 保存蓝图 |
| `Ctrl + F` | 搜索节点 |
| `Ctrl + Shift + F` | 全局搜索 |
| `C` | 添加注释框 |
| `Q` | 快速连接（拖线后） |
| `Alt + 点击连线` | 断开连线 |
| `Ctrl + B` | 跳转到定义 |
| `F9` | 添加断点 |

### 内容浏览器
| 快捷键 | 功能 |
|--------|------|
| `Ctrl + Space` | 聚焦搜索框 |
| `Ctrl + B` | 在浏览器中查找 |
| `Ctrl + S` | 保存全部 |
| `Ctrl + Shift + S` | 保存选中 |
| `F2` | 重命名 |
| `Delete` | 删除资产 |

---

## 🐍 Python脚本快速调用

### 关卡流程配置脚本
```python
# 验证关卡流程配置
unreal.PythonScriptLibrary.execute_python_command(
    "exec(open('Scripts/LevelDesign/LevelTransitionSetup.py', encoding='utf-8').read())"
)
```

### 关卡审计脚本
```python
# 检查关卡质量
unreal.PythonScriptLibrary.execute_python_command(
    "exec(open('Scripts/LevelDesign/LevelValidator.py', encoding='utf-8').read())"
)
```

### Day1 密码门验证
```python
# 验证DeepWater站点密码门配置
unreal.PythonScriptLibrary.execute_python_command(
    "exec(open('Scripts/Validation/VerifyDay1PasswordDoors.py', encoding='utf-8').read())"
)
```

### 在编辑器中运行Python
1. `Window → Developer Tools → Output Log`
2. 切换到 `Cmd` 标签
3. 输入：`py exec(open('路径/脚本.py').read())`

### 常用Python命令
```python
# 获取所有Actor
import unreal
actors = unreal.EditorLevelLibrary.get_all_level_actors()

# 选中的Actor
selected = unreal.EditorLevelLibrary.get_selected_level_actors()

# 生成Actor
location = unreal.Vector(0, 0, 0)
rotation = unreal.Rotator(0, 0, 0)
actor = unreal.EditorLevelLibrary.spawn_actor_from_class(
    unreal.StaticMeshActor, location, rotation
)

# 保存关卡
unreal.EditorLevelLibrary.save_current_level()
```

---

## 🎨 常用蓝图节点

### 玩家交互
```
Get Player Character → Cast to BP_HorrorPlayerCharacter
Get Player Controller → Cast to BP_HorrorPlayerController
Get Player Camera Manager
```

### 组件访问
```
Get Component by Class → FlashlightComponent
Get Component by Class → FearComponent
Get Component by Class → InteractionComponent
Get Component by Class → InventoryComponent
```

### 音频控制
```
Get Game Instance → Cast to BP_HorrorGameInstance
  → Get Audio Subsystem
    → Play Sound at Location
    → Play Music Track
    → Stop All Sounds
```

### AI控制
```
Get AI Controller → Cast to HorrorThreatAIController
  → Set Behavior State (Patrol/Chase/Attack)
  → Get Current Target
  → Set Patrol Points
```

### 事件总线
```
Get Game Instance → Get Event Bus
  → Publish Event (Tag, Payload)
  → Subscribe to Event (Tag, Callback)
```

### 恐惧系统
```
Get Fear Component
  → Increase Fear (Amount, Duration)
  → Decrease Fear (Amount)
  → Get Fear Value
  → On Fear Changed (Event)
```

### 目标系统
```
Get Game Instance → Get Objective Manager
  → Start Objective (ID)
  → Complete Objective (ID)
  → Update Objective Progress (ID, Progress)
```

### 证据系统
```
Get Game Instance → Get Evidence Archive
  → Collect Evidence (ID)
  → Has Evidence (ID)
  → Get Collected Evidence Count
```

---

## 📁 资产路径速查

### 核心蓝图
```
/Game/Blueprints/
  BP_HorrorPlayerCharacter
  BP_HorrorPlayerController
  BP_HorrorGameMode
  BP_HorrorGameInstance
```

### AI蓝图
```
/Game/StoneGolem/Blueprints/
  BP_StoneGolem
  BP_GolemAIController
  BT_GolemBehavior (行为树)
  BB_Golem (黑板)
```

### 交互物品
```
/Game/Blueprints/Interactables/
  BP_EvidenceActor
  BP_InteractableDoor
  BP_InteractableObject
  BP_PickupItem
```

### 音频资产
```
/Game/SoundsOfHorror/
  Atmosphere/        # 环境音
    Amb_Industrial_Loop
    Amb_WaterDrip
    Amb_Machinery_Hum
  Tension/          # 紧张音乐
    Mus_Tension_Low
    Mus_Tension_Medium
    Mus_Chase
  BuildUps/         # 渐强音效
    SFX_BuildUp_Short
    SFX_BuildUp_Long
  Jumpscares/       # 跳杀音效
    SFX_Jumpscare_01
```

### 环境资产
```
/Game/DeepWaterStation/        # 主场景
/Game/SD_Art/Industrial_Architecture/  # 工业建筑
/Game/Fantastic_Dungeon_Pack/  # 地牢模块
/Game/ForestOfSpikes/          # 森林
/Game/Industri        # 管道
```

### 特效
```
/Game/Bodycam_VHS_Effect/
  Blueprints/
    BP_CameraComponent
    BP_CameraShake_Walk
    BP_CameraShake_Run
  Materials/
    M_VHS_PostProcess
```

### 材质
```
/Game/Content/_SM13/Materials/
  Master/           # 主材质
  Instances/        # 材质实例
  Functions/        # 材质函数
```

---

## 🎯 常用控制台命令

### 性能分析
```
stat fps          # 显示帧率
stat unit         # 显示性能统计
stat game         # 游戏线程统计
stat gpu          # GPU统计
stat memory       # 内存使用
profilegpu        # GPU性能分析
```

### 调试命令
```
ghost             # 穿墙模式
fly               # 飞行模式
walk              # 恢复行走
god               # 无敌模式
slomo 0.5         # 慢动作（0.5倍速）
slomo 2.0         # 快进（2倍速）
```

### 显示选项
```
show collision    # 显示碰撞体
show bounds       # 显示边界框
show navigation   # 显示导航网格
show bones        # 显示骨骼
show fog          # 切换雾效
show postprocessing  # 切换后处理
```

### AI调试
```
showdebug ai      # AI调试信息
showdebug behavior  # 行为树调试
showdebug perception  # 感知系统调试
```

### 音频调试
```
au.Debug.Sounds 1  # 显示音频调试
au.3dVisualize.Enabled 1  # 3D音频可视化
stat soundwaves    # 音频统计
```

### 关卡管理
```
open MapName      # 打开关卡
servertravel MapName  # 服务器切换关卡
```

---

## 置文件

### 项目设置
```
ConfEngine.ini     # 引擎配置
Config/DefaultGame.ini       # 游戏配置
Config/DefaultInput.ini      # 输入配置
Config/DefaultGameplayTags.ini  # 游戏标签
```

### 关键配置项
```ini
# DefaultEngine.ini
[/Script/Engine.Engine]
bUseFixedFrameRate=False
FixedFrameRate=60.000000

[/Script/NavigationSystem.NavigationSystemV1]
bAutoCreateNavigationData=True

# DefaultGame.ini
[/Script/HorrorProject.HorrorGameMode]
PlayerCharacterClass=/Game/Blueprints/BP_HorrorPlayerCharacter
```

---

## 📊 性能目标参考

### 帧率目标
- **高端PC**: 60 FPS @ 1080p High
- **中端PC**: 60 FPS @ 1080p Medium
- **低端PC**: 30 FPS @ 1080p Low

### 内存预算
- **系统内存**: < 8GB
- **显存**: < 4GB
- **关卡大小**: < 500MB

### 优化检查清单
- [ ] 使用LOD（3-4级）
- [ ] 启用遮挡剔除
- [ ] 限制同时活跃AI（≤3）
- [ ] 使用音频衰减
- [ ] 合并静态网格
- [ ] 使用纹理流送
- [ ] 限制粒子数量
- [ ] 使用光照贴图

---

## 🎨 材质快速设置

### 标准PBR材质
```
Base Color: 纹理或颜色
Metallic: 0 (非金属) / 1 (金属)
Roughness: 0.3-0.7 (常用范围)
Normal: 法线贴图
```

### 发光材质
```
Emissive Color: 发光颜色
Emissive Strength: 1-10
Blend Mode: Additive
```

### 透明材质
```
Opacity: 0-1
Blend Mode: Translucent
Lighting Mode: Surface TranslucencyVolume
```

---

## 🎬 设置

### 第一人称相机
```
FOV: 90-100
Camera Lag: 0
Camera Rotation Lag: 0
```

### 恐怖效V: 70-80 (更压抑)
Vignette: 0.4
Chromatic Aberration: 0.5
Film Grain: 0.3
```

### 相机抖动
```
Walk Shake: BP_CameraShake_Walk
Run Shake: BP_CameraShake_Run
Idle Shake: BP_CameraShake_Idle
```

---

## 💡 快速技巧

### 快速原型
1. 使用BSP几何体快速搭建
2. 使用灰盒测试玩法
3. 后期替换为高质量资产

### 光照技巧
- 使用3点照明（主光、补光、轮廓光）
- 恐怖游戏：低照度 + 高对比度
- 使用体积光营造氛围

### 音频技巧
- 环境音循环播放
- 紧张音乐根据距离淡入淡出
- 脚步声根据材质变化

### AI技巧
- 巡逻点形成闭环
- 检测范围 > 攻击范围
- 使用视锥体检测（更真实）

---

## 🔗 相关文档

- [QuickStart.md](QuickStart.md) - 30分钟快速开始
- [VideoTutorialScript.md](VideoTutorialScript.md) - 视频教程脚本
- [FAQ.md](../FAQ.md) - 常见问题
- [LevelDesignGuide.md](../LevelDesignGuide.md) - 完整关卡设计指南

---

**提示**：将此页面加入书签，随时查阅！
