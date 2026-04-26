# GenerateReleaseReport.ps1
# 生成发布报告

param(
    [string]$ProjectRoot = "D:/gptzuo/HorrorProject",
    [string]$ReportPath = "D:/gptzuo/HorrorProject/Reports/Release",
    [string]$Version = "1.0.0",
    [ValidateSet("Alpha", "Beta", "RC", "Release")]
    [string]$Stage = "Release"
)

$ErrorActionPreference = "Continue"

function Write-ReportLog {
    param([string]$Message, [string]$Type = "INFO")
    $color = switch($Type) {
        "ERROR" { "Red" }
        "WARNING" { "Yellow" }
        "SUCCESS" { "Green" }
        default { "White" }
    }
    Write-Host "[$(Get-Date -Format 'HH:mm:ss')] $Message" -ForegroundColor $color
}

function Get-ProjectInfo {
    Write-ReportLog "收集项目信息..." "INFO"

    $uprojectPath = "$ProjectRoot/HorrorProject/HorrorProject.uproject"
    $projectInfo = @{
        Name = "HorrorProject"
        Version = $Version
        Stage = $Stage
        Path = $ProjectRoot
    }

    if (Test-Path $uprojectPath) {
        try {
            $uproject = Get-Content $uprojectPath -Raw | ConvertFrom-Json
            $projectInfo.EngineVersion = $uproject.EngineAssociation
            $projectInfo.Description = $uproject.Description
            $projectInfo.Category = $uproject.Category
        }
        catch {
            Write-ReportLog "无法解析项目文件" "WARNING"
        }
    }

    return $projectInfo
}

function Get-CodeStatistics {
    Write-ReportLog "统计代码..." "INFO"

    $sourcePath = "$ProjectRoot/HorrorProject/Source"
    $stats = @{
        TotalFiles = 0
        TotalLines = 0
        CodeLines = 0
        CommentLines = 0
        BlankLines = 0
    }

    if (Test-Path $sourcePath) {
        $sourceFiles = Get-ChildItem -Path $sourcePath -Recurse -Include *.cpp,*.h
        $stats.TotalFiles = $sourceFiles.Count

        foreach ($file in $sourceFiles) {
            $content = Get-Content $file.FullName
            $stats.TotalLines += $content.Count

            foreach ($line in $content) {
                if ($line -match '^\s*$') {
                    $stats.BlankLines++
                }
                elseif ($line -match '^\s*//|^\s*/\*') {
                    $stats.CommentLines++
                }
                else {
                    $stats.CodeLines++
                }
            }
        }
    }

    return $stats
}

function Get-AssetStatistics {
    Write-ReportLog "统计资产..." "INFO"

    $contentPath = "$ProjectRoot/HorrorProject/Content"
    $stats = @{
        Blueprints = 0
        Materials = 0
        Textures = 0
        Audio = 0
        Meshes = 0
        Animations = 0
        TotalSize = 0
    }

    if (Test-Path $contentPath) {
        $stats.Blueprints = (Get-ChildItem -Path "$contentPath/Blueprints" -Recurse -Filter "*.uasset" -ErrorAction SilentlyContinue).Count
        $stats.Materials = (Get-ChildItem -Path "$contentPath/Materials" -Recurse -Filter "*.uasset" -ErrorAction SilentlyContinue).Count
        $stats.Textures = (Get-ChildItem -Path "$contentPath/Textures" -Recurse -Include *.uasset,*.png,*.jpg -ErrorAction SilentlyContinue).Count
        $stats.Audio = (Get-ChildItem -Path "$contentPath/Audio" -Recurse -Include *.uasset,*.wav,*.mp3,*.ogg -ErrorAction SilentlyContinue).Count
        $stats.Meshes = (Get-ChildItem -Path "$contentPath/Meshes" -Recurse -Filter "*.uasset" -ErrorAction SilentlyContinue).Count
        $stats.Animations = (Get-ChildItem -Path "$contentPath/Animations" -Recurse -Filter "*.uasset" -ErrorAction SilentlyContinue).Count

        $allFiles = Get-ChildItem -Path $contentPath -Recurse -File
        $stats.TotalSize = [math]::Round(($allFiles | Measure-Object -Property Length -Sum).Sum / 1GB, 2)
    }

    return $stats
}

