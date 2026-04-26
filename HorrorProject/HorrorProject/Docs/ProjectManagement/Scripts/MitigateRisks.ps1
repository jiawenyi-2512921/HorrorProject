# MitigateRisks.ps1 - 缓解风险
# 为识别的风险制定和执行缓解措施

param(
    [string]$RiskFile = "D:/gptzuo/HorrorProject/HorrorProject/Docs/ProjectManagement/Data/risks.json",
    [string]$RiskId,
    [string]$MitigationPlan,
    [string]$Action = "list",  # list, plan, execute, close
    [switch]$AutoGenerate
)

$ErrorActionPreference = "Stop"

function Load-Risks {
    param([string]$FilePath)

    if (Test-Path $FilePath) {
        return Get-Content $FilePath -Raw -Encoding UTF8 | ConvertFrom-Json
    }

    Write-Host "错误: 风险文件不存在" -ForegroundColor Red
    exit 1
}

function Generate-MitigationPlan {
    param($Risk)

    $plan = @{
        RiskId = $Risk.Id
        RiskTitle = $Risk.Title
        Strategy = ""
        Actions = @()
        Owner = "Project Manager"
        Timeline = "2 weeks"
        Status = "Planned"
        CreatedDate = Get-Date -Format "yyyy-MM-dd"
    }

    # 根据风险类别生成缓解策略
    switch ($Risk.Category) {
        "Technical" {
            $plan.Strategy = "技术改进和代码优化"
            if ($Risk.Title -match "代码库规模") {
                $plan.Actions = @(
                    "进行代码审查，识别重复代码",
                    "重构大型模块，提高模块化",
                    "建立代码规范和最佳实践",
                    "定期进行技术债务清理"
                )
            }
            elseif ($Risk.Title -match "开发活动") {
                $plan.Actions = @(
                    "召开团队会议，了解阻碍因素",
                    "重新分配任务和资源",
                    "提供必要的技术支持",
                    "调整开发计划和优先级"
                )
            }
            elseif ($Risk.Title -match "分支") {
                $plan.Actions = @(
                    "清理已合并的分支",
                    "制定分支管理规范",
                    "定期合并长期分支",
                    "使用Git工具自动化管理"
                )
            }
        }
        "Schedule" {
            $plan.Strategy = "进度调整和资源优化"
            if ($Risk.Title -match "延期") {
                $plan.Actions = @(
                    "重新评估里程碑目标",
                    "调整任务优先级",
                    "增加资源投入",
                    "简化非关键功能",
                    "与利益相关者沟通调整预期"
                )
            }
            elseif ($Risk.Title -match "速度") {
                $plan.Actions = @(
                    "分析速度下降原因",
                    "移除开发障碍",
                    "优化开发流程",
                    "提供培训和支持",
                    "考虑引入自动化工具"
                )
            }
        }
        "Quality" {
            $plan.Strategy = "质量提升和流程改进"
            $plan.Actions = @(
                "增加代码审查频率",
                "提高测试覆盖率",
                "建立质量门禁",
                "定期进行质量审计",
                "培训团队质量意识"
            )
        }
        "Resource" {
            $plan.Strategy = "资源管理和优化"
            $plan.Actions = @(
                "评估资源需求",
                "优化资源分配",
                "寻找替代方案",
                "建立资源储备",
                "监控资源使用情况"
            )
        }
        default {
            $plan.Strategy = "风险监控和应对"
            $plan.Actions = @(
                "持续监控风险指标",
                "制定应急预案",
                "定期评估风险状态"
            )
        }
    }

    return $plan
}

function Add-MitigationPlan {
    param($RiskData, $RiskId, $Plan)

    $risk = $RiskData.Risks | Where-Object { $_.Id -eq $RiskId }

    if (-not $risk) {
        Write-Host "错误: 未找到风险 $RiskId" -ForegroundColor Red
        return $RiskData
    }

    if (-not $risk.MitigationPlan) {
        $risk | Add-Member -NotePropertyName "MitigationPlan" -NotePropertyValue $Plan
    }
    else {
        $risk.MitigationPlan = $Plan
    }

    Write-Host "已为风险 $RiskId 添加缓解计划" -ForegroundColor Green

    return $RiskData
}

function Execute-Mitigation {
    param($RiskData, $RiskId)

    $risk = $RiskData.Risks | Where-Object { $_.Id -eq $RiskId }

    if (-not $risk -or -not $risk.MitigationPlan) {
        Write-Host "错误: 风险或缓解计划不存在" -ForegroundColor Red
        return $RiskData
    }

    $risk.MitigationPlan.Status = "In Progress"
    $risk.MitigationPlan | Add-Member -NotePropertyName "StartDate" -NotePropertyValue (Get-Date -Format "yyyy-MM-dd") -Force

    Write-Host "已开始执行风险 $RiskId 的缓解措施" -ForegroundColor Green

    return $RiskData
}

function Close-Risk {
    param($RiskData, $RiskId)

    $risk = $RiskData.Risks | Where-Object { $_.Id -eq $RiskId }

    if (-not $risk) {
        Write-Host "错误: 未找到风险 $RiskId" -ForegroundColor Red
        return $RiskData
    }

    $risk.Status = "Closed"
    $risk | Add-Member -NotePropertyName "ClosedDate" -NotePropertyValue (Get-Date -Format "yyyy-MM-dd") -Force

    if ($risk.MitigationPlan) {
        $risk.MitigationPlan.Status = "Completed"
        $risk.MitigationPlan | Add-Member -NotePropertyName "CompletedDate" -NotePropertyValue (Get-Date -Format "yyyy-MM-dd") -Force
    }

    Write-Host "风险 $RiskId 已关闭" -ForegroundColor Green

    return $RiskData
}

