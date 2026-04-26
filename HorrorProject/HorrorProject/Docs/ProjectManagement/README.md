# 项目管理系统

HorrorProject 完整的项目管理和报告系统，提供自动化的进度追踪、风险管理、资源监控和报告生成功能。

## 目录结构

```
ProjectManagement/
├── Scripts/                    # 自动化脚本
│   ├── TrackProgress.ps1      # 进度追踪主程序
│   ├── UpdateMilestones.ps1   # 更新里程碑
│   ├── CalculateVelocity.ps1  # 计算开发速度
│   ├── PredictCompletion.ps1  # 预测完成时间
│   ├── GenerateDailyReport.ps1    # 每日报告
│   ├── GenerateWeeklyReport.ps1   # 周报
│   ├── GenerateSprintReport.ps1   # Sprint报告
│   ├── IdentifyRisks.ps1      # 识别风险
│   ├── AssessRisks.ps1        # 评估风险
│   ├── MitigateRisks.ps1      # 缓解风险
│   ├── TrackResources.ps1     # 资源追踪
│   └── RunAll.ps1             # 主控脚本
├── Data/                       # 数据文件
│   ├── progress_*.json        # 进度数据
│   ├── velocity_*.json        # 速度数据
│   ├── milestones.json        # 里程碑数据
│   ├── risks.json             # 风险数据
│   └── resources.json         # 资源数据
├── Reports/                    # 报告文件
│   ├── DailyReport_*.md       # 每日报告
│   ├── WeeklyReport_*.md      # 周报
│   └── SprintReport_*.md      # Sprint报告
├── Templates/                  # 模板文件
├── ProjectCharter.md          # 项目章程
├── ProjectPlan.md             # 项目计划
├── RiskRegister.md            # 风险登记册
├── LessonsLearned.md          # 经验教训
└── README.md                  # 本文件
```

## 快速开始

### 一键运行所有任务

```powershell
cd D:/gptzuo/HorrorProject/HorrorProject/Docs/ProjectManagement/Scripts
.\RunAll.ps1
```

### 生成完整报告

```powershell
.\RunAll.ps1 -FullReport
```

### 仅收集数据（跳过报告）

```powershell
.\RunAll.ps1 -SkipReports
```

### 详细输出模式

```powershell
.\RunAll.ps1 -Verbose
```

## 功能模块

### 1. 进度追踪系统

**TrackProgress.ps1** - 自动化收集和分析项目进度数据

功能：
- Git统计（提交次数、贡献者、分支等）
- 代码指标（文件数、代码行数、文件类型分布）
- 任务进度（完成率、进行中、受阻任务）
- Sprint指标（Sprint编号、剩余天数）
- 项目健康度评估

使用：
```powershell
.\TrackProgress.ps1
.\TrackProgress.ps1 -GenerateReport  # 同时生成报告
.\TrackProgress.ps1 -Verbose         # 显示详细信息
```

### 2. 里程碑管理

**UpdateMilestones.ps1** - 管理和追踪项目关键里程碑

功能：
- 列出所有里程碑
- 添加新里程碑
- 更新里程碑状态和进度
- 完成里程碑
- 生成里程碑报告

使用：
```powershell
# 列出里程碑
.\UpdateMilestones.ps1 -Action list

# 更新里程碑
.\UpdateMilestones.ps1 -Action update -MilestoneName "M003" -Progress 75

# 完成里程碑
.\UpdateMilestones.ps1 -Action complete -MilestoneName "M003"

# 生成报告
.\UpdateMilestones.ps1 -Action report
```

### 3. 速度计算

**CalculateVelocity.ps1** - 分析团队开发速度和生产力趋势

功能：
- 提交速度分析
- 代码变化速度
- 任务完成速度
- Sprint速度预测
- 趋势分析

使用：
```powershell
.\CalculateVelocity.ps1
.\CalculateVelocity.ps1 -ShowTrends    # 显示趋势分析
.\CalculateVelocity.ps1 -HistoryDays 30  # 分析最近30天
```

### 4. 完成预测

**PredictCompletion.ps1** - 基于历史数据预测项目完成时间

功能：
- 线性预测完成日期
- 场景分析（乐观/现实/悲观）
- 里程碑进度分析
- 风险识别
- 进度偏差计算

使用：
```powershell
.\PredictCompletion.ps1
.\PredictCompletion.ps1 -ShowScenarios      # 显示场景分析
.\PredictCompletion.ps1 -DetailedAnalysis   # 详细分析
```

### 5. 报告生成

#### 每日报告
**GenerateDailyReport.ps1** - 生成项目每日进度报告

功能：
- 执行摘要
- 今日活动统计
- 对比分析
- 项目健康分析
- 下一步行动建议

使用：
```powershell
.\GenerateDailyReport.ps1
```

#### 周报
**GenerateWeeklyReport.ps1** - 生成项目周报

功能：
- 周汇总统计
- 本周亮点
- 团队贡献排行
- 趋势分析
- 下周计划

使用：
```powershell
.\GenerateWeeklyReport.ps1
```

#### Sprint报告
**GenerateSprintReport.ps1** - 生成Sprint回顾报告

使用：
```powershell
.\GenerateSprintReport.ps1
.\GenerateSprintReport.ps1 -SprintNumber 5  # 指定Sprint编号
```

### 6. 风险管理系统

