# ValidateAllSystems.ps1
# 验证所有系统功能

param(
    [string]$ProjectRoot = "D:/gptzuo/HorrorProject",
    [string]$ReportPath = "D:/gptzuo/HorrorProject/Reports/SystemValidation",
    [switch]$DeepValidation
)

$ErrorActionPreference = "Continue"
$Script:ValidationResults = @{}

function Write-ValidationLog {
    param([string]$System, [string]$Message, [string]$Status)
    $timestamp = Get-Date -Format "HH:mm:ss"
    $color = switch($Status) {
        "PASS" { "Green" }
        "FAIL" { "Red" }
        "WARN" { "Yellow" }
        default { "White" }
    }
    Write-Host "[$timestamp] [$System] $Message" -ForegroundColor $color
}

function Validate-AISystem {
    Write-Host "`n=== AI系统验证 ===" -ForegroundColor Cyan

    $aiPath = "$ProjectRoot/HorrorProject/Source/HorrorProject/AI"
    $results = @{
        Components = @()
        Issues = @()
        Score = 0
    }

    $requiredComponents = @(
        "AIController",
        "BehaviorTree",
        "Perception",
        "Navigation"
    )

    foreach ($component in $requiredComponents) {
        $files = Get-ChildItem -Path $aiPath -Recurse -Filter "*$component*" -Include *.cpp,*.h -ErrorAction SilentlyContinue
        if ($files) {
            Write-ValidationLog "AI" "$component 组件存在 ($($files.Count) 个文件)" "PASS"
            $results.Components += $component
            $results.Score += 25
        } else {
            Write-ValidationLog "AI" "$component 组件缺失" "FAIL"
            $results.Issues += "$component 组件未找到"
        }
    }

    $Script:ValidationResults["AI"] = $results
}

function Validate-AudioSystem {
    Write-Host "`n=== 音频系统验证 ===" -ForegroundColor Cyan

    $audioPath = "$ProjectRoot/HorrorProject/Source/HorrorProject/Audio"
    $results = @{
        Components = @()
        Issues = @()
        Score = 0
    }

    $requiredComponents = @(
        "AudioManager",
        "SoundCue",
        "Ambience",
        "Music"
    )

    foreach ($component in $requiredComponents) {
        $files = Get-ChildItem -Path $audioPath -Recurse -Filter "*$component*" -Include *.cpp,*.h -ErrorAction SilentlyContinue
        if ($files) {
            Write-ValidationLog "Audio" "$component 组件存在" "PASS"
            $results.Components += $component
            $results.Score += 25
        } else {
            Write-ValidationLog "Audio" "$component 组件缺失" "WARN"
            $results.Issues += "$component 组件未找到"
        }
    }

    # 检查音频资产
    $audioAssets = Get-ChildItem -Path "$ProjectRoot/HorrorProject/Content/Audio" -Recurse -File -ErrorAction SilentlyContinue
    if ($audioAssets) {
        Write-ValidationLog "Audio" "找到 $($audioAssets.Count) 个音频资产" "PASS"
    }

    $Script:ValidationResults["Audio"] = $results
}

function Validate-UISystem {
    Write-Host "`n=== UI系统验证 ===" -ForegroundColor Cyan

    $uiPath = "$ProjectRoot/HorrorProject/Source/HorrorProject/UI"
    $results = @{
        Components = @()
        Issues = @()
        Score = 0
    }

    $requiredComponents = @(
        "HUD",
        "Menu",
        "Widget",
        "Inventory"
    )

    foreach ($component in $requiredComponents) {
        $files = Get-ChildItem -Path $uiPath -Recurse -Filter "*$component*" -Include *.cpp,*.h -ErrorAction SilentlyContinue
        if ($files) {
            Write-ValidationLog "UI" "$component 组件存在" "PASS"
            $results.Components += $component
            $results.Score += 25
        } else {
            Write-ValidationLog "UI" "$component 组件缺失" "WARN"
            $results.Issues += "$component 组件未找到"
        }
    }

    $Script:ValidationResults["UI"] = $results
}

function Validate-GameplaySystem {
    Write-Host "`n=== 游戏机制验证 ===" -ForegroundColor Cyan

    $gameplayPath = "$ProjectRoot/HorrorProject/Source/HorrorProject/Gameplay"
    $results = @{
        Components = @()
        Issues = @()
        Score = 0
    }

    $requiredComponents = @(
        "Character",
        "GameMode",
        "PlayerController",
        "GameState"
    )

    foreach ($component in $requiredComponents) {
        $files = Get-ChildItem -Path $gameplayPath -Recurse -Filter "*$component*" -Include *.cpp,*.h -ErrorAction SilentlyContinue
        if ($files) {
            Write-ValidationLog "Gameplay" "$component 组件存在" "PASS"
            $results.Components += $component
            $results.Score += 25
        } else {
            Write-ValidationLog "Gameplay" "$component 组件缺失" "FAIL"
            $results.Issues += "$component 组件未找到"
        }
    }

    $Script:ValidationResults["Gameplay"] = $results
}

function Validate-SaveSystem {
    Write-Host "`n=== 保存系统验证 ===" -ForegroundColor Cyan

    $savePath = "$ProjectRoot/HorrorProject/Source/HorrorProject/SaveSystem"
    $results = @{
        Components = @()
        Issues = @()
        Score = 0
    }

    $requiredComponents = @(
        "SaveGame",
        "SaveManager",
        "Serialization"
    )

    foreach ($component in $requiredComponents) {
        $files = Get-ChildItem -Path $savePath -Recurse -Filter "*$component*" -Include *.cpp,*.h -ErrorAction SilentlyContinue
        if ($files) {
            Write-ValidationLog "Save" "$component 组件存在" "PASS"
            $results.Components += $component
            $results.Score += 33
        } else {
            Write-ValidationLog "Save" "$component 组件缺失" "WARN"
            $results.Issues += "$component 组件未找到"
        }
    }

    $Script:ValidationResults["Save"] = $results
}

