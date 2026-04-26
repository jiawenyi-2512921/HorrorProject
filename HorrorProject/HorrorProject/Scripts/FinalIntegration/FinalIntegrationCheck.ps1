# FinalIntegrationCheck.ps1
# 最终集成检查脚本

param(
    [string]$ProjectRoot = "D:/gptzuo/HorrorProject",
    [string]$ReportPath = "D:/gptzuo/HorrorProject/Reports/FinalIntegration",
    [switch]$Verbose,
    [switch]$FixIssues
)

$ErrorActionPreference = "Continue"
$Script:Issues = @()
$Script:Warnings = @()
$Script:Passed = @()

function Write-Status {
    param([string]$Message, [string]$Type = "INFO")
    $timestamp = Get-Date -Format "yyyy-MM-dd HH:mm:ss"
    $color = switch($Type) {
        "ERROR" { "Red" }
        "WARNING" { "Yellow" }
        "SUCCESS" { "Green" }
        default { "White" }
    }
    Write-Host "[$timestamp] [$Type] $Message" -ForegroundColor $color
}

function Test-SystemIntegration {
    Write-Status "检查系统集成..." "INFO"

    $systems = @(
        @{Name="AI系统"; Path="$ProjectRoot/HorrorProject/Source/HorrorProject/AI"},
        @{Name="音频系统"; Path="$ProjectRoot/HorrorProject/Source/HorrorProject/Audio"},
        @{Name="UI系统"; Path="$ProjectRoot/HorrorProject/Source/HorrorProject/UI"},
        @{Name="游戏机制"; Path="$ProjectRoot/HorrorProject/Source/HorrorProject/Gameplay"},
        @{Name="保存系统"; Path="$ProjectRoot/HorrorProject/Source/HorrorProject/SaveSystem"},
        @{Name="性能系统"; Path="$ProjectRoot/HorrorProject/Source/HorrorProject/Performance"}
    )

    foreach ($system in $systems) {
        if (Test-Path $system.Path) {
            $files = Get-ChildItem -Path $system.Path -Recurse -Include *.cpp,*.h
            if ($files.Count -gt 0) {
                Write-Status "$($system.Name): $($files.Count) 个文件" "SUCCESS"
                $Script:Passed += "$($system.Name)集成检查通过"
            } else {
                Write-Status "$($system.Name): 没有找到源文件" "WARNING"
                $Script:Warnings += "$($system.Name)缺少源文件"
            }
        } else {
            Write-Status "$($system.Name): 路径不存在" "ERROR"
            $Script:Issues += "$($system.Name)路径不存在: $($system.Path)"
        }
    }
}

function Test-AssetIntegrity {
    Write-Status "检查资产完整性..." "INFO"

    $assetPaths = @(
        @{Name="内容"; Path="$ProjectRoot/HorrorProject/Content"},
        @{Name="蓝图"; Path="$ProjectRoot/HorrorProject/Content/Blueprints"},
        @{Name="材质"; Path="$ProjectRoot/HorrorProject/Content/Materials"},
        @{Name="音频"; Path="$ProjectRoot/HorrorProject/Content/Audio"},
        @{Name="UI"; Path="$ProjectRoot/HorrorProject/Content/UI"}
    )

    foreach ($asset in $assetPaths) {
        if (Test-Path $asset.Path) {
            $files = Get-ChildItem -Path $asset.Path -Recurse -File
            Write-Status "$($asset.Name): $($files.Count) 个资产文件" "SUCCESS"
            $Script:Passed += "$($asset.Name)资产完整性检查通过"
        } else {
            Write-Status "$($asset.Name): 路径不存在" "WARNING"
            $Script:Warnings += "$($asset.Name)资产路径不存在"
        }
    }
}

function Test-BuildConfiguration {
    Write-Status "检查构建配置..." "INFO"

    $configs = @(
        @{Name="项目文件"; Path="$ProjectRoot/HorrorProject/HorrorProject.uproject"},
        @{Name="构建配置"; Path="$ProjectRoot/HorrorProject/Config/DefaultEngine.ini"},
        @{Name="游戏配置"; Path="$ProjectRoot/HorrorProject/Config/DefaultGame.ini"},
        @{Name="输入配置"; Path="$ProjectRoot/HorrorProject/Config/DefaultInput.ini"}
    )

    foreach ($config in $configs) {
        if (Test-Path $config.Path) {
            $content = Get-Content $config.Path -Raw
            if ($content.Length -gt 0) {
                Write-Status "$($config.Name): 配置有效" "SUCCESS"
                $Script:Passed += "$($config.Name)配置检查通过"
            } else {
                Write-Status "$($config.Name): 配置为空" "ERROR"
                $Script:Issues += "$($config.Name)配置文件为空"
            }
        } else {
            Write-Status "$($config.Name): 文件不存在" "ERROR"
            $Script:Issues += "$($config.Name)不存在: $($config.Path)"
        }
    }
}

function Test-CodeQuality {
    Write-Status "检查代码质量..." "INFO"

    $sourceFiles = Get-ChildItem -Path "$ProjectRoot/HorrorProject/Source" -Recurse -Include *.cpp,*.h

    $totalLines = 0
    $filesWithTODO = 0
    $filesWithFIXME = 0

    foreach ($file in $sourceFiles) {
        $content = Get-Content $file.FullName -Raw
        $totalLines += ($content -split "`n").Count

        if ($content -match "TODO") { $filesWithTODO++ }
        if ($content -match "FIXME") { $filesWithFIXME++ }
    }

    Write-Status "代码统计: $($sourceFiles.Count) 个文件, $totalLines 行代码" "INFO"

    if ($filesWithTODO -gt 0) {
        Write-Status "发现 $filesWithTODO 个文件包含 TODO" "WARNING"
        $Script:Warnings += "$filesWithTODO 个文件包含未完成的 TODO"
    }

    if ($filesWithFIXME -gt 0) {
        Write-Status "发现 $filesWithFIXME 个文件包含 FIXME" "WARNING"
        $Script:Warnings += "$filesWithFIXME 个文件包含需要修复的 FIXME"
    }

    $Script:Passed += "代码质量检查完成: $($sourceFiles.Count) 个文件"
}