#### 识别风险
**IdentifyRisks.ps1** - 自动扫描和识别项目风险

功能：
- 技术风险扫描
- 进度风险识别
- 质量风险检测
- 资源风险分析

使用：
```powershell
.\IdentifyRisks.ps1
.\IdentifyRisks.ps1 -AutoAssess  # 自动评估
.\IdentifyRisks.ps1 -Verbose     # 详细输出
```

#### 评估风险
**AssessRisks.ps1** - 对识别的风险进行评估和优先级排序

功能：
- 风险分数计算
- 优先级排序
- 风险矩阵生成
- 评估报告

使用：
```powershell
.\AssessRisks.ps1
.\AssessRisks.ps1 -GenerateReport  # 生成详细报告
```

#### 缓解风险
**MitigateRisks.ps1** - 制定和执行风险缓解措施

功能：
- 列出缓解计划
- 自动生成缓解策略
- 执行缓解措施
- 关闭风险

使用：
```powershell
# 列出所有缓解计划
.\MitigateRisks.ps1 -Action list

# 为风险生成缓解计划
.\MitigateRisks.ps1 -Action plan -RiskId "TECH-001" -AutoGenerate

# 开始执行缓解措施
.\MitigateRisks.ps1 -Action execute -RiskId "TECH-001"

# 关闭风险
.\MitigateRisks.ps1 -Action close -RiskId "TECH-001"
```

### 7. 资源追踪

**TrackResources.ps1** - 追踪和监控项目资源使用情况

功能：
- 磁盘资源监控
- 团队资源分析
- 时间资源计算
- 资源利用率分析

使用：
```powershell
.\TrackResources.ps1
```

## 数据文件说明

### progress_*.json
进度追踪数据，包含：
- Git统计信息
- 代码指标
- 任务进度
- Sprint信息
- 项目健康度

### velocity_*.json
速度数据，包含：
- 提交速度
- 代码速度
- 任务速度
- 趋势信息

### milestones.json
里程碑数据，包含：
- 里程碑列表
- 进度状态
- 目标和实际日期
- 依赖关系

### risks.json
风险数据，包含：
- 风险列表
- 风险评估
- 缓解计划
- 风险状态

### resources.json
资源数据，包含：
- 磁盘使用情况
- 团队资源分配
- 时间资源消耗
- 利用率分析

## 报告说明

### 每日报告 (DailyReport_*.md)
- Markdown格式
- 包含HTML版本
- 执行摘要
- 今日活动
- 对比分析
- 健康度评估

### 周报 (WeeklyReport_*.md)
- 周汇总统计
- 本周亮点
- 团队贡献
- 趋势分析

### Sprint报告 (SprintReport_*.md)
- Sprint目标
- 完成情况
- 团队表现
- 经验教训

## 自动化建议

### 每日自动运行
创建Windows任务计划：
```powershell
# 每天早上9点运行
$action = New-ScheduledTaskAction -Execute "PowerShell.exe" -Argument "-File D:\gptzuo\HorrorProject\HorrorProject\Docs\ProjectManagement\Scripts\RunAll.ps1"
$trigger = New-ScheduledTaskTrigger -Daily -At 9am
Register-ScheduledTask -Action $action -Trigger $trigger -TaskName "HorrorProject-DailyReport"
```

### 每周自动生成周报
```powershell
# 每周五下午5点运行
$action = New-ScheduledTaskAction -Execute "PowerShell.exe" -Argument "-File D:\gptzuo\HorrorProject\HorrorProject\Docs\ProjectManagement\Scripts\RunAll.ps1 -FullReport"
$trigger = New-ScheduledTaskTrigger -Weekly -DaysOfWeek Friday -At 5pm
Register-ScheduledTask -Action $action -Trigger $trigger -TaskName "HorrorProject-WeeklyReport"
```

## 集成Git数据

所有脚本自动从Git仓库提取数据：
- 提交历史
- 贡献者信息
- 分支状态
- 代码变化

确保在Git仓库目录下运行脚本。

## 导出功能

### 支持的格式
- JSON: 结构化数据存储
- CSV: 历史数据追踪
- Markdown: 人类可读报告
- HTML: 网页格式报告

### 导出PDF（可选）
安装Pandoc后可导出PDF：
```powershell
pandoc DailyReport_2026-04-26.md -o DailyReport_2026-04-26.pdf
```

## 故障排除

### 脚本执行策略
如果遇到执行策略错误：
```powershell
Set-ExecutionPolicy -ExecutionPolicy RemoteSigned -Scope CurrentUser
```

### Git命令失败
确保Git已安装并在PATH中：
```powershell
git --version
```

### 数据文件不存在
首次运行会自动创建数据文件和目录。

## 最佳实践

1. **每日运行**: 建议每天运行一次TrackProgress.ps1
2. **定期评估**: 每周运行风险识别和评估
3. **及时更新**: 里程碑完成后及时更新状态
4. **保留历史**: 不要删除历史数据文件
5. **查看报告**: 定期查看生成的报告

## 技术要求

- Windows 10/11
- PowerShell 5.1+
- Git 2.0+
- 磁盘空间: 至少1GB

## 许可

本项目管理系统为HorrorProject内部使用。

## 支持

如有问题，请联系项目管理团队。

---

**最后更新**: 2026-04-26  
**版本**: 1.0  
**维护者**: 项目管理团队
