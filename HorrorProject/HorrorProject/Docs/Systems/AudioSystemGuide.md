# 音频系统完整指南

## 概述

HorrorProject的音频系统提供了完整的3D空间音效、动态音乐、区域环境音和事件驱动音频解决方案。

## 核心组件

### 1. HorrorAudioSubsystem

世界子系统，负责协调所有音频播放和管理。

**主要功能：**
- 3D/2D音效播放
- 音频区域管理
- 事件驱动音频
- 音频分类和音量控制
- 音频池和队列管理
- 遮挡系统

**音频分类：**
```cpp
enum class EHorrorAudioCategory : uint8
{
    Ambient,      // 环境音
    Anomaly,      // 异常音效
    Site,         // 站点音效
    Interaction,  // 交互音效
    Escape,       // 逃离音效
    Music         // 音乐
};
```

### 2. HorrorAudioZoneActor

用于定义空间音频区域的Actor。

**配置参数：**
```cpp
UPROPERTY(EditAnywhere, BlueprintReadWrite)
FName ZoneId = TEXT("Zone.Default");

UPROPERTY(EditAnywhere, BlueprintReadWrite)
USoundBase* AmbientSound;

UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0.0", ClampMax="1.0"))
float AmbientVolume = 0.5f;

UPROPERTY(EditAnywhere, BlueprintReadWrite)
bool bLoopAmbient = true;

UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0.0"))
float FadeInDuration = 2.0f;

UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0.0"))
float FadeOutDuration = 1.5f;

UPROPERTY(EditAnywhere, BlueprintReadWrite)
bool bAutoRegister = true;
```

## 使用指南

### 创建音频区域

**步骤1：在关卡中放置HorrorAudioZoneActor**
```cpp
// C++代码示例
AHorrorAudioZoneActor* AudioZone = GetWorld()->SpawnActor<AHorrorAudioZoneActor>();
AudioZone->ZoneId = TEXT("Zone.Basement");
AudioZone->AmbientSound = BasementAmbientSound;
AudioZone->AmbientVolume = 0.6f;
AudioZone->FadeInDuration = 3.0f;
AudioZone->FadeOutDuration = 2.0f;
```

**步骤2：配置区域触发器**
- 使用内置的`UHorrorAudioZoneTrigger`组件
- 当玩家进入区域时自动播放环境音
- 离开时自动淡出

### 播放3D音效

**使用HorrorAudioLibrary（蓝图友好）：**
```cpp
// 播放3D音效
UAudioComponent* AudioComp = UHorrorAudioLibrary::PlayHorrorSound3D(
    this,
    MySound,
    FVector(100, 200, 50),
    1.0f  // 音量倍数
);

// 播放2D音效
UAudioComponent* AudioComp2D = UHorrorAudioLibrary::PlayHorrorSound2D(
    this,
    UISound,
    0.8f
);
```

**直接使用Subsystem：**
```cpp
UHorrorAudioSubsystem* AudioSys = GetWorld()->GetSubsystem<UHorrorAudioSubsystem>();

// 在指定位置播放
UAudioComponent* Comp = AudioSys->PlaySoundAtLocation(
    MySound,
    FVector(0, 0, 100),
    1.0f,  // 音量
    1.0f   // 音调
);

// 附加到组件播放
UAudioComponent* AttachedComp = AudioSys->PlaySoundAttached(
    FootstepSound,
    CharacterMesh,
    TEXT("foot_r"),
    1.0f
);
```

### 事件驱动音频

**注册事件映射：**
```cpp
FHorrorAudioEventMapping Mapping;
Mapping.EventTag = FGameplayTag::RequestGameplayTag(TEXT("Event.Door.Open"));
Mapping.Category = EHorrorAudioCategory::Interaction;
Mapping.Sound = DoorOpenSound;
Mapping.VolumeMultiplier = 0.8f;
Mapping.bUse3DAttenuation = true;
Mapping.AttenuationRadius = 1500.0f;

AudioSubsystem->RegisterEventMapping(Mapping);
```

**触发事件音频：**
```cpp
// 通过蓝图库
UHorrorAudioLibrary::PlayHorrorEventSound(
    this,
    FGameplayTag::RequestGameplayTag(TEXT("Event.Door.Open")),
    DoorActor
);

// 直接调用
AudioSubsystem->PlayEventSound(
    FGameplayTag::RequestGameplayTag(TEXT("Event.Anomaly.Detected")),
    AnomalySource
);
```

### 音频区域切换

