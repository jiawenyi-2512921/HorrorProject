# QualityGateValidator.ps1
# 质量门禁验证器

param(
    [string]$ProjectRoot = "D:/gptzuo/HorrorProject",
    [string]$ReportPath = "D:/gptzuo/HorrorProject/Reports/QualityGate",
    [ValidateSet("Alpha", "Beta", "RC", "Release")]
    [string]$Stage = "Release",
    [switch]$StrictMode
)

$ErrorActionPreference = "Continue"
$Script:QualityGates = @{
    Critical = @()
    Major = @()
    Minor = @()
    Passed = @()
}

function Write-QualityLog {
    param([string]$Message, [string]$Level = "INFO")
    $color = switch($Level) {
        "CRITICAL" { "Red" }
        "MAJOR" { "Yellow" }
        "MINOR" { "Cyan" }
        "PASS" { "Green" }
        default { "White" }
    }
    Write-Host "[$(Get-Date -Format 'HH:mm:ss')] [$Level] $Message" -ForegroundColor $color
}

function Test-CodeQualityGate {
    Write-Host "`n=== 代码质量门禁 ===" -ForegroundColor Cyan

    $sourcePath = "$ProjectRoot/HorrorProject/Source"
    if (-not (Test-Path $sourcePath)) {
        Write-QualityLog "源代码目录不存在" "CRITICAL"
        $Script:QualityGates.Critical += "源代码目录缺失"
        return
    }

    $sourceFiles = Get-ChildItem -Path $sourcePath -Recurse -Include *.cpp,*.h

    # 检查代码量
    $totalLines = 0
    $commentLines = 0
    $todoCount = 0
    $fixmeCount = 0

    foreach ($file in $sourceFiles) {
        $content = Get-Content $file.FullName -Raw
        $lines = ($content -split "`n").Count
        $totalLines += $lines

        $commentLines += ([regex]::Matches($content, "//.*|/\*[\s\S]*?\*/")).Count
        $todoCount += ([regex]::Matches($content, "TODO", [System.Text.RegularExpressions.RegexOptions]::IgnoreCase)).Count
        $fixmeCount += ([regex]::Matches($content, "FIXME", [System.Text.RegularExpressions.RegexOptions]::IgnoreCase)).Count
    }

    Write-QualityLog "代码文件: $($sourceFiles.Count)" "INFO"
    Write-QualityLog "代码行数: $totalLines" "INFO"
    Write-QualityLog "注释行数: $commentLines" "INFO"

    # 注释率检查
    $commentRatio = if ($totalLines -gt 0) { [math]::Round(($commentLines / $totalLines) * 100, 2) } else { 0 }
    if ($commentRatio -lt 10) {
        Write-QualityLog "注释率过低: $commentRatio% (要求 >= 10%)" "MAJOR"
        $Script:QualityGates.Major += "注释率不足: $commentRatio%"
    } else {
        Write-QualityLog "注释率: $commentRatio%" "PASS"
        $Script:QualityGates.Passed += "注释率达标"
    }

    # TODO/FIXME检查
    if ($fixmeCount -gt 0) {
        Write-QualityLog "发现 $fixmeCount 个 FIXME" "CRITICAL"
        $Script:QualityGates.Critical += "$fixmeCount 个未修复的 FIXME"
    }

    if ($todoCount -gt 10) {
        Write-QualityLog "发现 $todoCount 个 TODO (建议 < 10)" "MINOR"
        $Script:QualityGates.Minor += "$todoCount 个未完成的 TODO"
    } elseif ($todoCount -gt 0) {
        Write-QualityLog "发现 $todoCount 个 TODO" "PASS"
    }
}

function Test-BuildQualityGate {
    Write-Host "`n=== 构建质量门禁 ===" -ForegroundColor Cyan

    # 检查项目文件
    $uprojectPath = "$ProjectRoot/HorrorProject/HorrorProject.uproject"
    if (-not (Test-Path $uprojectPath)) {
        Write-QualityLog "项目文件不存在" "CRITICAL"
        $Script:QualityGates.Critical += "项目文件缺失"
        return
    }

    try {
        $uproject = Get-Content $uprojectPath -Raw | ConvertFrom-Json

        # 检查版本号
        if ($uproject.PSObject.Properties.Name -contains "EngineAssociation") {
            Write-QualityLog "引擎版本: $($uproject.EngineAssociation)" "PASS"
            $Script:QualityGates.Passed += "引擎版本配置正确"
        } else {
            Write-QualityLog "引擎版本未配置" "CRITICAL"
            $Script:QualityGates.Critical += "引擎版本缺失"
        }

        # 检查模块
        if ($uproject.Modules -and $uproject.Modules.Count -gt 0) {
            Write-QualityLog "模块数量: $($uproject.Modules.Count)" "PASS"
            $Script:QualityGates.Passed += "模块配置完整"
        } else {
            Write-QualityLog "没有配置模块" "CRITICAL"
            $Script:QualityGates.Critical += "模块配置缺失"
        }
    }
    catch {
        Write-QualityLog "解析项目文件失败: $_" "CRITICAL"
        $Script:QualityGates.Critical += "项目文件解析失败"
    }

    # 检查构建目标
    $targetFiles = Get-ChildItem -Path "$ProjectRoot/HorrorProject/Source" -Filter "*.Target.cs" -Recurse -ErrorAction SilentlyContinue
    if ($targetFiles.Count -ge 2) {
        Write-QualityLog "构建目标: $($targetFiles.Count)" "PASS"
        $Script:QualityGates.Passed += "构建目标配置完整"
    } else {
        Write-QualityLog "构建目标不完整 (需要至少2个)" "MAJOR"
        $Script:QualityGates.Major += "构建目标配置不完整"
    }
}

