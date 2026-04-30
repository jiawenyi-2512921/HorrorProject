# 证据系统指南

## 概述

HorrorProject的证据系统提供完整的证据收集、分类和管理功能，支持照片、录像、实物等多种证据类型。

## 核心组件

### 1. EvidenceActor

可交互的证据Actor，放置在关卡中供玩家收集。

**主要功能：**
- 实现IInteractableInterface接口
- 自动注册证据元数据
- 收集后可隐藏或销毁
- 支持蓝图事件扩展

### 2. EvidenceCollectionComponent

玩家角色的证据收集组件，管理所有已收集的证据。

**主要功能：**
- 拍照证据捕获
- 录像证据记录
- 实物证据收集
- 证据元数据管理
- 证据查询和过滤

### 3. 证据类型系统

```cpp
enum class EEvidenceType : uint8
{
    Physical,   // 实物
    Photo,      // 照片
    Video,      // 录像
    Audio,      // 录音
    Note,       // 笔记
    Document    // 文档
};

enum class EEvidenceCategory : uint8
{
    Clue,           // 线索
    Witness,        // 证言
    Location,       // 地点证据
    Temporal,       // 时间异常
    Personal,       // 个人物品
    Environmental   // 环境证据
};
```

## 证据元数据结构

### FExtendedEvidenceMetadata

```cpp
USTRUCT(BlueprintType)
struct FExtendedEvidenceMetadata
{
    // 唯一标识符
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName EvidenceId = NAME_None;

    // 显示名称
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText DisplayName;

    // 描述
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText Description;

    // 证据类型
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EEvidenceType Type = EEvidenceType::Physical;

    // 证据分类
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EEvidenceCategory Category = EEvidenceCategory::Clue;

    // 游戏标签
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FGameplayTagContainer Tags;

    // 收集时间戳
    UPROPERTY(BlueprintReadOnly)
    FDateTime CollectionTimestamp;

    // 收集位置
    UPROPERTY(BlueprintReadOnly)
    FVector CollectionLocation = FVector::ZeroVector;

    // 缩略图
    UPROPERTY(BlueprintReadOnly)
    UTexture2D* ThumbnailImage = nullptr;

    // 重要程度（1-5）
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 ImportanceLevel = 1;

    // 是否为关键证据
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsKeyEvidence = false;

    // 关联证据ID列表
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FName> RelatedEvidenceIds;
};
```

## 使用指南

### 创建EvidenceActor

**步骤1：在关卡中放置EvidenceActor**
```cpp
// C++代码示例
AEvidenceActor* Evidence = GetWorld()->SpawnActor<AEvidenceActor>(
    AEvidenceActor::StaticClass(),
    SpawnLocation,
    FRotator::ZeroRotator
);

// 配置证据元数据
FExtendedEvidenceMetadata Metadata;
Metadata.EvidenceId = TEXT("Evidence.Diary.Page1");
Metadata.DisplayName = FText::FromString(TEXT("日记第一页"));
Metadata.Description = FText::FromString(TEXT("一页破旧的日记，记录了奇怪的事件..."));
Metadata.Type = EEvidenceType::Document;
Metadata.Category = EEvidenceCategory::Clue;
Metadata.ImportanceLevel = 3;
Metadata.bIsKeyEvidence = true;

Evidence->SetEvidenceMetadata(Metadata);
```

**步骤2：配置交互提示**
```cpp
// 自定义交互提示文本
Evidence->InteractionPromptOverride = FText::FromString(TEXT("按E拾取日记"));

// 配置收集行为
Evidence->bDestroyOnCollect = false;  // 收集后不销毁
Evidence->bHideOnCollect = true;      // 收集后隐藏
Evidence->bAutoRegisterMetadata = true; // 自动注册元数据
```

**蓝图方式：**
1. 在关卡中放置EvidenceActor
2. 在Details面板配置Evidence Metadata
3. 设置Mesh Component的静态网格
4. 配置交互选项

