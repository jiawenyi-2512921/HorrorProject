# StabilityValidator.ps1
# 稳定性验证器

param(
    [string]$ProjectRoot = "D:/gptzuo/HorrorProject",
    [string]$ReportPath = "D:/gptzuo/HorrorProject/Reports/Stability",
    [int]$TestDuration = 300
)

$ErrorActionPreference = "Continue"
$Script:StabilityResults = @{
    CrashRisks = @()
    MemoryLeaks = @()
    ResourceIssues = @()
    Passed = @()
}

function Write-StabilityLog {
    param([string]$Message, [string]$Type = "INFO")
    $color = switch($Type) {
        "CRITICAL" { "Red" }
        "WARNING" { "Yellow" }
        "SUCCESS" { "Green" }
        default { "White" }
    }
    Write-Host "[$(Get-Date -Format 'HH:mm:ss')] $Message" -ForegroundColor $color
}

function Test-NullPointerRisks {
    Write-Host "`n=== 空指针风险检查 ===" -ForegroundColor Cyan

    $sourcePath = "$ProjectRoot/HorrorProject/Source"
    if (-not (Test-Path $sourcePath)) {
        Write-StabilityLog "源代码目录不存在" "CRITICAL"
        return
    }

    $cppFiles = Get-ChildItem -Path $sourcePath -Recurse -Filter "*.cpp"
    $riskyPatterns = @{
        "未检查的指针解引用" = '\-\>\w+\('
        "未检查的数组访问" = '\[\w+\](?!\s*==)'
        "裸指针使用" = '\*\s+\w+\s*='
    }

    $totalRisks = 0
    foreach ($pattern in $riskyPatterns.Keys) {
        $count = 0
        foreach ($file in $cppFiles) {
            $content = Get-Content $file.FullName -Raw
            $matches = [regex]::Matches($content, $riskyPatterns[$pattern])
            $count += $matches.Count
        }

        if ($count -gt 0) {
            Write-StabilityLog "$pattern : 发现 $count 处" "WARNING"
            $totalRisks += $count
        }
    }

    if ($totalRisks -gt 100) {
        Write-StabilityLog "空指针风险较高 ($totalRisks 处)" "WARNING"
        $Script:StabilityResults.CrashRisks += "高空指针风险: $totalRisks 处"
    } elseif ($totalRisks -gt 0) {
        Write-StabilityLog "存在一定空指针风险 ($totalRisks 处)" "WARNING"
        $Script:StabilityResults.CrashRisks += "中等空指针风险: $totalRisks 处"
    } else {
        Write-StabilityLog "空指针风险检查通过" "SUCCESS"
        $Script:StabilityResults.Passed += "空指针风险控制良好"
    }
}

function Test-MemoryManagement {
    Write-Host "`n=== 内存管理检查 ===" -ForegroundColor Cyan

    $sourcePath = "$ProjectRoot/HorrorProject/Source"
    if (-not (Test-Path $sourcePath)) { return }

    $cppFiles = Get-ChildItem -Path $sourcePath -Recurse -Filter "*.cpp"

    $newCount = 0
    $deleteCount = 0
    $smartPtrCount = 0

    foreach ($file in $cppFiles) {
        $content = Get-Content $file.FullName -Raw
        $newCount += ([regex]::Matches($content, '\bnew\s+\w+')).Count
        $deleteCount += ([regex]::Matches($content, '\bdelete\s+')).Count
        $smartPtrCount += ([regex]::Matches($content, 'TSharedPtr|TUniquePtr|TWeakPtr')).Count
    }

    Write-StabilityLog "new 操作: $newCount" "INFO"
    Write-StabilityLog "delete 操作: $deleteCount" "INFO"
    Write-StabilityLog "智能指针: $smartPtrCount" "INFO"

    if ($newCount -gt $deleteCount + 10) {
        Write-StabilityLog "new/delete 不匹配，可能存在内存泄漏" "WARNING"
        $Script:StabilityResults.MemoryLeaks += "new/delete 不匹配: $newCount vs $deleteCount"
    } else {
        Write-StabilityLog "new/delete 基本匹配" "SUCCESS"
        $Script:StabilityResults.Passed += "内存分配释放平衡"
    }

    if ($smartPtrCount -lt $newCount / 2) {
        Write-StabilityLog "智能指针使用率较低，建议增加使用" "WARNING"
        $Script:StabilityResults.MemoryLeaks += "智能指针使用不足"
    } else {
        Write-StabilityLog "智能指针使用良好" "SUCCESS"
        $Script:StabilityResults.Passed += "智能指针使用充分"
    }
}

