# GenerateDailyReport.ps1 - 生成每日报告
# 自动生成项目每日进度报告

param(
    [string]$ProjectRoot = "D:/gptzuo/HorrorProject",
    [string]$DataPath = "D:/gptzuo/HorrorProject/HorrorProject/Docs/ProjectManagement/Data",
    [string]$ReportPath = "D:/gptzuo/HorrorProject/HorrorProject/Docs/ProjectManagement/Reports",
    [string]$DataFile,
    [switch]$SendEmail,
    [switch]$ExportPDF
)

$ErrorActionPreference = "Stop"

function Get-TodayData {
    param([string]$DataPath)

    $today = Get-Date -Format "yyyy-MM-dd"

    # 查找今天的数据文件
    $progressFiles = Get-ChildItem -Path $DataPath -Filter "progress_$today*.json" -ErrorAction SilentlyContinue

    if ($progressFiles.Count -gt 0) {
        $latestFile = $progressFiles | Sort-Object LastWriteTime -Descending | Select-Object -First 1
        $data = Get-Content $latestFile.FullName -Raw -Encoding UTF8 | ConvertFrom-Json
        return $data
    }

    return $null
}

function Get-YesterdayData {
    param([string]$DataPath)

    $yesterday = (Get-Date).AddDays(-1).ToString("yyyy-MM-dd")

    $progressFiles = Get-ChildItem -Path $DataPath -Filter "progress_$yesterday*.json" -ErrorAction SilentlyContinue

    if ($progressFiles.Count -gt 0) {
        $latestFile = $progressFiles | Sort-Object LastWriteTime -Descending | Select-Object -First 1
        $data = Get-Content $latestFile.FullName -Raw -Encoding UTF8 | ConvertFrom-Json
        return $data
    }

    return $null
}

function Compare-DailyProgress {
    param($TodayData, $YesterdayData)

    $comparison = @{
        CommitsDelta = 0
        LinesDelta = 0
        TasksDelta = 0
        CompletionRateDelta = 0
        HealthScoreDelta = 0
        Improvements = @()
        Concerns = @()
    }

    if ($YesterdayData) {
        $comparison.CommitsDelta = $TodayData.GitStatistics.TotalCommits - $YesterdayData.GitStatistics.TotalCommits
        $comparison.LinesDelta = $TodayData.CodeMetrics.TotalLines - $YesterdayData.CodeMetrics.TotalLines
        $comparison.TasksDelta = $TodayData.TaskProgress.CompletedTasks - $YesterdayData.TaskProgress.CompletedTasks
        $comparison.CompletionRateDelta = $TodayData.TaskProgress.CompletionRate - $YesterdayData.TaskProgress.CompletionRate
        $comparison.HealthScoreDelta = $TodayData.ProjectHealth.OverallScore - $YesterdayData.ProjectHealth.OverallScore

        # 分析改进
        if ($comparison.CommitsDelta -gt 5) {
            $comparison.Improvements += "提交活动显著增加"
        }
        if ($comparison.TasksDelta -gt 3) {
            $comparison.Improvements += "任务完成速度提升"
        }
        if ($comparison.HealthScoreDelta -gt 5) {
            $comparison.Improvements += "项目健康度改善"
        }

        # 分析问题
        if ($comparison.CommitsDelta -lt 0) {
            $comparison.Concerns += "提交活动减少"
        }
        if ($comparison.CompletionRateDelta -lt 0) {
            $comparison.Concerns += "完成率下降"
        }
        if ($comparison.HealthScoreDelta -lt -5) {
            $comparison.Concerns += "项目健康度下降"
        }
    }

    return $comparison
}

function Get-TodayHighlights {
    param($TodayData, $ProjectRoot)

    $highlights = @{
        TopCommits = @()
        NewFeatures = @()
        BugFixes = @()
        Documentation = @()
    }

    Push-Location $ProjectRoot
    try {
        # 获取今天的提交
        $today = Get-Date -Format "yyyy-MM-dd"
        $commits = git log --since="$today 00:00:00" --format="%h|%s|%an" 2>$null

        if ($commits) {
            $commitList = $commits -split "`n" | Where-Object { $_ }

            foreach ($commit in $commitList | Select-Object -First 10) {
                $parts = $commit -split '\|'
                $hash = $parts[0]
                $message = $parts[1]
                $author = $parts[2]

                $highlights.TopCommits += @{
                    Hash = $hash
                    Message = $message
                    Author = $author
                }

                # 分类提交
                if ($message -match '(feat|feature|add|new)') {
                    $highlights.NewFeatures += $message
                }
                elseif ($message -match '(fix|bug|issue)') {
                    $highlights.BugFixes += $message
                }
                elseif ($message -match '(doc|docs|documentation)') {
                    $highlights.Documentation += $message
                }
            }
        }
    }
    finally {
        Pop-Location
    }

    return $highlights
}

