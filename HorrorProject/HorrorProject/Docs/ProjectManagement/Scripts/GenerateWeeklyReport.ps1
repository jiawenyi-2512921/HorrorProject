# GenerateWeeklyReport.ps1 - 生成周报
# 自动生成项目周报，汇总一周的进展和趋势

param(
    [string]$DataPath = "D:/gptzuo/HorrorProject/HorrorProject/Docs/ProjectManagement/Data",
    [string]$ReportPath = "D:/gptzuo/HorrorProject/HorrorProject/Docs/ProjectManagement/Reports",
    [string]$ProjectRoot = "D:/gptzuo/HorrorProject",
    [DateTime]$WeekStart = (Get-Date).AddDays(-7),
    [DateTime]$WeekEnd = (Get-Date)
)

$ErrorActionPreference = "Stop"

function Get-WeeklyData {
    param([string]$DataPath, [DateTime]$StartDate, [DateTime]$EndDate)

    $data = @{
        ProgressSnapshots = @()
        VelocityRecords = @()
        StartDate = $StartDate
        EndDate = $EndDate
    }

    # 加载进度数据
    $progressFile = Join-Path $DataPath "progress_summary.csv"
    if (Test-Path $progressFile) {
        $allProgress = Import-Csv $progressFile -Encoding UTF8
        $data.ProgressSnapshots = $allProgress | Where-Object {
            $timestamp = [DateTime]::Parse($_.Timestamp)
            $timestamp -ge $StartDate -and $timestamp -le $EndDate
        }
    }

    # 加载速度数据
    $velocityFile = Join-Path $DataPath "velocity_history.csv"
    if (Test-Path $velocityFile) {
        $allVelocity = Import-Csv $velocityFile -Encoding UTF8
        $data.VelocityRecords = $allVelocity | Where-Object {
            $timestamp = [DateTime]::Parse($_.Timestamp)
            $timestamp -ge $StartDate -and $timestamp -le $EndDate
        }
    }

    return $data
}

function Calculate-WeeklySummary {
    param($WeeklyData, [string]$ProjectRoot)

    $summary = @{
        TotalCommits = 0
        TotalLinesAdded = 0
        TotalLinesDeleted = 0
        TasksCompleted = 0
        ProgressGain = 0
        AverageVelocity = 0
        TopContributors = @{}
        DailyActivity = @{}
    }

    Push-Location $ProjectRoot
    try {
        # Git统计
        $since = $WeeklyData.StartDate.ToString("yyyy-MM-dd")
        $until = $WeeklyData.EndDate.ToString("yyyy-MM-dd")

        $commits = git log --since=$since --until=$until --format="%ai|%an" 2>$null
        if ($commits) {
            $commitList = $commits -split "`n" | Where-Object { $_ }
            $summary.TotalCommits = $commitList.Count

            # 按作者统计
            foreach ($commit in $commitList) {
                $author = ($commit -split '\|')[1]
                if (-not $summary.TopContributors.ContainsKey($author)) {
                    $summary.TopContributors[$author] = 0
                }
                $summary.TopContributors[$author]++

                # 按日期统计
                $date = ([DateTime]::Parse(($commit -split '\|')[0])).ToString("yyyy-MM-dd")
                if (-not $summary.DailyActivity.ContainsKey($date)) {
                    $summary.DailyActivity[$date] = 0
                }
                $summary.DailyActivity[$date]++
            }
        }

        # 代码变化统计
        $stats = git log --since=$since --until=$until --numstat --format="" 2>$null
        if ($stats) {
            $lines = $stats -split "`n" | Where-Object { $_ -match '^\d+\s+\d+' }
            foreach ($line in $lines) {
                if ($line -match '^(\d+)\s+(\d+)') {
                    $summary.TotalLinesAdded += [int]$matches[1]
                    $summary.TotalLinesDeleted += [int]$matches[2]
                }
            }
        }
    }
    finally {
        Pop-Location
    }

    # 进度变化
    if ($WeeklyData.ProgressSnapshots.Count -gt 1) {
        $first = $WeeklyData.ProgressSnapshots | Select-Object -First 1
        $last = $WeeklyData.ProgressSnapshots | Select-Object -Last 1

        $summary.ProgressGain = [double]$last.CompletionRate - [double]$first.CompletionRate
        $summary.TasksCompleted = [int]$last.TasksCompleted - [int]$first.TasksCompleted
    }

    # 平均速度
    if ($WeeklyData.VelocityRecords.Count -gt 0) {
        $summary.AverageVelocity = ($WeeklyData.VelocityRecords | Measure-Object -Property VelocityScore -Average).Average
    }

    return $summary
}

