# Community Feedback Tracker
# 社区反馈追踪器

<#
.SYNOPSIS
    追踪和管理来自各个渠道的社区反馈

.DESCRIPTION
    此脚本从多个来源收集玩家反馈，分类、优先级排序，
    并生成报告，帮助团队快速响应社区需求。

.PARAMETER OutputPath
    输出报告的路径

.PARAMETER Days
    追踪最近多少天的反馈

.EXAMPLE
    .\CommunityFeedbackTracker.ps1 -OutputPath "D:\Reports" -Days 7
#>

param(
    [Parameter(Mandatory=$false)]
    [string]$OutputPath = ".\Reports",

    [Parameter(Mandatory=$false)]
    [int]$Days = 7,

    [Parameter(Mandatory=$false)]
    [string]$ProjectName = "HorrorProject",

    [Parameter(Mandatory=$false)]
    [switch]$ExportCSV = $false
)

$ErrorActionPreference = "Stop"

# 颜色输出
function Write-ColorOutput {
    param([string]$Message, [string]$Color = "White")
    Write-Host $Message -ForegroundColor $Color
}

# 反馈数据结构
class Feedback {
    [string]$Id
    [datetime]$Date
    [string]$Source
    [string]$Author
    [string]$Content
    [string]$Category
    [string]$Priority
    [string]$Status
    [string]$Tags
    [string]$AssignedTo
    [string]$Notes
}

# 初始化
$feedbackList = @()
$categories = @{
    "Bug" = 0
    "Feature" = 0
    "Balance" = 0
    "Performance" = 0
    "UI/UX" = 0
    "Content" = 0
    "Audio" = 0
    "Localization" = 0
    "Other" = 0
}

Write-ColorOutput "`n========================================" "Cyan"
Write-ColorOutput "  社区反馈追踪器" "Cyan"
Write-ColorOutput "========================================`n" "Cyan"

# 创建输出目录
if (-not (Test-Path $OutputPath)) {
    New-Item -ItemType Directory -Path $OutputPath -Force | Out-Null
}

# 模拟数据收集（实际使用中需要集成API）
Write-ColorOutput "收集反馈数据..." "Yellow"

# Steam评论分析（模拟）
Write-ColorOutput "  - 分析Steam评论..." "Gray"
$steamFeedback = @(
    @{
        Date = (Get-Date).AddDays(-2)
        Source = "Steam"
        Author = "Player123"
        Content = "游戏很棒，但第三章有个存档bug"
        Category = "Bug"
        Priority = "High"
    },
    @{
        Date = (Get-Date).AddDays(-1)
        Source = "Steam"
        Author = "GamerXYZ"
        Content = "希望能添加更多难度选项"
        Category = "Feature"
        Priority = "Medium"
    }
)

foreach ($item in $steamFeedback) {
    $fb = [Feedback]::new()
    $fb.Id = "STEAM-" + (Get-Random -Maximum 9999).ToString("D4")
    $fb.Date = $item.Date
    $fb.Source = $item.Source
    $fb.Author = $item.Author
    $fb.Content = $item.Content
    $fb.Category = $item.Category
    $fb.Priority = $item.Priority
    $fb.Status = "New"
    $feedbackList += $fb
    $categories[$item.Category]++
}

# Discord反馈（模拟）
Write-ColorOutput "  - 收集Discord反馈..." "Gray"
$discordFeedback = @(
    @{
        Date = (Get-Date).AddDays(-3)
        Source = "Discord"
        Author = "DiscordUser#1234"
        Content = "地下室关卡帧率下降严重"
        Category = "Performance"
        Priority = "High"
    },
    @{
        Date = (Get-Date).AddHours(-12)
        Source = "Discord"
        Author = "HorrorFan#5678"
        Content = "音效设计太棒了！"
        Category = "Other"
        Priority = "Low"
    }
)

foreach ($item in $discordFeedback) {
    $fb = [Feedback]::new()
    $fb.Id = "DISC-" + (Get-Random -Maximum 9999).ToString("D4")
    $fb.Date = $item.Date
    $fb.Source = $item.Source
    $fb.Author = $item.Author
    $fb.Content = $item.Content
    $fb.Category = $item.Category
    $fb.Priority = $item.Priority
    $fb.Status = "New"
    $feedbackList += $fb
    $categories[$item.Category]++
}

# Reddit讨论（模拟）
Write-ColorOutput "  - 扫描Reddit讨论..." "Gray"
$redditFeedback = @(
    @{
        Date = (Get-Date).AddDays(-1)
        Source = "Reddit"
        Author = "u/redditor123"
        Content = "UI字体太小，难以阅读"
        Category = "UI/UX"
        Priority = "Medium"
    }
)

foreach ($item in $redditFeedback) {
    $fb = [Feedback]::new()
    $fb.Id = "REDD-" + (Get-Random -Maximum 9999).ToString("D4")
    $fb.Date = $item.Date
    $fb.Source = $item.Source
    $fb.Author = $item.Author
    $fb.Content = $item.Content
    $fb.Category = $item.Category
    $fb.Priority = $item.Priority
    $fb.Status = "New"
    $feedbackList += $fb
    $categories[$item.Category]++
}

