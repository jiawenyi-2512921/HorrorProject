# 恐惧系统指南

## 概述

FearComponent是HorrorProject的核心恐惧机制，提供动态恐惧值管理、等级系统和视觉/音频反馈联动。

## 核心组件

### FearComponent

玩家角色的恐惧状态管理组件。

**恐惧等级：**
```cpp
enum class EFearLevel : uint8
{
    Calm,       // 平静 (0-20%)
    Uneasy,     // 不安 (20-40%)
    Afraid,     // 害怕 (40-60%)
    Terrified,  // 恐惧 (60-80%)
    Panicked    // 恐慌 (80-100%)
};
```

## 配置参数

### 基础参数

```cpp
// 最大恐惧值
UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(ClampMin="0.0", ClampMax="1000.0"))
float MaxFearValue = 100.0f;

// 恐惧衰减速率（每秒）
UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(ClampMin="0.0"))
float FearDecayRate = 5.0f;

// 恐惧衰减延迟（秒）
UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(ClampMin="0.0", ClampMax="100.0"))
float FearDecayDelay = 3.0f;
```

### 等级阈值

```cpp
// 不安阈值（20%）
UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(ClampMin="0.0", ClampMax="1.0"))
float UneasyThreshold = 0.2f;

// 害怕阈值（40%）
UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(ClampMin="0.0", ClampMax="1.0"))
float AfraidThreshold = 0.4f;

// 恐惧阈值（60%）
UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(ClampMin="0.0", ClampMax="1.0"))
float TerrifiedThreshold = 0.6f;

// 恐慌阈值（80%）
UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(ClampMin="0.0", ClampMax="1.0"))
float PanickedThreshold = 0.8f;
```

### 效果参数

```cpp
// 最小移动速度倍数（恐慌时）
UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(ClampMin="0.0", ClampMax="1.0"))
float MinSpeedMultiplier = 0.7f;

// 最大相机抖动强度
UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(ClampMin="0.0", ClampMax="10.0"))
float MaxCameraShakeIntensity = 1.0f;

// 最大暗角强度
UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(ClampMin="0.0", ClampMax="1.0"))
float MaxVignetteIntensity = 0.6f;

// 最大心跳音量
UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(ClampMin="0.0", ClampMax="1.0"))
float MaxHeartbeatVolume = 1.0f;
```

## 使用指南

### 添加FearComponent到角色

**C++方式：**
```cpp
// 在角色类头文件中
UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Horror")
UFearComponent* FearComponent;

// 在构造函数中创建
AHorrorPlayerCharacter::AHorrorPlayerCharacter()
{
    FearComponent = CreateDefaultSubobject<UFearComponent>(TEXT("FearComponent"));
}
```

**蓝图方式：**
1. 打开角色蓝图
2. 添加组件 > Fear Component
3. 在Details面板配置参数

### 增加恐惧值

```cpp
// 基础增加
FearComponent->AddFear(15.0f);

// 带来源ID（用于追踪）
FearComponent->AddFear(20.0f, TEXT("Golem_01"));

// 检查是否成功
bool bSuccess = FearComponent->AddFear(10.0f);
if (bSuccess)
{
    UE_LOG(LogTemp, Log, TEXT("恐惧值增加成功"));
}
```

### 减少恐惧值

```cpp
// 减少恐惧值（例如：找到安全区域）
FearComponent->RemoveFear(25.0f);

// 直接设置恐惧值
FearComponent->SetFearValue(50.0f);
```

### 查询恐惧状态

```cpp
// 获取当前恐惧值
float CurrentFear = FearComponent->GetFearValue();

// 获取恐惧百分比（0.0-1.0）
float FearPercent = FearComponent->GetFearPercent();

// 获取当前恐惧等级
EFearLevel Level = FearComponent->GetFearLevel();

// 检查是否处于特定等级
bool bIsPanicked = FearComponent->IsFearLevel(EFearLevel::Panicked);
```

### 获取效果参数

```cpp
// 移动速度倍数（用于角色移动）
float SpeedMult = FearComponent->GetMovementSpeedMultiplier();
CharacterMovement->MaxWalkSpeed = BaseSpeed * SpeedMult;

// 相机抖动强度
float ShakeIntensity = FearComponent->GetCameraShakeIntensity();
PlayerController->ClientStartCameraShake(ShakeClass, ShakeIntensity);

// 暗角强度（用于后处理）
float VignetteIntensity = FearComponent->GetVignetteIntensity();
PostProcessMaterial->SetScalarParameterValue(TEXT("VignetteIntensity"), VignetteIntensity);

// 心跳音量
float HeartbeatVolume = FearComponent->GetHeartbeatVolume();
HeartbeatAudioComponent->SetVolumeMultiplier(HeartbeatVolume);
```