### 添加EvidenceCollectionComponent

**C++方式：**
```cpp
// 在玩家角色头文件中
UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Evidence")
UEvidenceCollectionComponent* EvidenceCollection;

// 在构造函数中创建
AHorrorPlayerCharacter::AHorrorPlayerCharacter()
{
    EvidenceCollection = CreateDefaultSubobject<UEvidenceCollectionComponent>(TEXT("EvidenceCollection"));
}
```

### 收集实物证据

```cpp
// 玩家与EvidenceActor交互时自动调用
bool AEvidenceActor::Interact_Implementation(AActor* InstigatorActor, const FHitResult& Hit)
{
    if (bIsCollected) return false;

    // 获取玩家的证据收集组件
    if (AHorrorPlayerCharacter* Player = Cast<AHorrorPlayerCharacter>(InstigatorActor))
    {
        if (UEvidenceCollectionComponent* Collection = Player->EvidenceCollection)
        {
            // 收集证据
            bool bSuccess = Collection->CollectPhysicalEvidence(
                EvidenceMetadata.EvidenceId,
                EvidenceMetadata
            );

            if (bSuccess)
            {
                SetCollected(true);
                OnEvidenceCollected(InstigatorActor);
                return true;
            }
        }
    }

    return false;
}
```

### 拍照证据

**与QuantumCameraComponent集成：**
```cpp
void AHorrorPlayerCharacter::TakeEvidencePhoto()
{
    if (!EvidenceCollection || !QuantumCamera) return;

    // 检查相机是否可用
    if (!QuantumCamera->IsCameraAcquired() || !QuantumCamera->IsCameraEnabled())
    {
        return;
    }

    // 创建证据元数据
    FExtendedEvidenceMetadata PhotoMetadata;
    PhotoMetadata.EvidenceId = FName(*FString::Printf(TEXT("Photo_%d"), PhotoCounter++));
    PhotoMetadata.DisplayName = FText::FromString(TEXT("照片证据"));
    PhotoMetadata.Type = EEvidenceType::Photo;
    PhotoMetadata.Category = EEvidenceCategory::Location;

    // 添加上下文标签
    PhotoMetadata.Tags.AddTag(FGameplayTag::RequestGameplayTag(TEXT("Evidence.Photo")));

    // 捕获照片
    bool bSuccess = EvidenceCollection->CapturePhotoEvidence(
        PhotoMetadata.EvidenceId,
        PhotoMetadata
    );

    if (bSuccess)
    {
        // 播放拍照音效
        UGameplayStatics::PlaySound2D(this, CameraShutterSound);

        // 显示UI反馈
        ShowPhotoTakenNotification();
    }
}
```

### 录像证据

```cpp
void AHorrorPlayerCharacter::StartVideoRecording()
{
    if (!EvidenceCollection || !QuantumCamera) return;

    if (!QuantumCamera->IsCameraMode(EQuantumCameraMode::Recording))
    {
        return;
    }

    // 创建录像证据ID
    FName VideoId = FName(*FString::Printf(TEXT("Video_%d"), VideoCounter++));

    // 开始录像
    bool bSuccess = EvidenceCollection->StartVideoCapture(VideoId);

    if (bSuccess)
    {
        UE_LOG(LogTemp, Log, TEXT("开始录像: %s"), *VideoId.ToString());
    }
}

void AHorrorPlayerCharacter::StopVideoRecording()
{
    if (!EvidenceCollection) return;

    FName VideoId;
    float Duration;

    // 停止录像
    bool bSuccess = EvidenceCollection->StopVideoCapture(VideoId, Duration);

    if (bSuccess)
    {
        UE_LOG(LogTemp, Log, TEXT("录像完成: %s, 时长: %.2f秒"), *VideoId.ToString(), Duration);

        // 显示录像保存通知
        ShowVideoSavedNotification(Duration);
    }
}
```

### 查询证据