# Twitter提及（模拟）
Write-ColorOutput "  - 监控Twitter提及..." "Gray"

# 客服工单（模拟）
Write-ColorOutput "  - 导入客服工单..." "Gray"

Write-ColorOutput "✓ 收集了 $($feedbackList.Count) 条反馈" "Green"

# 分析反馈
Write-ColorOutput "`n分析反馈数据..." "Yellow"

$priorityCount = @{
    "Critical" = ($feedbackList | Where-Object { $_.Priority -eq "Critical" }).Count
    "High" = ($feedbackList | Where-Object { $_.Priority -eq "High" }).Count
    "Medium" = ($feedbackList | Where-Object { $_.Priority -eq "Medium" }).Count
    "Low" = ($feedbackList | Where-Object { $_.Priority -eq "Low" }).Count
}

$sourceCount = @{}
foreach ($fb in $feedbackList) {
    if ($sourceCount.ContainsKey($fb.Source)) {
        $sourceCount[$fb.Source]++
    } else {
        $sourceCount[$fb.Source] = 1
    }
}

# 生成报告
Write-ColorOutput "`n生成报告..." "Yellow"

$timestamp = Get-Date -Format "yyyyMMdd_HHmmss"
$reportPath = Join-Path $OutputPath "FeedbackReport_$timestamp.md"

$report = @"
# 社区反馈报告

**生成时间:** $(Get-Date -Format "yyyy-MM-dd HH:mm:ss")
**时间范围:** 最近 $Days 天
**总反馈数:** $($feedbackList.Count)

---

## 执行摘要

### 优先级分布
- **Critical:** $($priorityCount.Critical) 条
- **High:** $($priorityCount.High) 条
- **Medium:** $($priorityCount.Medium) 条
- **Low:** $($priorityCount.Low) 条

### 来源分布
"@

foreach ($source in $sourceCount.Keys) {
    $report += "`n- **$source:** $($sourceCount[$source]) 条"
}

$report += @"

### 类别分布
"@

foreach ($cat in $categories.Keys | Sort-Object { $categories[$_] } -Descending) {
    if ($categories[$cat] -gt 0) {
        $report += "`n- **$cat:** $($categories[$cat]) 条"
    }
}

$report += @"

---

## 高优先级反馈

"@

$highPriorityFeedback = $feedbackList | Where-Object { $_.Priority -in @("Critical", "High") } | Sort-Object Date -Descending

if ($highPriorityFeedback.Count -gt 0) {
    foreach ($fb in $highPriorityFeedback) {
        $report += @"

### [$($fb.Id)] $($fb.Category) - $($fb.Priority)
- **来源:** $($fb.Source)
- **作者:** $($fb.Author)
- **日期:** $($fb.Date.ToString("yyyy-MM-dd HH:mm"))
- **内容:** $($fb.Content)
- **状态:** $($fb.Status)

"@
    }
} else {
    $report += "`n*无高优先级反馈*`n"
}

$report += @"

---

## 按类别分类

"@

foreach ($cat in $categories.Keys | Sort-Object) {
    $catFeedback = $feedbackList | Where-Object { $_.Category -eq $cat }
    if ($catFeedback.Count -gt 0) {
        $report += @"

### $cat ($($catFeedback.Count) 条)

"@
        foreach ($fb in $catFeedback | Sort-Object Priority, Date -Descending | Select-Object -First 5) {
            $report += "- [$($fb.Priority)] $($fb.Content) - *$($fb.Source)* ($($fb.Date.ToString("MM-dd")))`n"
        }
        if ($catFeedback.Count -gt 5) {
            $report += "`n*...还有 $($catFeedback.Count - 5) 条反馈*`n"
        }
    }
}

$report += @"

---

## 趋势分析

### 最常见问题
"@

# 简单的关键词分析
$keywords = @{}
foreach ($fb in $feedbackList) {
    $words = $fb.Content -split '\s+' | Where-Object { $_.Length -gt 3 }
    foreach ($word in $words) {
        $word = $word.ToLower() -replace '[^\w]', ''
        if ($word.Length -gt 3) {
            if ($keywords.ContainsKey($word)) {
                $keywords[$word]++
            } else {
                $keywords[$word] = 1
            }
        }
    }
}

$topKeywords = $keywords.GetEnumerator() | Sort-Object Value -Descending | Select-Object -First 10

foreach ($kw in $topKeywords) {
    $report += "`n- **$($kw.Key):** 提及 $($kw.Value) 次"
}

$report += @"

