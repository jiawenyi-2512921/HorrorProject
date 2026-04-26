# TrackProgress.ps1 - 进度追踪主程序
# 自动化收集和分析项目进度数据

param(
    [string]$ProjectRoot = "D:/gptzuo/HorrorProject",
    [string]$OutputPath = "D:/gptzuo/HorrorProject/HorrorProject/Docs/ProjectManagement/Data",
    [switch]$GenerateReport,
    [switch]$Verbose
)

$ErrorActionPreference = "Stop"

# 配置
$Config = @{
    ProjectName = "HorrorProject"
    StartDate = Get-Date "2026-01-01"
    TargetDate = Get-Date "2026-12-31"
    WorkingDaysPerWeek = 5
    HoursPerDay = 8
}

# 数据收集函数
function Get-GitStatistics {
    param([string]$RepoPath)

    Push-Location $RepoPath
    try {
        $stats = @{
            TotalCommits = (git rev-list --all --count 2>$null) -as [int]
            CommitsThisWeek = (git rev-list --since="1 week ago" --all --count 2>$null) -as [int]
            CommitsToday = (git rev-list --since="1 day ago" --all --count 2>$null) -as [int]
            Contributors = @(git log --format='%an' | Sort-Object -Unique).Count
            Branches = @(git branch -a 2>$null).Count
            LastCommitDate = git log -1 --format='%ci' 2>$null
            FilesChanged = (git diff --stat HEAD~10 2>$null | Select-String "files? changed" | ForEach-Object {
                if ($_ -match '(\d+) files? changed') { [int]$matches[1] } else { 0 }
            })
        }
        return $stats
    }
    finally {
        Pop-Location
    }
}

function Get-CodeMetrics {
    param([string]$ProjectPath)

    $metrics = @{
        TotalFiles = 0
        TotalLines = 0
        CodeFiles = @{
            CPP = 0
            H = 0
            CS = 0
            Blueprint = 0
            Config = 0
        }
        FilesByType = @{}
    }

    if (Test-Path $ProjectPath) {
        # 统计各类文件
        $extensions = @('*.cpp', '*.h', '*.cs', '*.uasset', '*.ini', '*.json')
        foreach ($ext in $extensions) {
            $files = Get-ChildItem -Path $ProjectPath -Filter $ext -Recurse -File -ErrorAction SilentlyContinue
            $count = ($files | Measure-Object).Count
            $type = $ext.TrimStart('*.')
            $metrics.FilesByType[$type] = $count
            $metrics.TotalFiles += $count

            # 统计代码行数
            if ($ext -match '\.(cpp|h|cs)$') {
                foreach ($file in $files) {
                    try {
                        $lines = (Get-Content $file.FullName -ErrorAction SilentlyContinue | Measure-Object -Line).Lines
                        $metrics.TotalLines += $lines
                    }
                    catch { }
                }
            }
        }
    }

    return $metrics
}

function Get-TaskProgress {
    param([string]$DocsPath)

    $progress = @{
        TotalTasks = 0
        CompletedTasks = 0
        InProgressTasks = 0
        BlockedTasks = 0
        CompletionRate = 0
        Categories = @{}
    }

    # 扫描文档中的任务标记
    if (Test-Path $DocsPath) {
        $mdFiles = Get-ChildItem -Path $DocsPath -Filter "*.md" -Recurse -File

        foreach ($file in $mdFiles) {
            $content = Get-Content $file.FullName -Raw -ErrorAction SilentlyContinue
            if ($content) {
                # 统计任务标记
                $progress.TotalTasks += ([regex]::Matches($content, '\[[ x]\]')).Count
                $progress.CompletedTasks += ([regex]::Matches($content, '\[x\]')).Count
                $progress.InProgressTasks += ([regex]::Matches($content, '\[~\]')).Count
                $progress.BlockedTasks += ([regex]::Matches($content, '\[!\]')).Count
            }
        }
    }

    if ($progress.TotalTasks -gt 0) {
        $progress.CompletionRate = [math]::Round(($progress.CompletedTasks / $progress.TotalTasks) * 100, 2)
    }

    return $progress
}

function Get-SprintMetrics {
    $today = Get-Date
    $sprintStart = $today.AddDays(-7)

    return @{
        SprintNumber = [math]::Floor(($today - $Config.StartDate).TotalDays / 14) + 1
        SprintStartDate = $sprintStart
        SprintEndDate = $today
        DaysInSprint = 7
        DaysRemaining = 7 - ($today.DayOfWeek.value__ % 7)
        Velocity = 0  # 将由历史数据计算
        BurndownRate = 0
    }
}

