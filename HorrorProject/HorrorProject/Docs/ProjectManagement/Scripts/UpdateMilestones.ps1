# UpdateMilestones.ps1 - 更新项目里程碑
# 管理和追踪项目关键里程碑

param(
    [string]$MilestoneFile = "D:/gptzuo/HorrorProject/HorrorProject/Docs/ProjectManagement/Data/milestones.json",
    [string]$Action = "list",  # list, add, update, complete, report
    [string]$MilestoneName,
    [string]$Description,
    [DateTime]$TargetDate,
    [DateTime]$ActualDate,
    [string]$Status,  # planned, in-progress, completed, delayed, cancelled
    [int]$Progress = 0
)

$ErrorActionPreference = "Stop"

# 里程碑数据结构
function Initialize-MilestoneData {
    return @{
        Version = "1.0"
        LastUpdated = Get-Date -Format "yyyy-MM-dd HH:mm:ss"
        Milestones = @(
            @{
                Id = "M001"
                Name = "项目启动"
                Description = "项目初始化和环境搭建"
                Category = "Foundation"
                TargetDate = "2026-01-15"
                ActualDate = "2026-01-10"
                Status = "completed"
                Progress = 100
                Dependencies = @()
                Deliverables = @("项目结构", "开发环境", "版本控制")
            },
            @{
                Id = "M002"
                Name = "核心系统设计"
                Description = "游戏核心系统架构设计"
                Category = "Design"
                TargetDate = "2026-02-28"
                ActualDate = $null
                Status = "completed"
                Progress = 100
                Dependencies = @("M001")
                Deliverables = @("架构文档", "技术规范", "API设计")
            },
            @{
                Id = "M003"
                Name = "基础功能实现"
                Description = "实现游戏基础功能模块"
                Category = "Development"
                TargetDate = "2026-04-30"
                ActualDate = $null
                Status = "in-progress"
                Progress = 65
                Dependencies = @("M002")
                Deliverables = @("角色系统", "交互系统", "UI框架")
            },
            @{
                Id = "M004"
                Name = "恐怖氛围系统"
                Description = "实现恐怖游戏特色系统"
                Category = "Development"
                TargetDate = "2026-06-30"
                ActualDate = $null
                Status = "planned"
                Progress = 20
                Dependencies = @("M003")
                Deliverables = @("音效系统", "光影系统", "AI行为")
            },
            @{
                Id = "M005"
                Name = "关卡制作"
                Description = "完成主要游戏关卡"
                Category = "Content"
                TargetDate = "2026-08-31"
                ActualDate = $null
                Status = "planned"
                Progress = 10
                Dependencies = @("M003", "M004")
                Deliverables = @("3个主要关卡", "场景资源", "剧情内容")
            },
            @{
                Id = "M006"
                Name = "Alpha测试"
                Description = "内部Alpha版本测试"
                Category = "Testing"
                TargetDate = "2026-09-30"
                ActualDate = $null
                Status = "planned"
                Progress = 0
                Dependencies = @("M005")
                Deliverables = @("测试报告", "Bug列表", "性能数据")
            },
            @{
                Id = "M007"
                Name = "优化与打磨"
                Description = "游戏优化和细节打磨"
                Category = "Polish"
                TargetDate = "2026-11-15"
                ActualDate = $null
                Status = "planned"
                Progress = 0
                Dependencies = @("M006")
                Deliverables = @("性能优化", "视觉优化", "体验优化")
            },
            @{
                Id = "M008"
                Name = "Beta测试"
                Description = "公开Beta测试"
                Category = "Testing"
                TargetDate = "2026-11-30"
                ActualDate = $null
                Status = "planned"
                Progress = 0
                Dependencies = @("M007")
                Deliverables = @("Beta版本", "玩家反馈", "最终调整")
            },
            @{
                Id = "M009"
                Name = "正式发布"
                Description = "游戏正式发布"
                Category = "Release"
                TargetDate = "2026-12-31"
                ActualDate = $null
                Status = "planned"
                Progress = 0
                Dependencies = @("M008")
                Deliverables = @("发布版本", "宣传材料", "用户文档")
            }
        )
    }
}

function Load-Milestones {
    param([string]$FilePath)

    if (Test-Path $FilePath) {
        $json = Get-Content $FilePath -Raw -Encoding UTF8
        return $json | ConvertFrom-Json
    }
    else {
        return Initialize-MilestoneData
    }
}

