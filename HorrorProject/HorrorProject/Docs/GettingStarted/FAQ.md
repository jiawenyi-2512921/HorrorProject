# HorrorProject 新手常见问题

## 🚀 入门问题

### Q: 我需要什么才能开始？
**A:** 你需要：
- Unreal Engine 5.6 或更高版本
- Visual Studio 2022（用于C++开发）
- 至少16GB RAM
- 支持DirectX 12的显卡
- 约50GB硬盘空间

### Q: 我是UE新手，能跟上吗？
**A:** 可以！我们的快速开始指南专为初学者设计。建议按以下顺序学习：
1. [QuickStart.md](QuickStart.md) - 30分钟快速入门
2. [CheatSheet.md](CheatSheet.md) - 常用操作速查
3. [VideoTutorialScript.md](VideoTutorialScript.md) - 详细视频教程

### Q: 项目文件在哪里？
**A:** 主要目录结构：
```
HorrorProject/
├── Content/          # 游戏资产（蓝图、材质、模型）
├── Source/           # C++源代码
├── Docs/             # 文档（你现在在这里）
├── Scripts/          # Python自动化脚本
└── Config/           # 配置文件
```

### Q: 如何打开项目？
**A:**
1. 双击 `HorrorProject.uproject`
2. 或在Epic Games Launcher中添加项目
3. 首次打开可能需要编译，耐心等待

---

## 🎮 游戏开发问题

### Q: 如何创建第一个房间？
**A:** 跟随 [QuickStart.md](QuickStart.md) 的5分钟教程：
1. 创建新关卡
2. 添加照明
3. 使用模块化资产搭建
4. 添加玩家出生点
5. 测试游戏

### Q: 在哪里找到可用的资产？
**A:** 主要资产包位置：
```
Content/DeepWaterStation/        # 工业水下站
Content/SD_Art/Industrial_Architecture/  # 工业建筑
Content/StoneGolem/              # 石头魔像敌人
Content/SoundsOfHorror/          # 音效包
Content/Bodycam_VHS_Effect/      # 摄像机效果
```

### Q: 如何添加AI敌人？
**A:**
1. 导航到 `Content/StoneGolem/Blueprints/`
2. 拖入 `BP_StoneGolem` 到场景
3. 添加 `Nav Mesh Bounds Volume` 覆盖房间
4. 创建 `Target Point` 作为巡逻点
5. 在魔像的 `Patrol Points` 中添加巡逻点引用

详细教程见 [QuickStart.md](QuickStart.md) 的10分钟部分。

### Q: 如何配置音效？
**A:** 三种方式：
1. **环境音**：使用 `HorrorAudioZoneActor`
2. **点音源**：直接放置 `Ambient Sound` Actor
3. **事件音效**：在蓝图中使用 `Play Sound at Location`

音效资产在 `Content/SoundsOfHorror/` 目录。

### Q: 手电筒不工作怎么办？
**A:** 检查：
1. 玩家角色是否是 `BP_HorrorPlayerCharacter`
2. `FlashlightComponent` 是否启用
3. 按 `F` 键切换手电筒
4. 检查电池是否耗尽（默认100%）

### Q: AI不移动怎么办？
**A:** 常见原因：
1. **没有导航网格**：添加 `Nav Mesh Bounds Volume` 并按 `P` 键检查
2. **没有巡逻点**：添加 `Target Point` 并在AI中引用
3. **AI未激活**：检查 `Auto Possess AI` 设置为 `Placed in World`
4. **碰撞阻挡**：确保AI胶囊体能通过通道

---

## 🔧 技术问题

### Q: 编译失败怎么办？
**A:** 尝试以下步骤：
1. 关闭UE编辑器
2. 删除 `Binaries/`、`Intermediate/`、`Saved/` 文件夹
3. 右键 `.uproject` → Generate Visual Studio project files
4. 在Visual Studio中重新编译
5. 打开UE编辑器