## 触发恐惧事件

### 常见恐惧触发场景

**1. 遭遇敌人**
```cpp
void AHorrorThreatCharacter::OnPlayerSpotted(AHorrorPlayerCharacter* Player)
{
    if (UFearComponent* FearComp = Player->FindComponentByClass<UFearComponent>())
    {
        // 根据距离计算恐惧值
        float Distance = FVector::Dist(GetActorLocation(), Player->GetActorLocation());
        float FearAmount = FMath::GetMappedRangeValueClamped(
            FVector2D(0.0f, 2000.0f),    // 距离范围
            FVector2D(30.0f, 10.0f),     // 恐惧值范围
            Distance
        );

        FearComp->AddFear(FearAmount, GetFName());
    }
}
```

**2. 异常事件**
```cpp
void AAnomalyActor::TriggerAnomaly()
{
    // 获取范围内的所有玩家
    TArray<AActor*> Players;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AHorrorPlayerCharacter::StaticClass(), Players);

    for (AActor* Actor : Players)
    {
        AHorrorPlayerCharacter* Player = Cast<AHorrorPlayerCharacter>(Actor);
        if (Player && UFearComponent* FearComp = Player->FearComponent)
        {
            float Distance = FVector::Dist(GetActorLocation(), Player->GetActorLocation());
            if (Distance < AnomalyRadius)
            {
                float FearAmount = 25.0f * (1.0f - Distance / AnomalyRadius);
                FearComp->AddFear(FearAmount, TEXT("Anomaly"));
            }
        }
    }
}
```

**3. 环境恐惧**
```cpp
// 黑暗区域持续增加恐惧
void ADarkZone::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (PlayerInZone && PlayerFearComponent)
    {
        // 每秒增加2点恐惧值
        PlayerFearComponent->AddFear(2.0f * DeltaTime, TEXT("Darkness"));
    }
}
```

**4. 跳吓事件**
```cpp
void AJumpScareActor::ExecuteJumpScare(AHorrorPlayerCharacter* Player)
{
    if (UFearComponent* FearComp = Player->FearComponent)
    {
        // 跳吓造成大量瞬间恐惧
        FearComp->AddFear(40.0f, TEXT("JumpScare"));

        // 播放跳吓动画和音效
        PlayJumpScareEffects();
    }
}
```

**5. 音频触发**
```cpp
void AHorrorAudioTrigger::OnAudioCueHeard(AActor* Listener)
{
    if (AHorrorPlayerCharacter* Player = Cast<AHorrorPlayerCharacter>(Listener))
    {
        if (UFearComponent* FearComp = Player->FearComponent)
        {
            // 根据音频类型增加恐惧
            float FearAmount = 0.0f;
            switch (AudioCueType)
            {
                case EAudioCueType::Whisper:
                    FearAmount = 8.0f;
                    break;
                case EAudioCueType::Scream:
                    FearAmount = 20.0f;
                    break;
                case EAudioCueType::Footsteps:
                    FearAmount = 5.0f;
                    break;
            }

            FearComp->AddFear(FearAmount, TEXT("AudioCue"));
        }
    }
}
```

## 与视觉效果联动

### 后处理材质集成

**创建后处理材质参数：**
```cpp
// 在Tick或定时器中更新
void AHorrorPlayerCharacter::UpdateFearVisuals()
{
    if (!FearComponent || !PostProcessComponent) return;

    float FearPercent = FearComponent->GetFearPercent();
    float VignetteIntensity = FearComponent->GetVignetteIntensity();

    // 更新后处理材质参数
    if (UMaterialInstanceDynamic* PPMaterial = PostProcessComponent->GetMaterialInstance())
    {
        PPMaterial->SetScalarParameterValue(TEXT("VignetteIntensity"), VignetteIntensity);
        PPMaterial->SetScalarParameterValue(TEXT("Desaturation"), FearPercent * 0.5f);
        PPMaterial->SetScalarParameterValue(TEXT("ChromaticAberration"), FearPercent * 0.3f);
    }
}
```

**推荐后处理效果：**
- **Vignette（暗角）**: 随恐惧增加边缘变暗
- **Desaturation（去饱和）**: 高恐惧时降低色彩饱和度
- **Chromatic Aberration（色差）**: 恐慌时产生色差效果
- **Grain（颗粒）**: 增加胶片颗粒感
- **Blur（模糊）**: 轻微的边缘模糊

