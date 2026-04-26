# CalculateVelocity.ps1 - 计算开发速度
# 分析团队开发速度和生产力趋势

param(
    [string]$DataPath = "D:/gptzuo/HorrorProject/HorrorProject/Docs/ProjectManagement/Data",
    [string]$ProjectRoot = "D:/gptzuo/HorrorProject",
    [int]$SprintDays = 14,
    [int]$HistoryDays = 90,
    [switch]$ShowTrends,
    [switch]$ExportChart
)

$ErrorActionPreference = "Stop"

function Get-CommitVelocity {
    param([string]$RepoPath, [int]$Days)

    Push-Location $RepoPath
    try {
        $velocity = @{
            Period = $Days
            TotalCommits = 0
            CommitsPerDay = 0
            CommitsByDay = @{}
            CommitsByAuthor = @{}
            Trend = "stable"
        }

        # 获取指定天数内的提交
        $since = (Get-Date).AddDays(-$Days).ToString("yyyy-MM-dd")
        $commits = git log --since=$since --format="%ai|%an" 2>$null

        if ($commits) {
            $commitList = $commits -split "`n" | Where-Object { $_ }
            $velocity.TotalCommits = $commitList.Count
            $velocity.CommitsPerDay = [math]::Round($velocity.TotalCommits / $Days, 2)

            # 按日期统计
            foreach ($commit in $commitList) {
                $parts = $commit -split '\|'
                $date = ([DateTime]::Parse($parts[0])).ToString("yyyy-MM-dd")
                $author = $parts[1]

                if (-not $velocity.CommitsByDay.ContainsKey($date)) {
                    $velocity.CommitsByDay[$date] = 0
                }
                $velocity.CommitsByDay[$date]++

                if (-not $velocity.CommitsByAuthor.ContainsKey($author)) {
                    $velocity.CommitsByAuthor[$author] = 0
                }
                $velocity.CommitsByAuthor[$author]++
            }

            # 计算趋势
            $halfPoint = $Days / 2
            $recentCommits = $commitList | Where-Object {
                $date = ([DateTime]::Parse(($_ -split '\|')[0]))
                $date -gt (Get-Date).AddDays(-$halfPoint)
            }
            $recentRate = $recentCommits.Count / $halfPoint
            $overallRate = $velocity.CommitsPerDay

            if ($recentRate -gt $overallRate * 1.2) {
                $velocity.Trend = "increasing"
            }
            elseif ($recentRate -lt $overallRate * 0.8) {
                $velocity.Trend = "decreasing"
            }
        }

        return $velocity
    }
    finally {
        Pop-Location
    }
}

function Get-CodeVelocity {
    param([string]$RepoPath, [int]$Days)

    Push-Location $RepoPath
    try {
        $velocity = @{
            Period = $Days
            LinesAdded = 0
            LinesDeleted = 0
            NetLines = 0
            FilesChanged = 0
            LinesPerDay = 0
        }

        $since = (Get-Date).AddDays(-$Days).ToString("yyyy-MM-dd")
        $stats = git log --since=$since --numstat --format="" 2>$null

        if ($stats) {
            $lines = $stats -split "`n" | Where-Object { $_ -match '^\d+\s+\d+' }

            foreach ($line in $lines) {
                if ($line -match '^(\d+)\s+(\d+)') {
                    $velocity.LinesAdded += [int]$matches[1]
                    $velocity.LinesDeleted += [int]$matches[2]
                    $velocity.FilesChanged++
                }
            }

            $velocity.NetLines = $velocity.LinesAdded - $velocity.LinesDeleted
            $velocity.LinesPerDay = [math]::Round($velocity.NetLines / $Days, 2)
        }

        return $velocity
    }
    finally {
        Pop-Location
    }
}

function Get-TaskVelocity {
    param([string]$DocsPath, [int]$Days)

    $velocity = @{
        Period = $Days
        TasksCompleted = 0
        TasksPerDay = 0
        CompletionsByCategory = @{}
        AverageCompletionTime = 0
    }

    if (Test-Path $DocsPath) {
        # 扫描最近修改的文档
        $since = (Get-Date).AddDays(-$Days)
        $recentFiles = Get-ChildItem -Path $DocsPath -Filter "*.md" -Recurse -File |
            Where-Object { $_.LastWriteTime -gt $since }

        foreach ($file in $recentFiles) {
            $content = Get-Content $file.FullName -Raw -ErrorAction SilentlyContinue
            if ($content) {
                # 统计完成的任务标记
                $completed = ([regex]::Matches($content, '\[x\]')).Count
                $velocity.TasksCompleted += $completed

                # 按类别统计
                $category = $file.Directory.Name
                if (-not $velocity.CompletionsByCategory.ContainsKey($category)) {
                    $velocity.CompletionsByCategory[$category] = 0
                }
                $velocity.CompletionsByCategory[$category] += $completed
            }
        }

        $velocity.TasksPerDay = [math]::Round($velocity.TasksCompleted / $Days, 2)
    }

    return $velocity
}

