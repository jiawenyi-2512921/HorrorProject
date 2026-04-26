# PredictCompletion.ps1 - 预测项目完成时间
# 基于历史数据和当前进度预测项目完成日期

param(
    [string]$DataPath = "D:/gptzuo/HorrorProject/HorrorProject/Docs/ProjectManagement/Data",
    [string]$MilestoneFile = "D:/gptzuo/HorrorProject/HorrorProject/Docs/ProjectManagement/Data/milestones.json",
    [DateTime]$TargetDate = (Get-Date "2026-12-31"),
    [switch]$DetailedAnalysis,
    [switch]$ShowScenarios
)

$ErrorActionPreference = "Stop"

function Load-HistoricalData {
    param([string]$DataPath)

    $velocityFile = Join-Path $DataPath "velocity_history.csv"
    $progressFile = Join-Path $DataPath "progress_summary.csv"

    $data = @{
        VelocityHistory = @()
        ProgressHistory = @()
        HasData = $false
    }

    if (Test-Path $velocityFile) {
        $data.VelocityHistory = Import-Csv $velocityFile -Encoding UTF8
        $data.HasData = $true
    }

    if (Test-Path $progressFile) {
        $data.ProgressHistory = Import-Csv $progressFile -Encoding UTF8
        $data.HasData = $true
    }

    return $data
}

function Calculate-AverageVelocity {
    param($HistoricalData, [int]$RecentDays = 30)

    $velocity = @{
        CommitsPerDay = 0
        LinesPerDay = 0
        TasksPerDay = 0
        CompletionRatePerDay = 0
        Trend = "stable"
    }

    if ($HistoricalData.VelocityHistory.Count -gt 0) {
        $recent = $HistoricalData.VelocityHistory |
            Where-Object { [DateTime]::Parse($_.Timestamp) -gt (Get-Date).AddDays(-$RecentDays) }

        if ($recent.Count -gt 0) {
            $velocity.CommitsPerDay = ($recent | Measure-Object -Property CommitsPerDay -Average).Average
            $velocity.LinesPerDay = ($recent | Measure-Object -Property LinesPerDay -Average).Average
            $velocity.TasksPerDay = ($recent | Measure-Object -Property TasksPerDay -Average).Average

            # 计算趋势
            $halfPoint = [math]::Floor($recent.Count / 2)
            if ($halfPoint -gt 0) {
                $firstHalf = $recent | Select-Object -First $halfPoint
                $secondHalf = $recent | Select-Object -Last $halfPoint

                $firstAvg = ($firstHalf | Measure-Object -Property VelocityScore -Average).Average
                $secondAvg = ($secondHalf | Measure-Object -Property VelocityScore -Average).Average

                if ($secondAvg -gt $firstAvg * 1.1) {
                    $velocity.Trend = "increasing"
                }
                elseif ($secondAvg -lt $firstAvg * 0.9) {
                    $velocity.Trend = "decreasing"
                }
            }
        }
    }

    if ($HistoricalData.ProgressHistory.Count -gt 0) {
        $recent = $HistoricalData.ProgressHistory |
            Where-Object { [DateTime]::Parse($_.Timestamp) -gt (Get-Date).AddDays(-$RecentDays) }

        if ($recent.Count -gt 1) {
            $first = $recent | Select-Object -First 1
            $last = $recent | Select-Object -Last 1

            $rateChange = [double]$last.CompletionRate - [double]$first.CompletionRate
            $daysPassed = ([DateTime]::Parse($last.Timestamp) - [DateTime]::Parse($first.Timestamp)).Days

            if ($daysPassed -gt 0) {
                $velocity.CompletionRatePerDay = $rateChange / $daysPassed
            }
        }
    }

    return $velocity
}

function Get-CurrentProgress {
    param([string]$DataPath)

    $progressFile = Join-Path $DataPath "progress_summary.csv"

    if (Test-Path $progressFile) {
        $data = Import-Csv $progressFile -Encoding UTF8
        $latest = $data | Select-Object -Last 1

        return @{
            CompletionRate = [double]$latest.CompletionRate
            TasksCompleted = [int]$latest.TasksCompleted
            TasksTotal = [int]$latest.TasksTotal
            Timestamp = [DateTime]::Parse($latest.Timestamp)
        }
    }

    return @{
        CompletionRate = 0
        TasksCompleted = 0
        TasksTotal = 100
        Timestamp = Get-Date
    }
}

