# VerifyBuildConfigs.ps1
# 验证构建配置

param(
    [string]$ProjectRoot = "D:/gptzuo/HorrorProject",
    [string]$ReportPath = "D:/gptzuo/HorrorProject/Reports/BuildConfig",
    [ValidateSet("Development", "Shipping", "Debug", "Test", "All")]
    [string]$Configuration = "All"
)

$ErrorActionPreference = "Continue"
$Script:ConfigIssues = @()
$Script:ConfigWarnings = @()
$Script:ConfigPassed = @()

function Write-ConfigLog {
    param([string]$Message, [string]$Type = "INFO")
    $color = switch($Type) {
        "ERROR" { "Red" }
        "WARNING" { "Yellow" }
        "SUCCESS" { "Green" }
        default { "White" }
    }
    Write-Host "[$(Get-Date -Format 'HH:mm:ss')] $Message" -ForegroundColor $color
}

function Test-ProjectFile {
    Write-ConfigLog "检查项目文件..." "INFO"

    $uprojectPath = "$ProjectRoot/HorrorProject/HorrorProject.uproject"
    if (-not (Test-Path $uprojectPath)) {
        Write-ConfigLog "项目文件不存在: $uprojectPath" "ERROR"
        $Script:ConfigIssues += "项目文件缺失"
        return $false
    }

    try {
        $uproject = Get-Content $uprojectPath -Raw | ConvertFrom-Json

        # 检查必需字段
        $requiredFields = @("FileVersion", "EngineAssociation", "Category", "Description")
        foreach ($field in $requiredFields) {
            if ($uproject.PSObject.Properties.Name -contains $field) {
                Write-ConfigLog "项目字段 '$field' 存在" "SUCCESS"
                $Script:ConfigPassed += "项目字段 $field"
            } else {
                Write-ConfigLog "项目字段 '$field' 缺失" "WARNING"
                $Script:ConfigWarnings += "项目字段 $field 缺失"
            }
        }

        # 检查模块配置
        if ($uproject.Modules) {
            Write-ConfigLog "模块数量: $($uproject.Modules.Count)" "SUCCESS"
            foreach ($module in $uproject.Modules) {
                Write-ConfigLog "  - $($module.Name) ($($module.Type))" "INFO"
            }
            $Script:ConfigPassed += "模块配置"
        } else {
            Write-ConfigLog "没有配置模块" "ERROR"
            $Script:ConfigIssues += "缺少模块配置"
        }

        # 检查插件配置
        if ($uproject.Plugins) {
            Write-ConfigLog "插件数量: $($uproject.Plugins.Count)" "SUCCESS"
            $Script:ConfigPassed += "插件配置"
        }

        return $true
    }
    catch {
        Write-ConfigLog "解析项目文件失败: $_" "ERROR"
        $Script:ConfigIssues += "项目文件解析失败"
        return $false
    }
}

function Test-EngineConfig {
    Write-ConfigLog "检查引擎配置..." "INFO"

    $engineIniPath = "$ProjectRoot/HorrorProject/Config/DefaultEngine.ini"
    if (-not (Test-Path $engineIniPath)) {
        Write-ConfigLog "引擎配置文件不存在" "ERROR"
        $Script:ConfigIssues += "引擎配置文件缺失"
        return
    }

    $content = Get-Content $engineIniPath -Raw

    # 检查关键配置项
    $criticalSettings = @{
        "渲染设置" = @("r.DefaultFeature", "r.Shadow", "r.PostProcessAAQuality")
        "性能设置" = @("t.MaxFPS", "r.Streaming", "r.LODDistanceScale")
        "音频设置" = @("Audio.DefaultSoundClassName", "Audio.MaxChannels")
        "网络设置" = @("NetServerMaxTickRate", "MaxClientRate")
    }

    foreach ($category in $criticalSettings.Keys) {
        $found = 0
        foreach ($setting in $criticalSettings[$category]) {
            if ($content -match [regex]::Escape($setting)) {
                $found++
            }
        }

        if ($found -gt 0) {
            Write-ConfigLog "$category : $found/$($criticalSettings[$category].Count) 配置项" "SUCCESS"
            $Script:ConfigPassed += $category
        } else {
            Write-ConfigLog "$category : 未配置" "WARNING"
            $Script:ConfigWarnings += "$category 未配置"
        }
    }
}

