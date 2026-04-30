# 相机系统指南

## 概述

HorrorProject的相机系统提供量子相机（QuantumCamera）、录制功能、VHS效果和照片拍摄等完整功能。

## 核心组件

### 1. QuantumCameraComponent

量子相机核心组件，管理相机状态和模式切换。

**相机模式：**
```cpp
enum class EQuantumCameraMode : uint8
{
    Disabled,   // 已禁用
    Viewfinder, // 取景
    Recording,  // 录制
    Photo,      // 拍照
    Rewind      // 倒带
};
```

### 2. CameraRecordingComponent

相机录制组件，处理录像和倒带功能。

**主要功能：**
- 帧捕获和缓冲
- 录像时长管理
- 倒带播放
- 录制元数据

### 3. VHSEffectComponent

VHS视觉效果组件，提供复古录像带效果。

**主要功能：**
- 后处理材质管理
- VHS噪点生成
- 相机反馈效果
- Bodycam状态管理

### 4. CameraPhotoComponent

照片拍摄组件（如果项目中使用）。

## QuantumCameraComponent详解

### 配置和状态

**相机获取和启用：**
```cpp
// 设置相机已获取（玩家拾取相机道具时）
QuantumCamera->SetCameraAcquired(true);

// 检查是否已获取
bool bHasCamera = QuantumCamera->IsCameraAcquired();

// 启用/禁用相机
QuantumCamera->SetCameraEnabled(true);

// 检查是否已启用
bool bIsEnabled = QuantumCamera->IsCameraEnabled();
```

### 模式切换

```cpp
// 切换到取景模式
QuantumCamera->SetCameraMode(EQuantumCameraMode::Viewfinder);

// 切换到录制模式
QuantumCamera->SetCameraMode(EQuantumCameraMode::Recording);

// 切换到拍照模式
QuantumCamera->SetCameraMode(EQuantumCameraMode::Photo);

// 获取当前模式
EQuantumCameraMode CurrentMode = QuantumCamera->GetCameraMode();

// 检查是否处于特定模式
bool bIsRecording = QuantumCamera->IsCameraMode(EQuantumCameraMode::Recording);
```

### 录制控制

```cpp
// 开始录制
bool bStarted = QuantumCamera->StartRecording();
if (bStarted)
{
    UE_LOG(LogTemp, Log, TEXT("录制已开始"));
}

// 停止录制
bool bStopped = QuantumCamera->StopRecording();
if (bStopped)
{
    UE_LOG(LogTemp, Log, TEXT("录制已停止"));
}
```

### 拍照功能

```cpp
// 拍摄照片
bool bPhotoTaken = QuantumCamera->TakePhoto();
if (bPhotoTaken)
{
    // 播放快门音效
    UGameplayStatics::PlaySound2D(this, ShutterSound);
}
```

### 倒带功能

```cpp
// 开始倒带
bool bRewindStarted = QuantumCamera->StartRewind();

// 停止倒带
bool bRewindStopped = QuantumCamera->StopRewind();
```

### 电池系统集成

```cpp
// 获取电池组件
UCameraBatteryComponent* Battery = QuantumCamera->GetBatteryComponent();

if (Battery)
{
    float BatteryPercent = Battery->GetBatteryPercent();
    UE_LOG(LogTemp, Log, TEXT("电池电量: %.0f%%"), BatteryPercent * 100.0f);
}
```

### 事件委托

```cpp
// 绑定模式变化事件
QuantumCamera->OnCameraModeChanged.AddDynamic(this, &AHorrorPlayerCharacter::HandleCameraModeChanged);

// 绑定获取状态变化
QuantumCamera->OnCameraAcquiredChanged.AddDynamic(this, &AHorrorPlayerCharacter::HandleCameraAcquiredChanged);

// 绑定启用状态变化
QuantumCamera->OnCameraEnabledChanged.AddDynamic(this, &AHorrorPlayerCharacter::HandleCameraEnabledChanged);

// 处理函数
void AHorrorPlayerCharacter::HandleCameraModeChanged(EQuantumCameraMode NewMode)
{
    UE_LOG(LogTemp, Log, TEXT("相机模式变更: %d"), (int32)NewMode);

    // 根据模式更新UI
    UpdateCameraUI(NewMode);
}
```

