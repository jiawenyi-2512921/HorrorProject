# LODGeneration.ps1 - LOD自动生成
param(
    [Parameter(Mandatory=$true)]
    [string]$MeshPath,
    [int]$LODCount = 3,
    [float]$ReductionPercentage = 0.5,
    [string]$LogPath = "D:\gptzuo\HorrorProject\HorrorProject\Logs\LODGeneration.log",
    [switch]$ProcessSubfolders
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

function Calculate-LODSettings {
    param([int]$LODLevel, [float]$BaseReduction)

    $ReductionPercent = [math]::Pow($BaseReduction, $LODLevel)
    $ScreenSize = switch ($LODLevel) {
        1 { 0.5 }
        2 { 0.25 }
        3 { 0.125 }
        4 { 0.0625 }
        default { 0.03125 }
    }

    return @{
        Level = $LODLevel
        ReductionPercent = $ReductionPercent
        ScreenSize = $ScreenSize
    }
}

function Generate-LODChain {
    param(
        [string]$MeshFile,
        [int]$Index,
        [int]$Total
    )

    $ProgressPercent = [math]::Round(($Index / $Total) * 100, 2)
    Write-Progress -Activity "生成LOD" -Status "$ProgressPercent% 完成" -PercentComplete $ProgressPercent

    Write-Log "生成LOD [$Index/$Total]: $(Split-Path $MeshFile -Leaf)"

    try {
        $MeshConfig = Get-Content -Path $MeshFile -Raw | ConvertFrom-Json

        $LODSettings = @()

        for ($i = 1; $i -le $LODCount; $i++) {
            $Settings = Calculate-LODSettings -LODLevel $i -BaseReduction $ReductionPercentage

            Write-Log "  LOD $i : 减面 $([math]::Round($Settings.ReductionPercent * 100, 2))%, 屏幕大小 $($Settings.ScreenSize)"

            $LODSettings += @{
                LODIndex = $i
                ReductionSettings = @{
                    PercentTriangles = $Settings.ReductionPercent
                    MaxDeviation = 1.0
                    WeldingThreshold = 0.1
                    NormalsThreshold = 60.0
                    MaxBonesPerVertex = 4
                }
                ScreenSize = @{
                    Default = $Settings.ScreenSize
                }
                BuildSettings = @{
                    bRecomputeNormals = $true
                    bRecomputeTangents = $true
                    bUseMikkTSpace = $true
                    bComputeWeightedNormals = $true
                    bRemoveDegenerates = $true
                    bBuildAdjacencyBuffer = $true
                    bUseHighPrecisionTangentBasis = $false
                }
            }
        }

        # 添加LOD配置到网格配置
        $MeshConfig | Add-Member -NotePropertyName "LODSettings" -NotePropertyValue $LODSettings -Force
        $MeshConfig | Add-Member -NotePropertyName "LODGenerationDate" -NotePropertyValue (Get-Date -Format "yyyy-MM-dd HH:mm:ss") -Force

        # 保存配置
        $MeshConfig | ConvertTo-Json -Depth 10 | Out-File -FilePath $MeshFile -Encoding UTF8

        Write-Log "成功生成 $LODCount 个LOD级别" "SUCCESS"

        return @{
            Success = $true
            File = $MeshFile
            LODCount = $LODCount
            LODSettings = $LODSettings
        }
    }
    catch {
        Write-Log "生成LOD失败: $_" "ERROR"
        return @{
            Success = $false
            File = $MeshFile
            Error = $_.Exception.Message
        }
    }
}

function Process-Meshes {
    Write-Log "扫描网格文件..."

    $SearchOption = if ($ProcessSubfolders) { "-Recurse" } else { "" }
    $MeshFiles = Get-ChildItem -Path $MeshPath -Include "*.json" -File @SearchOption

    Write-Log "找到 $($MeshFiles.Count) 个网格配置文件"

    $Results = @()
    $Index = 0

    foreach ($MeshFile in $MeshFiles) {
        $Index++
        $Result = Generate-LODChain -MeshFile $MeshFile.FullName -Index $Index -Total $MeshFiles.Count
        $Results += $Result
    }

    Write-Progress -Activity "生成LOD" -Completed

    return $Results
}

function Generate-Report {
    param($Results)

    Write-Log "生成LOD报告..."

    $SuccessCount = ($Results | Where-Object { $_.Success }).Count
    $FailCount = ($Results | Where-Object { -not $_.Success }).Count
    $TotalLODs = ($Results | Where-Object { $_.Success } | Measure-Object -Property LODCount -Sum).Sum

    $ReportPath = "D:\gptzuo\HorrorProject\HorrorProject\Logs\LODGeneration_Report_$(Get-Date -Format 'yyyyMMdd_HHmmss').txt"

    $Report = @"
========================================
LOD生成报告
========================================
生成时间: $(Get-Date -Format 'yyyy-MM-dd HH:mm:ss')
网格路径: $MeshPath
LOD级别数: $LODCount
减面百分比: $([math]::Round($ReductionPercentage * 100, 2))%

统计信息:
- 总网格数: $($Results.Count)
- 成功生成: $SuccessCount
- 生成失败: $FailCount
- 总LOD数: $TotalLODs

生成详情:
$($Results | Where-Object { $_.Success } | ForEach-Object {
    "  ✓ $(Split-Path $_.File -Leaf)"
    "    LOD级别数: $($_.LODCount)"
    foreach ($LOD in $_.LODSettings) {
        "      LOD $($LOD.LODIndex): 减面 $([math]::Round($LOD.ReductionSettings.PercentTriangles * 100, 2))%, 屏幕大小 $($LOD.ScreenSize.Default)"
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
    Write-Log "开始生成LOD" "INFO"
    Write-Log "========================================" "INFO"

    if (-not (Test-Path $MeshPath)) {
        throw "网格路径不存在: $MeshPath"
    }

    $Results = Process-Meshes

    Generate-Report -Results $Results

    $Duration = (Get-Date) - $StartTime
    Write-Log "========================================" "INFO"
    Write-Log "LOD生成完成！" "SUCCESS"
    Write-Log "总耗时: $($Duration.ToString('hh\:mm\:ss'))" "INFO"
    Write-Log "========================================" "INFO"

    exit 0
}
catch {
    Write-Log "生成LOD失败: $_" "ERROR"
    exit 1
}
