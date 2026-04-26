# Quality Standards

**Project:** HorrorProject  
**Version:** 1.0  
**Last Updated:** 2026-04-26  

---

## Overview

本文档定义HorrorProject的质量标准，所有代码、测试和文档必须符合这些标准才能合并到主分支。

---

## Code Quality Standards

### 1. UE Coding Standards Compliance

**Mandatory:**
- ✅ 遵循Epic Games C++ Coding Standard
- ✅ 使用UE命名约定（U/A/F/E前缀）
- ✅ 使用UPROPERTY/UFUNCTION宏
- ✅ 正确使用TObjectPtr<>
- ✅ 使用UE容器类型（TArray, TMap, TSet）

**Naming Conventions:**
```cpp
// Classes
UMyComponent          // UObject derived
AMyActor             // AActor derived
FMyStruct            // Plain struct
EMyEnum              // Enum
IMyInterface         // Interface

// Variables
bIsEnabled           // Boolean (b prefix)
MyVariable           // PascalCase
MY_CONSTANT          // UPPER_CASE

// Functions
DoSomething()        // PascalCase
GetValue()           // Getter
SetValue()           // Setter
```

---

### 2. Error Handling

**Mandatory:**
- ✅ 检查所有指针是否为空
- ✅ 验证所有外部输入
- ✅ 使用check()/ensure()进行断言
- ✅ 记录错误到日志
- ✅ 实现优雅降级

**Example:**
```cpp
void UMyComponent::DoSomething()
{
    if (!IsValid(MyObject))
    {
        UE_LOG(LogHorror, Error, TEXT("MyObject is invalid"));
        return;
    }

    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogHorror, Error, TEXT("World is null"));
        return;
    }

    // Safe to proceed
}
```

**Minimum Requirements:**
- 90%+ 空指针检查覆盖率
- 所有public函数验证参数
- 所有错误记录日志

---

### 3. Memory Management

**Mandatory:**
- ✅ 使用TObjectPtr<>管理UObject引用
- ✅ 使用UPROPERTY()防止垃圾回收
- ✅ 避免裸指针
- ✅ 正确使用NewObject<>
- ✅ 清理动态分配的资源

**Example:**
```cpp
UPROPERTY()
TObjectPtr<UMyComponent> MyComponent;  // ✅ Good

UMyComponent* MyComponent;  // ❌ Bad (no UPROPERTY)
```

---

### 4. Performance

**Mandatory:**
- ✅ 避免在Tick中进行昂贵操作
- ✅ 使用对象池避免频繁创建/销毁
- ✅ 实现LOD系统
- ✅ 使用性能预算
- ✅ 避免不必要的类型转换

**Tick Guidelines:**
```cpp
// ❌ Bad - Expensive operation in Tick
void UMyComponent::TickComponent(float DeltaTime, ...)
{
    TArray<AActor*> Actors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), Actors);
    // Process actors...
}

// ✅ Good - Use Timer
void UMyComponent::BeginPlay()
{
    GetWorld()->GetTimerManager().SetTimer(
        UpdateTimer,
        this,
        &UMyComponent::UpdateActors,
        1.0f,  // Every 1 second
        true
    );
}
```

**Performance Budgets:**
- Tick函数: < 0.1ms
- 对象池大小: 明确限制
- 并发声音: < 16
- 粒子数量: < 1000

---

### 5. Blueprint Friendly

**Mandatory:**
- ✅ 使用BlueprintCallable/BlueprintPure
- ✅ 添加Category元数据
- ✅ 使用DisplayName/ToolTip
- ✅ 避免复杂的C++类型暴露给BP

**Example:**
```cpp
UFUNCTION(BlueprintCallable, Category="Horror|Audio", meta=(ToolTip="Plays a sound at the specified location"))
void PlaySoundAtLocation(USoundBase* Sound, FVector Location);
```

---

## Architecture Standards

### 1. SOLID Principles

**Single Responsibility:**
- 每个类只负责一个功能
- 避免"God Classes"