**获取所有证据：**
```cpp
TArray<FExtendedEvidenceMetadata> AllEvidence = EvidenceCollection->GetAllEvidenceMetadata();

for (const FExtendedEvidenceMetadata& Evidence : AllEvidence)
{
    UE_LOG(LogTemp, Log, TEXT("证据: %s - %s"),
        *Evidence.EvidenceId.ToString(),
        *Evidence.DisplayName.ToString());
}
```

**按类型查询：**
```cpp
// 获取所有照片证据
TArray<FExtendedEvidenceMetadata> Photos = EvidenceCollection->GetEvidenceByType(EEvidenceType::Photo);

// 获取所有实物证据
TArray<FExtendedEvidenceMetadata> PhysicalEvidence = EvidenceCollection->GetEvidenceByType(EEvidenceType::Physical);
```

**按分类查询：**
```cpp
// 获取所有线索
TArray<FExtendedEvidenceMetadata> Clues = EvidenceCollection->GetEvidenceByCategory(EEvidenceCategory::Clue);

// 获取所有时间异常证据
TArray<FExtendedEvidenceMetadata> TemporalEvidence = EvidenceCollection->GetEvidenceByCategory(EEvidenceCategory::Temporal);
```

**按标签查询：**
```cpp
// 创建标签容器
FGameplayTagContainer SearchTags;
SearchTags.AddTag(FGameplayTag::RequestGameplayTag(TEXT("Evidence.Location.Basement")));
SearchTags.AddTag(FGameplayTag::RequestGameplayTag(TEXT("Evidence.Type.Anomaly")));

// 查询包含任一标签的证据
TArray<FExtendedEvidenceMetadata> Results = EvidenceCollection->GetEvidenceByTags(SearchTags, false);

// 查询包含所有标签的证据
TArray<FExtendedEvidenceMetadata> StrictResults = EvidenceCollection->GetEvidenceByTags(SearchTags, true);
```

**获取单个证据：**
```cpp
FExtendedEvidenceMetadata Metadata;
bool bFound = EvidenceCollection->GetEvidenceMetadata(TEXT("Evidence.Diary.Page1"), Metadata);

if (bFound)
{
    UE_LOG(LogTemp, Log, TEXT("找到证据: %s"), *Metadata.DisplayName.ToString());
}
```

### 注册证据元数据

```cpp
// 手动注册证据元数据（不需要实际收集）
FExtendedEvidenceMetadata Metadata;
Metadata.EvidenceId = TEXT("Evidence.Hint.Tutorial");
Metadata.DisplayName = FText::FromString(TEXT("教程提示"));
Metadata.Description = FText::FromString(TEXT("游戏开始时的提示信息"));
Metadata.Type = EEvidenceType::Note;
Metadata.Category = EEvidenceCategory::Clue;

bool bSuccess = EvidenceCollection->RegisterEvidenceMetadata(Metadata);
```

## 与目标系统集成（HorrorObjectiveNode）

### 创建证据收集目标

```cpp
// 在HorrorObjectiveNode中检查证据收集
bool AHorrorObjectiveNode::CanComplete(AActor* InstigatorActor) const
{
    if (NodeType != EHorrorObjectiveNodeType::Interaction)
    {
        return false;
    }

    // 检查玩家是否收集了所需证据
    if (AHorrorPlayerCharacter* Player = Cast<AHorrorPlayerCharacter>(InstigatorActor))
    {
        if (UEvidenceCollectionComponent* Collection = Player->EvidenceCollection)
        {
            // 检查关键证据
            for (FName RequiredEvidenceId : RequiredEvidenceIds)
            {
                FExtendedEvidenceMetadata Metadata;
                if (!Collection->GetEvidenceMetadata(RequiredEvidenceId, Metadata))
                {
                    return false;  // 缺少必需证据
                }
            }

            return true;
        }
    }

    return false;
}
```