function Test-GameConfig {
    Write-ConfigLog "检查游戏配置..." "INFO"

    $gameIniPath = "$ProjectRoot/HorrorProject/Config/DefaultGame.ini"
    if (-not (Test-Path $gameIniPath)) {
        Write-ConfigLog "游戏配置文件不存在" "ERROR"
        $Script:ConfigIssues += "游戏配置文件缺失"
        return
    }

    $content = Get-Content $gameIniPath -Raw

    # 检查项目设置
    $projectSettings = @(
        "ProjectName",
        "ProjectVersion",
        "CompanyName",
        "CopyrightNotice"
    )

    foreach ($setting in $projectSettings) {
        if ($content -match [regex]::Escape($setting)) {
            Write-ConfigLog "游戏设置 '$setting' 已配置" "SUCCESS"
            $Script:ConfigPassed += "游戏设置 $setting"
        } else {
            Write-ConfigLog "游戏设置 '$setting' 未配置" "WARNING"
            $Script:ConfigWarnings += "游戏设置 $setting 未配置"
        }
    }
}

function Test-InputConfig {
    Write-ConfigLog "检查输入配置..." "INFO"

    $inputIniPath = "$ProjectRoot/HorrorProject/Config/DefaultInput.ini"
    if (-not (Test-Path $inputIniPath)) {
        Write-ConfigLog "输入配置文件不存在" "ERROR"
        $Script:ConfigIssues += "输入配置文件缺失"
        return
    }

    $content = Get-Content $inputIniPath -Raw

    # 检查输入绑定
    $inputBindings = @(
        "ActionMappings",
        "AxisMappings"
    )

    foreach ($binding in $inputBindings) {
        if ($content -match [regex]::Escape($binding)) {
            Write-ConfigLog "输入绑定 '$binding' 已配置" "SUCCESS"
            $Script:ConfigPassed += "输入绑定 $binding"
        } else {
            Write-ConfigLog "输入绑定 '$binding' 未配置" "WARNING"
            $Script:ConfigWarnings += "输入绑定 $binding 未配置"
        }
    }
}

function Test-BuildTargets {
    Write-ConfigLog "检查构建目标..." "INFO"

    $sourcePath = "$ProjectRoot/HorrorProject/Source"
    if (-not (Test-Path $sourcePath)) {
        Write-ConfigLog "Source目录不存在" "ERROR"
        $Script:ConfigIssues += "Source目录缺失"
        return
    }

    $targetFiles = Get-ChildItem -Path $sourcePath -Filter "*.Target.cs" -Recurse

    if ($targetFiles.Count -eq 0) {
        Write-ConfigLog "没有找到构建目标文件" "ERROR"
        $Script:ConfigIssues += "缺少构建目标文件"
    } else {
        Write-ConfigLog "找到 $($targetFiles.Count) 个构建目标" "SUCCESS"
        foreach ($target in $targetFiles) {
            Write-ConfigLog "  - $($target.Name)" "INFO"
        }
        $Script:ConfigPassed += "构建目标配置"
    }
}

function Test-ModuleRules {
    Write-ConfigLog "检查模块规则..." "INFO"

    $sourcePath = "$ProjectRoot/HorrorProject/Source"
    if (-not (Test-Path $sourcePath)) { return }

    $buildFiles = Get-ChildItem -Path $sourcePath -Filter "*.Build.cs" -Recurse

    if ($buildFiles.Count -eq 0) {
        Write-ConfigLog "没有找到模块构建文件" "ERROR"
        $Script:ConfigIssues += "缺少模块构建文件"
    } else {
        Write-ConfigLog "找到 $($buildFiles.Count) 个模块构建文件" "SUCCESS"

        foreach ($buildFile in $buildFiles) {
            $content = Get-Content $buildFile.FullName -Raw

            # 检查关键依赖
            $dependencies = @("Core", "CoreUObject", "Engine")
            $foundDeps = 0
            foreach ($dep in $dependencies) {
                if ($content -match [regex]::Escape($dep)) {
                    $foundDeps++
                }
            }

            if ($foundDeps -gt 0) {
                Write-ConfigLog "  - $($buildFile.Name): $foundDeps 个核心依赖" "SUCCESS"
            }
        }

        $Script:ConfigPassed += "模块规则配置"
    }
}

