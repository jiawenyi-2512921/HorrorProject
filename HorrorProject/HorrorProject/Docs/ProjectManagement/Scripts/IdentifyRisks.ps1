# IdentifyRisks.ps1 - 识别项目风险
# 自动扫描和识别项目中的潜在风险

param(
    [string]$ProjectRoot = "D:/gptzuo/HorrorProject",
    [string]$DataPath = "D:/gptzuo/HorrorProject/HorrorProject/Docs/ProjectManagement/Data",
    [string]$RiskFile = "D:/gptzuo/HorrorProject/HorrorProject/Docs/ProjectManagement/Data/risks.json",
    [switch]$AutoAssess,
    [switch]$Verbose
)

$ErrorActionPreference = "Stop"

function Initialize-RiskData {
    return @{
        Version = "1.0"
        LastUpdated = Get-Date -Format "yyyy-MM-dd HH:mm:ss"
        Risks = @()
        RiskCategories = @{
            Technical = @()
            Schedule = @()
            Resource = @()
            Quality = @()
            External = @()
        }
    }
}

function Scan-TechnicalRisks {
    param([string]$ProjectRoot, [string]$DataPath)

    $risks = @()

    # 检查代码复杂度
    $progressFile = Join-Path $DataPath "progress_summary.csv"
    if (Test-Path $progressFile) {
        $latest = Import-Csv $progressFile -Encoding UTF8 | Select-Object -Last 1

        if ([int]$latest.TotalLines -gt 50000) {
            $risks += @{
                Id = "TECH-001"
                Title = "代码库规模过大"
                Description = "代码行数超过50000行，可能导致维护困难"
                Category = "Technical"
                Severity = "Medium"
                Probability = "High"
                Impact = "Medium"
                Status = "Active"
                DetectedDate = Get-Date -Format "yyyy-MM-dd"
                Indicators = @("代码行数: $($latest.TotalLines)")
            }
        }
    }

    # 检查提交频率
    Push-Location $ProjectRoot
    try {
        $recentCommits = (git log --since="7 days ago" --format="%h" 2>$null | Measure-Object).Count

        if ($recentCommits -lt 10) {
            $risks += @{
                Id = "TECH-002"
                Title = "开发活动不足"
                Description = "近7天提交次数少于10次，开发进度可能受阻"
                Category = "Technical"
                Severity = "High"
                Probability = "High"
                Impact = "High"
                Status = "Active"
                DetectedDate = Get-Date -Format "yyyy-MM-dd"
                Indicators = @("7天提交数: $recentCommits")
            }
        }

        # 检查分支数量
        $branches = @(git branch -a 2>$null).Count
        if ($branches -gt 20) {
            $risks += @{
                Id = "TECH-003"
                Title = "分支管理混乱"
                Description = "分支数量过多($branches)，可能导致合并冲突"
                Category = "Technical"
                Severity = "Low"
                Probability = "Medium"
                Impact = "Low"
                Status = "Active"
                DetectedDate = Get-Date -Format "yyyy-MM-dd"
                Indicators = @("分支数: $branches")
            }
        }

        # 检查未合并的分支
        $unmergedBranches = @(git branch --no-merged 2>$null).Count
        if ($unmergedBranches -gt 5) {
            $risks += @{
                Id = "TECH-004"
                Title = "未合并分支过多"
                Description = "存在$unmergedBranches个未合并分支，可能存在集成风险"
                Category = "Technical"
                Severity = "Medium"
                Probability = "Medium"
                Impact = "Medium"
                Status = "Active"
                DetectedDate = Get-Date -Format "yyyy-MM-dd"
                Indicators = @("未合并分支: $unmergedBranches")
            }
        }
    }
    finally {
        Pop-Location
    }

    return $risks
}

