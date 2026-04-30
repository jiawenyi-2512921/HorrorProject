# HorrorProject 材质目录

> 完整的材质系统索引与使用指南

---

## 📊 材质统计总览

| 资产包 | 母材质 | 材质实例 | 主要用途 |
|--------|--------|----------|----------|
| Bodycam_VHS_Effect | 11 | 29 | 后处理/视觉效果 |
| DeepWaterStation | 15 | 60+ | 工业/水下场景 |
| Scrapopolis | 12 | 25+ | 废弃/破败环境 |
| Fantastic_Dungeon_Pack | 10 | 30+ | 地牢/石质结构 |

**总计**: 48个母材质 + 144+个材质实例

---

## 🎬 Bodycam_VHS_Effect 后处理材质

### 🎥 核心视觉效果

#### VHS复古效果
**M_OldVHS** / **MI_OldVHS**
- 老式录像带失真效果
- 扫描线、色彩偏移、噪点
- 适合: 监控录像、回忆场景、复古氛围

**技术参数**:
```
扫描线密度: 480-720线
色彩偏移: RGB通道分离
噪点强度: 0.15-0.35
帧抖动: 轻微垂直偏移
```

#### 噪点系统
**M_Noise** / **MI_Noise**
- 黑白噪点效果
- 可调节强度和频率
- 适合: 静电干扰、信号不良

**M_NoiseColor** / **MI_NoiseColor**
- 彩色噪点效果
- RGB通道独立噪点
- 适合: 电子故障、数字失真

**使用建议**:
```cpp
// 动态噪点强度
float NoiseIntensity = FMath::Lerp(0.1f, 0.5f, StressLevel);
PostProcessMaterial->SetScalarParameterValue("NoiseStrength", NoiseIntensity);
```

#### 色差效果
**M_ChromaticAbberation** / **MI_ChromaticAbberation**
- RGB通道分离效果
- 模拟镜头色差
- 适合: 损坏镜头、精神错乱视觉

**参数控制**:
```
偏移强度: 0.002-0.01
径向扭曲: 启用/禁用
中心点: 屏幕中心或自定义
```

#### 镜头效果
**M_FishEye** / **MI_FishEye**
- 鱼眼镜头扭曲
- 广角摄像头效果
- 适合: 监控摄像头、门镜视角

**M_CameraHole** / **MI_CameraHole**
- 镜头孔洞/暗角效果
- 圆形或椭圆形遮罩
- 适合: 望远镜、窥视孔、摄像机取景

**组合使用**:
```
监控摄像头效果 = FishEye + CameraHole + Noise + OldVHS
```

#### 图像处理
**M_Emboss** / **MI_Emboss**
- 浮雕效果
- 边缘检测风格
- 适合: 梦境、幻觉、特殊视觉状态

**M_Sharpen** / **MI_Sharpen**
- 锐化效果
- 增强边缘对比度
- 适合: 专注状态、瞄准模式

### 🏢 场景材质

#### 建筑墙体
**M_Building_Wall** (母材质)
- 通用墙体材质系统
- 支持法线、粗糙度、AO

**材质实例**:
- `MI_Building_Wall` - 标准墙体
- `MI_Building_Wall_01` - 变体1 (脏污)
- `MI_Building_Wall_02` - 变体2 (破损)
- `MI_Building_Wall_03` - 变体3 (涂鸦)
- `MI_Building_Wall_List_01/02/03` - 列表墙体

**参数**:
```
BaseColor: 墙体颜色
Roughness: 0.6-0.8 (粗糙表面)
Normal Strength: 0.5-1.0
Dirt Mask: 污渍遮罩
```

#### 地板系统
**MI_Building_Floor** - 标准地板
- 混凝土/瓷砖纹理
- 磨损细节

**MI_Building_Floor_Wood_01/02** - 木地板
- 木纹纹理
- 两种颜色变体
- 适合: 住宅区域、办公室

**参数调整**:
```cpp
// 动态磨损
float WearLevel = CalculateAreaUsage();
FloorMaterial->SetScalarParameterValue("WearAmount", WearLevel);
```

