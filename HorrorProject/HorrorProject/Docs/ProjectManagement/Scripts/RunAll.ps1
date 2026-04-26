# RunAll.ps1 - 项目管理系统主控脚本
# 一键运行所有项目管理任务

param(
    [string]$ProjectRoot = "D:/gptzuo/HorrorProject",
    [string]$DataPath = "D:/gptzuo/HorrorProject/HorrorProject/Docs/ProjectManagement/Data",
    [string]$ReportPath = "D:/gptzuo/HorrorProject/HorrorProject/Docs/ProjectManagement/Reports",
    [switch]$FullReport,
    [switch]$SkipReports,
    [switch]$Verbose
)

$ErrorActionPreference = "Stop"
$ScriptDir = $PSScriptRoot

Write-Host ""
Write-Host "╔════════════════════════════════════════════════════════════╗" -ForegroundColor Cyan
Write-Host "║                                                            ║" -ForegroundColor Cyan
Write-Host "║        HorrorProject 项目管理系统 - 完整执行流程          ║" -ForegroundColor Cyan
Write-Host "║                                                            ║" -ForegroundColor Cyan
Write-Host "╚════════════════════════════════════════════════════════════╝" -ForegroundColor Cyan
Write-Host ""
Write-Host "执行时间: $(Get-Date -Format 'yyyy-MM-dd HH:mm:ss')" -ForegroundColor Gray
Write-Host "项目路径: $ProjectRoot" -ForegroundColor Gray
Write-Host ""

$startTime = Get-Date
$results = @{
    Success = @()
    Failed = @()
    Skipped = @()
}

# 辅助函数
function Run-Script {
    param(
        [string]$ScriptName,
        [string]$Description,
        [hashtable]$Parameters = @{},
        [switch]$Optional
    )

    Write-Host "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" -ForegroundColor DarkGray
    Write-Host "▶ $Description" -ForegroundColor Yellow
    Write-Host ""

    $scriptPath = Join-Path $ScriptDir $ScriptName

    if (-not (Test-Path $scriptPath)) {
        Write-Host "  ⚠ 脚本不存在: $ScriptName" -ForegroundColor Red
        $results.Failed += $Description
        return $false
    }

    try {
        $params = @{
            ProjectRoot = $ProjectRoot
            DataPath = $DataPath
        }

        foreach ($key in $Parameters.Keys) {
            $params[$key] = $Parameters[$key]
        }

        if ($Verbose) {
            $params['Verbose'] = $true
        }

        & $scriptPath @params

        Write-Host ""
        Write-Host "  ✓ 完成: $Description" -ForegroundColor Green
        $results.Success += $Description
        return $true
    }
    catch {
        Write-Host ""
        Write-Host "  ✗ 失败: $Description" -ForegroundColor Red
        Write-Host "  错误: $($_.Exception.Message)" -ForegroundColor Red

        if ($Optional) {
            Write-Host "  (可选任务，继续执行)" -ForegroundColor Yellow
            $results.Skipped += $Description
            return $false
        }
        else {
            $results.Failed += $Description
            throw
        }
    }
}

# 第一阶段：数据收集
Write-Host "╔════════════════════════════════════════════════════════════╗" -ForegroundColor Cyan
Write-Host "║  阶段 1/4: 数据收集                                       ║" -ForegroundColor Cyan
Write-Host "╚════════════════════════════════════════════════════════════╝" -ForegroundColor Cyan
Write-Host ""

Run-Script -ScriptName "TrackProgress.ps1" -Description "追踪项目进度"
Run-Script -ScriptName "CalculateVelocity.ps1" -Description "计算开发速度" -Parameters @{ ShowTrends = $true }
Run-Script -ScriptName "TrackResources.ps1" -Description "追踪资源使用"

# 第二阶段：分析预测
Write-Host ""
Write-Host "╔════════════════════════════════════════════════════════════╗" -ForegroundColor Cyan
Write-Host "║  阶段 2/4: 分析预测                                       ║" -ForegroundColor Cyan
Write-Host "╚════════════════════════════════════════════════════════════╝" -ForegroundColor Cyan
Write-Host ""

Run-Script -ScriptName "PredictCompletion.ps1" -Description "预测完成时间" -Parameters @{ ShowScenarios = $true }
Run-Script -ScriptName "UpdateMilestones.ps1" -Description "更新里程碑状态" -Parameters @{ Action = "report" }

# 第三阶段：风险管理
Write-Host ""
Write-Host "╔════════════════════════════════════════════════════════════╗" -ForegroundColor Cyan
Write-Host "║  阶段 3/4: 风险管理                                       ║" -ForegroundColor Cyan
Write-Host "╚════════════════════════════════════════════════════════════╝" -ForegroundColor Cyan
Write-Host ""

Run-Script -ScriptName "IdentifyRisks.ps1" -Description "识别项目风险" -Parameters @{ Verbose = $Verbose }
Run-Script -ScriptName "AssessRisks.ps1" -Description "评估风险等级" -Parameters @{ GenerateReport = $FullReport }
Run-Script -ScriptName "MitigateRisks.ps1" -Description "查看缓解计划" -Parameters @{ Action = "list" } -Optional