function Calculate-ProjectHealth {
    param($GitStats, $CodeMetrics, $TaskProgress, $SprintMetrics)

    $health = @{
        OverallScore = 0
        Status = "Unknown"
        Indicators = @{
            CommitFrequency = "Unknown"
            CodeGrowth = "Unknown"
            TaskCompletion = "Unknown"
            SprintProgress = "Unknown"
        }
        Risks = @()
        Recommendations = @()
    }

    # 评估提交频率
    if ($GitStats.CommitsThisWeek -ge 20) {
        $health.Indicators.CommitFrequency = "Excellent"
        $health.OverallScore += 25
    }
    elseif ($GitStats.CommitsThisWeek -ge 10) {
        $health.Indicators.CommitFrequency = "Good"
        $health.OverallScore += 20
    }
    elseif ($GitStats.CommitsThisWeek -ge 5) {
        $health.Indicators.CommitFrequency = "Fair"
        $health.OverallScore += 15
        $health.Risks += "提交频率偏低，可能影响进度"
    }
    else {
        $health.Indicators.CommitFrequency = "Poor"
        $health.OverallScore += 5
        $health.Risks += "提交频率严重不足"
        $health.Recommendations += "增加开发活动频率"
    }

    # 评估代码增长
    if ($CodeMetrics.TotalLines -gt 10000) {
        $health.Indicators.CodeGrowth = "Excellent"
        $health.OverallScore += 25
    }
    elseif ($CodeMetrics.TotalLines -gt 5000) {
        $health.Indicators.CodeGrowth = "Good"
        $health.OverallScore += 20
    }
    else {
        $health.Indicators.CodeGrowth = "Fair"
        $health.OverallScore += 10
    }

    # 评估任务完成率
    if ($TaskProgress.CompletionRate -ge 80) {
        $health.Indicators.TaskCompletion = "Excellent"
        $health.OverallScore += 25
    }
    elseif ($TaskProgress.CompletionRate -ge 60) {
        $health.Indicators.TaskCompletion = "Good"
        $health.OverallScore += 20
    }
    elseif ($TaskProgress.CompletionRate -ge 40) {
        $health.Indicators.TaskCompletion = "Fair"
        $health.OverallScore += 15
    }
    else {
        $health.Indicators.TaskCompletion = "Poor"
        $health.OverallScore += 5
        $health.Risks += "任务完成率低于预期"
        $health.Recommendations += "重新评估任务优先级和资源分配"
    }

    # 评估Sprint进度
    $sprintProgress = (7 - $SprintMetrics.DaysRemaining) / 7 * 100
    if ($sprintProgress -ge 50) {
        $health.Indicators.SprintProgress = "On Track"
        $health.OverallScore += 25
    }
    else {
        $health.Indicators.SprintProgress = "Behind"
        $health.OverallScore += 10
        $health.Recommendations += "加速Sprint任务完成"
    }

    # 确定整体状态
    if ($health.OverallScore -ge 80) {
        $health.Status = "Healthy"
    }
    elseif ($health.OverallScore -ge 60) {
        $health.Status = "Fair"
    }
    elseif ($health.OverallScore -ge 40) {
        $health.Status = "At Risk"
    }
    else {
        $health.Status = "Critical"
    }

    return $health
}

function Export-ProgressData {
    param($Data, [string]$OutputPath)

    $timestamp = Get-Date -Format "yyyy-MM-dd_HHmmss"
    $jsonPath = Join-Path $OutputPath "progress_$timestamp.json"

    # 确保输出目录存在
    if (-not (Test-Path $OutputPath)) {
        New-Item -ItemType Directory -Path $OutputPath -Force | Out-Null
    }

    # 导出JSON
    $Data | ConvertTo-Json -Depth 10 | Out-File -FilePath $jsonPath -Encoding UTF8

    # 创建CSV摘要
    $csvPath = Join-Path $OutputPath "progress_summary.csv"
    $summary = [PSCustomObject]@{
        Timestamp = Get-Date -Format "yyyy-MM-dd HH:mm:ss"
        TotalCommits = $Data.GitStatistics.TotalCommits
        CommitsThisWeek = $Data.GitStatistics.CommitsThisWeek
        TotalFiles = $Data.CodeMetrics.TotalFiles
        TotalLines = $Data.CodeMetrics.TotalLines
        TasksCompleted = $Data.TaskProgress.CompletedTasks
        TasksTotal = $Data.TaskProgress.TotalTasks
        CompletionRate = $Data.TaskProgress.CompletionRate
        HealthScore = $Data.ProjectHealth.OverallScore
        HealthStatus = $Data.ProjectHealth.Status
    }

    # 追加到CSV
    if (Test-Path $csvPath) {
        $summary | Export-Csv -Path $csvPath -Append -NoTypeInformation -Encoding UTF8
    }
    else {
        $summary | Export-Csv -Path $csvPath -NoTypeInformation -Encoding UTF8
    }

    return @{
        JsonPath = $jsonPath
        CsvPath = $csvPath
    }
}

