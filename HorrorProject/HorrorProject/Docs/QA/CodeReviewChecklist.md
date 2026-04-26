# Code Review Checklist

**Project:** HorrorProject  
**Version:** 1.0  
**Last Updated:** 2026-04-26  

---

## Overview

使用此清单审查所有代码变更。所有项目必须检查并标记为通过(✅)或不适用(N/A)。

---

## General

- [ ] 代码编译无错误
- [ ] 代码编译无警告
- [ ] 符合UE编码标准
- [ ] 命名清晰且一致
- [ ] 无拼写错误
- [ ] 无调试代码（如硬编码路径、测试代码）
- [ ] 无注释掉的代码
- [ ] 无TODO/FIXME（或已创建ticket）

---

## Code Quality

### 1. Error Handling

- [ ] 所有指针使用前检查是否为空
- [ ] 所有GetWorld()调用检查返回值
- [ ] 所有数组访问检查边界
- [ ] 所有外部输入验证有效性
- [ ] 错误情况记录日志
- [ ] 实现优雅降级
- [ ] 使用check()/ensure()进行断言

**Example Check:**
```cpp
// ❌ Bad
void Foo()
{
    MyObject->DoSomething();  // No null check
}

// ✅ Good
void Foo()
{
    if (!IsValid(MyObject))
    {
        UE_LOG(LogHorror, Error, TEXT("MyObject is invalid"));
        return;
    }
    MyObject->DoSomething();
}
```

---

### 2. Memory Management

- [ ] 使用TObjectPtr<>管理UObject引用
- [ ] UObject引用使用UPROPERTY()
- [ ] 避免裸指针
- [ ] 动态分配的资源正确清理
- [ ] 无内存泄漏风险
- [ ] 正确使用NewObject<>
- [ ] 避免循环引用

---

### 3. Performance

- [ ] 避免在Tick中进行昂贵操作
- [ ] 使用对象池避免频繁创建/销毁
- [ ] 避免不必要的类型转换
- [ ] 避免不必要的拷贝
- [ ] 使用const引用传递大对象
- [ ] 算法复杂度合理
- [ ] 无性能瓶颈

**Tick Check:**
```cpp
// ❌ Bad - Expensive in Tick
void TickComponent(float DeltaTime, ...)
{
    TArray<AActor*> Actors;
    UGameplayStatics::GetAllActorsOfClass(...);
}

// ✅ Good - Use Timer
void BeginPlay()
{
    GetWorld()->GetTimerManager().SetTimer(...);
}
```

---

### 4. Thread Safety

- [ ] 共享数据使用锁保护
- [ ] 避免数据竞争
- [ ] 正确使用Async/ParallelFor
- [ ] 无死锁风险

---

## Architecture

### 1. Design

- [ ] 职责单一（Single Responsibility）
- [ ] 低耦合高内聚
- [ ] 接口清晰
- [ ] 扩展性良好
- [ ] 可测试性好
- [ ] 符合SOLID原则
- [ ] 使用合适的设计模式

---

### 2. Dependencies

- [ ] 无循环依赖
- [ ] 依赖最小化
- [ ] 使用前向声明
- [ ] 依赖注入而非硬编码

---

### 3. Modularity

- [ ] 代码组织清晰
- [ ] 模块边界明确
- [ ] 可复用性好

---

## Blueprint Integration

- [ ] 使用BlueprintCallable/BlueprintPure
- [ ] 添加Category元数据
- [ ] 添加DisplayName/ToolTip
- [ ] 避免复杂C++类型暴露给BP
- [ ] 参数类型BP友好
- [ ] 返回值类型BP友好

**Example:**
```cpp
UFUNCTION(BlueprintCallable, Category="Horror|Audio", 
    meta=(DisplayName="Play Sound", ToolTip="Plays a sound at location"))
void PlaySound(USoundBase* Sound, FVector Location);
```

---

## Testing

### 1. Test Coverage

- [ ] 新代码有单元测试
- [ ] 测试覆盖率 >= 80%
- [ ] 包含边界条件测试
- [ ] 包含错误路径测试
- [ ] 包含性能测试（如适用）

---

### 2. Test Quality

- [ ] 测试独立（无顺序依赖）
- [ ] 测试可重复
- [ ] 测试名称清晰
- [ ] 断言详细
- [ ] 使用Mock（如适用）