**Open/Closed:**
- 对扩展开放，对修改关闭
- 使用继承和接口

**Liskov Substitution:**
- 子类可以替换父类
- 不破坏父类契约

**Interface Segregation:**
- 接口小而专注
- 避免臃肿接口

**Dependency Inversion:**
- 依赖抽象而非具体实现
- 使用接口和基类

---

### 2. Design Patterns

**Recommended:**
- Subsystem Pattern (UWorldSubsystem, UGameInstanceSubsystem)
- Object Pool Pattern
- Observer Pattern (Delegates, EventBus)
- Strategy Pattern
- Factory Pattern

**Example - Subsystem:**
```cpp
UCLASS()
class UMySubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
};
```

---

### 3. Module Organization

**Structure:**
```
Source/
├── HorrorProject/
│   ├── AI/              # AI系统
│   ├── Audio/           # 音频系统
│   ├── Game/            # 游戏逻辑
│   ├── Player/          # 玩家系统
│   ├── UI/              # UI系统
│   ├── VFX/             # 视觉效果
│   └── Tests/           # 测试
└── HorrorProjectEditor/
    └── Tools/           # 编辑器工具
```

**Rules:**
- 按功能模块组织
- 避免循环依赖
- 使用前向声明减少依赖

---

## Testing Standards

### 1. Test Coverage

**Minimum Requirements:**
- Overall: 70%+
- Critical systems: 80%+
- New code: 80%+

**Coverage by Type:**
- Unit Tests: 60%+
- Integration Tests: 40%+
- System Tests: 20%+

---

### 2. Test Quality

**Mandatory:**
- ✅ 测试独立性（无顺序依赖）
- ✅ 测试可重复性
- ✅ 清晰的测试名称
- ✅ 详细的断言消息
- ✅ 边界条件测试
- ✅ 错误路径测试

**Example:**
```cpp
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FMyComponentTest,
    "HorrorProject.MyModule.MyComponent",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter
)

bool FMyComponentTest::RunTest(const FString& Parameters)
{
    // Arrange
    UMyComponent* Component = NewObject<UMyComponent>();
    TestNotNull(TEXT("Component should be created"), Component);

    // Act
    Component->DoSomething();

    // Assert
    TestEqual(TEXT("Value should be 42"), Component->GetValue(), 42);

    return true;
}
```

---

### 3. Test Categories

**Required Tests:**
- Null pointer tests
- Invalid parameter tests
- Boundary value tests
- Error path tests
- Performance tests
- Integration tests

---

## Documentation Standards

### 1. Code Comments

**Mandatory:**
- ✅ 所有public函数有注释
- ✅ 复杂逻辑有解释
- ✅ 使用Doxygen风格
- ✅ 参数和返回值说明

**Example:**
```cpp
/**
 * Plays a sound at the specified location with optional volume and pitch multipliers.
 * 
 * @param Sound The sound to play (must not be null)
 * @param Location World location to play the sound
 * @param VolumeMultiplier Volume multiplier (0.0 to 1.0)
 * @param PitchMultiplier Pitch multiplier (0.5 to 2.0)
 * @return The spawned audio component, or nullptr if failed
 */
UFUNCTION(BlueprintCallable, Category="Horror|Audio")
UAudioComponent* PlaySoundAtLocation(
    USoundBase* Sound,
    FVector Location,
    float VolumeMultiplier = 1.0f,
    float PitchMultiplier = 1.0f
);
```

---

### 2. API Documentation

**Required:**
- System overview
- Architecture diagram
- API reference
- Usage examples
- Best practices
- Performance considerations

---

### 3. README Files

**Required in Each Module:**
- Module purpose
- Key components
- Dependencies
- Usage examples
- Known issues

---

## Performance Standards

### 1. Frame Time Budget

**Target: 60 FPS (16.67ms per frame)**

**Budget Allocation:**
- Game Logic: 5ms
- Rendering: 8ms
- Audio: 1ms
- Physics: 2ms
- Other: 0.67ms