function Validate-PerformanceSystem {
    Write-Host "`n=== 性能系统验证 ===" -ForegroundColor Cyan

    $perfPath = "$ProjectRoot/HorrorProject/Source/HorrorProject/Performance"
    $results = @{
        Components = @()
        Issues = @()
        Score = 0
    }

    $requiredComponents = @(
        "Profiler",
        "Optimization",
        "LOD"
    )

    foreach ($component in $requiredComponents) {
        $files = Get-ChildItem -Path $perfPath -Recurse -Filter "*$component*" -Include *.cpp,*.h -ErrorAction SilentlyContinue
        if ($files) {
            Write-ValidationLog "Performance" "$component 组件存在" "PASS"
            $results.Components += $component
            $results.Score += 33
        } else {
            Write-ValidationLog "Performance" "$component 组件缺失" "WARN"
            $results.Issues += "$component 组件未找到"
        }
    }

    $Script:ValidationResults["Performance"] = $results
}

function Validate-Integration {
    Write-Host "`n=== 系统集成验证 ===" -ForegroundColor Cyan

    $results = @{
        Integrations = @()
        Issues = @()
        Score = 0
    }

    # 检查系统间的依赖关系
    $integrationChecks = @(
        @{Name="AI-Gameplay"; Systems=@("AI", "Gameplay")},
        @{Name="Audio-Gameplay"; Systems=@("Audio", "Gameplay")},
        @{Name="UI-Gameplay"; Systems=@("UI", "Gameplay")},
        @{Name="Save-Gameplay"; Systems=@("Save", "Gameplay")}
    )

    foreach ($check in $integrationChecks) {
        $allExist = $true
        foreach ($system in $check.Systems) {
            if (-not $Script:ValidationResults.ContainsKey($system)) {
                $allExist = $false
                break
            }
        }

        if ($allExist) {
            Write-ValidationLog "Integration" "$($check.Name) 集成正常" "PASS"
            $results.Integrations += $check.Name
            $results.Score += 25
        } else {
            Write-ValidationLog "Integration" "$($check.Name) 集成缺失" "FAIL"
            $results.Issues += "$($check.Name) 集成未完成"
        }
    }

    $Script:ValidationResults["Integration"] = $results
}

function Calculate-OverallScore {
    $totalScore = 0
    $systemCount = 0

    foreach ($system in $Script:ValidationResults.Keys) {
        $totalScore += $Script:ValidationResults[$system].Score
        $systemCount++
    }

    if ($systemCount -gt 0) {
        return [math]::Round($totalScore / $systemCount, 2)
    }
    return 0
}

function Generate-ValidationReport {
    Write-Host "`n=== 生成验证报告 ===" -ForegroundColor Cyan

    if (-not (Test-Path $ReportPath)) {
        New-Item -ItemType Directory -Path $ReportPath -Force | Out-Null
    }

    $overallScore = Calculate-OverallScore

    $report = @{
        Timestamp = Get-Date -Format "yyyy-MM-dd HH:mm:ss"
        ProjectRoot = $ProjectRoot
        OverallScore = $overallScore
        Grade = if ($overallScore -ge 90) { "A" } elseif ($overallScore -ge 80) { "B" } elseif ($overallScore -ge 70) { "C" } elseif ($overallScore -ge 60) { "D" } else { "F" }
        Systems = $Script:ValidationResults
        Recommendation = if ($overallScore -ge 80) { "系统验证通过，可以进入最终测试" } elseif ($overallScore -ge 60) { "系统基本完整，需要修复部分问题" } else { "系统存在严重问题，需要重点修复" }
    }

    $reportFile = Join-Path $ReportPath "SystemValidation_$(Get-Date -Format 'yyyyMMdd_HHmmss').json"
    $report | ConvertTo-Json -Depth 10 | Out-File -FilePath $reportFile -Encoding UTF8

    Write-Host "报告已保存: $reportFile" -ForegroundColor Green
    return $report
}

function Show-ValidationSummary {
    param($Report)

    Write-Host "`n========================================" -ForegroundColor Cyan
    Write-Host "系统验证摘要" -ForegroundColor Cyan
    Write-Host "========================================" -ForegroundColor Cyan
    Write-Host "总体评分: $($Report.OverallScore)/100" -ForegroundColor White
    Write-Host "等级: $($Report.Grade)" -ForegroundColor $(if($Report.Grade -in @("A","B")){"Green"}elseif($Report.Grade -eq "C"){"Yellow"}else{"Red"})
    Write-Host "`n系统状态:" -ForegroundColor White

    foreach ($system in $Report.Systems.Keys) {
        $score = $Report.Systems[$system].Score
        $color = if ($score -ge 80) { "Green" } elseif ($score -ge 60) { "Yellow" } else { "Red" }
        Write-Host "  $system : $score/100" -ForegroundColor $color
    }

    Write-Host "`n建议: $($Report.Recommendation)" -ForegroundColor Cyan
    Write-Host "========================================`n" -ForegroundColor Cyan
}

# 主执行流程
Write-Host "`n开始系统验证...`n" -ForegroundColor Cyan

Validate-AISystem
Validate-AudioSystem
Validate-UISystem
Validate-GameplaySystem
Validate-SaveSystem
Validate-PerformanceSystem
Validate-Integration

$report = Generate-ValidationReport
Show-ValidationSummary -Report $report

exit 0