---

### 3. Test Execution

- [ ] 所有测试通过
- [ ] 无测试被跳过
- [ ] 测试执行时间合理

---

## Documentation

### 1. Code Comments

- [ ] 所有public函数有注释
- [ ] 复杂逻辑有解释
- [ ] 使用Doxygen风格
- [ ] 参数和返回值说明
- [ ] 注释准确且最新

---

### 2. API Documentation

- [ ] 更新API文档（如适用）
- [ ] 添加使用示例
- [ ] 说明性能考虑
- [ ] 说明限制和注意事项

---

### 3. README

- [ ] 更新README（如适用）
- [ ] 更新依赖列表
- [ ] 更新使用说明

---

## Security

- [ ] 验证所有用户输入
- [ ] 验证所有网络数据
- [ ] 验证所有文件路径
- [ ] 无硬编码密钥
- [ ] 敏感数据加密
- [ ] 不记录敏感信息

---

## Accessibility

- [ ] UI支持键盘导航
- [ ] UI支持手柄导航
- [ ] 字体大小可调
- [ ] 色盲友好
- [ ] 音频有字幕（如适用）

---

## Integration

### 1. EventBus

- [ ] 正确使用EventBus（如适用）
- [ ] 事件注册/注销配对
- [ ] 事件命名清晰
- [ ] 避免事件循环

---

### 2. Subsystems

- [ ] 正确使用Subsystem（如适用）
- [ ] Initialize/Deinitialize配对
- [ ] 生命周期管理正确

---

### 3. Compatibility

- [ ] 与现有系统兼容
- [ ] 无接口冲突
- [ ] 向后兼容（如适用）

---

## Assets

- [ ] 资源引用正确
- [ ] 无硬编码资源路径
- [ ] 资源大小合理
- [ ] 资源命名规范

---

## Configuration

- [ ] 配置参数合理
- [ ] 默认值合理
- [ ] 配置可调整
- [ ] 配置有文档

---

## Logging

- [ ] 使用正确的日志级别
- [ ] 日志消息清晰
- [ ] 无日志泛滥
- [ ] 关键操作有日志

**Log Levels:**
- Error: 错误情况
- Warning: 警告情况
- Log: 一般信息
- Verbose: 详细信息
- VeryVerbose: 非常详细

---

## Version Control

### 1. Commit

- [ ] Commit消息清晰
- [ ] Commit消息格式正确
- [ ] Commit大小合理（不过大）
- [ ] 无不相关的变更

---

### 2. Branch

- [ ] 分支命名正确
- [ ] 基于正确的分支
- [ ] 无合并冲突

---

### 3. Pull Request

- [ ] PR描述清晰
- [ ] 链接相关issue
- [ ] 标记breaking changes
- [ ] 更新CHANGELOG（如适用）

---

## Platform Specific

### 1. Windows

- [ ] 路径使用正斜杠或FPaths
- [ ] 避免平台特定代码（或使用宏）

---

### 2. Console

- [ ] 考虑内存限制
- [ ] 考虑性能限制

---

## Specific Checks by System

### VFX System

- [ ] 粒子预算检查
- [ ] LOD实现
- [ ] 效果自动清理
- [ ] 性能优化

---

### Audio System

- [ ] 音频对象池使用
- [ ] 优先级队列使用
- [ ] 并发限制
- [ ] 遮挡系统（如适用）

---

### UI System

- [ ] Widget生命周期管理
- [ ] 事件注册/注销配对
- [ ] 批量更新使用（如适用）

---

## Final Checks

- [ ] 代码审查者理解所有变更
- [ ] 所有问题已解决
- [ ] 所有讨论已结束
- [ ] 准备合并

---

## Approval

**Reviewer:** _______________  
**Date:** _______________  
**Status:** [ ] Approved [ ] Changes Requested [ ] Rejected  

**Comments:**
_______________________________________________
_______________________________________________
_______________________________________________

---

## References

- [Quality Standards](QualityStandards.md)
- [Testing Standards](TestingStandards.md)
- [UE Coding Standard](https://docs.unrealengine.com/5.0/en-US/epic-cplusplus-coding-standard-for-unreal-engine/)