## CameraRecordingComponent详解

### 配置参数

```cpp
// 最大录制时长（秒）
UPROPERTY(EditDefaultsOnly, Category="CameraRecording|Settings")
float MaxRecordingDuration = 30.0f;

// 帧捕获率（FPS）
UPROPERTY(EditDefaultsOnly, Category="CameraRecording|Settings")
float FrameCaptureRate = 30.0f;

// 最大缓冲帧数
UPROPERTY(EditDefaultsOnly, Category="CameraRecording|Settings")
int32 MaxBufferFrames = 900;

// 倒带速度倍数
UPROPERTY(EditDefaultsOnly, Category="CameraRecording|Settings")
float RewindSpeed = 2.0f;
```

### 录制控制

```cpp
// 开始录制
bool bStarted = RecordingComponent->StartRecording();

// 停止录制
bool bStopped = RecordingComponent->StopRecording();

// 检查录制状态
bool bIsRecording = RecordingComponent->IsRecording();

// 获取当前录制时长
float Duration = RecordingComponent->GetCurrentRecordingDuration();

// 获取录制进度（0.0-1.0）
float Progress = RecordingComponent->GetRecordingProgress();

// 检查是否有录制内容
bool bHasRecording = RecordingComponent->HasRecording();

// 清除录制内容
RecordingComponent->ClearRecording();
```

### 倒带控制

```cpp
// 开始倒带
bool bRewindStarted = RecordingComponent->StartRewind();

// 停止倒带
bool bRewindStopped = RecordingComponent->StopRewind();

// 检查倒带状态
bool bIsRewinding = RecordingComponent->IsRewinding();
```

### 录制元数据

```cpp
// 获取录制元数据
FCameraRecordingMetadata Metadata = RecordingComponent->GetRecordingMetadata();

UE_LOG(LogTemp, Log, TEXT("录制信息:"));
UE_LOG(LogTemp, Log, TEXT("  总时长: %.2f秒"), Metadata.TotalDuration);
UE_LOG(LogTemp, Log, TEXT("  帧数: %d"), Metadata.FrameCount);
UE_LOG(LogTemp, Log, TEXT("  开始时间: %s"), *Metadata.RecordingStartTime.ToString());
UE_LOG(LogTemp, Log, TEXT("  开始位置: %s"), *Metadata.StartLocation.ToString());
UE_LOG(LogTemp, Log, TEXT("  包含音频: %s"), Metadata.bHasAudio ? TEXT("是") : TEXT("否"));
```

### 音效配置

```cpp
// 录制开始音效
UPROPERTY(EditDefaultsOnly, Category="CameraRecording|Audio")
USoundBase* RecordingStartSound;

// 录制停止音效
UPROPERTY(EditDefaultsOnly, Category="CameraRecording|Audio")
USoundBase* RecordingStopSound;

// 倒带音效
UPROPERTY(EditDefaultsOnly, Category="CameraRecording|Audio")
USoundBase* RewindSound;
```

### 事件标签

```cpp
// 配置GameplayTag事件
UPROPERTY(EditDefaultsOnly, Category="CameraRecording|Events")
FGameplayTag RecordingStartedEventTag;

UPROPERTY(EditDefaultsOnly, Category="CameraRecording|Events")
FGameplayTag RecordingStoppedEventTag;

UPROPERTY(EditDefaultsOnly, Category="CameraRecording|Events")
FGameplayTag RewindStartedEventTag;

UPROPERTY(EditDefaultsOnly, Category="CameraRecording|Events")
FGameplayTag RewindStoppedEventTag;
```

### 事件委托