function Test-Dependencies {
    Write-Status "检查依赖关系..." "INFO"

    $uprojectPath = "$ProjectRoot/HorrorProject/HorrorProject.uproject"
    if (Test-Path $uprojectPath) {
        $uproject = Get-Content $uprojectPath -Raw | ConvertFrom-Json

        if ($uproject.Modules) {
            Write-Status "模块数量: $($uproject.Modules.Count)" "SUCCESS"
            $Script:Passed += "模块依赖检查通过"
        }

        if ($uproject.Plugins) {
            Write-Status "插件数量: $($uproject.Plugins.Count)" "SUCCESS"
            $Script:Passed += "插件依赖检查通过"
        }
    }
}

function Test-PerformanceMetrics {
    Write-Status "检查性能指标..." "INFO"

    # 检查性能配置
    $engineIni = "$ProjectRoot/HorrorProject/Config/DefaultEngine.ini"
    if (Test-Path $engineIni) {
        $content = Get-Content $engineIni -Raw

        $perfChecks = @(
            @{Name="帧率限制"; Pattern="t.MaxFPS"},
            @{Name="LOD设置"; Pattern="r.LODDistanceScale"},
            @{Name="阴影质量"; Pattern="r.Shadow"},
            @{Name="抗锯齿"; Pattern="r.PostProcessAAQuality"}
        )

        foreach ($check in $perfChecks) {
            if ($content -match $check.Pattern) {
                Write-Status "$($check.Name): 已配置" "SUCCESS"
                $Script:Passed += "$($check.Name)配置检查通过"
            } else {
                Write-Status "$($check.Name): 未配置" "WARNING"
                $Script:Warnings += "$($check.Name)未在配置中找到"
            }
        }
    }
}

function Test-TestCoverage {
    Write-Status "检查测试覆盖率..." "INFO"

    $testPath = "$ProjectRoot/HorrorProject/Source/HorrorProject/Tests"
    if (Test-Path $testPath) {
        $testFiles = Get-ChildItem -Path $testPath -Recurse -Include *.cpp,*.h
        Write-Status "测试文件数量: $($testFiles.Count)" "SUCCESS"
        $Script:Passed += "测试覆盖率检查: $($testFiles.Count) 个测试文件"
    } else {
        Write-Status "测试目录不存在" "WARNING"
        $Script:Warnings += "缺少测试目录"
    }
}

function Generate-Report {
    Write-Status "生成集成报告..." "INFO"

    if (-not (Test-Path $ReportPath)) {
        New-Item -ItemType Directory -Path $ReportPath -Force | Out-Null
    }

    $reportFile = Join-Path $ReportPath "FinalIntegration_$(Get-Date -Format 'yyyyMMdd_HHmmss').json"

    $report = @{
        Timestamp = Get-Date -Format "yyyy-MM-dd HH:mm:ss"
        ProjectRoot = $ProjectRoot
        Summary = @{
            TotalChecks = $Script:Passed.Count + $Script:Warnings.Count + $Script:Issues.Count
            Passed = $Script:Passed.Count
            Warnings = $Script:Warnings.Count
            Issues = $Script:Issues.Count
        }
        Passed = $Script:Passed
        Warnings = $Script:Warnings
        Issues = $Script:Issues
        Status = if ($Script:Issues.Count -eq 0) { "READY" } elseif ($Script:Issues.Count -lt 5) { "NEEDS_ATTENTION" } else { "NOT_READY" }
    }

    $report | ConvertTo-Json -Depth 10 | Out-File -FilePath $reportFile -Encoding UTF8

    Write-Status "报告已保存: $reportFile" "SUCCESS"
    return $report
}

function Show-Summary {
    param($Report)

    Write-Host "`n========================================" -ForegroundColor Cyan
    Write-Host "最终集成检查摘要" -ForegroundColor Cyan
    Write-Host "========================================" -ForegroundColor Cyan
    Write-Host "总检查项: $($Report.Summary.TotalChecks)" -ForegroundColor White
    Write-Host "通过: $($Report.Summary.Passed)" -ForegroundColor Green
    Write-Host "警告: $($Report.Summary.Warnings)" -ForegroundColor Yellow
    Write-Host "问题: $($Report.Summary.Issues)" -ForegroundColor Red
    Write-Host "状态: $($Report.Status)" -ForegroundColor $(if($Report.Status -eq "READY"){"Green"}elseif($Report.Status -eq "NEEDS_ATTENTION"){"Yellow"}else{"Red"})
    Write-Host "========================================`n" -ForegroundColor Cyan
}

# 主执行流程
Write-Host "`n开始最终集成检查...`n" -ForegroundColor Cyan

Test-SystemIntegration
Test-AssetIntegrity
Test-BuildConfiguration
Test-CodeQuality
Test-Dependencies
Test-PerformanceMetrics
Test-TestCoverage

$report = Generate-Report
Show-Summary -Report $report

if ($report.Status -eq "READY") {
    Write-Status "项目已准备好发布!" "SUCCESS"
    exit 0
} elseif ($report.Status -eq "NEEDS_ATTENTION") {
    Write-Status "项目需要注意一些问题" "WARNING"
    exit 1
} else {
    Write-Status "项目尚未准备好发布" "ERROR"
    exit 2
}
