# TrackResources.ps1 - 资源追踪
# 追踪和监控项目资源使用情况

param(
    [string]$ProjectRoot = "D:/gptzuo/HorrorProject",
    [string]$DataPath = "D:/gptzuo/HorrorProject/HorrorProject/Docs/ProjectManagement/Data",
    [string]$OutputFile = "D:/gptzuo/HorrorProject/HorrorProject/Docs/ProjectManagement/Data/resources.json"
)

$ErrorActionPreference = "Stop"

function Get-DiskResources {
    param([string]$ProjectPath)

    $drive = (Get-Item $ProjectPath).PSDrive

    return @{
        DriveLetter = $drive.Name
        TotalSpace = [math]::Round($drive.Used / 1GB + $drive.Free / 1GB, 2)
        UsedSpace = [math]::Round($drive.Used / 1GB, 2)
        FreeSpace = [math]::Round($drive.Free / 1GB, 2)
        UsagePercent = [math]::Round(($drive.Used / ($drive.Used + $drive.Free)) * 100, 2)
        ProjectSize = 0
    }
}

function Get-ProjectSize {
    param([string]$ProjectPath)

    $size = 0
    try {
        $items = Get-ChildItem -Path $ProjectPath -Recurse -File -ErrorAction SilentlyContinue
        $size = ($items | Measure-Object -Property Length -Sum).Sum
    }
    catch { }

    return [math]::Round($size / 1GB, 2)
}

function Get-TeamResources {
    param([string]$ProjectRoot)

    $resources = @{
        TotalContributors = 0
        ActiveContributors = 0
        ContributorActivity = @{}
        WorkloadDistribution = @{}
    }

    Push-Location $ProjectRoot
    try {
        # 总贡献者
        $allContributors = git log --format='%an' 2>$null | Sort-Object -Unique
        $resources.TotalContributors = ($allContributors | Measure-Object).Count

        # 活跃贡献者（最近30天）
        $recentContributors = git log --since="30 days ago" --format='%an' 2>$null | Sort-Object -Unique
        $resources.ActiveContributors = ($recentContributors | Measure-Object).Count

        # 贡献者活动
        $commits = git log --since="30 days ago" --format='%an' 2>$null
        if ($commits) {
            $commitList = $commits -split "`n" | Where-Object { $_ }
            foreach ($author in $commitList) {
                if (-not $resources.ContributorActivity.ContainsKey($author)) {
                    $resources.ContributorActivity[$author] = 0
                }
                $resources.ContributorActivity[$author]++
            }
        }

        # 工作负载分布
        $totalCommits = $resources.ContributorActivity.Values | Measure-Object -Sum | Select-Object -ExpandProperty Sum
        foreach ($contributor in $resources.ContributorActivity.Keys) {
            $percentage = if ($totalCommits -gt 0) {
                [math]::Round(($resources.ContributorActivity[$contributor] / $totalCommits) * 100, 2)
            } else { 0 }
            $resources.WorkloadDistribution[$contributor] = $percentage
        }
    }
    finally {
        Pop-Location
    }

    return $resources
}

function Get-TimeResources {
    $projectStart = Get-Date "2026-01-01"
    $projectEnd = Get-Date "2026-12-31"
    $today = Get-Date

    $totalDays = ($projectEnd - $projectStart).Days
    $elapsedDays = ($today - $projectStart).Days
    $remainingDays = ($projectEnd - $today).Days

    return @{
        ProjectStartDate = $projectStart.ToString("yyyy-MM-dd")
        ProjectEndDate = $projectEnd.ToString("yyyy-MM-dd")
        CurrentDate = $today.ToString("yyyy-MM-dd")
        TotalDays = $totalDays
        ElapsedDays = $elapsedDays
        RemainingDays = $remainingDays
        ProgressPercent = [math]::Round(($elapsedDays / $totalDays) * 100, 2)
        WorkingDaysRemaining = [math]::Floor($remainingDays * 5 / 7)
    }
}

function Analyze-ResourceUtilization {
    param($DiskResources, $TeamResources, $TimeResources)

    $analysis = @{
        DiskUtilization = "Normal"
        TeamUtilization = "Normal"
        TimeUtilization = "Normal"
        Warnings = @()
        Recommendations = @()
    }

    # 磁盘使用分析
    if ($DiskResources.UsagePercent -gt 90) {
        $analysis.DiskUtilization = "Critical"
        $analysis.Warnings += "磁盘空间严重不足"
        $analysis.Recommendations += "清理不必要的文件或扩展存储空间"
    }
    elseif ($DiskResources.UsagePercent -gt 80) {
        $analysis.DiskUtilization = "Warning"
        $analysis.Warnings += "磁盘空间紧张"
        $analysis.Recommendations += "监控磁盘使用，准备扩展空间"
    }

    # 团队资源分析
    if ($TeamResources.ActiveContributors -lt 2) {
        $analysis.TeamUtilization = "Low"
        $analysis.Warnings += "活跃贡献者过少"
        $analysis.Recommendations += "增加团队成员或提高参与度"
    }

    # 检查工作负载不均
    $maxWorkload = ($TeamResources.WorkloadDistribution.Values | Measure-Object -Maximum).Maximum
    if ($maxWorkload -gt 60) {
        $analysis.TeamUtilization = "Unbalanced"
        $analysis.Warnings += "工作负载分布不均"
        $analysis.Recommendations += "重新分配任务，平衡团队负载"
    }

    # 时间资源分析
    $timeProgress = $TimeResources.ProgressPercent
    if ($timeProgress -gt 75 -and $timeProgress -lt 90) {
        $analysis.TimeUtilization = "Warning"
        $analysis.Warnings += "项目时间已使用超过75%"
        $analysis.Recommendations += "加速关键任务，确保按时完成"
    }
    elseif ($timeProgress -ge 90) {
        $analysis.TimeUtilization = "Critical"
        $analysis.Warnings += "项目时间所剩无几"
        $analysis.Recommendations += "聚焦核心功能，考虑调整范围"
    }

    return $analysis
}