```cpp
// 绑定录制事件
RecordingComponent->OnRecordingStarted.AddDynamic(this, &AHorrorPlayerCharacter::HandleRecordingStarted);
RecordingComponent->OnRecordingStopped.AddDynamic(this, &AHorrorPlayerCharacter::HandleRecordingStopped);
RecordingComponent->OnRecordingProgress.AddDynamic(this, &AHorrorPlayerCharacter::HandleRecordingProgress);

// 绑定倒带事件
RecordingComponent->OnRewindStarted.AddDynamic(this, &AHorrorPlayerCharacter::HandleRewindStarted);
RecordingComponent->OnRewindStopped.AddDynamic(this, &AHorrorPlayerCharacter::HandleRewindStopped);
RecordingComponent->OnRewindProgress.AddDynamic(this, &AHorrorPlayerCharacter::HandleRewindProgress);

// 处理函数
void AHorrorPlayerCharacter::HandleRecordingStarted()
{
    UE_LOG(LogTemp, Log, TEXT("录制开始"));
    ShowRecordingIndicator(true);
}

void AHorrorPlayerCharacter::HandleRecordingStopped(float Duration)
{
    UE_LOG(LogTemp, Log, TEXT("录制停止，时长: %.2f秒"), Duration);
    ShowRecordingIndicator(false);
}

void AHorrorPlayerCharacter::HandleRecordingProgress(float CurrentDuration)
{
    // 更新录制进度UI
    UpdateRecordingProgressBar(CurrentDuration / RecordingComponent->GetMaxRecordingDuration());
}
```

## VHSEffectComponent详解

### Bodycam状态管理

```cpp
// 设置Bodycam已获取
VHSEffect->SetBodycamAcquired(true);

// 检查是否已获取
bool bHasBodycam = VHSEffect->IsBodycamAcquired();

// 启用/禁用Bodycam
VHSEffect->SetBodycamEnabled(true);

// 检查是否已启用
bool bIsEnabled = VHSEffect->IsBodycamEnabled();
```

### 反馈效果

```cpp
// 设置反馈相机模式
VHSEffect->SetFeedbackCameraMode(EQuantumCameraMode::Recording);

// 检查反馈是否激活
bool bFeedbackActive = VHSEffect->IsFeedbackActive();

// 获取反馈相机模式
EQuantumCameraMode FeedbackMode = VHSEffect->GetFeedbackCameraMode();
```

### 后处理材质

```cpp
// 配置VHS后处理材质
UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="VHS|PostProcess")
UMaterialInterface* VHSPostProcessMaterial;

// 后处理混合权重
UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="VHS|PostProcess", meta=(ClampMin="0.0", ClampMax="1.0"))
float VHSPostProcessBlendWeight = 1.0f;

// 绑定后处理相机
VHSEffect->BindPostProcessCamera(CameraComponent);

// 解析默认后处理材质
VHSEffect->ResolveDefaultPostProcessMaterial();

// 刷新后处理绑定
VHSEffect->RefreshPostProcessBinding();

// 获取绑定的相机
UCameraComponent* BoundCamera = VHSEffect->GetBoundPostProcessCamera();
```

### 噪点生成器

```cpp
// 初始化噪点生成器
VHSEffect->InitializeNoiseGenerator();

// 获取噪点生成器
UVHSNoiseGenerator* NoiseGen = VHSEffect->GetNoiseGenerator();

// 更新噪点（通常在Tick中调用）
void AHorrorPlayerCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (VHSEffect)
    {
        float StressLevel = FearComponent ? FearComponent->GetFearPercent() : 0.0f;
        float BatteryLevel = QuantumCamera ? QuantumCamera->GetBatteryComponent()->GetBatteryPercent() : 1.0f;

        VHSEffect->UpdateNoiseGenerator(DeltaTime, StressLevel, BatteryLevel);
    }
}
```

### 事件委托

```cpp
// 绑定Bodycam事件
VHSEffect->OnBodycamAcquiredChanged.AddDynamic(this, &AHorrorPlayerCharacter::HandleBodycamAcquiredChanged);
VHSEffect->OnBodycamEnabledChanged.AddDynamic(this, &AHorrorPlayerCharacter::HandleBodycamEnabledChanged);
VHSEffect->OnVHSFeedbackChanged.AddDynamic(this, &AHorrorPlayerCharacter::HandleVHSFeedbackChanged);

// 处理函数
void AHorrorPlayerCharacter::HandleVHSFeedbackChanged(bool bIsActive, EQuantumCameraMode CameraMode)
{
    UE_LOG(LogTemp, Log, TEXT("VHS反馈变化: %s, 模式: %d"),
        bIsActive ? TEXT("激活") : TEXT("未激活"),
        (int32)CameraMode);
}
```