function Test-PlatformConfigs {
    Write-ConfigLog "检查平台配置..." "INFO"

    $configPath = "$ProjectRoot/HorrorProject/Config"
    if (-not (Test-Path $configPath)) { return }

    $platforms = @("Windows", "Linux", "Mac", "Android", "iOS")
    $foundPlatforms = 0

    foreach ($platform in $platforms) {
        $platformConfig = Join-Path $configPath "$platform"
        if (Test-Path $platformConfig) {
            Write-ConfigLog "平台配置 '$platform' 存在" "SUCCESS"
            $foundPlatforms++
        }
    }

    if ($foundPlatforms -gt 0) {
        Write-ConfigLog "找到 $foundPlatforms 个平台配置" "SUCCESS"
        $Script:ConfigPassed += "平台配置"
    } else {
        Write-ConfigLog "没有找到平台特定配置" "WARNING"
        $Script:ConfigWarnings += "缺少平台特定配置"
    }
}

function Test-PackagingSettings {
    Write-ConfigLog "检查打包设置..." "INFO"

    $engineIniPath = "$ProjectRoot/HorrorProject/Config/DefaultEngine.ini"
    if (-not (Test-Path $engineIniPath)) { return }

    $content = Get-Content $engineIniPath -Raw

    $packagingSettings = @(
        "UsePakFile",
        "bCompressed",
        "bGenerateChunks"
    )

    $foundSettings = 0
    foreach ($setting in $packagingSettings) {
        if ($content -match [regex]::Escape($setting)) {
            Write-ConfigLog "打包设置 '$setting' 已配置" "SUCCESS"
            $foundSettings++
        }
    }

    if ($foundSettings -gt 0) {
        $Script:ConfigPassed += "打包设置"
    } else {
        Write-ConfigLog "打包设置未配置" "WARNING"
        $Script:ConfigWarnings += "打包设置未配置"
    }
}

function Generate-ConfigReport {
    Write-ConfigLog "生成配置报告..." "INFO"

    if (-not (Test-Path $ReportPath)) {
        New-Item -ItemType Directory -Path $ReportPath -Force | Out-Null
    }

    $reportFile = Join-Path $ReportPath "BuildConfig_$(Get-Date -Format 'yyyyMMdd_HHmmss').json"

    $report = @{
        Timestamp = Get-Date -Format "yyyy-MM-dd HH:mm:ss"
        ProjectRoot = $ProjectRoot
        Configuration = $Configuration
        Summary = @{
            TotalChecks = $Script:ConfigPassed.Count + $Script:ConfigWarnings.Count + $Script:ConfigIssues.Count
            Passed = $Script:ConfigPassed.Count
            Warnings = $Script:ConfigWarnings.Count
            Issues = $Script:ConfigIssues.Count
        }
        Passed = $Script:ConfigPassed
        Warnings = $Script:ConfigWarnings
        Issues = $Script:ConfigIssues
        Status = if ($Script:ConfigIssues.Count -eq 0) { "VALID" } elseif ($Script:ConfigIssues.Count -lt 3) { "NEEDS_FIX" } else { "INVALID" }
    }

    $report | ConvertTo-Json -Depth 10 | Out-File -FilePath $reportFile -Encoding UTF8

    Write-ConfigLog "报告已保存: $reportFile" "SUCCESS"
    return $report
}

function Show-ConfigSummary {
    param($Report)

    Write-Host "`n========================================" -ForegroundColor Cyan
    Write-Host "构建配置验证摘要" -ForegroundColor Cyan
    Write-Host "========================================" -ForegroundColor Cyan
    Write-Host "配置类型: $($Report.Configuration)" -ForegroundColor White
    Write-Host "通过检查: $($Report.Summary.Passed)" -ForegroundColor Green
    Write-Host "警告: $($Report.Summary.Warnings)" -ForegroundColor Yellow
    Write-Host "问题: $($Report.Summary.Issues)" -ForegroundColor Red
    Write-Host "状态: $($Report.Status)" -ForegroundColor $(if($Report.Status -eq "VALID"){"Green"}elseif($Report.Status -eq "NEEDS_FIX"){"Yellow"}else{"Red"})
    Write-Host "========================================`n" -ForegroundColor Cyan
}

# 主执行流程
Write-Host "`n开始构建配置验证...`n" -ForegroundColor Cyan

Test-ProjectFile
Test-EngineConfig
Test-GameConfig
Test-InputConfig
Test-BuildTargets
Test-ModuleRules
Test-PlatformConfigs
Test-PackagingSettings

$report = Generate-ConfigReport
Show-ConfigSummary -Report $report

exit 0
