# HorrorProject 快速参考指南

> 常用资产路径、推荐组合与性能优化速查表

---

## 📂 常用资产路径速查

### 核心蓝图
```
玩家角色: /Game/Blueprints/BP_HorrorPlayerCharacter
玩家控制器: /Game/Blueprints/BP_HorrorPlayerController
摄像机组件: /Game/Bodycam_VHS_Effect/Blueprints/BP_CameraComponent
时间系统: /Game/Bodycam_VHS_Effect/Blueprints/BP_TimeOfDay
```

### VHS效果系统
```
后处理材质:
  /Game/Bodycam_VHS_Effect/Materials/MI_OldVHS
  /Game/Bodycam_VHS_Effect/Materials/MI_Noise
  /Game/Bodycam_VHS_Effect/Materials/MI_ChromaticAbberation
  /Game/Bodycam_VHS_Effect/Materials/MI_FishEye
  /Game/Bodycam_VHS_Effect/Materials/MI_CameraHole

相机抖动:
  /Game/Bodycam_VHS_Effect/Blueprints/CameraShakes/BP_CameraShake_Idle
  /Game/Bodycam_VHS_Effect/Blueprints/CameraShakes/BP_CameraShake_Walk
  /Game/Bodycam_VHS_Effect/Blueprints/CameraShakes/BP_CameraShake_Run
```

### 音效系统
```
环境音: /Game/SoundsOfHorror/Atmosphere/CUE_SOH_ATM_*
跳杀音: /Game/SoundsOfHorror/Jumpscares/CUE_SOH_JS_*
紧张音: /Game/SoundsOfHorror/Tension/CUE_SOH_TS_*
线索音: /Game/SoundsOfHorror/Clues/CUE_SOH_Clue_*
```

### 场景资产
```
水下基地: /Game/DeepWaterStation/Meshes/SM_*
工业框架: /Game/SD_Art/Industrial_Architecture/SM_Beam_*
废弃装饰: /Game/Scrapopolis/Meshes/SM_*
地牢模块: /Game/Fantastic_Dungeon_Pack/meshes/MOD_*
```

---

## 🎯 场景快速搭建方案

### 方案1: 水下研究站走廊

**结构组件**
```
地板: SM_FloorBase4x4 (4x4网格排列)
墙体: SM_MetalWall07 + SM_MetalWall08 (交替使用)
天花板: SM_Ceiling06
门: SM_MetalDoor01
窗户: SM_WinMetal01
```

**照明**
```
主光源: Point Light (蓝色调, 强度800)
辅助光: SM_Lamp01 + MI_Building_Light
闪烁灯: SM_Lamp05 + MI_LightFlicker_01
```

**装饰细节**
```
管道: SM_AdapterPipe01, SM_Ventilation01
设备: SM_Devices01, SM_Terminal01
电缆: SM_Cable_Wall_01 (Industrial_Architecture)
道具: SM_GasCylinderA01, SM_Props01
```

**音效配置**
```
环境层: CUE_SOH_ATM_03 (35%音量, 循环)
威胁层: CUE_SOH_RU_02 (25%音量, 循环)
紧张层: CUE_SOH_TS_05 (45%音量, 根据玩家位置)
```

**后处理**
```
MI_OldVHS (权重0.6)
MI_Noise (权重0.4)
MI_CameraHole (权重0.8)
蓝色色调 (Temperature: -20)
```

**预计性能**: 60-90 FPS (中等配置)

---

### 方案2: 废弃工厂大厅

**结构组件**
```
框架: SM_Beam_80x40_* (Industrial_Architecture)
连接件: SM_Beam_80x40_Connection_*
墙体: SM_House_Back (Scrapopolis)
地面: MI_Building_Floor + MI_Puddles_1
```

**支撑与装饰**
```
支撑柱: SM_Support_1 至 SM_Support_10 (随机分布)
管道: SM_Pipe_1 至 SM_Pipe_5
电线: SM_wires_1, SM_wires_2, SM_wires_3
碎片: SM_Debris_rock_*, SM_Debris_gear_*
```

**照明**
```
主光源: Directional Light (透过破损屋顶)
点光源: 稀疏分布, 50%闪烁
火光: SM_CampFire + M_CampFire材质
体积雾: M_Volumetric_Fog (密度0.02)
```

**音效配置**
```
环境层: CUE_SOH_ATM_08 (40%音量)
风声层: CUE_SOH_ATM_12 (30%音量)
金属音: CUE_SOH_RU_06 (间歇触发)
```

**后处理**
```
MI_OldVHS (权重0.8)
MI_NoiseColor (权重0.5)
MI_ChromaticAbberation (权重0.3)
暖色调 (Temperature: +10)
饱和度降低 (-0.3)
```

