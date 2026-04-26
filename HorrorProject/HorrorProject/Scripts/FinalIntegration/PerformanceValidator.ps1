# PerformanceValidator.ps1
# 性能验证器

param(
    [string]$ProjectRoot = "D:/gptzuo/HorrorProject",
    [string]$ReportPath = "D:/gptzuo/HorrorProject/Reports/Performance",
    [ValidateSet("Low", "Medium", "High", "Ultra")]
    [string]$TargetQuality = "High",
    [int]$TargetFPS = 60
)

$ErrorActionPreference = "Continue"
$Script:PerfResults = @{
    Metrics = @{}
    Issues = @()
    Warnings = @()
    Passed = @()
}

function Write-PerfLog {
    param([string]$Message, [string]$Type = "INFO")
    $color = switch($Type) {
        "ERROR" { "Red" }
        "WARNING" { "Yellow" }
        "SUCCESS" { "Green" }
        default { "White" }
    }
    Write-Host "[$(Get-Date -Format 'HH:mm:ss')] $Message" -ForegroundColor $color
}

function Test-RenderingPerformance {
    Write-Host "`n=== 渲染性能检查 ===" -ForegroundColor Cyan

    $engineIni = "$ProjectRoot/HorrorProject/Config/DefaultEngine.ini"
    if (-not (Test-Path $engineIni)) {
        Write-PerfLog "引擎配置文件不存在" "ERROR"
        $Script:PerfResults.Issues += "引擎配置缺失"
        return
    }

    $content = Get-Content $engineIni -Raw

    # 检查帧率设置
    if ($content -match "t\.MaxFPS=(\d+)") {
        $maxFPS = [int]$matches[1]
        Write-PerfLog "最大帧率: $maxFPS" "INFO"
        $Script:PerfResults.Metrics["MaxFPS"] = $maxFPS

        if ($maxFPS -ge $TargetFPS) {
            Write-PerfLog "帧率设置符合目标" "SUCCESS"
            $Script:PerfResults.Passed += "帧率配置正确"
        } else {
            Write-PerfLog "帧率设置低于目标 ($TargetFPS)" "WARNING"
            $Script:PerfResults.Warnings += "帧率配置过低"
        }
    } else {
        Write-PerfLog "未配置最大帧率" "WARNING"
        $Script:PerfResults.Warnings += "缺少帧率配置"
    }

    # 检查VSync设置
    if ($content -match "r\.VSync=(\d+)") {
        $vsync = [int]$matches[1]
        Write-PerfLog "VSync: $(if($vsync -eq 1){'启用'}else{'禁用'})" "INFO"
        $Script:PerfResults.Metrics["VSync"] = $vsync
    }

    # 检查阴影质量
    $shadowSettings = @{
        "阴影距离" = "r\.Shadow\.MaxResolution"
        "级联阴影" = "r\.Shadow\.CSM"
        "动态阴影" = "r\.Shadow\.Dynamic"
    }

    foreach ($setting in $shadowSettings.Keys) {
        if ($content -match [regex]::Escape($shadowSettings[$setting])) {
            Write-PerfLog "$setting : 已配置" "SUCCESS"
            $Script:PerfResults.Passed += "$setting 配置"
        } else {
            Write-PerfLog "$setting : 未配置" "WARNING"
            $Script:PerfResults.Warnings += "$setting 未配置"
        }
    }
}