### Q: 游戏运行很卡怎么办？
**A:** 优化建议：
1. 按 `` ` `` 打开控制台，输入 `stat fps` 查看帧率
2. 输入 `stat unit` 查看性能瓶颈
3. 减少同时活跃的AI数量（推荐≤3）
4. 使用LOD（细节层次）
5. 启用遮挡剔除
6. 降低后处理质量

### Q: 资产加载失败怎么办？
**A:**
1. 检查资产路径是否正确
2. 在Content Browser中右键 → Fix Up Redirectors
3. 验证资产完整性：右键 → Validate
4. 重新导入资产

### Q: 蓝图报错怎么办？
**A:**
1. 查看错误信息（通常在编译日志中）
2. 检查节点连接是否正确
3. 确保变量类型匹配
4. 使用 `Print String` 节点调试
5. 查看 [蓝图最佳实践](../BestPractices/BlueprintDesign.md)

### Q: 如何调试蓝图？
**A:**
1. 在节点上按 `F9` 添加断点
2. 按 `Alt + P` 开始游戏
3. 执行到断点时会暂停
4. 查看变量值
5. 按 `F10` 单步执行

---

## 🎨 内容创作问题

### Q: 如何导入自己的模型？
**A:**
1. 准备FBX格式文件
2. 在Content Browser中点击 Import
3. 选择FBX文件
4. 配置导入选项：
   - Skeletal Mesh（骨骼网格）用于角色
   - Static Mesh（静态网格）用于环境
5. 点击 Import All

### Q: 如何创建材质？
**A:**
1. 右键 Content Browser → Material
2. 双击打开材质编辑器
3. 基础PBR材质需要：
   - Base Color（基础颜色）
   - Roughness（粗糙度）
   - Metallic（金属度）
   - Normal（法线贴图）
4. 连接到对应输出节点
5. 保存并应用到模型

### Q: 如何录制自己的音效？
**A:**
1. 使用Audacity等软件录制
2. 导出为WAV格式（44.1kHz）
3. 导入到UE（自动转换为压缩格式）
4. 创建Sound Cue进行高级配置
5. 添加衰减设置（Attenuation）

### Q: 如何制作过场动画？
**A:**
1. 打开 Sequencer（Window → Cinematics → Sequencer）
2. 创建新Level Sequence
3. 添加相机轨道
4. 添加角色动画轨道
5. 设置关键帧
6. 在蓝图中播放：`Play Level Sequence`

---

## 🤖 AI系统问题

### Q: AI检测不到玩家怎么办？
**A:** 检查：
1. AI Controller是否正确设置
2. AI Perception Component是否配置
3. Sight Config的检测范围（默认1500）
4. 玩家是否在AI视锥体内
5. 中间是否有障碍物阻挡视线

调试：输入控制台命令 `showdebug ai` 查看AI状态。

### Q: 如何让AI更聪明？
**A:**
1. 调整行为树逻辑
2. 增加感知系统灵敏度
3. 添加更多巡逻点（增加不可预测性）
4. 实现记忆系统（记住玩家最后位置）
5. 添加协作行为（多个AI配合）

### Q: 如何创建不同类型的AI？
**A:**
1. 复制 `BP_StoneGolem`
2. 修改参数：
   - 移动速度
   - 检测范围
   - 攻击方式
3. 替换模型和动画
4. 调整行为树逻辑

### Q: AI卡在墙里怎么办？
**A:**
1. 检查导航网格是否正确
2. 调整AI胶囊体大小
3. 确保墙壁有碰撞
4. 增加 `Nav Mesh Bounds Volume` 的 `Agent Radius`
5. 重新构建导航：Build → Build Paths

---

## 🔊 音频系统问题

### Q: 听不到音效怎么办？
**A:** 检查：
1. 音量设置（Volume）是否为0
2. 音效是否在衰减范围内
3. Sound Class设置是否正确
4. 是否被其他音效覆盖
5. 音频设备是否正常

调试：控制台输入 `au.Debug.Sounds 1` 显示音频调试信息。

### Q: 音效太大声/太小声？
**A:**
1. 调整Sound Cue的Volume Multiplier
2. 配置Attenuation Settings：
   - Inner Radius（内半径）：全音量范围
   - Falloff Distance（衰减距离）：音量衰减范围
3. 使用Sound Class统一管理音量
4. 添加Audio Volume控制区域音量

### Q: 如何实现3D空间音效？
**A:**
1. 使用 `Play Sound at Location`（不是 `Play Sound 2D`）
2. 配置Attenuation Settings
3. 启用Spatialization
4. 调整Spread（扩散角度）
5. 测试不同位置的音效

### Q: 背景音乐如何无缝循环？
**A:**
1. 确保音频文件首尾无缝
2. 在Sound Cue中启用Looping
3. 使用Audio Component而不是Play Sound
4. 设置合适的Fade In/Out时间

---

## 🎯 游戏机制问题

### Q: 如何实现存档系统？
**A:**
1. 创建SaveGame类（继承自USaveGame）
2. 保存数据：`Create Save Game Object` → `Save Game to Slot`
3. 加载数据：`Load Game from Slot` → `Cast to YourSaveGame`
4. 保存玩家位置、物品、进度等

示例代码见 `Source/HorrorProject/Save/` 目录。

### Q: 如何实现物品栏系统？
**A:**
1. 使用 `InventoryComponent`（已实现）
2. 添加物品：`Add Item to Inventory`
3. 移除物品：`Remove Item from Inventory`
4. 检查物品：`Has Item in Inventory`
5. 创建UI显示物品栏

### Q: 如何实现门锁机制？
**A:**
1. 使用 `BP_InteractableDoor`
2. 设置 `Requires Key = true`
3. 设置 `Key ID`（如 "rusty_key"）
4. 玩家拾取钥匙时添加到物品栏
5. 交互门时检查是否有钥匙

### Q: 如何实现恐惧系统？
**A:**
1. 使用 `FearComponent`（已实现）
2. 增加恐惧：`Increase Fear`
3. 减少恐惧：`Decrease Fear`
4. 监听变化：`On Fear Changed` 事件
5. 根据恐惧值触发效果（视觉、音效）

---

## 🎬 视觉效果问题

### Q: 如何添加VHS效果？
**A:**
1. 在玩家角色中添加 `VHSEffectComponent`
2. 配置参数：
   - Noise Intensity（噪点强度）
   - Scanline Speed（扫描线速度）
   - Chromatic Aberration（色差）
3. 可以动态调整强度（如根据恐惧值）

### Q: 如何实现相机抖动？
**A:**
1. 创建Camera Shake类（或使用现有的）
2. 在蓝图中调用：`Play World Camera Shake`
3. 不同情况使用不同抖动：
   - 行走：`BP_CameraShake_Walk`
   - 奔跑：`BP_CameraShake_Run`
   - 爆炸：自定义强烈抖动

### Q: 如何添加后处理效果？
**A:**
1. 在场景中放置 `Post Process Volume`
2. 勾选 `Infinite Extent`（全局生效）
3. 调整参数：
   - Vignette（暗角）
   - Film Grain（胶片颗粒）
   - Bloom（泛光）
   - Color Grading（调色）

### Q: 如何实现血迹效果？
**A:**
1. 使用Decal（贴花）
2. 创建血迹材质
3. 在蓝图中生成：`Spawn Decal at Location`
4. 设置Fade Out时间（可选）
5. 限制同时存在的贴花数量（性能）

---

## 🌐 多人游戏问题

### Q: 如何测试多人模式？
**A:**
1. 在编辑器中：Play → Number of Players = 2+
2. 独立测试：
   - 打包游戏
   - 一个实例作为服务器：`YourGame.exe -log`
   - 另一个实例作为客户端：`YourGame.exe 127.0.0.1 -log`

### Q: 如何实现网络同步？
**A:**
1. 在变量上添加 `Replicated` 标记
2. 实现 `GetLifetimeReplicatedProps`
3. 使用 `Replicated` 或 `ReplicatedUsing` 函数
4. 服务器权威：重要逻辑在服务器执行

详见 [网络多人游戏文档](../../../Docs/Network/MultiplayerSetup.md)。

### Q: 客户端看不到服务器怎么办？
**A:** 检查：
1. 防火墙设置
2. 端口是否开放（默认7777）
3. 是否在同一局域网
4. Session是否正确创建和启动
5. Online Subsystem配置

---

## 📊 性能优化问题

### Q: 如何提高帧率？
**A:** 优先级排序：
1. **光照优化**：使用烘焙光照，减少动态光源
2. **AI优化**：限制同时活跃AI数量，增加Tick间隔
3. **LOD**：为所有模型创建LOD
4. **遮挡剔除**：使用Occlusion Culling
5. **纹理优化**：使用合适的分辨率和压缩

### Q: 如何检测性能瓶颈？
**A:**
1. `stat fps` - 查看帧率
2. `stat unit` - 查看CPU/GPU/渲染时间
3. `stat game` - 游戏线程统计
4. `stat gpu` - GPU详细统计
5. `profilegpu` - GPU性能分析器

### Q: 内存占用太高怎么办？
**A:**
1. `stat memory` 查看内存使用
2. 减少纹理分辨率
3. 启用纹理流送（Texture Streaming）
4. 卸载未使用的资产
5. 使用对象池（Object Pooling）

---

## 🛠️ Python脚本问题

### Q: 如何运行Python脚本？
**A:**
1. 在UE编辑器中：Window → Developer Tools → Output Log
2. 切换到 `Cmd` 标签
3. 输入：`py exec(open('路径/脚本.py').read())`
4. 或在项目设置中启用Python插件后直接运行

### Q: Python脚本报错怎么办？
**A:**
1. 检查Python插件是否启用
2. 检查脚本路径是否正确
3. 查看Output Log中的错误信息
4. 确保使用UE的Python API（unreal模块）
5. 检查Python版本兼容性（UE 5.6使用Python 3.9）

### Q: 有哪些实用的Python脚本？
**A:** 项目提供的脚本：
- `CodexCreateVerticalSliceMap.py` - 自动创建关卡
- `CodexAuditVerticalSliceMap.py` - 关卡质量检查
- `CodexFixDeepWaterSpawn.py` - 修复生成点
- 更多脚本见 `Saved/` 目录

---

## 📚 学习资源

### Q: 推荐的学习路径？
**A:**
1. **第1周**：UE基础操作 + QuickStart教程
2. **第2周**：蓝图编程 + AI系统
3. **第3周**：音频系统 + 视觉效果
4. **第4周**：完整项目实践

### Q: 有哪些推荐的外部资源？
**A:**
- **官方文档**：[docs.unrealengine.com](https://docs.unrealengine.com/)
- **视频教程**：Unreal Engine官方YouTube频道
- **社区**：Unreal Slackers Discord
- **论坛**：Unreal Engine Forums

### Q: 如何获得帮助？
**A:**
1. 查看项目文档（`Docs/` 目录）
2. 搜索现有Issues
3. 在团队聊天中提问
4. 创建新Issue（附带详细信息）
5. 参考 [Troubleshooting](../Troubleshooting/) 文档

---

## 🎓 进阶问题

### Q: 如何添加新的游戏机制？
**A:**
1. 研究现有系统架构
2. 创建新的Component或Actor类
3. 实现核心逻辑
4. 添加蓝图接口
5. 编写文档和测试
6. 遵循 [编码规范](../Developer/CodingStandards.md)

### Q: 如何贡献代码？
**A:**
1. Fork项目
2. 创建功能分支
3. 实现功能并测试
4. 提交Pull Request
5. 等待代码审查
6. 根据反馈修改

### Q: 如何打包发布游戏？
**A:**
1. Project Settings → Packaging
2. 配置目标平台
3. 设置打包选项（Shipping配置）
4. File → Package Project → Windows (64-bit)
5. 等待打包完成
6. 测试打包后的游戏

详见 [构建指南](../Developer/BuildGuide.md)。

---

## 💡 最佳实践

### Q: 恐怖游戏设计的关键是什么？
**A:**
1. **节奏控制**：紧张-放松-高潮循环
2. **环境叙事**：用场景讲故事
3. **音效设计**：声音比视觉更重要
4. **玩家引导**：不要让玩家迷路
5. **威胁管理**：隐藏的威胁比看见的更可怕

详见 [关卡设计指南](../LevelDesignGuide.md)。

### Q: 如何保持项目整洁？
**A:**
1. 使用清晰的命名规范
2. 按功能组织文件夹结构
3. 定期清理未使用的资产
4. 使用版本控制（Git）
5. 编写文档和注释

### Q: 如何提高开发效率？
**A:**
1. 使用快捷键（见 [CheatSheet.md](CheatSheet.md)）
2. 创建蓝图函数库（复用代码）
3. 使用Python脚本自动化
4. 建立资产模板
5. 使用版本控制

---

## 🔗 相关文档

- [QuickStart.md](QuickStart.md) - 30分钟快速开始
- [CheatSheet.md](CheatSheet.md) - 快捷键和常用操作
- [VideoTutorialScript.md](VideoTutorialScript.md) - 视频教程脚本
- [完整FAQ](../FAQ.md) - 更多系统级问题
- [故障排除](../Troubleshooting/) - 详细问题解决方案

---

**提示**：如果这里没有找到答案，请查看完整文档或在社区提问！

**最后更新**：2026-04-28