# 主执行流程
Write-Host "=== 资源追踪系统 ===" -ForegroundColor Cyan
Write-Host "追踪时间: $(Get-Date -Format 'yyyy-MM-dd HH:mm:ss')" -ForegroundColor Gray
Write-Host ""

Write-Host "[1/5] 检查磁盘资源..." -ForegroundColor Yellow
$diskResources = Get-DiskResources -ProjectPath $ProjectRoot
$diskResources.ProjectSize = Get-ProjectSize -ProjectPath $ProjectRoot

Write-Host "[2/5] 分析团队资源..." -ForegroundColor Yellow
$teamResources = Get-TeamResources -ProjectRoot $ProjectRoot

Write-Host "[3/5] 计算时间资源..." -ForegroundColor Yellow
$timeResources = Get-TimeResources

Write-Host "[4/5] 分析资源利用率..." -ForegroundColor Yellow
$utilization = Analyze-ResourceUtilization -DiskResources $diskResources -TeamResources $teamResources -TimeResources $timeResources

Write-Host "[5/5] 保存资源数据..." -ForegroundColor Yellow

$resourceData = @{
    Timestamp = Get-Date -Format "yyyy-MM-dd HH:mm:ss"
    DiskResources = $diskResources
    TeamResources = $teamResources
    TimeResources = $timeResources
    Utilization = $utilization
}

$dir = Split-Path $OutputFile -Parent
if (-not (Test-Path $dir)) {
    New-Item -ItemType Directory -Path $dir -Force | Out-Null
}

$resourceData | ConvertTo-Json -Depth 10 | Out-File -FilePath $OutputFile -Encoding UTF8

# 显示摘要
Write-Host ""
Write-Host "=== 资源摘要 ===" -ForegroundColor Cyan
Write-Host ""

Write-Host "磁盘资源:" -ForegroundColor Yellow
Write-Host "  总空间: $($diskResources.TotalSpace) GB" -ForegroundColor White
Write-Host "  已使用: $($diskResources.UsedSpace) GB ($($diskResources.UsagePercent)%)" -ForegroundColor White
Write-Host "  剩余: $($diskResources.FreeSpace) GB" -ForegroundColor White
Write-Host "  项目大小: $($diskResources.ProjectSize) GB" -ForegroundColor White
Write-Host ""

Write-Host "团队资源:" -ForegroundColor Yellow
Write-Host "  总贡献者: $($teamResources.TotalContributors) 人" -ForegroundColor White
Write-Host "  活跃贡献者: $($teamResources.ActiveContributors) 人" -ForegroundColor White

if ($teamResources.WorkloadDistribution.Count -gt 0) {
    Write-Host "  工作负载分布:" -ForegroundColor Gray
    $sortedWorkload = $teamResources.WorkloadDistribution.GetEnumerator() | Sort-Object Value -Descending
    foreach ($item in $sortedWorkload) {
        Write-Host "    $($item.Key): $($item.Value)%" -ForegroundColor DarkGray
    }
}
Write-Host ""

Write-Host "时间资源:" -ForegroundColor Yellow
Write-Host "  项目周期: $($timeResources.ProjectStartDate) 至 $($timeResources.ProjectEndDate)" -ForegroundColor White
Write-Host "  已用时间: $($timeResources.ElapsedDays) 天 ($($timeResources.ProgressPercent)%)" -ForegroundColor White
Write-Host "  剩余时间: $($timeResources.RemainingDays) 天 ($($timeResources.WorkingDaysRemaining) 工作日)" -ForegroundColor White
Write-Host ""

Write-Host "资源利用率:" -ForegroundColor Yellow
Write-Host "  磁盘: $($utilization.DiskUtilization)" -ForegroundColor White
Write-Host "  团队: $($utilization.TeamUtilization)" -ForegroundColor White
Write-Host "  时间: $($utilization.TimeUtilization)" -ForegroundColor White

if ($utilization.Warnings.Count -gt 0) {
    Write-Host ""
    Write-Host "警告:" -ForegroundColor Red
    foreach ($warning in $utilization.Warnings) {
        Write-Host "  - $warning" -ForegroundColor Yellow
    }
}

if ($utilization.Recommendations.Count -gt 0) {
    Write-Host ""
    Write-Host "建议:" -ForegroundColor Cyan
    foreach ($rec in $utilization.Recommendations) {
        Write-Host "  - $rec" -ForegroundColor White
    }
}

Write-Host ""
Write-Host "资源数据已保存: $OutputFile" -ForegroundColor Green
Write-Host ""
Write-Host "=== 追踪完成 ===" -ForegroundColor Cyan