function Scan-ScheduleRisks {
    param([string]$DataPath, [string]$MilestoneFile)

    $risks = @()

    # 检查里程碑延期风险
    if (Test-Path $MilestoneFile) {
        $milestones = (Get-Content $MilestoneFile -Raw -Encoding UTF8 | ConvertFrom-Json).Milestones
        $today = Get-Date

        foreach ($milestone in $milestones) {
            if ($milestone.Status -ne "completed") {
                $targetDate = [DateTime]::Parse($milestone.TargetDate)
                $daysLeft = ($targetDate - $today).Days

                if ($daysLeft -lt 0) {
                    $risks += @{
                        Id = "SCHED-001-$($milestone.Id)"
                        Title = "里程碑已延期: $($milestone.Name)"
                        Description = "里程碑已超过目标日期$([Math]::Abs($daysLeft))天"
                        Category = "Schedule"
                        Severity = "High"
                        Probability = "Certain"
                        Impact = "High"
                        Status = "Active"
                        DetectedDate = Get-Date -Format "yyyy-MM-dd"
                        Indicators = @("延期天数: $([Math]::Abs($daysLeft))", "进度: $($milestone.Progress)%")
                    }
                }
                elseif ($daysLeft -lt 14 -and $milestone.Progress -lt 70) {
                    $risks += @{
                        Id = "SCHED-002-$($milestone.Id)"
                        Title = "里程碑延期风险: $($milestone.Name)"
                        Description = "剩余$daysLeft天，但进度仅$($milestone.Progress)%"
                        Category = "Schedule"
                        Severity = "High"
                        Probability = "High"
                        Impact = "High"
                        Status = "Active"
                        DetectedDate = Get-Date -Format "yyyy-MM-dd"
                        Indicators = @("剩余天数: $daysLeft", "进度: $($milestone.Progress)%")
                    }
                }
            }
        }
    }

    # 检查速度下降
    $velocityFile = Join-Path $DataPath "velocity_history.csv"
    if (Test-Path $velocityFile) {
        $velocityData = Import-Csv $velocityFile -Encoding UTF8
        if ($velocityData.Count -gt 7) {
            $recent = $velocityData | Select-Object -Last 7
            $older = $velocityData | Select-Object -Last 14 | Select-Object -First 7

            $recentAvg = ($recent | Measure-Object -Property VelocityScore -Average).Average
            $olderAvg = ($older | Measure-Object -Property VelocityScore -Average).Average

            if ($recentAvg -lt $olderAvg * 0.7) {
                $risks += @{
                    Id = "SCHED-003"
                    Title = "开发速度显著下降"
                    Description = "近期速度比之前下降了$([math]::Round((1 - $recentAvg/$olderAvg) * 100, 1))%"
                    Category = "Schedule"
                    Severity = "High"
                    Probability = "High"
                    Impact = "High"
                    Status = "Active"
                    DetectedDate = Get-Date -Format "yyyy-MM-dd"
                    Indicators = @("当前速度: $([math]::Round($recentAvg, 1))", "之前速度: $([math]::Round($olderAvg, 1))")
                }
            }
        }
    }

    return $risks
}

function Scan-QualityRisks {
    param([string]$ProjectRoot, [string]$DataPath)

    $risks = @()

    # 检查健康度
    $progressFile = Join-Path $DataPath "progress_summary.csv"
    if (Test-Path $progressFile) {
        $recent = Import-Csv $progressFile -Encoding UTF8 | Select-Object -Last 5

        $avgHealth = ($recent | Measure-Object -Property HealthScore -Average).Average

        if ($avgHealth -lt 50) {
            $risks += @{
                Id = "QUAL-001"
                Title = "项目健康度低"
                Description = "近期平均健康度仅$([math]::Round($avgHealth, 1))分"
                Category = "Quality"
                Severity = "High"
                Probability = "High"
                Impact = "High"
                Status = "Active"
                DetectedDate = Get-Date -Format "yyyy-MM-dd"
                Indicators = @("平均健康度: $([math]::Round($avgHealth, 1))")
            }
        }
    }

    # 检查大文件
    Push-Location $ProjectRoot
    try {
        $largeFiles = Get-ChildItem -Path . -Recurse -File -ErrorAction SilentlyContinue |
            Where-Object { $_.Length -gt 1MB -and $_.Extension -match '\.(cpp|h|cs)$' }

        if ($largeFiles.Count -gt 0) {
            $risks += @{
                Id = "QUAL-002"
                Title = "存在大型代码文件"
                Description = "发现$($largeFiles.Count)个超过1MB的代码文件，可能需要重构"
                Category = "Quality"
                Severity = "Medium"
                Probability = "High"
                Impact = "Medium"
                Status = "Active"
                DetectedDate = Get-Date -Format "yyyy-MM-dd"
                Indicators = @("大文件数: $($largeFiles.Count)")
            }
        }
    }
    finally {
        Pop-Location
    }

    return $risks
}

function Scan-ResourceRisks {
    param([string]$DataPath)

    $risks = @()

    # 检查单点依赖
    $velocityFile = Join-Path $DataPath "velocity_history.csv"
    if (Test-Path $velocityFile) {
        # 这里可以添加更多资源相关的风险检测
        # 例如：检查贡献者分布、工作负载等
    }

    # 检查磁盘空间（示例）
    $drive = (Get-Item $DataPath).PSDrive
    $freeSpace = $drive.Free / 1GB

    if ($freeSpace -lt 10) {
        $risks += @{
            Id = "RES-001"
            Title = "磁盘空间不足"
            Description = "项目所在磁盘剩余空间仅$([math]::Round($freeSpace, 1))GB"
            Category = "Resource"
            Severity = "Medium"
            Probability = "High"
            Impact = "Medium"
            Status = "Active"
            DetectedDate = Get-Date -Format "yyyy-MM-dd"
            Indicators = @("剩余空间: $([math]::Round($freeSpace, 1))GB")
        }
    }

    return $risks
}