#### 天花板
**MI_Building_Ceiling** - 标准天花板
**MI_Building_Ceiling_Vent** - 带通风口
**MI_Building_Ceiling_Wire** - 带电线

**用途**: 工业建筑、办公楼、地下设施

#### 照明材质
**M_Building_Light** / **MI_Building_Light**
- 发光材质基础
- 可调节发光强度和颜色

**彩色照明变体**:
- `MI_Building_Light_Purple` - 紫色灯光 (神秘/不安)
- `MI_Building_Light_Red` - 红色灯光 (危险/警报)

**M_LightFlicker** / **MI_LightFlicker_01**
- 闪烁灯光效果
- 需要蓝图控制闪烁频率

**闪烁控制**:
```cpp
// 随机闪烁
void UpdateFlickerLight() {
    float FlickerValue = FMath::FRandRange(0.3f, 1.0f);
    LightMaterial->SetScalarParameterValue("Intensity", FlickerValue);
}
```

#### 道具材质
**M_Props** - 通用道具材质
**MI_TableChair01** - 桌椅材质
**MI_Television01** - 电视材质
**M_Flashlight** - 手电筒材质
**PM_Carpet** - 地毯物理材质

---

## 🌊 DeepWaterStation 工业材质

### 🏗️ 母材质系统

#### 基础材质
**M_Base** - 通用基础材质
**M_BaseSolid** - 实心材质
**M_BaseT** - 透明材质
**M_BaseE** - 发光材质
**M_BaseEm** / **M_BaseEmissive** - 自发光材质
**M_BaseLight** - 灯光材质

#### 虚拟纹理材质
**M_Base_VT_01/02/03** - 虚拟纹理系统
- 高分辨率纹理流送
- 优化大场景性能
- 适合: 大型工业设施

#### 特殊材质
**M_Glass** / **M_Glass2** - 玻璃材质
- 透明度、折射、反射
- 适合: 窗户、容器、屏幕

**M_Water04** / **MI_Water04** - 水体材质
- 水面波动、反射
- 适合: 水下场景、积水

**M_caustick01** - 焦散效果
- 水下光线效果
- 配合水体使用

### 🔧 工业组件材质

#### 金属结构
- `MI_MetalWall07/08/10` - 金属墙体系列
- `MI_MetalDoor01` - 金属门
- `MI_MetalGate01` - 金属闸门
- `MI_MetalWin01/02` - 金属窗框

**金属参数**:
```
Metallic: 0.8-1.0
Roughness: 0.3-0.6 (根据磨损程度)
Specular: 0.5
```

#### 管道系统
- `MI_AdapterPipe01` - 管道适配器
- `MI_Ventilation01` - 通风管道

#### 设备材质
- `MI_Devices01/02` - 设备外壳
- `MI_LabProps01` - 实验室道具
- `MI_Container01` - 容器
- `MI_ContainerGlass` - 玻璃容器

#### 地板与墙体
- `MI_BaseSolidTile` - 实心瓷砖
- `MI_FloorPart07` - 地板部件
- `MI_Ceiling06` - 天花板
- `MI_Wall09` - 墙体

### 🌊 海洋元素材质
- `MI_Fish02/04` - 鱼类材质
- `MI_Mollusk03` - 软体动物
- `MI_Buoy01` - 浮标

### 🪨 环境材质
- `MI_Rock14/15/16` - 岩石材质
- `MI_RockSet01/05` - 岩石组合
- `MI_Terrain_ShaderNoTess` / `M_Terrain_ShaderNoTess` - 地形材质

### 🛏️ 室内材质
- `MI_BedBase01` - 床架
- `MI_BedSheets01/02` - 床单
- `MI_Pillows01` - 枕头
- `MI_Chair_04` / `M_Chair_fabrick_04` - 椅子
- `MI_Bathroom02` - 浴室材质

### 💡 照明与特效
- `MI_Light01` - 灯光材质
- `MI_Lamp06/07` - 灯具材质
- `M_Dust_Particle` / `M_dust03` / `MI_dust03` - 灰尘粒子

### 🔌 电气材质
- `MI_Wires01` / `M_Wires` - 电线材质
- `MI_CellophaneFilm01` - 塑料薄膜