function Test-AssetPerformance {
    Write-Host "`n=== 资产性能检查 ===" -ForegroundColor Cyan

    $contentPath = "$ProjectRoot/HorrorProject/Content"
    if (-not (Test-Path $contentPath)) {
        Write-PerfLog "Content目录不存在" "ERROR"
        $Script:PerfResults.Issues += "Content目录缺失"
        return
    }

    # 检查纹理大小
    $textures = Get-ChildItem -Path $contentPath -Recurse -Include *.png,*.jpg,*.tga -ErrorAction SilentlyContinue
    $largeTextures = @()
    $totalTextureSize = 0

    foreach ($texture in $textures) {
        $totalTextureSize += $texture.Length
        if ($texture.Length -gt 10MB) {
            $largeTextures += $texture
            Write-PerfLog "大纹理: $($texture.Name) - $([math]::Round($texture.Length/1MB, 2)) MB" "WARNING"
        }
    }

    $totalTextureSizeMB = [math]::Round($totalTextureSize / 1MB, 2)
    Write-PerfLog "纹理总数: $($textures.Count)" "INFO"
    Write-PerfLog "纹理总大小: $totalTextureSizeMB MB" "INFO"
    $Script:PerfResults.Metrics["TextureCount"] = $textures.Count
    $Script:PerfResults.Metrics["TextureSizeMB"] = $totalTextureSizeMB

    if ($largeTextures.Count -gt 0) {
        Write-PerfLog "发现 $($largeTextures.Count) 个大纹理 (>10MB)" "WARNING"
        $Script:PerfResults.Warnings += "$($largeTextures.Count) 个大纹理需要优化"
    } else {
        Write-PerfLog "纹理大小合理" "SUCCESS"
        $Script:PerfResults.Passed += "纹理大小优化良好"
    }

    # 检查网格复杂度
    $meshes = Get-ChildItem -Path $contentPath -Recurse -Filter "*.uasset" -ErrorAction SilentlyContinue |
              Where-Object { $_.DirectoryName -like "*Meshes*" }

    Write-PerfLog "网格数量: $($meshes.Count)" "INFO"
    $Script:PerfResults.Metrics["MeshCount"] = $meshes.Count

    if ($meshes.Count -gt 1000) {
        Write-PerfLog "网格数量较多，建议优化" "WARNING"
        $Script:PerfResults.Warnings += "网格数量过多"
    }
}

function Test-MemoryUsage {
    Write-Host "`n=== 内存使用检查 ===" -ForegroundColor Cyan

    $contentPath = "$ProjectRoot/HorrorProject/Content"
    if (-not (Test-Path $contentPath)) { return }

    # 计算总资产大小
    $allAssets = Get-ChildItem -Path $contentPath -Recurse -File
    $totalSize = ($allAssets | Measure-Object -Property Length -Sum).Sum
    $totalSizeMB = [math]::Round($totalSize / 1MB, 2)
    $totalSizeGB = [math]::Round($totalSize / 1GB, 2)

    Write-PerfLog "总资产大小: $totalSizeGB GB ($totalSizeMB MB)" "INFO"
    $Script:PerfResults.Metrics["TotalAssetSizeMB"] = $totalSizeMB

    # 内存预算检查
    $memoryBudgets = @{
        "Low" = 2048
        "Medium" = 4096
        "High" = 6144
        "Ultra" = 8192
    }

    $budget = $memoryBudgets[$TargetQuality]
    Write-PerfLog "目标质量: $TargetQuality (预算: $budget MB)" "INFO"

    if ($totalSizeMB -gt $budget) {
        Write-PerfLog "资产大小超出内存预算" "WARNING"
        $Script:PerfResults.Warnings += "内存使用超出预算 ($totalSizeMB MB > $budget MB)"
    } else {
        Write-PerfLog "内存使用在预算内" "SUCCESS"
        $Script:PerfResults.Passed += "内存使用合理"
    }

    # 检查单个大文件
    $largeFiles = $allAssets | Where-Object { $_.Length -gt 100MB }
    if ($largeFiles.Count -gt 0) {
        Write-PerfLog "发现 $($largeFiles.Count) 个超大文件 (>100MB)" "WARNING"
        foreach ($file in $largeFiles) {
            Write-PerfLog "  - $($file.Name): $([math]::Round($file.Length/1MB, 2)) MB" "WARNING"
        }
        $Script:PerfResults.Warnings += "$($largeFiles.Count) 个超大文件"
    }
}