function Save-Milestones {
    param($Data, [string]$FilePath)

    $Data.LastUpdated = Get-Date -Format "yyyy-MM-dd HH:mm:ss"

    $dir = Split-Path $FilePath -Parent
    if (-not (Test-Path $dir)) {
        New-Item -ItemType Directory -Path $dir -Force | Out-Null
    }

    $Data | ConvertTo-Json -Depth 10 | Out-File -FilePath $FilePath -Encoding UTF8
}

function Show-MilestoneList {
    param($Milestones)

    Write-Host ""
    Write-Host "=== 项目里程碑列表 ===" -ForegroundColor Cyan
    Write-Host ""

    $categories = $Milestones.Milestones | Group-Object -Property Category

    foreach ($category in $categories) {
        Write-Host "[$($category.Name)]" -ForegroundColor Yellow

        foreach ($milestone in $category.Group) {
            $statusColor = switch ($milestone.Status) {
                "completed" { "Green" }
                "in-progress" { "Cyan" }
                "delayed" { "Red" }
                "planned" { "Gray" }
                default { "White" }
            }

            $statusIcon = switch ($milestone.Status) {
                "completed" { "[✓]" }
                "in-progress" { "[→]" }
                "delayed" { "[!]" }
                "planned" { "[ ]" }
                default { "[?]" }
            }

            Write-Host "  $statusIcon $($milestone.Id): $($milestone.Name)" -ForegroundColor $statusColor
            Write-Host "      进度: $($milestone.Progress)% | 目标: $($milestone.TargetDate)" -ForegroundColor Gray

            if ($milestone.ActualDate) {
                Write-Host "      完成: $($milestone.ActualDate)" -ForegroundColor Gray
            }
        }
        Write-Host ""
    }
}

function Add-Milestone {
    param($Data, $Name, $Description, $TargetDate, $Category = "Development")

    $newId = "M{0:D3}" -f ($Data.Milestones.Count + 1)

    $newMilestone = @{
        Id = $newId
        Name = $Name
        Description = $Description
        Category = $Category
        TargetDate = $TargetDate.ToString("yyyy-MM-dd")
        ActualDate = $null
        Status = "planned"
        Progress = 0
        Dependencies = @()
        Deliverables = @()
    }

    $Data.Milestones += $newMilestone

    Write-Host "已添加里程碑: $newId - $Name" -ForegroundColor Green
    return $Data
}

function Update-Milestone {
    param($Data, $MilestoneName, $Status, $Progress)

    $milestone = $Data.Milestones | Where-Object { $_.Name -eq $MilestoneName -or $_.Id -eq $MilestoneName }

    if ($milestone) {
        if ($Status) {
            $milestone.Status = $Status
        }
        if ($Progress -ge 0) {
            $milestone.Progress = $Progress
        }
        if ($Progress -eq 100 -and -not $milestone.ActualDate) {
            $milestone.ActualDate = Get-Date -Format "yyyy-MM-dd"
            $milestone.Status = "completed"
        }

        Write-Host "已更新里程碑: $($milestone.Id) - $($milestone.Name)" -ForegroundColor Green
        Write-Host "  状态: $($milestone.Status) | 进度: $($milestone.Progress)%" -ForegroundColor Gray
    }
    else {
        Write-Host "未找到里程碑: $MilestoneName" -ForegroundColor Red
    }

    return $Data
}

function Complete-Milestone {
    param($Data, $MilestoneName, $ActualDate)

    $milestone = $Data.Milestones | Where-Object { $_.Name -eq $MilestoneName -or $_.Id -eq $MilestoneName }

    if ($milestone) {
        $milestone.Status = "completed"
        $milestone.Progress = 100
        $milestone.ActualDate = if ($ActualDate) { $ActualDate.ToString("yyyy-MM-dd") } else { Get-Date -Format "yyyy-MM-dd" }

        Write-Host "已完成里程碑: $($milestone.Id) - $($milestone.Name)" -ForegroundColor Green
        Write-Host "  完成日期: $($milestone.ActualDate)" -ForegroundColor Gray

        # 检查延期
        $target = [DateTime]::Parse($milestone.TargetDate)
        $actual = [DateTime]::Parse($milestone.ActualDate)
        $delay = ($actual - $target).Days

        if ($delay -gt 0) {
            Write-Host "  延期: $delay 天" -ForegroundColor Yellow
        }
        elseif ($delay -lt 0) {
            Write-Host "  提前: $([Math]::Abs($delay)) 天" -ForegroundColor Green
        }
    }
    else {
        Write-Host "未找到里程碑: $MilestoneName" -ForegroundColor Red
    }

    return $Data
}