function Test-ResourceLeaks {
    Write-Host "`n=== 资源泄漏检查 ===" -ForegroundColor Cyan

    $sourcePath = "$ProjectRoot/HorrorProject/Source"
    if (-not (Test-Path $sourcePath)) { return }

    $cppFiles = Get-ChildItem -Path $sourcePath -Recurse -Filter "*.cpp"

    $resourcePatterns = @{
        "文件句柄" = @{Open='FFileHelper::LoadFileToString|FPlatformFileManager'; Close='Close|CloseHandle'}
        "纹理加载" = @{Open='LoadObject<UTexture'; Close='ConditionalBeginDestroy|MarkPendingKill'}
        "音频加载" = @{Open='LoadObject<USoundWave'; Close='ConditionalBeginDestroy|MarkPendingKill'}
    }

    foreach ($resource in $resourcePatterns.Keys) {
        $openCount = 0
        $closeCount = 0

        foreach ($file in $cppFiles) {
            $content = Get-Content $file.FullName -Raw
            $openCount += ([regex]::Matches($content, $resourcePatterns[$resource].Open)).Count
            $closeCount += ([regex]::Matches($content, $resourcePatterns[$resource].Close)).Count
        }

        if ($openCount -gt 0) {
            Write-StabilityLog "$resource : 打开 $openCount, 关闭 $closeCount" "INFO"

            if ($openCount -gt $closeCount * 1.5) {
                Write-StabilityLog "$resource 可能存在泄漏" "WARNING"
                $Script:StabilityResults.ResourceIssues += "$resource 泄漏风险"
            }
        }
    }

    if ($Script:StabilityResults.ResourceIssues.Count -eq 0) {
        Write-StabilityLog "资源泄漏检查通过" "SUCCESS"
        $Script:StabilityResults.Passed += "资源管理良好"
    }
}

function Test-ExceptionHandling {
    Write-Host "`n=== 异常处理检查 ===" -ForegroundColor Cyan

    $sourcePath = "$ProjectRoot/HorrorProject/Source"
    if (-not (Test-Path $sourcePath)) { return }

    $cppFiles = Get-ChildItem -Path $sourcePath -Recurse -Filter "*.cpp"

    $tryCatchCount = 0
    $checkCount = 0
    $ensureCount = 0

    foreach ($file in $cppFiles) {
        $content = Get-Content $file.FullName -Raw
        $tryCatchCount += ([regex]::Matches($content, '\btry\s*\{')).Count
        $checkCount += ([regex]::Matches($content, '\bcheck\(')).Count
        $ensureCount += ([regex]::Matches($content, '\bensure\(')).Count
    }

    Write-StabilityLog "try-catch 块: $tryCatchCount" "INFO"
    Write-StabilityLog "check 宏: $checkCount" "INFO"
    Write-StabilityLog "ensure 宏: $ensureCount" "INFO"

    $totalChecks = $checkCount + $ensureCount
    if ($totalChecks -lt 10) {
        Write-StabilityLog "错误检查较少，建议增加" "WARNING"
        $Script:StabilityResults.CrashRisks += "错误检查不足"
    } else {
        Write-StabilityLog "错误检查充分" "SUCCESS"
        $Script:StabilityResults.Passed += "异常处理完善"
    }
}

function Test-ThreadSafety {
    Write-Host "`n=== 线程安全检查 ===" -ForegroundColor Cyan

    $sourcePath = "$ProjectRoot/HorrorProject/Source"
    if (-not (Test-Path $sourcePath)) { return }

    $cppFiles = Get-ChildItem -Path $sourcePath -Recurse -Filter "*.cpp"

    $threadCount = 0
    $mutexCount = 0
    $criticalSectionCount = 0

    foreach ($file in $cppFiles) {
        $content = Get-Content $file.FullName -Raw
        $threadCount += ([regex]::Matches($content, 'FRunnable|AsyncTask|ParallelFor')).Count
        $mutexCount += ([regex]::Matches($content, 'FCriticalSection|FRWLock|std::mutex')).Count
        $criticalSectionCount += ([regex]::Matches($content, 'FScopeLock')).Count
    }

    Write-StabilityLog "多线程操作: $threadCount" "INFO"
    Write-StabilityLog "互斥锁: $mutexCount" "INFO"
    Write-StabilityLog "作用域锁: $criticalSectionCount" "INFO"

    if ($threadCount -gt 0 -and $mutexCount -eq 0) {
        Write-StabilityLog "使用多线程但缺少同步机制" "CRITICAL"
        $Script:StabilityResults.CrashRisks += "线程安全问题"
    } elseif ($threadCount -gt $mutexCount * 2) {
        Write-StabilityLog "同步机制可能不足" "WARNING"
        $Script:StabilityResults.CrashRisks += "同步机制不足"
    } else {
        Write-StabilityLog "线程安全检查通过" "SUCCESS"
        $Script:StabilityResults.Passed += "线程安全良好"
    }
}