**预计性能**: 50-70 FPS (中等配置)

---

### 方案3: 地下地牢迷宫

**模块化搭建**
```
直线段: MOD_Base_01_straight_small/med/large
转角: MOD_Base_01_angle_small/med/large
交叉: MOD_Base_01_convex_* + MOD_Base_01_concave_*
柱子: MOD_Column_01_base + MOD_Column_01_block_1
```

**材质配置**
```
地板: MI_MOD_Floor_01_v1/v2/v3 (混合使用)
墙体: MI_MOD_Wall_01_v1
装饰: MI_MOD_Trim_Stone_01_v1/v2/v3
```

**照明与特效**
```
火把: MI_FX_fire_dungeon (间隔5-8米)
雾效: MI_FX_fog_dungeon_medium (地面层)
门缝雾: MI_FX_fog_dungeon_door (门后)
粒子: MI_FX_particles_dungeon (灰尘)
```

**敌人配置**
```
守卫: Stone_Golem (SKM_Stone_Golem)
动画: ThirdPersonIdle, ThirdPersonWalk, ThirdPersonRun
```

**音效配置**
```
环境层: CUE_SOH_ATM_10 (40%音量)
回声层: CUE_SOH_RU_08 (35%音量, 长混响)
紧张层: CUE_SOH_TS_09 (敌人附近60%音量)
```

**后处理**
```
MI_Noise (权重0.6)
MI_Emboss (权重0.2, 梦境感)
低饱和度 (-0.5)
暗角效果 (Vignette: 0.6)
```

**预计性能**: 70-100 FPS (模块化优化良好)

---

## 🎨 推荐材质组合

### 组合1: 监控摄像头效果
```cpp
PostProcessSettings.WeightedBlendables = {
    {1.0f, MI_FishEye},           // 鱼眼镜头
    {0.9f, MI_CameraHole},        // 镜头暗角
    {0.7f, MI_NoiseColor},        // 彩色噪点
    {0.8f, MI_OldVHS},            // VHS失真
};

// 时间戳叠加
AddWidget(WBP_CameraTimestamp);

// 扫描线效果
PostProcessSettings.FilmGrainIntensity = 0.6f;
```

**适用场景**: 监控录像回放、安保摄像头视角

---

### 组合2: 精神错乱视觉
```cpp
// 根据精神值动态调整
float SanityLevel = GetPlayerSanity(); // 0.0 - 1.0
float Insanity = 1.0f - SanityLevel;

PostProcessSettings.WeightedBlendables = {
    {Insanity * 0.8f, MI_ChromaticAbberation}, // 色差
    {Insanity * 1.0f, MI_Noise},               // 噪点
    {Insanity * 0.5f, MI_Emboss},              // 浮雕
};

// 视野扭曲
PostProcessSettings.LensDistortion = Insanity * 0.3f;

// 颜色异常
PostProcessSettings.ColorSaturation = FVector4(
    1.0f - Insanity * 0.5f,  // 降低饱和度
    1.0f - Insanity * 0.3f,
    1.0f - Insanity * 0.7f,
    1.0f
);
```

**适用场景**: 精神值系统、恐惧效果、药物影响

---

### 组合3: 水下视觉效果
```cpp
// 水下后处理
PostProcessSettings.WeightedBlendables = {
    {0.6f, MI_FishEye},           // 轻微扭曲
    {0.4f, MI_Noise},             // 水中颗粒
};

// 蓝绿色调
PostProcessSettings.ColorGrading.Global.Saturation = FVector4(0.7f);
PostProcessSettings.ColorGrading.Global.Gain = FVector4(
    0.6f, 0.8f, 1.0f, 1.0f  // 蓝绿色调
);

// 体积雾模拟水体
ExponentialHeightFog->FogDensity = 0.05f;
ExponentialHeightFog->FogInscatteringColor = FLinearColor(0.1f, 0.3f, 0.4f);

// 焦散效果
SpawnDecal(M_caustick01, FloorSurface);
```

**适用场景**: 水下关卡、淹没区域、水箱内部

---

### 组合4: 夜视仪效果
```cpp
// 夜视绿色调
PostProcessSettings.ColorGrading.Global.Gain = FVector4(
    0.2f, 1.0f, 0.3f, 1.0f  // 绿色主导
);

// 高对比度
PostProcessSettings.ColorGrading.Global.Contrast = FVector4(1.5f);

// 噪点和扫描线
PostProcessSettings.WeightedBlendables = {
    {0.8f, MI_Noise},
    {0.5f, MI_Sharpen},  // 锐化
};

// 暗角
PostProcessSettings.VignetteIntensity = 0.8f;

// 降低色彩
PostProcessSettings.ColorGrading.Global.Saturation = FVector4(0.1f);
```