function Calculate-SprintVelocity {
    param($CommitVelocity, $CodeVelocity, $TaskVelocity, [int]$SprintDays)

    $sprintVelocity = @{
        SprintLength = $SprintDays
        ProjectedCommits = [math]::Round($CommitVelocity.CommitsPerDay * $SprintDays, 0)
        ProjectedLines = [math]::Round($CodeVelocity.LinesPerDay * $SprintDays, 0)
        ProjectedTasks = [math]::Round($TaskVelocity.TasksPerDay * $SprintDays, 0)
        VelocityScore = 0
        Confidence = "medium"
    }

    # 计算速度得分 (0-100)
    $commitScore = [math]::Min($CommitVelocity.CommitsPerDay * 5, 40)
    $codeScore = [math]::Min($CodeVelocity.LinesPerDay / 10, 30)
    $taskScore = [math]::Min($TaskVelocity.TasksPerDay * 10, 30)

    $sprintVelocity.VelocityScore = [math]::Round($commitScore + $codeScore + $taskScore, 0)

    # 评估置信度
    if ($CommitVelocity.TotalCommits -gt 100 -and $TaskVelocity.TasksCompleted -gt 50) {
        $sprintVelocity.Confidence = "high"
    }
    elseif ($CommitVelocity.TotalCommits -lt 20) {
        $sprintVelocity.Confidence = "low"
    }

    return $sprintVelocity
}

function Show-VelocityTrends {
    param($CommitVelocity, $CodeVelocity, $TaskVelocity)

    Write-Host ""
    Write-Host "=== 速度趋势分析 ===" -ForegroundColor Cyan
    Write-Host ""

    # 提交趋势
    Write-Host "提交趋势: " -NoNewline
    switch ($CommitVelocity.Trend) {
        "increasing" { Write-Host "↑ 上升" -ForegroundColor Green }
        "decreasing" { Write-Host "↓ 下降" -ForegroundColor Red }
        "stable" { Write-Host "→ 稳定" -ForegroundColor Yellow }
    }

    # 最活跃的日期
    if ($CommitVelocity.CommitsByDay.Count -gt 0) {
        $topDays = $CommitVelocity.CommitsByDay.GetEnumerator() |
            Sort-Object Value -Descending |
            Select-Object -First 5

        Write-Host ""
        Write-Host "最活跃日期:" -ForegroundColor Yellow
        foreach ($day in $topDays) {
            Write-Host "  $($day.Key): $($day.Value) 次提交" -ForegroundColor White
        }
    }

    # 贡献者统计
    if ($CommitVelocity.CommitsByAuthor.Count -gt 0) {
        Write-Host ""
        Write-Host "贡献者统计:" -ForegroundColor Yellow
        $topAuthors = $CommitVelocity.CommitsByAuthor.GetEnumerator() |
            Sort-Object Value -Descending

        foreach ($author in $topAuthors) {
            $percentage = [math]::Round(($author.Value / $CommitVelocity.TotalCommits) * 100, 1)
            Write-Host "  $($author.Key): $($author.Value) 次提交 ($percentage%)" -ForegroundColor White
        }
    }

    # 代码变化趋势
    Write-Host ""
    Write-Host "代码变化:" -ForegroundColor Yellow
    Write-Host "  新增: +$($CodeVelocity.LinesAdded) 行" -ForegroundColor Green
    Write-Host "  删除: -$($CodeVelocity.LinesDeleted) 行" -ForegroundColor Red
    Write-Host "  净增: $($CodeVelocity.NetLines) 行" -ForegroundColor $(if ($CodeVelocity.NetLines -gt 0) { "Green" } else { "Red" })
    Write-Host "  文件: $($CodeVelocity.FilesChanged) 个" -ForegroundColor White

    # 任务完成趋势
    if ($TaskVelocity.CompletionsByCategory.Count -gt 0) {
        Write-Host ""
        Write-Host "任务完成分布:" -ForegroundColor Yellow
        foreach ($cat in $TaskVelocity.CompletionsByCategory.GetEnumerator()) {
            Write-Host "  $($cat.Key): $($cat.Value) 个任务" -ForegroundColor White
        }
    }
}

function Export-VelocityData {
    param($Data, [string]$OutputPath)

    $timestamp = Get-Date -Format "yyyy-MM-dd_HHmmss"
    $jsonPath = Join-Path $OutputPath "velocity_$timestamp.json"

    if (-not (Test-Path $OutputPath)) {
        New-Item -ItemType Directory -Path $OutputPath -Force | Out-Null
    }

    $Data | ConvertTo-Json -Depth 10 | Out-File -FilePath $jsonPath -Encoding UTF8

    # 追加到历史CSV
    $csvPath = Join-Path $OutputPath "velocity_history.csv"
    $record = [PSCustomObject]@{
        Timestamp = Get-Date -Format "yyyy-MM-dd HH:mm:ss"
        CommitsPerDay = $Data.CommitVelocity.CommitsPerDay
        LinesPerDay = $Data.CodeVelocity.LinesPerDay
        TasksPerDay = $Data.TaskVelocity.TasksPerDay
        VelocityScore = $Data.SprintVelocity.VelocityScore
        Trend = $Data.CommitVelocity.Trend
    }

    if (Test-Path $csvPath) {
        $record | Export-Csv -Path $csvPath -Append -NoTypeInformation -Encoding UTF8
    }
    else {
        $record | Export-Csv -Path $csvPath -NoTypeInformation -Encoding UTF8
    }

    return @{
        JsonPath = $jsonPath
        CsvPath = $csvPath
    }
}