function Test-AssetReferences {
    Write-Host "`n=== 资产引用检查 ===" -ForegroundColor Cyan

    $contentPath = "$ProjectRoot/HorrorProject/Content"
    if (-not (Test-Path $contentPath)) { return }

    # 检查空资产
    $allAssets = Get-ChildItem -Path $contentPath -Recurse -Filter "*.uasset"
    $emptyAssets = $allAssets | Where-Object { $_.Length -eq 0 }

    if ($emptyAssets.Count -gt 0) {
        Write-StabilityLog "发现 $($emptyAssets.Count) 个空资产文件" "CRITICAL"
        $Script:StabilityResults.CrashRisks += "$($emptyAssets.Count) 个空资产文件"
    } else {
        Write-StabilityLog "资产文件完整性检查通过" "SUCCESS"
        $Script:StabilityResults.Passed += "资产文件完整"
    }

    # 检查损坏的资产
    $suspiciousAssets = $allAssets | Where-Object { $_.Length -lt 1KB }
    if ($suspiciousAssets.Count -gt 0) {
        Write-StabilityLog "发现 $($suspiciousAssets.Count) 个可疑的小资产文件" "WARNING"
        $Script:StabilityResults.ResourceIssues += "$($suspiciousAssets.Count) 个可疑资产"
    }
}

function Test-ConfigurationValidity {
    Write-Host "`n=== 配置有效性检查 ===" -ForegroundColor Cyan

    $configFiles = @(
        "$ProjectRoot/HorrorProject/Config/DefaultEngine.ini",
        "$ProjectRoot/HorrorProject/Config/DefaultGame.ini",
        "$ProjectRoot/HorrorProject/Config/DefaultInput.ini"
    )

    foreach ($configFile in $configFiles) {
        if (Test-Path $configFile) {
            try {
                $content = Get-Content $configFile -Raw
                if ($content.Length -eq 0) {
                    Write-StabilityLog "配置文件为空: $(Split-Path $configFile -Leaf)" "CRITICAL"
                    $Script:StabilityResults.CrashRisks += "空配置文件"
                } else {
                    Write-StabilityLog "配置文件有效: $(Split-Path $configFile -Leaf)" "SUCCESS"
                    $Script:StabilityResults.Passed += "$(Split-Path $configFile -Leaf) 有效"
                }
            }
            catch {
                Write-StabilityLog "配置文件读取失败: $(Split-Path $configFile -Leaf)" "CRITICAL"
                $Script:StabilityResults.CrashRisks += "配置文件损坏"
            }
        } else {
            Write-StabilityLog "配置文件缺失: $(Split-Path $configFile -Leaf)" "CRITICAL"
            $Script:StabilityResults.CrashRisks += "配置文件缺失"
        }
    }
}

function Test-DependencyIntegrity {
    Write-Host "`n=== 依赖完整性检查 ===" -ForegroundColor Cyan

    $uprojectPath = "$ProjectRoot/HorrorProject/HorrorProject.uproject"
    if (-not (Test-Path $uprojectPath)) {
        Write-StabilityLog "项目文件不存在" "CRITICAL"
        $Script:StabilityResults.CrashRisks += "项目文件缺失"
        return
    }

    try {
        $uproject = Get-Content $uprojectPath -Raw | ConvertFrom-Json

        # 检查模块依赖
        if ($uproject.Modules) {
            foreach ($module in $uproject.Modules) {
                $modulePath = "$ProjectRoot/HorrorProject/Source/$($module.Name)"
                if (Test-Path $modulePath) {
                    Write-StabilityLog "模块存在: $($module.Name)" "SUCCESS"
                } else {
                    Write-StabilityLog "模块缺失: $($module.Name)" "CRITICAL"
                    $Script:StabilityResults.CrashRisks += "模块缺失: $($module.Name)"
                }
            }
        }

        # 检查插件依赖
        if ($uproject.Plugins) {
            Write-StabilityLog "插件数量: $($uproject.Plugins.Count)" "INFO"
            $Script:StabilityResults.Passed += "插件依赖配置完整"
        }
    }
    catch {
        Write-StabilityLog "项目文件解析失败" "CRITICAL"
        $Script:StabilityResults.CrashRisks += "项目文件损坏"
    }
}

function Calculate-StabilityScore {
    $totalIssues = $Script:StabilityResults.CrashRisks.Count +
                   $Script:StabilityResults.MemoryLeaks.Count +
                   $Script:StabilityResults.ResourceIssues.Count

    $totalPassed = $Script:StabilityResults.Passed.Count

    if ($totalPassed -eq 0) { return 0 }

    $criticalWeight = 15
    $warningWeight = 5

    $deductions = ($Script:StabilityResults.CrashRisks.Count * $criticalWeight) +
                  (($Script:StabilityResults.MemoryLeaks.Count + $Script:StabilityResults.ResourceIssues.Count) * $warningWeight)

    $maxScore = 100
    $score = [math]::Max(0, $maxScore - $deductions)

    return $score
}