### 情绪分析
- **正面:** $(($feedbackList | Where-Object { $_.Content -match '棒|好|喜欢|优秀|完美' }).Count) 条
- **负面:** $(($feedbackList | Where-Object { $_.Content -match 'bug|问题|糟糕|失望|差' }).Count) 条
- **中性:** $(($feedbackList | Where-Object { $_.Content -notmatch '棒|好|喜欢|优秀|完美|bug|问题|糟糕|失望|差' }).Count) 条

---

## 行动建议

### 立即处理
"@

$criticalItems = $feedbackList | Where-Object { $_.Priority -eq "Critical" -or $_.Priority -eq "High" }
if ($criticalItems.Count -gt 0) {
    foreach ($item in $criticalItems | Select-Object -First 5) {
        $report += "`n- [ ] [$($item.Id)] $($item.Content)"
    }
} else {
    $report += "`n*无紧急项目*"
}

$report += @"

### 短期计划（本周）
"@

$mediumItems = $feedbackList | Where-Object { $_.Priority -eq "Medium" }
if ($mediumItems.Count -gt 0) {
    foreach ($item in $mediumItems | Select-Object -First 5) {
        $report += "`n- [ ] [$($item.Id)] $($item.Content)"
    }
} else {
    $report += "`n*无中等优先级项目*"
}

$report += @"

### 长期考虑
"@

$lowItems = $feedbackList | Where-Object { $_.Priority -eq "Low" }
if ($lowItems.Count -gt 0) {
    foreach ($item in $lowItems | Select-Object -First 5) {
        $report += "`n- [ ] [$($item.Id)] $($item.Content)"
    }
} else {
    $report += "`n*无低优先级项目*"
}

$report += @"

---

## 附录

### 完整反馈列表

| ID | 日期 | 来源 | 类别 | 优先级 | 内容 |
|----|------|------|------|--------|------|
"@

foreach ($fb in $feedbackList | Sort-Object Date -Descending) {
    $shortContent = if ($fb.Content.Length -gt 50) { $fb.Content.Substring(0, 47) + "..." } else { $fb.Content }
    $report += "`n| $($fb.Id) | $($fb.Date.ToString("MM-dd HH:mm")) | $($fb.Source) | $($fb.Category) | $($fb.Priority) | $shortContent |"
}

$report += @"

---

**报告生成器:** CommunityFeedbackTracker v1.0
**下次更新:** $(Get-Date).AddDays(1).ToString("yyyy-MM-dd")
"@

# 保存报告
$report | Out-File -FilePath $reportPath -Encoding UTF8
Write-ColorOutput "✓ 报告已保存: $reportPath" "Green"

# 导出CSV（如果需要）
if ($ExportCSV) {
    $csvPath = Join-Path $OutputPath "FeedbackData_$timestamp.csv"
    $feedbackList | Export-Csv -Path $csvPath -NoTypeInformation -Encoding UTF8
    Write-ColorOutput "✓ CSV已导出: $csvPath" "Green"
}

# 生成统计摘要
Write-ColorOutput "`n========================================" "Cyan"
Write-ColorOutput "  反馈统计摘要" "Cyan"
Write-ColorOutput "========================================`n" "Cyan"

Write-ColorOutput "总反馈数: $($feedbackList.Count)" "White"
Write-ColorOutput "`n优先级分布:" "Yellow"
Write-ColorOutput "  Critical: $($priorityCount.Critical)" $(if ($priorityCount.Critical -gt 0) { "Red" } else { "Gray" })
Write-ColorOutput "  High: $($priorityCount.High)" $(if ($priorityCount.High -gt 0) { "Yellow" } else { "Gray" })
Write-ColorOutput "  Medium: $($priorityCount.Medium)" "White"
Write-ColorOutput "  Low: $($priorityCount.Low)" "Gray"

Write-ColorOutput "`n热门类别:" "Yellow"
$topCategories = $categories.GetEnumerator() | Sort-Object Value -Descending | Select-Object -First 3
foreach ($cat in $topCategories) {
    if ($cat.Value -gt 0) {
        Write-ColorOutput "  $($cat.Key): $($cat.Value)" "White"
    }
}

Write-ColorOutput "`n主要来源:" "Yellow"
foreach ($source in $sourceCount.Keys) {
    Write-ColorOutput "  $source: $($sourceCount[$source])" "White"
}

Write-ColorOutput "`n下一步行动:" "Yellow"
if ($priorityCount.Critical -gt 0 -or $priorityCount.High -gt 0) {
    Write-ColorOutput "  ⚠ 有 $($priorityCount.Critical + $priorityCount.High) 条高优先级反馈需要立即处理！" "Red"
} else {
    Write-ColorOutput "  ✓ 无紧急问题" "Green"
}

Write-ColorOutput "`n报告位置: $reportPath" "Cyan"

# 打开报告（可选）
if ($Host.Name -eq "ConsoleHost") {
    $openReport = Read-Host "`n是否打开报告？(Y/N)"
    if ($openReport -eq "Y" -or $openReport -eq "y") {
        Invoke-Item $reportPath
    }
}

Write-ColorOutput "`n完成！`n" "Green"