function Test-LODConfiguration {
    Write-Host "`n=== LOD配置检查 ===" -ForegroundColor Cyan

    $engineIni = "$ProjectRoot/HorrorProject/Config/DefaultEngine.ini"
    if (-not (Test-Path $engineIni)) { return }

    $content = Get-Content $engineIni -Raw

    # 检查LOD设置
    $lodSettings = @{
        "LOD距离缩放" = "r\.LODDistanceScale"
        "最小LOD大小" = "r\.MinScreenRadiusForLights"
        "自动LOD" = "r\.AutoLOD"
    }

    $lodConfigured = 0
    foreach ($setting in $lodSettings.Keys) {
        if ($content -match [regex]::Escape($lodSettings[$setting])) {
            Write-PerfLog "$setting : 已配置" "SUCCESS"
            $Script:PerfResults.Passed += "$setting 配置"
            $lodConfigured++
        } else {
            Write-PerfLog "$setting : 未配置" "WARNING"
        }
    }

    if ($lodConfigured -eq 0) {
        Write-PerfLog "LOD系统未配置" "WARNING"
        $Script:PerfResults.Warnings += "LOD系统未配置"
    }
}

function Test-StreamingConfiguration {
    Write-Host "`n=== 流送配置检查 ===" -ForegroundColor Cyan

    $engineIni = "$ProjectRoot/HorrorProject/Config/DefaultEngine.ini"
    if (-not (Test-Path $engineIni)) { return }

    $content = Get-Content $engineIni -Raw

    # 检查纹理流送
    if ($content -match "r\.Streaming\.PoolSize=(\d+)") {
        $poolSize = [int]$matches[1]
        Write-PerfLog "纹理流送池: $poolSize MB" "INFO"
        $Script:PerfResults.Metrics["StreamingPoolMB"] = $poolSize

        if ($poolSize -lt 1000) {
            Write-PerfLog "纹理流送池较小，可能影响性能" "WARNING"
            $Script:PerfResults.Warnings += "纹理流送池配置过小"
        } else {
            Write-PerfLog "纹理流送池配置合理" "SUCCESS"
            $Script:PerfResults.Passed += "纹理流送配置正确"
        }
    } else {
        Write-PerfLog "未配置纹理流送池" "WARNING"
        $Script:PerfResults.Warnings += "缺少纹理流送配置"
    }

    # 检查关卡流送
    if ($content -match "s\.LevelStreamingActorsUpdateTimeLimit") {
        Write-PerfLog "关卡流送: 已配置" "SUCCESS"
        $Script:PerfResults.Passed += "关卡流送配置"
    } else {
        Write-PerfLog "关卡流送: 未配置" "WARNING"
    }
}

function Test-DrawCallOptimization {
    Write-Host "`n=== 绘制调用优化检查 ===" -ForegroundColor Cyan

    $engineIni = "$ProjectRoot/HorrorProject/Config/DefaultEngine.ini"
    if (-not (Test-Path $engineIni)) { return }

    $content = Get-Content $engineIni -Raw

    # 检查实例化
    if ($content -match "r\.StaticMeshLODDistanceScale") {
        Write-PerfLog "静态网格LOD: 已配置" "SUCCESS"
        $Script:PerfResults.Passed += "静态网格LOD配置"
    }

    # 检查遮挡剔除
    if ($content -match "r\.AllowOcclusionQueries=1") {
        Write-PerfLog "遮挡剔除: 启用" "SUCCESS"
        $Script:PerfResults.Passed += "遮挡剔除启用"
    } else {
        Write-PerfLog "遮挡剔除: 未启用" "WARNING"
        $Script:PerfResults.Warnings += "遮挡剔除未启用"
    }

    # 检查距离剔除
    if ($content -match "r\.CullDistanceQuality") {
        Write-PerfLog "距离剔除: 已配置" "SUCCESS"
        $Script:PerfResults.Passed += "距离剔除配置"
    }
}