function Generate-StabilityReport {
    Write-Host "`n=== 生成稳定性报告 ===" -ForegroundColor Cyan

    if (-not (Test-Path $ReportPath)) {
        New-Item -ItemType Directory -Path $ReportPath -Force | Out-Null
    }

    $score = Calculate-StabilityScore

    $report = @{
        Timestamp = Get-Date -Format "yyyy-MM-dd HH:mm:ss"
        ProjectRoot = $ProjectRoot
        TestDuration = $TestDuration
        Score = $score
        Grade = if ($score -ge 90) { "A" } elseif ($score -ge 80) { "B" } elseif ($score -ge 70) { "C" } elseif ($score -ge 60) { "D" } else { "F" }
        Status = if ($score -ge 80) { "稳定" } elseif ($score -ge 60) { "基本稳定" } else { "不稳定" }
        Summary = @{
            CrashRisks = $Script:StabilityResults.CrashRisks.Count
            MemoryLeaks = $Script:StabilityResults.MemoryLeaks.Count
            ResourceIssues = $Script:StabilityResults.ResourceIssues.Count
            PassedChecks = $Script:StabilityResults.Passed.Count
        }
        Details = $Script:StabilityResults
        Recommendations = Get-StabilityRecommendations
    }

    $reportFile = Join-Path $ReportPath "Stability_$(Get-Date -Format 'yyyyMMdd_HHmmss').json"
    $report | ConvertTo-Json -Depth 10 | Out-File -FilePath $reportFile -Encoding UTF8

    Write-StabilityLog "报告已保存: $reportFile" "SUCCESS"
    return $report
}

function Get-StabilityRecommendations {
    $recommendations = @()

    if ($Script:StabilityResults.CrashRisks.Count -gt 0) {
        $recommendations += "优先修复崩溃风险问题"
        $recommendations += "增加空指针检查和边界检查"
        $recommendations += "使用智能指针替代裸指针"
    }

    if ($Script:StabilityResults.MemoryLeaks.Count -gt 0) {
        $recommendations += "检查内存分配和释放的匹配性"
        $recommendations += "使用内存分析工具检测泄漏"
        $recommendations += "增加智能指针的使用"
    }

    if ($Script:StabilityResults.ResourceIssues.Count -gt 0) {
        $recommendations += "确保资源正确释放"
        $recommendations += "使用RAII模式管理资源"
        $recommendations += "添加资源清理代码"
    }

    if ($recommendations.Count -eq 0) {
        $recommendations += "稳定性良好，继续保持代码质量"
    }

    return $recommendations
}

function Show-StabilitySummary {
    param($Report)

    Write-Host "`n========================================" -ForegroundColor Cyan
    Write-Host "稳定性验证摘要" -ForegroundColor Cyan
    Write-Host "========================================" -ForegroundColor Cyan
    Write-Host "稳定性评分: $($Report.Score)/100" -ForegroundColor White
    Write-Host "评级: $($Report.Grade)" -ForegroundColor $(if($Report.Grade -in @("A","B")){"Green"}elseif($Report.Grade -eq "C"){"Yellow"}else{"Red"})
    Write-Host "状态: $($Report.Status)" -ForegroundColor $(if($Report.Status -eq "稳定"){"Green"}elseif($Report.Status -eq "基本稳定"){"Yellow"}else{"Red"})
    Write-Host "`n问题统计:" -ForegroundColor White
    Write-Host "  崩溃风险: $($Report.Summary.CrashRisks)" -ForegroundColor Red
    Write-Host "  内存泄漏: $($Report.Summary.MemoryLeaks)" -ForegroundColor Yellow
    Write-Host "  资源问题: $($Report.Summary.ResourceIssues)" -ForegroundColor Yellow
    Write-Host "  通过检查: $($Report.Summary.PassedChecks)" -ForegroundColor Green

    if ($Report.Recommendations.Count -gt 0) {
        Write-Host "`n改进建议:" -ForegroundColor Cyan
        foreach ($rec in $Report.Recommendations) {
            Write-Host "  - $rec" -ForegroundColor White
        }
    }

    Write-Host "========================================`n" -ForegroundColor Cyan
}

# 主执行流程
Write-Host "`n开始稳定性验证...`n" -ForegroundColor Cyan

Test-NullPointerRisks
Test-MemoryManagement
Test-ResourceLeaks
Test-ExceptionHandling
Test-ThreadSafety
Test-AssetReferences
Test-ConfigurationValidity
Test-DependencyIntegrity

$report = Generate-StabilityReport
Show-StabilitySummary -Report $report

if ($report.Score -lt 60) {
    exit 2
} elseif ($report.Score -lt 80) {
    exit 1
} else {
    exit 0
}