function Show-MilestoneReport {
    param($Milestones)

    Write-Host ""
    Write-Host "=== 里程碑报告 ===" -ForegroundColor Cyan
    Write-Host ""

    $total = $Milestones.Milestones.Count
    $completed = ($Milestones.Milestones | Where-Object { $_.Status -eq "completed" }).Count
    $inProgress = ($Milestones.Milestones | Where-Object { $_.Status -eq "in-progress" }).Count
    $planned = ($Milestones.Milestones | Where-Object { $_.Status -eq "planned" }).Count
    $delayed = ($Milestones.Milestones | Where-Object { $_.Status -eq "delayed" }).Count

    $completionRate = [math]::Round(($completed / $total) * 100, 2)
    $avgProgress = [math]::Round(($Milestones.Milestones | Measure-Object -Property Progress -Average).Average, 2)

    Write-Host "总体统计:" -ForegroundColor Yellow
    Write-Host "  总里程碑数: $total" -ForegroundColor White
    Write-Host "  已完成: $completed ($completionRate%)" -ForegroundColor Green
    Write-Host "  进行中: $inProgress" -ForegroundColor Cyan
    Write-Host "  计划中: $planned" -ForegroundColor Gray
    Write-Host "  延期: $delayed" -ForegroundColor Red
    Write-Host "  平均进度: $avgProgress%" -ForegroundColor White
    Write-Host ""

    # 关键路径分析
    Write-Host "关键路径:" -ForegroundColor Yellow
    $criticalPath = $Milestones.Milestones | Where-Object { $_.Status -ne "completed" } | Sort-Object TargetDate | Select-Object -First 3
    foreach ($m in $criticalPath) {
        Write-Host "  - $($m.Name) (目标: $($m.TargetDate), 进度: $($m.Progress)%)" -ForegroundColor White
    }
    Write-Host ""

    # 风险评估
    Write-Host "风险评估:" -ForegroundColor Yellow
    $today = Get-Date
    $atRisk = $Milestones.Milestones | Where-Object {
        $_.Status -ne "completed" -and
        ([DateTime]::Parse($_.TargetDate) - $today).Days -lt 30 -and
        $_.Progress -lt 50
    }

    if ($atRisk.Count -gt 0) {
        foreach ($m in $atRisk) {
            $daysLeft = ([DateTime]::Parse($m.TargetDate) - $today).Days
            Write-Host "  - $($m.Name): 剩余 $daysLeft 天, 进度仅 $($m.Progress)%" -ForegroundColor Red
        }
    }
    else {
        Write-Host "  无高风险里程碑" -ForegroundColor Green
    }
}

# 主执行流程
Write-Host "=== 里程碑管理系统 ===" -ForegroundColor Cyan

$data = Load-Milestones -FilePath $MilestoneFile

switch ($Action.ToLower()) {
    "list" {
        Show-MilestoneList -Milestones $data
    }
    "add" {
        if (-not $MilestoneName -or -not $TargetDate) {
            Write-Host "错误: 添加里程碑需要 -MilestoneName 和 -TargetDate 参数" -ForegroundColor Red
            exit 1
        }
        $data = Add-Milestone -Data $data -Name $MilestoneName -Description $Description -TargetDate $TargetDate
        Save-Milestones -Data $data -FilePath $MilestoneFile
    }
    "update" {
        if (-not $MilestoneName) {
            Write-Host "错误: 更新里程碑需要 -MilestoneName 参数" -ForegroundColor Red
            exit 1
        }
        $data = Update-Milestone -Data $data -MilestoneName $MilestoneName -Status $Status -Progress $Progress
        Save-Milestones -Data $data -FilePath $MilestoneFile
    }
    "complete" {
        if (-not $MilestoneName) {
            Write-Host "错误: 完成里程碑需要 -MilestoneName 参数" -ForegroundColor Red
            exit 1
        }
        $data = Complete-Milestone -Data $data -MilestoneName $MilestoneName -ActualDate $ActualDate
        Save-Milestones -Data $data -FilePath $MilestoneFile
    }
    "report" {
        Show-MilestoneReport -Milestones $data
    }
    default {
        Write-Host "未知操作: $Action" -ForegroundColor Red
        Write-Host "可用操作: list, add, update, complete, report" -ForegroundColor Yellow
    }
}

Write-Host ""
Write-Host "=== 完成 ===" -ForegroundColor Cyan
