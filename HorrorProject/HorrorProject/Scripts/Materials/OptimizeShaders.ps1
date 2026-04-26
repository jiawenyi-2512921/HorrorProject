# OptimizeShaders.ps1 - Shader优化建议生成器
# 基于分析结果生成优化建议

param(
    [string]$AnalysisPath = "D:\gptzuo\HorrorProject\HorrorProject\Scripts\Materials\ShaderAnalysis.json",
    [string]$OutputPath = "D:\gptzuo\HorrorProject\HorrorProject\Scripts\Materials\OptimizationReport.md"
)

$ErrorActionPreference = "Continue"

Write-Host "=== Shader优化建议生成器 ===" -ForegroundColor Cyan

# 读取分析结果
if (-not (Test-Path $AnalysisPath)) {
    Write-Host "错误: 找不到分析文件 $AnalysisPath" -ForegroundColor Red
    Write-Host "请先运行 AnalyzeShaders.ps1" -ForegroundColor Yellow
    exit 1
}

$analysis = Get-Content $AnalysisPath -Raw | ConvertFrom-Json

# 生成优化建议
$report = @"
# Shader优化报告
生成时间: $(Get-Date -Format "yyyy-MM-dd HH:mm:ss")

## 执行摘要
- 总材质数: $($analysis.Summary.TotalMaterials)
- 需要优化的材质: $($analysis.ComplexityDistribution.VeryHigh + $analysis.ComplexityDistribution.High)
- 优化优先级: 极高复杂度材质

## 优化目标
- 指令数 < 300 per material
- 纹理采样 < 16 per material
- 实现材质质量分级系统
- 减少overdraw和透明度使用

## 高优先级优化项

### 1. 极高复杂度材质 ($($analysis.ComplexityDistribution.VeryHigh) 个)
这些材质文件大小 > 100KB，可能包含过多节点和复杂计算。

**优化策略:**
- 移除冗余节点和未使用的输入
- 合并重复的数学运算
- 使用材质函数复用逻辑
- 考虑拆分为多个简单材质
- 启用材质质量开关

### 2. 后处理材质 ($($analysis.Summary.PostProcessMaterials) 个)
后处理材质对性能影响最大，需要特别优化。

**优化策略:**
- 减少SceneTexture采样次数
- 避免复杂的循环和分支
- 使用LUT替代复杂计算
- 考虑使用Compute Shader
- 实现可配置的质量级别

### 3. 半透明材质 ($($analysis.CategoryDistribution.Translucent) 个)
半透明材质会导致overdraw，需要谨慎优化。

**优化策略:**
- 尽可能使用Masked代替Translucent
- 减少半透明层数
- 优化排序和渲染顺序
- 使用简单的blend模式
- 考虑使用Dithered Opacity

## 材质标准化建议

### 主材质架构
创建统一的主材质系统:

1. **M_Master_Opaque** - 不透明主材质
   - 支持BaseColor, Normal, Roughness, Metallic, AO
   - 可选: Emissive, DetailNormal, Parallax
   - 质量开关: Low/Medium/High/Epic

2. **M_Master_Masked** - 遮罩主材质
   - 继承Opaque功能
   - 优化的Opacity Mask
   - Dithered LOD支持

3. **M_Master_Translucent** - 半透明主材质
   - 简化的光照模型
   - 优化的透明度计算
   - 可选折射

4. **M_Master_Water** - 水体主材质
   - 优化的水面效果
   - 可配置的波浪和反射
   - 性能友好的焦散

### 材质函数库
创建可复用的材质函数:

1. **MF_Parallax** - 视差映射
   - 可配置的采样次数
   - 质量分级支持

2. **MF_DetailNormal** - 细节法线
   - 优化的法线混合
   - 可配置的强度

3. **MF_Wetness** - 湿润效果
   - 统一的湿润计算
   - 性能优化的反射

4. **MF_TriplanarMapping** - 三平面映射
   - 优化的采样策略
   - 可配置的混合

## 质量分级系统

### 实现方案
使用Static Switch Parameters实现质量分级:

```
Quality Level | Instructions | Texture Samples | Features
-------------|--------------|-----------------|----------
Low          | < 150        | < 8             | 基础PBR
Medium       | < 200        | < 12            | + Detail Normal
High         | < 250        | < 14            | + Parallax
Epic         | < 300        | < 16            | + 所有特效
```

### 配置方法
1. 在主材质中添加StaticSwitchParameter "QualityLevel"
2. 使用Quality Switch节点控制特性启用
3. 在材质实例中设置质量级别
4. 通过Scalability设置自动切换

## 具体优化步骤

### 阶段1: 审计和分类 (已完成)
- ✓ 扫描所有材质文件
- ✓ 分类材质类型和用途
- ✓ 识别高复杂度材质

### 阶段2: 创建主材质系统
1. 创建4个主材质模板
2. 实现质量分级系统
3. 创建材质函数库
4. 测试和验证

### 阶段3: 迁移现有材质
1. 将现有材质转换为实例
2. 优化高复杂度材质
3. 统一命名规范
4. 更新引用

### 阶段4: 性能验证
1. 使用Shader Complexity视图检查
2. 测试不同质量级别
3. 性能profiling
4. 优化瓶颈

## 工具和命令

### UE5编辑器命令
```
# 显示Shader复杂度
viewmode shadercomplexity

# 显示材质指令数
stat shadercompiling

# 显示纹理采样
stat textures

# 重新编译所有材质
recompileshaders changed
```

### 材质编辑器快捷键
- Ctrl+Shift+S: 保存并应用
- Ctrl+F: 查找节点
- Ctrl+D: 复制节点
- Alt+拖动: 断开连接

## 性能基准

### 目标指标
- 不透明材质: < 200 instructions
- 遮罩材质: < 220 instructions
- 半透明材质: < 250 instructions
- 后处理材质: < 300 instructions
- 纹理采样: < 16 per material
- 参数数量: < 32 per material

### 当前状态
- 极高复杂度: $($analysis.ComplexityDistribution.VeryHigh) 个 (需要立即优化)
- 高复杂度: $($analysis.ComplexityDistribution.High) 个 (需要优化)
- 中等复杂度: $($analysis.ComplexityDistribution.Medium) 个 (可接受)
- 低复杂度: $($analysis.ComplexityDistribution.Low) 个 (良好)

## 下一步行动

### 立即执行
1. 创建主材质模板系统
2. 创建材质函数库
3. 优化前10个最复杂的材质

### 短期目标 (1-2天)
1. 迁移所有材质到新系统
2. 实现质量分级
3. 性能测试和验证

### 长期目标 (1周)
1. 完整的材质文档
2. 材质创建指南
3. 自动化优化工具
4. 持续性能监控

## 参考资源
- UE5 Material Best Practices
- GPU Performance Optimization
- Shader Complexity Guidelines
- Material Quality Scaling

---
报告生成: $(Get-Date -Format "yyyy-MM-dd HH:mm:ss")
"@

# 保存报告
$report | Out-File -FilePath $OutputPath -Encoding UTF8

Write-Host "优化报告已生成: $OutputPath" -ForegroundColor Green
Write-Host "`n关键发现:" -ForegroundColor Cyan
Write-Host "- 需要优化的材质: $($analysis.ComplexityDistribution.VeryHigh + $analysis.ComplexityDistribution.High) 个" -ForegroundColor Yellow
Write-Host "- 后处理材质: $($analysis.Summary.PostProcessMaterials) 个 (高优先级)" -ForegroundColor Yellow
Write-Host "- 半透明材质: $($analysis.CategoryDistribution.Translucent) 个 (需要审查)" -ForegroundColor Yellow