### 相机抖动

```cpp
void AHorrorPlayerCharacter::UpdateFearCameraShake()
{
    if (!FearComponent) return;

    float ShakeIntensity = FearComponent->GetCameraShakeIntensity();

    if (ShakeIntensity > 0.1f)
    {
        // 根据恐惧等级选择不同的抖动类
        TSubclassOf<UCameraShakeBase> ShakeClass;

        EFearLevel Level = FearComponent->GetFearLevel();
        switch (Level)
        {
            case EFearLevel::Uneasy:
                ShakeClass = CameraShake_Uneasy;
                break;
            case EFearLevel::Afraid:
                ShakeClass = CameraShake_Afraid;
                break;
            case EFearLevel::Terrified:
                ShakeClass = CameraShake_Terrified;
                break;
            case EFearLevel::Panicked:
                ShakeClass = CameraShake_Panicked;
                break;
            default:
                return;
        }

        if (APlayerController* PC = Cast<APlayerController>(GetController()))
        {
            PC->ClientStartCameraShake(ShakeClass, ShakeIntensity);
        }
    }
}
```

### UI反馈

```cpp
// 在HUD或Widget中
void UHorrorHUD::UpdateFearIndicator()
{
    if (!PlayerCharacter || !PlayerCharacter->FearComponent) return;

    float FearPercent = PlayerCharacter->FearComponent->GetFearPercent();
    EFearLevel Level = PlayerCharacter->FearComponent->GetFearLevel();

    // 更新恐惧条
    FearBar->SetPercent(FearPercent);

    // 根据等级改变颜色
    FLinearColor BarColor;
    switch (Level)
    {
        case EFearLevel::Calm:
            BarColor = FLinearColor::Green;
            break;
        case EFearLevel::Uneasy:
            BarColor = FLinearColor::Yellow;
            break;
        case EFearLevel::Afraid:
            BarColor = FLinearColor(1.0f, 0.5f, 0.0f);  // 橙色
            break;
        case EFearLevel::Terrified:
            BarColor = FLinearColor(1.0f, 0.2f, 0.0f);  // 深橙
            break;
        case EFearLevel::Panicked:
            BarColor = FLinearColor::Red;
            break;
    }

    FearBar->SetFillColorAndOpacity(BarColor);

    // 显示等级文本
    FearLevelText->SetText(GetFearLevelText(Level));
}
```

## 与音频效果联动

### 心跳音效

```cpp
void AHorrorPlayerCharacter::UpdateFearAudio()
{
    if (!FearComponent || !HeartbeatAudioComponent) return;

    float HeartbeatVolume = FearComponent->GetHeartbeatVolume();
    float FearPercent = FearComponent->GetFearPercent();

    // 更新心跳音量
    HeartbeatAudioComponent->SetVolumeMultiplier(HeartbeatVolume);

    // 根据恐惧等级调整心跳速度
    float HeartbeatPitch = FMath::Lerp(0.8f, 1.5f, FearPercent);
    HeartbeatAudioComponent->SetPitchMultiplier(HeartbeatPitch);

    // 高恐惧时播放呼吸急促音效
    if (FearComponent->GetFearLevel() >= EFearLevel::Terrified)
    {
        if (!BreathingAudioComponent->IsPlaying())
        {
            BreathingAudioComponent->Play();
        }
    }
    else
    {
        if (BreathingAudioComponent->IsPlaying())
        {
            BreathingAudioComponent->FadeOut(1.0f, 0.0f);
        }
    }
}
```

### 环境音效调制

```cpp
void AHorrorPlayerCharacter::ModulateAmbientAudio()
{
    if (!FearComponent) return;

    float FearPercent = FearComponent->GetFearPercent();

    // 获取音频子系统
    UHorrorAudioSubsystem* AudioSys = GetWorld()->GetSubsystem<UHorrorAudioSubsystem>();
    if (!AudioSys) return;

    // 高恐惧时降低环境音音量，增加异常音效
    float AmbientVolume = FMath::Lerp(1.0f, 0.4f, FearPercent);
    float AnomalyVolume = FMath::Lerp(0.3f, 1.0f, FearPercent);

    AudioSys->SetCategoryVolume(EHorrorAudioCategory::Ambient, AmbientVolume);
    AudioSys->SetCategoryVolume(EHorrorAudioCategory::Anomaly, AnomalyVolume);
}
```

## 事件委托