```cpp
UHorrorAudioSubsystem* AudioSys = GetWorld()->GetSubsystem<UHorrorAudioSubsystem>();

// 进入新区域
AudioSys->EnterAudioZone(TEXT("Zone.Corridor"));

// 离开区域
AudioSys->ExitAudioZone(TEXT("Zone.Corridor"));

// 停止所有环境音
AudioSys->StopAllAmbient(2.0f);  // 2秒淡出
```

### 分类音量控制

```cpp
// 设置分类音量
AudioSubsystem->SetCategoryVolume(EHorrorAudioCategory::Ambient, 0.7f);
AudioSubsystem->SetCategoryVolume(EHorrorAudioCategory::Music, 0.5f);

// 获取分类音量
float AmbientVol = AudioSubsystem->GetCategoryVolume(EHorrorAudioCategory::Ambient);
```

### 优先级和队列系统

```cpp
// 播放高优先级音效
UAudioComponent* HighPrioritySound = AudioSubsystem->PlaySoundWithPriority(
    ImportantSound,
    Location,
    100,  // 优先级（默认50）
    1.0f
);

// 将音效加入队列（当达到并发限制时）
AudioSubsystem->QueueSound(
    QueuedSound,
    Location,
    50,   // 优先级
    1.0f
);
```

### 音频遮挡

```cpp
// 启用/禁用遮挡
AudioSubsystem->SetOcclusionEnabled(true);

// 手动更新遮挡（通常自动处理）
AudioSubsystem->UpdateOcclusion(DeltaTime);
```

**遮挡配置参数：**
- `bEnableOcclusion`: 启用遮挡（默认true）
- `OcclusionUpdateRate`: 更新频率（默认0.1秒）
- `OcclusionVolumeMultiplier`: 遮挡时音量倍数（默认0.3）
- `OcclusionInterpSpeed`: 插值速度（默认8.0）

### 音频预加载

```cpp
// 预加载音效资源
AudioSubsystem->PreloadSound(HeavySound);

// 卸载音效
AudioSubsystem->UnloadSound(HeavySound);
```

## SoundsOfHorror资产库索引

### 推荐资产组织结构

```
Content/Audio/
├── Ambient/
│   ├── Zone_Basement.uasset
│   ├── Zone_Corridor.uasset
│   └── Zone_Exterior.uasset
├── Anomaly/
│   ├── Whisper_01.uasset
│   ├── Static_Burst.uasset
│   └── Reality_Tear.uasset
├── Interaction/
│   ├── Door_Open.uasset
│   ├── Door_Close.uasset
│   └── Item_Pickup.uasset
├── Footsteps/
│   ├── Concrete_Walk.uasset
│   ├── Metal_Walk.uasset
│   └── Water_Walk.uasset
└── Music/
    ├── Exploration_Layer.uasset
    ├── Tension_Layer.uasset
    └── Chase_Layer.uasset
```

### 音效命名规范

- **环境音**: `AMB_[Location]_[Variant]`
- **异常音效**: `ANM_[Type]_[Intensity]`
- **交互音效**: `INT_[Object]_[Action]`
- **脚步声**: `FS_[Surface]_[Movement]`
- **音乐**: `MUS_[Mood]_[Layer]`

## 3D音效配置

### 衰减设置

**创建自定义衰减配置：**
```cpp
// 在编辑器中创建SoundAttenuation资产
// 或在代码中配置
FSoundAttenuationSettings AttenuationSettings;
AttenuationSettings.bAttenuate = true;
AttenuationSettings.AttenuationShape = EAttenuationShape::Sphere;
AttenuationSettings.FalloffDistance = 2000.0f;  // 衰减距离（厘米）
AttenuationSettings.AttenuationShapeExtents = FVector(2000.0f);
```

**推荐衰减距离：**
- 小型交互音效: 500-1000cm
- 脚步声: 1000-1500cm
- 门/物体音效: 1500-2000cm
- 异常音效: 2000-3000cm
- 环境音: 3000-5000cm

### 空间化配置

```cpp
// 启用双耳音频（HRTF）
// 在项目设置 > Audio > Spatialization Plugin
// 选择 "Built-in Spatialization" 或第三方插件

// 在Sound资产中启用
Sound->bEnableSpatialization = true;
Sound->SpatializationAlgorithm = ESoundSpatializationAlgorithm::SPATIALIZATION_HRTF;
```

## 动态音乐系统

### Day1音频阶段系统

```cpp
enum class EHorrorDay1AudioStage : uint8
{
    Exploration,  // 探索
    Objective,    // 目标
    Anomaly,      // 异常
    Chase,        // 追逐
    Resolved,     // 已解决
    Escape,       // 逃离
    Complete,     // 完成
    Failure       // 失败
};
```