function Save-Risks {
    param($Risks, [string]$FilePath)

    $riskData = Initialize-RiskData
    $riskData.Risks = $Risks

    # 按类别分组
    foreach ($risk in $Risks) {
        $category = $risk.Category
        if ($riskData.RiskCategories.ContainsKey($category)) {
            $riskData.RiskCategories[$category] += $risk
        }
    }

    $dir = Split-Path $FilePath -Parent
    if (-not (Test-Path $dir)) {
        New-Item -ItemType Directory -Path $dir -Force | Out-Null
    }

    $riskData | ConvertTo-Json -Depth 10 | Out-File -FilePath $FilePath -Encoding UTF8
}

function Show-RiskSummary {
    param($Risks)

    Write-Host ""
    Write-Host "=== 风险识别摘要 ===" -ForegroundColor Cyan
    Write-Host ""

    $total = $Risks.Count
    $bySeverity = $Risks | Group-Object Severity
    $byCategory = $Risks | Group-Object Category

    Write-Host "总风险数: $total" -ForegroundColor White
    Write-Host ""

    Write-Host "按严重程度:" -ForegroundColor Yellow
    foreach ($group in $bySeverity) {
        $color = switch ($group.Name) {
            "High" { "Red" }
            "Medium" { "Yellow" }
            "Low" { "Gray" }
            default { "White" }
        }
        Write-Host "  $($group.Name): $($group.Count)" -ForegroundColor $color
    }

    Write-Host ""
    Write-Host "按类别:" -ForegroundColor Yellow
    foreach ($group in $byCategory) {
        Write-Host "  $($group.Name): $($group.Count)" -ForegroundColor White
    }

    Write-Host ""
    Write-Host "高风险项:" -ForegroundColor Red
    $highRisks = $Risks | Where-Object { $_.Severity -eq "High" }
    if ($highRisks.Count -gt 0) {
        foreach ($risk in $highRisks) {
            Write-Host "  - [$($risk.Id)] $($risk.Title)" -ForegroundColor Yellow
            if ($Verbose) {
                Write-Host "    $($risk.Description)" -ForegroundColor Gray
            }
        }
    }
    else {
        Write-Host "  无高风险项" -ForegroundColor Green
    }
}

# 主执行流程
Write-Host "=== 风险识别系统 ===" -ForegroundColor Cyan
Write-Host "扫描时间: $(Get-Date -Format 'yyyy-MM-dd HH:mm:ss')" -ForegroundColor Gray
Write-Host ""

$allRisks = @()

Write-Host "[1/5] 扫描技术风险..." -ForegroundColor Yellow
$techRisks = Scan-TechnicalRisks -ProjectRoot $ProjectRoot -DataPath $DataPath
$allRisks += $techRisks
Write-Host "  发现 $($techRisks.Count) 个技术风险" -ForegroundColor Gray

Write-Host "[2/5] 扫描进度风险..." -ForegroundColor Yellow
$milestoneFile = Join-Path $DataPath "milestones.json"
$schedRisks = Scan-ScheduleRisks -DataPath $DataPath -MilestoneFile $milestoneFile
$allRisks += $schedRisks
Write-Host "  发现 $($schedRisks.Count) 个进度风险" -ForegroundColor Gray

Write-Host "[3/5] 扫描质量风险..." -ForegroundColor Yellow
$qualRisks = Scan-QualityRisks -ProjectRoot $ProjectRoot -DataPath $DataPath
$allRisks += $qualRisks
Write-Host "  发现 $($qualRisks.Count) 个质量风险" -ForegroundColor Gray

Write-Host "[4/5] 扫描资源风险..." -ForegroundColor Yellow
$resRisks = Scan-ResourceRisks -DataPath $DataPath
$allRisks += $resRisks
Write-Host "  发现 $($resRisks.Count) 个资源风险" -ForegroundColor Gray

Write-Host "[5/5] 保存风险数据..." -ForegroundColor Yellow
Save-Risks -Risks $allRisks -FilePath $RiskFile

Show-RiskSummary -Risks $allRisks

Write-Host ""
Write-Host "风险数据已保存: $RiskFile" -ForegroundColor Green

if ($AutoAssess) {
    Write-Host ""
    Write-Host "启动风险评估..." -ForegroundColor Yellow
    $assessScript = Join-Path $PSScriptRoot "AssessRisks.ps1"
    if (Test-Path $assessScript) {
        & $assessScript -RiskFile $RiskFile
    }
}

Write-Host ""
Write-Host "=== 识别完成 ===" -ForegroundColor Cyan