function Predict-CompletionDate {
    param($CurrentProgress, $AverageVelocity, [DateTime]$TargetDate)

    $prediction = @{
        Method = "Linear"
        PredictedDate = $null
        DaysRequired = 0
        OnSchedule = $false
        ScheduleVariance = 0
        Confidence = "medium"
        Scenarios = @{
            Optimistic = $null
            Realistic = $null
            Pessimistic = $null
        }
    }

    $remainingWork = 100 - $CurrentProgress.CompletionRate

    if ($AverageVelocity.CompletionRatePerDay -gt 0) {
        # 线性预测
        $prediction.DaysRequired = [math]::Ceiling($remainingWork / $AverageVelocity.CompletionRatePerDay)
        $prediction.PredictedDate = (Get-Date).AddDays($prediction.DaysRequired)

        # 计算进度偏差
        $daysToTarget = ($TargetDate - (Get-Date)).Days
        $prediction.ScheduleVariance = $daysToTarget - $prediction.DaysRequired
        $prediction.OnSchedule = $prediction.ScheduleVariance -ge 0

        # 评估置信度
        if ($AverageVelocity.Trend -eq "stable" -and $CurrentProgress.CompletionRate -gt 30) {
            $prediction.Confidence = "high"
        }
        elseif ($AverageVelocity.Trend -eq "decreasing" -or $CurrentProgress.CompletionRate -lt 20) {
            $prediction.Confidence = "low"
        }

        # 场景分析
        # 乐观场景: 速度提升20%
        $optimisticRate = $AverageVelocity.CompletionRatePerDay * 1.2
        $optimisticDays = [math]::Ceiling($remainingWork / $optimisticRate)
        $prediction.Scenarios.Optimistic = (Get-Date).AddDays($optimisticDays)

        # 现实场景: 当前速度
        $prediction.Scenarios.Realistic = $prediction.PredictedDate

        # 悲观场景: 速度下降20%
        $pessimisticRate = $AverageVelocity.CompletionRatePerDay * 0.8
        $pessimisticDays = [math]::Ceiling($remainingWork / $pessimisticRate)
        $prediction.Scenarios.Pessimistic = (Get-Date).AddDays($pessimisticDays)
    }
    else {
        # 基于任务数量的简单预测
        $remainingTasks = $CurrentProgress.TasksTotal - $CurrentProgress.TasksCompleted
        if ($AverageVelocity.TasksPerDay -gt 0) {
            $prediction.DaysRequired = [math]::Ceiling($remainingTasks / $AverageVelocity.TasksPerDay)
            $prediction.PredictedDate = (Get-Date).AddDays($prediction.DaysRequired)
            $prediction.Confidence = "low"
        }
    }

    return $prediction
}

function Analyze-MilestoneSchedule {
    param([string]$MilestoneFile, $AverageVelocity)

    $analysis = @{
        TotalMilestones = 0
        CompletedMilestones = 0
        OnTrackMilestones = 0
        AtRiskMilestones = 0
        DelayedMilestones = 0
        MilestoneDetails = @()
    }

    if (Test-Path $MilestoneFile) {
        $data = Get-Content $MilestoneFile -Raw -Encoding UTF8 | ConvertFrom-Json

        $analysis.TotalMilestones = $data.Milestones.Count
        $today = Get-Date

        foreach ($milestone in $data.Milestones) {
            $detail = @{
                Name = $milestone.Name
                Status = $milestone.Status
                Progress = $milestone.Progress
                TargetDate = [DateTime]::Parse($milestone.TargetDate)
                PredictedDate = $null
                Risk = "low"
            }

            if ($milestone.Status -eq "completed") {
                $analysis.CompletedMilestones++
                $detail.Risk = "none"
            }
            else {
                $remainingWork = 100 - $milestone.Progress
                $daysToTarget = ($detail.TargetDate - $today).Days

                # 预测完成日期
                if ($AverageVelocity.CompletionRatePerDay -gt 0) {
                    $daysNeeded = [math]::Ceiling($remainingWork / $AverageVelocity.CompletionRatePerDay)
                    $detail.PredictedDate = $today.AddDays($daysNeeded)

                    $variance = $daysToTarget - $daysNeeded

                    if ($variance -ge 7) {
                        $analysis.OnTrackMilestones++
                        $detail.Risk = "low"
                    }
                    elseif ($variance -ge 0) {
                        $analysis.AtRiskMilestones++
                        $detail.Risk = "medium"
                    }
                    else {
                        $analysis.DelayedMilestones++
                        $detail.Risk = "high"
                    }
                }
            }

            $analysis.MilestoneDetails += $detail
        }
    }

    return $analysis
}