**适用场景**: 夜视装备、特殊视觉模式

---

## 🔊 音效分层推荐配置

### 配置1: 探索阶段 (低威胁)
```
Layer 1 - 环境基础 (30-40%):
  CUE_SOH_ATM_01 (循环)

Layer 2 - 细节层 (20-30%):
  CUE_SOH_ATM_08 (循环, 随机音高±5%)

Layer 3 - 互动反馈 (70-80%):
  CUE_SOH_Clue_* (拾取物品时)
  CUE_SOH_PZ_* (解谜时)

总音量: 60-70%
混响: 中等 (1.5-2.5秒)
```

---

### 配置2: 紧张阶段 (中威胁)
```
Layer 1 - 环境基础 (25-35%):
  CUE_SOH_ATM_10 (循环)

Layer 2 - 威胁层 (30-40%):
  CUE_SOH_RU_06 (循环)
  CUE_SOH_TS_07 (循环)

Layer 3 - 事件层 (80-90%):
  CUE_SOH_BU_* (事件前3秒)
  CUE_SOH_IP_* (突发事件)

总音量: 70-80%
混响: 大空间 (3-4秒)
低频增强: +6dB (60-80Hz)
```

---

### 配置3: 追逐阶段 (高威胁)
```
Layer 1 - 环境基础 (15-20%):
  CUE_SOH_ATM_12 (循环, 压抑感)

Layer 2 - 紧张层 (60-70%):
  CUE_SOH_TS_09 (循环, 高强度)
  CUE_SOH_RU_08 (循环, Boss接近)

Layer 3 - 冲击层 (90-100%):
  CUE_SOH_JS_* (跳杀时刻)
  CUE_SOH_IP_* (攻击反馈)

总音量: 85-95%
混响: 短混响 (0.5-1秒, 紧迫感)
压缩: 重度压缩 (保持响度)
心跳音效: 叠加低频脉冲 (60-80 BPM)
```

---

### 配置4: 安全区域 (无威胁)
```
Layer 1 - 环境基础 (40-50%):
  CUE_SOH_ATM_01 (循环, 平静)

Layer 2 - 音乐层 (50-60%):
  CUE_SOH_MD_03 (循环, 舒缓旋律)

Layer 3 - 互动层 (70%):
  CUE_SOH_Clue_02 (存档提示)
  CUE_SOH_PZ_12 (物品整理)

总音量: 50-60%
混响: 小空间 (0.8-1.2秒)
高频柔化: -3dB (8kHz+)
```

---

## ⚡ 性能优化速查

### LOD配置建议

**大型静态网格 (>5000三角形)**
```
LOD0: 原始模型 (0-10米)
LOD1: 70%三角形 (10-25米)
LOD2: 40%三角形 (25-50米)
LOD3: 20%三角形 (50米+)
```

**中型静态网格 (1000-5000三角形)**
```
LOD0: 原始模型 (0-15米)
LOD1: 60%三角形 (15-40米)
LOD2: 30%三角形 (40米+)
```

**小型道具 (<1000三角形)**
```
LOD0: 原始模型 (0-20米)
LOD1: 50%三角形 (20米+)
```

---

### 光照优化

**静态光照 (推荐)**
```
Lightmap Resolution:
  - 大型结构: 128-256
  - 中型物体: 64-128
  - 小型道具: 32-64

Lightmass Settings:
  - Num Indirect Lighting Bounces: 3
  - Indirect Lighting Quality: 2-3
  - Indirect Lighting Smoothness: 0.8
```

**动态光照 (谨慎使用)**
```
最大动态光源数: 4-6个
阴影距离: 1000-2000单位
阴影质量: 中等
使用光源函数替代复杂动画
```

**混合方案**
```
主光源: 静态 (Directional Light)
辅助光: 静态 (Point/Spot Light)
特效光: 动态 (火光、闪烁灯)
角色光: 动态 (手电筒)
```

---

### 材质优化

**纹理设置**
```
BaseColor: 2048x2048 (主要资产) / 1024x1024 (次要)
Normal: 2048x2048 / 1024x1024
ORM (AO+Roughness+Metallic): 2048x2048 / 1024x1024

压缩格式:
  - BaseColor: DXT1 (无Alpha) / DXT5 (有Alpha)
  - Normal: BC5 / DXT5
  - ORM: DXT1

Mipmap: 启用
Streaming: 启用
```

**材质复杂度**
```
指令数限制:
  - 简单材质: <100指令
  - 标准材质: 100-200指令
  - 复杂材质: 200-300指令
  - 后处理: <150指令

纹理采样: <16个
数学运算: 最小化除法和三角函数
```

---

### 音效优化