## VHS效果配置

### 后处理材质参数

**推荐材质参数：**
```cpp
// 在材质中创建以下参数
- VHSIntensity (Scalar): VHS效果强度 (0.0-1.0)
- NoiseIntensity (Scalar): 噪点强度 (0.0-1.0)
- ScanlineIntensity (Scalar): 扫描线强度 (0.0-1.0)
- ChromaticAberration (Scalar): 色差强度 (0.0-1.0)
- Distortion (Scalar): 画面扭曲 (0.0-1.0)
- Vignette (Scalar): 暗角强度 (0.0-1.0)
- DateTimeOverlay (Texture): 日期时间叠加
- RecordingIndicator (Scalar): 录制指示器 (0.0-1.0)
```

**动态更新材质参数：**
```cpp
void AHorrorPlayerCharacter::UpdateVHSMaterialParameters()
{
    if (!VHSEffect) return;

    UCameraComponent* Camera = VHSEffect->GetBoundPostProcessCamera();
    if (!Camera) return;

    // 获取动态材质实例
    UMaterialInstanceDynamic* DynMaterial = nullptr;
    if (Camera->PostProcessSettings.WeightedBlendables.Array.Num() > 0)
    {
        DynMaterial = Cast<UMaterialInstanceDynamic>(
            Camera->PostProcessSettings.WeightedBlendables.Array[0].Object
        );
    }

    if (DynMaterial)
    {
        // 根据相机模式调整参数
        EQuantumCameraMode Mode = QuantumCamera->GetCameraMode();

        float VHSIntensity = 0.0f;
        float NoiseIntensity = 0.0f;

        switch (Mode)
        {
            case EQuantumCameraMode::Viewfinder:
                VHSIntensity = 0.3f;
                NoiseIntensity = 0.2f;
                break;
            case EQuantumCameraMode::Recording:
                VHSIntensity = 0.6f;
                NoiseIntensity = 0.4f;
                break;
            case EQuantumCameraMode::Rewind:
                VHSIntensity = 0.8f;
                NoiseIntensity = 0.6f;
                break;
        }

        // 根据电池电量调整噪点
        if (UCameraBatteryComponent* Battery = QuantumCamera->GetBatteryComponent())
        {
            float BatteryPercent = Battery->GetBatteryPercent();
            NoiseIntensity += (1.0f - BatteryPercent) * 0.3f;
        }

        // 根据恐惧值调整扭曲
        float Distortion = 0.0f;
        if (FearComponent)
        {
            Distortion = FearComponent->GetFearPercent() * 0.2f;
        }

        // 更新材质参数
        DynMaterial->SetScalarParameterValue(TEXT("VHSIntensity"), VHSIntensity);
        DynMaterial->SetScalarParameterValue(TEXT("NoiseIntensity"), NoiseIntensity);
        DynMaterial->SetScalarParameterValue(TEXT("Distortion"), Distortion);
        DynMaterial->SetScalarParameterValue(TEXT("RecordingIndicator"),
            RecordingComponent->IsRecording() ? 1.0f : 0.0f);
    }
}
```

### 创建VHS后处理材质

**材质节点设置示例：**
1. 创建新材质，Domain设置为Post Process
2. 添加SceneTexture节点（PostProcessInput0）
3. 添加噪点纹理和扫描线效果
4. 使用Time节点创建动态效果
5. 添加色差效果（分离RGB通道并偏移）
6. 添加暗角效果
7. 混合所有效果并输出到Emissive Color

## 完整使用示例

### 玩家角色集成