function Get-WeeklyHighlights {
    param([string]$ProjectRoot, [DateTime]$StartDate, [DateTime]$EndDate)

    $highlights = @{
        MajorFeatures = @()
        BugFixes = @()
        Improvements = @()
        Documentation = @()
        TopCommits = @()
    }

    Push-Location $ProjectRoot
    try {
        $since = $StartDate.ToString("yyyy-MM-dd")
        $until = $EndDate.ToString("yyyy-MM-dd")

        $commits = git log --since=$since --until=$until --format="%h|%s|%an|%ai" 2>$null

        if ($commits) {
            $commitList = $commits -split "`n" | Where-Object { $_ }

            foreach ($commit in $commitList) {
                $parts = $commit -split '\|'
                $hash = $parts[0]
                $message = $parts[1]
                $author = $parts[2]
                $date = $parts[3]

                $commitInfo = @{
                    Hash = $hash
                    Message = $message
                    Author = $author
                    Date = $date
                }

                # 分类
                if ($message -match '(feat|feature|add|new|implement)') {
                    $highlights.MajorFeatures += $commitInfo
                }
                elseif ($message -match '(fix|bug|issue|resolve)') {
                    $highlights.BugFixes += $commitInfo
                }
                elseif ($message -match '(improve|enhance|optimize|refactor)') {
                    $highlights.Improvements += $commitInfo
                }
                elseif ($message -match '(doc|docs|documentation|readme)') {
                    $highlights.Documentation += $commitInfo
                }

                $highlights.TopCommits += $commitInfo
            }
        }
    }
    finally {
        Pop-Location
    }

    return $highlights
}

function Analyze-WeeklyTrends {
    param($WeeklyData)

    $trends = @{
        VelocityTrend = "stable"
        ProductivityTrend = "stable"
        QualityTrend = "stable"
        Observations = @()
    }

    if ($WeeklyData.VelocityRecords.Count -gt 3) {
        $records = $WeeklyData.VelocityRecords | Sort-Object Timestamp

        $firstHalf = $records | Select-Object -First ([math]::Floor($records.Count / 2))
        $secondHalf = $records | Select-Object -Last ([math]::Floor($records.Count / 2))

        $firstAvg = ($firstHalf | Measure-Object -Property VelocityScore -Average).Average
        $secondAvg = ($secondHalf | Measure-Object -Property VelocityScore -Average).Average

        if ($secondAvg -gt $firstAvg * 1.15) {
            $trends.VelocityTrend = "increasing"
            $trends.Observations += "开发速度显著提升"
        }
        elseif ($secondAvg -lt $firstAvg * 0.85) {
            $trends.VelocityTrend = "decreasing"
            $trends.Observations += "开发速度有所下降"
        }
        else {
            $trends.Observations += "开发速度保持稳定"
        }
    }

    if ($WeeklyData.ProgressSnapshots.Count -gt 3) {
        $snapshots = $WeeklyData.ProgressSnapshots | Sort-Object Timestamp

        $healthScores = $snapshots | ForEach-Object { [int]$_.HealthScore }
        $avgHealth = ($healthScores | Measure-Object -Average).Average

        if ($avgHealth -ge 75) {
            $trends.QualityTrend = "excellent"
            $trends.Observations += "项目质量保持优秀"
        }
        elseif ($avgHealth -ge 60) {
            $trends.QualityTrend = "good"
            $trends.Observations += "项目质量良好"
        }
        else {
            $trends.QualityTrend = "needs-attention"
            $trends.Observations += "项目质量需要关注"
        }
    }

    return $trends
}