function Test-AssetQualityGate {
    Write-Host "`n=== 资产质量门禁 ===" -ForegroundColor Cyan

    $contentPath = "$ProjectRoot/HorrorProject/Content"
    if (-not (Test-Path $contentPath)) {
        Write-QualityLog "Content目录不存在" "CRITICAL"
        $Script:QualityGates.Critical += "Content目录缺失"
        return
    }

    # 检查资产类型
    $assetTypes = @{
        "Blueprints" = "*.uasset"
        "Materials" = "*.uasset"
        "Textures" = "*.uasset"
        "Audio" = "*.uasset"
        "Meshes" = "*.uasset"
    }

    $missingTypes = @()
    foreach ($type in $assetTypes.Keys) {
        $typePath = Join-Path $contentPath $type
        if (Test-Path $typePath) {
            $files = Get-ChildItem -Path $typePath -Recurse -Filter $assetTypes[$type] -ErrorAction SilentlyContinue
            if ($files.Count -gt 0) {
                Write-QualityLog "$type : $($files.Count) 个资产" "PASS"
                $Script:QualityGates.Passed += "$type 资产存在"
            } else {
                Write-QualityLog "$type : 目录为空" "MAJOR"
                $missingTypes += $type
            }
        } else {
            Write-QualityLog "$type : 目录不存在" "MAJOR"
            $missingTypes += $type
        }
    }

    if ($missingTypes.Count -gt 0) {
        $Script:QualityGates.Major += "缺少资产类型: $($missingTypes -join ', ')"
    }

    # 检查空文件
    $allFiles = Get-ChildItem -Path $contentPath -Recurse -File
    $emptyFiles = $allFiles | Where-Object { $_.Length -eq 0 }
    if ($emptyFiles.Count -gt 0) {
        Write-QualityLog "发现 $($emptyFiles.Count) 个空文件" "MAJOR"
        $Script:QualityGates.Major += "$($emptyFiles.Count) 个空资产文件"
    }
}

function Test-PerformanceQualityGate {
    Write-Host "`n=== 性能质量门禁 ===" -ForegroundColor Cyan

    # 检查性能配置
    $engineIni = "$ProjectRoot/HorrorProject/Config/DefaultEngine.ini"
    if (Test-Path $engineIni) {
        $content = Get-Content $engineIni -Raw

        $perfChecks = @{
            "帧率限制" = "t.MaxFPS"
            "LOD系统" = "r.LODDistanceScale"
            "阴影质量" = "r.Shadow"
            "纹理质量" = "r.Streaming"
        }

        $missingConfigs = @()
        foreach ($check in $perfChecks.Keys) {
            if ($content -match [regex]::Escape($perfChecks[$check])) {
                Write-QualityLog "$check : 已配置" "PASS"
                $Script:QualityGates.Passed += "$check 配置存在"
            } else {
                Write-QualityLog "$check : 未配置" "MINOR"
                $missingConfigs += $check
            }
        }

        if ($missingConfigs.Count -gt 0) {
            $Script:QualityGates.Minor += "缺少性能配置: $($missingConfigs -join ', ')"
        }
    } else {
        Write-QualityLog "引擎配置文件不存在" "CRITICAL"
        $Script:QualityGates.Critical += "引擎配置文件缺失"
    }
}

function Test-TestCoverageGate {
    Write-Host "`n=== 测试覆盖率门禁 ===" -ForegroundColor Cyan

    $testPath = "$ProjectRoot/HorrorProject/Source/HorrorProject/Tests"
    if (Test-Path $testPath) {
        $testFiles = Get-ChildItem -Path $testPath -Recurse -Include *.cpp,*.h
        $testCount = $testFiles.Count

        Write-QualityLog "测试文件数量: $testCount" "INFO"

        if ($testCount -eq 0) {
            Write-QualityLog "没有测试文件" "CRITICAL"
            $Script:QualityGates.Critical += "缺少测试文件"
        } elseif ($testCount -lt 5) {
            Write-QualityLog "测试文件过少 (建议 >= 5)" "MAJOR"
            $Script:QualityGates.Major += "测试覆盖率不足"
        } else {
            Write-QualityLog "测试文件数量充足" "PASS"
            $Script:QualityGates.Passed += "测试覆盖率达标"
        }
    } else {
        Write-QualityLog "测试目录不存在" "CRITICAL"
        $Script:QualityGates.Critical += "测试目录缺失"
    }
}