function Test-ParticlePerformance {
    Write-Host "`n=== 粒子性能检查 ===" -ForegroundColor Cyan

    $engineIni = "$ProjectRoot/HorrorProject/Config/DefaultEngine.ini"
    if (-not (Test-Path $engineIni)) { return }

    $content = Get-Content $engineIni -Raw

    # 检查粒子预算
    if ($content -match "fx\.MaxCPUParticlesPerEmitter=(\d+)") {
        $maxParticles = [int]$matches[1]
        Write-PerfLog "最大CPU粒子数: $maxParticles" "INFO"
        $Script:PerfResults.Metrics["MaxCPUParticles"] = $maxParticles

        if ($maxParticles -gt 1000) {
            Write-PerfLog "CPU粒子数较高，可能影响性能" "WARNING"
            $Script:PerfResults.Warnings += "CPU粒子预算过高"
        } else {
            Write-PerfLog "CPU粒子预算合理" "SUCCESS"
            $Script:PerfResults.Passed += "粒子预算配置合理"
        }
    }

    # 检查GPU粒子
    if ($content -match "fx\.AllowGPUParticles=1") {
        Write-PerfLog "GPU粒子: 启用" "SUCCESS"
        $Script:PerfResults.Passed += "GPU粒子启用"
    } else {
        Write-PerfLog "GPU粒子: 未启用（建议启用以提升性能）" "WARNING"
    }
}

function Test-AudioPerformance {
    Write-Host "`n=== 音频性能检查 ===" -ForegroundColor Cyan

    $audioPath = "$ProjectRoot/HorrorProject/Content/Audio"
    if (-not (Test-Path $audioPath)) {
        Write-PerfLog "音频目录不存在" "WARNING"
        return
    }

    $audioFiles = Get-ChildItem -Path $audioPath -Recurse -Include *.wav,*.mp3,*.ogg
    $totalAudioSize = ($audioFiles | Measure-Object -Property Length -Sum).Sum
    $totalAudioSizeMB = [math]::Round($totalAudioSize / 1MB, 2)

    Write-PerfLog "音频文件数: $($audioFiles.Count)" "INFO"
    Write-PerfLog "音频总大小: $totalAudioSizeMB MB" "INFO"
    $Script:PerfResults.Metrics["AudioCount"] = $audioFiles.Count
    $Script:PerfResults.Metrics["AudioSizeMB"] = $totalAudioSizeMB

    # 检查未压缩的WAV文件
    $wavFiles = $audioFiles | Where-Object { $_.Extension -eq ".wav" }
    if ($wavFiles.Count -gt 0) {
        $wavSize = ($wavFiles | Measure-Object -Property Length -Sum).Sum
        $wavSizeMB = [math]::Round($wavSize / 1MB, 2)
        Write-PerfLog "未压缩WAV文件: $($wavFiles.Count) 个 ($wavSizeMB MB)" "WARNING"
        $Script:PerfResults.Warnings += "存在未压缩的WAV文件，建议转换为OGG"
    }

    if ($totalAudioSizeMB -gt 500) {
        Write-PerfLog "音频资产较大，建议优化压缩" "WARNING"
        $Script:PerfResults.Warnings += "音频资产大小过大"
    } else {
        Write-PerfLog "音频资产大小合理" "SUCCESS"
        $Script:PerfResults.Passed += "音频资产优化良好"
    }
}

function Calculate-PerformanceScore {
    $totalChecks = $Script:PerfResults.Passed.Count +
                   $Script:PerfResults.Warnings.Count +
                   $Script:PerfResults.Issues.Count

    if ($totalChecks -eq 0) { return 0 }

    $passedWeight = 10
    $warningPenalty = 3
    $issuePenalty = 10

    $score = ($Script:PerfResults.Passed.Count * $passedWeight) -
             ($Script:PerfResults.Warnings.Count * $warningPenalty) -
             ($Script:PerfResults.Issues.Count * $issuePenalty)

    $maxScore = $totalChecks * $passedWeight
    $percentage = [math]::Max(0, [math]::Min(100, ($score / $maxScore) * 100))

    return [math]::Round($percentage, 2)
}