**切换音乐阶段：**
```cpp
AudioSubsystem->SetDay1AudioStage(
    EHorrorDay1AudioStage::Chase,
    FGameplayTag::RequestGameplayTag(TEXT("Event.Enemy.Spotted")),
    TEXT("Golem_01")
);

// 获取当前阶段
EHorrorDay1AudioStage CurrentStage = AudioSubsystem->GetDay1AudioStage();
```

**处理Day1事件：**
```cpp
// 通过GameplayTag
AudioSubsystem->HandleDay1Event(
    FGameplayTag::RequestGameplayTag(TEXT("Event.Objective.Complete")),
    TEXT("Objective_Main")
);

// 通过事件名称
AudioSubsystem->HandleDay1EventName(
    TEXT("AnomalyDetected"),
    TEXT("Anomaly_Temporal")
);
```

### 音乐层混合

**实现动态音乐层：**
```cpp
// 注册不同强度的音乐层
FHorrorAudioEventMapping ExplorationLayer;
ExplorationLayer.EventTag = FGameplayTag::RequestGameplayTag(TEXT("Music.Exploration"));
ExplorationLayer.Category = EHorrorAudioCategory::Music;
ExplorationLayer.Sound = ExplorationMusic;
ExplorationLayer.VolumeMultiplier = 0.6f;
ExplorationLayer.bUse3DAttenuation = false;

FHorrorAudioEventMapping TensionLayer;
TensionLayer.EventTag = FGameplayTag::RequestGameplayTag(TEXT("Music.Tension"));
TensionLayer.Category = EHorrorAudioCategory::Music;
TensionLayer.Sound = TensionMusic;
TensionLayer.VolumeMultiplier = 0.8f;
TensionLayer.bUse3DAttenuation = false;

AudioSubsystem->RegisterEventMapping(ExplorationLayer);
AudioSubsystem->RegisterEventMapping(TensionLayer);

// 根据游戏状态切换
AudioSubsystem->PlayEventSound(
    FGameplayTag::RequestGameplayTag(TEXT("Music.Tension")),
    nullptr
);
```

## 性能优化

### 音频池配置

```cpp
// 在HorrorAudioSubsystem中配置
MaxPooledComponents = 32;      // 最大池化组件数
PoolCleanupInterval = 10.0f;   // 清理间隔（秒）
```

### 并发限制

```cpp
MaxConcurrentSounds = 16;  // 最大同时播放音效数
```

### 调试工具

```cpp
// 获取活动音频组件数量
int32 ActiveCount = AudioSubsystem->GetActiveAudioComponentCount();

// 获取当前区域ID
FName CurrentZone = AudioSubsystem->GetCurrentZoneId();

// 获取最后的Day1事件信息
FGameplayTag LastEvent = AudioSubsystem->GetLastDay1AudioEventTag();
FName LastSource = AudioSubsystem->GetLastDay1AudioSourceId();
```

## 蓝图使用示例

### 播放区域环境音

```
节点: Play Horror Sound 3D
- World Context Object: Self
- Sound: [选择音效资产]
- Location: (X=0, Y=0, Z=100)
- Volume Multiplier: 1.0
```

### 进入音频区域

```
节点: Enter Horror Audio Zone
- World Context Object: Self
- Zone Id: "Zone.Basement"
```

### 设置分类音量

```
节点: Set Horror Category Volume
- World Context Object: Self
- Category: Ambient
- Volume: 0.7
```

## 常见问题

**Q: 音效没有3D空间感？**
A: 检查Sound资产的Attenuation Settings，确保启用了衰减和空间化。

**Q: 音效播放延迟？**
A: 使用PreloadSound预加载重要音效，或增加MaxPooledComponents。

**Q: 音效被截断？**
A: 检查MaxConcurrentSounds限制，或提高音效的Priority值。

**Q: 环境音切换不平滑？**
A: 调整FadeInDuration和FadeOutDuration参数，增加过渡时间。

## 最佳实践

1. **使用音频区域管理环境音**：避免手动管理复杂的环境音切换逻辑
2. **利用事件系统**：将音效与GameplayTag绑定，实现解耦的音频触发
3. **合理设置优先级**：重要音效（对话、关键事件）使用高优先级
4. **预加载关键音效**：在关卡加载时预加载频繁使用的音效
5. **使用音频分类**：便于统一控制不同类型音效的音量
6. **测试遮挡效果**：确保墙壁和障碍物正确遮挡音效
7. **优化并发数量**：根据目标平台调整MaxConcurrentSounds
8. **使用音频池**：减少运行时创建/销毁AudioComponent的开销