# 第四阶段：报告生成
if (-not $SkipReports) {
    Write-Host ""
    Write-Host "╔════════════════════════════════════════════════════════════╗" -ForegroundColor Cyan
    Write-Host "║  阶段 4/4: 报告生成                                       ║" -ForegroundColor Cyan
    Write-Host "╚════════════════════════════════════════════════════════════╝" -ForegroundColor Cyan
    Write-Host ""

    Run-Script -ScriptName "GenerateDailyReport.ps1" -Description "生成每日报告" -Parameters @{ ReportPath = $ReportPath }

    if ($FullReport) {
        Run-Script -ScriptName "GenerateWeeklyReport.ps1" -Description "生成周报" -Parameters @{ ReportPath = $ReportPath } -Optional
        Run-Script -ScriptName "GenerateSprintReport.ps1" -Description "生成Sprint报告" -Parameters @{ ReportPath = $ReportPath } -Optional
    }
}
else {
    Write-Host ""
    Write-Host "跳过报告生成阶段" -ForegroundColor Yellow
    $results.Skipped += "报告生成"
}

# 执行总结
$endTime = Get-Date
$duration = $endTime - $startTime

Write-Host ""
Write-Host "╔════════════════════════════════════════════════════════════╗" -ForegroundColor Cyan
Write-Host "║  执行总结                                                  ║" -ForegroundColor Cyan
Write-Host "╚════════════════════════════════════════════════════════════╝" -ForegroundColor Cyan
Write-Host ""

Write-Host "执行时间: $($duration.TotalSeconds.ToString('F2')) 秒" -ForegroundColor White
Write-Host ""

Write-Host "任务统计:" -ForegroundColor Yellow
Write-Host "  ✓ 成功: $($results.Success.Count) 个" -ForegroundColor Green
Write-Host "  ✗ 失败: $($results.Failed.Count) 个" -ForegroundColor Red
Write-Host "  ⊘ 跳过: $($results.Skipped.Count) 个" -ForegroundColor Gray
Write-Host ""

if ($results.Success.Count -gt 0) {
    Write-Host "成功任务:" -ForegroundColor Green
    foreach ($task in $results.Success) {
        Write-Host "  ✓ $task" -ForegroundColor DarkGreen
    }
    Write-Host ""
}

if ($results.Failed.Count -gt 0) {
    Write-Host "失败任务:" -ForegroundColor Red
    foreach ($task in $results.Failed) {
        Write-Host "  ✗ $task" -ForegroundColor DarkRed
    }
    Write-Host ""
}

if ($results.Skipped.Count -gt 0) {
    Write-Host "跳过任务:" -ForegroundColor Gray
    foreach ($task in $results.Skipped) {
        Write-Host "  ⊘ $task" -ForegroundColor DarkGray
    }
    Write-Host ""
}

# 输出位置
Write-Host "输出位置:" -ForegroundColor Cyan
Write-Host "  数据文件: $DataPath" -ForegroundColor Gray
if (-not $SkipReports) {
    Write-Host "  报告文件: $ReportPath" -ForegroundColor Gray
}
Write-Host ""

# 快速访问命令
Write-Host "快速访问命令:" -ForegroundColor Cyan
Write-Host "  查看数据: explorer '$DataPath'" -ForegroundColor Gray
if (-not $SkipReports) {
    Write-Host "  查看报告: explorer '$ReportPath'" -ForegroundColor Gray
}
Write-Host ""

# 下次运行建议
Write-Host "下次运行建议:" -ForegroundColor Cyan
Write-Host "  完整报告: .\RunAll.ps1 -FullReport" -ForegroundColor Gray
Write-Host "  仅数据收集: .\RunAll.ps1 -SkipReports" -ForegroundColor Gray
Write-Host "  详细输出: .\RunAll.ps1 -Verbose" -ForegroundColor Gray
Write-Host ""

# 最终状态
if ($results.Failed.Count -eq 0) {
    Write-Host "╔════════════════════════════════════════════════════════════╗" -ForegroundColor Green
    Write-Host "║                                                            ║" -ForegroundColor Green
    Write-Host "║                  ✓ 所有任务执行成功！                     ║" -ForegroundColor Green
    Write-Host "║                                                            ║" -ForegroundColor Green
    Write-Host "╚════════════════════════════════════════════════════════════╝" -ForegroundColor Green
    exit 0
}
else {
    Write-Host "╔════════════════════════════════════════════════════════════╗" -ForegroundColor Red
    Write-Host "║                                                            ║" -ForegroundColor Red
    Write-Host "║              ✗ 部分任务执行失败，请检查日志               ║" -ForegroundColor Red
    Write-Host "║                                                            ║" -ForegroundColor Red
    Write-Host "╚════════════════════════════════════════════════════════════╝" -ForegroundColor Red
    exit 1
}