**配置目标节点：**
```cpp
// 创建证据收集目标
AHorrorObjectiveNode* ObjectiveNode = GetWorld()->SpawnActor<AHorrorObjectiveNode>();
ObjectiveNode->NodeType = EHorrorObjectiveNodeType::Interaction;
ObjectiveNode->NodeId = TEXT("Objective.CollectDiaryPages");
ObjectiveNode->NodeTitle = FText::FromString(TEXT("收集日记页"));
ObjectiveNode->NodeDescription = FText::FromString(TEXT("找到并收集所有日记页"));

// 设置必需证据
ObjectiveNode->RequiredEvidenceIds.Add(TEXT("Evidence.Diary.Page1"));
ObjectiveNode->RequiredEvidenceIds.Add(TEXT("Evidence.Diary.Page2"));
ObjectiveNode->RequiredEvidenceIds.Add(TEXT("Evidence.Diary.Page3"));

// 配置事件标签
ObjectiveNode->EventTag = FGameplayTag::RequestGameplayTag(TEXT("Objective.DiaryComplete"));
ObjectiveNode->bPublishEventOnComplete = true;
```

### 监听证据收集事件

```cpp
void AHorrorObjectiveManager::BeginPlay()
{
    Super::BeginPlay();

    // 获取玩家的证据收集组件
    if (AHorrorPlayerCharacter* Player = GetPlayerCharacter())
    {
        if (UEvidenceCollectionComponent* Collection = Player->EvidenceCollection)
        {
            // 绑定证据收集事件
            Collection->OnEvidenceCaptured.AddDynamic(this, &AHorrorObjectiveManager::HandleEvidenceCaptured);
            Collection->OnPhotoTaken.AddDynamic(this, &AHorrorObjectiveManager::HandlePhotoTaken);
            Collection->OnVideoRecorded.AddDynamic(this, &AHorrorObjectiveManager::HandleVideoRecorded);
        }
    }
}

void AHorrorObjectiveManager::HandleEvidenceCaptured(FName EvidenceId, const FEvidenceCaptureData& CaptureData)
{
    UE_LOG(LogTemp, Log, TEXT("证据已收集: %s"), *EvidenceId.ToString());

    // 检查是否完成相关目标
    CheckObjectiveCompletion(EvidenceId);
}

void AHorrorObjectiveManager::HandlePhotoTaken(FName PhotoId, UTexture2D* Photo, FVector Location)
{
    UE_LOG(LogTemp, Log, TEXT("照片已拍摄: %s at %s"), *PhotoId.ToString(), *Location.ToString());
}

void AHorrorObjectiveManager::HandleVideoRecorded(FName VideoId, float Duration, FVector Location)
{
    UE_LOG(LogTemp, Log, TEXT("录像已记录: %s, 时长: %.2f秒"), *VideoId.ToString(), Duration);
}
```

## 配置参数

### EvidenceCollectionComponent设置

```cpp
// 最大照片数量
UPROPERTY(EditAnywhere, BlueprintReadWrite)
int32 MaxPhotos = 50;

// 最大录像数量
UPROPERTY(EditAnywhere, BlueprintReadWrite)
int32 MaxVideos = 10;

// 最大录像时长（秒）
UPROPERTY(EditAnywhere, BlueprintReadWrite)
float MaxVideoDuration = 30.0f;
```

### EvidenceActor设置

```cpp
// 自动注册元数据
UPROPERTY(EditAnywhere, BlueprintReadWrite)
bool bAutoRegisterMetadata = true;

// 收集后销毁
UPROPERTY(EditAnywhere, BlueprintReadWrite)
bool bDestroyOnCollect = false;

// 收集后隐藏
UPROPERTY(EditAnywhere, BlueprintReadWrite)
bool bHideOnCollect = true;
```

## 蓝图使用示例

### 收集实物证据
```
节点: Collect Physical Evidence
- Target: Evidence Collection Component
- Evidence Id: "Evidence.Key.Basement"
- Metadata: [配置的元数据结构]
- Return Value: Success (bool)
```