function Test-DocumentationGate {
    Write-Host "`n=== 文档质量门禁 ===" -ForegroundColor Cyan

    $docFiles = @(
        @{Name="README"; Path="$ProjectRoot/README.md"},
        @{Name="LICENSE"; Path="$ProjectRoot/LICENSE"},
        @{Name="CHANGELOG"; Path="$ProjectRoot/CHANGELOG.md"}
    )

    $missingDocs = @()
    foreach ($doc in $docFiles) {
        if (Test-Path $doc.Path) {
            $content = Get-Content $doc.Path -Raw
            if ($content.Length -gt 100) {
                Write-QualityLog "$($doc.Name) : 存在且有内容" "PASS"
                $Script:QualityGates.Passed += "$($doc.Name) 文档完整"
            } else {
                Write-QualityLog "$($doc.Name) : 内容过少" "MINOR"
                $Script:QualityGates.Minor += "$($doc.Name) 文档内容不足"
            }
        } else {
            Write-QualityLog "$($doc.Name) : 不存在" "MINOR"
            $missingDocs += $doc.Name
        }
    }

    if ($missingDocs.Count -gt 0) {
        $Script:QualityGates.Minor += "缺少文档: $($missingDocs -join ', ')"
    }
}

function Test-SecurityGate {
    Write-Host "`n=== 安全质量门禁 ===" -ForegroundColor Cyan

    # 检查敏感文件
    $sensitivePatterns = @("*.key", "*.pem", "*.p12", "*.pfx", "*.env", "*password*", "*secret*")
    $sensitiveFiles = @()

    foreach ($pattern in $sensitivePatterns) {
        $files = Get-ChildItem -Path $ProjectRoot -Recurse -Filter $pattern -ErrorAction SilentlyContinue
        $sensitiveFiles += $files
    }

    if ($sensitiveFiles.Count -gt 0) {
        Write-QualityLog "发现 $($sensitiveFiles.Count) 个敏感文件" "CRITICAL"
        $Script:QualityGates.Critical += "存在敏感文件未排除"
        foreach ($file in $sensitiveFiles) {
            Write-QualityLog "  - $($file.FullName)" "CRITICAL"
        }
    } else {
        Write-QualityLog "未发现敏感文件" "PASS"
        $Script:QualityGates.Passed += "安全检查通过"
    }

    # 检查 .gitignore
    $gitignorePath = "$ProjectRoot/.gitignore"
    if (Test-Path $gitignorePath) {
        Write-QualityLog ".gitignore 存在" "PASS"
        $Script:QualityGates.Passed += ".gitignore 配置存在"
    } else {
        Write-QualityLog ".gitignore 不存在" "MAJOR"
        $Script:QualityGates.Major += "缺少 .gitignore"
    }
}

function Calculate-QualityScore {
    $criticalWeight = 10
    $majorWeight = 5
    $minorWeight = 1

    $totalDeductions = ($Script:QualityGates.Critical.Count * $criticalWeight) +
                       ($Script:QualityGates.Major.Count * $majorWeight) +
                       ($Script:QualityGates.Minor.Count * $minorWeight)

    $maxScore = 100
    $score = [math]::Max(0, $maxScore - $totalDeductions)

    return $score
}

function Get-QualityGrade {
    param([int]$Score)

    if ($Score -ge 95) { return "A+" }
    elseif ($Score -ge 90) { return "A" }
    elseif ($Score -ge 85) { return "B+" }
    elseif ($Score -ge 80) { return "B" }
    elseif ($Score -ge 75) { return "C+" }
    elseif ($Score -ge 70) { return "C" }
    elseif ($Score -ge 60) { return "D" }
    else { return "F" }
}

function Get-ReleaseRecommendation {
    param([int]$Score, [string]$Stage)

    $criticalCount = $Script:QualityGates.Critical.Count
    $majorCount = $Script:QualityGates.Major.Count

    if ($criticalCount -gt 0) {
        return "不建议发布 - 存在严重问题必须修复"
    }

    switch ($Stage) {
        "Alpha" {
            if ($Score -ge 60) { return "可以发布 Alpha 版本" }
            else { return "不建议发布 - 质量不足" }
        }
        "Beta" {
            if ($Score -ge 75 -and $majorCount -le 5) { return "可以发布 Beta 版本" }
            else { return "不建议发布 - 需要修复主要问题" }
        }
        "RC" {
            if ($Score -ge 85 -and $majorCount -le 2) { return "可以发布 RC 版本" }
            else { return "不建议发布 - 需要进一步打磨" }
        }
        "Release" {
            if ($Score -ge 90 -and $majorCount -eq 0) { return "可以正式发布" }
            else { return "不建议发布 - 未达到发布标准" }
        }
    }
}