### 🏗️ 模块化材质
- `MI_Mod02` / `MI_Mod02Int_01/02` / `MI_Mod02OUT` - 模块02系列
- `MI_Mod03` - 模块03
- `MI_MOD05` / `MI_Mod05Ceiling01` / `MI_Mod05Wall0` / `MI_Mod05IndProps01` - 模块05系列
- `MI_Mod07Wall01` - 模块07墙体

---

## 🏚️ Scrapopolis 废弃材质

### 🏗️ 建筑材质
**M_Master_Material** - 主材质系统
- 通用PBR材质
- 支持污渍、锈蚀、破损

#### 建筑组件
- `MI_House_Back` - 房屋背面
- `MI_House_Door` - 房屋门
- `MI_Fence` - 围栏

#### 支撑结构
- `MI_Support_1` 至 `MI_Support_10` - 支撑柱系列
- 不同锈蚀程度和颜色

### 🔧 工业材质
- `MI_Pipe_1/2/3` - 管道系列
- `MI_Pipe_4_5` - 管道4-5
- `M_Wires` - 电线

### 🌫️ 环境效果
**M_Cloud** / **MI_Cloud** / **MI_Cloud_2**
- 云雾效果
- 体积雾材质

**M_Volumetric_Fog**
- 体积雾系统
- 适合: 废弃工厂、污染区域

**M_HDRI**
- HDRI环境贴图
- 天空盒材质

**M_For_Overview**
- 场景总览材质

### 🔥 特效材质
**M_CampFire** / **M_FIre**
- 营火/火焰效果
- 发光和粒子配合

**M_candle** / **M_candle_flame**
- 蜡烛火焰
- 小型光源

### 🌊 地形与水体
**M_Landscape**
- 地形材质系统
- 支持多层混合

**图层信息**:
- `Brown_Mud_LayerInfo` - 棕色泥土
- `Brown_Mud_Rocks_LayerInfo` - 泥土+岩石
- `Coast_and_rocks_LayerInfo` - 海岸+岩石

**M_Puddles** / **MI_Puddles_1**
- 水坑材质
- 反射和波纹

### 📦 道具材质
- `MI_Props` - 通用道具
- `MI_Debris` - 碎片
- `MI_Cloth` / `M_Cloth` - 布料

---

## 🏰 Fantastic_Dungeon_Pack 地牢材质

### 🎨 母材质系统

#### 不透明材质
**M_Master_opaque_dungeon** - 基础不透明材质
**M_Master_opaque_normal_dungeon** - 带法线贴图
**M_Master_opaque_normal_metallic_dungeon** - 带法线+金属度
**M_Master_opaque_emissive_dungeon** - 带自发光

#### 透明材质
**M_Master_transparent_dungeon** - 透明材质
**M_Master_transparent_cutout_dungeon** - 镂空材质

#### 特效材质
**M_FX_Master_fire_steam_dungeon** - 火焰/蒸汽
**M_FX_Master_fog_dungeon** - 雾效
**M_FX_Master_particles_dungeon** - 粒子效果

#### 颜色材质
**M_CLR_Master_dungeon** - 颜色主材质
**MI_CLR_yellow_E** - 黄色发光

### 🏛️ 结构材质实例

#### 地板
- `MI_MOD_Floor_01_v1/v2/v3` - 地板变体1-3
- 不同磨损和颜色

#### 墙体
- `MI_MOD_Wall_01_v1` - 墙体变体1
- 石质纹理

#### 装饰
- `MI_MOD_Trim_Stone_01_v1/v2/v3` - 石质装饰条

### 🔥 特效材质实例

#### 火焰系统
**MI_FX_fire_dungeon**
- 火把/火盆效果
- 配合粒子系统

**MI_FX_steam_dungeon**
- 蒸汽效果
- 适合: 地热、管道泄漏

#### 雾效系统
- `MI_FX_fog_dungeon_small` - 小型雾效
- `MI_FX_fog_dungeon_medium` - 中型雾效
- `MI_FX_fog_dungeon_big_01/02` - 大型雾效
- `MI_FX_fog_dungeon_door` - 门缝雾效

**使用场景**:
```
小型: 地面薄雾、角落
中型: 房间雾气
大型: 大厅、洞穴
门缝: 门后神秘感
```