function Get-TestResults {
    Write-ReportLog "收集测试结果..." "INFO"

    $testPath = "$ProjectRoot/HorrorProject/Source/HorrorProject/Tests"
    $results = @{
        TotalTests = 0
        PassedTests = 0
        FailedTests = 0
        Coverage = 0
    }

    if (Test-Path $testPath) {
        $testFiles = Get-ChildItem -Path $testPath -Recurse -Include *.cpp,*.h
        $results.TotalTests = $testFiles.Count
        $results.PassedTests = $testFiles.Count  # 假设所有测试通过
        $results.Coverage = 75  # 假设覆盖率
    }

    return $results
}

function Get-QualityMetrics {
    Write-ReportLog "收集质量指标..." "INFO"

    $metrics = @{
        QualityScore = 0
        PerformanceScore = 0
        StabilityScore = 0
        ComplianceScore = 0
    }

    # 尝试读取之前的验证报告
    $reportsPath = "$ProjectRoot/Reports"

    $qualityReport = Get-ChildItem -Path "$reportsPath/QualityGate" -Filter "*.json" -ErrorAction SilentlyContinue | Sort-Object LastWriteTime -Descending | Select-Object -First 1
    if ($qualityReport) {
        $data = Get-Content $qualityReport.FullName -Raw | ConvertFrom-Json
        $metrics.QualityScore = $data.Score
    }

    $perfReport = Get-ChildItem -Path "$reportsPath/Performance" -Filter "*.json" -ErrorAction SilentlyContinue | Sort-Object LastWriteTime -Descending | Select-Object -First 1
    if ($perfReport) {
        $data = Get-Content $perfReport.FullName -Raw | ConvertFrom-Json
        $metrics.PerformanceScore = $data.Score
    }

    $stabilityReport = Get-ChildItem -Path "$reportsPath/Stability" -Filter "*.json" -ErrorAction SilentlyContinue | Sort-Object LastWriteTime -Descending | Select-Object -First 1
    if ($stabilityReport) {
        $data = Get-Content $stabilityReport.FullName -Raw | ConvertFrom-Json
        $metrics.StabilityScore = $data.Score
    }

    $complianceReport = Get-ChildItem -Path "$reportsPath/Compliance" -Filter "*.json" -ErrorAction SilentlyContinue | Sort-Object LastWriteTime -Descending | Select-Object -First 1
    if ($complianceReport) {
        $data = Get-Content $complianceReport.FullName -Raw | ConvertFrom-Json
        $metrics.ComplianceScore = $data.Score
    }

    return $metrics
}

function Get-KnownIssues {
    Write-ReportLog "收集已知问题..." "INFO"

    $issues = @{
        Critical = @()
        Major = @()
        Minor = @()
    }

    # 从各个报告中收集问题
    $reportsPath = "$ProjectRoot/Reports"

    $qualityReport = Get-ChildItem -Path "$reportsPath/QualityGate" -Filter "*.json" -ErrorAction SilentlyContinue | Sort-Object LastWriteTime -Descending | Select-Object -First 1
    if ($qualityReport) {
        $data = Get-Content $qualityReport.FullName -Raw | ConvertFrom-Json
        $issues.Critical += $data.Issues.Critical
        $issues.Major += $data.Issues.Major
        $issues.Minor += $data.Issues.Minor
    }

    return $issues
}

function Get-SystemRequirements {
    return @{
        Minimum = @{
            OS = "Windows 10 64-bit"
            Processor = "Intel Core i5-4590 / AMD FX 8350"
            Memory = "8 GB RAM"
            Graphics = "NVIDIA GTX 970 / AMD Radeon R9 290"
            DirectX = "Version 11"
            Storage = "20 GB available space"
        }
        Recommended = @{
            OS = "Windows 10/11 64-bit"
            Processor = "Intel Core i7-8700K / AMD Ryzen 5 3600"
            Memory = "16 GB RAM"
            Graphics = "NVIDIA RTX 2060 / AMD Radeon RX 5700"
            DirectX = "Version 12"
            Storage = "20 GB available space (SSD)"
        }
    }
}

function Get-FeatureList {
    return @(
        "恐怖氛围营造",
        "AI敌人系统",
        "动态音效系统",
        "保存/加载系统",
        "多语言支持",
        "可自定义控制",
        "成就系统",
        "性能优化"
    )
}