### 监听恐惧等级变化

```cpp
// 绑定委托
void AHorrorPlayerCharacter::BeginPlay()
{
    Super::BeginPlay();

    if (FearComponent)
    {
        FearComponent->OnFearLevelChanged.AddDynamic(this, &AHorrorPlayerCharacter::HandleFearLevelChanged);
        FearComponent->OnFearValueChanged.AddDynamic(this, &AHorrorPlayerCharacter::HandleFearValueChanged);
    }
}

// 处理等级变化
void AHorrorPlayerCharacter::HandleFearLevelChanged(EFearLevel NewLevel, float FearPercent)
{
    UE_LOG(LogTemp, Log, TEXT("恐惧等级变化: %d, 百分比: %f"), (int32)NewLevel, FearPercent);

    // 触发相应的游戏逻辑
    switch (NewLevel)
    {
        case EFearLevel::Panicked:
            // 进入恐慌状态，可能触发特殊事件
            OnEnterPanicState();
            break;
        case EFearLevel::Calm:
            // 恢复平静，移除负面效果
            OnReturnToCalm();
            break;
    }
}

// 处理恐惧值变化
void AHorrorPlayerCharacter::HandleFearValueChanged(float FearValue, float FearPercent)
{
    // 实时更新UI和效果
    UpdateFearVisuals();
    UpdateFearAudio();
}
```

## 蓝图使用示例

### 增加恐惧值
```
节点: Add Fear
- Target: Fear Component
- Amount: 15.0
- Source Id: "Enemy_01"
- Return Value: Success (bool)
```

### 检查恐惧等级
```
节点: Get Fear Level
- Target: Fear Component
- Return Value: Fear Level (enum)

节点: Is Fear Level
- Target: Fear Component
- Level: Panicked
- Return Value: Is Panicked (bool)
```

### 获取效果参数
```
节点: Get Movement Speed Multiplier
- Target: Fear Component
- Return Value: Speed Multiplier (float)

使用: Set Max Walk Speed
- Target: Character Movement
- Max Walk Speed: Base Speed * Speed Multiplier
```

## 性能优化

### 衰减定时器

FearComponent使用定时器优化恐惧衰减更新：
```cpp
// 仅在需要时启动定时器
void UFearComponent::StartFearDecayTimer()
{
    if (!GetWorld()) return;

    GetWorld()->GetTimerManager().SetTimer(
        FearDecayTimerHandle,
        this,
        &UFearComponent::UpdateFearDecayTimer,
        0.1f,  // 每0.1秒更新一次
        true
    );
}

// 恐惧值为0时停止定时器
void UFearComponent::StopFearDecayTimer()
{
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(FearDecayTimerHandle);
    }
}
```

## 最佳实践

1. **渐进式恐惧累积**：避免瞬间大量增加恐惧值（除跳吓外）
2. **提供恢复机制**：设置安全区域或物品来降低恐惧值
3. **平衡恐惧衰减**：FearDecayRate和FearDecayDelay需要仔细调整
4. **视听联动**：确保视觉和音频效果与恐惧等级同步
5. **测试阈值**：根据游戏节奏调整各等级阈值
6. **使用来源ID**：便于调试和追踪恐惧来源
7. **避免过度效果**：高恐惧时不要让玩家完全失去控制
8. **提供反馈**：让玩家清楚了解当前恐惧状态

## 调试技巧

```cpp
// 在开发构建中显示恐惧调试信息
#if !UE_BUILD_SHIPPING
void UFearComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (GEngine && bShowDebugInfo)
    {
        FString DebugText = FString::Printf(
            TEXT("Fear: %.1f/%.1f (%.0f%%) | Level: %s | Speed: %.2f | Shake: %.2f"),
            FearValue,
            MaxFearValue,
            GetFearPercent() * 100.0f,
            *UEnum::GetValueAsString(CurrentFearLevel),
            GetMovementSpeedMultiplier(),
            GetCameraShakeIntensity()
        );

        GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Yellow, DebugText);
    }
}
#endif
```

## 常见问题

**Q: 恐惧值增加太快？**
A: 调整各触发源的FearAmount值，或增加MaxFearValue。

**Q: 恐惧衰减太慢？**
A: 增加FearDecayRate或减少FearDecayDelay。

**Q: 效果不明显？**
A: 检查MaxCameraShakeIntensity、MaxVignetteIntensity等参数是否过小。

**Q: 恐慌状态持续时间过长？**
A: 降低PanickedThreshold或增加恐惧衰减速率。
