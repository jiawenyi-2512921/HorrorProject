# MeshBatchImport.ps1 - 批量导入网格资产
param(
    [Parameter(Mandatory=$true)]
    [string]$SourcePath,
    [Parameter(Mandatory=$true)]
    [string]$DestinationPath,
    [string]$UProjectPath = "D:\gptzuo\HorrorProject\HorrorProject\HorrorProject.uproject",
    [switch]$GenerateLODs = $true,
    [switch]$GenerateCollision = $true,
    [switch]$ImportMaterials = $true,
    [string]$LogPath = "D:\gptzuo\HorrorProject\HorrorProject\Logs\MeshImport.log"
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

function Get-MeshFiles {
    Write-Log "扫描网格文件..."

    $MeshExtensions = @("*.fbx", "*.obj", "*.dae", "*.3ds", "*.blend")
    $MeshFiles = @()

    foreach ($ext in $MeshExtensions) {
        $Files = Get-ChildItem -Path $SourcePath -Filter $ext -Recurse -File
        $MeshFiles += $Files
    }

    Write-Log "找到 $($MeshFiles.Count) 个网格文件"
    return $MeshFiles
}

function Get-MeshInfo {
    param([string]$FilePath)

    try {
        # 使用FBX SDK或其他工具获取网格信息
        # 这里简化为文件大小
        $FileInfo = Get-Item $FilePath

        return @{
            FileName = $FileInfo.Name
            Size = $FileInfo.Length
            Extension = $FileInfo.Extension
        }
    }
    catch {
        Write-Log "无法获取网格信息: $_" "ERROR"
        return $null
    }
}

function Import-MeshFile {
    param(
        [System.IO.FileInfo]$MeshFile,
        [int]$Index,
        [int]$Total
    )

    $ProgressPercent = [math]::Round(($Index / $Total) * 100, 2)
    Write-Progress -Activity "导入网格文件" -Status "$ProgressPercent% 完成" -PercentComplete $ProgressPercent

    Write-Log "导入网格 [$Index/$Total]: $($MeshFile.Name)"

    $RelativePath = $MeshFile.FullName.Substring($SourcePath.Length).TrimStart('\', '/')
    $TargetPath = Join-Path $DestinationPath $RelativePath
    $TargetDir = Split-Path $TargetPath -Parent

    if (-not (Test-Path $TargetDir)) {
        New-Item -ItemType Directory -Path $TargetDir -Force | Out-Null
    }

    try {
        # 复制网格文件
        Copy-Item -Path $MeshFile.FullName -Destination $TargetPath -Force

        # 创建导入配置
        $ImportConfig = @{
            MeshFile = $TargetPath
            GenerateLODs = $GenerateLODs.IsPresent
            GenerateCollision = $GenerateCollision.IsPresent
            ImportMaterials = $ImportMaterials.IsPresent
            ImportSettings = @{
                CombineMeshes = $false
                ImportTextures = $ImportMaterials.IsPresent
                ImportAnimations = $false
                ConvertScene = $true
                ForceFrontXAxis = $false
                ConvertSceneUnit = $true
            }
        }

        $ConfigFile = "$TargetPath.import.json"
        $ImportConfig | ConvertTo-Json -Depth 10 | Out-File -FilePath $ConfigFile -Encoding UTF8

        Write-Log "成功导入: $($MeshFile.Name)" "SUCCESS"

        return @{
            Success = $true
            File = $MeshFile.Name
            Size = $MeshFile.Length
            Path = $TargetPath
            ConfigFile = $ConfigFile
        }
    }
    catch {
        Write-Log "导入失败: $($MeshFile.Name) - $_" "ERROR"
        return @{
            Success = $false
            File = $MeshFile.Name
            Error = $_.Exception.Message
        }
    }
}

function Generate-ImportReport {
    param($Results)

    Write-Log "生成导入报告..."

    $SuccessCount = ($Results | Where-Object { $_.Success }).Count
    $FailCount = ($Results | Where-Object { -not $_.Success }).Count
    $TotalSize = ($Results | Where-Object { $_.Success } | Measure-Object -Property Size -Sum).Sum

    $ExtStats = $Results | Where-Object { $_.Success } |
        ForEach-Object { [System.IO.Path]::GetExtension($_.File) } |
        Group-Object | Select-Object Name, Count

    $ReportPath = "D:\gptzuo\HorrorProject\HorrorProject\Logs\MeshImport_Report_$(Get-Date -Format 'yyyyMMdd_HHmmss').txt"

    $Report = @"
========================================
网格批量导入报告
========================================
导入时间: $(Get-Date -Format 'yyyy-MM-dd HH:mm:ss')
源路径: $SourcePath
目标路径: $DestinationPath

导入选项:
- 生成LOD: $($GenerateLODs.IsPresent)
- 生成碰撞: $($GenerateCollision.IsPresent)
- 导入材质: $($ImportMaterials.IsPresent)

统计信息:
- 总文件数: $($Results.Count)
- 成功导入: $SuccessCount
- 导入失败: $FailCount
- 总大小: $([math]::Round($TotalSize / 1MB, 2)) MB

格式分布:
$($ExtStats | ForEach-Object { "  - $($_.Name): $($_.Count) 个文件" } | Out-String)

成功导入的文件:
$($Results | Where-Object { $_.Success } | ForEach-Object {
    "  ✓ $($_.File) - $([math]::Round($_.Size / 1KB, 2)) KB"
} | Out-String)

失败的文件:
$($Results | Where-Object { -not $_.Success } | ForEach-Object {
    "  ✗ $($_.File) - $($_.Error)"
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
    Write-Log "开始网格批量导入" "INFO"
    Write-Log "========================================" "INFO"

    if (-not (Test-Path $SourcePath)) {
        throw "源路径不存在: $SourcePath"
    }

    $MeshFiles = Get-MeshFiles

    if ($MeshFiles.Count -eq 0) {
        Write-Log "未找到网格文件，退出" "WARNING"
        exit 0
    }

    $Results = @()
    $Index = 0

    foreach ($MeshFile in $MeshFiles) {
        $Index++
        $Result = Import-MeshFile -MeshFile $MeshFile -Index $Index -Total $MeshFiles.Count
        $Results += $Result
    }

    Write-Progress -Activity "导入网格文件" -Completed

    Generate-ImportReport -Results $Results

    $Duration = (Get-Date) - $StartTime
    Write-Log "========================================" "INFO"
    Write-Log "网格导入完成！" "SUCCESS"
    Write-Log "总耗时: $($Duration.ToString('hh\:mm\:ss'))" "INFO"
    Write-Log "========================================" "INFO"

    exit 0
}
catch {
    Write-Log "网格导入失败: $_" "ERROR"
    exit 1
}