function Show-PredictionReport {
    param($Prediction, $CurrentProgress, $MilestoneAnalysis, [DateTime]$TargetDate)

    Write-Host ""
    Write-Host "=== 项目完成预测报告 ===" -ForegroundColor Cyan
    Write-Host ""

    # 当前进度
    Write-Host "当前进度:" -ForegroundColor Yellow
    Write-Host "  完成率: $($CurrentProgress.CompletionRate)%" -ForegroundColor White
    Write-Host "  已完成任务: $($CurrentProgress.TasksCompleted)/$($CurrentProgress.TasksTotal)" -ForegroundColor White
    Write-Host "  最后更新: $($CurrentProgress.Timestamp.ToString('yyyy-MM-dd HH:mm'))" -ForegroundColor Gray
    Write-Host ""

    # 预测结果
    Write-Host "完成预测:" -ForegroundColor Yellow
    if ($Prediction.PredictedDate) {
        Write-Host "  预计完成日期: $($Prediction.PredictedDate.ToString('yyyy-MM-dd'))" -ForegroundColor White
        Write-Host "  所需天数: $($Prediction.DaysRequired) 天" -ForegroundColor White
        Write-Host "  目标日期: $($TargetDate.ToString('yyyy-MM-dd'))" -ForegroundColor White

        if ($Prediction.OnSchedule) {
            Write-Host "  进度状态: 按计划进行" -ForegroundColor Green
            Write-Host "  提前: $($Prediction.ScheduleVariance) 天" -ForegroundColor Green
        }
        else {
            Write-Host "  进度状态: 落后于计划" -ForegroundColor Red
            Write-Host "  延期: $([Math]::Abs($Prediction.ScheduleVariance)) 天" -ForegroundColor Red
        }

        Write-Host "  置信度: $($Prediction.Confidence)" -ForegroundColor $(
            switch ($Prediction.Confidence) {
                "high" { "Green" }
                "medium" { "Yellow" }
                "low" { "Red" }
                default { "White" }
            }
        )
    }
    else {
        Write-Host "  数据不足，无法预测" -ForegroundColor Red
    }

    # 里程碑分析
    if ($MilestoneAnalysis.TotalMilestones -gt 0) {
        Write-Host ""
        Write-Host "里程碑状态:" -ForegroundColor Yellow
        Write-Host "  总计: $($MilestoneAnalysis.TotalMilestones)" -ForegroundColor White
        Write-Host "  已完成: $($MilestoneAnalysis.CompletedMilestones)" -ForegroundColor Green
        Write-Host "  按计划: $($MilestoneAnalysis.OnTrackMilestones)" -ForegroundColor Cyan
        Write-Host "  有风险: $($MilestoneAnalysis.AtRiskMilestones)" -ForegroundColor Yellow
        Write-Host "  已延期: $($MilestoneAnalysis.DelayedMilestones)" -ForegroundColor Red
    }
}

function Show-ScenarioAnalysis {
    param($Prediction)

    Write-Host ""
    Write-Host "=== 场景分析 ===" -ForegroundColor Cyan
    Write-Host ""

    if ($Prediction.Scenarios.Optimistic) {
        Write-Host "乐观场景 (速度提升20%):" -ForegroundColor Green
        Write-Host "  完成日期: $($Prediction.Scenarios.Optimistic.ToString('yyyy-MM-dd'))" -ForegroundColor White
        Write-Host ""

        Write-Host "现实场景 (当前速度):" -ForegroundColor Yellow
        Write-Host "  完成日期: $($Prediction.Scenarios.Realistic.ToString('yyyy-MM-dd'))" -ForegroundColor White
        Write-Host ""

        Write-Host "悲观场景 (速度下降20%):" -ForegroundColor Red
        Write-Host "  完成日期: $($Prediction.Scenarios.Pessimistic.ToString('yyyy-MM-dd'))" -ForegroundColor White
        Write-Host ""

        $range = ($Prediction.Scenarios.Pessimistic - $Prediction.Scenarios.Optimistic).Days
        Write-Host "日期范围: $range 天" -ForegroundColor Gray
    }
}