# 主执行流程
Write-Host "=== 开发速度计算系统 ===" -ForegroundColor Cyan
Write-Host "分析周期: $HistoryDays 天" -ForegroundColor Gray
Write-Host ""

# 计算各项速度指标
Write-Host "[1/4] 计算提交速度..." -ForegroundColor Yellow
$commitVelocity = Get-CommitVelocity -RepoPath $ProjectRoot -Days $HistoryDays

Write-Host "[2/4] 计算代码速度..." -ForegroundColor Yellow
$codeVelocity = Get-CodeVelocity -RepoPath $ProjectRoot -Days $HistoryDays

Write-Host "[3/4] 计算任务速度..." -ForegroundColor Yellow
$docsPath = Join-Path $ProjectRoot "HorrorProject/Docs"
$taskVelocity = Get-TaskVelocity -DocsPath $docsPath -Days $HistoryDays

Write-Host "[4/4] 计算Sprint速度..." -ForegroundColor Yellow
$sprintVelocity = Calculate-SprintVelocity -CommitVelocity $commitVelocity -CodeVelocity $codeVelocity -TaskVelocity $taskVelocity -SprintDays $SprintDays

# 汇总数据
$velocityData = @{
    Timestamp = Get-Date -Format "yyyy-MM-dd HH:mm:ss"
    AnalysisPeriod = $HistoryDays
    CommitVelocity = $commitVelocity
    CodeVelocity = $codeVelocity
    TaskVelocity = $taskVelocity
    SprintVelocity = $sprintVelocity
}

# 显示结果
Write-Host ""
Write-Host "=== 速度指标 ===" -ForegroundColor Cyan
Write-Host ""
Write-Host "提交速度:" -ForegroundColor Yellow
Write-Host "  总提交数: $($commitVelocity.TotalCommits)" -ForegroundColor White
Write-Host "  每日平均: $($commitVelocity.CommitsPerDay) 次" -ForegroundColor White
Write-Host "  趋势: $($commitVelocity.Trend)" -ForegroundColor White
Write-Host ""
Write-Host "代码速度:" -ForegroundColor Yellow
Write-Host "  净增代码: $($codeVelocity.NetLines) 行" -ForegroundColor White
Write-Host "  每日平均: $($codeVelocity.LinesPerDay) 行" -ForegroundColor White
Write-Host "  文件变更: $($codeVelocity.FilesChanged) 个" -ForegroundColor White
Write-Host ""
Write-Host "任务速度:" -ForegroundColor Yellow
Write-Host "  完成任务: $($taskVelocity.TasksCompleted) 个" -ForegroundColor White
Write-Host "  每日平均: $($taskVelocity.TasksPerDay) 个" -ForegroundColor White
Write-Host ""
Write-Host "Sprint预测 ($SprintDays 天):" -ForegroundColor Yellow
Write-Host "  预计提交: $($sprintVelocity.ProjectedCommits) 次" -ForegroundColor White
Write-Host "  预计代码: $($sprintVelocity.ProjectedLines) 行" -ForegroundColor White
Write-Host "  预计任务: $($sprintVelocity.ProjectedTasks) 个" -ForegroundColor White
Write-Host "  速度得分: $($sprintVelocity.VelocityScore)/100" -ForegroundColor $(
    if ($sprintVelocity.VelocityScore -ge 70) { "Green" }
    elseif ($sprintVelocity.VelocityScore -ge 40) { "Yellow" }
    else { "Red" }
)
Write-Host "  置信度: $($sprintVelocity.Confidence)" -ForegroundColor White

# 显示趋势
if ($ShowTrends) {
    Show-VelocityTrends -CommitVelocity $commitVelocity -CodeVelocity $codeVelocity -TaskVelocity $taskVelocity
}

# 导出数据
Write-Host ""
Write-Host "导出速度数据..." -ForegroundColor Yellow
$exportResult = Export-VelocityData -Data $velocityData -OutputPath $DataPath

Write-Host ""
Write-Host "数据已保存:" -ForegroundColor Green
Write-Host "  JSON: $($exportResult.JsonPath)" -ForegroundColor Gray
Write-Host "  CSV:  $($exportResult.CsvPath)" -ForegroundColor Gray

Write-Host ""
Write-Host "=== 计算完成 ===" -ForegroundColor Cyan