### 拍照证据
```
节点: Capture Photo Evidence
- Target: Evidence Collection Component
- Evidence Id: "Photo_001"
- Metadata: [照片元数据]
- Return Value: Success (bool)
```

### 查询证据
```
节点: Get Evidence By Type
- Target: Evidence Collection Component
- Type: Photo
- Return Value: Array of Evidence Metadata
```

### 检查证据是否收集
```
节点: Get Evidence Metadata
- Target: Evidence Collection Component
- Evidence Id: "Evidence.Diary.Page1"
- Out Metadata: [输出元数据]
- Return Value: Found (bool)
```

## UI集成

### 证据清单界面

```cpp
void UEvidenceInventoryWidget::RefreshEvidenceList()
{
    if (!PlayerCharacter || !PlayerCharacter->EvidenceCollection) return;

    // 清空列表
    EvidenceListView->ClearListItems();

    // 获取所有证据
    TArray<FExtendedEvidenceMetadata> AllEvidence =
        PlayerCharacter->EvidenceCollection->GetAllEvidenceMetadata();

    // 按重要程度排序
    AllEvidence.Sort([](const FExtendedEvidenceMetadata& A, const FExtendedEvidenceMetadata& B)
    {
        if (A.bIsKeyEvidence != B.bIsKeyEvidence)
            return A.bIsKeyEvidence > B.bIsKeyEvidence;
        return A.ImportanceLevel > B.ImportanceLevel;
    });

    // 添加到列表
    for (const FExtendedEvidenceMetadata& Evidence : AllEvidence)
    {
        UEvidenceListItem* Item = NewObject<UEvidenceListItem>();
        Item->SetEvidenceData(Evidence);
        EvidenceListView->AddItem(Item);
    }
}
```

### 证据详情界面

```cpp
void UEvidenceDetailWidget::DisplayEvidence(FName EvidenceId)
{
    if (!PlayerCharacter || !PlayerCharacter->EvidenceCollection) return;

    FExtendedEvidenceMetadata Metadata;
    if (PlayerCharacter->EvidenceCollection->GetEvidenceMetadata(EvidenceId, Metadata))
    {
        // 显示基本信息
        TitleText->SetText(Metadata.DisplayName);
        DescriptionText->SetText(Metadata.Description);

        // 显示类型和分类
        TypeText->SetText(GetEvidenceTypeText(Metadata.Type));
        CategoryText->SetText(GetEvidenceCategoryText(Metadata.Category));

        // 显示缩略图
        if (Metadata.ThumbnailImage)
        {
            ThumbnailImage->SetBrushFromTexture(Metadata.ThumbnailImage);
        }

        // 显示收集信息
        FString CollectionInfo = FString::Printf(
            TEXT("收集时间: %s\n位置: %s"),
            *Metadata.CollectionTimestamp.ToString(),
            *Metadata.CollectionLocation.ToString()
        );
        CollectionInfoText->SetText(FText::FromString(CollectionInfo));

        // 显示重要程度
        ImportanceStars->SetStarCount(Metadata.ImportanceLevel);
        KeyEvidenceBadge->SetVisibility(
            Metadata.bIsKeyEvidence ? ESlateVisibility::Visible : ESlateVisibility::Collapsed
        );

        // 显示关联证据
        RefreshRelatedEvidence(Metadata.RelatedEvidenceIds);
    }
}
```

## 保存和加载

### 保存证据数据

```cpp
void UHorrorSaveGame::SaveEvidenceData(UEvidenceCollectionComponent* Collection)
{
    if (!Collection) return;

    // 获取所有证据元数据
    CollectedEvidence = Collection->GetAllEvidenceMetadata();

    // 保存照片数量和录像数量
    PhotoCount = Collection->GetEvidenceByType(EEvidenceType::Photo).Num();
    VideoCount = Collection->GetEvidenceByType(EEvidenceType::Video).Num();
}
```

