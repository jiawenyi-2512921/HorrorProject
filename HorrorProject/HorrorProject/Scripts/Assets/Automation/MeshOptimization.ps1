# MeshOptimization.ps1 - 网格优化
param(
    [Parameter(Mandatory=$true)]
    [string]$MeshPath,
    [ValidateSet("Low", "Medium", "High")]
    [string]$OptimizationLevel = "Medium",
    [string]$LogPath = "D:\gptzuo\HorrorProject\HorrorProject\Logs\MeshOptimization.log",
    [switch]$RemoveDegenerates,
    [switch]$WeldVertices,
    [switch]$OptimizeForGPU
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
                WeldingThreshold = 0.5
                MaxVertices = 100000
                MaxTriangles = 200000
                NormalThreshold = 80.0
            }
        }
        "Medium" {
            return @{
                WeldingThreshold = 0.1
                MaxVertices = 65536
                MaxTriangles = 100000
                NormalThreshold = 60.0
            }
        }
        "High" {
            return @{
                WeldingThreshold = 0.01
                MaxVertices = 32768
                MaxTriangles = 50000
                NormalThreshold = 45.0
            }
        }
    }
}

function Optimize-Mesh {
    param(
        [string]$MeshFile,
        [hashtable]$Settings,
        [int]$Index,
        [int]$Total
    )

    $ProgressPercent = [math]::Round(($Index / $Total) * 100, 2)
    Write-Progress -Activity "优化网格" -Status "$ProgressPercent% 完成" -PercentComplete $ProgressPercent

    Write-Log "优化网格 [$Index/$Total]: $(Split-Path $MeshFile -Leaf)"

    try {
        $MeshConfig = Get-Content -Path $MeshFile -Raw | ConvertFrom-Json

        $Optimizations = @()

        # 优化配置
        $OptimizationConfig = @{
            BuildSettings = @{
                bRecomputeNormals = $true
                bRecomputeTangents = $true
                bUseMikkTSpace = $true
                bComputeWeightedNormals = $true
                bRemoveDegenerates = $RemoveDegenerates.IsPresent
                bBuildAdjacencyBuffer = $true
                bUseHighPrecisionTangentBasis = $false
                bUseFullPrecisionUVs = $false
                bGenerateLightmapUVs = $true
            }
            OptimizationSettings = @{
                WeldingThreshold = $Settings.WeldingThreshold
                NormalsThreshold = $Settings.NormalThreshold
                MaxVertices = $Settings.MaxVertices
                MaxTriangles = $Settings.MaxTriangles
            }
        }

        # 移除退化三角形
        if ($RemoveDegenerates) {
            $Optimizations += "移除退化三角形"
            Write-Log "  启用: 移除退化三角形"
        }

        # 焊接顶点
        if ($WeldVertices) {
            $Optimizations += "焊接顶点 (阈值: $($Settings.WeldingThreshold))"
            Write-Log "  启用: 焊接顶点 (阈值: $($Settings.WeldingThreshold))"
        }

        # GPU优化
        if ($OptimizeForGPU) {
            $OptimizationConfig.GPUOptimization = @{
                bOptimizeForGPU = $true
                CacheOptimize = $true
                VertexCacheSize = 32
            }
            $Optimizations += "GPU缓存优化"
            Write-Log "  启用: GPU缓存优化"
        }

        # 添加优化配置
        $MeshConfig | Add-Member -NotePropertyName "OptimizationConfig" -NotePropertyValue $OptimizationConfig -Force
        $MeshConfig | Add-Member -NotePropertyName "OptimizationInfo" -NotePropertyValue @{
            OptimizationLevel = $OptimizationLevel
            OptimizedDate = Get-Date -Format "yyyy-MM-dd HH:mm:ss"
            Optimizations = $Optimizations
        } -Force

        # 保存配置
        $MeshConfig | ConvertTo-Json -Depth 10 | Out-File -FilePath $MeshFile -Encoding UTF8

        Write-Log "网格优化完成" "SUCCESS"

        return @{
            Success = $true
            File = $MeshFile
            OptimizationLevel = $OptimizationLevel
            Optimizations = $Optimizations
        }
    }
    catch {
        Write-Log "优化网格失败: $_" "ERROR"
        return @{
            Success = $false
            File = $MeshFile
            Error = $_.Exception.Message
        }
    }
}

function Process-Meshes {
    Write-Log "扫描网格文件..."

    $MeshFiles = Get-ChildItem -Path $MeshPath -Include "*.json" -Recurse -File

    Write-Log "找到 $($MeshFiles.Count) 个网格配置文件"

    $Settings = Get-OptimizationSettings -Level $OptimizationLevel

    Write-Log "优化设置:"
    Write-Log "  焊接阈值: $($Settings.WeldingThreshold)"
    Write-Log "  最大顶点数: $($Settings.MaxVertices)"
    Write-Log "  最大三角形数: $($Settings.MaxTriangles)"
    Write-Log "  法线阈值: $($Settings.NormalThreshold)"

    $Results = @()
    $Index = 0

    foreach ($MeshFile in $MeshFiles) {
        $Index++
        $Result = Optimize-Mesh -MeshFile $MeshFile.FullName -Settings $Settings -Index $Index -Total $MeshFiles.Count
        $Results += $Result
    }

    Write-Progress -Activity "优化网格" -Completed

    return $Results
}

function Generate-Report {
    param($Results)

    Write-Log "生成优化报告..."

    $SuccessCount = ($Results | Where-Object { $_.Success }).Count
    $FailCount = ($Results | Where-Object { -not $_.Success }).Count

    $ReportPath = "D:\gptzuo\HorrorProject\HorrorProject\Logs\MeshOptimization_Report_$(Get-Date -Format 'yyyyMMdd_HHmmss').txt"

    $Report = @"
========================================
网格优化报告
========================================
优化时间: $(Get-Date -Format 'yyyy-MM-dd HH:mm:ss')
网格路径: $MeshPath
优化级别: $OptimizationLevel

统计信息:
- 总网格数: $($Results.Count)
- 成功优化: $SuccessCount
- 优化失败: $FailCount

优化详情:
$($Results | Where-Object { $_.Success } | ForEach-Object {
    "  ✓ $(Split-Path $_.File -Leaf)"
    "    优化级别: $($_.OptimizationLevel)"
    if ($_.Optimizations.Count -gt 0) {
        "    优化操作:"
        foreach ($Opt in $_.Optimizations) {
            "      - $Opt"
        }
    }
    ""
} | Out-String)

失败的网格:
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
    Write-Log "开始网格优化" "INFO"
    Write-Log "========================================" "INFO"

    if (-not (Test-Path $MeshPath)) {
        throw "网格路径不存在: $MeshPath"
    }

    $Results = Process-Meshes

    Generate-Report -Results $Results

    $Duration = (Get-Date) - $StartTime
    Write-Log "========================================" "INFO"
    Write-Log "网格优化完成！" "SUCCESS"
    Write-Log "总耗时: $($Duration.ToString('hh\:mm\:ss'))" "INFO"
    Write-Log "========================================" "INFO"

    exit 0
}
catch {
    Write-Log "网格优化失败: $_" "ERROR"
    exit 1
}
