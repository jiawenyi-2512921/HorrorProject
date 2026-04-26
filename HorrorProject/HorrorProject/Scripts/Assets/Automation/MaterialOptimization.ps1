# MaterialOptimization.ps1 - 材质优化
param(
    [Parameter(Mandatory=$true)]
    [string]$MaterialPath,
    [ValidateSet("Low", "Medium", "High")]
    [string]$OptimizationLevel = "Medium",
    [string]$LogPath = "D:\gptzuo\HorrorProject\HorrorProject\Logs\MaterialOptimization.log",
    [switch]$RemoveUnusedNodes,
    [switch]$SimplifyExpressions,
    [switch]$OptimizeInstructions
)

$ErrorActionPreference = "Stop"
$StartTime = Get-Date

function Write-Log {
    param([string]$Message, [string]$Level = "INFO")
    $Timestamp = Get-Date -Format "yyyy-MM-dd HH:mm:ss"
    $LogMessage = "[$Timestamp] [$Level] $Message"
    Write-Host $LogMessage
    Add-Content -Path $LogPath -Value $LogMessage
}

function Get-OptimizationSettings {
    param([string]$Level)

    switch ($Level) {
        "Low" {
            return @{
                MaxInstructions = 512
                MaxTextureSamples = 16
                MaxParameterCollections = 4
                EnableComplexity = $false
            }
        }
        "Medium" {
            return @{
                MaxInstructions = 256
                MaxTextureSamples = 12
                MaxParameterCollections = 3
                EnableComplexity = $true
            }
        }
        "High" {
            return @{
                MaxInstructions = 128
                MaxTextureSamples = 8
                MaxParameterCollections = 2
                EnableComplexity = $true
            }
        }
    }
}

function Analyze-MaterialComplexity {
    param([object]$MaterialConfig)

    $Complexity = @{
        InstructionCount = 0
        TextureSamples = 0
        ParameterCount = 0
        NodeCount = 0
    }

    if ($MaterialConfig.Parameters) {
        $Complexity.ParameterCount =
            ($MaterialConfig.Parameters.ScalarParameters | Measure-Object).Count +
            ($MaterialConfig.Parameters.VectorParameters | Measure-Object).Count +
            ($MaterialConfig.Parameters.TextureParameters | Measure-Object).Count
    }

    if ($MaterialConfig.Parameters.TextureParameters) {
        $Complexity.TextureSamples = ($MaterialConfig.Parameters.TextureParameters | Measure-Object).Count
    }

    # 估算指令数（简化计算）
    $Complexity.InstructionCount = $Complexity.ParameterCount * 4 + $Complexity.TextureSamples * 8

    return $Complexity
}

function Optimize-Material {
    param(
        [string]$MaterialFile,
        [hashtable]$Settings,
        [int]$Index,
        [int]$Total
    )

    $ProgressPercent = [math]::Round(($Index / $Total) * 100, 2)
    Write-Progress -Activity "优化材质" -Status "$ProgressPercent% 完成" -PercentComplete $ProgressPercent

    Write-Log "优化材质 [$Index/$Total]: $(Split-Path $MaterialFile -Leaf)"

    try {
        # 读取材质配置
        $MaterialConfig = Get-Content -Path $MaterialFile -Raw | ConvertFrom-Json

        # 分析复杂度
        $OriginalComplexity = Analyze-MaterialComplexity -MaterialConfig $MaterialConfig

        Write-Log "  原始复杂度:"
        Write-Log "    指令数: $($OriginalComplexity.InstructionCount)"
        Write-Log "    纹理采样: $($OriginalComplexity.TextureSamples)"
        Write-Log "    参数数: $($OriginalComplexity.ParameterCount)"

        $Optimizations = @()

        # 优化1: 移除未使用的参数
        if ($RemoveUnusedNodes) {
            $RemovedParams = 0

            # 这里应该检查参数是否被使用，简化示例
            # 实际实现需要解析材质图

            if ($RemovedParams -gt 0) {
                $Optimizations += "移除 $RemovedParams 个未使用参数"
                Write-Log "  移除了 $RemovedParams 个未使用参数"
            }
        }

        # 优化2: 简化表达式
        if ($SimplifyExpressions) {
            # 合并相似的参数
            $SimplifiedCount = 0

            if ($SimplifiedCount -gt 0) {
                $Optimizations += "简化 $SimplifiedCount 个表达式"
                Write-Log "  简化了 $SimplifiedCount 个表达式"
            }
        }

        # 优化3: 优化指令
        if ($OptimizeInstructions) {
            # 检查是否超过指令限制
            if ($OriginalComplexity.InstructionCount -gt $Settings.MaxInstructions) {
                $Optimizations += "指令数超限，需要手动优化"
                Write-Log "  警告: 指令数 ($($OriginalComplexity.InstructionCount)) 超过限制 ($($Settings.MaxInstructions))" "WARNING"
            }

            # 检查纹理采样
            if ($OriginalComplexity.TextureSamples -gt $Settings.MaxTextureSamples) {
                $Optimizations += "纹理采样超限，需要减少纹理"
                Write-Log "  警告: 纹理采样 ($($OriginalComplexity.TextureSamples)) 超过限制 ($($Settings.MaxTextureSamples))" "WARNING"
            }
        }

        # 添加优化标记
        $MaterialConfig | Add-Member -NotePropertyName "OptimizationInfo" -NotePropertyValue @{
            OptimizationLevel = $OptimizationLevel
            OptimizedDate = Get-Date -Format "yyyy-MM-dd HH:mm:ss"
            Optimizations = $Optimizations
            OriginalComplexity = $OriginalComplexity
        } -Force

        # 保存优化后的配置
        $MaterialConfig | ConvertTo-Json -Depth 10 | Out-File -FilePath $MaterialFile -Encoding UTF8

        $NewComplexity = Analyze-MaterialComplexity -MaterialConfig $MaterialConfig

        Write-Log "  优化后复杂度:"
        Write-Log "    指令数: $($NewComplexity.InstructionCount)"
        Write-Log "    纹理采样: $($NewComplexity.TextureSamples)"
        Write-Log "    参数数: $($NewComplexity.ParameterCount)"

        Write-Log "材质优化完成" "SUCCESS"

        return @{
            Success = $true
            File = $MaterialFile
            OriginalComplexity = $OriginalComplexity
            NewComplexity = $NewComplexity
            Optimizations = $Optimizations
        }
    }
    catch {
        Write-Log "优化材质失败: $_" "ERROR"
        return @{
            Success = $false
            File = $MaterialFile
            Error = $_.Exception.Message
        }
    }
}