**音频格式**
```
短音效 (<5秒): WAV (未压缩)
中等音效 (5-30秒): OGG Vorbis (质量0.7)
长音效/音乐 (>30秒): OGG Vorbis (质量0.5) + Streaming

采样率:
  - 环境音: 22050 Hz
  - 音效: 44100 Hz
  - 音乐: 44100 Hz
```

**音效池管理**
```cpp
// 限制同时播放数量
MaxConcurrentSounds = 16;

// 优先级系统
enum ESoundPriority {
    Background = 0,   // 可被剔除
    Low = 1,
    Medium = 2,
    High = 3,
    Critical = 4      // 不可剔除
};

// 距离衰减
AttenuationSettings:
  - 环境音: 1000-3000单位
  - 音效: 500-1500单位
  - 对话: 300-800单位
```

---

### 粒子系统优化

**发射器设置**
```
最大粒子数:
  - 小型效果: 10-50
  - 中型效果: 50-200
  - 大型效果: 200-500

更新频率: 30-60 FPS (不需要全帧率)
LOD距离:
  - LOD0: 0-15米
  - LOD1: 15-30米 (减少50%粒子)
  - LOD2: 30米+ (减少75%粒子)
```

**材质优化**
```
使用简单材质 (Unlit)
纹理大小: 256x256 或 512x512
避免复杂计算
使用Sprite而非Mesh粒子
```

---

## 🎯 常见问题快速解决

### 问题1: 后处理效果不显示
```
检查清单:
☐ PostProcessVolume的Unbound勾选
☐ 材质Domain设置为Post Process
☐ 材质Blendable Location正确
☐ 权重值>0
☐ 后处理体积优先级正确
```

---

### 问题2: 音效播放卡顿
```
解决方案:
1. 启用音频流送 (长音效)
2. 降低同时播放数量
3. 使用Sound Cue的随机化避免重复
4. 检查音频格式和压缩设置
5. 预加载关键音效
```

---

### 问题3: 材质闪烁/Z-Fighting
```
解决方案:
1. 调整网格位置避免重叠
2. 使用Decal而非重叠网格
3. 增加相机Near Clip Plane (0.1 → 1.0)
4. 调整材质Depth Bias
5. 使用Translucent Sort Priority
```

---

### 问题4: 光照漏光
```
解决方案:
1. 增加墙体厚度
2. 调整Lightmap Resolution
3. 使用Lightmass Importance Volume
4. 检查网格UV2 (Lightmap UV)
5. 增加Indirect Lighting Quality
6. 使用Light Portals
```

---

### 问题5: 性能突然下降
```
诊断步骤:
1. 使用 stat fps 查看帧率
2. 使用 stat unit 查看瓶颈 (Game/Draw/GPU)
3. 使用 stat scenerendering 查看渲染统计
4. 使用 profilegpu 查看GPU性能
5. 检查Draw Call数量 (目标<2000)
6. 检查三角形数量 (目标<2M)
```

---

## 📋 资产命名规范

### 前缀系统
```
SM_  - Static Mesh (静态网格)
SK_  - Skeletal Mesh (骨骼网格)
SKM_ - Skeletal Mesh (带动画)
M_   - Material (母材质)
MI_  - Material Instance (材质实例)
T_   - Texture (纹理)
BP_  - Blueprint (蓝图)
WBP_ - Widget Blueprint (UI蓝图)
CUE_ - Sound Cue (音效Cue)
WAV_ - Wave File (音频文件)
ABP_ - Animation Blueprint (动画蓝图)
MOD_ - Modular (模块化组件)
```

### 后缀系统
```
_D   - Diffuse/Albedo (漫反射)
_N   - Normal (法线)
_ORM - Occlusion+Roughness+Metallic (打包贴图)
_E   - Emissive (自发光)
_M   - Mask (遮罩)
_H   - Height (高度)
_01, _02 - 变体编号
_LOD0, _LOD1 - LOD级别
```

---

## 🔗 相关文档链接

- [完整资产目录](AssetCatalog.md) - 所有资产详细列表
- [音效目录](SoundsCatalog.md) - 音效系统完整指南
- [材质目录](MaterialsCatalog.md) - 材质系统详细说明

---

## 📞 技术支持

遇到问题时的检查顺序:
1. 查看本快速参考文档
2. 查阅对应的详细目录文档
3. 检查Unreal Engine官方文档
4. 查看资产包原始文档
5. 联系项目技术负责人

---

**文档版本**: 1.0
**最后更新**: 2026-04-28
**维护者**: HorrorProject Team

**快速导航**:
- [返回资产目录](AssetCatalog.md)
- [返回音效目录](SoundsCatalog.md)
- [返回材质目录](MaterialsCatalog.md)
- [返回项目文档首页](../README.md)