#### 渐变效果
- `MI_FX_gradient_linear_dungeon_01/02` - 线性渐变
- 适合: 光束、魔法效果

#### 粒子材质
**MI_FX_particles_dungeon**
- 通用粒子材质
- 适合: 灰尘、火花、魔法粒子

### 🔩 金属材质
- `MI_metal_dungeon_01/02` - 金属材质变体
- 适合: 铁门、链条、武器

---

## 🎯 材质使用指南

### 后处理材质堆栈

#### 恐怖氛围配置
```cpp
// 后处理体积设置
PostProcessVolume->Settings.WeightedBlendables.Array.Add({
    {1.0f, MI_OldVHS},              // VHS效果
    {0.6f, MI_Noise},               // 噪点
    {0.4f, MI_ChromaticAbberation}, // 色差
    {0.8f, MI_CameraHole}           // 暗角
});
```

#### 监控摄像头效果
```cpp
// 摄像头材质组合
CameraPostProcess = {
    MI_FishEye,           // 鱼眼镜头
    MI_CameraHole,        // 镜头孔洞
    MI_NoiseColor,        // 彩色噪点
    MI_OldVHS             // VHS失真
};
```

#### 精神错乱效果
```cpp
// 动态精神状态
void UpdateMentalState(float SanityLevel) {
    float Intensity = 1.0f - SanityLevel;

    // 色差强度随精神值降低而增加
    ChromaticMaterial->SetScalarParameterValue(
        "Intensity",
        Intensity * 0.02f
    );

    // 噪点增加
    NoiseMaterial->SetScalarParameterValue(
        "NoiseAmount",
        Intensity * 0.5f
    );

    // 扭曲效果
    if (SanityLevel < 0.3f) {
        EnablePostProcessMaterial(MI_Emboss, Intensity);
    }
}
```

### 场景材质组合

#### 水下基地场景
```
结构: DeepWaterStation金属材质
照明: MI_Building_Light (蓝色调)
特效: M_Water04 + M_caustick01
氛围: MI_dust03 (水中悬浮物)
```

#### 废弃工厂场景
```
结构: Scrapopolis锈蚀材质
地面: MI_Puddles_1 (水坑)
照明: MI_LightFlicker_01 (闪烁灯)
氛围: M_Volumetric_Fog
```

#### 地下地牢场景
```
结构: Fantastic_Dungeon石质材质
照明: MI_FX_fire_dungeon (火把)
特效: MI_FX_fog_dungeon_medium
粒子: MI_FX_particles_dungeon
```

### 动态材质实例

#### 损坏进度系统
```cpp
// 随时间损坏的墙体
UFUNCTION()
void UpdateWallDamage(float DamageLevel) {
    UMaterialInstanceDynamic* DynMaterial =
        UMaterialInstanceDynamic::Create(MI_Building_Wall, this);

    DynMaterial->SetScalarParameterValue("DamageAmount", DamageLevel);
    DynMaterial->SetScalarParameterValue("DirtAmount", DamageLevel * 0.8f);
    DynMaterial->SetVectorParameterValue("DamageColor",
        FLinearColor(0.3f, 0.2f, 0.1f));

    WallMesh->SetMaterial(0, DynMaterial);
}
```

#### 灯光闪烁系统
```cpp
// 不稳定灯光
void FlickerLight() {
    if (FMath::FRand() < FlickerChance) {
        float RandomIntensity = FMath::FRandRange(0.2f, 1.0f);
        LightMaterialInstance->SetScalarParameterValue(
            "EmissiveStrength",
            RandomIntensity * BaseIntensity
        );

        // 同步灯光组件
        PointLight->SetIntensity(RandomIntensity * BaseLightIntensity);
    }
}
```

#### 水体动态效果
```cpp
// 水面波动
void UpdateWaterSurface(float DeltaTime) {
    WaterTime += DeltaTime * WaveSpeed;

    WaterMaterial->SetScalarParameterValue("Time", WaterTime);
    WaterMaterial->SetScalarParameterValue("WaveHeight", CurrentWaveHeight);
    WaterMaterial->SetScalarParameterValue("WaveFrequency", WaveFrequency);
}
```