function Show-DetailedAnalysis {
    param($MilestoneAnalysis, $AverageVelocity)

    Write-Host ""
    Write-Host "=== 详细分析 ===" -ForegroundColor Cyan
    Write-Host ""

    Write-Host "平均速度指标:" -ForegroundColor Yellow
    Write-Host "  提交速度: $([math]::Round($AverageVelocity.CommitsPerDay, 2)) 次/天" -ForegroundColor White
    Write-Host "  代码速度: $([math]::Round($AverageVelocity.LinesPerDay, 2)) 行/天" -ForegroundColor White
    Write-Host "  任务速度: $([math]::Round($AverageVelocity.TasksPerDay, 2)) 个/天" -ForegroundColor White
    Write-Host "  完成速度: $([math]::Round($AverageVelocity.CompletionRatePerDay, 2))%/天" -ForegroundColor White
    Write-Host "  趋势: $($AverageVelocity.Trend)" -ForegroundColor White

    if ($MilestoneAnalysis.MilestoneDetails.Count -gt 0) {
        Write-Host ""
        Write-Host "里程碑详情:" -ForegroundColor Yellow

        $atRisk = $MilestoneAnalysis.MilestoneDetails | Where-Object { $_.Risk -in @("medium", "high") -and $_.Status -ne "completed" }

        if ($atRisk.Count -gt 0) {
            foreach ($m in $atRisk) {
                $riskColor = if ($m.Risk -eq "high") { "Red" } else { "Yellow" }
                Write-Host ""
                Write-Host "  $($m.Name)" -ForegroundColor $riskColor
                Write-Host "    进度: $($m.Progress)%" -ForegroundColor Gray
                Write-Host "    目标: $($m.TargetDate.ToString('yyyy-MM-dd'))" -ForegroundColor Gray
                if ($m.PredictedDate) {
                    Write-Host "    预计: $($m.PredictedDate.ToString('yyyy-MM-dd'))" -ForegroundColor Gray
                    $delay = ($m.PredictedDate - $m.TargetDate).Days
                    if ($delay -gt 0) {
                        Write-Host "    延期: $delay 天" -ForegroundColor Red
                    }
                }
                Write-Host "    风险: $($m.Risk)" -ForegroundColor $riskColor
            }
        }
        else {
            Write-Host "  所有里程碑按计划进行" -ForegroundColor Green
        }
    }
}

# 主执行流程
Write-Host "=== 项目完成预测系统 ===" -ForegroundColor Cyan
Write-Host "目标日期: $($TargetDate.ToString('yyyy-MM-dd'))" -ForegroundColor Gray
Write-Host ""

# 加载历史数据
Write-Host "[1/5] 加载历史数据..." -ForegroundColor Yellow
$historicalData = Load-HistoricalData -DataPath $DataPath

if (-not $historicalData.HasData) {
    Write-Host "警告: 没有足够的历史数据进行预测" -ForegroundColor Red
    Write-Host "请先运行 TrackProgress.ps1 和 CalculateVelocity.ps1 收集数据" -ForegroundColor Yellow
    exit 1
}

# 计算平均速度
Write-Host "[2/5] 计算平均速度..." -ForegroundColor Yellow
$averageVelocity = Calculate-AverageVelocity -HistoricalData $historicalData

# 获取当前进度
Write-Host "[3/5] 获取当前进度..." -ForegroundColor Yellow
$currentProgress = Get-CurrentProgress -DataPath $DataPath

# 预测完成日期
Write-Host "[4/5] 预测完成日期..." -ForegroundColor Yellow
$prediction = Predict-CompletionDate -CurrentProgress $currentProgress -AverageVelocity $averageVelocity -TargetDate $TargetDate

# 分析里程碑
Write-Host "[5/5] 分析里程碑进度..." -ForegroundColor Yellow
$milestoneAnalysis = Analyze-MilestoneSchedule -MilestoneFile $MilestoneFile -AverageVelocity $averageVelocity

# 显示报告
Show-PredictionReport -Prediction $prediction -CurrentProgress $currentProgress -MilestoneAnalysis $milestoneAnalysis -TargetDate $TargetDate

if ($ShowScenarios) {
    Show-ScenarioAnalysis -Prediction $prediction
}

if ($DetailedAnalysis) {
    Show-DetailedAnalysis -MilestoneAnalysis $milestoneAnalysis -AverageVelocity $averageVelocity
}

# 保存预测结果
$predictionData = @{
    Timestamp = Get-Date -Format "yyyy-MM-dd HH:mm:ss"
    CurrentProgress = $currentProgress
    AverageVelocity = $averageVelocity
    Prediction = $prediction
    MilestoneAnalysis = $milestoneAnalysis
}

$outputFile = Join-Path $DataPath "prediction_$(Get-Date -Format 'yyyy-MM-dd').json"
$predictionData | ConvertTo-Json -Depth 10 | Out-File -FilePath $outputFile -Encoding UTF8

Write-Host ""
Write-Host "预测结果已保存: $outputFile" -ForegroundColor Green
Write-Host ""
Write-Host "=== 预测完成 ===" -ForegroundColor Cyan