function Generate-MarkdownReport {
    param($TodayData, $YesterdayData, $Comparison, $Highlights, [string]$OutputPath)

    $today = Get-Date -Format "yyyy-MM-dd"
    $reportDate = Get-Date -Format "yyyy年MM月dd日"

    $markdown = @"
# HorrorProject 每日进度报告

**日期**: $reportDate
**报告生成时间**: $(Get-Date -Format 'HH:mm:ss')

---

## 执行摘要

### 项目健康度
- **状态**: $($TodayData.ProjectHealth.Status)
- **得分**: $($TodayData.ProjectHealth.OverallScore)/100
"@

    if ($Comparison.HealthScoreDelta -ne 0) {
        $deltaSymbol = if ($Comparison.HealthScoreDelta -gt 0) { "↑" } else { "↓" }
        $markdown += " ($deltaSymbol $([Math]::Abs($Comparison.HealthScoreDelta)))"
    }

    $markdown += @"


### 关键指标
| 指标 | 今日 | 变化 |
|------|------|------|
| 提交次数 | $($TodayData.GitStatistics.TotalCommits) | +$($Comparison.CommitsDelta) |
| 代码行数 | $($TodayData.CodeMetrics.TotalLines) | +$($Comparison.LinesDelta) |
| 任务完成 | $($TodayData.TaskProgress.CompletedTasks)/$($TodayData.TaskProgress.TotalTasks) | +$($Comparison.TasksDelta) |
| 完成率 | $($TodayData.TaskProgress.CompletionRate)% | +$([Math]::Round($Comparison.CompletionRateDelta, 2))% |

---

## 今日活动

### Git统计
- **今日提交**: $($TodayData.GitStatistics.CommitsToday) 次
- **本周提交**: $($TodayData.GitStatistics.CommitsThisWeek) 次
- **活跃分支**: $($TodayData.GitStatistics.Branches) 个
- **贡献者**: $($TodayData.GitStatistics.Contributors) 人

### 代码变化
- **总文件数**: $($TodayData.CodeMetrics.TotalFiles)
- **总代码行**: $($TodayData.CodeMetrics.TotalLines)
- **文件分布**:
"@

    foreach ($type in $TodayData.CodeMetrics.FilesByType.PSObject.Properties) {
        $markdown += "`n  - $($type.Name): $($type.Value) 个"
    }

    $markdown += @"


### 任务进度
- **已完成**: $($TodayData.TaskProgress.CompletedTasks) 个
- **进行中**: $($TodayData.TaskProgress.InProgressTasks) 个
- **受阻**: $($TodayData.TaskProgress.BlockedTasks) 个
- **完成率**: $($TodayData.TaskProgress.CompletionRate)%

---

## 今日亮点

"@

    if ($Highlights.TopCommits.Count -gt 0) {
        $markdown += "### 主要提交`n`n"
        foreach ($commit in $Highlights.TopCommits | Select-Object -First 5) {
            $markdown += "- ``$($commit.Hash)`` $($commit.Message) - *$($commit.Author)*`n"
        }
        $markdown += "`n"
    }

    if ($Highlights.NewFeatures.Count -gt 0) {
        $markdown += "### 新功能`n`n"
        foreach ($feature in $Highlights.NewFeatures | Select-Object -First 5) {
            $markdown += "- $feature`n"
        }
        $markdown += "`n"
    }

    if ($Highlights.BugFixes.Count -gt 0) {
        $markdown += "### Bug修复`n`n"
        foreach ($fix in $Highlights.BugFixes | Select-Object -First 5) {
            $markdown += "- $fix`n"
        }
        $markdown += "`n"
    }

    $markdown += @"

---

## 项目健康分析

### 健康指标
"@

    foreach ($indicator in $TodayData.ProjectHealth.Indicators.PSObject.Properties) {
        $markdown += "`n- **$($indicator.Name)**: $($indicator.Value)"
    }

    if ($TodayData.ProjectHealth.Risks.Count -gt 0) {
        $markdown += "`n`n### 风险提示`n`n"
        foreach ($risk in $TodayData.ProjectHealth.Risks) {
            $markdown += "- ⚠️ $risk`n"
        }
    }

    if ($TodayData.ProjectHealth.Recommendations.Count -gt 0) {
        $markdown += "`n### 改进建议`n`n"
        foreach ($rec in $TodayData.ProjectHealth.Recommendations) {
            $markdown += "- 💡 $rec`n"
        }
    }

    $markdown += @"


---

## 对比分析

"@

    if ($Comparison.Improvements.Count -gt 0) {
        $markdown += "### 改进项`n`n"
        foreach ($improvement in $Comparison.Improvements) {
            $markdown += "- ✅ $improvement`n"
        }
        $markdown += "`n"
    }

    if ($Comparison.Concerns.Count -gt 0) {
        $markdown += "### 关注点`n`n"
        foreach ($concern in $Comparison.Concerns) {
            $markdown += "- ⚠️ $concern`n"
        }
        $markdown += "`n"
    }

    $markdown += @"

---

## Sprint信息

- **Sprint编号**: 第 $($TodayData.SprintMetrics.SprintNumber) 个Sprint
- **Sprint周期**: $($TodayData.SprintMetrics.SprintStartDate.ToString('yyyy-MM-dd')) 至 $($TodayData.SprintMetrics.SprintEndDate.ToString('yyyy-MM-dd'))
- **剩余天数**: $($TodayData.SprintMetrics.DaysRemaining) 天

---

## 下一步行动

"@

    # 基于健康状态生成建议
    if ($TodayData.ProjectHealth.Status -eq "Critical" -or $TodayData.ProjectHealth.Status -eq "At Risk") {
        $markdown += @"
1. 立即召开团队会议，讨论当前问题
2. 重新评估任务优先级
3. 识别并移除阻碍因素
4. 考虑调整资源分配

"@
    }
    elseif ($TodayData.TaskProgress.BlockedTasks -gt 0) {
        $markdown += @"
1. 解决 $($TodayData.TaskProgress.BlockedTasks) 个受阻任务
2. 继续推进进行中的任务
3. 保持当前开发节奏

"@
    }
    else {
        $markdown += @"
1. 继续保持良好的开发节奏
2. 关注即将到来的里程碑
3. 定期更新文档和测试

"@
    }

    $markdown += @"

---

*报告由自动化系统生成*
*数据来源: $($TodayData.Timestamp)*
"@

    # 保存报告
    if (-not (Test-Path $OutputPath)) {
        New-Item -ItemType Directory -Path $OutputPath -Force | Out-Null
    }

    $reportFile = Join-Path $OutputPath "DailyReport_$today.md"
    $markdown | Out-File -FilePath $reportFile -Encoding UTF8

    return $reportFile
}