```cpp
// HorrorPlayerCharacter.h
UCLASS()
class AHorrorPlayerCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Camera")
    UQuantumCameraComponent* QuantumCamera;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Camera")
    UCameraRecordingComponent* RecordingComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Camera")
    UVHSEffectComponent* VHSEffect;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Camera")
    UCameraComponent* FirstPersonCamera;

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

    // 输入处理
    void OnToggleCamera();
    void OnStartRecording();
    void OnStopRecording();
    void OnTakePhoto();
    void OnStartRewind();

    // 事件处理
    UFUNCTION()
    void HandleCameraModeChanged(EQuantumCameraMode NewMode);

    UFUNCTION()
    void HandleRecordingStarted();

    UFUNCTION()
    void HandleRecordingStopped(float Duration);
};

// HorrorPlayerCharacter.cpp
AHorrorPlayerCharacter::AHorrorPlayerCharacter()
{
    // 创建组件
    FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
    FirstPersonCamera->SetupAttachment(GetMesh(), TEXT("head"));

    QuantumCamera = CreateDefaultSubobject<UQuantumCameraComponent>(TEXT("QuantumCamera"));
    RecordingComponent = CreateDefaultSubobject<UCameraRecordingComponent>(TEXT("RecordingComponent"));
    VHSEffect = CreateDefaultSubobject<UVHSEffectComponent>(TEXT("VHSEffect"));
}

void AHorrorPlayerCharacter::BeginPlay()
{
    Super::BeginPlay();

    // 绑定VHS效果到相机
    VHSEffect->BindPostProcessCamera(FirstPersonCamera);
    VHSEffect->ResolveDefaultPostProcessMaterial();

    // 绑定事件
    QuantumCamera->OnCameraModeChanged.AddDynamic(this, &AHorrorPlayerCharacter::HandleCameraModeChanged);
    RecordingComponent->OnRecordingStarted.AddDynamic(this, &AHorrorPlayerCharacter::HandleRecordingStarted);
    RecordingComponent->OnRecordingStopped.AddDynamic(this, &AHorrorPlayerCharacter::HandleRecordingStopped);
}

void AHorrorPlayerCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // 更新VHS效果
    if (VHSEffect && QuantumCamera)
    {
        float StressLevel = FearComponent ? FearComponent->GetFearPercent() : 0.0f;
        float BatteryLevel = 1.0f;

        if (UCameraBatteryComponent* Battery = QuantumCamera->GetBatteryComponent())
        {
            BatteryLevel = Battery->GetBatteryPercent();
        }

        VHSEffect->UpdateNoiseGenerator(DeltaTime, StressLevel, BatteryLevel);
    }

    // 更新材质参数
    UpdateVHSMaterialParameters();
}

void AHorrorPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    // 绑定输入
    PlayerInputComponent->BindAction("ToggleCamera", IE_Pressed, this, &AHorrorPlayerCharacter::OnToggleCamera);
    PlayerInputComponent->BindAction("StartRecording", IE_Pressed, this, &AHorrorPlayerCharacter::OnStartRecording);
    PlayerInputComponent->BindAction("StopRecording", IE_Pressed, this, &AHorrorPlayerCharacter::OnStopRecording);
    PlayerInputComponent->BindAction("TakePhoto", IE_Pressed, this, &AHorrorPlayerCharacter::OnTakePhoto);
    PlayerInputComponent->BindAction("StartRewind", IE_Pressed, this, &AHorrorPlayerCharacter::OnStartRewind);
}

void AHorrorPlayerCharacter::OnToggleCamera()
{
    if (!QuantumCamera->IsCameraAcquired()) return;

    bool bNewEnabled = !QuantumCamera->IsCameraEnabled();
    QuantumCamera->SetCameraEnabled(bNewEnabled);
    VHSEffect->SetBodycamEnabled(bNewEnabled);
}

void AHorrorPlayerCharacter::OnStartRecording()
{
    if (!QuantumCamera->IsCameraEnabled()) return;

    if (QuantumCamera->StartRecording())
    {
        UE_LOG(LogTemp, Log, TEXT("开始录制"));
    }
}

void AHorrorPlayerCharacter::OnStopRecording()
{
    if (QuantumCamera->StopRecording())
    {
        UE_LOG(LogTemp, Log, TEXT("停止录制"));
    }
}

void AHorrorPlayerCharacter::OnTakePhoto()
{
    if (!QuantumCamera->IsCameraEnabled()) return;

    if (QuantumCamera->TakePhoto())
    {
        UGameplayStatics::PlaySound2D(this, CameraShutterSound);
    }
}

void AHorrorPlayerCharacter::OnStartRewind()
{
    if (!RecordingComponent->HasRecording()) return;

    if (QuantumCamera->StartRewind())
    {
        UE_LOG(LogTemp, Log, TEXT("开始倒带"));
    }
}

void AHorrorPlayerCharacter::HandleCameraModeChanged(EQuantumCameraMode NewMode)
{
    // 更新UI显示
    UpdateCameraUI(NewMode);

    // 更新VHS反馈
    VHSEffect->SetFeedbackCameraMode(NewMode);
}

void AHorrorPlayerCharacter::HandleRecordingStarted()
{
    ShowRecordingIndicator(true);
}

void AHorrorPlayerCharacter::HandleRecordingStopped(float Duration)
{
    ShowRecordingIndicator(false);
    ShowRecordingSavedNotification(Duration);
}
```