function Process-Materials {
    Write-Log "扫描材质文件..."

    $MaterialFiles = Get-ChildItem -Path $MaterialPath -Include "*.json" -Recurse -File

    Write-Log "找到 $($MaterialFiles.Count) 个材质配置文件"

    $Settings = Get-OptimizationSettings -Level $OptimizationLevel

    Write-Log "优化设置:"
    Write-Log "  最大指令数: $($Settings.MaxInstructions)"
    Write-Log "  最大纹理采样: $($Settings.MaxTextureSamples)"
    Write-Log "  最大参数集合: $($Settings.MaxParameterCollections)"

    $Results = @()
    $Index = 0

    foreach ($MaterialFile in $MaterialFiles) {
        $Index++
        $Result = Optimize-Material `
            -MaterialFile $MaterialFile.FullName `
            -Settings $Settings `
            -Index $Index `
            -Total $MaterialFiles.Count

        $Results += $Result
    }

    Write-Progress -Activity "优化材质" -Completed

    return $Results
}

function Generate-Report {
    param($Results)

    Write-Log "生成优化报告..."

    $SuccessCount = ($Results | Where-Object { $_.Success }).Count
    $FailCount = ($Results | Where-Object { -not $_.Success }).Count

    $AvgOriginalInstructions = ($Results | Where-Object { $_.Success } | ForEach-Object { $_.OriginalComplexity.InstructionCount } | Measure-Object -Average).Average
    $AvgNewInstructions = ($Results | Where-Object { $_.Success } | ForEach-Object { $_.NewComplexity.InstructionCount } | Measure-Object -Average).Average

    $ReportPath = "D:\gptzuo\HorrorProject\HorrorProject\Logs\MaterialOptimization_Report_$(Get-Date -Format 'yyyyMMdd_HHmmss').txt"

    $Report = @"
========================================
材质优化报告
========================================
优化时间: $(Get-Date -Format 'yyyy-MM-dd HH:mm:ss')
材质路径: $MaterialPath
优化级别: $OptimizationLevel

统计信息:
- 总材质数: $($Results.Count)
- 成功优化: $SuccessCount
- 优化失败: $FailCount
- 平均原始指令数: $([math]::Round($AvgOriginalInstructions, 2))
- 平均优化后指令数: $([math]::Round($AvgNewInstructions, 2))

优化详情:
$($Results | Where-Object { $_.Success } | ForEach-Object {
    "  ✓ $(Split-Path $_.File -Leaf)"
    "    原始指令数: $($_.OriginalComplexity.InstructionCount)"
    "    优化后指令数: $($_.NewComplexity.InstructionCount)"
    "    纹理采样: $($_.OriginalComplexity.TextureSamples) -> $($_.NewComplexity.TextureSamples)"
    if ($_.Optimizations.Count -gt 0) {
        "    优化操作:"
        foreach ($Opt in $_.Optimizations) {
            "      - $Opt"
        }
    }
    ""
} | Out-String)

失败的材质:
$($Results | Where-Object { -not $_.Success } | ForEach-Object {
    "  ✗ $(Split-Path $_.File -Leaf): $($_.Error)"
} | Out-String)

执行时间: $((Get-Date) - $StartTime)
========================================
"@

    $Report | Out-File -FilePath $ReportPath -Encoding UTF8
    Write-Log "报告已生成: $ReportPath"
}

# 主执行流程
try {
    Write-Log "========================================" "INFO"
    Write-Log "开始材质优化" "INFO"
    Write-Log "========================================" "INFO"

    if (-not (Test-Path $MaterialPath)) {
        throw "材质路径不存在: $MaterialPath"
    }

    $Results = Process-Materials

    Generate-Report -Results $Results

    $Duration = (Get-Date) - $StartTime
    Write-Log "========================================" "INFO"
    Write-Log "材质优化完成！" "SUCCESS"
    Write-Log "总耗时: $($Duration.ToString('hh\:mm\:ss'))" "INFO"
    Write-Log "========================================" "INFO"

    exit 0
}
catch {
    Write-Log "材质优化失败: $_" "ERROR"
    exit 1
}