# 主执行流程
Write-Host "=== HorrorProject 进度追踪系统 ===" -ForegroundColor Cyan
Write-Host "开始时间: $(Get-Date -Format 'yyyy-MM-dd HH:mm:ss')" -ForegroundColor Gray
Write-Host ""

# 收集数据
Write-Host "[1/5] 收集Git统计数据..." -ForegroundColor Yellow
$gitStats = Get-GitStatistics -RepoPath $ProjectRoot
if ($Verbose) {
    $gitStats | Format-Table -AutoSize
}

Write-Host "[2/5] 分析代码指标..." -ForegroundColor Yellow
$codeMetrics = Get-CodeMetrics -ProjectPath $ProjectRoot
if ($Verbose) {
    $codeMetrics | Format-Table -AutoSize
}

Write-Host "[3/5] 统计任务进度..." -ForegroundColor Yellow
$docsPath = Join-Path $ProjectRoot "HorrorProject/Docs"
$taskProgress = Get-TaskProgress -DocsPath $docsPath
if ($Verbose) {
    $taskProgress | Format-Table -AutoSize
}

Write-Host "[4/5] 计算Sprint指标..." -ForegroundColor Yellow
$sprintMetrics = Get-SprintMetrics
if ($Verbose) {
    $sprintMetrics | Format-Table -AutoSize
}

Write-Host "[5/5] 评估项目健康度..." -ForegroundColor Yellow
$projectHealth = Calculate-ProjectHealth -GitStats $gitStats -CodeMetrics $codeMetrics -TaskProgress $taskProgress -SprintMetrics $sprintMetrics

# 汇总数据
$progressData = @{
    Timestamp = Get-Date -Format "yyyy-MM-dd HH:mm:ss"
    ProjectInfo = $Config
    GitStatistics = $gitStats
    CodeMetrics = $codeMetrics
    TaskProgress = $taskProgress
    SprintMetrics = $sprintMetrics
    ProjectHealth = $projectHealth
}

# 导出数据
Write-Host ""
Write-Host "导出进度数据..." -ForegroundColor Yellow
$exportResult = Export-ProgressData -Data $progressData -OutputPath $OutputPath

# 显示摘要
Write-Host ""
Write-Host "=== 进度摘要 ===" -ForegroundColor Cyan
Write-Host "Git提交: $($gitStats.TotalCommits) 总计 | $($gitStats.CommitsThisWeek) 本周 | $($gitStats.CommitsToday) 今日" -ForegroundColor White
Write-Host "代码规模: $($codeMetrics.TotalFiles) 文件 | $($codeMetrics.TotalLines) 行代码" -ForegroundColor White
Write-Host "任务进度: $($taskProgress.CompletedTasks)/$($taskProgress.TotalTasks) 完成 ($($taskProgress.CompletionRate)%)" -ForegroundColor White
Write-Host "Sprint: 第 $($sprintMetrics.SprintNumber) 个Sprint | 剩余 $($sprintMetrics.DaysRemaining) 天" -ForegroundColor White
Write-Host ""
Write-Host "项目健康度: $($projectHealth.Status) (得分: $($projectHealth.OverallScore)/100)" -ForegroundColor $(
    switch ($projectHealth.Status) {
        "Healthy" { "Green" }
        "Fair" { "Yellow" }
        "At Risk" { "DarkYellow" }
        "Critical" { "Red" }
        default { "White" }
    }
)

if ($projectHealth.Risks.Count -gt 0) {
    Write-Host ""
    Write-Host "风险提示:" -ForegroundColor Red
    foreach ($risk in $projectHealth.Risks) {
        Write-Host "  - $risk" -ForegroundColor Yellow
    }
}

if ($projectHealth.Recommendations.Count -gt 0) {
    Write-Host ""
    Write-Host "改进建议:" -ForegroundColor Cyan
    foreach ($rec in $projectHealth.Recommendations) {
        Write-Host "  - $rec" -ForegroundColor White
    }
}

Write-Host ""
Write-Host "数据已保存:" -ForegroundColor Green
Write-Host "  JSON: $($exportResult.JsonPath)" -ForegroundColor Gray
Write-Host "  CSV:  $($exportResult.CsvPath)" -ForegroundColor Gray

# 生成报告
if ($GenerateReport) {
    Write-Host ""
    Write-Host "生成每日报告..." -ForegroundColor Yellow
    $reportScript = Join-Path $PSScriptRoot "GenerateDailyReport.ps1"
    if (Test-Path $reportScript) {
        & $reportScript -DataFile $exportResult.JsonPath
    }
}

Write-Host ""
Write-Host "完成时间: $(Get-Date -Format 'yyyy-MM-dd HH:mm:ss')" -ForegroundColor Gray
Write-Host "=== 追踪完成 ===" -ForegroundColor Cyan