## 蓝图使用示例

### 切换相机模式
```
节点: Set Camera Mode
- Target: Quantum Camera Component
- New Mode: Recording
- Return Value: Success (bool)
```

### 开始录制
```
节点: Start Recording
- Target: Quantum Camera Component
- Return Value: Success (bool)
```

### 检查录制状态
```
节点: Is Recording
- Target: Camera Recording Component
- Return Value: Is Recording (bool)
```

### 获取录制进度
```
节点: Get Recording Progress
- Target: Camera Recording Component
- Return Value: Progress (float 0.0-1.0)
```

## 性能优化

### 帧缓冲管理

```cpp
// 限制最大缓冲帧数
MaxBufferFrames = 900;  // 30秒 @ 30fps

// 动态调整帧率
if (bLowPerformanceMode)
{
    FrameCaptureRate = 15.0f;  // 降低到15fps
}
```

### 后处理优化

```cpp
// 根据平台调整VHS效果质量
#if PLATFORM_MOBILE
    VHSPostProcessBlendWeight = 0.5f;  // 移动平台降低强度
#else
    VHSPostProcessBlendWeight = 1.0f;
#endif
```

## 最佳实践

1. **电池系统集成**：相机功能应消耗电池，增加资源管理深度
2. **模式切换反馈**：提供清晰的视觉和音频反馈
3. **录制时长限制**：合理设置MaxRecordingDuration避免内存问题
4. **VHS效果强度**：根据游戏风格调整效果强度，避免过度
5. **性能监控**：监控帧捕获和后处理的性能影响
6. **保存录制数据**：实现录制内容的保存和加载
7. **UI反馈**：显示录制状态、剩余时间、电池电量等信息
8. **输入响应**：确保相机操作输入响应流畅

## 常见问题

**Q: VHS效果不显示？**
A: 检查VHSPostProcessMaterial是否正确设置，BindPostProcessCamera是否调用。

**Q: 录制帧率不稳定？**
A: 调整FrameCaptureRate和MaxBufferFrames参数，考虑降低捕获分辨率。

**Q: 倒带播放卡顿？**
A: 增加RewindSpeed或优化帧数据结构，考虑使用压缩。

**Q: 相机模式切换无效？**
A: 检查IsCameraAcquired和IsCameraEnabled状态，确保相机已启用。

## 调试技巧

```cpp
// 显示相机调试信息
#if !UE_BUILD_SHIPPING
void AHorrorPlayerCharacter::DisplayCameraDebugInfo()
{
    if (!GEngine || !QuantumCamera || !RecordingComponent) return;

    FString DebugText = FString::Printf(
        TEXT("相机状态:\n")
        TEXT("  已获取: %s | 已启用: %s\n")
        TEXT("  模式: %s\n")
        TEXT("  录制中: %s | 时长: %.1f/%.1f秒\n")
        TEXT("  倒带中: %s\n")
        TEXT("  缓冲帧数: %d"),
        QuantumCamera->IsCameraAcquired() ? TEXT("是") : TEXT("否"),
        QuantumCamera->IsCameraEnabled() ? TEXT("是") : TEXT("否"),
        *UEnum::GetValueAsString(QuantumCamera->GetCameraMode()),
        RecordingComponent->IsRecording() ? TEXT("是") : TEXT("否"),
        RecordingComponent->GetCurrentRecordingDuration(),
        RecordingComponent->GetMaxRecordingDuration(),
        RecordingComponent->IsRewinding() ? TEXT("是") : TEXT("否"),
        RecordingComponent->GetRecordingMetadata().FrameCount
    );

    GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Cyan, DebugText);
}
#endif
```