function Generate-WeeklyMarkdownReport {
    param($Summary, $Highlights, $Trends, $WeeklyData, [string]$OutputPath)

    $weekStart = $WeeklyData.StartDate.ToString("yyyy-MM-dd")
    $weekEnd = $WeeklyData.EndDate.ToString("yyyy-MM-dd")
    $weekNumber = (Get-Date).DayOfYear / 7

    $markdown = @"
# HorrorProject 周报

**报告周期**: $weekStart 至 $weekEnd
**周次**: 第 $([math]::Ceiling($weekNumber)) 周
**生成时间**: $(Get-Date -Format 'yyyy-MM-dd HH:mm:ss')

---

## 执行摘要

本周项目进展顺利，团队完成了 **$($Summary.TasksCompleted)** 个任务，提交了 **$($Summary.TotalCommits)** 次代码，项目完成度提升了 **$([math]::Round($Summary.ProgressGain, 2))%**。

### 关键成果
- 代码提交: $($Summary.TotalCommits) 次
- 代码增量: +$($Summary.TotalLinesAdded) / -$($Summary.TotalLinesDeleted) 行
- 任务完成: $($Summary.TasksCompleted) 个
- 进度提升: $([math]::Round($Summary.ProgressGain, 2))%
- 平均速度: $([math]::Round($Summary.AverageVelocity, 1))/100

---

## 本周亮点

"@

    if ($Highlights.MajorFeatures.Count -gt 0) {
        $markdown += "### 主要功能开发`n`n"
        foreach ($feature in $Highlights.MajorFeatures | Select-Object -First 10) {
            $markdown += "- ``$($feature.Hash)`` $($feature.Message) - *$($feature.Author)*`n"
        }
        $markdown += "`n"
    }

    if ($Highlights.BugFixes.Count -gt 0) {
        $markdown += "### Bug修复`n`n"
        foreach ($fix in $Highlights.BugFixes | Select-Object -First 10) {
            $markdown += "- ``$($fix.Hash)`` $($fix.Message) - *$($fix.Author)*`n"
        }
        $markdown += "`n"
    }

    if ($Highlights.Improvements.Count -gt 0) {
        $markdown += "### 改进优化`n`n"
        foreach ($imp in $Highlights.Improvements | Select-Object -First 10) {
            $markdown += "- ``$($imp.Hash)`` $($imp.Message) - *$($imp.Author)*`n"
        }
        $markdown += "`n"
    }

    $markdown += @"

---

## 团队贡献

### 贡献者排行
"@

    $topContributors = $Summary.TopContributors.GetEnumerator() | Sort-Object Value -Descending | Select-Object -First 10

    $markdown += "`n`n| 贡献者 | 提交次数 | 占比 |`n"
    $markdown += "|--------|----------|------|`n"

    foreach ($contributor in $topContributors) {
        $percentage = [math]::Round(($contributor.Value / $Summary.TotalCommits) * 100, 1)
        $markdown += "| $($contributor.Key) | $($contributor.Value) | $percentage% |`n"
    }

    $markdown += @"


### 每日活动分布
"@

    if ($Summary.DailyActivity.Count -gt 0) {
        $markdown += "`n`n| 日期 | 提交次数 |`n"
        $markdown += "|------|----------|`n"

        $sortedDays = $Summary.DailyActivity.GetEnumerator() | Sort-Object Key

        foreach ($day in $sortedDays) {
            $markdown += "| $($day.Key) | $($day.Value) |`n"
        }
    }

    $markdown += @"


---

## 趋势分析

### 速度趋势
- **状态**: $($Trends.VelocityTrend)
- **生产力**: $($Trends.ProductivityTrend)
- **质量**: $($Trends.QualityTrend)

### 观察结果
"@

    foreach ($obs in $Trends.Observations) {
        $markdown += "`n- $obs"
    }

    $markdown += @"


---

## 统计数据

### 代码统计
- **新增代码**: $($Summary.TotalLinesAdded) 行
- **删除代码**: $($Summary.TotalLinesDeleted) 行
- **净增代码**: $($Summary.TotalLinesAdded - $Summary.TotalLinesDeleted) 行
- **提交次数**: $($Summary.TotalCommits) 次
- **日均提交**: $([math]::Round($Summary.TotalCommits / 7, 1)) 次

### 任务统计
- **完成任务**: $($Summary.TasksCompleted) 个
- **进度提升**: $([math]::Round($Summary.ProgressGain, 2))%
- **日均任务**: $([math]::Round($Summary.TasksCompleted / 7, 1)) 个

---

## 下周计划

基于本周进展，下周重点关注：

1. 继续保持开发节奏
2. 关注代码质量和测试覆盖
3. 推进关键里程碑任务
4. 及时解决技术债务

---

*本报告由自动化系统生成*
*数据统计周期: $weekStart 至 $weekEnd*
"@

    # 保存报告
    if (-not (Test-Path $OutputPath)) {
        New-Item -ItemType Directory -Path $OutputPath -Force | Out-Null
    }

    $reportFile = Join-Path $OutputPath "WeeklyReport_$weekStart.md"
    $markdown | Out-File -FilePath $reportFile -Encoding UTF8

    return $reportFile
}