function Generate-PerformanceReport {
    Write-Host "`n=== 生成性能报告 ===" -ForegroundColor Cyan

    if (-not (Test-Path $ReportPath)) {
        New-Item -ItemType Directory -Path $ReportPath -Force | Out-Null
    }

    $score = Calculate-PerformanceScore

    $report = @{
        Timestamp = Get-Date -Format "yyyy-MM-dd HH:mm:ss"
        ProjectRoot = $ProjectRoot
        TargetQuality = $TargetQuality
        TargetFPS = $TargetFPS
        Score = $score
        Grade = if ($score -ge 90) { "优秀" } elseif ($score -ge 75) { "良好" } elseif ($score -ge 60) { "及格" } else { "需改进" }
        Metrics = $Script:PerfResults.Metrics
        Summary = @{
            PassedChecks = $Script:PerfResults.Passed.Count
            Warnings = $Script:PerfResults.Warnings.Count
            Issues = $Script:PerfResults.Issues.Count
        }
        Details = @{
            Passed = $Script:PerfResults.Passed
            Warnings = $Script:PerfResults.Warnings
            Issues = $Script:PerfResults.Issues
        }
        Recommendations = Get-PerformanceRecommendations
    }

    $reportFile = Join-Path $ReportPath "Performance_$(Get-Date -Format 'yyyyMMdd_HHmmss').json"
    $report | ConvertTo-Json -Depth 10 | Out-File -FilePath $reportFile -Encoding UTF8

    Write-PerfLog "报告已保存: $reportFile" "SUCCESS"
    return $report
}

function Get-PerformanceRecommendations {
    $recommendations = @()

    if ($Script:PerfResults.Warnings -contains "纹理流送池配置过小") {
        $recommendations += "增加纹理流送池大小到至少1000MB"
    }

    if ($Script:PerfResults.Warnings -match "大纹理") {
        $recommendations += "压缩或降低大纹理的分辨率"
    }

    if ($Script:PerfResults.Warnings -contains "LOD系统未配置") {
        $recommendations += "配置LOD系统以提升远距离渲染性能"
    }

    if ($Script:PerfResults.Warnings -contains "遮挡剔除未启用") {
        $recommendations += "启用遮挡剔除以减少不可见物体的渲染"
    }

    if ($Script:PerfResults.Warnings -match "WAV文件") {
        $recommendations += "将WAV音频文件转换为OGG格式以减小体积"
    }

    if ($recommendations.Count -eq 0) {
        $recommendations += "性能配置良好，继续保持"
    }

    return $recommendations
}

function Show-PerformanceSummary {
    param($Report)

    Write-Host "`n========================================" -ForegroundColor Cyan
    Write-Host "性能验证摘要" -ForegroundColor Cyan
    Write-Host "========================================" -ForegroundColor Cyan
    Write-Host "目标质量: $($Report.TargetQuality)" -ForegroundColor White
    Write-Host "目标帧率: $($Report.TargetFPS) FPS" -ForegroundColor White
    Write-Host "性能评分: $($Report.Score)/100" -ForegroundColor White
    Write-Host "评级: $($Report.Grade)" -ForegroundColor $(if($Report.Grade -eq "优秀"){"Green"}elseif($Report.Grade -eq "良好"){"Cyan"}elseif($Report.Grade -eq "及格"){"Yellow"}else{"Red"})
    Write-Host "`n检查结果:" -ForegroundColor White
    Write-Host "  通过: $($Report.Summary.PassedChecks)" -ForegroundColor Green
    Write-Host "  警告: $($Report.Summary.Warnings)" -ForegroundColor Yellow
    Write-Host "  问题: $($Report.Summary.Issues)" -ForegroundColor Red

    if ($Report.Recommendations.Count -gt 0) {
        Write-Host "`n优化建议:" -ForegroundColor Cyan
        foreach ($rec in $Report.Recommendations) {
            Write-Host "  - $rec" -ForegroundColor White
        }
    }

    Write-Host "========================================`n" -ForegroundColor Cyan
}

# 主执行流程
Write-Host "`n开始性能验证...`n" -ForegroundColor Cyan

Test-RenderingPerformance
Test-AssetPerformance
Test-MemoryUsage
Test-LODConfiguration
Test-StreamingConfiguration
Test-DrawCallOptimization
Test-ParticlePerformance
Test-AudioPerformance

$report = Generate-PerformanceReport
Show-PerformanceSummary -Report $report

exit 0
