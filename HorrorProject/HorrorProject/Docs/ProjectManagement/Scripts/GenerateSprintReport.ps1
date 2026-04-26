# GenerateSprintReport.ps1 - 生成Sprint报告
# Sprint结束时生成完整的Sprint回顾报告

param(
    [string]$DataPath = "D:/gptzuo/HorrorProject/HorrorProject/Docs/ProjectManagement/Data",
    [string]$ReportPath = "D:/gptzuo/HorrorProject/HorrorProject/Docs/ProjectManagement/Reports",
    [int]$SprintNumber,
    [int]$SprintDays = 14
)

$ErrorActionPreference = "Stop"

# 如果未指定Sprint编号，计算当前Sprint
if (-not $SprintNumber) {
    $projectStart = Get-Date "2026-01-01"
    $SprintNumber = [math]::Floor(((Get-Date) - $projectStart).TotalDays / $SprintDays) + 1
}

$sprintStart = (Get-Date "2026-01-01").AddDays(($SprintNumber - 1) * $SprintDays)
$sprintEnd = $sprintStart.AddDays($SprintDays - 1)

Write-Host "=== Sprint $SprintNumber 报告生成 ===" -ForegroundColor Cyan
Write-Host "Sprint周期: $($sprintStart.ToString('yyyy-MM-dd')) 至 $($sprintEnd.ToString('yyyy-MM-dd'))" -ForegroundColor Gray
Write-Host ""

# 生成Sprint报告内容
$markdown = @"
# Sprint $SprintNumber 回顾报告

**Sprint周期**: $($sprintStart.ToString('yyyy-MM-dd')) 至 $($sprintEnd.ToString('yyyy-MM-dd'))
**Sprint时长**: $SprintDays 天
**生成时间**: $(Get-Date -Format 'yyyy-MM-dd HH:mm:ss')

---

## Sprint目标

本Sprint的主要目标：
- 完成核心功能模块开发
- 提升代码质量和测试覆盖率
- 优化性能和用户体验
- 解决关键技术债务

---

## Sprint成果

### 完成的工作
- 功能开发: X个功能模块
- Bug修复: X个问题
- 代码审查: X次
- 文档更新: X篇

### 未完成的工作
- 待完成任务: X个
- 延期原因分析

---

## 团队表现

### 速度指标
- 计划速度: X点
- 实际速度: X点
- 完成率: X%

### 质量指标
- 代码提交: X次
- 代码审查通过率: X%
- Bug引入率: X%
- 测试覆盖率: X%

---

## 问题与挑战

### 遇到的问题
1. 技术问题
2. 流程问题
3. 资源问题

### 解决方案
1. 采取的措施
2. 效果评估

---

## 经验教训

### 做得好的地方
- 团队协作高效
- 技术方案合理
- 进度控制良好

### 需要改进的地方
- 估算准确性
- 沟通效率
- 工具使用

---

## 下个Sprint计划

### 目标设定
- 主要目标1
- 主要目标2
- 主要目标3

### 风险识别
- 潜在风险1
- 潜在风险2
- 缓解措施

---

*本报告由自动化系统生成*
*Sprint $SprintNumber | $(Get-Date -Format 'yyyy-MM-dd')*
"@

# 保存报告
if (-not (Test-Path $ReportPath)) {
    New-Item -ItemType Directory -Path $ReportPath -Force | Out-Null
}

$reportFile = Join-Path $ReportPath "SprintReport_$SprintNumber.md"
$markdown | Out-File -FilePath $reportFile -Encoding UTF8

Write-Host "Sprint报告已生成: $reportFile" -ForegroundColor Green
Write-Host ""
Write-Host "=== 生成完成 ===" -ForegroundColor Cyan