### 加载证据数据

```cpp
void UHorrorSaveGame::LoadEvidenceData(UEvidenceCollectionComponent* Collection)
{
    if (!Collection) return;

    // 恢复所有证据元数据
    for (const FExtendedEvidenceMetadata& Metadata : CollectedEvidence)
    {
        Collection->RegisterEvidenceMetadata(Metadata);
    }
}
```

## 最佳实践

1. **使用唯一ID**：确保每个证据的EvidenceId全局唯一
2. **合理分类**：正确设置Type和Category便于查询和UI展示
3. **添加标签**：使用GameplayTag系统实现灵活的证据关联
4. **设置重要程度**：ImportanceLevel和bIsKeyEvidence帮助玩家识别关键证据
5. **关联证据**：使用RelatedEvidenceIds建立证据之间的联系
6. **限制数量**：合理设置MaxPhotos和MaxVideos避免内存问题
7. **提供反馈**：证据收集时提供清晰的UI和音频反馈
8. **保存缩略图**：为照片和录像生成缩略图便于UI展示

## 性能优化

### 缩略图生成优化

```cpp
// EvidenceCollectionComponent内部实现
float UEvidenceCollectionComponent::CalculateThumbnailResizeScale(const FIntPoint& ViewportSize) const
{
    // 目标缩略图尺寸
    const int32 TargetSize = 256;

    // 计算缩放比例
    float Scale = (float)TargetSize / FMath::Max(ViewportSize.X, ViewportSize.Y);
    return FMath::Clamp(Scale, 0.1f, 1.0f);
}
```

### 证据查询缓存

```cpp
// 缓存常用查询结果
TMap<EEvidenceType, TArray<FExtendedEvidenceMetadata>> CachedEvidenceByType;

TArray<FExtendedEvidenceMetadata> UEvidenceCollectionComponent::GetEvidenceByType(EEvidenceType Type) const
{
    // 检查缓存
    if (CachedEvidenceByType.Contains(Type))
    {
        return CachedEvidenceByType[Type];
    }

    // 执行查询并缓存
    TArray<FExtendedEvidenceMetadata> Results;
    for (const auto& Pair : EvidenceMetadataMap)
    {
        if (Pair.Value.Type == Type)
        {
            Results.Add(Pair.Value);
        }
    }

    CachedEvidenceByType.Add(Type, Results);
    return Results;
}
```

## 常见问题

**Q: 照片拍摄后没有缩略图？**
A: 检查游戏视口是否有效，确保在PIE或打包后运行。

**Q: 证据收集后仍然可以重复收集？**
A: 检查EvidenceActor的bIsCollected状态和CanInteract实现。

**Q: 录像时长超过限制？**
A: EvidenceCollectionComponent会自动限制录像时长到MaxVideoDuration。

**Q: 证据查询返回空数组？**
A: 确保证据已正确注册，检查EvidenceId是否匹配。

## 调试技巧

```cpp
// 显示证据收集调试信息
#if !UE_BUILD_SHIPPING
void UEvidenceCollectionComponent::DebugDisplayEvidence()
{
    if (!GEngine) return;

    TArray<FExtendedEvidenceMetadata> AllEvidence = GetAllEvidenceMetadata();

    GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Cyan,
        FString::Printf(TEXT("总证据数: %d"), AllEvidence.Num()));

    int32 Index = 0;
    for (const FExtendedEvidenceMetadata& Evidence : AllEvidence)
    {
        FString DebugText = FString::Printf(
            TEXT("[%d] %s | Type: %s | Category: %s | Key: %s"),
            Index++,
            *Evidence.DisplayName.ToString(),
            *UEnum::GetValueAsString(Evidence.Type),
            *UEnum::GetValueAsString(Evidence.Category),
            Evidence.bIsKeyEvidence ? TEXT("Yes") : TEXT("No")
        );

        GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Yellow, DebugText);
    }
}
#endif
```