function Generate-HTMLReport {
    param([string]$MarkdownFile, [string]$OutputPath)

    $htmlFile = $MarkdownFile -replace '\.md$', '.html'

    $markdown = Get-Content $MarkdownFile -Raw -Encoding UTF8

    # 简单的Markdown到HTML转换
    $html = @"
<!DOCTYPE html>
<html lang="zh-CN">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>HorrorProject 每日报告 - $(Get-Date -Format 'yyyy-MM-dd')</title>
    <style>
        body {
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            line-height: 1.6;
            max-width: 1200px;
            margin: 0 auto;
            padding: 20px;
            background-color: #f5f5f5;
        }
        .container {
            background-color: white;
            padding: 30px;
            border-radius: 8px;
            box-shadow: 0 2px 4px rgba(0,0,0,0.1);
        }
        h1 {
            color: #2c3e50;
            border-bottom: 3px solid #3498db;
            padding-bottom: 10px;
        }
        h2 {
            color: #34495e;
            margin-top: 30px;
            border-left: 4px solid #3498db;
            padding-left: 10px;
        }
        h3 {
            color: #7f8c8d;
        }
        table {
            width: 100%;
            border-collapse: collapse;
            margin: 20px 0;
        }
        th, td {
            padding: 12px;
            text-align: left;
            border-bottom: 1px solid #ddd;
        }
        th {
            background-color: #3498db;
            color: white;
        }
        tr:hover {
            background-color: #f5f5f5;
        }
        .status-healthy { color: #27ae60; font-weight: bold; }
        .status-fair { color: #f39c12; font-weight: bold; }
        .status-risk { color: #e74c3c; font-weight: bold; }
        .metric-positive { color: #27ae60; }
        .metric-negative { color: #e74c3c; }
        code {
            background-color: #f4f4f4;
            padding: 2px 6px;
            border-radius: 3px;
            font-family: 'Courier New', monospace;
        }
        ul {
            list-style-type: none;
            padding-left: 0;
        }
        li {
            padding: 5px 0;
            padding-left: 20px;
        }
        li:before {
            content: "▸ ";
            color: #3498db;
            font-weight: bold;
        }
        .footer {
            margin-top: 40px;
            padding-top: 20px;
            border-top: 1px solid #ddd;
            color: #7f8c8d;
            font-size: 0.9em;
            text-align: center;
        }
    </style>
</head>
<body>
    <div class="container">
"@

    # 转换Markdown内容
    $html += $markdown -replace '# (.*)', '<h1>$1</h1>' `
        -replace '## (.*)', '<h2>$1</h2>' `
        -replace '### (.*)', '<h3>$1</h3>' `
        -replace '\*\*(.*?)\*\*', '<strong>$1</strong>' `
        -replace '\*(.*?)\*', '<em>$1</em>' `
        -replace '``(.*?)``', '<code>$1</code>' `
        -replace '---', '<hr>' `
        -replace '^- (.*)', '<li>$1</li>' `
        -replace '\n\n', '</p><p>'

    $html += @"
    </div>
</body>
</html>
"@

    $html | Out-File -FilePath $htmlFile -Encoding UTF8

    return $htmlFile
}

# 主执行流程
Write-Host "=== 每日报告生成系统 ===" -ForegroundColor Cyan
Write-Host "日期: $(Get-Date -Format 'yyyy-MM-dd')" -ForegroundColor Gray
Write-Host ""

# 获取数据
Write-Host "[1/5] 获取今日数据..." -ForegroundColor Yellow
$todayData = if ($DataFile) {
    Get-Content $DataFile -Raw -Encoding UTF8 | ConvertFrom-Json
} else {
    Get-TodayData -DataPath $DataPath
}

if (-not $todayData) {
    Write-Host "错误: 未找到今日数据" -ForegroundColor Red
    Write-Host "请先运行 TrackProgress.ps1 收集数据" -ForegroundColor Yellow
    exit 1
}

Write-Host "[2/5] 获取昨日数据..." -ForegroundColor Yellow
$yesterdayData = Get-YesterdayData -DataPath $DataPath

Write-Host "[3/5] 对比分析..." -ForegroundColor Yellow
$comparison = Compare-DailyProgress -TodayData $todayData -YesterdayData $yesterdayData

Write-Host "[4/5] 收集今日亮点..." -ForegroundColor Yellow
$highlights = Get-TodayHighlights -TodayData $todayData -ProjectRoot $ProjectRoot

Write-Host "[5/5] 生成报告..." -ForegroundColor Yellow
$markdownReport = Generate-MarkdownReport -TodayData $todayData -YesterdayData $yesterdayData -Comparison $comparison -Highlights $highlights -OutputPath $ReportPath

Write-Host ""
Write-Host "Markdown报告已生成: $markdownReport" -ForegroundColor Green

# 生成HTML版本
$htmlReport = Generate-HTMLReport -MarkdownFile $markdownReport -OutputPath $ReportPath
Write-Host "HTML报告已生成: $htmlReport" -ForegroundColor Green

# 显示摘要
Write-Host ""
Write-Host "=== 报告摘要 ===" -ForegroundColor Cyan
Write-Host "项目状态: $($todayData.ProjectHealth.Status)" -ForegroundColor $(
    switch ($todayData.ProjectHealth.Status) {
        "Healthy" { "Green" }
        "Fair" { "Yellow" }
        default { "Red" }
    }
)
Write-Host "健康得分: $($todayData.ProjectHealth.OverallScore)/100" -ForegroundColor White
Write-Host "今日提交: $($todayData.GitStatistics.CommitsToday) 次" -ForegroundColor White
Write-Host "任务完成: $($todayData.TaskProgress.CompletedTasks)/$($todayData.TaskProgress.TotalTasks) ($($todayData.TaskProgress.CompletionRate)%)" -ForegroundColor White

if ($comparison.Improvements.Count -gt 0) {
    Write-Host ""
    Write-Host "改进: $($comparison.Improvements.Count) 项" -ForegroundColor Green
}

if ($comparison.Concerns.Count -gt 0) {
    Write-Host "关注: $($comparison.Concerns.Count) 项" -ForegroundColor Yellow
}

Write-Host ""
Write-Host "=== 报告生成完成 ===" -ForegroundColor Cyan