function Generate-QualityReport {
    Write-Host "`n=== 生成质量报告 ===" -ForegroundColor Cyan

    if (-not (Test-Path $ReportPath)) {
        New-Item -ItemType Directory -Path $ReportPath -Force | Out-Null
    }

    $score = Calculate-QualityScore
    $grade = Get-QualityGrade -Score $score
    $recommendation = Get-ReleaseRecommendation -Score $score -Stage $Stage

    $report = @{
        Timestamp = Get-Date -Format "yyyy-MM-dd HH:mm:ss"
        ProjectRoot = $ProjectRoot
        Stage = $Stage
        Score = $score
        Grade = $grade
        Recommendation = $recommendation
        Issues = @{
            Critical = $Script:QualityGates.Critical
            Major = $Script:QualityGates.Major
            Minor = $Script:QualityGates.Minor
        }
        Passed = $Script:QualityGates.Passed
        Summary = @{
            TotalIssues = $Script:QualityGates.Critical.Count + $Script:QualityGates.Major.Count + $Script:QualityGates.Minor.Count
            CriticalIssues = $Script:QualityGates.Critical.Count
            MajorIssues = $Script:QualityGates.Major.Count
            MinorIssues = $Script:QualityGates.Minor.Count
            PassedChecks = $Script:QualityGates.Passed.Count
        }
    }

    $reportFile = Join-Path $ReportPath "QualityGate_$(Get-Date -Format 'yyyyMMdd_HHmmss').json"
    $report | ConvertTo-Json -Depth 10 | Out-File -FilePath $reportFile -Encoding UTF8

    Write-QualityLog "报告已保存: $reportFile" "PASS"
    return $report
}

function Show-QualitySummary {
    param($Report)

    Write-Host "`n========================================" -ForegroundColor Cyan
    Write-Host "质量门禁验证摘要" -ForegroundColor Cyan
    Write-Host "========================================" -ForegroundColor Cyan
    Write-Host "发布阶段: $($Report.Stage)" -ForegroundColor White
    Write-Host "质量评分: $($Report.Score)/100" -ForegroundColor White
    Write-Host "质量等级: $($Report.Grade)" -ForegroundColor $(if($Report.Grade -match "^A"){"Green"}elseif($Report.Grade -match "^[BC]"){"Yellow"}else{"Red"})
    Write-Host "`n问题统计:" -ForegroundColor White
    Write-Host "  严重问题: $($Report.Summary.CriticalIssues)" -ForegroundColor Red
    Write-Host "  主要问题: $($Report.Summary.MajorIssues)" -ForegroundColor Yellow
    Write-Host "  次要问题: $($Report.Summary.MinorIssues)" -ForegroundColor Cyan
    Write-Host "  通过检查: $($Report.Summary.PassedChecks)" -ForegroundColor Green
    Write-Host "`n建议: $($Report.Recommendation)" -ForegroundColor Cyan
    Write-Host "========================================`n" -ForegroundColor Cyan

    # 显示详细问题
    if ($Report.Issues.Critical.Count -gt 0) {
        Write-Host "严重问题:" -ForegroundColor Red
        foreach ($issue in $Report.Issues.Critical) {
            Write-Host "  - $issue" -ForegroundColor Red
        }
        Write-Host ""
    }

    if ($Report.Issues.Major.Count -gt 0) {
        Write-Host "主要问题:" -ForegroundColor Yellow
        foreach ($issue in $Report.Issues.Major) {
            Write-Host "  - $issue" -ForegroundColor Yellow
        }
        Write-Host ""
    }
}

# 主执行流程
Write-Host "`n开始质量门禁验证...`n" -ForegroundColor Cyan
Write-Host "发布阶段: $Stage" -ForegroundColor White
Write-Host "严格模式: $StrictMode`n" -ForegroundColor White

Test-CodeQualityGate
Test-BuildQualityGate
Test-AssetQualityGate
Test-PerformanceQualityGate
Test-TestCoverageGate
Test-DocumentationGate
Test-SecurityGate

$report = Generate-QualityReport
Show-QualitySummary -Report $report

# 返回退出码
if ($report.Summary.CriticalIssues -gt 0) {
    exit 2
} elseif ($StrictMode -and $report.Summary.MajorIssues -gt 0) {
    exit 1
} elseif ($report.Score -lt 70) {
    exit 1
} else {
    exit 0
}