function Show-MitigationList {
    param($RiskData)

    Write-Host ""
    Write-Host "=== 风险缓解计划列表 ===" -ForegroundColor Cyan
    Write-Host ""

    $risksWithPlans = $RiskData.Risks | Where-Object { $_.MitigationPlan }

    if ($risksWithPlans.Count -eq 0) {
        Write-Host "暂无缓解计划" -ForegroundColor Gray
        return
    }

    foreach ($risk in $risksWithPlans) {
        $statusColor = switch ($risk.MitigationPlan.Status) {
            "Completed" { "Green" }
            "In Progress" { "Cyan" }
            "Planned" { "Yellow" }
            default { "White" }
        }

        Write-Host "[$($risk.Id)] $($risk.Title)" -ForegroundColor Yellow
        Write-Host "  优先级: $($risk.Priority) | 状态: $($risk.MitigationPlan.Status)" -ForegroundColor $statusColor
        Write-Host "  策略: $($risk.MitigationPlan.Strategy)" -ForegroundColor White
        Write-Host "  负责人: $($risk.MitigationPlan.Owner) | 时间线: $($risk.MitigationPlan.Timeline)" -ForegroundColor Gray

        if ($risk.MitigationPlan.Actions.Count -gt 0) {
            Write-Host "  行动项:" -ForegroundColor Gray
            foreach ($action in $risk.MitigationPlan.Actions | Select-Object -First 3) {
                Write-Host "    - $action" -ForegroundColor DarkGray
            }
        }
        Write-Host ""
    }
}

function Show-MitigationDetails {
    param($Risk)

    Write-Host ""
    Write-Host "=== 风险缓解详情 ===" -ForegroundColor Cyan
    Write-Host ""
    Write-Host "风险ID: $($Risk.Id)" -ForegroundColor Yellow
    Write-Host "风险标题: $($Risk.Title)" -ForegroundColor White
    Write-Host "类别: $($Risk.Category) | 优先级: $($Risk.Priority)" -ForegroundColor Gray
    Write-Host ""

    if ($Risk.MitigationPlan) {
        $plan = $Risk.MitigationPlan

        Write-Host "缓解策略: $($plan.Strategy)" -ForegroundColor Cyan
        Write-Host "状态: $($plan.Status)" -ForegroundColor White
        Write-Host "负责人: $($plan.Owner)" -ForegroundColor White
        Write-Host "时间线: $($plan.Timeline)" -ForegroundColor White
        Write-Host "创建日期: $($plan.CreatedDate)" -ForegroundColor Gray

        if ($plan.StartDate) {
            Write-Host "开始日期: $($plan.StartDate)" -ForegroundColor Gray
        }

        if ($plan.CompletedDate) {
            Write-Host "完成日期: $($plan.CompletedDate)" -ForegroundColor Gray
        }

        Write-Host ""
        Write-Host "行动项:" -ForegroundColor Yellow
        $actionNum = 1
        foreach ($action in $plan.Actions) {
            Write-Host "  $actionNum. $action" -ForegroundColor White
            $actionNum++
        }
    }
    else {
        Write-Host "暂无缓解计划" -ForegroundColor Red
    }
}

# 主执行流程
Write-Host "=== 风险缓解系统 ===" -ForegroundColor Cyan
Write-Host ""

$riskData = Load-Risks -FilePath $RiskFile

switch ($Action.ToLower()) {
    "list" {
        Show-MitigationList -RiskData $riskData
    }
    "plan" {
        if (-not $RiskId) {
            Write-Host "错误: 需要指定 -RiskId 参数" -ForegroundColor Red
            exit 1
        }

        $risk = $riskData.Risks | Where-Object { $_.Id -eq $RiskId }
        if (-not $risk) {
            Write-Host "错误: 未找到风险 $RiskId" -ForegroundColor Red
            exit 1
        }

        if ($AutoGenerate) {
            $plan = Generate-MitigationPlan -Risk $risk
            $riskData = Add-MitigationPlan -RiskData $riskData -RiskId $RiskId -Plan $plan
            Show-MitigationDetails -Risk ($riskData.Risks | Where-Object { $_.Id -eq $RiskId })
        }
        else {
            Show-MitigationDetails -Risk $risk
        }

        $riskData | ConvertTo-Json -Depth 10 | Out-File -FilePath $RiskFile -Encoding UTF8
    }
    "execute" {
        if (-not $RiskId) {
            Write-Host "错误: 需要指定 -RiskId 参数" -ForegroundColor Red
            exit 1
        }

        $riskData = Execute-Mitigation -RiskData $riskData -RiskId $RiskId
        $riskData | ConvertTo-Json -Depth 10 | Out-File -FilePath $RiskFile -Encoding UTF8
    }
    "close" {
        if (-not $RiskId) {
            Write-Host "错误: 需要指定 -RiskId 参数" -ForegroundColor Red
            exit 1
        }

        $riskData = Close-Risk -RiskData $riskData -RiskId $RiskId
        $riskData | ConvertTo-Json -Depth 10 | Out-File -FilePath $RiskFile -Encoding UTF8
    }
    default {
        Write-Host "未知操作: $Action" -ForegroundColor Red
        Write-Host "可用操作: list, plan, execute, close" -ForegroundColor Yellow
    }
}

Write-Host ""
Write-Host "=== 完成 ===" -ForegroundColor Cyan