---

## 🎨 材质参数快速参考

### 常用参数名称

#### 基础参数
```
BaseColor / Albedo - 基础颜色
Metallic - 金属度 (0=非金属, 1=金属)
Roughness - 粗糙度 (0=光滑, 1=粗糙)
Specular - 高光强度 (通常0.5)
Normal - 法线贴图
AO / AmbientOcclusion - 环境光遮蔽
```

#### 发光参数
```
EmissiveColor - 发光颜色
EmissiveStrength / Intensity - 发光强度
Glow - 辉光效果
```

#### 透明参数
```
Opacity - 不透明度
OpacityMask - 不透明遮罩
Refraction - 折射
```

#### 特效参数
```
Time - 时间驱动动画
Speed - 速度
Intensity - 强度
Tiling - 平铺
Offset - 偏移
```

### 材质混合模式

```cpp
// 不透明 (Opaque) - 默认，最高性能
BlendMode = BLEND_Opaque

// 遮罩 (Masked) - 镂空效果
BlendMode = BLEND_Masked
OpacityMaskClipValue = 0.5

// 半透明 (Translucent) - 透明效果
BlendMode = BLEND_Translucent

// 叠加 (Additive) - 发光叠加
BlendMode = BLEND_Additive

// 调制 (Modulate) - 颜色调制
BlendMode = BLEND_Modulate
```

---

## 📊 性能优化建议

### 材质复杂度
1. **优先使用材质实例** (MI_) 而非母材质 (M_)
2. **减少纹理采样数量** (建议<16个)
3. **避免复杂数学运算** 在像素着色器中
4. **使用LOD材质** 远距离简化材质

### 后处理优化
```cpp
// 根据性能动态调整后处理
void AdjustPostProcessQuality(int32 QualityLevel) {
    switch (QualityLevel) {
        case 0: // 低
            DisablePostProcess(MI_Emboss);
            DisablePostProcess(MI_Sharpen);
            break;
        case 1: // 中
            EnablePostProcess(MI_OldVHS, 0.5f);
            break;
        case 2: // 高
            EnablePostProcess(MI_OldVHS, 1.0f);
            EnablePostProcess(MI_ChromaticAbberation, 0.6f);
            break;
    }
}
```

### 材质实例动态创建
```cpp
// 缓存动态材质实例
TMap<FName, UMaterialInstanceDynamic*> CachedMaterials;

UMaterialInstanceDynamic* GetOrCreateDynamicMaterial(
    FName MaterialName,
    UMaterialInterface* ParentMaterial
) {
    if (!CachedMaterials.Contains(MaterialName)) {
        CachedMaterials.Add(
            MaterialName,
            UMaterialInstanceDynamic::Create(ParentMaterial, this)
        );
    }
    return CachedMaterials[MaterialName];
}
```

---

## 🎯 快速查找表

| 需求 | 推荐材质 | 资产包 |
|------|----------|--------|
| VHS复古效果 | MI_OldVHS | Bodycam_VHS_Effect |
| 监控摄像头 | MI_FishEye + MI_CameraHole | Bodycam_VHS_Effect |
| 闪烁灯光 | MI_LightFlicker_01 | Bodycam_VHS_Effect |
| 金属墙体 | MI_MetalWall07/08/10 | DeepWaterStation |
| 水体效果 | M_Water04 | DeepWaterStation |
| 玻璃材质 | M_Glass / M_Glass2 | DeepWaterStation |
| 锈蚀金属 | MI_Support_1-10 | Scrapopolis |
| 水坑 | MI_Puddles_1 | Scrapopolis |
| 体积雾 | M_Volumetric_Fog | Scrapopolis |
| 石质地板 | MI_MOD_Floor_01_v1/v2/v3 | Fantastic_Dungeon_Pack |
| 火把效果 | MI_FX_fire_dungeon | Fantastic_Dungeon_Pack |
| 地牢雾气 | MI_FX_fog_dungeon_* | Fantastic_Dungeon_Pack |

---

**文档版本**: 1.0
**最后更新**: 2026-04-28
**材质总数**: 192+
**维护者**: HorrorProject Team