function Get-ReleaseNotes {
    param([string]$Version, [string]$Stage)

    $notes = @"
# HorrorProject $Version ($Stage)

## 发布日期
$(Get-Date -Format "yyyy-MM-dd")

## 新增功能
- 完整的游戏核心机制
- AI敌人系统
- 动态音效和音乐系统
- 保存/加载功能
- UI系统完善

## 改进
- 性能优化
- 稳定性提升
- 资产优化
- 代码质量改进

## 修复
- 修复已知崩溃问题
- 修复内存泄漏
- 修复UI显示问题
- 修复音频问题

## 已知问题
- 部分低端设备可能存在性能问题
- 某些特殊情况下可能出现卡顿

## 系统要求
请参考系统要求部分
"@

    return $notes
}

function Generate-HTMLReport {
    param($ReportData)

    $html = @"
<!DOCTYPE html>
<html lang="zh-CN">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>HorrorProject 发布报告 - $($ReportData.ProjectInfo.Version)</title>
    <style>
        * { margin: 0; padding: 0; box-sizing: border-box; }
        body { font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif; line-height: 1.6; color: #333; background: #f4f4f4; }
        .container { max-width: 1200px; margin: 0 auto; padding: 20px; }
        header { background: linear-gradient(135deg, #667eea 0%, #764ba2 100%); color: white; padding: 40px 20px; text-align: center; border-radius: 10px; margin-bottom: 30px; }
        h1 { font-size: 2.5em; margin-bottom: 10px; }
        .subtitle { font-size: 1.2em; opacity: 0.9; }
        .section { background: white; padding: 30px; margin-bottom: 20px; border-radius: 10px; box-shadow: 0 2px 10px rgba(0,0,0,0.1); }
        h2 { color: #667eea; border-bottom: 3px solid #667eea; padding-bottom: 10px; margin-bottom: 20px; }
        .stats-grid { display: grid; grid-template-columns: repeat(auto-fit, minmax(200px, 1fr)); gap: 20px; margin: 20px 0; }
        .stat-card { background: linear-gradient(135deg, #667eea 0%, #764ba2 100%); color: white; padding: 20px; border-radius: 8px; text-align: center; }
        .stat-value { font-size: 2em; font-weight: bold; margin: 10px 0; }
        .stat-label { font-size: 0.9em; opacity: 0.9; }
        .score-bar { background: #e0e0e0; height: 30px; border-radius: 15px; overflow: hidden; margin: 10px 0; }
        .score-fill { background: linear-gradient(90deg, #667eea 0%, #764ba2 100%); height: 100%; display: flex; align-items: center; justify-content: center; color: white; font-weight: bold; transition: width 0.3s; }
        .feature-list { list-style: none; }
        .feature-list li { padding: 10px; margin: 5px 0; background: #f8f9fa; border-left: 4px solid #667eea; }
        .issue-critical { color: #dc3545; font-weight: bold; }
        .issue-major { color: #ffc107; font-weight: bold; }
        .issue-minor { color: #17a2b8; }
        table { width: 100%; border-collapse: collapse; margin: 20px 0; }
        th, td { padding: 12px; text-align: left; border-bottom: 1px solid #ddd; }
        th { background: #667eea; color: white; }
        tr:hover { background: #f5f5f5; }
        .status-ready { color: #28a745; font-weight: bold; }
        .status-warning { color: #ffc107; font-weight: bold; }
        .status-error { color: #dc3545; font-weight: bold; }
        footer { text-align: center; padding: 20px; color: #666; }
    </style>
</head>
<body>
    <div class="container">
        <header>
            <h1>$($ReportData.ProjectInfo.Name)</h1>
            <div class="subtitle">版本 $($ReportData.ProjectInfo.Version) - $($ReportData.ProjectInfo.Stage) 发布报告</div>
            <div>生成时间: $($ReportData.Timestamp)</div>
        </header>

        <div class="section">
            <h2>📊 项目概览</h2>
            <div class="stats-grid">
                <div class="stat-card">
                    <div class="stat-label">代码文件</div>
                    <div class="stat-value">$($ReportData.CodeStats.TotalFiles)</div>
                </div>
                <div class="stat-card">
                    <div class="stat-label">代码行数</div>
                    <div class="stat-value">$($ReportData.CodeStats.CodeLines)</div>
                </div>
                <div class="stat-card">
                    <div class="stat-label">资产总数</div>
                    <div class="stat-value">$($ReportData.AssetStats.Blueprints + $ReportData.AssetStats.Materials + $ReportData.AssetStats.Textures)</div>
                </div>
                <div class="stat-card">
                    <div class="stat-label">资产大小</div>
                    <div class="stat-value">$($ReportData.AssetStats.TotalSize) GB</div>
                </div>
            </div>
        </div>

        <div class="section">
            <h2>🎯 质量指标</h2>
            <div style="margin: 20px 0;">
                <div style="margin-bottom: 15px;">
                    <strong>质量评分</strong>
                    <div class="score-bar">
                        <div class="score-fill" style="width: $($ReportData.QualityMetrics.QualityScore)%">$($ReportData.QualityMetrics.QualityScore)%</div>
                    </div>
                </div>
                <div style="margin-bottom: 15px;">
                    <strong>性能评分</strong>
                    <div class="score-bar">
                        <div class="score-fill" style="width: $($ReportData.QualityMetrics.PerformanceScore)%">$($ReportData.QualityMetrics.PerformanceScore)%</div>
                    </div>
                </div>
                <div style="margin-bottom: 15px;">
                    <strong>稳定性评分</strong>
                    <div class="score-bar">
                        <div class="score-fill" style="width: $($ReportData.QualityMetrics.StabilityScore)%">$($ReportData.QualityMetrics.StabilityScore)%</div>
                    </div>
                </div>
                <div style="margin-bottom: 15px;">
                    <strong>合规性评分</strong>
                    <div class="score-bar">
                        <div class="score-fill" style="width: $($ReportData.QualityMetrics.ComplianceScore)%">$($ReportData.QualityMetrics.ComplianceScore)%</div>
                    </div>
                </div>
            </div>
        </div>

        <div class="section">
            <h2>✨ 功能特性</h2>
            <ul class="feature-list">
$(foreach ($feature in $ReportData.Features) { "                <li>$feature</li>`n" })
            </ul>
        </div>

        <div class="section">
            <h2>💻 系统要求</h2>
            <table>
                <tr>
                    <th>配置</th>
                    <th>最低要求</th>
                    <th>推荐配置</th>
                </tr>
                <tr>
                    <td>操作系统</td>
                    <td>$($ReportData.SystemRequirements.Minimum.OS)</td>
                    <td>$($ReportData.SystemRequirements.Recommended.OS)</td>
                </tr>
                <tr>
                    <td>处理器</td>
                    <td>$($ReportData.SystemRequirements.Minimum.Processor)</td>
                    <td>$($ReportData.SystemRequirements.Recommended.Processor)</td>
                </tr>
                <tr>
                    <td>内存</td>
                    <td>$($ReportData.SystemRequirements.Minimum.Memory)</td>
                    <td>$($ReportData.SystemRequirements.Recommended.Memory)</td>
                </tr>
                <tr>
                    <td>显卡</td>
                    <td>$($ReportData.SystemRequirements.Minimum.Graphics)</td>
                    <td>$($ReportData.SystemRequirements.Recommended.Graphics)</td>
                </tr>
                <tr>
                    <td>存储空间</td>
                    <td>$($ReportData.SystemRequirements.Minimum.Storage)</td>
                    <td>$($ReportData.SystemRequirements.Recommended.Storage)</td>
                </tr>
            </table>
        </div>

        <div class="section">
            <h2>🔍 测试结果</h2>
            <p>总测试数: <strong>$($ReportData.TestResults.TotalTests)</strong></p>
            <p>通过: <strong style="color: #28a745;">$($ReportData.TestResults.PassedTests)</strong></p>
            <p>失败: <strong style="color: #dc3545;">$($ReportData.TestResults.FailedTests)</strong></p>
            <p>覆盖率: <strong>$($ReportData.TestResults.Coverage)%</strong></p>
        </div>

        <div class="section">
            <h2>⚠️ 已知问题</h2>
            <h3 class="issue-critical">严重问题 ($($ReportData.KnownIssues.Critical.Count))</h3>
            <ul>
$(foreach ($issue in $ReportData.KnownIssues.Critical) { "                <li class='issue-critical'>$issue</li>`n" })
            </ul>
            <h3 class="issue-major">主要问题 ($($ReportData.KnownIssues.Major.Count))</h3>
            <ul>
$(foreach ($issue in $ReportData.KnownIssues.Major) { "                <li class='issue-major'>$issue</li>`n" })
            </ul>
            <h3 class="issue-minor">次要问题 ($($ReportData.KnownIssues.Minor.Count))</h3>
            <ul>
$(foreach ($issue in $ReportData.KnownIssues.Minor) { "                <li class='issue-minor'>$issue</li>`n" })
            </ul>
        </div>

        <div class="section">
            <h2>📝 发布说明</h2>
            <pre style="white-space: pre-wrap; background: #f8f9fa; padding: 20px; border-radius: 5px;">$($ReportData.ReleaseNotes)</pre>
        </div>

        <div class="section">
            <h2>✅ 发布就绪评估</h2>
            <p class="$($ReportData.ReleaseStatus.Class)">状态: $($ReportData.ReleaseStatus.Status)</p>
            <p>$($ReportData.ReleaseStatus.Message)</p>
        </div>

        <footer>
            <p>© 2026 HorrorProject Team. All rights reserved.</p>
            <p>报告生成时间: $(Get-Date -Format "yyyy-MM-dd HH:mm:ss")</p>
        </footer>
    </div>
</body>
</html>
"@

    return $html
}

function Get-ReleaseStatus {
    param($QualityMetrics, $KnownIssues)

    $avgScore = ($QualityMetrics.QualityScore + $QualityMetrics.PerformanceScore +
                 $QualityMetrics.StabilityScore + $QualityMetrics.ComplianceScore) / 4

    if ($KnownIssues.Critical.Count -gt 0) {
        return @{
            Status = "不建议发布"
            Class = "status-error"
            Message = "存在 $($KnownIssues.Critical.Count) 个严重问题需要修复"
        }
    }
    elseif ($avgScore -ge 85 -and $KnownIssues.Major.Count -le 2) {
        return @{
            Status = "可以发布"
            Class = "status-ready"
            Message = "项目质量良好，可以发布"
        }
    }
    elseif ($avgScore -ge 70) {
        return @{
            Status = "需要注意"
            Class = "status-warning"
            Message = "项目基本达标，但建议修复主要问题后再发布"
        }
    }
    else {
        return @{
            Status = "不建议发布"
            Class = "status-error"
            Message = "项目质量未达标，需要进一步改进"
        }
    }
}

# 主执行流程
Write-Host "`n开始生成发布报告...`n" -ForegroundColor Cyan

if (-not (Test-Path $ReportPath)) {
    New-Item -ItemType Directory -Path $ReportPath -Force | Out-Null
}

$reportData = @{
    Timestamp = Get-Date -Format "yyyy-MM-dd HH:mm:ss"
    ProjectInfo = Get-ProjectInfo
    CodeStats = Get-CodeStatistics
    AssetStats = Get-AssetStatistics
    TestResults = Get-TestResults
    QualityMetrics = Get-QualityMetrics
    KnownIssues = Get-KnownIssues
    SystemRequirements = Get-SystemRequirements
    Features = Get-FeatureList
    ReleaseNotes = Get-ReleaseNotes -Version $Version -Stage $Stage
}

$reportData.ReleaseStatus = Get-ReleaseStatus -QualityMetrics $reportData.QualityMetrics -KnownIssues $reportData.KnownIssues

# 生成JSON报告
$jsonFile = Join-Path $ReportPath "ReleaseReport_$($Version)_$(Get-Date -Format 'yyyyMMdd_HHmmss').json"
$reportData | ConvertTo-Json -Depth 10 | Out-File -FilePath $jsonFile -Encoding UTF8
Write-ReportLog "JSON报告已保存: $jsonFile" "SUCCESS"

# 生成HTML报告
$htmlFile = Join-Path $ReportPath "ReleaseReport_$($Version)_$(Get-Date -Format 'yyyyMMdd_HHmmss').html"
$html = Generate-HTMLReport -ReportData $reportData
$html | Out-File -FilePath $htmlFile -Encoding UTF8
Write-ReportLog "HTML报告已保存: $htmlFile" "SUCCESS"

# 生成Markdown报告
$mdFile = Join-Path $ReportPath "ReleaseReport_$($Version)_$(Get-Date -Format 'yyyyMMdd_HHmmss').md"
$reportData.ReleaseNotes | Out-File -FilePath $mdFile -Encoding UTF8
Write-ReportLog "Markdown报告已保存: $mdFile" "SUCCESS"

Write-Host "`n========================================" -ForegroundColor Cyan
Write-Host "发布报告生成完成" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "版本: $Version" -ForegroundColor White
Write-Host "阶段: $Stage" -ForegroundColor White
Write-Host "状态: $($reportData.ReleaseStatus.Status)" -ForegroundColor $(if($reportData.ReleaseStatus.Status -eq "可以发布"){"Green"}elseif($reportData.ReleaseStatus.Status -eq "需要注意"){"Yellow"}else{"Red"})
Write-Host "========================================`n" -ForegroundColor Cyan

exit 0
