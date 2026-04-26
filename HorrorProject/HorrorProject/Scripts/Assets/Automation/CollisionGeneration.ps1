# CollisionGeneration.ps1 - 碰撞体生成
param(
    [Parameter(Mandatory=$true)]
    [string]$MeshPath,
    [ValidateSet("Box", "Sphere", "Capsule", "Convex", "Complex")]
    [string]$CollisionType = "Convex",
    [int]$MaxHullCount = 4,
    [int]$MaxHullVerts = 16,
    [string]$LogPath = "D:\gptzuo\HorrorProject\HorrorProject\Logs\CollisionGeneration.log",
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

function Get-CollisionSettings {
    param([string]$Type)

    switch ($Type) {
        "Box" {
            return @{
                Type = "Box"
                Method = "UseSimpleAsComplex"
                Complexity = "Simple"
            }
        }
        "Sphere" {
            return @{
                Type = "Sphere"
                Method = "UseSimpleAsComplex"
                Complexity = "Simple"
            }
        }
        "Capsule" {
            return @{
                Type = "Capsule"
                Method = "UseSimpleAsComplex"
                Complexity = "Simple"
            }
        }
        "Convex" {
            return @{
                Type = "Convex"
                Method = "UseComplexAsSimple"
                Complexity = "Complex"
                HullCount = $MaxHullCount
                MaxHullVerts = $MaxHullVerts
                HullPrecision = 100000
            }
        }
        "Complex" {
            return @{
                Type = "Complex"
                Method = "UseComplexAsSimple"
                Complexity = "Complex"
            }
        }
    }
}

function Generate-Collision {
    param(
        [string]$MeshFile,
        [int]$Index,
        [int]$Total
    )

    $ProgressPercent = [math]::Round(($Index / $Total) * 100, 2)
    Write-Progress -Activity "生成碰撞体" -Status "$ProgressPercent% 完成" -PercentComplete $ProgressPercent

    Write-Log "生成碰撞 [$Index/$Total]: $(Split-Path $MeshFile -Leaf)"

    try {
        $MeshConfig = Get-Content -Path $MeshFile -Raw | ConvertFrom-Json

        $CollisionSettings = Get-CollisionSettings -Type $CollisionType

        Write-Log "  碰撞类型: $CollisionType"

        # 创建碰撞配置
        $CollisionConfig = @{
            CollisionComplexity = $CollisionSettings.Complexity
            CollisionTraceFlag = "CTF_UseDefault"
            bCustomizedCollision = $true
            CollisionSettings = $CollisionSettings
            GenerationMethod = $CollisionSettings.Method
        }

        # 如果是凸包碰撞，添加额外设置
        if ($CollisionType -eq "Convex") {
            $CollisionConfig.ConvexDecomposition = @{
                HullCount = $MaxHullCount
                MaxHullVerts = $MaxHullVerts
                HullPrecision = 100000
            }
            Write-Log "  凸包设置: $MaxHullCount 个凸包, 每个最多 $MaxHullVerts 个顶点"
        }

        # 添加碰撞配置到网格配置
        $MeshConfig | Add-Member -NotePropertyName "CollisionConfig" -NotePropertyValue $CollisionConfig -Force
        $MeshConfig | Add-Member -NotePropertyName "CollisionGenerationDate" -NotePropertyValue (Get-Date -Format "yyyy-MM-dd HH:mm:ss") -Force

        # 保存配置
        $MeshConfig | ConvertTo-Json -Depth 10 | Out-File -FilePath $MeshFile -Encoding UTF8

        Write-Log "成功生成碰撞体" "SUCCESS"

        return @{
            Success = $true
            File = $MeshFile
            CollisionType = $CollisionType
            Settings = $CollisionSettings
        }
    }
    catch {
        Write-Log "生成碰撞体失败: $_" "ERROR"
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
        $Result = Generate-Collision -MeshFile $MeshFile.FullName -Index $Index -Total $MeshFiles.Count
        $Results += $Result
    }

    Write-Progress -Activity "生成碰撞体" -Completed

    return $Results
}

function Generate-Report {
    param($Results)

    Write-Log "生成碰撞体报告..."

    $SuccessCount = ($Results | Where-Object { $_.Success }).Count
    $FailCount = ($Results | Where-Object { -not $_.Success }).Count

    $TypeStats = $Results | Where-Object { $_.Success } | Group-Object -Property CollisionType | Select-Object Name, Count

    $ReportPath = "D:\gptzuo\HorrorProject\HorrorProject\Logs\CollisionGeneration_Report_$(Get-Date -Format 'yyyyMMdd_HHmmss').txt"

    $Report = @"
========================================
碰撞体生成报告
========================================
生成时间: $(Get-Date -Format 'yyyy-MM-dd HH:mm:ss')
网格路径: $MeshPath
碰撞类型: $CollisionType

统计信息:
- 总网格数: $($Results.Count)
- 成功生成: $SuccessCount
- 生成失败: $FailCount

碰撞类型分布:
$($TypeStats | ForEach-Object { "  - $($_.Name): $($_.Count) 个网格" } | Out-String)

生成详情:
$($Results | Where-Object { $_.Success } | ForEach-Object {
    "  ✓ $(Split-Path $_.File -Leaf)"
    "    碰撞类型: $($_.CollisionType)"
    "    复杂度: $($_.Settings.Complexity)"
    if ($_.Settings.HullCount) {
        "    凸包数: $($_.Settings.HullCount)"
        "    最大顶点数: $($_.Settings.MaxHullVerts)"
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
    Write-Log "开始生成碰撞体" "INFO"
    Write-Log "========================================" "INFO"

    if (-not (Test-Path $MeshPath)) {
        throw "网格路径不存在: $MeshPath"
    }

    $Results = Process-Meshes

    Generate-Report -Results $Results

    $Duration = (Get-Date) - $StartTime
    Write-Log "========================================" "INFO"
    Write-Log "碰撞体生成完成！" "SUCCESS"
    Write-Log "总耗时: $($Duration.ToString('hh\:mm\:ss'))" "INFO"
    Write-Log "========================================" "INFO"

    exit 0
}
catch {
    Write-Log "生成碰撞体失败: $_" "ERROR"
    exit 1
}