# 主执行流程
Write-Host "=== 周报生成系统 ===" -ForegroundColor Cyan
Write-Host "统计周期: $($WeekStart.ToString('yyyy-MM-dd')) 至 $($WeekEnd.ToString('yyyy-MM-dd'))" -ForegroundColor Gray
Write-Host ""

Write-Host "[1/5] 收集周数据..." -ForegroundColor Yellow
$weeklyData = Get-WeeklyData -DataPath $DataPath -StartDate $WeekStart -EndDate $WeekEnd

if ($weeklyData.ProgressSnapshots.Count -eq 0 -and $weeklyData.VelocityRecords.Count -eq 0) {
    Write-Host "警告: 本周没有足够的数据" -ForegroundColor Yellow
}

Write-Host "[2/5] 计算周汇总..." -ForegroundColor Yellow
$summary = Calculate-WeeklySummary -WeeklyData $weeklyData -ProjectRoot $ProjectRoot

Write-Host "[3/5] 收集本周亮点..." -ForegroundColor Yellow
$highlights = Get-WeeklyHighlights -ProjectRoot $ProjectRoot -StartDate $WeekStart -EndDate $WeekEnd

Write-Host "[4/5] 分析趋势..." -ForegroundColor Yellow
$trends = Analyze-WeeklyTrends -WeeklyData $weeklyData

Write-Host "[5/5] 生成报告..." -ForegroundColor Yellow
$reportFile = Generate-WeeklyMarkdownReport -Summary $summary -Highlights $highlights -Trends $trends -WeeklyData $weeklyData -OutputPath $ReportPath

Write-Host ""
Write-Host "周报已生成: $reportFile" -ForegroundColor Green

Write-Host ""
Write-Host "=== 周报摘要 ===" -ForegroundColor Cyan
Write-Host "提交次数: $($summary.TotalCommits)" -ForegroundColor White
Write-Host "任务完成: $($summary.TasksCompleted) 个" -ForegroundColor White
Write-Host "进度提升: $([math]::Round($summary.ProgressGain, 2))%" -ForegroundColor White
Write-Host "代码净增: $($summary.TotalLinesAdded - $summary.TotalLinesDeleted) 行" -ForegroundColor White
Write-Host "速度趋势: $($trends.VelocityTrend)" -ForegroundColor White

Write-Host ""
Write-Host "=== 生成完成 ===" -ForegroundColor Cyan