---

### 2. Memory Budget

**Target: < 4GB RAM**

**Budget Allocation:**
- Textures: 1.5GB
- Meshes: 800MB
- Audio: 500MB
- Code: 200MB
- Other: 1GB

---

### 3. Asset Standards

**Textures:**
- Max size: 2048x2048 (4096 for special cases)
- Use compression
- Generate mipmaps

**Meshes:**
- Max triangles: 10,000 (50,000 for hero assets)
- Use LODs (4 levels)
- Optimize topology

**Audio:**
- Sample rate: 44.1kHz
- Bit depth: 16-bit
- Use compression (Vorbis)

---

## Security Standards

### 1. Input Validation

**Mandatory:**
- ✅ 验证所有用户输入
- ✅ 验证所有网络数据
- ✅ 验证所有文件路径
- ✅ 防止注入攻击

---

### 2. Data Protection

**Mandatory:**
- ✅ 不在代码中硬编码密钥
- ✅ 使用加密存储敏感数据
- ✅ 不记录敏感信息到日志

---

## Accessibility Standards

### 1. UI Accessibility

**Mandatory:**
- ✅ 支持键盘导航
- ✅ 支持手柄导航
- ✅ 可调整字体大小
- ✅ 高对比度模式
- ✅ 色盲友好

---

### 2. Audio Accessibility

**Mandatory:**
- ✅ 字幕支持
- ✅ 音频提示可视化
- ✅ 可调整音量

---

## Version Control Standards

### 1. Commit Messages

**Format:**
```
<type>(<scope>): <subject>

<body>

<footer>
```

**Types:**
- feat: 新功能
- fix: 修复
- docs: 文档
- style: 格式
- refactor: 重构
- test: 测试
- chore: 构建/工具

**Example:**
```
feat(audio): add audio occlusion system

Implement line-trace based audio occlusion with configurable update rate.
Includes object pooling for audio components.

Closes #123
```

---

### 2. Branch Strategy

**Branches:**
- main: 生产代码
- develop: 开发分支
- feature/*: 功能分支
- bugfix/*: 修复分支
- hotfix/*: 紧急修复

---

### 3. Pull Request Requirements

**Mandatory:**
- ✅ 通过所有测试
- ✅ 代码审查批准
- ✅ 符合编码标准
- ✅ 更新文档
- ✅ 无合并冲突

---

## Quality Gates

### 1. Pre-Commit

**Checks:**
- Code compiles
- No syntax errors
- Formatting correct

---

### 2. Pre-Merge

**Checks:**
- All tests pass
- Code review approved
- Coverage >= 70%
- No critical issues
- Documentation updated

---

### 3. Pre-Release

**Checks:**
- All tests pass
- Performance benchmarks met
- No known critical bugs
- Documentation complete
- Security audit passed

---

## Enforcement

### 1. Automated Checks

**CI/CD Pipeline:**
- Compile check
- Unit tests
- Integration tests
- Code coverage
- Static analysis
- Performance tests

---

### 2. Code Review

**Required:**
- At least 1 approval
- No unresolved comments
- Checklist completed

---

### 3. Quality Metrics

**Tracked:**
- Code quality score
- Test coverage
- Bug density
- Technical debt
- Performance metrics

---

## Exceptions

### Requesting Exception

**Process:**
1. Document reason
2. Assess risk
3. Get approval from QA Director
4. Create technical debt ticket
5. Set payoff deadline

**Valid Reasons:**
- Prototype/POC code
- Third-party integration
- Performance optimization
- Legacy code compatibility

---

## References

- [UE Coding Standard](https://docs.unrealengine.com/5.0/en-US/epic-cplusplus-coding-standard-for-unreal-engine/)
- [Code Review Checklist](CodeReviewChecklist.md)
- [Testing Standards](TestingStandards.md)
- [Acceptance Criteria](AcceptanceCriteria.md)

---

**Approved by:** QA Director  
**Date:** 2026-04-26  
**Version:** 1.0
